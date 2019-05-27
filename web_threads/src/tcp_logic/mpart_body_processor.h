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

class MPartBodyProcessor
{
public:
    MPartBodyProcessor();
    ~MPartBodyProcessor();

    int Initialize(Req* http_req);
    void Finalize();

    // Returning a value other than 0 from the callbacks will abort message processing.
    static int OnPartDataBegin(struct multipart_parser* parser);
    static int OnHeaderField(struct multipart_parser* parser, const char* at, size_t length);
    static int OnHeaderValue(struct multipart_parser* parser, const char* at, size_t length);
    static int OnHeadersComplete(struct multipart_parser* parser);
    static int OnPartData(struct multipart_parser* parser, const char* at, size_t length);
    static int OnPartDataEnd(struct multipart_parser* parser);
    static int OnBodyEnd(struct multipart_parser* parser);

    struct multipart_parser* GetParser()
    {
        return parser_;
    }

private:
    Req* http_req_;
    struct multipart_parser* parser_;

    // headers of current part - don't try to use it outside callbacks it's reset on every part
    HeaderMap part_headers_;
    std::string last_header_name_;
    ParamEntry* current_param_;
};
}
}
}

#endif // WEB_THREADS_SRC_TCP_LOGIC_MPART_BODY_PROCESSOR_H_
