#include "evhtp/evhtp.h"
#include "multipart_parser.h"
#include <string>
#include <string.h>

typedef struct thr_arg_s
{
    evthr_t* thread;
//    memcached_st *cache_conn;
//    memcached_st *beansdb_conn;
//    redisContext *ssdb_conn;
//    lua_State* L;
} thr_arg_t;

typedef struct
{
    evhtp_request_t* req;
    thr_arg_t* thr_arg;
    char address[16];
    int partno;
    int succno;
    int check_name;
} mp_arg_t;

/**
 * @brief get_request_thr get the request's thread
 *
 * @param request the evhtp request
 *
 * @return the thread dealing with the request
 */
static evthr_t* get_request_thr(evhtp_request_t* request)
{
    evhtp_connection_t* htpconn;
    evthr_t* thread;

    htpconn = evhtp_request_get_connection(request);
    thread = htpconn->thread;

    return thread;
}

int on_header_field(multipart_parser* p, const char* at, size_t length)
{
    char* header_name = (char*) malloc(length + 1);
    snprintf(header_name, length + 1, "%s", at);
//    LOG_PRINT(LOG_DEBUG, "header_name %d %s: ", length, header_name);
    free(header_name);
    return 0;
}

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
                char fileType[32];
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
        if (filename[0] != '\0' && mp_arg->check_name == -1)
        {
//            LOG_PRINT(LOG_ERROR, "%s fail post type", mp_arg->address);
            evbuffer_add_printf(mp_arg->req->buffer_out,
                                "<h1>File: %s</h1>\n"
                                "<p>File type is not supported!</p>\n",
                                filename
            );
        }
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
    if (mp_arg->check_name == -1)
    {
        mp_arg->check_name = 0;
        return 0;
    }
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

    printf("------------data: %s, len: %d\n", at, (int) length);

    FILE* fp = fopen("./abc", "w");
    fwrite(at, length, 1, fp);
    fclose(fp);

    return 0;
}

int binary_parse(evhtp_request_t* req, const char* content_type, const char* buff, int post_size)
{
    printf("buff: %s, len: %d\n", buff, post_size);
    return 0;
}

int multipart_parse(evhtp_request_t* req, const char* content_type, const char* buff, int post_size)
{
    int err_no = 0;
    char* boundary = NULL, * boundary_end = NULL;
    char* boundaryPattern = NULL;
    int boundary_len = 0;
    mp_arg_t* mp_arg = NULL;
    evthr_t* thread = NULL;
    thr_arg_t* thr_arg = NULL;
    multipart_parser* parser = NULL;

    evbuffer_add_printf(req->buffer_out,
                        "<html>\n<head>\n"
                        "<title>Upload Result</title>\n"
                        "</head>\n"
                        "<body>\n"
    );

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

    thread = get_request_thr(req);
    thr_arg = (thr_arg_t*) evthr_get_aux(thread);
    mp_arg->req = req;
    mp_arg->thr_arg = thr_arg;
    mp_arg->partno = 0;
    mp_arg->succno = 0;
    mp_arg->check_name = 0;
    multipart_parser_set_data(parser, mp_arg);
    multipart_parser_execute(parser, buff, post_size);
    multipart_parser_free(parser);

    evbuffer_add_printf(req->buffer_out, "</body>\n</html>\n");
    evhtp_headers_add_header(req->headers_out, evhtp_header_new("Content-Type", "text/html", 0, 0));
    err_no = 0;

    done:
    free(boundaryPattern);
    free(mp_arg);
    return err_no;
}

void testcb(evhtp_request_t* req, void* a)
{
    const char* str = (const char*) a;

    int post_size = 0;
    char* buff = NULL;
    int err_no = 0;

    htp_method req_method = evhtp_request_get_method(req);
    if (req_method != htp_method_POST)
    {
        return;
    }

    const char* content_len = evhtp_header_find(req->headers_in, "Content-Length");
    if (!content_len)
    {
        return;
    }
    post_size = atoi(content_len);
    if (post_size <= 0)
    {
        return;
    }

    printf("content len: %d\n", post_size);

    if (post_size > 1024768 * 50)
    {
        return;
    }

//    if(post_size > settings.max_size)
//    {
//        LOG_PRINT(LOG_DEBUG, "Image Size Too Large!");
//        LOG_PRINT(LOG_ERROR, "%s fail post large", address);
//        err_no = 7;
//        goto err;
//    }
    const char* content_type = evhtp_header_find(req->headers_in, "Content-Type");
    if (!content_type)
    {
        return;
    }
    evbuf_t* buf;
    buf = req->buffer_in;
    buff = (char*) malloc(post_size);
    if (buff == NULL)
    {
        return;
    }
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
//    evhtp_headers_add_header(req->headers_out, evhtp_header_new("Server", settings.server_name, 0, 1));
    evbuffer_add_printf(req->buffer_out, "%s", str);
    evhtp_send_reply(req, EVHTP_RES_OK);
//    LOG_PRINT(LOG_DEBUG, "============post_request_cb() DONE!===============");

    free(buff);

//    size_t len = evbuffer_get_length(req->buffer_in);
//    char* buf = (char*) malloc(len + 1);
//
//    evbuffer_remove(req->buffer_in, buf, len);
//    printf("%s\n", buf);
//
//    evbuffer_add_printf(req->buffer_out, "%s", str);
//    evhtp_send_reply(req, EVHTP_RES_OK);
//
//    free(buf);
}

int main(int argc, char** argv)
{
    evbase_t* evbase = event_base_new();
    evhtp_t* htp = evhtp_new(evbase, NULL);

    evhtp_set_cb(htp, "/upload/", testcb, (void*) "upload");;

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

