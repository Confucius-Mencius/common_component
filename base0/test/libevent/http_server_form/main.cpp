#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include "event2/event.h"
#include "event2/http.h"
#include "event2/buffer.h"
#include "event2/keyvalq_struct.h"
#include "event2/bufferevent.h"
#include "multipart_parser.h"

////////////////////////////////////////////////////////////////////////////////


typedef struct
{
    evhttp_request* req;
    int partno;
    int succno;
    int check_name; // 是否取到filename字段。=-1未取到，=0取到
} mp_arg_t;

/**
 * @brief strnstr find the sub string in a string
 *
 * @param s the string
 * @param find the sub string
 * @param slen find length
 *
 * @return the position of sub string or NULL
 */
char* strnstr(const char* s, const char* find, size_t slen)
{
    char c, sc;
    size_t len;

    if ((c = *find++) != '\0')
    {
        len = strlen(find);
        do
        {
            do
            {
                if ((sc = *s++) == '\0' || slen-- < 1)
                {
                    return (NULL);
                }
            } while (sc != c);
            if (len > slen)
            {
                return (NULL);
            }
        } while (strncmp(s, find, len) != 0);
        s--;
    }
    return ((char*) s);
}


/**
 * @brief strnchr find the pointer of a char in a string
 *
 * @param p the string
 * @param c the char
 * @param n find length
 *
 * @return the char position or 0
 */
char* strnchr(const char* p, char c, size_t n)
{
    if (!p)
    {
        return 0;
    }

    while (n-- > 0)
    {
        if (*p == c)
        {
            return ((char*) p);
        }
        p++;
    }
    return 0;
}

int on_header_field(multipart_parser* p, const char* at, size_t length)
{
    char* header_name = (char*) malloc(length + 1);
    snprintf(header_name, length + 1, "%s", at);
    printf("header name: %s\n", header_name);
//    LOG_PRINT(LOG_DEBUG, "header_name %d %s: ", length, header_name);
    free(header_name);
    return 0;
}

int on_header_value(multipart_parser* p, const char* at, size_t length)
{
    mp_arg_t* mp_arg = (mp_arg_t*) multipart_parser_get_data(p);
    char* filename = strnstr(at, "filename=", length);
    char* nameend = NULL;
    if (filename)
    {
        filename += 9;
        if (filename[0] == '\"')
        {
            filename++;
            nameend = strnchr(filename, '\"', length - (filename - at));
            if (!nameend)
            {
                mp_arg->check_name = -1;
            }
            else
            {
                nameend[0] = '\0';
                printf("filename: %s\n", filename);
                mp_arg->check_name = 0;

//                char fileType[32];
//                LOG_PRINT(LOG_DEBUG, "File[%s]", filename);
//                if(get_type(filename, fileType) == -1)
//                {
//                    LOG_PRINT(LOG_DEBUG, "Get Type of File[%s] Failed!", filename);
//                    mp_arg->check_name = -1;
//                }
//                else
//                {
//                    LOG_PRINT(LOG_DEBUG, "fileType[%s]", fileType);
//                    if(is_img(fileType) != 1)
//                    {
//                        LOG_PRINT(LOG_DEBUG, "fileType[%s] is Not Supported!", fileType);
//                        mp_arg->check_name = -1;
//                    }
//                }
            }
        }
//        if (filename[0] != '\0' && mp_arg->check_name == -1)
//        {
////            LOG_PRINT(LOG_ERROR, "%s fail post type", mp_arg->address);
//            evbuffer_add_printf(mp_arg->req->buffer_out,
//                                "<h1>File: %s</h1>\n"
//                                "<p>File type is not supported!</p>\n",
//                                filename
//            );
//        }
    }
    //multipart_parser_set_data(p, mp_arg);
    char* header_value = (char*) malloc(length + 1);
    snprintf(header_value, length + 1, "%s", at);
    printf("header_value %d %s\n", (int) length, header_value);
    free(header_value);
    return 0;
}

