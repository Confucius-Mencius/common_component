#ifndef WEB_THREADS_SRC_TCP_LOGIC_MPART_BODY_PROCESSOR_H_
#define WEB_THREADS_SRC_TCP_LOGIC_MPART_BODY_PROCESSOR_H_

#include "multipart_parser.h"
#include "web.h"

namespace tcp
{
namespace web
{
namespace http
{
struct Req;

struct ParamEntry
{
    char* name;
    char* val;
    bool is_file;
    FILE* file;
};

// TODO 调通后转成c++风格
struct MPartBodyProcessor
{
    Req* http_req;
    struct multipart_parser* parser;

    // headers of current part - don't try to use it outside callbacks it's reset on every part
    HeaderMap part_headers;
    std::string last_header_name;
    ParamEntry* current_param;

    MPartBodyProcessor() : part_headers(), last_header_name()
    {
        http_req = nullptr;
        parser = nullptr;
        current_param = nullptr;
    }

    ~MPartBodyProcessor() {}
};

MPartBodyProcessor* MPartBodyProcessorInit(const Req* http_req);
void MPartBodyProcessorFree(MPartBodyProcessor* processor);
}
}
}

#endif // WEB_THREADS_SRC_TCP_LOGIC_MPART_BODY_PROCESSOR_H_
