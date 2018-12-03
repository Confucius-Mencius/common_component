/*
 * libwebsockets-test-client - libwebsockets test implementation
 *
 * Copyright (C) 2011-2016 Andy Green <andy@warmcat.com>
 *
 * This file is made available under the Creative Commons CC0 1.0
 * Universal Public Domain Dedication.
 *
 * The person who associated a work with this deed has dedicated
 * the work to the public domain by waiving all of his or her rights
 * to the work worldwide under copyright law, including all related
 * and neighboring rights, to the extent allowed by law. You can copy,
 * modify, distribute and perform the work, even for commercial purposes,
 * all without asking permission.
 *
 * The test apps are intended to be adapted for use in your code, which
 * may be proprietary.  So unlike the library itself, they are licensed
 * Public Domain.
 */

#include <string>

#include <string.h>
#include <signal.h>

#include <sys/time.h>

#include "libwebsockets.h"
#include "num_util.h"

static int deny_deflate, test_post;
static struct lws* wsi_dumb;
struct lws_context* context;
struct lws_client_connect_info i;

#if defined(LWS_OPENSSL_SUPPORT) && defined(LWS_HAVE_SSL_CTX_set1_param)
char crl_path[1024] = "";
#endif

/*
 * This demo shows how to connect multiple websockets simultaneously to a
 * websocket server (there is no restriction on their having to be the same
 * server just it simplifies the demo).
 *
 *  dumb-increment-protocol:  we connect to the server and print the number
 *				we are given
 *
 *  lws-mirror-protocol: draws random circles, which are mirrored on to every
 *				client (see them being drawn in every browser
 *				session also using the test server)
 */

static void show_http_content(const char* p, size_t l)
{
//    std::string x(p, l);
//    printf("------------%s\n", x.c_str());

//    if (lwsl_visible(LLL_INFO)) {
    while (l--)
    {
        if (*p < 0x7f)
        {
            putchar(*p++);
        }
        else
        {
            putchar('.');
        }
    }
//    }

    putchar('\n');
}


/*
 * dumb_increment protocol
 *
 * since this also happens to be protocols[0], some callbacks that are not
 * bound to a specific protocol also turn up here.
 */