int on_chunk_data(multipart_parser* p, const char* at, size_t length)
{
    mp_arg_t* mp_arg = (mp_arg_t*) multipart_parser_get_data(p);
    mp_arg->partno++;

    if (length < 1)
    {
        return 0;
    }
    //multipart_parser_set_data(p, mp_arg);
//    char md5sum[33];
//    if(save_img(mp_arg->thr_arg, at, length, md5sum) == -1)
//    {
//        LOG_PRINT(LOG_DEBUG, "Image Save Failed!");
//        LOG_PRINT(LOG_ERROR, "%s fail post save", mp_arg->address);
//        evbuffer_add_printf(mp_arg->req->buffer_out,
//                            "<h1>Failed!</h1>\n"
//                            "<p>File save failed!</p>\n"
//        );
//    }
//    else
//    {
//        mp_arg->succno++;
//        LOG_PRINT(LOG_INFO, "%s succ post pic:%s size:%d", mp_arg->address, md5sum, length);
//        evbuffer_add_printf(mp_arg->req->buffer_out,
//                            "<h1>MD5: %s</h1>\n"
//                            "Image upload successfully! You can get this image via this address:<br/><br/>\n"
//                            "<a href=\"/%s\">http://yourhostname:%d/%s</a>?w=width&h=height&g=isgray&x=position_x&y=position_y&r=rotate&q=quality&f=format\n",
//                            md5sum, md5sum, settings.port, md5sum
//        );
//    }

    char* data = (char*) malloc(length + 1);
    memmove(data, at, length);

    printf("------------data:\n %s, len: %d\n", data, (int) length);

    FILE* fp = fopen("./abc", "w");
    fwrite(at, length, 1, fp);
    fclose(fp);

    return 0;
}

int binary_parse(struct evhttp_request* req, const char* content_type, const char* buff, int post_size)
{
    printf("buff: %s, len: %d\n", buff, post_size);
    return 0;
}

int multipart_parse(struct evhttp_request* req, const char* content_type, const char* buff, int post_size)
{
    int err_no = 0;
    char* boundary = NULL, * boundary_end = NULL;
    char* boundaryPattern = NULL;
    int boundary_len = 0;
    mp_arg_t* mp_arg = NULL;
    multipart_parser* parser = NULL;

//    evbuffer_add_printf(req->buffer_out,
//                        "<html>\n<head>\n"
//                        "<title>Upload Result</title>\n"
//                        "</head>\n"
//                        "<body>\n"
//    );

    if (strstr(content_type, "boundary") == 0)
    {
        printf("boundary NOT found!\n");
        err_no = -1;
        goto done;
    }

    boundary = (char*) strchr(content_type, '=');
    boundary++;
    boundary_len = strlen(boundary);

    if (boundary[0] == '"')
    {
        boundary++;
        boundary_end = strchr(boundary, '"');
        if (!boundary_end)
        {
            printf("Invalid boundary in multipart/form-data POST data\n");
            err_no = -1;
            goto done;
        }
    }
    else
    {
        /* search for the end of the boundary */
        boundary_end = strpbrk(boundary, ",;");
    }
    if (boundary_end)
    {
        boundary_end[0] = '\0';
        boundary_len = boundary_end - boundary;
    }

    printf("boundary Find. boundary = %s\n", boundary);
    boundaryPattern = (char*) malloc(boundary_len + 3);
    if (boundaryPattern == NULL)
    {
        printf("boundarypattern malloc failed!\n");
        err_no = -1;
        goto done;
    }
    snprintf(boundaryPattern, boundary_len + 3, "--%s", boundary);
    printf("boundaryPattern = %s, strlen = %d\n", boundaryPattern, (int) strlen(boundaryPattern));

    multipart_parser_settings settings;
    memset(&settings, 0, sizeof(settings));
    settings.on_header_field = on_header_field;
    settings.on_header_value = on_header_value;
    settings.on_chunk_data = on_chunk_data;

    parser = multipart_parser_init(boundaryPattern, &settings);
    if (!parser)
    {
        printf("Multipart_parser Init Failed!\n");
        err_no = -1;
        goto done;
    }
    mp_arg = (mp_arg_t*) malloc(sizeof(mp_arg_t));
    if (!mp_arg)
    {
        printf("Multipart_parser Arg Malloc Failed!\n");
        err_no = -1;
        goto done;
    }

    mp_arg->req = req;
    mp_arg->partno = 0;
    mp_arg->succno = 0;
    mp_arg->check_name = 0;
    multipart_parser_set_data(parser, mp_arg);
    multipart_parser_execute(parser, buff, post_size);
    multipart_parser_free(parser);

    printf("partno: %d, succno: %d, check name: %d", mp_arg->partno, mp_arg->succno, mp_arg->check_name);

//    evbuffer_add_printf(req->buffer_out, "</body>\n</html>\n");
//    evhtp_headers_add_header(req->headers_out, evhtp_header_new("Content-Type", "text/html", 0, 0));
    err_no = 0;

    done:
    free(boundaryPattern);
    free(mp_arg);
    return err_no;
}
////////////////////////////////////////////////////////////////////////////////

