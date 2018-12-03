#ifndef APP_FRAME_INC_APP_FRAME_CONF_MGR_INTERFACE_H_
#define APP_FRAME_INC_APP_FRAME_CONF_MGR_INTERFACE_H_

#include <stdint.h>
#include "vector_type.h"

namespace app_frame
{
class ConfMgrInterface
{
public:
    virtual ~ConfMgrInterface()
    {
    }

    virtual bool EnableCpuProfiling() = 0;
    virtual bool EnableMemProfiling() = 0;
    virtual bool ReleaseFreeMem() = 0;
    virtual std::string GetGlobalLogicSo() = 0;
    virtual std::string GetTcpAddrPort() = 0;
    virtual int GetTcpConnCountLimit() = 0;
    virtual int GetTcpInactiveConnCheckIntervalSec() = 0;
    virtual int GetTcpInactiveConnCheckIntervalUsec() = 0;
    virtual int GetTcpInactiveConnLife() = 0;
    virtual bool TcpDoChecksum() = 0;
    virtual int GetTcpMaxMsgBodyLen() = 0;
    virtual int GetTcpPartMsgCheckInterval() = 0;
    virtual int GetTcpPartMsgConnLife() = 0;
    virtual int GetTcpThreadCount() = 0;
    virtual std::string GetTcpLocalLogicSo() = 0;
    virtual StrGroup GetTcpLogicSoGroup() = 0;
    virtual std::string GetRawTcpAddrPort() = 0;
    virtual int GetRawTcpConnCountLimit() = 0;
    virtual int GetRawTcpInactiveConnCheckIntervalSec() = 0;
    virtual int GetRawTcpInactiveConnCheckIntervalUsec() = 0;
    virtual int GetRawTcpInactiveConnLife() = 0;
    virtual int GetRawTcpThreadCount() = 0;
    virtual std::string GetRawTcpLocalLogicSo() = 0;
    virtual StrGroup GetRawTcpLogicSoGroup() = 0;
    virtual std::string GetHttpAddrPort() = 0;
    virtual std::string GetHttpsAddrPort() = 0;
    virtual std::string GetHttpsCertificateChainFilePath() = 0;
    virtual std::string GetHttpsPrivateKeyFilePath() = 0;
    virtual int GetHttpConnCountLimit() = 0;
    virtual int GetHttpMaxHeaderSize() = 0;
    virtual int GetHttpMaxBodySize() = 0;
    virtual int GetHttpConnTimeoutSec() = 0;
    virtual int GetHttpConnTimeoutUsec() = 0;
    virtual int GetHttpThreadCount() = 0;
    virtual std::string GetHttpLocalLogicSo() = 0;
    virtual StrGroup GetHttpLogicSoGroup() = 0;
    virtual bool HttpPrintParsedInfo() = 0;
    virtual bool HttpDecodeUri() = 0;
    virtual std::string GetHttpContentType() = 0;
    virtual bool HttpNoCache() = 0;
    virtual std::string GetHttpFlashCrossDomainPath() = 0;
    virtual std::string GetHttpFileUploadPath() = 0;
    virtual std::string GetHttpFileDownloadPath() = 0;
    virtual std::string GetHttpFileStorageDir() = 0;
    virtual std::string GetUdpAddrPort() = 0;
    virtual int GetUdpInactiveConnCheckIntervalSec() = 0;
    virtual int GetUdpInactiveConnCheckIntervalUsec() = 0;
    virtual int GetUdpInactiveConnLife() = 0;
    virtual bool UdpDoChecksum() = 0;
    virtual int GetUdpMaxMsgBodyLen() = 0;
    virtual int GetUdpThreadCount() = 0;
    virtual std::string GetUdpLocalLogicSo() = 0;
    virtual StrGroup GetUdpLogicSoGroup() = 0;
    virtual int GetPeerNeedReplyMsgCheckInterval() = 0;
    virtual int GetPeerTcpConnIntervalSec() = 0;
    virtual int GetPeerTcpConnIntervalUsec() = 0;
    virtual int GetPeerHttpConnTimeout() = 0;
    virtual int GetPeerHttpConnMaxRetry() = 0;
    virtual int GetWorkThreadCount() = 0;
    virtual std::string GetWorkLocalLogicSo() = 0;
    virtual StrGroup GetWorkLogicSoGroup() = 0;
    virtual int GetIoToWorkTqSizeLimit() = 0;
    virtual int GetBurdenThreadCount() = 0;
    virtual std::string GetBurdenLocalLogicSo() = 0;
    virtual StrGroup GetBurdenLogicSoGroup() = 0;
};
}

#endif // APP_FRAME_INC_APP_FRAME_CONF_MGR_INTERFACE_H_
