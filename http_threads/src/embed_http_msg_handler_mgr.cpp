#include "embed_http_msg_handler_mgr.h"
#include "app_frame_conf_mgr_interface.h"
#include "http_thread_sink.h"

namespace http
{
EmbedMsgHandlerMgr::EmbedMsgHandlerMgr()
{
    thread_sink_ = NULL;
}

EmbedMsgHandlerMgr::~EmbedMsgHandlerMgr()
{

}

int EmbedMsgHandlerMgr::Initialize(const void* ctx)
{
    if (NULL == ctx)
    {
        return -1;
    }

    http::LogicCtx* http_logic_ctx = (http::LogicCtx*) ctx;
    msg_dispatcher_ = http_logic_ctx->msg_dispatcher;

    if (InitializeMsgHandlerVec() != 0)
    {
        return -1;
    }

    for (http::MsgHandlerMgrTemplate<EmbedMsgHandler>::MsgHandlerVec::iterator it = msg_handler_vec_.begin();
            it != msg_handler_vec_.end(); ++it)
    {
        EmbedMsgHandler* msg_handler = *it;

        if (msg_handler->Initialize(ctx) != 0)
        {
            return -1;
        }

        if (msg_dispatcher_->AttachMsgHandler(msg_handler->GetHttpReqPath(), msg_handler) != 0)
        {
            return -1;
        }
    }

    return 0;
}

int EmbedMsgHandlerMgr::InitializeMsgHandlerVec()
{
    if (thread_sink_->GetConfMgr()->GetHttpFlashCrossDomainPath().length() > 0)
    {
        flash_cross_domain_req_handler_.SetThreadSink(thread_sink_);

        if (flash_cross_domain_req_handler_.GetHttpReqPath() != NULL)
        {
            msg_handler_vec_.push_back(&flash_cross_domain_req_handler_);
        }
    }

    if (thread_sink_->GetConfMgr()->GetHttpFileUploadPath().length() > 0)
    {
        file_upload_req_handler_.SetThreadSink(thread_sink_);

        if (file_upload_req_handler_.GetHttpReqPath() != NULL)
        {
            msg_handler_vec_.push_back(&file_upload_req_handler_);
        }
    }

    if (thread_sink_->GetConfMgr()->GetHttpFileDownloadPath().length() > 0)
    {
        file_download_req_handler_.SetThreadSink(thread_sink_);

        if (file_download_req_handler_.GetHttpReqPath() != NULL)
        {
            msg_handler_vec_.push_back(&file_download_req_handler_);
        }
    }

    return 0;
}

void EmbedMsgHandlerMgr::FinalizeMsgHandlerVec()
{
    msg_handler_vec_.clear();
}
}
