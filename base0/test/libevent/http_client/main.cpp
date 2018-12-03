/* Base on code from:
 http://archives.seul.org/libevent/users/Sep-2010/msg00050.html
 */

#include <stdlib.h>
#include <string.h>

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/http.h>
#include <event2/http_struct.h>
#include <event2/keyvalq_struct.h>

// (default)
#define HTTP_CONTENT_TYPE_URL_ENCODED   "application/x-www-form-urlencoded"
// (use for files: picture, mp3, tar-file etc.)
#define HTTP_CONTENT_TYPE_FORM_DATA     "multipart/form-data"
// (use for plain text)
#define HTTP_CONTENT_TYPE_TEXT_PLAIN    "text/plain"

#define REQUEST_POST_FLAG               2
#define REQUEST_GET_FLAG                3

struct http_request_get
{
    struct evhttp_uri* uri;
    struct event_base* base;
    struct evhttp_connection* cn;
    struct evhttp_request* req;
};

struct http_request_post
{
    struct evhttp_uri* uri;
    struct event_base* base;
    struct evhttp_connection* cn;
    struct evhttp_request* req;
    char* content_type;
    char* post_data;
};

/************************** Ahead Declare ******************************/
void http_requset_post_cb(struct evhttp_request* req, void* arg);

void http_requset_get_cb(struct evhttp_request* req, void* arg);

int start_url_request(void* http_req, int req_get_flag);

/************************** Tools Function ******************************/
static inline void print_request_head_info(struct evkeyvalq* input_headers)
{
//    struct evkeyval* first_node = input_headers->tqh_first;
//    while (first_node)
//    {
//        printf("%s: %s\n", first_node->key, first_node->value);
//        first_node = first_node->next.tqe_next;
//    }

    for (struct evkeyval* header = input_headers->tqh_first; header != NULL; header = header->next.tqe_next)
    {
        printf("%s: %s\n", header->key, header->value);
    }
}

static void print_uri_parts_info(const struct evhttp_uri* http_uri)
{
    printf("scheme:%s\n", evhttp_uri_get_scheme(http_uri));
    printf("host:%s\n", evhttp_uri_get_host(http_uri));
    printf("path:%s\n", evhttp_uri_get_path(http_uri));
    printf("port:%d\n", evhttp_uri_get_port(http_uri));
    printf("query:%s\n", evhttp_uri_get_query(http_uri));
    printf("userinfo:%s\n", evhttp_uri_get_userinfo(http_uri));
    printf("fragment:%s\n", evhttp_uri_get_fragment(http_uri));
}

/************************** Request Function ******************************/
void http_requset_post_cb(struct evhttp_request* req, void* arg)
{
    struct http_request_post* http_req_post = (struct http_request_post*) arg;
    switch (req->response_code)
    {
        case HTTP_OK:
        {
            struct evbuffer* buf = evhttp_request_get_input_buffer(req);
            size_t len = evbuffer_get_length(buf);
            printf("print the head info:\n");
            print_request_head_info(req->output_headers);

            printf("len:%zu  body size_:%zu\n", len, req->body_size);
            char* tmp = (char*) malloc(len + 1);
            memcpy(tmp, evbuffer_pullup(buf, -1), len);
            tmp[len] = '\0';
            printf("print the body:\n");
            printf("HTML BODY:%s", tmp);
            free(tmp);

            event_base_loopexit(http_req_post->base, 0);
            break;
        }
        case HTTP_MOVEPERM:
            printf("%s", "the uri moved permanently\n");
            break;
        case HTTP_MOVETEMP:
        {
            const char* new_location = evhttp_find_header(req->input_headers,
                                                          "Location");
            struct evhttp_uri* new_uri = evhttp_uri_parse(new_location);
            evhttp_uri_free(http_req_post->uri);
            http_req_post->uri = new_uri;
            start_url_request(http_req_post, REQUEST_POST_FLAG);
            return;
        }

        default:
            event_base_loopexit(http_req_post->base, 0);
            return;
    }
}

