#ifndef HTTP_THREADS_SRC_FILE_DOWNLOAD_REQ_HANDLER_H_
#define HTTP_THREADS_SRC_FILE_DOWNLOAD_REQ_HANDLER_H_

#include "embed_http_msg_handler.h"

namespace http
{
class FileDownloadReqHandler : public EmbedMsgHandler
{
public:
    FileDownloadReqHandler();
    virtual ~FileDownloadReqHandler();

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
    int DownloadRightCheck();
    int ReadDirRightCheck();
    void SendFile(const ConnGuid* conn_guid, struct evhttp_request* evhttp_req, int http_code,
                  const KeyValMap* http_header_map, int fd, size_t offset, size_t len,
                  const char* filename, size_t file_len, time_t last_modified);

private:
    std::string file_download_req_path_;
};
}

#endif // HTTP_THREADS_SRC_FILE_DOWNLOAD_REQ_HANDLER_H_
