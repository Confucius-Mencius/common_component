#include <stdlib.h>
#include "evhtp/evhtp.h"

void
testcb(evhtp_request_t* req, void* a)
{
    const char* str = (const char*) a;

    size_t len = evbuffer_get_length(req->buffer_in);
    char* buf = (char*) malloc(len + 1);

    evbuffer_remove(req->buffer_in, buf, len);
    printf("%s\n", buf);

    evbuffer_add_printf(req->buffer_out, "%s", str);
    evhtp_send_reply(req, EVHTP_RES_OK);

    free(buf);
}

int
main(int argc, char** argv)
{
    evbase_t* evbase = event_base_new();
    evhtp_t* htp = evhtp_new(evbase, NULL);

    evhtp_set_cb(htp, "/simple/", testcb, (void*) "simple");
    evhtp_set_cb(htp, "/1/ping", testcb, (void*) "one");
    evhtp_set_cb(htp, "/1/ping.json", testcb, (void*) "two");

#ifndef EVHTP_DISABLE_EVTHR
    evhtp_use_threads(htp, NULL, 4, NULL);
#endif

    evhtp_bind_socket(htp, "0.0.0.0", 8081, 1024);

    event_base_loop(evbase, 0);

    evhtp_unbind_socket(htp);
    evhtp_free(htp);
    event_base_free(evbase);

    return 0;
}