static int
callback_dumb_increment(struct lws* wsi, enum lws_callback_reasons reason,
                        void* user, void* in, size_t len)
{
    const char* which = "http";
    char which_wsi[10], buf[50 + LWS_PRE];
    int n;

    switch (reason)
    {

        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            lwsl_info("dumb: LWS_CALLBACK_CLIENT_ESTABLISHED\n");
            break;

        case LWS_CALLBACK_CLOSED:
            lwsl_notice("dumb: LWS_CALLBACK_CLOSED\n");
            wsi_dumb = NULL;
            break;

        case LWS_CALLBACK_CLIENT_RECEIVE:
            ((char*) in)[len] = '\0';
            lwsl_info("rx %d '%s'\n", (int) len, (char*) in);
            break;

            /* because we are protocols[0] ... */

        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            if (wsi == wsi_dumb)
            {
                which = "dumb";
                wsi_dumb = NULL;
            }

            lwsl_err("CLIENT_CONNECTION_ERROR: %s: %s\n", which,
                     in ? (char*) in : "(null)");
            break;

        case LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED:
            if ((strcmp((const char*) in, "deflate-stream") == 0) && deny_deflate)
            {
                lwsl_notice("denied deflate-stream extension\n");
                return 1;
            }
            if ((strcmp((const char*) in, "x-webkit-deflate-frame") == 0))
            {
                return 1;
            }
            if ((strcmp((const char*) in, "deflate-frame") == 0))
            {
                return 1;
            }
            break;

        case LWS_CALLBACK_ESTABLISHED_CLIENT_HTTP:
            lwsl_notice("lws_http_client_http_response %d\n",
                        lws_http_client_http_response(wsi));
            break;

            /* chunked content */
        case LWS_CALLBACK_RECEIVE_CLIENT_HTTP_READ:
            lwsl_notice("LWS_CALLBACK_RECEIVE_CLIENT_HTTP_READ: %ld\n",
                        (long) len);
            show_http_content((const char*) in, len);
            break;

            /* unchunked content */
        case LWS_CALLBACK_RECEIVE_CLIENT_HTTP:
        {
            char buffer[1024 + LWS_PRE];
            char* px = buffer + LWS_PRE;
            int lenx = sizeof(buffer) - LWS_PRE;

            /*
             * Often you need to flow control this by something
             * else being writable.  In that case call the api
             * to get a callback when writable here, and do the
             * pending client read in the writeable callback of
             * the output.
             *
             * In the case of chunked content, this will call back
             * LWS_CALLBACK_RECEIVE_CLIENT_HTTP_READ once per
             * chunk or partial chunk in the buffer, and report
             * zero length back here.
             */
            if (lws_http_client_read(wsi, &px, &lenx) < 0)
            {
                printf("---\n");
                return -1;
            }

            printf("----------------%s\n", buffer);
        }
            break;

        case LWS_CALLBACK_CLIENT_WRITEABLE:
            lwsl_info("Client wsi %p writable\n", wsi);
            break;

        case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER:
            if (test_post)
            {
                unsigned char** p = (unsigned char**) in, * end = (*p) + len;

                if (lws_add_http_header_by_token(wsi,
                                                 WSI_TOKEN_HTTP_CONTENT_LENGTH,
                                                 (unsigned char*) "29", 2, p, end))
                {
                    return -1;
                }
                if (lws_add_http_header_by_token(wsi,
                                                 WSI_TOKEN_HTTP_CONTENT_TYPE,
                                                 (unsigned char*) "application/x-www-form-urlencoded", 33, p, end))
                {
                    return -1;
                }

                /* inform lws we have http body to send */
                lws_client_http_body_pending(wsi, 1);
                lws_callback_on_writable(wsi);
            }
            break;

        case LWS_CALLBACK_CLIENT_HTTP_WRITEABLE:
            strcpy(buf + LWS_PRE, "text=hello&send=Send+the+form");
            n = lws_write(wsi, (unsigned char*) &buf[LWS_PRE], strlen(&buf[LWS_PRE]), LWS_WRITE_HTTP);
            if (n < 0)
            {
                return -1;
            }
            /* we only had one thing to send, so inform lws we are done
             * if we had more to send, call lws_callback_on_writable(wsi);
             * and just return 0 from callback.  On having sent the last
             * part, call the below api instead.*/
            lws_client_http_body_pending(wsi, 0);
            break;

        case LWS_CALLBACK_COMPLETED_CLIENT_HTTP:
            if (0 == lws_http_transaction_completed(wsi_dumb))
            {
                printf("------------0");

                lws_client_connect_via_info(&i);

//                {
//                    strcpy(buf + LWS_PRE, "a=1");
//                    n = lws_write(wsi, (unsigned char *)&buf[LWS_PRE], strlen(&buf[LWS_PRE]), LWS_WRITE_HTTP);
//                    if (n < 0)
//                        return -1;
//                    /* we only had one thing to send, so inform lws we are done
//                     * if we had more to send, call lws_callback_on_writable(wsi);
//                     * and just return 0 from callback.  On having sent the last
//                     * part, call the below api instead.*/
//                    lws_client_http_body_pending(wsi, 0);
//                    lws_callback_on_writable(wsi);
//                }

            }
            else
            {
                printf("------------1");
            }

            break;

#if defined(LWS_OPENSSL_SUPPORT) && defined(LWS_HAVE_SSL_CTX_set1_param)
        case LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS:
            if (crl_path[0])
            {
                /* Enable CRL checking of the server certificate */
                X509_VERIFY_PARAM* param = X509_VERIFY_PARAM_new();
                X509_VERIFY_PARAM_set_flags(param, X509_V_FLAG_CRL_CHECK);
                SSL_CTX_set1_param((SSL_CTX*) user, param);
                X509_STORE* store = SSL_CTX_get_cert_store((SSL_CTX*) user);
                X509_LOOKUP* lookup = X509_STORE_add_lookup(store, X509_LOOKUP_file());
                int n = X509_load_cert_crl_file(lookup, crl_path, X509_FILETYPE_PEM);
                X509_VERIFY_PARAM_free(param);
                if (n != 1)
                {
                    char errbuf[256];
                    n = ERR_get_error();
                    lwsl_err("LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS: SSL error: %s (%d)\n",
                             ERR_error_string(n, errbuf), n);
                    return 1;
                }
            }
            break;
#endif

        default:
            break;
    }

    return 0;
}


