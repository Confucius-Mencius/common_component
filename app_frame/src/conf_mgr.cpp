#include "conf_mgr.h"

namespace app_frame
{
ConfMgr::ConfMgr() : rwlock_()
{
}

ConfMgr::~ConfMgr()
{
}

int ConfMgr::Load()
{
    AUTO_THREAD_WLOCK(rwlock_);

    enable_cpu_profiling_ = false;
    enable_heap_profiling_ = false;
    release_free_mem_ = false;
    global_logic_so_ = "";
    work_thread_count_ = 0;
    work_common_logic_so_ = "";
    work_logic_so_group_.clear();
    burden_thread_count_ = 0;
    burden_common_logic_so_ = "";
    burden_logic_so_group_.clear();
    tcp_conn_count_limit_ = 0;
    tcp_inactive_conn_check_interval_sec_ = 0;
    tcp_inactive_conn_check_interval_usec_ = 0;
    tcp_inactive_conn_life_ = 0;
    tcp_storm_interval_ = 0;
    tcp_storm_threshold_ = 0;
    raw_tcp_addr_ = "";
    raw_tcp_port_ = 0;
    raw_tcp_thread_count_ = 0;
    raw_tcp_common_logic_so_ = "";
    raw_tcp_logic_so_group_.clear();
    proto_do_checksum_ = false;
    proto_max_msg_body_len_ = 0;
    proto_part_msg_check_interval_ = 0;
    proto_part_msg_conn_life_ = 0;
    proto_tcp_addr_ = "";
    proto_tcp_port_ = 0;
    proto_tcp_thread_count_ = 0;
    proto_tcp_common_logic_so_ = "";
    proto_tcp_logic_so_group_.clear();
    ws_part_msg_check_interval_ = 0;
    ws_part_msg_conn_life_ = 0;
    ws_addr_ = "";
    ws_port_ = 0;
    ws_security_port_ = 0;
    ws_certificate_chain_file_path_ = "";
    ws_private_key_file_path_ = "";
    ws_thread_count_ = 0;
    ws_common_logic_so_ = "";
    ws_logic_so_group_.clear();
    udp_inactive_conn_check_interval_sec_ = 0;
    udp_inactive_conn_check_interval_usec_ = 0;
    udp_inactive_conn_life_ = 0;
    udp_addr_ = "";
    udp_port_ = 0;
    udp_thread_count_ = 0;
    udp_common_logic_so_ = "";
    udp_logic_so_group_.clear();

    if (LoadEnableCPUProfiling() != 0)
    {
        return -1;
    }

    if (LoadEnableHeapProfiling() != 0)
    {
        return -1;
    }

    if (LoadReleaseFreeMem() != 0)
    {
        return -1;
    }

    if (LoadGlobalLogicSo() != 0)
    {
        return -1;
    }

    if (LoadWorkThreadCount() != 0)
    {
        return -1;
    }

    if (LoadWorkCommonLogicSo() != 0)
    {
        return -1;
    }

    if (LoadWorkLogicSoGroup() != 0)
    {
        return -1;
    }

    if (LoadBurdenThreadCount() != 0)
    {
        return -1;
    }

    if (LoadBurdenCommonLogicSo() != 0)
    {
        return -1;
    }

    if (LoadBurdenLogicSoGroup() != 0)
    {
        return -1;
    }

    if (LoadTCPConnCountLimit() != 0)
    {
        return -1;
    }

    if (LoadTCPInactiveConnCheckIntervalSec() != 0)
    {
        return -1;
    }

    if (LoadTCPInactiveConnCheckIntervalUsec() != 0)
    {
        return -1;
    }

    if (LoadTCPInactiveConnLife() != 0)
    {
        return -1;
    }

    if (LoadTCPStormInterval() != 0)
    {
        return -1;
    }

    if (LoadTCPStormThreshold() != 0)
    {
        return -1;
    }

    if (LoadRawTCPAddr() != 0)
    {
        return -1;
    }

    if (LoadRawTCPPort() != 0)
    {
        return -1;
    }

    if (LoadRawTCPThreadCount() != 0)
    {
        return -1;
    }

    if (LoadRawTCPCommonLogicSo() != 0)
    {
        return -1;
    }

    if (LoadRawTCPLogicSoGroup() != 0)
    {
        return -1;
    }

    if (LoadProtoDoChecksum() != 0)
    {
        return -1;
    }

    if (LoadProtoMaxMsgBodyLen() != 0)
    {
        return -1;
    }

    if (LoadProtoPartMsgCheckInterval() != 0)
    {
        return -1;
    }

    if (LoadProtoPartMsgConnLife() != 0)
    {
        return -1;
    }

    if (LoadProtoTCPAddr() != 0)
    {
        return -1;
    }

    if (LoadProtoTCPPort() != 0)
    {
        return -1;
    }

    if (LoadProtoTCPThreadCount() != 0)
    {
        return -1;
    }

    if (LoadProtoTCPCommonLogicSo() != 0)
    {
        return -1;
    }

    if (LoadProtoTCPLogicSoGroup() != 0)
    {
        return -1;
    }

    if (LoadWSPartMsgCheckInterval() != 0)
    {
        return -1;
    }

    if (LoadWSPartMsgConnLife() != 0)
    {
        return -1;
    }

    if (LoadWSAddr() != 0)
    {
        return -1;
    }

    if (LoadWSPort() != 0)
    {
        return -1;
    }

    if (LoadWSSecurityPort() != 0)
    {
        return -1;
    }

    if (LoadWSCertificateChainFilePath() != 0)
    {
        return -1;
    }

    if (LoadWSPrivateKeyFilePath() != 0)
    {
        return -1;
    }

    if (LoadWSThreadCount() != 0)
    {
        return -1;
    }

    if (LoadWSCommonLogicSo() != 0)
    {
        return -1;
    }

    if (LoadWSLogicSoGroup() != 0)
    {
        return -1;
    }

    if (LoadUDPInactiveConnCheckIntervalSec() != 0)
    {
        return -1;
    }

    if (LoadUDPInactiveConnCheckIntervalUsec() != 0)
    {
        return -1;
    }

    if (LoadUDPInactiveConnLife() != 0)
    {
        return -1;
    }

    if (LoadUDPAddr() != 0)
    {
        return -1;
    }

    if (LoadUDPPort() != 0)
    {
        return -1;
    }

    if (LoadUDPThreadCount() != 0)
    {
        return -1;
    }

    if (LoadUDPCommonLogicSo() != 0)
    {
        return -1;
    }

    if (LoadUDPLogicSoGroup() != 0)
    {
        return -1;
    }

    return 0;
}
}
