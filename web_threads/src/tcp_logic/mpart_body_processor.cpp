#include "mpart_body_processor.h"
#include <stdlib.h>
#include <string.h>
//#include <sys/types.h>
//#include <sys/stat.h>
#include "log_util.h"
#include "the_http_parser.h"

namespace tcp
{
namespace web
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

static int header_field_cb(struct multipart_parser* parser, const char* at, size_t length)
{
    LOG_TRACE("Parser::OnMessageBegin");

    MPartBodyProcessor* processor = (MPartBodyProcessor*) parser->data;
    processor->last_header_name.assign(at, length);

    return 0;
}

static int header_value_cb(struct multipart_parser* parser, const char* at, size_t length)
{
    MPartBodyProcessor* processor = (MPartBodyProcessor*)parser->data;

    std::string header_value(at, length);
    processor->part_headers.insert(HeaderMap::value_type(processor->last_header_name, header_value));

    return 0;
}

static int headers_complete_cb(struct multipart_parser* parser)
{
    MPartBodyProcessor* processor = (MPartBodyProcessor*)parser->data;
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

static int part_data_cb(struct multipart_parser* parser, const char* at, size_t length)
{
    if (length != 0)
    {
//        mpart_body_processor* processor = (mpart_body_processor*)p->data;
//        param_entry_append(processor->current_param, buf, len);
    }
    return 0;
}

static int part_data_begin_cb(struct multipart_parser* parser)
{
    MPartBodyProcessor* processor = (MPartBodyProcessor*)parser->data;
    processor->last_header_name.clear();
    processor->part_headers.clear();

    return 0;
}

static int part_data_end_cb(struct multipart_parser* parser)
{
    MPartBodyProcessor* processor = (MPartBodyProcessor*)parser->data;
//    Req* request = (Req*)processor->http_req;

//    params_map_add(request->params, processor->current_param);

    processor->part_headers.clear();

    return 0;
}

static int body_end_cb(struct multipart_parser* parser)
{
    return 0;
}

// 去掉字符串首尾的space
static char* str_trim(char* s)
{
    char* end;

    while (isspace(*s))
    {
        ++s;
    }

    if (*s == '\0')
    {
        return s;
    }

    end = s + strlen(s) - 1;
    while (end > s && isspace(*end))
    {
        --end;
    }

    *(end + 1) = '\0';
    return s;
}

static bool is_quote(char c)
{
    return (c == '"' || c == '\'');
}

// 去掉字符串首尾的引号（包括单引号和双引号）
static char* str_trim_quotes(char* s)
{
    char* end;

    while (is_quote(*s))
    {
        ++s;
    }

    if (*s == '\0')
    {
        return s;
    }

    end = s + strlen(s) - 1;
    while (end > s && is_quote(*end))
    {
        --end;
    }

    *(end + 1) = '\0';
    return s;
}

typedef std::map<std::string, std::string> AttrMap;

void ParseAttr(AttrMap& attrs, const char* str)
{
    const std::string s(str);
    char* header_str = (char*) s.c_str();

    while (isspace(*header_str))
    {
        header_str++;
    }

    char* pair, *name, *value;

    while ((pair = strsep(&header_str, ";")) && pair != nullptr)
    {
        name = strsep(&pair, "=");
        value = strsep(&pair, "=");

        attrs.insert(AttrMap::value_type(str_trim(name), str_trim(str_trim_quotes(value))));
    }
}

static std::string GetBoundary(const Req* http_req)
{
    HeaderMap::const_iterator it = http_req->headers.find("Content-Type");
    const std::string& content_type = it->second;
    LOG_DEBUG("content type: " << content_type);

    AttrMap attrs;
    ParseAttr(attrs, content_type.c_str() + strlen("multipart/form-data;"));

    AttrMap::const_iterator it_boundary = attrs.find("boundary");
    if (it_boundary == attrs.end())
    {
        LOG_ERROR("failed to get boundary from " << content_type);
        return "";
    }

    LOG_DEBUG("boundary: " << it_boundary->second);
    return "--" + it_boundary->second;
}

MPartBodyProcessor* MPartBodyProcessorInit(const Req* http_req)
{
    MPartBodyProcessor* processor = new MPartBodyProcessor();
    if (nullptr == processor)
    {
        LOG_ERROR("failed to alloc memory");
        return nullptr;
    }

    const std::string boundary = GetBoundary(http_req);

    processor->http_req = const_cast<Req*>(http_req);
    processor->parser = multipart_parser_init(boundary.c_str(), &settings);
    processor->parser->data = processor;
    processor->current_param = nullptr;

    return processor;
}

void MPartBodyProcessorFree(MPartBodyProcessor* processor)
{
    multipart_parser_free(processor->parser);
    delete processor;
}
}
}
}
