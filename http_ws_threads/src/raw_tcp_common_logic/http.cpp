#include "http.h"

namespace tcp
{
namespace http_ws
{


int on_message_begin(http_parser* parser)
{
    //printf("\n***MESSAGE BEGIN***\n\n");
    return 0;
}

int on_headers_complete(http_parser* parser)
{
    client_t* client = (client_t*) parser->data;

    LOGF("[ %5d ] http message parsed\n", client->request_num);

    return 0;
}

int on_message_complete(http_parser* parser)
{
    //printf("\n***MESSAGE COMPLETE***\n\n");

    client_t* client = (client_t*) parser->data;
    uv_write(
        &client->write_req,
        (uv_stream_t*)&client->handle,
        &resbuf,
        1,
        after_write);

    return 0;
}

int on_url(http_parser* parser, const char* at, size_t length)
{
    client_t* client = (client_t*)parser->data;
    strlncat(client->msg.url,
             1024, at, length);
    //printf("Url: %d,%s\n", (int)length, client->msg.url);

    return 0;
}

int on_status(http_parser* parser, const char* at, size_t length)
{
    client_t* client = (client_t*)parser->data;
    strlncat(client->msg.url,
             1024, at, length);
    //printf("status: %d,%s\n", (int)length, client->msg.url);

    return 0;
}


int on_header_field(http_parser* parser, const char* at, size_t length)
{
    //printf("Header field: %d,%p\n", (int)length, at);
    client_t* client = (client_t*)parser->data;
    if (client->msg.last_header_element != FIELD)
    {
        ++client->msg.header_num;
    }

    strlncat(client->msg.headers[client->msg.header_num - 1].field,
             1024, at, length);
    client->msg.last_header_element = FIELD;
    return 0;
}

int on_header_value(http_parser* parser, const char* at, size_t length)
{
    //printf("Header value: %d,%p\n", (int)length, at);
    client_t* client = (client_t*)parser->data;
    strlncat(client->msg.headers[client->msg.header_num - 1].value,
             1024, at, length);
    client->msg.last_header_element = VALUE;
    return 0;
}

int on_body(http_parser* parser, const char* at, size_t length)
{
    //printf("Body: %d,%p\n", (int)length, at);
    return 0;
}

int on_chunk_header(http_parser* parser)
{
    //printf("\n***chunk_header***\n\n");
    return 0;
}

int on_chunk_complete(http_parser* parser)
{
    //printf("\n***chunk_complete***\n\n");
    return 0;
}

HTTPParser::HttpParser()
{
    parser_settings.on_message_begin = on_message_begin;
    parser_settings.on_url = on_url;
    parser_settings.on_status = on_status;
    parser_settings.on_header_field = on_header_field;
    parser_settings.on_header_value = on_header_value;
    parser_settings.on_headers_complete = on_headers_complete;
    parser_settings.on_body = on_body;
    parser_settings.on_message_complete = on_message_complete;
    parser_settings.on_chunk_header = on_chunk_header;
    parser_settings.on_chunk_complete = on_chunk_complete;
}

}
}