void OnHttpReqClose(struct evhttp_connection* evcon, void* args)
{
    printf("connection closed\n"); // 连接是客户端关闭的
}

struct event_base* g_eventBase;

void show_help()
{
    char help[] = "written by Min (http://54min.com)\n\n"
                  "-l <ip_addr> interface to listen on, default is 0.0.0.0\n"
                  "-p <num>     port number to listen on, default is 1984\n"
                  "-d           run as a deamon\n"
                  "-t <second>  timeout for a http request, default is 120 seconds\n"
                  "-h           print this help and exit\n"
                  "\n";
    fprintf(stderr, "%s", (char*) help);
}

//当向进程发出SIGTERM/SIGHUP/SIGINT/SIGQUIT的时候，终止event的事件侦听循环
void signal_handler(int sig)
{
    switch (sig)
    {
        case SIGTERM:
        case SIGHUP:
        case SIGQUIT:
        case SIGINT:
            event_base_loopbreak(g_eventBase); //终止侦听event_dispatch()的事件侦听循环，执行之后的代码
            break;
    }
}

char uri_root[512];

static const struct table_entry
{
    const char* extension;
    const char* content_type;
} content_type_table[] = {{"txt",  "text/plain"},
                          {"c",    "text/plain"},
                          {
                           "h",    "text/plain"
                          },
                          {"html", "text/html"},
                          {"htm",  "text/htm"},
                          {
                           "css",  "text/css"
                          },
                          {"gif",  "image/gif"},
                          {"jpg",  "image/jpeg"},
                          {
                           "jpeg", "image/jpeg"
                          },
                          {"png",  "image/png"},
                          {
                           "pdf",
                                   "application/pdf"
                          },
                          {"ps",   "application/postsript"},
                          {NULL, NULL}};

/* Try to guess a good content-Type for 'path' */
static const char* guess_content_type(const char* path)
{
    const char* last_period, * extension;
    const struct table_entry* ent;
    last_period = strrchr(path, '.');
    if (!last_period || strchr(last_period, '/'))
    {
        goto not_found;
    }
    /* no exension */
    extension = last_period + 1;
    for (ent = &content_type_table[0]; ent->extension; ++ent)
    {
        if (!evutil_ascii_strcasecmp(ent->extension, extension))
        {
            return ent->content_type;
        }
    }

    not_found:
    return "application/misc";
}

/* Callback used for the /dump URI, and for every non-GET request:
 * dumps all information to stdout and gives back a trivial 200 ok */
