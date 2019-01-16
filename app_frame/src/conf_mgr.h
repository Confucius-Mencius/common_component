#ifndef APP_FRAME_SRC_CONF_MGR_H_
#define APP_FRAME_SRC_CONF_MGR_H_

#include <string.h>
#include <vector>
#include "app_frame_conf_mgr_interface.h"
#include "conf_center_interface.h"
#include "conf_mgr_interface.h"
#include "conf_xpath_define.h"
#include "log_util.h"
#include "thread_lock.h"

// 注意：多个so中的conf_mgr文件名可以相同，类名不能相同，可以通过加namespace来解决。

namespace app_frame
{
class ConfMgr : public base::ConfMgrInterface, public ConfMgrInterface
{
public:
    ConfMgr();
    virtual ~ConfMgr();

private:
    ///////////////////////// base::ConfMgrInterface /////////////////////////
    virtual int Load();

public:
    ///////////////////////// ConfMgrInterface /////////////////////////
    bool EnableCPUProfiling() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return enable_cpu_profiling_;
    }

    bool EnableMemProfiling() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return enable_mem_profiling_;
    }

    bool ReleaseFreeMem() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return release_free_mem_;
    }

    std::string GetGlobalLogicSo() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return global_logic_so_;
    }

    std::string GetTCPAddrPort() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return tcp_addr_port_;
    }

    int GetTCPConnCountLimit() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return tcp_conn_count_limit_;
    }

    int GetTCPInactiveConnCheckIntervalSec() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return tcp_inactive_conn_check_interval_sec_;
    }

    int GetTCPInactiveConnCheckIntervalUsec() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return tcp_inactive_conn_check_interval_usec_;
    }

    int GetTCPInactiveConnLife() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return tcp_inactive_conn_life_;
    }

    int GetTCPStormInterval() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return tcp_storm_interval_;
    }

    int GetTCPStormRecvCount() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return tcp_storm_recv_count_;
    }

    int GetTCPThreadCount() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return tcp_thread_count_;
    }

    std::string GetTCPLocalLogicSo() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return tcp_local_logic_so_;
    }

    StrGroup GetTCPLogicSoGroup() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return tcp_logic_so_group_;
    }

    std::string GetHTTPAddrPort() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return http_addr_port_;
    }

    std::string GetHTTPsAddrPort() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return https_addr_port_;
    }

    std::string GetHTTPsCertificateChainFilePath() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return https_certificate_chain_file_path_;
    }

    std::string GetHTTPsPrivateKeyFilePath() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return https_private_key_file_path_;
    }

    int GetHTTPConnCountLimit() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return http_conn_count_limit_;
    }

    int GetHTTPMaxHeaderSize() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return http_max_header_size_;
    }

    int GetHTTPMaxBodySize() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return http_max_body_size_;
    }

    int GetHTTPConnTimeoutSec() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return http_conn_timeout_sec_;
    }

    int GetHTTPConnTimeoutUsec() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return http_conn_timeout_usec_;
    }

    int GetHTTPThreadCount() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return http_thread_count_;
    }

    std::string GetHTTPLocalLogicSo() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return http_local_logic_so_;
    }

    StrGroup GetHTTPLogicSoGroup() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return http_logic_so_group_;
    }

    bool HTTPPrintParsedInfo() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return http_print_parsed_info_;
    }

    bool HTTPDecodeUri() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return http_decode_uri_;
    }

    std::string GetHTTPContentType() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return http_content_type_;
    }

    bool HTTPNoCache() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return http_no_cache_;
    }

    std::string GetHTTPFlashCrossDomainPath() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return http_flash_cross_domain_path_;
    }

    std::string GetHTTPFileUploadPath() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return http_file_upload_path_;
    }

    std::string GetHTTPFileDownloadPath() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return http_file_download_path_;
    }

    std::string GetHTTPFileStorageDir() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return http_file_storage_dir_;
    }

    std::string GetUDPAddrPort() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return udp_addr_port_;
    }

    int GetUDPInactiveConnCheckIntervalSec() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return udp_inactive_conn_check_interval_sec_;
    }

    int GetUDPInactiveConnCheckIntervalUsec() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return udp_inactive_conn_check_interval_usec_;
    }

    int GetUDPInactiveConnLife() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return udp_inactive_conn_life_;
    }

    bool UDPDoChecksum() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return udp_do_checksum_;
    }

    int GetUDPMaxMsgBodyLen() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return udp_max_msg_body_len_;
    }

    int GetUDPThreadCount() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return udp_thread_count_;
    }

    std::string GetUDPLocalLogicSo() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return udp_local_logic_so_;
    }

    StrGroup GetUDPLogicSoGroup() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return udp_logic_so_group_;
    }

    int GetPeerNeedReplyMsgCheckInterval() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return peer_need_reply_msg_check_interval_;
    }

    int GetPeerTCPConnIntervalSec() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return peer_tcp_conn_interval_sec_;
    }

    int GetPeerTCPConnIntervalUsec() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return peer_tcp_conn_interval_usec_;
    }

    int GetPeerHTTPConnTimeout() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return peer_http_conn_timeout_;
    }

    int GetPeerHTTPConnMaxRetry() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return peer_http_conn_max_retry_;
    }

    int GetWorkThreadCount() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return work_thread_count_;
    }

    std::string GetWorkLocalLogicSo() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return work_local_logic_so_;
    }

    StrGroup GetWorkLogicSoGroup() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return work_logic_so_group_;
    }

    int GetIOToWorkTQSizeLimit() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return io_to_work_tq_size_limit_;
    }

    int GetBurdenThreadCount() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return burden_thread_count_;
    }

    std::string GetBurdenLocalLogicSo() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return burden_local_logic_so_;
    }

    StrGroup GetBurdenLogicSoGroup() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return burden_logic_so_group_;
    }

