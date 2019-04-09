#include "http.h"
#include "common_logic.h"
#include "log_util.h"

namespace tcp
{
namespace http
{
HTTPReq::HTTPReq() : client_ip(), url(), path_(), query_params_(), headers_(),
    body(), schema_(), host_(), query_()
{
    method = HTTP_GET;
    major_version_ = 1;
    minor_version_ = 1;
    port_ = 0;
}

HTTPReq::~HTTPReq()
{
}

void HTTPReq::ParseURL()
{
    if (!schema_.empty())
    {
        return;
    }

    struct http_parser_url u;
    http_parser_url_init(&u);

    if (http_parser_parse_url(url.c_str(), url.length(), 0, &u) != 0)
    {
        LOG_ERROR("parse url error: " << url.c_str());
        return;
    }

    if (u.field_set & (1 << UF_SCHEMA))
    {
        schema_ = url.substr(u.field_data[UF_SCHEMA].off, u.field_data[UF_SCHEMA].len);
    }

    if (u.field_set & (1 << UF_HOST))
    {
        host_ = url.substr(u.field_data[UF_HOST].off, u.field_data[UF_HOST].len);
    }

    if (u.field_set & (1 << UF_PORT))
    {
        port_ = u.port;
    }
    else
    {
        if (strcasecmp(schema_.c_str(), "https") == 0 || strcasecmp(schema_.c_str(), "wss") == 0)
        {
            port_ = 443;
        }
        else
        {
            port_ = 80;
        }
    }

    if (u.field_set & (1 << UF_PATH))
    {
        path_ = url.substr(u.field_data[UF_PATH].off, u.field_data[UF_PATH].len);
    }

    if (u.field_set & (1 << UF_QUERY))
    {
        query_ = url.substr(u.field_data[UF_QUERY].off, u.field_data[UF_QUERY].len);
        ParseQuery();
    }
}

void HTTPReq::ParseClientIP()
{
    Headers::const_iterator it =  headers_.find("X-Forwarded-For");
    if (it != headers_.end())
    {
        const char* x_forwarded_for = it->second.c_str();
        LOG_DEBUG("X-Forwarded-For: " << x_forwarded_for);

        const char* p = strchr(x_forwarded_for, ',');
        if (nullptr == p)
        {
            client_ip.assign(x_forwarded_for);
        }
        else
        {
            client_ip.assign(x_forwarded_for, p - x_forwarded_for);
        }
    }
}

void HTTPReq::ParseQuery()
{
    if (query_.empty() || !query_params_.empty())
    {
        return;
    }

    char* str1, *str2, *token, *subtoken;
    char* saveptr1, *saveptr2;
    int j;

    std::string q = query_;

    for (j = 1, str1 = (char*) q.c_str(); ; ++j, str1 = NULL)
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

        LOG_DEBUG("key: " << key << ", value: " << value);
        query_params_.insert(make_pair(key, value));
    }
}

std::string HTTPReq::Dump()
{
//    static const char HOST_NAME[] = "Host";
//    static const char CONTENT_LENGTH_NAME[] = "Content-Length";

    char buf[1024] = "";
    int n = 0;

    if (path_.empty())
    {
        path_ = "/";
    }

    if (query_.empty())
    {
        n = snprintf(buf, sizeof(buf), "%s %s HTTP/%d.%d\r\n",
                     http_method_str(method), path_.c_str(), major_version_, minor_version_);
    }
    else
    {
        n = snprintf(buf, sizeof(buf), "%s %s?%s HTTP/%d.%d\r\n",
                     http_method_str(method), path_.c_str(), query_.c_str(), major_version_, minor_version_);
    }

    std::string str;
    str.append(buf, n);

//    headers_.erase(HOST_NAME);

//    if (port_ == 80 || port_ == 443)
//    {
//        headers_.insert(make_pair(HOST_NAME, host_));
//    }
//    else
//    {
//        n = snprintf(buf, sizeof(buf), "%s:%d", host_.c_str(), port_);
//        headers_.insert(make_pair(HOST_NAME, std::string(buf, n)));
//    }

//    if (HTTP_POST == method)
//    {
//        headers_.erase(CONTENT_LENGTH_NAME);

//        n = snprintf(buf, sizeof(buf), "%d", int(body.size()));
//        headers_.insert(make_pair(CONTENT_LENGTH_NAME, std::string(buf, n)));
//    }

    auto iter = headers_.cbegin();
    while (iter != headers_.cend())
    {
        int n = snprintf(buf, sizeof(buf), "%s: %s\r\n", iter->first.c_str(), iter->second.c_str());
        str.append(buf, n);
        ++iter;
    }

    str.append("\r\n");
    str.append(body);

    return str;
}

HTTPParser::HTTPParser() : http_request_(), last_header_name_()
{
    settings_.on_message_begin = HTTPParser::OnMessageBegin;
    settings_.on_url = HTTPParser::OnURL;
    settings_.on_status = nullptr;
    settings_.on_header_field = HTTPParser::OnHeaderField;
    settings_.on_header_value = HTTPParser::OnHeaderValue;
    settings_.on_headers_complete = HTTPParser::OnHeadersComplete;
    settings_.on_body = HTTPParser::OnBody;
    settings_.on_message_complete = HTTPParser::OnMessageComplete;
    settings_.on_chunk_header = nullptr;
    settings_.on_chunk_complete = nullptr;

    http_parser_init(&parser_, HTTP_REQUEST);
    parser_.data = this;

    http_ws_raw_tcp_common_logic_ = nullptr;
    conn_id_ = INVALID_CONN_ID;
}

HTTPParser::~HTTPParser()
{
}

int HTTPParser::Execute(const char* buffer, size_t count)
{
    size_t n = http_parser_execute(&parser_, &settings_, buffer, count);
    if (parser_.upgrade)
    {
        LOG_TRACE("upgrade");
//        onUpgrade(buffer + n, count - n);
        return 0;
    }
    else if (n != count)
    {
        LOG_ERROR("parser error: " << std::string(buffer, count));
        return parser_.http_errno;
    }

    return 0;
}

int HTTPParser::OnMessageBegin(http_parser* parser)
{
    LOG_TRACE("HTTPParser::OnMessageBegin");
    return 0;
}

int HTTPParser::OnURL(http_parser* parser, const char* at, size_t length)
{
    LOG_TRACE("HTTPParser::OnURL");

    HTTPParser* hp = static_cast<HTTPParser*>(parser->data);
    hp->http_request_.url.assign(at, length);
    hp->http_request_.ParseURL();

    return 0;
}

int HTTPParser::OnHeaderField(http_parser* parser, const char* at, size_t length)
{
    LOG_TRACE("HTTPParser::OnHeaderField");

    HTTPParser* hp = static_cast<HTTPParser*>(parser->data);
    hp->last_header_name_.assign(at, length);

    return 0;
}

int HTTPParser::OnHeaderValue(http_parser* parser, const char* at, size_t length)
{
    LOG_TRACE("HTTPParser::OnHeaderValue");

    HTTPParser* hp = static_cast<HTTPParser*>(parser->data);

    if (hp->last_header_name_.empty())
    {
        LOG_ERROR("error");
        return -1;
    }

    hp->http_request_.AddHeader(hp->last_header_name_, std::string(at, length));
    hp->last_header_name_ = "";

    return 0;
}

int HTTPParser::OnHeadersComplete(http_parser* parser)
{
    LOG_TRACE("HTTPParser::OnHeadersComplete");

    HTTPParser* hp = static_cast<HTTPParser*>(parser->data);
    hp->http_request_.ParseClientIP();
    hp->last_header_name_ = "";

    return 0;
}

int HTTPParser::OnBody(http_parser* parser, const char* at, size_t length)
{
    LOG_TRACE("HTTPParser::OnBody");

    HTTPParser* hp = static_cast<HTTPParser*>(parser->data);
    hp->http_request_.method = HTTP_POST;
    hp->http_request_.body.assign(at, length);

    return 0;
}

int HTTPParser::OnMessageComplete(http_parser* parser)
{
    LOG_TRACE("HTTPParser::OnMessageComplete");

    HTTPParser* hp = static_cast<HTTPParser*>(parser->data);
    LOG_DEBUG(hp->http_request_.Dump());

    if (hp->http_ws_raw_tcp_common_logic_ != nullptr)
    {
        hp->http_ws_raw_tcp_common_logic_->OnHTTPReq(hp->conn_id_, hp->http_request_);
    }

    return 0;
}
}
}
