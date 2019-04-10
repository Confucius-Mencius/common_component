#include "ws_parser.h"
#include <string.h>
#include <openssl/sha.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include "http.h"
#include "log_util.h"

namespace tcp
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

WSParser::WSParser() : key(), protocol(), payloads()
{
    is_text_frame = false;
}

WSParser::~WSParser()
{
}

int WSParser::CheckUpgrade(const http::HTTPReq& http_req)
{
    http::Headers::const_iterator it = http_req.headers_.find("Upgrade");
    if (it == http_req.headers_.end() || it->second != "websocket")
    {
        LOG_ERROR("should have header: { Upgrade: websocket }");
        return -1;
    }

    it = http_req.headers_.find("Connection");
    if (it == http_req.headers_.end() || it->second != "upgrade")
    {
        LOG_ERROR("should have header: { Connection: upgrade }");
        return -1;
    }

    it = http_req.headers_.find("Sec-Websocket-Version");
    if (it == http_req.headers_.end() || it->second != "13") // 13表示RFC6455
    {
        LOG_ERROR("should have header: { Sec-Websocket-Version: 13 }");
        return -1;
    }

    it = http_req.headers_.find("Sec-Websocket-Key");
    if (it == http_req.headers_.end())
    {
        LOG_ERROR("no Sec-Websocket-Key header");
        return -1;
    }

    this->key = it->second;

    it = http_req.headers_.find("Sec-Websocket-Protocol");
    if (it != http_req.headers_.end())
    {
        LOG_DEBUG("Sec-Websocket-Protocol: " << it->second);
        this->protocol = it->second;
    }

    return 0;
}

std::string WSParser::MakeHandshake()
{
    std::string answer;
    answer.reserve(128);

    answer += "HTTP/1.1 101 Switching Protocols\r\n";
    answer += "Upgrade: WebSocket\r\n";
    answer += "Connection: Upgrade\r\n";

    std::string accept_key;
    accept_key.reserve(128);

    accept_key += this->key;
    accept_key += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"; // RFC6544_MAGIC_KEY

    unsigned char digest[20] = ""; // 160 bit sha1 digest
    SHA1((const unsigned char*) accept_key.data(), (accept_key.size()), (unsigned char*) digest);

    // little endian to big endian
    for (int i = 0; i < 20; i += 4)
    {
        unsigned char c;

        c = digest[i];
        digest[i] = digest[i + 3];
        digest[i + 3] = c;

        c = digest[i + 1];
        digest[i + 1] = digest[i + 2];
        digest[i + 2] = c;
    }

    unsigned char out[128] = "";
    Base64Encode(out, digest, 20);

    answer += "Sec-WebSocket-Accept: ";
    answer += (char*) out;
    answer += "\r\n";

    if (this->protocol.length() > 0)
    {
        answer += "Sec-WebSocket-Protocol: " + (this->protocol) + "\r\n"; // 如果客户端请求的protocol有多个，服务器只回复一个？
    }

    answer += "\r\n";
    return answer;
}

FrameType WSParser::ParseFrame(size_t& offset, const char* data, size_t len)
{
    if (len < 3)
    {
        return INCOMPLETE;
    }

    unsigned char msg_opcode = data[0] & 0x0F; // 表示被传输帧的类型：x0 表示一个后续帧；x1 表示一个文本帧；x2 表示一个二进制帧；x3-7 为以后的非控制帧保留；x8 表示一个连接关闭；x9 表示一个ping；xA 表示一个pong；xB-F 为以后的控制帧保留。
    unsigned char msg_fin = (data[0] >> 7) & 0x01; // 表示此帧是否是消息的最后帧，第一帧也可能是最后帧。
    unsigned char msg_masked = (data[1] >> 7) & 0x01; // 表示净荷是否有掩码（只适用于客户端发送给服务器的消息）。【从客户端进行发送的帧必须置此位为1，从服务器发送的帧必须置为0。如果任何一方收到的帧不符合此要求，则发送关闭帧(Close frame)关闭连接】

    uint64_t payload_length = 0; // 净荷长度由可变长度字段表示： 如果是 0~125，就是净荷长度；如果是 126，则接下来 2 字节表示的 16 位无符号整数才是这一帧的长度； 如果是 127，则接下来 8 字节表示的 64 位无符号整数才是这一帧的长度
    int pos = 2;
    int length_field = data[1] & (~0x80);
    unsigned int mask = 0;

    if (length_field <= 125)
    {
        payload_length = length_field;
    }
    else if (length_field == 126)  // msglen is 16bit!
    {
        payload_length = (data[2] << 8) | data[3];
        pos += 2;
    }
    else if (length_field == 127)  // msglen is 64bit!
    {
        payload_length = (uint64_t(data[2]) << 56)
                         | (uint64_t(data[3]) << 48)
                         | (uint64_t(data[4]) << 40)
                         | (uint64_t(data[5]) << 32)
                         | (uint64_t(data[6]) << 24)
                         | (uint64_t(data[7]) << 16)
                         | (uint64_t(data[8]) << 8)
                         | uint64_t(data[9]);
        pos += 8;
    }

    if (len < payload_length + pos)
    {
        return INCOMPLETE;
    }

    if (msg_masked)
    {
        mask = *((unsigned int*)(data + pos)); // 用于给净荷加掩护，客户端到服务器标记。
        pos += 4;

        // unmask data
        char* c = (char*) (data + pos);
        for (uint64_t i = 0; i < payload_length; ++i)
        {
            c[i] = c[i] ^ ((char*)(&mask))[i % 4];
        }
    }

    if (payload_length > 0)
    {
        this->payloads.append(data + pos, payload_length); // CLOSE PING PONG是否有payload？
    }

    offset = pos + payload_length;

    if (msg_fin)
    {
        if (msg_opcode == 0x0)
        {
            return (this->is_text_frame ? TEXT_FRAME : BINARY_FRAME);
        }
        if (msg_opcode == 0x8)
        {
            return CLOSE_FRAME;
        }
        if (msg_opcode == 0x9)
        {
            return PING_FRAME;
        }
        if (msg_opcode == 0xA)
        {
            return PONG_FRAME;
        }
    }
    else
    {
        if (msg_opcode == 0x1)
        {
            this->is_text_frame = true;
        }
        if (msg_opcode == 0x2)
        {
            this->is_text_frame = false;
        }

        return INCOMPLETE;
    }

    return ERROR;
}
}
}
