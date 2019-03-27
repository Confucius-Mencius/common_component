#ifndef APP_FRAME_SRC_CONF_MGR_H_
#define APP_FRAME_SRC_CONF_MGR_H_

#include <string.h>
#include <vector>
#include "app_frame_conf_mgr_interface.h"
#include "conf_center_interface.h"
#include "conf_mgr_interface.h"
#include "conf_xpath.h"
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
    int Load() override;

public:
    ///////////////////////// ConfMgrInterface /////////////////////////
    bool EnableCPUProfiling() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return enable_cpu_profiling_;
    }

    bool EnableHeapProfiling() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return enable_heap_profiling_;
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

    int GetWorkThreadCount() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return work_thread_count_;
    }

    std::string GetWorkCommonLogicSo() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return work_common_logic_so_;
    }

    StrGroup GetWorkLogicSoGroup() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return work_logic_so_group_;
    }

    int GetBurdenThreadCount() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return burden_thread_count_;
    }

    std::string GetBurdenCommonLogicSo() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return burden_common_logic_so_;
    }

    StrGroup GetBurdenLogicSoGroup() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return burden_logic_so_group_;
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

    int GetTCPStormThreshold() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return tcp_storm_threshold_;
    }

    std::string GetRawTCPAddr() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return raw_tcp_addr_;
    }

    int GetRawTCPPort() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return raw_tcp_port_;
    }

    int GetRawTCPThreadCount() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return raw_tcp_thread_count_;
    }

    std::string GetRawTCPCommonLogicSo() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return raw_tcp_common_logic_so_;
    }

    StrGroup GetRawTCPLogicSoGroup() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return raw_tcp_logic_so_group_;
    }

    bool ProtoDoChecksum() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return proto_do_checksum_;
    }

    int GetProtoMaxMsgBodyLen() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return proto_max_msg_body_len_;
    }

    int GetProtoPartMsgCheckInterval() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return proto_part_msg_check_interval_;
    }

    int GetProtoPartMsgConnLife() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return proto_part_msg_conn_life_;
    }

    std::string GetProtoTCPAddr() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return proto_tcp_addr_;
    }

    int GetProtoTCPPort() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return proto_tcp_port_;
    }

    int GetProtoTCPThreadCount() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return proto_tcp_thread_count_;
    }

    std::string GetProtoTCPCommonLogicSo() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return proto_tcp_common_logic_so_;
    }

    StrGroup GetProtoTCPLogicSoGroup() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return proto_tcp_logic_so_group_;
    }

    int GetWSPartMsgCheckInterval() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return ws_part_msg_check_interval_;
    }

    int GetWSPartMsgConnLife() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return ws_part_msg_conn_life_;
    }

    std::string GetWSAddr() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return ws_addr_;
    }

    int GetWSPort() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return ws_port_;
    }

    int GetWSSecurityPort() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return ws_security_port_;
    }

    std::string GetWSCertificateChainFilePath() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return ws_certificate_chain_file_path_;
    }

    std::string GetWSPrivateKeyFilePath() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return ws_private_key_file_path_;
    }

    int GetWSThreadCount() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return ws_thread_count_;
    }

    std::string GetWSCommonLogicSo() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return ws_common_logic_so_;
    }

    StrGroup GetWSLogicSoGroup() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return ws_logic_so_group_;
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

    std::string GetUDPAddr() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return udp_addr_;
    }

    int GetUDPPort() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return udp_port_;
    }

    int GetUDPThreadCount() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return udp_thread_count_;
    }

    std::string GetUDPCommonLogicSo() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return udp_common_logic_so_;
    }

    StrGroup GetUDPLogicSoGroup() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return udp_logic_so_group_;
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

    int LoadEnableHeapProfiling()
    {
        int enable_heap_profiling = 0;
        if (conf_center_->GetConf(enable_heap_profiling, ENABLE_HEAP_PROFILING_XPATH, true, 0) != 0)
        {
            LOG_ERROR("failed to get " << ENABLE_HEAP_PROFILING_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        enable_heap_profiling_ = (enable_heap_profiling != 0);
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

    int LoadWorkThreadCount()
    {
        if (conf_center_->GetConf(work_thread_count_, WORK_THREAD_COUNT_XPATH, true, 0) != 0)
        {
            LOG_ERROR("failed to get " << WORK_THREAD_COUNT_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadWorkCommonLogicSo()
    {
        char* work_common_logic_so = NULL;
        if (conf_center_->GetConf(&work_common_logic_so, WORK_COMMON_LOGIC_SO_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << WORK_COMMON_LOGIC_SO_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&work_common_logic_so);
            return -1;
        }
        work_common_logic_so_ = work_common_logic_so;
        conf_center_->ReleaseConf(&work_common_logic_so);
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

    int LoadBurdenThreadCount()
    {
        if (conf_center_->GetConf(burden_thread_count_, BURDEN_THREAD_COUNT_XPATH, true, 0) != 0)
        {
            LOG_ERROR("failed to get " << BURDEN_THREAD_COUNT_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadBurdenCommonLogicSo()
    {
        char* burden_common_logic_so = NULL;
        if (conf_center_->GetConf(&burden_common_logic_so, BURDEN_COMMON_LOGIC_SO_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << BURDEN_COMMON_LOGIC_SO_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&burden_common_logic_so);
            return -1;
        }
        burden_common_logic_so_ = burden_common_logic_so;
        conf_center_->ReleaseConf(&burden_common_logic_so);
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

    int LoadTCPStormThreshold()
    {
        if (conf_center_->GetConf(tcp_storm_threshold_, TCP_STORM_THRESHOLD_XPATH, true, 1000) != 0)
        {
            LOG_ERROR("failed to get " << TCP_STORM_THRESHOLD_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadRawTCPAddr()
    {
        char* raw_tcp_addr = NULL;
        if (conf_center_->GetConf(&raw_tcp_addr, RAW_TCP_ADDR_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << RAW_TCP_ADDR_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&raw_tcp_addr);
            return -1;
        }
        raw_tcp_addr_ = raw_tcp_addr;
        conf_center_->ReleaseConf(&raw_tcp_addr);
        return 0;
    }

    int LoadRawTCPPort()
    {
        if (conf_center_->GetConf(raw_tcp_port_, RAW_TCP_PORT_XPATH, true, 0) != 0)
        {
            LOG_ERROR("failed to get " << RAW_TCP_PORT_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadRawTCPThreadCount()
    {
        if (conf_center_->GetConf(raw_tcp_thread_count_, RAW_TCP_THREAD_COUNT_XPATH, true, 0) != 0)
        {
            LOG_ERROR("failed to get " << RAW_TCP_THREAD_COUNT_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadRawTCPCommonLogicSo()
    {
        char* raw_tcp_common_logic_so = NULL;
        if (conf_center_->GetConf(&raw_tcp_common_logic_so, RAW_TCP_COMMON_LOGIC_SO_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << RAW_TCP_COMMON_LOGIC_SO_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&raw_tcp_common_logic_so);
            return -1;
        }
        raw_tcp_common_logic_so_ = raw_tcp_common_logic_so;
        conf_center_->ReleaseConf(&raw_tcp_common_logic_so);
        return 0;
    }

    int LoadRawTCPLogicSoGroup()
    {
        char** raw_tcp_logic_so = NULL;
        int n = 0;
        if (conf_center_->GetConf(&raw_tcp_logic_so, n, RAW_TCP_LOGIC_SO_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << RAW_TCP_LOGIC_SO_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&raw_tcp_logic_so, n);
            return -1;
        }
        for (int i = 0; i < n; ++i)
        {
            if (strlen(raw_tcp_logic_so[i]) > 0)
            {
                raw_tcp_logic_so_group_.push_back(raw_tcp_logic_so[i]);
            }
        }
        conf_center_->ReleaseConf(&raw_tcp_logic_so, n);
        return 0;
    }

    int LoadProtoDoChecksum()
    {
        int proto_do_checksum = 0;
        if (conf_center_->GetConf(proto_do_checksum, PROTO_DO_CHECKSUM_XPATH, true, 1) != 0)
        {
            LOG_ERROR("failed to get " << PROTO_DO_CHECKSUM_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        proto_do_checksum_ = (proto_do_checksum != 0);
        return 0;
    }

    int LoadProtoMaxMsgBodyLen()
    {
        if (conf_center_->GetConf(proto_max_msg_body_len_, PROTO_MAX_MSG_BODY_LEN_XPATH, true, 1048576) != 0)
        {
            LOG_ERROR("failed to get " << PROTO_MAX_MSG_BODY_LEN_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadProtoPartMsgCheckInterval()
    {
        if (conf_center_->GetConf(proto_part_msg_check_interval_, PROTO_PART_MSG_CHECK_INTERVAL_XPATH, true, 5) != 0)
        {
            LOG_ERROR("failed to get " << PROTO_PART_MSG_CHECK_INTERVAL_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadProtoPartMsgConnLife()
    {
        if (conf_center_->GetConf(proto_part_msg_conn_life_, PROTO_PART_MSG_CONN_LIFE_XPATH, true, 60) != 0)
        {
            LOG_ERROR("failed to get " << PROTO_PART_MSG_CONN_LIFE_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadProtoTCPAddr()
    {
        char* proto_tcp_addr = NULL;
        if (conf_center_->GetConf(&proto_tcp_addr, PROTO_TCP_ADDR_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << PROTO_TCP_ADDR_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&proto_tcp_addr);
            return -1;
        }
        proto_tcp_addr_ = proto_tcp_addr;
        conf_center_->ReleaseConf(&proto_tcp_addr);
        return 0;
    }

    int LoadProtoTCPPort()
    {
        if (conf_center_->GetConf(proto_tcp_port_, PROTO_TCP_PORT_XPATH, true, 0) != 0)
        {
            LOG_ERROR("failed to get " << PROTO_TCP_PORT_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadProtoTCPThreadCount()
    {
        if (conf_center_->GetConf(proto_tcp_thread_count_, PROTO_TCP_THREAD_COUNT_XPATH, true, 0) != 0)
        {
            LOG_ERROR("failed to get " << PROTO_TCP_THREAD_COUNT_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadProtoTCPCommonLogicSo()
    {
        char* proto_tcp_common_logic_so = NULL;
        if (conf_center_->GetConf(&proto_tcp_common_logic_so, PROTO_TCP_COMMON_LOGIC_SO_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << PROTO_TCP_COMMON_LOGIC_SO_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&proto_tcp_common_logic_so);
            return -1;
        }
        proto_tcp_common_logic_so_ = proto_tcp_common_logic_so;
        conf_center_->ReleaseConf(&proto_tcp_common_logic_so);
        return 0;
    }

    int LoadProtoTCPLogicSoGroup()
    {
        char** proto_tcp_logic_so = NULL;
        int n = 0;
        if (conf_center_->GetConf(&proto_tcp_logic_so, n, PROTO_TCP_LOGIC_SO_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << PROTO_TCP_LOGIC_SO_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&proto_tcp_logic_so, n);
            return -1;
        }
        for (int i = 0; i < n; ++i)
        {
            if (strlen(proto_tcp_logic_so[i]) > 0)
            {
                proto_tcp_logic_so_group_.push_back(proto_tcp_logic_so[i]);
            }
        }
        conf_center_->ReleaseConf(&proto_tcp_logic_so, n);
        return 0;
    }

    int LoadWSPartMsgCheckInterval()
    {
        if (conf_center_->GetConf(ws_part_msg_check_interval_, WS_PART_MSG_CHECK_INTERVAL_XPATH, true, 5) != 0)
        {
            LOG_ERROR("failed to get " << WS_PART_MSG_CHECK_INTERVAL_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadWSPartMsgConnLife()
    {
        if (conf_center_->GetConf(ws_part_msg_conn_life_, WS_PART_MSG_CONN_LIFE_XPATH, true, 60) != 0)
        {
            LOG_ERROR("failed to get " << WS_PART_MSG_CONN_LIFE_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadWSAddr()
    {
        char* ws_addr = NULL;
        if (conf_center_->GetConf(&ws_addr, WS_ADDR_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << WS_ADDR_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&ws_addr);
            return -1;
        }
        ws_addr_ = ws_addr;
        conf_center_->ReleaseConf(&ws_addr);
        return 0;
    }

    int LoadWSPort()
    {
        if (conf_center_->GetConf(ws_port_, WS_PORT_XPATH, true, 0) != 0)
        {
            LOG_ERROR("failed to get " << WS_PORT_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadWSSecurityPort()
    {
        if (conf_center_->GetConf(ws_security_port_, WS_SECURITY_PORT_XPATH, true, 0) != 0)
        {
            LOG_ERROR("failed to get " << WS_SECURITY_PORT_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadWSCertificateChainFilePath()
    {
        char* ws_certificate_chain_file_path = NULL;
        if (conf_center_->GetConf(&ws_certificate_chain_file_path, WS_CERTIFICATE_CHAIN_FILE_PATH_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << WS_CERTIFICATE_CHAIN_FILE_PATH_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&ws_certificate_chain_file_path);
            return -1;
        }
        ws_certificate_chain_file_path_ = ws_certificate_chain_file_path;
        conf_center_->ReleaseConf(&ws_certificate_chain_file_path);
        return 0;
    }

    int LoadWSPrivateKeyFilePath()
    {
        char* ws_private_key_file_path = NULL;
        if (conf_center_->GetConf(&ws_private_key_file_path, WS_PRIVATE_KEY_FILE_PATH_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << WS_PRIVATE_KEY_FILE_PATH_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&ws_private_key_file_path);
            return -1;
        }
        ws_private_key_file_path_ = ws_private_key_file_path;
        conf_center_->ReleaseConf(&ws_private_key_file_path);
        return 0;
    }

    int LoadWSThreadCount()
    {
        if (conf_center_->GetConf(ws_thread_count_, WS_THREAD_COUNT_XPATH, true, 0) != 0)
        {
            LOG_ERROR("failed to get " << WS_THREAD_COUNT_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

    int LoadWSCommonLogicSo()
    {
        char* ws_common_logic_so = NULL;
        if (conf_center_->GetConf(&ws_common_logic_so, WS_COMMON_LOGIC_SO_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << WS_COMMON_LOGIC_SO_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&ws_common_logic_so);
            return -1;
        }
        ws_common_logic_so_ = ws_common_logic_so;
        conf_center_->ReleaseConf(&ws_common_logic_so);
        return 0;
    }

    int LoadWSLogicSoGroup()
    {
        char** ws_logic_so = NULL;
        int n = 0;
        if (conf_center_->GetConf(&ws_logic_so, n, WS_LOGIC_SO_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << WS_LOGIC_SO_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&ws_logic_so, n);
            return -1;
        }
        for (int i = 0; i < n; ++i)
        {
            if (strlen(ws_logic_so[i]) > 0)
            {
                ws_logic_so_group_.push_back(ws_logic_so[i]);
            }
        }
        conf_center_->ReleaseConf(&ws_logic_so, n);
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

    int LoadUDPAddr()
    {
        char* udp_addr = NULL;
        if (conf_center_->GetConf(&udp_addr, UDP_ADDR_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << UDP_ADDR_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&udp_addr);
            return -1;
        }
        udp_addr_ = udp_addr;
        conf_center_->ReleaseConf(&udp_addr);
        return 0;
    }

    int LoadUDPPort()
    {
        if (conf_center_->GetConf(udp_port_, UDP_PORT_XPATH, true, 0) != 0)
        {
            LOG_ERROR("failed to get " << UDP_PORT_XPATH << ": " << conf_center_->GetLastErrMsg());
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

    int LoadUDPCommonLogicSo()
    {
        char* udp_common_logic_so = NULL;
        if (conf_center_->GetConf(&udp_common_logic_so, UDP_COMMON_LOGIC_SO_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << UDP_COMMON_LOGIC_SO_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&udp_common_logic_so);
            return -1;
        }
        udp_common_logic_so_ = udp_common_logic_so;
        conf_center_->ReleaseConf(&udp_common_logic_so);
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

private:
    ThreadRWLock rwlock_;
    bool enable_cpu_profiling_;
    bool enable_heap_profiling_;
    bool release_free_mem_;
    std::string global_logic_so_;
    int work_thread_count_;
    std::string work_common_logic_so_;
    StrGroup work_logic_so_group_;
    int burden_thread_count_;
    std::string burden_common_logic_so_;
    StrGroup burden_logic_so_group_;
    int tcp_conn_count_limit_;
    int tcp_inactive_conn_check_interval_sec_;
    int tcp_inactive_conn_check_interval_usec_;
    int tcp_inactive_conn_life_;
    int tcp_storm_interval_;
    int tcp_storm_threshold_;
    std::string raw_tcp_addr_;
    int raw_tcp_port_;
    int raw_tcp_thread_count_;
    std::string raw_tcp_common_logic_so_;
    StrGroup raw_tcp_logic_so_group_;
    bool proto_do_checksum_;
    int proto_max_msg_body_len_;
    int proto_part_msg_check_interval_;
    int proto_part_msg_conn_life_;
    std::string proto_tcp_addr_;
    int proto_tcp_port_;
    int proto_tcp_thread_count_;
    std::string proto_tcp_common_logic_so_;
    StrGroup proto_tcp_logic_so_group_;
    int ws_part_msg_check_interval_;
    int ws_part_msg_conn_life_;
    std::string ws_addr_;
    int ws_port_;
    int ws_security_port_;
    std::string ws_certificate_chain_file_path_;
    std::string ws_private_key_file_path_;
    int ws_thread_count_;
    std::string ws_common_logic_so_;
    StrGroup ws_logic_so_group_;
    int udp_inactive_conn_check_interval_sec_;
    int udp_inactive_conn_check_interval_usec_;
    int udp_inactive_conn_life_;
    std::string udp_addr_;
    int udp_port_;
    int udp_thread_count_;
    std::string udp_common_logic_so_;
    StrGroup udp_logic_so_group_;
};
}

#endif // APP_FRAME_SRC_CONF_MGR_H_
