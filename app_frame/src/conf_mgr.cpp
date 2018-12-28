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
    enable_mem_profiling_ = false;
    release_free_mem_ = false;
    tcp_addr_port_ = "";
    tcp_conn_count_limit_ = 0;
    tcp_inactive_conn_check_interval_sec_ = 0;
    tcp_inactive_conn_check_interval_usec_ = 0;
    tcp_inactive_conn_life_ = 0;
    tcp_thread_count_ = 0;
    tcp_local_logic_so_ = "";
    tcp_logic_so_group_.clear();
    http_addr_port_ = "";
    https_addr_port_ = "";
    https_certificate_chain_file_path_ = "";
    https_private_key_file_path_ = "";
    http_conn_count_limit_ = 0;
    http_max_header_size_ = 0;
    http_max_body_size_ = 0;
    http_conn_timeout_sec_ = 0;
    http_conn_timeout_usec_ = 0;
    http_thread_count_ = 0;
    http_local_logic_so_ = "";
    http_logic_so_group_.clear();
    http_print_parsed_info_ = false;
    http_decode_uri_ = false;
    http_content_type_ = "";
    http_no_cache_ = false;
    http_flash_cross_domain_path_ = "";
    http_file_upload_path_ = "";
    http_file_download_path_ = "";
    http_file_storage_dir_ = "";
    udp_addr_port_ = "";
    udp_inactive_conn_check_interval_sec_ = 0;
    udp_inactive_conn_check_interval_usec_ = 0;
    udp_inactive_conn_life_ = 0;
    udp_thread_count_ = 0;
    udp_local_logic_so_ = "";
    udp_logic_so_group_.clear();
    global_logic_so_ = "";
    work_thread_count_ = 0;
    work_local_logic_so_ = "";
    work_logic_so_group_.clear();
    io_to_work_tq_size_limit_ = 0;
    burden_thread_count_ = 0;
    burden_local_logic_so_ = "";
    burden_logic_so_group_.clear();
    peer_need_reply_msg_check_interval_ = 0;
    peer_tcp_conn_interval_sec_ = 0;
    peer_tcp_conn_interval_usec_ = 0;
    peer_http_conn_timeout_ = 0;
    peer_http_conn_max_retry_ = 0;

    if (LoadEnableCPUProfiling() != 0)
    {
        return -1;
    }

    if (LoadEnableMemProfiling() != 0)
    {
        return -1;
    }

    if (LoadReleaseFreeMem() != 0)
    {
        return -1;
    }

    if (LoadTCPAddrPort() != 0)
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

    if (LoadTCPThreadCount() != 0)
    {
        return -1;
    }

    if (LoadTCPLocalLogicSo() != 0)
    {
        return -1;
    }

    if (LoadTCPLogicSoGroup() != 0)
    {
        return -1;
    }

    if (LoadHTTPAddrPort() != 0)
    {
        return -1;
    }

    if (LoadHTTPsAddrPort() != 0)
    {
        return -1;
    }

    if (LoadHTTPsCertificateChainFilePath() != 0)
    {
        return -1;
    }

    if (LoadHTTPsPrivateKeyFilePath() != 0)
    {
        return -1;
    }

    if (LoadHTTPConnCountLimit() != 0)
    {
        return -1;
    }

    if (LoadHTTPMaxHeaderSize() != 0)
    {
        return -1;
    }

    if (LoadHTTPMaxBodySize() != 0)
    {
        return -1;
    }

    if (LoadHTTPConnTimeoutSec() != 0)
    {
        return -1;
    }

    if (LoadHTTPConnTimeoutUsec() != 0)
    {
        return -1;
    }

    if (LoadHTTPThreadCount() != 0)
    {
        return -1;
    }

    if (LoadHTTPLocalLogicSo() != 0)
    {
        return -1;
    }

    if (LoadHTTPLogicSoGroup() != 0)
    {
        return -1;
    }

    if (LoadHTTPPrintParsedInfo() != 0)
    {
        return -1;
    }

    if (LoadHTTPDecodeUri() != 0)
    {
        return -1;
    }

    if (LoadHTTPContentType() != 0)
    {
        return -1;
    }

    if (LoadHTTPNoCache() != 0)
    {
        return -1;
    }

    if (LoadHTTPFlashCrossDomainPath() != 0)
    {
        return -1;
    }

    if (LoadHTTPFileUploadPath() != 0)
    {
        return -1;
    }

    if (LoadHTTPFileDownloadPath() != 0)
    {
        return -1;
    }

    if (LoadHTTPFileStorageDir() != 0)
    {
        return -1;
    }

    if (LoadUDPAddrPort() != 0)
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

    if (LoadUDPThreadCount() != 0)
    {
        return -1;
    }

    if (LoadUDPLocalLogicSo() != 0)
    {
        return -1;
    }

    if (LoadUDPLogicSoGroup() != 0)
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

    if (LoadWorkLocalLogicSo() != 0)
    {
        return -1;
    }

    if (LoadWorkLogicSoGroup() != 0)
    {
        return -1;
    }

    if (LoadIOToWorkTQSizeLimit() != 0)
    {
        return -1;
    }

    if (LoadBurdenThreadCount() != 0)
    {
        return -1;
    }

    if (LoadBurdenLocalLogicSo() != 0)
    {
        return -1;
    }

    if (LoadBurdenLogicSoGroup() != 0)
    {
        return -1;
    }

    if (LoadPeerNeedReplyMsgCheckInterval() != 0)
    {
        return -1;
    }

    if (LoadPeerTCPConnIntervalSec() != 0)
    {
        return -1;
    }

    if (LoadPeerTCPConnIntervalUsec() != 0)
    {
        return -1;
    }

    if (LoadPeerHTTPConnTimeout() != 0)
    {
        return -1;
    }

    if (LoadPeerHTTPConnMaxRetry() != 0)
    {
        return -1;
    }

    return 0;
}
}
