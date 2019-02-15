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
    virtual std::string GetGlobalCommonLogicSo() = 0;
    virtual StrGroup GetGlobalLogicSoGroup() = 0;
    virtual std::string GetTCPAddrPort() = 0;
    virtual int GetTCPConnCountLimit() = 0;
    virtual int GetTCPInactiveConnCheckIntervalSec() = 0;
    virtual int GetTCPInactiveConnCheckIntervalUsec() = 0;
    virtual int GetTCPInactiveConnLife() = 0;
    virtual int GetTCPStormInterval() = 0;
    virtual int GetTCPStormThreshold() = 0;
    virtual int GetTCPThreadCount() = 0;
    virtual std::string GetTCPCommonLogicSo() = 0;
    virtual StrGroup GetTCPLogicSoGroup() = 0;
    virtual std::string GetWSIface() = 0;
    virtual int GetWSPort() = 0;
    virtual int GetWSSPort() = 0;
    virtual std::string GetWSSCertificateChainFilePath() = 0;
    virtual std::string GetWSSPrivateKeyFilePath() = 0;
    virtual int GetWSConnCountLimit() = 0;
    virtual int GetWSInactiveConnCheckIntervalSec() = 0;
    virtual int GetWSInactiveConnCheckIntervalUsec() = 0;
    virtual int GetWSInactiveConnLife() = 0;
    virtual int GetWSStormInterval() = 0;
    virtual int GetWSStormThreshold() = 0;
    virtual int GetWSThreadCount() = 0;
    virtual std::string GetWSCommonLogicSo() = 0;
    virtual StrGroup GetWSLogicSoGroup() = 0;
    virtual std::string GetUDPAddrPort() = 0;
    virtual int GetUDPInactiveConnCheckIntervalSec() = 0;
    virtual int GetUDPInactiveConnCheckIntervalUsec() = 0;
    virtual int GetUDPInactiveConnLife() = 0;
    virtual int GetUDPThreadCount() = 0;
    virtual std::string GetUDPCommonLogicSo() = 0;
    virtual StrGroup GetUDPLogicSoGroup() = 0;
    virtual int GetWorkThreadCount() = 0;
    virtual std::string GetWorkCommonLogicSo() = 0;
    virtual StrGroup GetWorkLogicSoGroup() = 0;
    virtual int GetIOToWorkTQSizeLimit() = 0;
    virtual int GetBurdenThreadCount() = 0;
    virtual std::string GetBurdenCommonLogicSo() = 0;
    virtual StrGroup GetBurdenLogicSoGroup() = 0;
};
}

#endif // APP_FRAME_INC_APP_FRAME_CONF_MGR_INTERFACE_H_
