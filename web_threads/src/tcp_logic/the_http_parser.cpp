#include "the_http_parser.h"
#include "web_logic.h"
#include "log_util.h"
#include "mpart_body_processor.h"
#include "singleton.h"
#include "web_util.h"

namespace tcp
{
namespace web
{
namespace http
{
Req::Req() : client_ip(), url(), schema(), host(), path(), query(), queries(),
    fragment(), user_info(), headers(), body(), mpart_body_ctx()
{
    method = HTTP_GET;
    major_version = 1;
    minor_version = 1;
    port = 0;
    url_decode = false;
}

Req::~Req()
{
}

void Req::Reset()
{
    method = HTTP_GET;
    major_version = 0;
    minor_version = 0;
    client_ip = "";
    url.clear();
    schema.clear();
    host.clear();
    port = 0;
    path.clear();
    query.clear();
    queries.clear();
    fragment.clear();
    user_info.clear();
    headers.clear();
    url_decode = false;
    body.clear();
    mpart_body_ctx.Reset();
}

void Req::ParseURL(const char* at, size_t length)
{
    this->url.assign(at, length);

    struct http_parser_url u;
    http_parser_url_init(&u);

    if (http_parser_parse_url(this->url.c_str(), this->url.length(), 0, &u) != 0)
    {
        LOG_ERROR("parse url error: " << this->url);
        return;
    }

    if (u.field_set & (1 << UF_SCHEMA))
    {
        this->schema = this->url.substr(u.field_data[UF_SCHEMA].off, u.field_data[UF_SCHEMA].len);
        LOG_DEBUG("schema: " << this->schema);
    }

    if (u.field_set & (1 << UF_HOST))
    {
        this->host = this->url.substr(u.field_data[UF_HOST].off, u.field_data[UF_HOST].len);
        LOG_DEBUG("host: " << this->host);
    }

    if (u.field_set & (1 << UF_PORT))
    {
        this->port = u.port;
        LOG_DEBUG("port: " << this->port);
    }

    if (u.field_set & (1 << UF_PATH))
    {
        this->path = this->url.substr(u.field_data[UF_PATH].off, u.field_data[UF_PATH].len);
        LOG_DEBUG("path: " << this->path);
    }

    if (u.field_set & (1 << UF_QUERY))
    {
        this->query = this->url.substr(u.field_data[UF_QUERY].off, u.field_data[UF_QUERY].len);
        LOG_DEBUG("origin query: " << this->query);
    }

    if (u.field_set & (1 << UF_FRAGMENT))
    {
        this->fragment = this->url.substr(u.field_data[UF_FRAGMENT].off, u.field_data[UF_FRAGMENT].len);
        LOG_DEBUG("fragment: " << this->fragment);
    }

    if (u.field_set & (1 << UF_USERINFO))
    {
        this->user_info = this->url.substr(u.field_data[UF_USERINFO].off, u.field_data[UF_USERINFO].len);
        LOG_DEBUG("user info: " << this->user_info);
    }
}

void Req::ParseQuery()
{
    char* str1, *str2, *token, *subtoken;
    char* saveptr1, *saveptr2;
    int j;

    std::unique_ptr<char[]> query(new char[this->query.size() + 1]);
    if (nullptr == query)
    {
        LOG_ERROR("failed to alloc memory");
        return;
    }

    memcpy(query.get(), this->query.data(), this->query.size());
    query[this->query.size()] = '\0';

    for (j = 1, str1 = query.get(); ; ++j, str1 = NULL)
    {
        token = strtok_r(str1, "&", &saveptr1);
        if (token == NULL)
        {
            break;
        }

        std::string key;
        std::string value;

        for (str2 = token; ; str2 = NULL)
        {
            subtoken = strtok_r(str2, "=", &saveptr2);
            if (subtoken == NULL)
            {
                break;
            }

            if (str2 != NULL)
            {
                key = subtoken;
            }
            else
            {
                value = subtoken;
            }
        }

        LOG_DEBUG("query key: " << key << ", value: " << value);
        this->queries.insert(QueryMap::value_type(key, value));
    }
}

void Req::ParseClientIP()
{
    HeaderMap::const_iterator it =  this->headers.find("X-Forwarded-For");
    if (it != this->headers.cend())
    {
        const char* x_forwarded_for = it->second.c_str();
        LOG_DEBUG("X-Forwarded-For: " << x_forwarded_for);

        const char* p = strchr(x_forwarded_for, ',');
        if (nullptr == p)
        {
            this->client_ip.assign(x_forwarded_for);
        }
        else
        {
            this->client_ip.assign(x_forwarded_for, p - x_forwarded_for);
        }
    }
}

std::string Req::Dump()
{
    char buf[1024] = "";
    int n = 0;

    if (this->path.empty())
    {
        this->path = "/";
    }

    if (query.empty())
    {
        n = snprintf(buf, sizeof(buf), "%s %s HTTP/%d.%d\r\n",
                     http_method_str(this->method), this->path.c_str(),
                     this->major_version, this->minor_version);
    }
    else
    {
        n = snprintf(buf, sizeof(buf), "%s %s?%s HTTP/%d.%d\r\n",
                     http_method_str(this->method), this->path.c_str(), this->query.c_str(),
                     this->major_version, this->minor_version);
    }

    std::string s(buf, n);

    HeaderMap::const_iterator it = this->headers.cbegin();
    while (it != this->headers.cend())
    {
        int n = snprintf(buf, sizeof(buf), "%s: %s\r\n", it->first.c_str(), it->second.c_str());
        s.append(buf, n);
        ++it;
    }

    s.append("\r\n");
    s.append(this->body);

    return s;
}

class ParserSettings
{
public:
    ParserSettings()
    {
        http_parser_settings_init(&settings_);
        settings_.on_message_begin = Parser::OnMessageBegin;
        settings_.on_url = Parser::OnURL;
        settings_.on_header_field = Parser::OnHeaderField;
        settings_.on_header_value = Parser::OnHeaderValue;
        settings_.on_headers_complete = Parser::OnHeadersComplete;
        settings_.on_body = Parser::OnBody;
        settings_.on_message_complete = Parser::OnMessageComplete;
    }