private:
    int LoadEnableCPUProfiling()
    {
        int enable_cpu_profiling = 0;
        if (conf_center_->GetConf(enable_cpu_profiling, ENABLE_CPU_PROFILING_XPATH, true, 0) != 0)
        {
            LOG_ERROR("failed to get " << ENABLE_CPU_PROFILING_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        enable_cpu_profiling_ = (enable_cpu_profiling != 0);
        return 0;
    }

    int LoadEnableMemProfiling()
    {
        int enable_mem_profiling = 0;
        if (conf_center_->GetConf(enable_mem_profiling, ENABLE_MEM_PROFILING_XPATH, true, 0) != 0)
        {
            LOG_ERROR("failed to get " << ENABLE_MEM_PROFILING_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        enable_mem_profiling_ = (enable_mem_profiling != 0);
        return 0;
    }

    int LoadReleaseFreeMem()
    {
        int release_free_mem = 0;
        if (conf_center_->GetConf(release_free_mem, RELEASE_FREE_MEM_XPATH, true, 0) != 0)
        {
            LOG_ERROR("failed to get " << RELEASE_FREE_MEM_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        release_free_mem_ = (release_free_mem != 0);
        return 0;
    }

    int LoadGlobalLogicSo()
    {
        char* global_logic_so = NULL;
        if (conf_center_->GetConf(&global_logic_so, GLOBAL_LOGIC_SO_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << GLOBAL_LOGIC_SO_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&global_logic_so);
            return -1;
        }
        global_logic_so_ = global_logic_so;
        conf_center_->ReleaseConf(&global_logic_so);
        return 0;
    }

    int LoadTCPAddrPort()
    {
        char* tcp_addr_port = NULL;
        if (conf_center_->GetConf(&tcp_addr_port, TCP_ADDR_PORT_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << TCP_ADDR_PORT_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&tcp_addr_port);
            return -1;
        }
        tcp_addr_port_ = tcp_addr_port;
        conf_center_->ReleaseConf(&tcp_addr_port);
        return 0;
    }

    int LoadTCPConnCountLimit()
    {
        if (conf_center_->GetConf(tcp_conn_count_limit_, TCP_CONN_COUNT_LIMIT_XPATH, true, 0) != 0)
        {
            LOG_ERROR("failed to get " << TCP_CONN_COUNT_LIMIT_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadTCPInactiveConnCheckIntervalSec()
    {
        if (conf_center_->GetConf(tcp_inactive_conn_check_interval_sec_, TCP_INACTIVE_CONN_CHECK_INTERVAL_SEC_XPATH, true, 60) != 0)
        {
            LOG_ERROR("failed to get " << TCP_INACTIVE_CONN_CHECK_INTERVAL_SEC_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadTCPInactiveConnCheckIntervalUsec()
    {
        if (conf_center_->GetConf(tcp_inactive_conn_check_interval_usec_, TCP_INACTIVE_CONN_CHECK_INTERVAL_USEC_XPATH, true, 0) != 0)
        {
            LOG_ERROR("failed to get " << TCP_INACTIVE_CONN_CHECK_INTERVAL_USEC_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadTCPInactiveConnLife()
    {
        if (conf_center_->GetConf(tcp_inactive_conn_life_, TCP_INACTIVE_CONN_LIFE_XPATH, true, 1800) != 0)
        {
            LOG_ERROR("failed to get " << TCP_INACTIVE_CONN_LIFE_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadTCPStormInterval()
    {
        if (conf_center_->GetConf(tcp_storm_interval_, TCP_STORM_INTERVAL_XPATH, true, 10) != 0)
        {
            LOG_ERROR("failed to get " << TCP_STORM_INTERVAL_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadTCPStormRecvCount()
    {
        if (conf_center_->GetConf(tcp_storm_recv_count_, TCP_STORM_RECV_COUNT_XPATH, true, 1000) != 0)
        {
            LOG_ERROR("failed to get " << TCP_STORM_RECV_COUNT_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadTCPThreadCount()
    {
        if (conf_center_->GetConf(tcp_thread_count_, TCP_THREAD_COUNT_XPATH, true, 0) != 0)
        {
            LOG_ERROR("failed to get " << TCP_THREAD_COUNT_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadTCPLocalLogicSo()
    {
        char* tcp_local_logic_so = NULL;
        if (conf_center_->GetConf(&tcp_local_logic_so, TCP_LOCAL_LOGIC_SO_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << TCP_LOCAL_LOGIC_SO_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&tcp_local_logic_so);
            return -1;
        }
        tcp_local_logic_so_ = tcp_local_logic_so;
        conf_center_->ReleaseConf(&tcp_local_logic_so);
        return 0;
    }

    int LoadTCPLogicSoGroup()
    {
        char** tcp_logic_so = NULL;
        int n = 0;
        if (conf_center_->GetConf(&tcp_logic_so, n, TCP_LOGIC_SO_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << TCP_LOGIC_SO_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&tcp_logic_so, n);
            return -1;
        }
        for (int i = 0; i < n; ++i)
        {
            if (strlen(tcp_logic_so[i]) > 0)
            {
                tcp_logic_so_group_.push_back(tcp_logic_so[i]);
            }
        }
        conf_center_->ReleaseConf(&tcp_logic_so, n);
        return 0;
    }

    int LoadHTTPAddrPort()
    {
        char* http_addr_port = NULL;
        if (conf_center_->GetConf(&http_addr_port, HTTP_ADDR_PORT_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << HTTP_ADDR_PORT_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&http_addr_port);
            return -1;
        }
        http_addr_port_ = http_addr_port;
        conf_center_->ReleaseConf(&http_addr_port);
        return 0;
    }

    int LoadHTTPsAddrPort()
    {
        char* https_addr_port = NULL;
        if (conf_center_->GetConf(&https_addr_port, HTTPS_ADDR_PORT_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << HTTPS_ADDR_PORT_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&https_addr_port);
            return -1;
        }
        https_addr_port_ = https_addr_port;
        conf_center_->ReleaseConf(&https_addr_port);
        return 0;
    }

    int LoadHTTPsCertificateChainFilePath()
    {
        char* https_certificate_chain_file_path = NULL;
        if (conf_center_->GetConf(&https_certificate_chain_file_path, HTTPS_CERTIFICATE_CHAIN_FILE_PATH_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << HTTPS_CERTIFICATE_CHAIN_FILE_PATH_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&https_certificate_chain_file_path);
            return -1;
        }
        https_certificate_chain_file_path_ = https_certificate_chain_file_path;
        conf_center_->ReleaseConf(&https_certificate_chain_file_path);
        return 0;
    }

    int LoadHTTPsPrivateKeyFilePath()
    {
        char* https_private_key_file_path = NULL;
        if (conf_center_->GetConf(&https_private_key_file_path, HTTPS_PRIVATE_KEY_FILE_PATH_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << HTTPS_PRIVATE_KEY_FILE_PATH_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&https_private_key_file_path);
            return -1;
        }
        https_private_key_file_path_ = https_private_key_file_path;
        conf_center_->ReleaseConf(&https_private_key_file_path);
        return 0;
    }

    int LoadHTTPConnCountLimit()
    {
        if (conf_center_->GetConf(http_conn_count_limit_, HTTP_CONN_COUNT_LIMIT_XPATH, true, 0) != 0)
        {
            LOG_ERROR("failed to get " << HTTP_CONN_COUNT_LIMIT_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadHTTPMaxHeaderSize()
    {
        if (conf_center_->GetConf(http_max_header_size_, HTTP_MAX_HEADER_SIZE_XPATH, true, 0) != 0)
        {
            LOG_ERROR("failed to get " << HTTP_MAX_HEADER_SIZE_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadHTTPMaxBodySize()
    {
        if (conf_center_->GetConf(http_max_body_size_, HTTP_MAX_BODY_SIZE_XPATH, true, 0) != 0)
        {
            LOG_ERROR("failed to get " << HTTP_MAX_BODY_SIZE_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadHTTPConnTimeoutSec()
    {
        if (conf_center_->GetConf(http_conn_timeout_sec_, HTTP_CONN_TIMEOUT_SEC_XPATH, true, 0) != 0)
        {
            LOG_ERROR("failed to get " << HTTP_CONN_TIMEOUT_SEC_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadHTTPConnTimeoutUsec()
    {
        if (conf_center_->GetConf(http_conn_timeout_usec_, HTTP_CONN_TIMEOUT_USEC_XPATH, true, 0) != 0)
        {
            LOG_ERROR("failed to get " << HTTP_CONN_TIMEOUT_USEC_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadHTTPThreadCount()
    {
        if (conf_center_->GetConf(http_thread_count_, HTTP_THREAD_COUNT_XPATH, true, 0) != 0)
        {
            LOG_ERROR("failed to get " << HTTP_THREAD_COUNT_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadHTTPLocalLogicSo()
    {
        char* http_local_logic_so = NULL;
        if (conf_center_->GetConf(&http_local_logic_so, HTTP_LOCAL_LOGIC_SO_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << HTTP_LOCAL_LOGIC_SO_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&http_local_logic_so);
            return -1;
        }
        http_local_logic_so_ = http_local_logic_so;
        conf_center_->ReleaseConf(&http_local_logic_so);
        return 0;
    }

    int LoadHTTPLogicSoGroup()
    {
        char** http_logic_so = NULL;
        int n = 0;
        if (conf_center_->GetConf(&http_logic_so, n, HTTP_LOGIC_SO_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << HTTP_LOGIC_SO_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&http_logic_so, n);
            return -1;
        }
        for (int i = 0; i < n; ++i)
        {
            if (strlen(http_logic_so[i]) > 0)
            {
                http_logic_so_group_.push_back(http_logic_so[i]);
            }
        }
        conf_center_->ReleaseConf(&http_logic_so, n);
        return 0;
    }

    int LoadHTTPPrintParsedInfo()
    {
        int http_print_parsed_info = 0;
        if (conf_center_->GetConf(http_print_parsed_info, HTTP_PRINT_PARSED_INFO_XPATH, true, 1) != 0)
        {
            LOG_ERROR("failed to get " << HTTP_PRINT_PARSED_INFO_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        http_print_parsed_info_ = (http_print_parsed_info != 0);
        return 0;
    }

    int LoadHTTPDecodeUri()
    {
        int http_decode_uri = 0;
        if (conf_center_->GetConf(http_decode_uri, HTTP_DECODE_URI_XPATH, true, 0) != 0)
        {
            LOG_ERROR("failed to get " << HTTP_DECODE_URI_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        http_decode_uri_ = (http_decode_uri != 0);
        return 0;
    }

    int LoadHTTPContentType()
    {
        char* http_content_type = NULL;
        if (conf_center_->GetConf(&http_content_type, HTTP_CONTENT_TYPE_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << HTTP_CONTENT_TYPE_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&http_content_type);
            return -1;
        }
        http_content_type_ = http_content_type;
        conf_center_->ReleaseConf(&http_content_type);
        return 0;
    }

    int LoadHTTPNoCache()
    {
        int http_no_cache = 0;
        if (conf_center_->GetConf(http_no_cache, HTTP_NO_CACHE_XPATH, true, 1) != 0)
        {
            LOG_ERROR("failed to get " << HTTP_NO_CACHE_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        http_no_cache_ = (http_no_cache != 0);
        return 0;
    }

    int LoadHTTPFlashCrossDomainPath()
    {
        char* http_flash_cross_domain_path = NULL;
        if (conf_center_->GetConf(&http_flash_cross_domain_path, HTTP_FLASH_CROSS_DOMAIN_PATH_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << HTTP_FLASH_CROSS_DOMAIN_PATH_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&http_flash_cross_domain_path);
            return -1;
        }
        http_flash_cross_domain_path_ = http_flash_cross_domain_path;
        conf_center_->ReleaseConf(&http_flash_cross_domain_path);
        return 0;
    }

    int LoadHTTPFileUploadPath()
    {
        char* http_file_upload_path = NULL;
        if (conf_center_->GetConf(&http_file_upload_path, HTTP_FILE_UPLOAD_PATH_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << HTTP_FILE_UPLOAD_PATH_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&http_file_upload_path);
            return -1;
        }
        http_file_upload_path_ = http_file_upload_path;
        conf_center_->ReleaseConf(&http_file_upload_path);
        return 0;
    }

    int LoadHTTPFileDownloadPath()
    {
        char* http_file_download_path = NULL;
        if (conf_center_->GetConf(&http_file_download_path, HTTP_FILE_DOWNLOAD_PATH_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << HTTP_FILE_DOWNLOAD_PATH_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&http_file_download_path);
            return -1;
        }
        http_file_download_path_ = http_file_download_path;
        conf_center_->ReleaseConf(&http_file_download_path);
        return 0;
    }

    int LoadHTTPFileStorageDir()
    {
        char* http_file_storage_dir = NULL;
        if (conf_center_->GetConf(&http_file_storage_dir, HTTP_FILE_STORAGE_DIR_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << HTTP_FILE_STORAGE_DIR_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&http_file_storage_dir);
            return -1;
        }
        http_file_storage_dir_ = http_file_storage_dir;
        conf_center_->ReleaseConf(&http_file_storage_dir);
        return 0;
    }

    int LoadUDPAddrPort()
    {
        char* udp_addr_port = NULL;
        if (conf_center_->GetConf(&udp_addr_port, UDP_ADDR_PORT_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << UDP_ADDR_PORT_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&udp_addr_port);
            return -1;
        }
        udp_addr_port_ = udp_addr_port;
        conf_center_->ReleaseConf(&udp_addr_port);
        return 0;
    }

    int LoadUDPInactiveConnCheckIntervalSec()
    {
        if (conf_center_->GetConf(udp_inactive_conn_check_interval_sec_, UDP_INACTIVE_CONN_CHECK_INTERVAL_SEC_XPATH, true, 60) != 0)
        {
            LOG_ERROR("failed to get " << UDP_INACTIVE_CONN_CHECK_INTERVAL_SEC_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadUDPInactiveConnCheckIntervalUsec()
    {
        if (conf_center_->GetConf(udp_inactive_conn_check_interval_usec_, UDP_INACTIVE_CONN_CHECK_INTERVAL_USEC_XPATH, true, 0) != 0)
        {
            LOG_ERROR("failed to get " << UDP_INACTIVE_CONN_CHECK_INTERVAL_USEC_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadUDPInactiveConnLife()
    {
        if (conf_center_->GetConf(udp_inactive_conn_life_, UDP_INACTIVE_CONN_LIFE_XPATH, true, 1800) != 0)
        {
            LOG_ERROR("failed to get " << UDP_INACTIVE_CONN_LIFE_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadUDPDoChecksum()
    {
        int udp_do_checksum = 0;
        if (conf_center_->GetConf(udp_do_checksum, UDP_DO_CHECKSUM_XPATH, true, 1) != 0)
        {
            LOG_ERROR("failed to get " << UDP_DO_CHECKSUM_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        udp_do_checksum_ = (udp_do_checksum != 0);
        return 0;
    }

    int LoadUDPMaxMsgBodyLen()
    {
        if (conf_center_->GetConf(udp_max_msg_body_len_, UDP_MAX_MSG_BODY_LEN_XPATH, true, 65408) != 0)
        {
            LOG_ERROR("failed to get " << UDP_MAX_MSG_BODY_LEN_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadUDPThreadCount()
    {
        if (conf_center_->GetConf(udp_thread_count_, UDP_THREAD_COUNT_XPATH, true, 0) != 0)
        {
            LOG_ERROR("failed to get " << UDP_THREAD_COUNT_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadUDPLocalLogicSo()
    {
        char* udp_local_logic_so = NULL;
        if (conf_center_->GetConf(&udp_local_logic_so, UDP_LOCAL_LOGIC_SO_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << UDP_LOCAL_LOGIC_SO_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&udp_local_logic_so);
            return -1;
        }
        udp_local_logic_so_ = udp_local_logic_so;
        conf_center_->ReleaseConf(&udp_local_logic_so);
        return 0;
    }

    int LoadUDPLogicSoGroup()
    {
        char** udp_logic_so = NULL;
        int n = 0;
        if (conf_center_->GetConf(&udp_logic_so, n, UDP_LOGIC_SO_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << UDP_LOGIC_SO_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&udp_logic_so, n);
            return -1;
        }
        for (int i = 0; i < n; ++i)
        {
            if (strlen(udp_logic_so[i]) > 0)
            {
                udp_logic_so_group_.push_back(udp_logic_so[i]);
            }
        }
        conf_center_->ReleaseConf(&udp_logic_so, n);
        return 0;
    }

    int LoadPeerNeedReplyMsgCheckInterval()
    {
        if (conf_center_->GetConf(peer_need_reply_msg_check_interval_, PEER_NEED_REPLY_MSG_CHECK_INTERVAL_XPATH, true, 1) != 0)
        {
            LOG_ERROR("failed to get " << PEER_NEED_REPLY_MSG_CHECK_INTERVAL_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadPeerTCPConnIntervalSec()
    {
        if (conf_center_->GetConf(peer_tcp_conn_interval_sec_, PEER_TCP_CONN_INTERVAL_SEC_XPATH, true, 1) != 0)
        {
            LOG_ERROR("failed to get " << PEER_TCP_CONN_INTERVAL_SEC_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadPeerTCPConnIntervalUsec()
    {
        if (conf_center_->GetConf(peer_tcp_conn_interval_usec_, PEER_TCP_CONN_INTERVAL_USEC_XPATH, true, 0) != 0)
        {
            LOG_ERROR("failed to get " << PEER_TCP_CONN_INTERVAL_USEC_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadPeerHTTPConnTimeout()
    {
        if (conf_center_->GetConf(peer_http_conn_timeout_, PEER_HTTP_CONN_TIMEOUT_XPATH, true, 0) != 0)
        {
            LOG_ERROR("failed to get " << PEER_HTTP_CONN_TIMEOUT_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadPeerHTTPConnMaxRetry()
    {
        if (conf_center_->GetConf(peer_http_conn_max_retry_, PEER_HTTP_CONN_MAX_RETRY_XPATH, true, 0) != 0)
        {
            LOG_ERROR("failed to get " << PEER_HTTP_CONN_MAX_RETRY_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadWorkThreadCount()
    {
        if (conf_center_->GetConf(work_thread_count_, WORK_THREAD_COUNT_XPATH, true, 0) != 0)
        {
            LOG_ERROR("failed to get " << WORK_THREAD_COUNT_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadWorkLocalLogicSo()
    {
        char* work_local_logic_so = NULL;
        if (conf_center_->GetConf(&work_local_logic_so, WORK_LOCAL_LOGIC_SO_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << WORK_LOCAL_LOGIC_SO_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&work_local_logic_so);
            return -1;
        }
        work_local_logic_so_ = work_local_logic_so;
        conf_center_->ReleaseConf(&work_local_logic_so);
        return 0;
    }

    int LoadWorkLogicSoGroup()
    {
        char** work_logic_so = NULL;
        int n = 0;
        if (conf_center_->GetConf(&work_logic_so, n, WORK_LOGIC_SO_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << WORK_LOGIC_SO_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&work_logic_so, n);
            return -1;
        }
        for (int i = 0; i < n; ++i)
        {
            if (strlen(work_logic_so[i]) > 0)
            {
                work_logic_so_group_.push_back(work_logic_so[i]);
            }
        }
        conf_center_->ReleaseConf(&work_logic_so, n);
        return 0;
    }

    int LoadIOToWorkTQSizeLimit()
    {
        if (conf_center_->GetConf(io_to_work_tq_size_limit_, IO_TO_WORK_TQ_SIZE_LIMIT_XPATH, true, 50000) != 0)
        {
            LOG_ERROR("failed to get " << IO_TO_WORK_TQ_SIZE_LIMIT_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadBurdenThreadCount()
    {
        if (conf_center_->GetConf(burden_thread_count_, BURDEN_THREAD_COUNT_XPATH, true, 0) != 0)
        {
            LOG_ERROR("failed to get " << BURDEN_THREAD_COUNT_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadBurdenLocalLogicSo()
    {
        char* burden_local_logic_so = NULL;
        if (conf_center_->GetConf(&burden_local_logic_so, BURDEN_LOCAL_LOGIC_SO_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << BURDEN_LOCAL_LOGIC_SO_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&burden_local_logic_so);
            return -1;
        }
        burden_local_logic_so_ = burden_local_logic_so;
        conf_center_->ReleaseConf(&burden_local_logic_so);
        return 0;
    }

    int LoadBurdenLogicSoGroup()
    {
        char** burden_logic_so = NULL;
        int n = 0;
        if (conf_center_->GetConf(&burden_logic_so, n, BURDEN_LOGIC_SO_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << BURDEN_LOGIC_SO_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&burden_logic_so, n);
            return -1;
        }
        for (int i = 0; i < n; ++i)
        {
            if (strlen(burden_logic_so[i]) > 0)
            {
                burden_logic_so_group_.push_back(burden_logic_so[i]);
            }
        }
        conf_center_->ReleaseConf(&burden_logic_so, n);
        return 0;
    }

private:
    ThreadRWLock rwlock_;
    bool enable_cpu_profiling_;
    bool enable_mem_profiling_;
    bool release_free_mem_;
    std::string global_logic_so_;
    std::string tcp_addr_port_;
    int tcp_conn_count_limit_;
    int tcp_inactive_conn_check_interval_sec_;
    int tcp_inactive_conn_check_interval_usec_;
    int tcp_inactive_conn_life_;
    int tcp_storm_interval_;
    int tcp_storm_recv_count_;
    int tcp_thread_count_;
    std::string tcp_local_logic_so_;
    StrGroup tcp_logic_so_group_;
    std::string http_addr_port_;
    std::string https_addr_port_;
    std::string https_certificate_chain_file_path_;
    std::string https_private_key_file_path_;
    int http_conn_count_limit_;
    int http_max_header_size_;
    int http_max_body_size_;
    int http_conn_timeout_sec_;
    int http_conn_timeout_usec_;
    int http_thread_count_;
    std::string http_local_logic_so_;
    StrGroup http_logic_so_group_;
    bool http_print_parsed_info_;
    bool http_decode_uri_;
    std::string http_content_type_;
    bool http_no_cache_;
    std::string http_flash_cross_domain_path_;
    std::string http_file_upload_path_;
    std::string http_file_download_path_;
    std::string http_file_storage_dir_;
    std::string udp_addr_port_;
    int udp_inactive_conn_check_interval_sec_;
    int udp_inactive_conn_check_interval_usec_;
    int udp_inactive_conn_life_;
    bool udp_do_checksum_;
    int udp_max_msg_body_len_;
    int udp_thread_count_;
    std::string udp_local_logic_so_;
    StrGroup udp_logic_so_group_;
    int peer_need_reply_msg_check_interval_;
    int peer_tcp_conn_interval_sec_;
    int peer_tcp_conn_interval_usec_;
    int peer_http_conn_timeout_;
    int peer_http_conn_max_retry_;
    int work_thread_count_;
    std::string work_local_logic_so_;
    StrGroup work_logic_so_group_;
    int io_to_work_tq_size_limit_;
    int burden_thread_count_;
    std::string burden_local_logic_so_;
    StrGroup burden_logic_so_group_;
};
}

#endif // APP_FRAME_SRC_CONF_MGR_H_
