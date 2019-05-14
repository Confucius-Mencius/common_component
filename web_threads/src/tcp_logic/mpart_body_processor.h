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

struct param_entry
{
    char* name;
    char* val;
    bool is_file;
    FILE* file;
};

struct mpart_body_processor
{
    Req* http_req;
    struct multipart_parser* parser;

    // headers of current part - don't try to use it outside callbacks it's reset on every part
    HeaderMap part_headers;
    std::string last_header_name;
    param_entry* current_param;

    mpart_body_processor() : last_header_name()
    {
    }
};

mpart_body_processor* mpart_body_processor_init(const Req* http_req);
void mpart_body_processor_free(mpart_body_processor* p);
}
}
}

#endif // WEB_THREADS_SRC_TCP_LOGIC_MPART_BODY_PROCESSOR_H_