void http_requset_get_cb(struct evhttp_request* req, void* arg)
{
    printf("xxxxxxxxxxxx\n");
    struct http_request_get* http_req_get = (struct http_request_get*) arg;
    switch (req->response_code)
    {
        case HTTP_OK:
        {
            struct evbuffer* buf = evhttp_request_get_input_buffer(req);
            size_t len = evbuffer_get_length(buf);
            printf("print the head info:");
            print_request_head_info(req->output_headers);

            printf("len:%zu  body size_:%zu", len, req->body_size);
            char* tmp = (char*) malloc(len + 1);
            memcpy(tmp, evbuffer_pullup(buf, -1), len);
            tmp[len] = '\0';
            printf("print the body:");
            printf("HTML BODY:%s", tmp);
            free(tmp);

            event_base_loopexit(http_req_get->base, 0);
            break;
        }
        case HTTP_MOVEPERM:
            printf("%s", "the uri moved permanently");
            break;
        case HTTP_MOVETEMP:
        {
            const char* new_location = evhttp_find_header(req->input_headers,
                                                          "Location");
            struct evhttp_uri* new_uri = evhttp_uri_parse(new_location);
            evhttp_uri_free(http_req_get->uri);
            http_req_get->uri = new_uri;
            start_url_request(http_req_get, REQUEST_GET_FLAG);
            return;
        }

        default:
            event_base_loopexit(http_req_get->base, 0);
            return;
    }
}

int header_cb(struct evhttp_request* req, void* arg)
{
    printf("header cb, http_req: %p, %d, %s\n", arg, req->response_code, req->response_code_line);
    print_request_head_info(req->input_headers);

    return 0;
}

void chunk_cb(struct evhttp_request* req, void* arg)
{
    struct evbuffer* buf = evhttp_request_get_input_buffer(req);
    size_t len = evbuffer_get_length(buf);

    char* tmp = (char*) malloc(len + 1);
    memcpy(tmp, evbuffer_pullup(buf, -1), len);
    tmp[len] = '\0';

    printf("chunk cb, http_req: %p, input buf len: %zu, body size_:%zu\n", arg, len, req->body_size);

    free(tmp);
}

int start_url_request(void* http_req, int req_get_flag)
{
    /**
     * Request will be released by evhttp connection
     * See info of evhttp_make_request()
     */
    if (req_get_flag == REQUEST_POST_FLAG)
    {
        struct http_request_post* http_req_post =
            (struct http_request_post*) http_req;

        if (http_req_post->cn != NULL)
        {
            evhttp_connection_free(http_req_post->cn);
        }

        int port = evhttp_uri_get_port(http_req_post->uri);
        http_req_post->cn = evhttp_connection_base_new(http_req_post->base,
                                                       NULL, evhttp_uri_get_host(http_req_post->uri),
                                                       (port == -1 ? 80 : port));

        http_req_post->req = evhttp_request_new(http_requset_post_cb,
                                                http_req); // The callback is executed when the request completed or an error occurred.

        const char* path = evhttp_uri_get_path(http_req_post->uri);

        evhttp_make_request(http_req_post->cn, http_req_post->req,
                            EVHTTP_REQ_POST, path ? path : "/"); // Make an HTTP request over the specified connection.

        /** Set the post data */
        evbuffer_add(http_req_post->req->output_buffer,
                     http_req_post->post_data, strlen(http_req_post->post_data));

        evhttp_add_header(http_req_post->req->output_headers, "Content-Type",
                          http_req_post->content_type); // 可以通过evhttp_find_header查找hender中的项

        /** Set the header properties */
        evhttp_add_header(http_req_post->req->output_headers, "Host",
                          evhttp_uri_get_host(http_req_post->uri));
    }
    else if (req_get_flag == REQUEST_GET_FLAG)
    {
        struct http_request_get* http_req_get =
            (struct http_request_get*) http_req;

        if (http_req_get->cn != NULL)
        {
            evhttp_connection_free(http_req_get->cn);
        }

        int port = evhttp_uri_get_port(http_req_get->uri);
        http_req_get->cn = evhttp_connection_base_new(http_req_get->base, NULL,
                                                      evhttp_uri_get_host(http_req_get->uri),
                                                      (port == -1 ? 80 : port));

        http_req_get->req = evhttp_request_new(http_requset_get_cb, http_req);
        printf("http_req: %p\n", http_req);
        evhttp_request_set_header_cb(http_req_get->req, header_cb);
        evhttp_request_set_chunked_cb(http_req_get->req, chunk_cb);

        const char* query = evhttp_uri_get_query(http_req_get->uri);
        const char* path = evhttp_uri_get_path(http_req_get->uri);
        size_t len = (query ? strlen(query) : 0)
                     + (path ? strlen(path) : 0) + 1;
        char* path_query = NULL;
        if (len > 1)
        {
            path_query = (char*) calloc(len, sizeof(char));
            sprintf(path_query, "%s?%s", path, query);
        }

        evhttp_make_request(http_req_get->cn, http_req_get->req, EVHTTP_REQ_GET,
                            path_query ? path_query : "/");

        /** Set the header properties */
        evhttp_add_header(http_req_get->req->output_headers, "Host",
                          evhttp_uri_get_host(http_req_get->uri));
    }

    return 0;
}

