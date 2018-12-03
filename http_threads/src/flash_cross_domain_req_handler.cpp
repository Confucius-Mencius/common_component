#include "flash_cross_domain_req_handler.h"
#include "app_frame_conf_mgr_interface.h"
#include "http_thread_sink.h"

namespace http
{
static const char flash_policy[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                                   "<cross-domain-policy>\n"
                                   "<allow-access-from domain=\"*\"/>\n"
                                   "</cross-domain-policy>";

FlashCrossDomainReqHandler::FlashCrossDomainReqHandler()
{

}

FlashCrossDomainReqHandler::~FlashCrossDomainReqHandler()
{

}

int FlashCrossDomainReqHandler::Initialize(const void* ctx)
{
    if (EmbedMsgHandler::Initialize(ctx) != 0)
    {
        return -1;
    }

    out_header_map_["Content-Type"] = Variant(Variant::TYPE_STR, "text/xml;charset=UTF-8",
                                      strlen("text/xml;charset=UTF-8"));
    return 0;
}

const char* FlashCrossDomainReqHandler::GetHttpReqPath()
{
    flash_cross_domain_req_path_ = thread_sink_->GetConfMgr()->GetHttpFlashCrossDomainPath();
    return flash_cross_domain_req_path_.c_str();
}

void FlashCrossDomainReqHandler::OnHttpHeadReq(const ConnGuid* conn_guid, struct evhttp_request* evhttp_req,
        bool https, const KeyValMap* http_header_map, const KeyValMap* http_query_map)
{

}

void FlashCrossDomainReqHandler::OnHttpGetReq(const ConnGuid* conn_guid, struct evhttp_request* evhttp_req,
        bool https, const KeyValMap* http_header_map, const KeyValMap* http_query_map)
{
    LOG_TRACE("FlashCrossDomainReqHandler::OnHttpGetReq");
    if (logic_ctx_->scheduler->SendToClient(conn_guid, evhttp_req, HTTP_OK, &out_header_map_, flash_policy,
                                            sizeof(flash_policy) - 1) != 0)
    {
        LOG_ERROR("failed to send to client: " << conn_guid);
        return;
    }
}

void FlashCrossDomainReqHandler::OnHttpPostReq(const ConnGuid* conn_guid, struct evhttp_request* evhttp_req,
        bool https, const KeyValMap* http_header_map,
        const KeyValMap* http_query_map, const char* data, int data_len)
{

}

void FlashCrossDomainReqHandler::OnHttpPutReq(const ConnGuid* conn_guid, struct evhttp_request* evhttp_req,
        bool https, const KeyValMap* http_header_map,
        const KeyValMap* http_query_map, const char* data, int data_len)
{

}
}
