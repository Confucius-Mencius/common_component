#include "mpart_body_processor.h"
#include <stdlib.h>
#include <string.h>
//#include <sys/types.h>
//#include <sys/stat.h>
#include "log_util.h"
#include "singleton.h"
#include "the_http_parser.h"

namespace tcp
{
namespace web
{
namespace http
{
// 去掉字符串首尾的space
static char* StrTrimSpace(char* s)
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

static bool IsQuote(char c)
{
    return (c == '"' || c == '\'');
}

// 去掉字符串首尾的引号（包括单引号和双引号）
static char* StrTrimQuotes(char* s)
{
    char* end;

    while (IsQuote(*s))
    {
        ++s;
    }

    if (*s == '\0')
    {
        return s;
    }

    end = s + strlen(s) - 1;
    while (end > s && IsQuote(*end))
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

        attrs.insert(AttrMap::value_type(StrTrimSpace(name), StrTrimSpace(StrTrimQuotes(value))));
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

class MPartParserSettings
{
public:
    MPartParserSettings()
    {
        settings_.on_header_field = MPartBodyProcessor::OnHeaderField;
        settings_.on_header_value = MPartBodyProcessor::OnHeaderValue;
        settings_.on_part_data = MPartBodyProcessor::OnPartData;
        settings_.on_part_data_begin = MPartBodyProcessor::OnPartDataBegin;
        settings_.on_headers_complete = MPartBodyProcessor::OnHeadersComplete;
        settings_.on_part_data_end = MPartBodyProcessor::OnPartDataEnd;
        settings_.on_body_end = MPartBodyProcessor::OnBodyEnd;
    }

    const struct multipart_parser_settings* Get() const
    {
        return &settings_;
    }

private:
    struct multipart_parser_settings settings_;
};

#define TheMPartParserSettings Singleton<MPartParserSettings>::Instance()

MPartBodyProcessor::MPartBodyProcessor() : part_headers_(), last_header_name_()
{
    http_req_ = nullptr;
    parser_ = nullptr;
    current_param_ = nullptr;
}

MPartBodyProcessor::~MPartBodyProcessor()
{
    Finalize();
}

int MPartBodyProcessor::Initialize(Req* http_req)
{
    const std::string boundary = GetBoundary(http_req);
    if (boundary.empty())
    {
        return -1;
    }

    http_req_ = http_req;

    parser_ = multipart_parser_init(boundary.c_str(), TheMPartParserSettings->Get());
    if (nullptr == parser_)
    {
        LOG_ERROR("multipart_parser_init failed");
        return -1;
    }

    parser_->data = this;
    current_param_ = nullptr;

    return 0;
}

void MPartBodyProcessor::Finalize()
{
    if (parser_ != nullptr)
    {
        multipart_parser_free(parser_);
        parser_ = nullptr;
    }
}

int MPartBodyProcessor::OnPartDataBegin(multipart_parser* parser)
{
    LOG_TRACE("MPartBodyProcessor::OnPartDataBegin");
    MPartBodyProcessor* processor = (MPartBodyProcessor*) parser->data;

    processor->last_header_name_.clear();
    processor->part_headers_.clear();

    return 0;
}

int MPartBodyProcessor::OnHeaderField(multipart_parser* parser, const char* at, size_t length)
{
    LOG_TRACE("MPartBodyProcessor::OnHeaderField, " << std::string(at, length));

    MPartBodyProcessor* processor = (MPartBodyProcessor*) parser->data;
    processor->last_header_name_.assign(at, length);

    return 0;
}

int MPartBodyProcessor::OnHeaderValue(multipart_parser* parser, const char* at, size_t length)
{
    LOG_TRACE("MPartBodyProcessor::OnHeaderValue, " << std::string(at, length));
    MPartBodyProcessor* processor = (MPartBodyProcessor*) parser->data;

    std::string header_value(at, length);
    processor->part_headers_.insert(HeaderMap::value_type(processor->last_header_name_, header_value));

    return 0;
}

int MPartBodyProcessor::OnHeadersComplete(multipart_parser* parser)
{
    LOG_TRACE("MPartBodyProcessor::OnHeadersComplete");
    MPartBodyProcessor* processor = (MPartBodyProcessor*) parser->data;

    //    Req* http_req = processor->http_req;

    HeaderMap::const_iterator it = processor->part_headers_.find("Content-Disposition");
    if (it == processor->part_headers_.end())
    {
        return 0;
    }

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

int MPartBodyProcessor::OnPartData(multipart_parser* parser, const char* at, size_t length)
{
    LOG_TRACE("MPartBodyProcessor::OnPartData, " << std::string(at, length));
//    MPartBodyProcessor2* processor = (MPartBodyProcessor2*) parser->data;

    if (length != 0)
    {
//        param_entry_append(processor->current_param, buf, len);
    }

    return 0;
}

int MPartBodyProcessor::OnPartDataEnd(multipart_parser* parser)
{
    LOG_TRACE("MPartBodyProcessor::OnPartDataEnd");
    MPartBodyProcessor* processor = (MPartBodyProcessor*) parser->data;

    //    Req* request = (Req*)processor->http_req;

    //    params_map_add(request->params, processor->current_param);

    processor->part_headers_.clear();

    return 0;
}

int MPartBodyProcessor::OnBodyEnd(multipart_parser* parser)
{
    LOG_TRACE("MPartBodyProcessor::OnBodyEnd");
    return 0;
}
}
}
}
