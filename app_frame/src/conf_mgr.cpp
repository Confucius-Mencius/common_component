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
    global_logic_so_ = "";
    tcp_addr_port_ = "";
    tcp_conn_count_limit_ = 0;
    tcp_inactive_conn_check_interval_sec_ = 0;
    tcp_inactive_conn_check_interval_usec_ = 0;
    tcp_inactive_conn_life_ = 0;
    tcp_do_checksum_ = false;
    tcp_max_msg_body_len_ = 0;
    tcp_part_msg_check_interval_ = 0;
    tcp_part_msg_conn_life_ = 0;
    tcp_thread_count_ = 0;
    tcp_local_logic_so_ = "";
    tcp_logic_so_group_.clear();
    raw_tcp_addr_port_ = "";
    raw_tcp_conn_count_limit_ = 0;
    raw_tcp_inactive_conn_check_interval_sec_ = 0;
    raw_tcp_inactive_conn_check_interval_usec_ = 0;
    raw_tcp_inactive_conn_life_ = 0;
    raw_tcp_thread_count_ = 0;
    raw_tcp_local_logic_so_ = "";
    raw_tcp_logic_so_group_.clear();
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
    udp_do_checksum_ = false;
    udp_max_msg_body_len_ = 0;
    udp_thread_count_ = 0;
    udp_local_logic_so_ = "";
    udp_logic_so_group_.clear();
    peer_need_reply_msg_check_interval_ = 0;
    peer_tcp_conn_interval_sec_ = 0;
    peer_tcp_conn_interval_usec_ = 0;
    peer_http_conn_timeout_ = 0;
    peer_http_conn_max_retry_ = 0;
    work_thread_count_ = 0;
    work_local_logic_so_ = "";
    work_logic_so_group_.clear();
    io_to_work_tq_size_limit_ = 0;
    burden_thread_count_ = 0;
    burden_local_logic_so_ = "";
    burden_logic_so_group_.clear();

    if (LoadEnableCpuProfiling() != 0)
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

    if (LoadGlobalLogicSo() != 0)
    {
        return -1;
    }

    if (LoadTcpAddrPort() != 0)
    {
        return -1;
    }

    if (LoadTcpConnCountLimit() != 0)
    {
        return -1;
    }

    if (LoadTcpInactiveConnCheckIntervalSec() != 0)
    {
        return -1;
    }

    if (LoadTcpInactiveConnCheckIntervalUsec() != 0)
    {
        return -1;
    }

    if (LoadTcpInactiveConnLife() != 0)
    {
        return -1;
    }

    if (LoadTcpDoChecksum() != 0)
    {
        return -1;
    }

    if (LoadTcpMaxMsgBodyLen() != 0)
    {
        return -1;
    }

    if (LoadTcpPartMsgCheckInterval() != 0)
    {
        return -1;
    }

    if (LoadTcpPartMsgConnLife() != 0)
    {
        return -1;
    }

    if (LoadTcpThreadCount() != 0)
    {
        return -1;
    }

    if (LoadTcpLocalLogicSo() != 0)
    {
        return -1;
    }

    if (LoadTcpLogicSoGroup() != 0)
    {
        return -1;
    }

    if (LoadRawTcpAddrPort() != 0)
    {
        return -1;
    }

    if (LoadRawTcpConnCountLimit() != 0)
    {
        return -1;
    }

    if (LoadRawTcpInactiveConnCheckIntervalSec() != 0)
    {
        return -1;
    }

    if (LoadRawTcpInactiveConnCheckIntervalUsec() != 0)
    {
        return -1;
    }

    if (LoadRawTcpInactiveConnLife() != 0)
    {
        return -1;
    }

    if (LoadRawTcpThreadCount() != 0)
    {
        return -1;
    }

    if (LoadRawTcpLocalLogicSo() != 0)
    {
        return -1;
    }

    if (LoadRawTcpLogicSoGroup() != 0)
    {
        return -1;
    }

    if (LoadHttpAddrPort() != 0)
    {
        return -1;
    }

    if (LoadHttpsAddrPort() != 0)
    {
        return -1;
    }

    if (LoadHttpsCertificateChainFilePath() != 0)
    {
        return -1;
    }

    if (LoadHttpsPrivateKeyFilePath() != 0)
    {
        return -1;
    }

    if (LoadHttpConnCountLimit() != 0)
    {
        return -1;
    }

    if (LoadHttpMaxHeaderSize() != 0)
    {
        return -1;
    }

    if (LoadHttpMaxBodySize() != 0)
    {
        return -1;
    }

    if (LoadHttpConnTimeoutSec() != 0)
    {
        return -1;
    }

    if (LoadHttpConnTimeoutUsec() != 0)
    {
        return -1;
    }

    if (LoadHttpThreadCount() != 0)
    {
        return -1;
    }

    if (LoadHttpLocalLogicSo() != 0)
    {
        return -1;
    }

    if (LoadHttpLogicSoGroup() != 0)
    {
        return -1;
    }

    if (LoadHttpPrintParsedInfo() != 0)
    {
        return -1;
    }

    if (LoadHttpDecodeUri() != 0)
    {
        return -1;
    }

    if (LoadHttpContentType() != 0)
    {
        return -1;
    }

    if (LoadHttpNoCache() != 0)
    {
        return -1;
    }

    if (LoadHttpFlashCrossDomainPath() != 0)
    {
        return -1;
    }

    if (LoadHttpFileUploadPath() != 0)
    {
        return -1;
    }

    if (LoadHttpFileDownloadPath() != 0)
    {
        return -1;
    }

    if (LoadHttpFileStorageDir() != 0)
    {
        return -1;
    }

    if (LoadUdpAddrPort() != 0)
    {
        return -1;
    }

    if (LoadUdpInactiveConnCheckIntervalSec() != 0)
    {
        return -1;
    }

    if (LoadUdpInactiveConnCheckIntervalUsec() != 0)
    {
        return -1;
    }

    if (LoadUdpInactiveConnLife() != 0)
    {
        return -1;
    }

    if (LoadUdpDoChecksum() != 0)
    {
        return -1;
    }

    if (LoadUdpMaxMsgBodyLen() != 0)
    {
        return -1;
    }

    if (LoadUdpThreadCount() != 0)
    {
        return -1;
    }

    if (LoadUdpLocalLogicSo() != 0)
    {
        return -1;
    }

    if (LoadUdpLogicSoGroup() != 0)
    {
        return -1;
    }

    if (LoadPeerNeedReplyMsgCheckInterval() != 0)
    {
        return -1;
    }

    if (LoadPeerTcpConnIntervalSec() != 0)
    {
        return -1;
    }

    if (LoadPeerTcpConnIntervalUsec() != 0)
    {
        return -1;
    }

    if (LoadPeerHttpConnTimeout() != 0)
    {
        return -1;
    }

    if (LoadPeerHttpConnMaxRetry() != 0)
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

    if (LoadIoToWorkTqSizeLimit() != 0)
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

    return 0;
}
}
