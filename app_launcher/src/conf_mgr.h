#ifndef APP_LAUNCHER_SRC_CONF_MGR_H_
#define APP_LAUNCHER_SRC_CONF_MGR_H_

#include "conf_center_interface.h"
#include "conf_mgr_interface.h"
#include "conf_xpath_define.h"
#include "log_util.h"
#include "thread_lock.h"

namespace app_launcher
{
class ConfMgr : public base::ConfMgrInterface
{
public:
    ConfMgr();
    virtual ~ConfMgr();

private:
    ///////////////////////// base::ConfMgrInterface /////////////////////////
    virtual int Load();

public:
    ///////////////////////// ConfMgrInterface /////////////////////////
    int GetConfCheckInterval()
    {
        AUTO_THREAD_RLOCK(rwlock_);
        return conf_check_interval_;
    }

private:
    int LoadConfCheckInterval()
    {
        if (conf_center_->GetConf(conf_check_interval_, CONF_CHECK_INTERVAL_XPATH, true, 0) != 0)
        {
            LOG_ERROR("failed to get " << CONF_CHECK_INTERVAL_XPATH << ": " << conf_center_->GetLastErrMsg());
            return -1;
        }
        return 0;
    }

private:
    ThreadRWLock rwlock_;
    int conf_check_interval_;
};
}

#endif // APP_LAUNCHER_SRC_CONF_MGR_H_
