#include "conf_mgr.h"

namespace app_launcher
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

    conf_check_interval_ = 0;

    if (LoadConfCheckInterval() != 0)
    {
        return -1;
    }

    return 0;
}
}
