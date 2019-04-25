#include "the_http_parser.h"
#include "common_logic.h"
#include "log_util.h"
#include "mpart_body_processor.h"
#include "singleton.h"

namespace tcp
{
namespace http_ws
{
namespace http
{
Req::Req() : ClientIP(), URL(), Schema(), Host(), Path(), Query(), Queries(),
    Fragment(), UserInfo(), Headers(), Body()
{
    Method = HTTP_GET;
    MajorVersion = 1;
    MinorVersion = 1;
    Port = 0;
}

Req::~Req()
{
}

void Req::Reset()
{
    Method = HTTP_GET;
    MajorVersion = 0;
    MinorVersion = 0;
    ClientIP = "";
    URL.clear();
    Schema.clear();
    Host.clear();
    Port = 0;
    Path.clear();
    Query.clear();
    Queries.clear();
    Fragment.clear();
    UserInfo.clear();
    Headers.clear();
    Body.clear();
}

void Req::ParseURL(const char* at, size_t length)
{
    this->URL.assign(at, length);

    struct http_parser_url u;
    http_parser_url_init(&u);

    if (http_parser_parse_url(this->URL.c_str(), this->URL.length(), 0, &u) != 0)
    {
        LOG_ERROR("parse url error: " << this->URL);
        return;
    }

    if (u.field_set & (1 << UF_SCHEMA))
    {
        this->Schema = this->URL.substr(u.field_data[UF_SCHEMA].off, u.field_data[UF_SCHEMA].len);
        LOG_DEBUG("schema: " << this->Schema);
    }

    if (u.field_set & (1 << UF_HOST))
    {
        this->Host = this->URL.substr(u.field_data[UF_HOST].off, u.field_data[UF_HOST].len);
        LOG_DEBUG("host: " << this->Host);
    }

    if (u.field_set & (1 << UF_PORT))
    {
        this->Port = u.port;
        LOG_DEBUG("port: " << this->Port);
    }

    if (u.field_set & (1 << UF_PATH))
    {
        this->Path = this->URL.substr(u.field_data[UF_PATH].off, u.field_data[UF_PATH].len);
        LOG_DEBUG("path: " << this->Path);
    }

    if (u.field_set & (1 << UF_QUERY))
    {
        this->Query = this->URL.substr(u.field_data[UF_QUERY].off, u.field_data[UF_QUERY].len);
        LOG_DEBUG("query: " << this->Query);
        ParseQuery(Query.data(), Query.size());
    }

    if (u.field_set & (1 << UF_FRAGMENT))
    {
        this->Fragment = this->URL.substr(u.field_data[UF_FRAGMENT].off, u.field_data[UF_FRAGMENT].len);
        LOG_DEBUG("fragment: " << this->Fragment);
    }

    if (u.field_set & (1 << UF_USERINFO))
    {
        this->UserInfo = this->URL.substr(u.field_data[UF_USERINFO].off, u.field_data[UF_USERINFO].len);
        LOG_DEBUG("user info: " << this->UserInfo);
    }
}

void Req::ParseQuery(const char* at, size_t length)
{
    this->Query.assign(at, length);

    char* str1, *str2, *token, *subtoken;
    char* saveptr1, *saveptr2;
    int j;

    std::unique_ptr<char[]> query(new char[length + 1]);
    if (nullptr == query)
    {
        LOG_ERROR("failed to alloc memory");
        return;
    }

    memcpy(query.get(), at, length);
    query[length] = '\0';

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
        this->Queries.insert(QueryMap::value_type(key, value));
    }
}

void Req::ParseClientIP()
{
    HeaderMap::const_iterator it =  this->Headers.find("X-Forwarded-For");
    if (it != this->Headers.cend())
    {
        const char* x_forwarded_for = it->second.c_str();
        LOG_DEBUG("X-Forwarded-For: " << x_forwarded_for);

        const char* p = strchr(x_forwarded_for, ',');
        if (nullptr == p)
        {
            this->ClientIP.assign(x_forwarded_for);
        }
        else
        {
            this->ClientIP.assign(x_forwarded_for, p - x_forwarded_for);
        }
    }
}

std::string Req::Dump()
{
    char buf[1024] = "";
    int n = 0;

    if (this->Path.empty())
    {
        this->Path = "/";
    }

    if (Query.empty())
    {
        n = snprintf(buf, sizeof(buf), "%s %s HTTP/%d.%d\r\n",
                     http_method_str(this->Method), this->Path.c_str(),
                     this->MajorVersion, this->MinorVersion);
    }
    else
    {
        n = snprintf(buf, sizeof(buf), "%s %s?%s HTTP/%d.%d\r\n",
                     http_method_str(this->Method), this->Path.c_str(), this->Query.c_str(),
                     this->MajorVersion, this->MinorVersion);
    }

    std::string s(buf, n);

    HeaderMap::const_iterator it = this->Headers.cbegin();
    while (it != this->Headers.cend())
    {
        int n = snprintf(buf, sizeof(buf), "%s: %s\r\n", it->first.c_str(), it->second.c_str());
        s.append(buf, n);
        ++it;
    }

    s.append("\r\n");
    s.append(this->Body);

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
    http_ws_tcp_common_logic_ = nullptr;
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

        if (http_ws_tcp_common_logic_ != nullptr)
        {
            http_ws_tcp_common_logic_->OnUpgrade(conn_id_, http_req_, buffer + n, count - n);
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
        if (http_ws_tcp_common_logic_ != nullptr)
        {
            http_ws_tcp_common_logic_->OnHTTPReq(conn_id_, http_req_);
        }

        // 反复使用
        http_req_.Reset();
        last_header_name_ = "";
        complete_ = false;
    }

    return 0;
}

int Parser::OnMessageBegin(http_parser* parser)
{
    LOG_TRACE("Parser::OnMessageBegin");
    Parser* hp = static_cast<Parser*>(parser->data);

    if (hp->http_ws_tcp_common_logic_ != nullptr)
    {
        hp->http_ws_tcp_common_logic_->RecordPartMsg(hp->conn_id_);
    }

    return 0;
}

int Parser::OnURL(http_parser* parser, const char* at, size_t length)
{
    LOG_TRACE("Parser::OnURL");

    Parser* hp = static_cast<Parser*>(parser->data);
    hp->http_req_.ParseURL(at, length);

    if (hp->http_ws_tcp_common_logic_ != nullptr)
    {
        hp->http_ws_tcp_common_logic_->RecordPartMsg(hp->conn_id_);
    }

    return 0;
}

int Parser::OnHeaderField(http_parser* parser, const char* at, size_t length)
{
    LOG_TRACE("Parser::OnHeaderField");

    Parser* hp = static_cast<Parser*>(parser->data);
    hp->last_header_name_.assign(at, length);

    if (hp->http_ws_tcp_common_logic_ != nullptr)
    {
        hp->http_ws_tcp_common_logic_->RecordPartMsg(hp->conn_id_);
    }

    return 0;
}

int Parser::OnHeaderValue(http_parser* parser, const char* at, size_t length)
{
    LOG_TRACE("Parser::OnHeaderValue");

    Parser* hp = static_cast<Parser*>(parser->data);
    hp->http_req_.AddHeader(hp->last_header_name_, std::string(at, length));

    if (hp->http_ws_tcp_common_logic_ != nullptr)
    {
        hp->http_ws_tcp_common_logic_->RecordPartMsg(hp->conn_id_);
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

    hp->http_req_.Method = (http_method) parser->method;
    hp->http_req_.MajorVersion = parser->http_major;
    hp->http_req_.MinorVersion = parser->http_minor;
    hp->http_req_.ParseClientIP();

    LOG_DEBUG("method: " << hp->http_req_.Method
              << ", major version: " << hp->http_req_.MajorVersion
              << ", minor version: " << hp->http_req_.MinorVersion
              << ", client ip: " << hp->http_req_.ClientIP);

    HeaderMap::const_iterator it = hp->http_req_.Headers.find("Content-Type");
    if (it != hp->http_req_.Headers.end())
    {
        if (0 == strncasecmp(it->second.c_str(), "multipart/form-data", strlen("multipart/form-data")))
        {
            hp->http_req_._s.body_processor = mpart_body_processor_init(&(hp->http_req_));
            hp->http_req_._s.free_body_parser_func = (free_body_parser) mpart_body_processor_free;

            const_cast<struct http_parser_settings*>(HTTPParserSettings->Get())->on_body = Parser::mpart_body_process;
        }
    }

    if (hp->http_ws_tcp_common_logic_ != nullptr)
    {
        hp->http_ws_tcp_common_logic_->RecordPartMsg(hp->conn_id_);
    }

    return 0;
}

int Parser::OnBody(http_parser* parser, const char* at, size_t length)
{
    // 注意：body回调可能不止调用一次
    LOG_TRACE("Parser::OnBody");

    Parser* hp = static_cast<Parser*>(parser->data);
    hp->http_req_.AppendBody(at, length);

    if (hp->http_ws_tcp_common_logic_ != nullptr)
    {
        hp->http_ws_tcp_common_logic_->RecordPartMsg(hp->conn_id_);
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

int Parser::mpart_body_process(http_parser* parser, const char* at, size_t length)
{
    Parser* hp = static_cast<Parser*>(parser->data);
    mpart_body_processor* processor = (mpart_body_processor*) hp->http_req_._s.body_processor;
    hp->http_req_._s.parsed += multipart_parser_execute(processor->parser, at, length);

    return 0;
}
}
}
}