    const struct http_parser_settings* Get() const
    {
        return &settings_;
    }

private:
    struct http_parser_settings settings_;
};

#define HTTPParserSettings Singleton<ParserSettings>::Instance()

Parser::Parser() : http_req_(), last_header_name_()
{
    web_logic_ = nullptr;
    conn_id_ = INVALID_CONN_ID;

    http_parser_init(&parser_, HTTP_REQUEST);
    parser_.data = this;

    complete_ = false;
}

Parser::~Parser()
{
}

int Parser::Execute(const char* buffer, size_t count)
{
    LOG_TRACE("http parser execute");

    size_t n = http_parser_execute(&parser_, HTTPParserSettings->Get(), buffer, count);
    if (parser_.upgrade)
    {
        LOG_TRACE("** upgrade to websocket **");

        if (web_logic_ != nullptr)
        {
            web_logic_->OnUpgrade(conn_id_, http_req_, buffer + n, count - n);
        }

        return 0;
    }
    else if (n != count)
    {
        LOG_ERROR("failed to parse " << std::string(buffer, count)
                  << ", err msg: " << http_errno_description((http_errno) parser_.http_errno));
        return parser_.http_errno;
    }

    if (complete_)
    {
        if (http_req_.url_decode)
        {
            if (http_req_.query.size() > 0)
            {
                // 对query要做url decode
                std::string query = http_req_.query;
                const size_t len = URLDecode((char*) query.data(), query.size());
                http_req_.query.assign(query.data(), len);
                LOG_DEBUG("decoded query: " << http_req_.query);
            }

            if (http_req_.body.size() > 0)
            {
                // 对body要做url decode
                std::string body = http_req_.body;
                const size_t len = URLDecode((char*) body.data(), body.size());
                http_req_.body.assign(body.data(), len);
                LOG_DEBUG("decoded body: " << http_req_.body);
            }
        }

        if (http_req_.query.size() > 0)
        {
            http_req_.ParseQuery();
        }

        bool conn_closed = false;

        if (web_logic_ != nullptr)
        {
            web_logic_->OnHTTPReq(conn_closed, conn_id_, http_req_);
        }

        // 反复使用。消息处理器中可能已经关闭了连接，这里要先判断
        if (!conn_closed)
        {
            http_req_.Reset();
            last_header_name_ = "";
            complete_ = false;

            // 恢复回调
            const_cast<struct http_parser_settings*>(HTTPParserSettings->Get())->on_body = Parser::OnBody;
        }
    }

    return 0;
}

int Parser::OnMessageBegin(http_parser* parser)
{
    LOG_TRACE("Parser::OnMessageBegin");
    Parser* hp = static_cast<Parser*>(parser->data);

    if (hp->web_logic_ != nullptr)
    {
        hp->web_logic_->RecordPartMsg(hp->conn_id_);
    }

    return 0;
}

int Parser::OnURL(http_parser* parser, const char* at, size_t length)
{
    LOG_TRACE("Parser::OnURL");

    Parser* hp = static_cast<Parser*>(parser->data);
    hp->http_req_.ParseURL(at, length);

    if (hp->web_logic_ != nullptr)
    {
        hp->web_logic_->RecordPartMsg(hp->conn_id_);
    }

    return 0;
}

int Parser::OnHeaderField(http_parser* parser, const char* at, size_t length)
{
    LOG_TRACE("Parser::OnHeaderField");

    Parser* hp = static_cast<Parser*>(parser->data);
    hp->last_header_name_.assign(at, length);

    if (hp->web_logic_ != nullptr)
    {
        hp->web_logic_->RecordPartMsg(hp->conn_id_);
    }

    return 0;
}

int Parser::OnHeaderValue(http_parser* parser, const char* at, size_t length)
{
    LOG_TRACE("Parser::OnHeaderValue");

    Parser* hp = static_cast<Parser*>(parser->data);
    hp->http_req_.AddHeader(hp->last_header_name_, std::string(at, length));

    if (hp->web_logic_ != nullptr)
    {
        hp->web_logic_->RecordPartMsg(hp->conn_id_);
    }

    return 0;
}

int Parser::OnHeadersComplete(http_parser* parser)
{
    // Scalar valued message information such as status_code, method, and the HTTP version are stored in the parser structure.
    // This data is only temporally stored in http_parser and gets reset on each new message.
    // If this information is needed later, copy it out of the structure during the headers_complete callback

    LOG_TRACE("Parser::OnHeadersComplete");
    Parser* hp = static_cast<Parser*>(parser->data);

    hp->http_req_.method = (http_method) parser->method;
    hp->http_req_.major_version = parser->http_major;
    hp->http_req_.minor_version = parser->http_minor;
    hp->http_req_.ParseClientIP();

    LOG_DEBUG("method: " << hp->http_req_.method
              << ", major version: " << hp->http_req_.major_version
              << ", minor version: " << hp->http_req_.minor_version
              << ", client ip: " << hp->http_req_.client_ip);

    HeaderMap::const_iterator it = hp->http_req_.headers.find("Content-Type");
    if (it != hp->http_req_.headers.end())
    {
        if (0 == strncasecmp(it->second.c_str(), "application/x-www-form-urlencoded", strlen("application/x-www-form-urlencoded")))
        {
            // Content-Type:application/x-www-from-urlencoded时，
            // 会将表单内的数据转换为键值对，形如name=java&age=23，并做urlencode。
            // GET请求时作为query，POST请求时放在body中。
            hp->http_req_.url_decode = true;
        }
        else if (0 == strncasecmp(it->second.c_str(), "multipart/form-data", strlen("multipart/form-data")))
        {
            // Content-Type:multipart/form-data时，
            // 会将表单的数据处理为一条消息，用分隔符分开。既可以上传键值对，也可以同时上传文件。
            // 每部分中，Content-Type表示类型，Content-Disposition表示一些描述信息。
            // 形如：
            //Content-Type: multipart/form-data; boundary=${bound}
            //
            //--${bound}
            //Content-Disposition: form-data; name="file000"; filename="HTTP协议详解.pdf"
            //Content-Type: application/octet-stream
            //
            //file content
            //
            //--${bound}
            //Content-Disposition: form-data; name="Upload"
            //
            //Submit Query
            //
            //--${bound}--
            //

            if (hp->http_req_.mpart_body_ctx.processor.Initialize(&(hp->http_req_)) != 0)
            {
                hp->http_req_.mpart_body_ctx.processor.Finalize();
                return -1;
            }

            // 接管http parser的on_body回调
            const_cast<struct http_parser_settings*>(HTTPParserSettings->Get())->on_body = Parser::OnMPartBody;
        }
    }

    if (hp->web_logic_ != nullptr)
    {
        hp->web_logic_->RecordPartMsg(hp->conn_id_);
    }

    return 0;
}

int Parser::OnBody(http_parser* parser, const char* at, size_t length)
{
    // 注意：body回调可能不止调用一次
    LOG_TRACE("Parser::OnBody");

    Parser* hp = static_cast<Parser*>(parser->data);
    hp->http_req_.AppendBody(at, length);

    if (hp->web_logic_ != nullptr)
    {
        hp->web_logic_->RecordPartMsg(hp->conn_id_);
    }

    return 0;
}

int Parser::OnMessageComplete(http_parser* parser)
{
    LOG_TRACE("Parser::OnMessageComplete");

    Parser* hp = static_cast<Parser*>(parser->data);
    hp->complete_ = true;
    LOG_DEBUG("http req =>\n" << hp->http_req_.Dump());

    return 0;
}

int Parser::OnMPartBody(http_parser* parser, const char* at, size_t length)
{
    Parser* hp = static_cast<Parser*>(parser->data);
    hp->http_req_.mpart_body_ctx.parsed += multipart_parser_execute(
            hp->http_req_.mpart_body_ctx.processor.GetParser(),
            at, length);

    return 0;
}
}
}
}
