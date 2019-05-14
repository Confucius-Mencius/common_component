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
    virtual bool EnableHeapProfiling() = 0;
    virtual bool ReleaseFreeMem() = 0;
    virtual std::string GetGlobalLogicSo() = 0;
    virtual int GetWorkThreadCount() = 0;
    virtual std::string GetWorkCommonLogicSo() = 0;
    virtual StrGroup GetWorkLogicSoGroup() = 0;
    virtual int GetBurdenThreadCount() = 0;
    virtual std::string GetBurdenCommonLogicSo() = 0;
    virtual StrGroup GetBurdenLogicSoGroup() = 0;
    virtual int GetTCPConnCountLimit() = 0;
    virtual int GetTCPInactiveConnCheckIntervalSec() = 0;
    virtual int GetTCPInactiveConnCheckIntervalUsec() = 0;
    virtual int GetTCPInactiveConnLife() = 0;
    virtual int GetTCPStormInterval() = 0;
    virtual int GetTCPStormThreshold() = 0;
    virtual bool RawTCPUseBufferevent() = 0;
    virtual std::string GetRawTCPAddr() = 0;
    virtual int GetRawTCPPort() = 0;
    virtual int GetRawTCPThreadCount() = 0;
    virtual std::string GetRawTCPCommonLogicSo() = 0;
    virtual StrGroup GetRawTCPLogicSoGroup() = 0;
    virtual bool ProtoTCPUseBufferevent() = 0;
    virtual bool ProtoDoChecksum() = 0;
    virtual int GetProtoMaxMsgBodyLen() = 0;
    virtual int GetProtoPartMsgCheckInterval() = 0;
    virtual int GetProtoPartMsgConnLife() = 0;
    virtual std::string GetProtoTCPAddr() = 0;
    virtual int GetProtoTCPPort() = 0;
    virtual int GetProtoTCPThreadCount() = 0;
    virtual std::string GetProtoTCPCommonLogicSo() = 0;
    virtual StrGroup GetProtoTCPLogicSoGroup() = 0;
    virtual bool WebUseBufferevent() = 0;
    virtual int GetWebPartMsgCheckInterval() = 0;
    virtual int GetWebPartMsgConnLife() = 0;
    virtual std::string GetWebAddr() = 0;
    virtual int GetWebPort() = 0;
    virtual int GetWebSecurityPort() = 0;
    virtual std::string GetWebCertificateChainFilePath() = 0;
    virtual std::string GetWebPrivateKeyFilePath() = 0;
    virtual int GetWebThreadCount() = 0;
    virtual std::string GetWebCommonLogicSo() = 0;
    virtual StrGroup GetWebLogicSoGroup() = 0;
    virtual int GetUDPInactiveConnCheckIntervalSec() = 0;
    virtual int GetUDPInactiveConnCheckIntervalUsec() = 0;
    virtual int GetUDPInactiveConnLife() = 0;
    virtual std::string GetUDPAddr() = 0;
    virtual int GetUDPPort() = 0;
    virtual int GetUDPThreadCount() = 0;
    virtual std::string GetUDPCommonLogicSo() = 0;
    virtual StrGroup GetUDPLogicSoGroup() = 0;
    virtual int GetPeerRspCheckInterval() = 0;
    virtual int GetPeerProtoTCPReconnIntervalSec() = 0;
    virtual int GetPeerProtoTCPReconnIntervalUsec() = 0;
    virtual int GetPeerProtoTCPReconnLimit() = 0;
    virtual int GetPeerHTTPConnTimeout() = 0;
    virtual int GetPeerHTTPConnMaxRetry() = 0;
};
}

#endif // APP_FRAME_INC_APP_FRAME_CONF_MGR_INTERFACE_H_
