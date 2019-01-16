#ifndef APP_FRAME_INC_APP_FRAME_CONF_MGR_INTERFACE_H_
#define APP_FRAME_INC_APP_FRAME_CONF_MGR_INTERFACE_H_

#include "vector_types.h"

namespace app_frame
{
class ConfMgrInterface
{
public:
    virtual ~ConfMgrInterface()
    {
    }

    virtual bool EnableCPUProfiling() = 0;
    virtual bool EnableMemProfiling() = 0;
    virtual bool ReleaseFreeMem() = 0;
    virtual std::string GetGlobalLogicSo() = 0;
    virtual std::string GetTCPAddrPort() = 0;
    virtual int GetTCPConnCountLimit() = 0;
    virtual int GetTCPInactiveConnCheckIntervalSec() = 0;
    virtual int GetTCPInactiveConnCheckIntervalUsec() = 0;
    virtual int GetTCPInactiveConnLife() = 0;
    virtual int GetTCPStormInterval() = 0;
    virtual int GetTCPStormRecvCount() = 0;
    virtual int GetTCPThreadCount() = 0;
    virtual std::string GetTCPLocalLogicSo() = 0;
    virtual StrGroup GetTCPLogicSoGroup() = 0;
    virtual std::string GetHTTPAddrPort() = 0;
    virtual std::string GetHTTPsAddrPort() = 0;
    virtual std::string GetHTTPsCertificateChainFilePath() = 0;
    virtual std::string GetHTTPsPrivateKeyFilePath() = 0;
    virtual int GetHTTPConnCountLimit() = 0;
    virtual int GetHTTPMaxHeaderSize() = 0;
    virtual int GetHTTPMaxBodySize() = 0;
    virtual int GetHTTPConnTimeoutSec() = 0;
    virtual int GetHTTPConnTimeoutUsec() = 0;
    virtual int GetHTTPThreadCount() = 0;
    virtual std::string GetHTTPLocalLogicSo() = 0;
    virtual StrGroup GetHTTPLogicSoGroup() = 0;
    virtual bool HTTPPrintParsedInfo() = 0;
    virtual bool HTTPDecodeUri() = 0;
    virtual std::string GetHTTPContentType() = 0;
    virtual bool HTTPNoCache() = 0;
    virtual std::string GetHTTPFlashCrossDomainPath() = 0;
    virtual std::string GetHTTPFileUploadPath() = 0;
    virtual std::string GetHTTPFileDownloadPath() = 0;
    virtual std::string GetHTTPFileStorageDir() = 0;
    virtual std::string GetUDPAddrPort() = 0;
    virtual int GetUDPInactiveConnCheckIntervalSec() = 0;
    virtual int GetUDPInactiveConnCheckIntervalUsec() = 0;
    virtual int GetUDPInactiveConnLife() = 0;
    virtual bool UDPDoChecksum() = 0;
    virtual int GetUDPMaxMsgBodyLen() = 0;
    virtual int GetUDPThreadCount() = 0;
    virtual std::string GetUDPLocalLogicSo() = 0;
    virtual StrGroup GetUDPLogicSoGroup() = 0;
    virtual int GetPeerNeedReplyMsgCheckInterval() = 0;
    virtual int GetPeerTCPConnIntervalSec() = 0;
    virtual int GetPeerTCPConnIntervalUsec() = 0;
    virtual int GetPeerHTTPConnTimeout() = 0;
    virtual int GetPeerHTTPConnMaxRetry() = 0;
    virtual int GetWorkThreadCount() = 0;
    virtual std::string GetWorkLocalLogicSo() = 0;
    virtual StrGroup GetWorkLogicSoGroup() = 0;
    virtual int GetIOToWorkTQSizeLimit() = 0;
    virtual int GetBurdenThreadCount() = 0;
    virtual std::string GetBurdenLocalLogicSo() = 0;
    virtual StrGroup GetBurdenLogicSoGroup() = 0;
};
}

#endif // APP_FRAME_INC_APP_FRAME_CONF_MGR_INTERFACE_H_