// http://localhost/dump
static void dump_request_cb(struct evhttp_request* req, void* arg)
{
    struct evhttp_connection* evcon = evhttp_request_get_connection(req);

    struct bufferevent* buffer_event = evhttp_connection_get_bufferevent(evcon);
    evutil_socket_t socketFD = bufferevent_getfd(buffer_event);

    char* peer_address = NULL;
    ev_uint16_t peer_port;
    evhttp_connection_get_peer(evcon, (&peer_address), &peer_port);
    printf("socketFD: %d, peer_address: %s, port: %d\n", socketFD,
           peer_address, peer_port);

    /** Set a callback for connection close. */
    evhttp_connection_set_closecb(evcon, OnHttpReqClose, NULL);

    const char* cmdtype;
    struct evkeyvalq* headers;
    struct evkeyval* header;
    struct evbuffer* buf;

    switch (evhttp_request_get_command(req))
    {
        case EVHTTP_REQ_GET:
            cmdtype = "GET";
            break;
        case EVHTTP_REQ_POST:
            cmdtype = "POST";
            break;
        case EVHTTP_REQ_HEAD:
            cmdtype = "HEAD";
            break;
        case EVHTTP_REQ_PUT:
            cmdtype = "PUT";
            break;
        case EVHTTP_REQ_DELETE:
            cmdtype = "DELETE";
            break;
        case EVHTTP_REQ_OPTIONS:
            cmdtype = "OPTIONS";
            break;
        case EVHTTP_REQ_TRACE:
            cmdtype = "TRACE";
            break;
        case EVHTTP_REQ_CONNECT:
            cmdtype = "CONNECT";
            break;
        case EVHTTP_REQ_PATCH:
            cmdtype = "PATCH";
            break;
        default:
            cmdtype = "unknown";
            break;
    }

    printf("Received a %s request for %s\nHeaders:\n", cmdtype,
           evhttp_request_get_uri(req));

    headers = evhttp_request_get_input_headers(req);
    for (header = headers->tqh_first; header; header = header->next.tqe_next)
    {
        printf("  %s: %s\n", header->key, header->value);
    }

//    puts("Input data: <<<");
//    while (evbuffer_get_length(buf))
//    {
//        int n;
//        char cbuf[128];
//        n = evbuffer_remove(buf, cbuf, sizeof(buf) - 1); // 从buf中取“sizeof(buf) - 1”个字节到cbuf中
//        if (n > 0)
//        {
//            (void) fwrite(cbuf, 1, n, stdout);
//        }
//    }
//    puts(">>>");

    const char* content_len = evhttp_find_header(headers, "Content-Length");
    if (!content_len)
    {
        return;
    }
    int post_size = atoi(content_len);
    if (post_size <= 0)
    {
        return;
    }

    printf("content len: %d\n", post_size);

    if (post_size > 1024768 * 50)
    {
        return;
    }

    const char* content_type = evhttp_find_header(headers, "Content-Type");
    if (!content_type)
    {
        return;
    }
    char* buff = (char*) malloc(post_size);
    if (buff == NULL)
    {
        return;
    }

    buf = evhttp_request_get_input_buffer(req);

    int rmblen, evblen;
    if (evbuffer_get_length(buf) <= 0)
    {
        free(buff);
        return;
    }
    while ((evblen = evbuffer_get_length(buf)) > 0)
    {
        printf("evblen = %d\n", evblen);
        rmblen = evbuffer_remove(buf, buff, evblen);
        printf("rmblen = %d\n", rmblen);
        if (rmblen < 0)
        {
            free(buff);
            return;
        }
    }

    puts("Input data: <<<");
    (void) fwrite(buff, post_size, 1, stdout);
    puts(">>>");


    int err_no = 0;

    if (strstr(content_type, "multipart/form-data") == NULL)
    {
        err_no = binary_parse(req, content_type, buff, post_size);
    }
    else
    {
        err_no = multipart_parse(req, content_type, buff, post_size);
    }
    if (err_no != 0)
    {
        return;
    }

    evhttp_send_reply(req, 200, "OK", NULL);
}

/* This callback gets invoked when we get any http request that doesn't match
 * any other callback.  Like any evhttp server callback, it has a simple job:
 * it must eventually call evhttp_send_error() or evhttp_send_reply().
 */
