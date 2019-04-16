#include "the_ws_parser.h"
#include <string.h>
#include <openssl/sha.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include "common_logic.h"
#include "hex_dump.h"
#include "log_util.h"
#include "the_http_parser.h"
#include "singleton.h"

namespace tcp
{
namespace http_ws
{
namespace ws
{
static int Base64Encode(unsigned char* out, const unsigned char* in, int len)
{
    BIO* b64, *bio;
    BUF_MEM* bptr = NULL;
    size_t size = 0;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_write(bio, in, len);
    BIO_flush(bio);

    BIO_get_mem_ptr(bio, &bptr);
    memcpy(out, bptr->data, bptr->length);
    out[bptr->length] = '\0';
    size = bptr->length;

    BIO_free_all(bio);
    return size;
}

class ParserSettings
{
public:
    ParserSettings()
    {
        websocket_parser_settings_init(&settings_);
        settings_.on_frame_header = Parser::OnFrameHeader;
        settings_.on_frame_body = Parser::OnFrameBody;
        settings_.on_frame_end = Parser::OnFrameEnd;
    }

    const struct websocket_parser_settings* Get() const
    {
        return &settings_;
    }

private:
    struct websocket_parser_settings settings_;
};

#define WSParserSettings Singleton<ParserSettings>::Instance()

Parser::Parser() : key_(), protocol_(), body_(), payloads_()
{
    http_ws_raw_tcp_common_logic_ = nullptr;
    conn_id_ = INVALID_CONN_ID;

    websocket_parser_init(&parser_);
    parser_.data = this;

    opcode_ = 0;
    is_text_ = false;
    is_binary_ = false;
    fin_ = 0;
}

Parser::~Parser()
{
}

int Parser::CheckUpgrade(const http::Req& http_req)
{
    http::HeaderMap::const_iterator it = http_req.Headers.find("Upgrade");
    if (it == http_req.Headers.cend() || (it->second != "websocket" && it->second != "WebSocket" && it->second != "Websocket"))
    {
        LOG_ERROR("should have header: { Upgrade: websocket }");
        return -1;
    }

    it = http_req.Headers.find("Connection");
    if (it == http_req.Headers.cend() || (it->second != "Upgrade" && it->second != "upgrade"))
    {
        LOG_ERROR("should have header: { Connection: Upgrade }");
        return -1;
    }

    it = http_req.Headers.find("Sec-WebSocket-Version");
    if (it == http_req.Headers.cend() || it->second != "13") // 13表示RFC6455
    {
        LOG_ERROR("should have header: { Sec-Websocket-Version: 13 }");
        return -1;
    }

    it = http_req.Headers.find("Sec-WebSocket-Key");
    if (it == http_req.Headers.cend())
    {
        LOG_ERROR("no Sec-Websocket-Key header");
        return -1;
    }

    this->key_ = it->second;

    it = http_req.Headers.find("Sec-WebSocket-Protocol");
    if (it != http_req.Headers.cend())
    {
        LOG_DEBUG("Sec-WebSocket-Protocol: " << it->second);
        this->protocol_ = it->second;
    }

    return 0;
}

std::string Parser::MakeHandshake()
{
    std::string handshake;
    handshake.reserve(128);

    handshake += "HTTP/1.1 101 Switching Protocols\r\n";
    handshake += "Upgrade: WebSocket\r\n";
    handshake += "Connection: Upgrade\r\n";

    std::string accept_key;
    accept_key.reserve(128);

    accept_key += this->key_;
    accept_key += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"; // RFC6544_MAGIC_KEY

    unsigned char digest[20] = ""; // 160 bit sha1 digest
    SHA1((const unsigned char*) accept_key.data(), (accept_key.size()), (unsigned char*) digest);

    unsigned char out[128] = "";
    int size = Base64Encode(out, digest, 20);

    handshake += "Sec-WebSocket-Accept: ";
    handshake.append((const char*) out, size - 1); // 去掉结尾的\n
    handshake += "\r\n";

    if (this->protocol_.length() > 0)
    {
        handshake += "Sec-WebSocket-Protocol: " + (this->protocol_) + "\r\n"; // 如果客户端请求的protocol有多个，服务器只回复一个？
    }

    time_t now;
    struct tm* tm_now;
    char time_str[256] = "";

    time(&now);
    tm_now = localtime(&now);
    strftime(time_str, sizeof(time_str), "Date: %a, %d %b %Y %T %Z", tm_now);

    handshake.append(time_str);
    handshake += "\r\n\r\n";

    return handshake;
}

int Parser::Execute(const char* buffer, size_t count)
{
    LOG_TRACE("websocket parser execute");

    size_t n = websocket_parser_execute(&parser_, WSParserSettings->Get(), buffer, count);
    if (n != count)
    {
        LOG_ERROR("failed to parse " << std::string(buffer, count));
        return -1;
    }

    return 0;
}

int Parser::OnFrameHeader(websocket_parser* parser)
{
    LOG_TRACE("Parser::OnFrameHeader");
    Parser* wsp = static_cast<Parser*>(parser->data);

    wsp->opcode_ = parser->flags & WS_OP_MASK;

    if (WS_OP_TEXT == wsp->opcode_)
    {
        wsp->is_text_ = true;
    }
    else if (WS_OP_BINARY == wsp->opcode_)
    {
        wsp->is_binary_ = true;
    }

    wsp->fin_ = parser->flags & WS_FIN;
    wsp->body_.resize(parser->length);

    LOG_DEBUG("opcode: " << wsp->opcode_ << ", fin: " << (wsp->fin_ != 0) << ", length: " << parser->length);

    if (wsp->http_ws_raw_tcp_common_logic_ != nullptr)
    {
        wsp->http_ws_raw_tcp_common_logic_->RecordPartMsg(wsp->conn_id_);
    }

    return 0;
}

int Parser::OnFrameBody(websocket_parser* parser, const char* at, size_t length)
{
    LOG_TRACE("Parser::OnFrameBody, length: " << length);
    Parser* wsp = static_cast<Parser*>(parser->data);

    if (parser->flags & WS_HAS_MASK)
    {
        // if frame has mask, we have to copy and decode data via websocket_parser_copy_masked function
        websocket_parser_decode(&wsp->body_[parser->offset], at, length, parser);
    }
    else
    {
        memcpy(&wsp->body_[parser->offset], at, length);
    }

    if (wsp->http_ws_raw_tcp_common_logic_ != nullptr)
    {
        wsp->http_ws_raw_tcp_common_logic_->RecordPartMsg(wsp->conn_id_);
    }

    return 0;
}

int Parser::OnFrameEnd(websocket_parser* parser)
{
    LOG_TRACE("Parser::OnFrameEnd");

    Parser* wsp = static_cast<Parser*>(parser->data);

    if (wsp->is_text_)
    {
        LOG_DEBUG("body: " << wsp->body_ << ", length: " << wsp->body_.size());
    }
    else
    {
        LOG_DEBUG("body length: " << wsp->body_.size());
    }

    if (wsp->body_.size() > 0)
    {
        wsp->payloads_.append(wsp->body_);
    }

    if (wsp->fin_)
    {
        if (wsp->is_text_)
        {
            LOG_DEBUG("payloads: " << wsp->payloads_ << ", length: " << wsp->payloads_.size());
        }
        else
        {
            LOG_DEBUG("payloads length: " << wsp->payloads_.size());
        }

        if (WS_OP_CLOSE == wsp->opcode_)
        {
            char s[2];
            s[0] = wsp->payloads_[1];
            s[1] = wsp->payloads_[0];
            uint16_t cc = *((uint16_t*) s);
            LOG_DEBUG("close code: " << cc);
        }

        if (wsp->http_ws_raw_tcp_common_logic_ != nullptr)
        {
            int opcode = wsp->opcode_;
            if (wsp->is_text_)
            {
                opcode = WS_OP_TEXT;
            }
            else if (wsp->is_binary_)
            {
                opcode = WS_OP_BINARY;
            }

            wsp->http_ws_raw_tcp_common_logic_->OnWSMsg(wsp->conn_id_, opcode, wsp->payloads_.data(), wsp->payloads_.size());
        }

        wsp->payloads_.clear();
        wsp->is_text_ = false;
        wsp->is_binary_ = false;
    }

    return 0;
}
}
}
}
