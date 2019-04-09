#include "websocket.h"
#include <string.h>
#include <openssl/sha.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

namespace tcp
{
namespace ws
{
static int base64_encode(unsigned char* in, int len, unsigned char* out)
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

WebSocket::WebSocket()
{

}

WebSocketFrameType WebSocket::parseHandshake(unsigned char* input_frame, int input_len)
{
    // 1. copy char*/len into std::string
    // 2. try to parse headers until \r\n occurs
    std::string headers((char*)input_frame, input_len);
    std::string::size_type header_end = headers.find("\r\n\r\n");

    if (header_end == std::string::npos)  // end-of-headers not found - do not parse
    {
        return INCOMPLETE_FRAME;
    }

    headers.resize(header_end); // trim off any data we don't need after the headers
    std::vector<std::string> headers_rows = explode(headers, std::string("\r\n"));
    for (int i = 0; i < (int) headers_rows.size(); i++)
    {
        std::string& header = headers_rows[i];
        if (header.find("GET") == 0)
        {
            std::vector<std::string> get_tokens = explode(header, std::string(" "));
            if (get_tokens.size() >= 2)
            {
                this->resource = get_tokens[1];
            }
        }
        else
        {
            std::string::size_type pos = header.find(":");
            if (pos != std::string::npos)
            {
                std::string header_key(header, 0, pos);
                std::string header_value(header, pos + 1);
                header_value = trim(header_value);
                if (header_key == "Host")
                {
                    this->host = header_value;
                }
                else if (header_key == "Origin")
                {
                    this->origin = header_value;
                }
                else if (header_key == "Sec-WebSocket-Key")
                {
                    this->key = header_value;
                }
                else if (header_key == "Sec-WebSocket-Protocol")
                {
                    this->protocol = header_value;
                }
            }
        }
    }

    //this->key = "dGhlIHNhbXBsZSBub25jZQ==";
    //printf("PARSED_KEY:%s \n", this->key.data());

    //return FrameType::OPENING_FRAME;
    printf("HANDSHAKE-PARSED\n");
    return OPENING_FRAME;
}

std::string WebSocket::trim(std::string str)
{
    //printf("TRIM\n");
    char whitespace[] = " \t\r\n";
    std::string::size_type pos = str.find_last_not_of(whitespace);
    if (pos != std::string::npos)
    {
        str.erase(pos + 1);
        pos = str.find_first_not_of(whitespace);
        if (pos != std::string::npos)
        {
            str.erase(0, pos);
        }
    }
    else
    {
        return std::string();
    }
    return str;
}

std::vector<std::string> WebSocket::explode(
    std::string  theString,
    std::string  theDelimiter,
    bool    theIncludeEmptyStrings)
{
    //printf("EXPLODE\n");
    //UASSERT( theDelimiter.size(), >, 0 );

    std::vector<std::string> theStringVector;
    std::string::size_type  start = 0, end = 0, length = 0;

    while ( end != std::string::npos )
    {
        end = theString.find( theDelimiter, start );

        // If at end, use length=maxLength.  Else use length=end-start.
        length = (end == std::string::npos) ? std::string::npos : end - start;

        if (theIncludeEmptyStrings
                || (   ( length > 0 ) /* At end, end == length == std::string::npos */
                       && ( start  < theString.size() ) ) )
        {
            theStringVector.push_back( theString.substr( start, length ) );
        }

        // If at end, use start=maxSize.  Else use start=end+delimiter.
        start = (   ( end > (std::string::npos - theDelimiter.size()) )
                    ?  std::string::npos  :  end + theDelimiter.size()     );
    }
    return theStringVector;
}

std::string WebSocket::answerHandshake()
{
    unsigned char digest[20]; // 160 bit sha1 digest

    std::string answer;
    answer += "HTTP/1.1 101 Switching Protocols\r\n";
    answer += "Upgrade: WebSocket\r\n";
    answer += "Connection: Upgrade\r\n";
    if (this->key.length() > 0)
    {
        std::string accept_key;
        accept_key += this->key;
        accept_key += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"; //RFC6544_MAGIC_KEY

        //printf("INTERMEDIATE_KEY:(%s)\n", accept_key.data());


        SHA1((unsigned char*)accept_key.data(), (accept_key.size()), (unsigned char*) digest);


        //printf("DIGEST:"); for(int i=0; i<20; i++) printf("%02x ",digest[i]); printf("\n");

        //little endian to big endian TODO
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

        //printf("DIGEST:"); for(int i=0; i<20; i++) printf("%02x ",digest[i]); printf("\n");

        unsigned char out[128] = "";
        base64_encode(digest, 20, out);  //160bit = 20 bytes/chars

        answer += "Sec-WebSocket-Accept: ";
        answer += (char*) out;
        answer += "\r\n";
    }
    if (this->protocol.length() > 0)
    {
        answer += "Sec-WebSocket-Protocol: " + (this->protocol) + "\r\n";
    }
    answer += "\r\n";
    return answer;

    //return WS_OPENING_FRAME;
}

int WebSocket::makeFrame(WebSocketFrameType frame_type, unsigned char* msg, int msg_length, unsigned char* buffer, int buffer_size)
{
    int pos = 0;
    int size = msg_length;
    buffer[pos++] = (unsigned char)frame_type; // text frame

    if (size <= 125)
    {
        buffer[pos++] = size;
    }
    else if (size <= 65535)
    {
        buffer[pos++] = 126; //16 bit length follows

        buffer[pos++] = (size >> 8) & 0xFF; // leftmost first
        buffer[pos++] = size & 0xFF;
    }
    else   // >2^16-1 (65535)
    {
        buffer[pos++] = 127; //64 bit length follows

        // write 8 bytes length (significant first)

        // since msg_length is int it can be no longer than 4 bytes = 2^32-1
        // padd zeroes for the first 4 bytes
        for (int i = 3; i >= 0; i--)
        {
            buffer[pos++] = 0;
        }
        // write the actual 32bit msg_length in the next 4 bytes
        for (int i = 3; i >= 0; i--)
        {
            buffer[pos++] = ((size >> 8 * i) & 0xFF);
        }
    }
    memcpy((void*)(buffer + pos), msg, size);
    return (size + pos);
}

WebSocketFrameType WebSocket::getFrame(unsigned char* in_buffer, int in_length, unsigned char* out_buffer, int out_size, int* out_length)
{
    //printf("getTextFrame()\n");
    if (in_length < 3)
    {
        return INCOMPLETE_FRAME;
    }

    unsigned char msg_opcode = in_buffer[0] & 0x0F;
    unsigned char msg_fin = (in_buffer[0] >> 7) & 0x01;
    unsigned char msg_masked = (in_buffer[1] >> 7) & 0x01;

    // *** message decoding

    int payload_length = 0;
    int pos = 2;
    int length_field = in_buffer[1] & (~0x80);
    unsigned int mask = 0;

    //printf("IN:"); for(int i=0; i<20; i++) printf("%02x ",buffer[i]); printf("\n");

    if (length_field <= 125)
    {
        payload_length = length_field;
    }
    else if (length_field == 126)  //msglen is 16bit!
    {
        //payload_length = in_buffer[2] + (in_buffer[3]<<8);
        payload_length = (
                             (in_buffer[2] << 8) |
                             (in_buffer[3])
                         );
        pos += 2;
    }
    else if (length_field == 127)  //msglen is 64bit!
    {
        payload_length = (
                             (in_buffer[2] << 56) |
                             (in_buffer[3] << 48) |
                             (in_buffer[4] << 40) |
                             (in_buffer[5] << 32) |
                             (in_buffer[6] << 24) |
                             (in_buffer[7] << 16) |
                             (in_buffer[8] << 8) |
                             (in_buffer[9])
                         );
        pos += 8;
    }

    //printf("PAYLOAD_LEN: %08x\n", payload_length);
    if (in_length < payload_length + pos)
    {
        return INCOMPLETE_FRAME;
    }

    if (msg_masked)
    {
        mask = *((unsigned int*)(in_buffer + pos));
        //printf("MASK: %08x\n", mask);
        pos += 4;

        // unmask data:
        unsigned char* c = in_buffer + pos;
        for (int i = 0; i < payload_length; i++)
        {
            c[i] = c[i] ^ ((unsigned char*)(&mask))[i % 4];
        }
    }

    if (payload_length > out_size)
    {
        //TODO: if output buffer is too small -- ERROR or resize(free and allocate bigger one) the buffer ?
    }

    memcpy((void*)out_buffer, (void*)(in_buffer + pos), payload_length);
    out_buffer[payload_length] = 0;
    *out_length = payload_length + 1;

    //printf("TEXT: %s\n", out_buffer);

    if (msg_opcode == 0x0)
    {
        return (msg_fin) ? TEXT_FRAME : INCOMPLETE_TEXT_FRAME;    // continuation frame ?
    }
    if (msg_opcode == 0x1)
    {
        return (msg_fin) ? TEXT_FRAME : INCOMPLETE_TEXT_FRAME;
    }
    if (msg_opcode == 0x2)
    {
        return (msg_fin) ? BINARY_FRAME : INCOMPLETE_BINARY_FRAME;
    }
    if (msg_opcode == 0x9)
    {
        return PING_FRAME;
    }
    if (msg_opcode == 0xA)
    {
        return PONG_FRAME;
    }

    return ERROR_FRAME;
}
}
}