// http://localhost/not_dump
static void send_document_cb(struct evhttp_request* req, void* arg)
{
    struct evbuffer* evb = NULL;
    const char* docroot = (const char*) arg;
    const char* uri = evhttp_request_get_uri(req);
    struct evhttp_uri* decoded = NULL;
    const char* path;
    char* decoded_path;
    char* whole_path = NULL;
    size_t len;
    int fd = -1;
    struct stat st;

    if (evhttp_request_get_command(req) != EVHTTP_REQ_GET)
    {
        dump_request_cb(req, arg);
        return;
    }

    printf("Got a GET request for <%s>\n", uri);

    /* Decode the URI */
    decoded = evhttp_uri_parse(uri);
    if (!decoded)
    {
        printf("It's not a good URI. Sending BADREQUEST\n");
        evhttp_send_error(req, HTTP_BADREQUEST, 0);
        return;
    }

    /* Let's see what path the user asked for. */
    path = evhttp_uri_get_path(decoded);
    if (!path)
    {
        path = "/";
    }

    /* We need to decode it, to see what path the user really wanted. */
    decoded_path = evhttp_uridecode(path, 0, NULL);
    if (decoded_path == NULL)
    {
        goto err;
    }
    /* Don't allow any ".."s in the path, to avoid exposing stuff outside
     * of the docroot.  This test is both overzealous and underzealous:
     * it forbids aceptable paths like "/this/one..here", but it doesn't
     * do anything to prevent symlink following." */
    if (strstr(decoded_path, ".."))
    {
        goto err;
    }

    if (strstr(uri, "favicon.ico"))
    {
//        LOG_PRINT(LOG_DEBUG, "favicon.ico Request, Denied.");
//        evhtp_headers_add_header(req->headers_out, evhtp_header_new("Server", settings.server_name, 0, 1));
//        evhtp_headers_add_header(req->headers_out, evhtp_header_new("Content-Type", "text/html", 0, 0));
//        zimg_headers_add(req, settings.headers);
//        evhtp_send_reply(req, 200);
        evhttp_send_reply(req, 200, NULL, NULL);
        goto done;
    }


    len = strlen(decoded_path) + strlen(docroot) + 2;
    if (!(whole_path = (char*) malloc(len)))
    {
        perror("malloc");
        goto err;
    }
    evutil_snprintf(whole_path, len, "%s/%s", docroot, decoded_path);

    if (stat(whole_path, & st) < 0)
    {
        goto err;
    }

    /* This holds the content we're sending. */
    evb = evbuffer_new();

    if (S_ISDIR(st.st_mode))
    {
        /* If it's a directory, read the comments and make a little
         * index page */
        DIR* d;
        struct dirent* ent;
        const char* trailing_slash = "";

        if (!strlen(path) || path[strlen(path) - 1] != '/')
        {
            trailing_slash = "/";
        }

        if (!(d = opendir(whole_path)))
        {
            goto err;
        }

        evbuffer_add_printf(evb, "<html>\n <head>\n"
                                 "  <title>%s</title>\n"
                                 "  <base href='%s%s%s'>\n"
                                 " </head>\n"
                                 " <body>\n"
                                 "  <h1>%s</h1>\n"
                                 "  <ul>\n", decoded_path, /* XXX html-escape this. */
                            uri_root, path, /* XXX html-escape this? */
                            trailing_slash, decoded_path /* XXX html-escape this */);

        while ((ent = readdir(d)))
        {
            const char* name = ent->d_name;
            evbuffer_add_printf(evb, "    <li><a href=\"%s\">%s</a>\n", name,
                                name);/* XXX escape this */
        }
        evbuffer_add_printf(evb, "</ul></body></html>\n");

        closedir(d);

        evhttp_add_header(evhttp_request_get_output_headers(req),
                          "Content-Type", "text/html");
    }
    else
    {
        /* Otherwise it's a file; add it to the buffer to get
         * sent via sendfile */
        const char* type = guess_content_type(decoded_path);
        if ((fd = open(whole_path, O_RDONLY)) < 0)
        {
            perror("open");
            goto err;
        }

        if (fstat(fd, &st) < 0)
        {
            /* Make sure the length still matches, now that we
             * opened the file :/ */
            perror("fstat");
            goto err;
        }
        evhttp_add_header(evhttp_request_get_output_headers(req),
                          "Content-Type", type);
        evbuffer_add_file(evb, fd, 0, st.st_size);
    }

    evhttp_send_reply(req, 200, "OK", evb);
    goto done;

    err:
    evhttp_send_error(req, 404, "Document was not found");
    if (fd >= 0)
    {
        close(fd);
    }

    done:
    if (decoded)
    {
        evhttp_uri_free(decoded);
    }
    if (decoded_path)
    {
        free(decoded_path);
    }
    if (whole_path)
    {
        free(whole_path);
    }
    if (evb)
    {
        evbuffer_free(evb);
    }
}

static void syntax(void)
{
    fprintf(stdout, "Syntax: http-server <docroot>\n");
}

