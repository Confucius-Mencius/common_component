#include "evhttp.h"
#include <stdlib.h>
#include "event2/dns.h"

#define VERIFY(cond) do {                       \
    if (!(cond)) {                              \
        fprintf(stderr, "[error] %s\n", #cond); \
    }                                           \
} while (0);                                    \

#define URL_MAX 4096

struct connect_base
{
    struct evhttp_connection* evcon;
    struct evhttp_uri* location;
};

static void get_cb(struct evhttp_request* req, void* arg)
{
    ev_ssize_t len;
    struct evbuffer* evbuf;

    VERIFY(req);

    evbuf = evhttp_request_get_input_buffer(req);
    len = evbuffer_get_length(evbuf);
    fwrite(evbuffer_pullup(evbuf, len), len, 1, stdout);
    evbuffer_drain(evbuf, len);
}

//The callback is executed when the request completed or an error occurred.
static void event_callback(struct evhttp_request* proxy_req, void* arg)
{
    int code = evhttp_request_get_response_code(proxy_req);
    printf("code: %d\n", code); // code =0: 连不上
    return;

    char buffer[URL_MAX];

    struct connect_base* base = (struct connect_base*) arg;
    struct evhttp_connection* evcon = base->evcon;
    struct evhttp_uri* location = base->location;

    VERIFY(proxy_req);
    if (evcon)
    {
        printf("11111");
//        struct evhttp_request* req = evhttp_request_new(get_cb, NULL);
//        evhttp_add_header(req->output_headers, "Connection", "close");
//        VERIFY(!evhttp_make_request(evcon, req, EVHTTP_REQ_GET,
//                                    evhttp_uri_join(location, buffer, URL_MAX)));
    }
    else
    {
        printf("2222");
    }
}

void close_cb(struct evhttp_connection* evcon, void* arg)
{
    printf("close....");
}

void error_cb(enum evhttp_request_error, void*)
{
    printf("error....");
}

int main(int argc, const char** argv)
{
    char buffer[URL_MAX];

    struct evhttp_uri* host_port;
    struct evhttp_uri* location;
    struct evhttp_uri* proxy;

    struct event_base* base;
    struct evhttp_connection* evcon;
    struct evhttp_request* req;

    struct connect_base connect_base;

//    if (argc != 3)
//    {
//        printf("Usage: %s proxy url\n", argv[0]);
//        return 1;
//    }
//    char argv2[] = "http://127.0.0.7:8000/?data=wanghuan_test,10729091,bac,source_ip,receive_time";
    char argv2[] = "http://127.0.0.1:8000/";
//    char argv2[] = "http://192.168.0.58:80/";
//    {
//        proxy = evhttp_uri_parse(argv[1]);
//        VERIFY(evhttp_uri_get_host(proxy));
//        VERIFY(evhttp_uri_get_port(proxy) > 0);
//    }

    host_port = evhttp_uri_parse(argv2);
    const char* host = evhttp_uri_get_host(host_port);
    int port = evhttp_uri_get_port(host_port);

//        evhttp_uri_set_scheme(host_port, NULL);
//        evhttp_uri_set_userinfo(host_port, NULL);
//        evhttp_uri_set_path(host_port, NULL);
//        evhttp_uri_set_query(host_port, NULL);
//        evhttp_uri_set_fragment(host_port, NULL);
//        VERIFY(evhttp_uri_get_host(host_port));
//        VERIFY(evhttp_uri_get_port(host_port) > 0);

    {
//        location = evhttp_uri_parse(argv2);
//        evhttp_uri_set_scheme(location, NULL);
//        evhttp_uri_set_userinfo(location, 0);
//        evhttp_uri_set_host(location, NULL);
//        evhttp_uri_set_port(location, -1);
    }

    VERIFY(base = event_base_new());
//    struct evdns_base *dns_base = evdns_base_new(base, 1);

    VERIFY(evcon = evhttp_connection_base_new(base, NULL, host, port));
//    evhttp_connection_set_retries(evcon, 1);
    evhttp_connection_set_timeout(evcon, 5);
    evhttp_connection_set_closecb(evcon, close_cb, NULL);
    connect_base = (struct connect_base) {
        .evcon = evcon,
        .location = host_port,
    };
    VERIFY(req = evhttp_request_new(event_callback, &connect_base));
    evhttp_request_set_error_cb(req, error_cb);

    evhttp_add_header(req->output_headers, "Connection", "keep-alive");
//    evhttp_add_header(req->output_headers, "Proxy-Connection", "keep-alive");
//    evutil_snprintf(buffer, URL_MAX, "%s:%d", host, port);
    evhttp_make_request(evcon, req, EVHTTP_REQ_GET, "/?a=b&c=2");

    event_base_dispatch(base);
    evhttp_connection_free(evcon);
    event_base_free(base);
//    evhttp_uri_free(proxy);
    evhttp_uri_free(host_port);
//    evhttp_uri_free(location);
    return 0;
}