/* list of supported protocols and callbacks */
enum demo_protocols
{

    PROTOCOL_DUMB_INCREMENT,

    /* always last */
        DEMO_PROTOCOL_COUNT
};

static const struct lws_protocols protocols[] = {
    {
        "dumb-increment-protocol",
              callback_dumb_increment,
                    0,
                       20,
    },
//    {
//        "lws-mirror-protocol",
//        callback_lws_mirror,
//                  0,
//                     128,
//    },
// {
//        "lws-test-raw-client",
//        callback_test_raw_client,
//                  0,
//                     128
//    },
    {   NULL, NULL, 0, 0} /* end */
};

static const struct lws_extension exts[] = {
//    {
//        "permessage-deflate",
//        lws_extension_callback_pm_deflate,
//        "permessage-deflate; client_no_context_takeover"
//    },
//    {
//        "deflate-frame",
//        lws_extension_callback_pm_deflate,
//        "deflate_frame"
//    },
    {NULL, NULL, NULL /* terminator */ }
};


void signal_cb(evutil_socket_t sock_fd, short events, void* ctx)
{
    struct event_base* event_base_loop = (struct event_base*) ctx;

    lwsl_notice("Signal caught, exiting...\n");
    event_base_loopbreak(event_base_loop);
}

static int ratelimit_connects(unsigned int* last, unsigned int secs)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);

    if (tv.tv_sec - (*last) < secs)
    {
        return 0;
    }

    *last = tv.tv_sec;

    return 1;
}

static void
ev_timeout_cb(evutil_socket_t sock_fd, short events, void* ctx)
{
    lws_callback_on_writable_all_protocol(context,
                                          &protocols[PROTOCOL_DUMB_INCREMENT]);
}

