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
    virtual std::string GetRawTCPAddr() = 0;
    virtual int GetRawTCPPort() = 0;
    virtual int GetRawTCPThreadCount() = 0;
    virtual std::string GetRawTCPCommonLogicSo() = 0;
    virtual StrGroup GetRawTCPLogicSoGroup() = 0;
    virtual bool ProtoDoChecksum() = 0;
    virtual int GetProtoMaxMsgBodyLen() = 0;
    virtual int GetProtoPartMsgCheckInterval() = 0;
    virtual int GetProtoPartMsgConnLife() = 0;
    virtual std::string GetProtoTCPAddr() = 0;
    virtual int GetProtoTCPPort() = 0;
    virtual int GetProtoTCPThreadCount() = 0;
    virtual std::string GetProtoTCPCommonLogicSo() = 0;
    virtual StrGroup GetProtoTCPLogicSoGroup() = 0;
    virtual int GetWSPartMsgCheckInterval() = 0;
    virtual int GetWSPartMsgConnLife() = 0;
    virtual std::string GetWSAddr() = 0;
    virtual int GetWSPort() = 0;
    virtual int GetWSSecurityPort() = 0;
    virtual std::string GetWSCertificateChainFilePath() = 0;
    virtual std::string GetWSPrivateKeyFilePath() = 0;
    virtual int GetWSThreadCount() = 0;
    virtual std::string GetWSCommonLogicSo() = 0;
    virtual StrGroup GetWSLogicSoGroup() = 0;
    virtual int GetUDPInactiveConnCheckIntervalSec() = 0;
    virtual int GetUDPInactiveConnCheckIntervalUsec() = 0;
    virtual int GetUDPInactiveConnLife() = 0;
    virtual std::string GetUDPAddr() = 0;
    virtual int GetUDPPort() = 0;
    virtual int GetUDPThreadCount() = 0;
    virtual std::string GetUDPCommonLogicSo() = 0;
    virtual StrGroup GetUDPLogicSoGroup() = 0;
};
}

#endif // APP_FRAME_INC_APP_FRAME_CONF_MGR_INTERFACE_H_
