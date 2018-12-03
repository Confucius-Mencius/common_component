/*
 * lws-minimal-http-server
 *
 * Copyright (C) 2018 Andy Green <andy@warmcat.com>
 *
 * This file is made available under the Creative Commons CC0 1.0
 * Universal Public Domain Dedication.
 *
 * This demonstrates the most minimal http server you can make with lws.
 *
 * To keep it simple, it serves stuff in the directory it was started in.
 * You can change that by changing mount.origin
 */

#include "libwebsockets.h"
#include <string.h>
#include <signal.h>

static int interrupted;

//protocols用来指明该服务器可以处理的协议类型，以数组的形式提供并NULL作为数组结尾，如果不指定则默认使用http协议。
//mounts用来设置与http服务器相关的参数，比如主机路径、URL路径、默认的主页文件等等。

static const struct lws_http_mount mount =
{
    /* .mount_next */       NULL,       /* linked-list "next" */
    /* .mountpoint */       "/",        /* mountpoint URL */
    /* .origin */           "/home/sunlands/workspace/hilton/common_component/build/output/debug/test/websockets/http/mount-origin",        /* serve from dir */
    /* .def */          "index.html",   /* default filename */
    /* .protocol */         NULL,
    /* .cgienv */           NULL,
    /* .extra_mimetypes */      NULL,
    /* .interpret */        NULL,
    /* .cgi_timeout */      0,
    /* .cache_max_age */        0,
    /* .auth_mask */        0,
    /* .cache_reusable */       0,
    /* .cache_revalidate */     0,
    /* .cache_intermediaries */ 0,
    /* .origin_protocol */      LWSMPRO_FILE,   /* files in a dir */
    /* .mountpoint_len */       1,      /* char count */
    /* .basic_auth_login_file */    NULL,
};

void sigint_handler(int sig)
{
    interrupted = 1;
}

int main(int argc, char** argv)
{
    struct lws_context_creation_info info;
    struct lws_context* context;
    int n = 0;

    signal(SIGINT, sigint_handler);

    memset(&info, 0, sizeof info); /* otherwise uninitialized garbage */
    info.port = 7681;
    info.mounts = &mount;

    lws_set_log_level(LLL_ERR | LLL_WARN | LLL_NOTICE | LLL_USER
        /* | LLL_INFO */ /* | LLL_DEBUG */, NULL);

    lwsl_user("LWS minimal http server | visit http://localhost:7681\n");

    context = lws_create_context(&info);
    if (!context)
    {
        lwsl_err("lws init failed\n");
        return 1;
    }

    while (n >= 0 && !interrupted)
    {
        n = lws_service(context, 1000);
    }

    lws_context_destroy(context);

    return 0;
}
