#ifndef HTTP_THREADS_SRC_FILE_UPLOAD_REQ_HANDLER_H_
#define HTTP_THREADS_SRC_FILE_UPLOAD_REQ_HANDLER_H_

#include "embed_http_msg_handler.h"

namespace http
{
class FileUploadReqHandler : public EmbedMsgHandler
{
public:
    FileUploadReqHandler();
    virtual ~FileUploadReqHandler();

    ///////////////////////// MsgHandlerInterface /////////////////////////
    const char* GetHttpReqPath() override;
    void OnHttpHeadReq(const ConnGuid* conn_guid, struct evhttp_request* evhttp_req, bool https,
                       const KeyValMap* http_header_map, const KeyValMap* http_query_map) override;
    void OnHttpGetReq(const ConnGuid* conn_guid, struct evhttp_request* evhttp_req, bool https,
                      const KeyValMap* http_header_map, const KeyValMap* http_query_map) override;
    void OnHttpPostReq(const ConnGuid* conn_guid, struct evhttp_request* evhttp_req, bool https,
                       const KeyValMap* http_header_map, const KeyValMap* http_query_map, const char* data,
                       int data_len) override;
    void OnHttpPutReq(const ConnGuid* conn_guid, struct evhttp_request* evhttp_req, bool https,
                      const KeyValMap* http_header_map, const KeyValMap* http_query_map, const char* data,
                      int data_len) override;

private:
    int UploadRightCheck();
    int WriteDirRightCheck();

private:
    std::string file_upload_req_path_;
};
}

#endif // HTTP_THREADS_SRC_FILE_UPLOAD_REQ_HANDLER_H_