int main(int argc, char* argv[])
{
    //自定义信号处理函数
    signal(SIGHUP, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGQUIT, signal_handler);

    //默认参数
    char httpd_option_listen[32] = "0.0.0.0";
    int httpd_option_port = 48081;
    int httpd_option_daemon = 0;
    int httpd_option_timeout = 120; //in seconds
    (void) httpd_option_timeout;

    //获取参数
    int c;
    while ((c = getopt(argc, argv, "l:p:dt:h")) != -1)
    {
        switch (c)
        {
            case 'l':
                memset(httpd_option_listen, 0, sizeof(httpd_option_listen));
                strncpy(httpd_option_listen, optarg,
                        sizeof(httpd_option_listen) - 1);
                break;
            case 'p':
                httpd_option_port = atoi(optarg);
                break;
            case 'd':
                httpd_option_daemon = 1;
                break;
            case 't':
                httpd_option_timeout = atoi(optarg);
                break;
            case 'h':
            default:
                show_help();
                exit(EXIT_SUCCESS);
        }
    }

    //判断是否设置了-d，以daemon运行
    if (httpd_option_daemon)
    {
        pid_t pid;
        pid = fork();
        if (pid < 0)
        {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
        if (pid > 0)
        {
            //生成子进程成功，退出父进程
            exit(EXIT_SUCCESS);
        }
    }

    struct evhttp* http;
    struct evhttp_bound_socket* handle;

    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    {

        if (argc < 2)
        {
            syntax();
            return 1;
        }
    }

    g_eventBase = event_base_new();
    if (!g_eventBase)
    {
        fprintf(stderr, "Couldn't create an event_base: exiting\n");
        return 1;
    }

    /* Create a new evhttp object to handle requests. */
    http = evhttp_new(g_eventBase);
    if (!http)
    {
        fprintf(stderr, "couldn't create evhttp. Exiting.\n");
        event_base_free(g_eventBase);
        return 1;
    }

    /* The /dump URI will dump all requests to stdout and say 200 ok. */
    int ret = evhttp_set_cb(http, "/dump", dump_request_cb, NULL);
    if (ret != 0)
    {
        // error log
        evhttp_free(http);
        event_base_free(g_eventBase);
        return 1;
    }

    /* We want to accept arbitrary requests, so we need to set a "generic"
     * cb.  We can also add callbacks for specific paths. */
    evhttp_set_gencb(http, send_document_cb, argv[1]);

    /* Now we tell the evhttp what port to listen on */
    handle = evhttp_bind_socket_with_handle(http, httpd_option_listen,
                                            httpd_option_port);
    if (!handle)
    {
        fprintf(stderr, "couldn't bind to port %d. Exiting.\n",
                (int) httpd_option_port);
        return 1;
    }

    {
        /* Extract and display the address we're listening on. */
        struct sockaddr_storage ss;
        evutil_socket_t fd;
        ev_socklen_t socklen = sizeof(ss);
        char addrbuf[128];
        void* inaddr;
        const char* addr;
        int got_port = -1;
        fd = evhttp_bound_socket_get_fd(handle);
        memset(&ss, 0, sizeof(ss));
        if (getsockname(fd, (struct sockaddr*) &ss, &socklen))
        {
            perror("getsockname() failed");
            return 1;
        }
        if (ss.ss_family == AF_INET)
        {
            got_port = ntohs(((struct sockaddr_in*) &ss)->sin_port);
            inaddr = &((struct sockaddr_in*) &ss)->sin_addr;
        }
        else if (ss.ss_family == AF_INET6)
        {
            got_port = ntohs(((struct sockaddr_in6*) &ss)->sin6_port);
            inaddr = &((struct sockaddr_in6*) &ss)->sin6_addr;
        }
        else
        {
            fprintf(stderr, "Weird address family %d\n", ss.ss_family);
            return 1;
        }
        addr = evutil_inet_ntop(ss.ss_family, inaddr, addrbuf, sizeof(addrbuf));
        if (addr)
        {
            printf("Listening on %s:%d\n", addr, got_port);
            evutil_snprintf(uri_root, sizeof(uri_root), "http://%s:%d", addr,
                            got_port);
        }
        else
        {
            fprintf(stderr, "evutil_inet_ntop failed\n");
            return 1;
        }
    }

    event_base_dispatch(g_eventBase);

    evhttp_free(http);
    event_base_free(g_eventBase);
}
