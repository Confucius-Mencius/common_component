#ifndef HTTP_THREADS_SRC_EMBED_HTTP_MSG_HANDLER_MGR_H_
#define HTTP_THREADS_SRC_EMBED_HTTP_MSG_HANDLER_MGR_H_

#include "file_download_req_handler.h"
#include "file_upload_req_handler.h"
#include "flash_cross_domain_req_handler.h"
#include "http_msg_handler_mgr_template.h"

namespace http
{
class ThreadSink;

class EmbedMsgHandlerMgr : public MsgHandlerMgrTemplate<EmbedMsgHandler>
{
public:
    EmbedMsgHandlerMgr();
    virtual ~EmbedMsgHandlerMgr();

    void SetThreadSink(ThreadSink* sink)
    {
        thread_sink_ = sink;
    }

    ///////////////////////// http::MsgHandlerMgrInterface /////////////////////////
    virtual int Initialize(const void* ctx);

private:
    ///////////////////////// http::MsgHandlerMgrTemplate /////////////////////////
    virtual int InitializeMsgHandlerVec();
    virtual void FinalizeMsgHandlerVec();

private:
    ThreadSink* thread_sink_;
    FlashCrossDomainReqHandler flash_cross_domain_req_handler_;
    FileUploadReqHandler file_upload_req_handler_;
    FileDownloadReqHandler file_download_req_handler_;
};
}

#endif // HTTP_THREADS_SRC_EMBED_HTTP_MSG_HANDLER_MGR_H_
