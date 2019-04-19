#include "mpart_body_processor.h"
#include <stdlib.h>
#include <string.h>
//#include <sys/types.h>
//#include <sys/stat.h>
#include "log_util.h"
#include "the_http_parser.h"

namespace tcp
{
namespace http_ws
{
namespace http
{
static int header_field_cb(struct multipart_parser* p, const char* buf, size_t len);
static int header_value_cb(struct multipart_parser* p, const char* buf, size_t len);
static int part_data_cb(struct multipart_parser* p, const char* buf, size_t len);
static int part_data_begin_cb(struct multipart_parser* p);
static int headers_complete_cb(struct multipart_parser* p);
static int part_data_end_cb(struct multipart_parser* p);
static int body_end_cb(struct multipart_parser* p);

static multipart_parser_settings settings =
{
    .on_header_field = header_field_cb,
    .on_header_value = header_value_cb,
    .on_part_data = part_data_cb,
    .on_part_data_begin = part_data_begin_cb,
    .on_headers_complete = headers_complete_cb,
    .on_part_data_end = part_data_end_cb,
    .on_body_end = body_end_cb
};

static int header_field_cb(struct multipart_parser* p, const char* buf, size_t len)
{
    mpart_body_processor* processor = (mpart_body_processor*) p->data;
    processor->last_header_name.assign(buf, len);

    return 0;
}

static int header_value_cb(struct multipart_parser* p, const char* buf, size_t len)
{
    mpart_body_processor* processor = (mpart_body_processor*)p->data;

    std::string header_value(buf, len);
    processor->part_headers.insert(HeaderMap::value_type(processor->last_header_name, header_value));

    return 0;
}

static int headers_complete_cb(struct multipart_parser* p)
{
    mpart_body_processor* processor = (mpart_body_processor*)p->data;
//    Req* http_req = processor->http_req;

    HeaderMap::const_iterator it = processor->part_headers.find("Content-Disposition");
    const std::string& content_disposition = it->second;

    if (0 == strcasecmp(content_disposition.c_str(), "form-data;"))
    {
//        attrs_map_parse(cd_attrs_map, content_disposition + strlen("form-data;")); // 先;再=

//        char* name = attrs_map_get(cd_attrs_map, "name");
//        char* filename = attrs_map_get(cd_attrs_map, "filename");
//        bool is_file = (filename != NULL);

//        str_sanitize(filename);

//        processor->current_param = param_entry_init(name, NULL, is_file);

//        if (is_file)
//        {
//            char* upload_folder_path = http_request_uploads_path(http_req);
//            mkdir(upload_folder_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

//            char* file_path = malloc_str(strlen(upload_folder_path) + strlen("/") + strlen(filename));
//            sprintf(file_path, "%s/%s", upload_folder_path, filename);

//            processor->current_param->file = fopen(file_path, "a");

//            free(upload_folder_path);
//            free(file_path);
//        }
    }

    return 0;
}

static int part_data_cb(struct multipart_parser* p, const char* buf, size_t len)
{
    if (len != 0)
    {
//        mpart_body_processor* processor = (mpart_body_processor*)p->data;
//        param_entry_append(processor->current_param, buf, len);
    }
    return 0;
}

static int part_data_begin_cb(struct multipart_parser* p)
{
    mpart_body_processor* processor = (mpart_body_processor*)p->data;
    processor->last_header_name.clear();
    processor->part_headers.clear();

    return 0;
}

static int part_data_end_cb(struct multipart_parser* p)
{
    mpart_body_processor* processor = (mpart_body_processor*)p->data;
//    Req* request = (Req*)processor->http_req;

//    params_map_add(request->params, processor->current_param);

    processor->part_headers.clear();

    return 0;
}

static int body_end_cb(struct multipart_parser* p)
{
    return 0;
}

static char* str_trim(char* str)
{
    char* end;

    while (isspace(*str))
    {
        str++;
    }

    if (*str == 0)
    {
        return str;
    }

    end = str + strlen(str) - 1;
    while (end > str && isspace(*end))
    {
        end--;
    }

    *(end + 1) = 0;

    return str;
}

static bool is_quote(char c)
{
    return (c == '"' || c == '\'');
}

static char* str_strip_quotes(char* str)
{
    char* end;

    while (is_quote(*str))
    {
        str++;
    }

    if (*str == 0)
    {
        return str;
    }

    end = str + strlen(str) - 1;
    while (end > str && is_quote(*end))
    {
        end--;
    }

    *(end + 1) = 0;

    return str;
}

typedef std::map<std::string, std::string> AttrMap;

void ParseAttr(AttrMap& attrs, const char* str)
{
    char* pair, *name, *value, *header_str, *original_ptr;
    header_str = strdup(str);
    original_ptr = header_str;

    while (isspace(*header_str))
    {
        header_str++;
    }

    while ((pair = strsep(&header_str, ";")) && pair != NULL)
    {
        name = strsep(&pair, "=");
        value = strsep(&pair, "=");

        attrs.insert(AttrMap::value_type(str_trim(name), str_trim(str_strip_quotes(value))));
    }

    free(original_ptr);
}
static std::string get_boundary(const Req* http_req)
{
    HeaderMap::const_iterator it = http_req->Headers.find("Content-Type");
    const std::string& content_type = it->second;
    LOG_DEBUG("content type: " << content_type);

    AttrMap attrs;
    ParseAttr(attrs, content_type.c_str() + strlen("multipart/form-data;"));

    AttrMap::const_iterator it_boundary = attrs.find("boundary");
    if (it_boundary == attrs.end())
    {
        return "";
    }

    LOG_DEBUG("boundary: " << it_boundary->second);
    return "--" + it_boundary->second;
}

mpart_body_processor* mpart_body_processor_init(const Req* http_req)
{
    mpart_body_processor* processor = new mpart_body_processor;
    const std::string boundary = get_boundary(http_req);

    processor->http_req = const_cast<Req*>(http_req);
    processor->parser = multipart_parser_init(boundary.c_str(), &settings);
    processor->parser->data = processor;
    processor->current_param = NULL;

    return processor;
}

void mpart_body_processor_free(mpart_body_processor* processor)
{
    multipart_parser_free(processor->parser);
    delete processor;
}
}
}
}
