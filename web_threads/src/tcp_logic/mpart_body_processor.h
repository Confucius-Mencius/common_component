#ifndef WEB_THREADS_SRC_TCP_LOGIC_MPART_BODY_PROCESSOR_H_
#define WEB_THREADS_SRC_TCP_LOGIC_MPART_BODY_PROCESSOR_H_

//TODO

//form方式有三种content type：
// 1 Content-Type:application/x-www-from-urlencoded,会将表单内的数据转换为键值对，比如,name=java&age = 23. 这种只能上传键值对。消息内容会经过URL编码(空格转换为 "+" 加号，特殊符号转换为 ASCII HEX 值)
//POST / HTTP/1.1
//Content-Type:application/x-www-form-urlencoded
//Accept-Encoding: gzip, deflate
//Host: w.sohu.com
//Content-Length: 21
//Connection: Keep-Alive
//Cache-Control: no-cache

//txt1=hello&txt2=world

// 2 Content-Type:multipart/form-data,它会将表单的数据处理为一条消息，以标签为单元，用分隔符分开。既可以上传键值对，也可以上传文件，是对第一种的扩展。
//当上传的字段是文件时，会有Content-Type来表名文件类型；content-disposition，用来说明字段的一些信息；
//由于有boundary隔离，所以multipart/form-data既可以上传文件，也可以上传键值对，它采用了键值对的方式，所以可以上传多个文件。
//不对字符编码
//POST /t2/upload.do HTTP/1.1
//User-Agent: SOHUWapRebot
//Accept-Language: zh-cn,zh;q=0.5
//Accept-Charset: GBK,utf-8;q=0.7,*;q=0.7
//Connection: keep-alive
//Content-Length: 60408
//Content-Type:multipart/form-data; boundary=ZnGpDtePMx0KrHh_G0X99Yef9r8JZsRJSXC
//Host: w.sohu.com

//--ZnGpDtePMx0KrHh_G0X99Yef9r8JZsRJSXC
//Content-Disposition: form-data;name="desc"
//Content-Type: text/plain; charset=UTF-8
//Content-Transfer-Encoding: 8bit

//[......][......][......][......]...........................

//--ZnGpDtePMx0KrHh_G0X99Yef9r8JZsRJSXC
//Content-Disposition: form-data;name="pic"; filename="photo.jpg"
//Content-Type: application/octet-stream
//Content-Transfer-Encoding: binary

//[图片二进制数据]

//--ZnGpDtePMx0KrHh_G0X99Yef9r8JZsRJSXC--

// 3 text/plain 空格转换为 "+" 加号，但不对特殊字符编码。
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
