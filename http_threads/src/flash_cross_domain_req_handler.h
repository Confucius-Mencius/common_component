#ifndef HTTP_THREADS_SRC_FLASH_CROSS_DOMAIN_REQ_HANDLER_H_
#define HTTP_THREADS_SRC_FLASH_CROSS_DOMAIN_REQ_HANDLER_H_

#include "embed_http_msg_handler.h"

namespace http
{
class FlashCrossDomainReqHandler : public EmbedMsgHandler
{
public:
    FlashCrossDomainReqHandler();
    virtual ~FlashCrossDomainReqHandler();

    ///////////////////////// MsgHandlerInterface /////////////////////////
    int Initialize(const void* ctx) override;
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
    std::string flash_cross_domain_req_path_;
    KeyValMap out_header_map_;
};
}

#endif // HTTP_THREADS_SRC_FLASH_CROSS_DOMAIN_REQ_HANDLER_H_