int main(int argc, char** argv)
{
    int n = 0, m, ret = 0, port = 7681, use_ssl = 0, ietf_version = -1;
    unsigned int rl_dumb = 0, rl_mirror = 0, do_ws = 1, pp_secs = 0, do_multi = 0;
    struct lws_context_creation_info info;
    const char* prot, * p;
    char path[300];
    char cert_path[1024] = "";
    char key_path[1024] = "";
    char ca_path[1024] = "";
    struct event_base* ev_base = event_base_new();
    struct event* timeout_watcher;
    struct timeval tv = {0, 50000};


    memset(&info, 0, sizeof info);

//    lws_set_log_level(0xffffffff, NULL);

    lwsl_notice("libwebsockets test client - license LGPL2.1+SLE\n");
    lwsl_notice("(C) Copyright 2010-2016 Andy Green <andy@warmcat.com>\n");

    int sigs[] = {SIGINT};
    struct event* signals[COUNT_OF(sigs)];

    for (n = 0; n < (int) COUNT_OF(sigs); n++)
    {
        signals[n] = evsignal_new(ev_base, sigs[n], signal_cb, ev_base);

        evsignal_add(signals[n], NULL);
    }

    memset(&i, 0, sizeof(i));

    char uri[] = "http://39.108.184.171:9000/guest";

    i.port = port;
    if (lws_parse_uri(uri, &prot, &i.address, &i.port, &p))
    {
        goto usage;
    }

    /* add back the leading / on path */
    path[0] = '/';
    strncpy(path + 1, p, sizeof(path) - 2);
    path[sizeof(path) - 1] = '\0';
    i.path = path;

    if (!strcmp(prot, "http") || !strcmp(prot, "ws"))
    {
        use_ssl = 0;
    }
    if (!strcmp(prot, "https") || !strcmp(prot, "wss"))
    {
        if (!use_ssl)
        {
            use_ssl = LCCSCF_USE_SSL;
        }
    }

    /*
     * create the websockets context.  This tracks open connections and
     * knows how to route any traffic and which protocol version to use,
     * and if each connection is client or server side.
     *
     * For this client-only demo, we tell it to not listen on any port.
     */

    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = protocols;
    info.gid = -1;
    info.uid = -1;
    info.ws_ping_pong_interval = pp_secs;
    info.extensions = exts;

#if defined(LWS_OPENSSL_SUPPORT)
    info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
#endif

    info.options |= LWS_SERVER_OPTION_LIBEVENT;

    if (use_ssl)
    {
        /*
         * If the server wants us to present a valid SSL client certificate
         * then we can set it up here.
         */

        if (cert_path[0])
        {
            info.ssl_cert_filepath = cert_path;
        }
        if (key_path[0])
        {
            info.ssl_private_key_filepath = key_path;
        }

        /*
         * A CA cert and CRL can be used to validate the cert send by the server
         */
        if (ca_path[0])
        {
            info.ssl_ca_filepath = ca_path;
        }

#if defined(LWS_OPENSSL_SUPPORT) && defined(LWS_HAVE_SSL_CTX_set1_param)
        else if (crl_path[0])
            lwsl_notice("WARNING, providing a CRL requires a CA cert!\n");
#endif
    }

    if (use_ssl & LCCSCF_USE_SSL)
    {
        lwsl_notice(" Using SSL\n");
    }
    else
    {
        lwsl_notice(" SSL disabled\n");
    }
    if (use_ssl & LCCSCF_ALLOW_SELFSIGNED)
    {
        lwsl_notice(" Selfsigned certs allowed\n");
    }
    else
    {
        lwsl_notice(" Cert must validate correctly (use -s to allow selfsigned)\n");
    }
    if (use_ssl & LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK)
    {
        lwsl_notice(" Skipping peer cert hostname check\n");
    }
    else
    {
        lwsl_notice(" Requiring peer cert hostname matches\n");
    }

    context = lws_create_context(&info);
    if (context == NULL)
    {
        fprintf(stderr, "Creating libwebsocket context failed\n");
        return 1;
    }

    i.context = context;
    i.ssl_connection = use_ssl;
    i.host = i.address;
    i.origin = i.address;
    i.ietf_version_or_minus_one = ietf_version;

    if (!strcmp(prot, "http") || !strcmp(prot, "https"))
    {
        lwsl_notice("using %s mode (non-ws)\n", prot);
        if (test_post)
        {
            i.method = "POST";
            lwsl_notice("POST mode\n");
        }
        else
        {
            i.method = "GET";
        }
        do_ws = 0;
    }
    else if (!strcmp(prot, "raw"))
    {
        i.method = "RAW";
        i.protocol = "lws-test-raw-client";
        lwsl_notice("using RAW mode connection\n");
        do_ws = 0;
    }
    else
    {
        lwsl_notice("using %s mode (ws)\n", prot);
    }

    /*
     * sit there servicing the websocket context to handle incoming
     * packets, and drawing random circles on the mirror protocol websocket
     *
     * nothing happens until the client websocket connection is
     * asynchronously established... calling lws_client_connect() only
     * instantiates the connection logically, lws_service() progresses it
     * asynchronously.
     */


//    timeout_watcher = event_new(ev_base, -1, EV_PERSIST, ev_timeout_cb, NULL);
//
//    evtimer_add(timeout_watcher, &tv);


    // TODO libevent api
//    // Don't use the default Signal Event Watcher & Handler
//    lws_event_sigint_cfg(context, 0, NULL);
//    // Initialize the LWS with libevent loop
//    lws_event_initloop(context, ev_base, 0);

    if (do_ws)
    {
        if (!wsi_dumb && ratelimit_connects(&rl_dumb, 2u))
        {
            lwsl_notice("dumb: connecting\n");
            i.protocol = protocols[PROTOCOL_DUMB_INCREMENT].name;
            i.pwsi = &wsi_dumb;
            lws_client_connect_via_info(&i);
        }
    }
    else if (!wsi_dumb && ratelimit_connects(&rl_dumb, 2u))
    {
        lwsl_notice("http: connecting\n");
        i.pwsi = &wsi_dumb;
        lws_client_connect_via_info(&i);
    }

    event_base_dispatch(ev_base);

    lwsl_err("Exiting\n");
    lws_context_destroy(context);

    return ret;

    usage:
    fprintf(stderr, "Usage: libwebsockets-test-client "
                    "<server address> [--port=<p>] "
                    "[--ssl] [-k] [-v <ver>] "
                    "[-d <log bitfield>] [-l]\n");
    return 1;
}