/************************** New/Free Function ******************************/
/**
 * @param get_flag: refer REQUEST_GET_*
 *
 */
void* http_request_new(struct event_base* base, const char* url,
                       int req_get_flag, const char* content_type, const char* data)
{
    int len = 0;

    if (req_get_flag == REQUEST_GET_FLAG)
    {
        len = sizeof(struct http_request_get);

        struct http_request_get* http_req_get =
            (struct http_request_get*) calloc(1, len);

        http_req_get->uri = evhttp_uri_parse(url);
        print_uri_parts_info(http_req_get->uri);

        http_req_get->base = base;

        return http_req_get;
    }
    else if (req_get_flag == REQUEST_POST_FLAG)
    {
        len = sizeof(struct http_request_post);

        struct http_request_post* http_req_post =
            (struct http_request_post*) calloc(1, len);

        http_req_post->uri = evhttp_uri_parse(url);
        print_uri_parts_info(http_req_post->uri);

        http_req_post->base = base;

        if (content_type == NULL)
        {
            content_type = HTTP_CONTENT_TYPE_URL_ENCODED;
        }

        http_req_post->content_type = strdup(content_type);

        if (data == NULL)
        {
            http_req_post->post_data = NULL;
        }
        else
        {
            http_req_post->post_data = strdup(data);
        }

        return http_req_post;
    }

    return NULL;
}

void http_request_free(void* http_req, int req_get_flag)
{
    if (req_get_flag == REQUEST_GET_FLAG)
    {
        struct http_request_get* http_req_get =
            (struct http_request_get*) http_req;

        evhttp_connection_free(http_req_get->cn);
        evhttp_uri_free(http_req_get->uri);

        free(http_req_get);
    }
    else if (req_get_flag == REQUEST_POST_FLAG)
    {
        struct http_request_post* http_req_post =
            (struct http_request_post*) http_req;

        evhttp_connection_free(http_req_post->cn);
        evhttp_uri_free(http_req_post->uri);

        if (http_req_post->content_type != NULL)
        {
            free(http_req_post->content_type);
        }

        if (http_req_post->post_data != NULL)
        {
            free(http_req_post->post_data);
        }

        free(http_req_post);
    }
}

/************************** Start POST/GET Function ******************************/
/**
 * @param content_type: refer HTTP_CONTENT_TYPE_*
 */
void* start_http_requset(struct event_base* base, const char* url,
                         int req_get_flag, const char* content_type, const char* data)
{
    void* http_req = http_request_new(base, url, req_get_flag, content_type,
                                      data);
    start_url_request(http_req, req_get_flag);
    return http_req;
}

int main(int argc, char* argv[])
{
    struct event_base* base = event_base_new();

//    struct http_request_post* http_req_post =
//            (struct http_request_post*) start_http_requset(base,
//                    //"http://172.16.239.93:8899/base/truck/delete",
//                                                           "http://192.168.0.58:8000/",
//                                                           REQUEST_POST_FLAG,
//                                                           HTTP_CONTENT_TYPE_URL_ENCODED, "name=winlin&code=1234");

//    struct http_request_get* http_req_get =
//            (struct http_request_get*) start_http_requset(base,
//                                                          "http://192.168.0.58:8000/?winlin,1,2,3",
//                                                          REQUEST_GET_FLAG,
//                                                          NULL, NULL);

    struct http_request_get* http_req_get =
        (struct http_request_get*) start_http_requset(base,
                                                      "https://download.jetbrains.8686c.com/cpp/CLion-2016.1.3.tar.gz",
                                                      REQUEST_GET_FLAG,
                                                      NULL,
                                                      NULL);
    event_base_dispatch(base);

//    http_request_free(http_req_post, REQUEST_POST_FLAG);
    http_request_free(http_req_get, REQUEST_GET_FLAG);

    event_base_free(base);
    return 0;
}
