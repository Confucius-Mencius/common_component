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
    int Load() override;

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

    std::string GetGlobalCommonLogicSo() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return global_common_logic_so_;
    }

    StrGroup GetGlobalLogicSoGroup() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return global_logic_so_group_;
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

    int GetTCPStormThreshold() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return tcp_storm_threshold_;
    }

    int GetTCPThreadCount() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return tcp_thread_count_;
    }

    std::string GetTCPCommonLogicSo() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return tcp_common_logic_so_;
    }

    StrGroup GetTCPLogicSoGroup() override
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return tcp_logic_so_group_;
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

    int LoadGlobalCommonLogicSo()
    {
        char* global_common_logic_so = NULL;
        if (conf_center_->GetConf(&global_common_logic_so, GLOBAL_COMMON_LOGIC_SO_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << GLOBAL_COMMON_LOGIC_SO_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&global_common_logic_so);
            return -1;
        }
        global_common_logic_so_ = global_common_logic_so;
        conf_center_->ReleaseConf(&global_common_logic_so);
        return 0;
    }

    int LoadGlobalLogicSoGroup()
    {
        char** global_logic_so = NULL;
        int n = 0;
        if (conf_center_->GetConf(&global_logic_so, n, GLOBAL_LOGIC_SO_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << GLOBAL_LOGIC_SO_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&global_logic_so, n);
            return -1;
        }
        for (int i = 0; i < n; ++i)
        {
            if (strlen(global_logic_so[i]) > 0)
            {
                global_logic_so_group_.push_back(global_logic_so[i]);
            }
        }
        conf_center_->ReleaseConf(&global_logic_so, n);
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

    int LoadTCPStormThreshold()
    {
        if (conf_center_->GetConf(tcp_storm_threshold_, TCP_STORM_THRESHOLD_XPATH, true, 1000) != 0)
        {
            LOG_ERROR("failed to get " << TCP_STORM_THRESHOLD_XPATH << ": " << conf_center_->GetLastErrMsg());
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

    int LoadTCPCommonLogicSo()
    {
        char* tcp_common_logic_so = NULL;
        if (conf_center_->GetConf(&tcp_common_logic_so, TCP_COMMON_LOGIC_SO_XPATH, true, "") != 0)
        {
            LOG_ERROR("failed to get " << TCP_COMMON_LOGIC_SO_XPATH << ": " << conf_center_->GetLastErrMsg());
            conf_center_->ReleaseConf(&tcp_common_logic_so);
            return -1;
        }
        tcp_common_logic_so_ = tcp_common_logic_so;
        conf_center_->ReleaseConf(&tcp_common_logic_so);
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

private:
    ThreadRWLock rwlock_;
    bool enable_cpu_profiling_;
    bool enable_mem_profiling_;
    bool release_free_mem_;
    std::string global_common_logic_so_;
    StrGroup global_logic_so_group_;
    std::string tcp_addr_port_;
    int tcp_conn_count_limit_;
    int tcp_inactive_conn_check_interval_sec_;
    int tcp_inactive_conn_check_interval_usec_;
    int tcp_inactive_conn_life_;
    int tcp_storm_interval_;
    int tcp_storm_threshold_;
    int tcp_thread_count_;
    std::string tcp_common_logic_so_;
    StrGroup tcp_logic_so_group_;
    std::string udp_addr_port_;
    int udp_inactive_conn_check_interval_sec_;
    int udp_inactive_conn_check_interval_usec_;
    int udp_inactive_conn_life_;
    int udp_thread_count_;
    std::string udp_common_logic_so_;
    StrGroup udp_logic_so_group_;
    int work_thread_count_;
    std::string work_common_logic_so_;
    StrGroup work_logic_so_group_;
    int io_to_work_tq_size_limit_;
    int burden_thread_count_;
    std::string burden_common_logic_so_;
    StrGroup burden_logic_so_group_;
};
}

#endif // APP_FRAME_SRC_CONF_MGR_H_
