#ifndef BURDEN_THREADS_SRC_WORK_LOGIC_COMMON_LOGIC_H_
#define BURDEN_THREADS_SRC_WORK_LOGIC_COMMON_LOGIC_H_

#include <vector>
#include "burden_logic_args.h"
#include "burden_logic_interface.h"
#include "module_loader.h"
#include "msg_dispatcher.h"
#include "scheduler.h"
#include "timer_axis_interface.h"
#include "work_logic_interface.h"

namespace work
{
struct BurdenLogicItem
{
    std::string logic_so_path;
    ModuleLoader logic_loader;
    burden::LogicInterface* logic;

    BurdenLogicItem() : logic_so_path(), logic_loader()
    {
        logic = nullptr;
    }
};

typedef std::vector<BurdenLogicItem> BurdenLogicItemVec;

class BurdenLogic : public LogicInterface, public TimerSinkInterface
{
public:
    BurdenLogic();
    virtual ~BurdenLogic();

    ///////////////////////// ModuleInterface /////////////////////////
    const char* GetVersion() const override;
    const char* GetLastErrMsg() const override;
    void Release() override;
    int Initialize(const void* ctx) override;
    void Finalize() override;
    int Activate() override;
    void Freeze() override;

    ///////////////////////// tcp::LogicInterface /////////////////////////
    void OnStop() override;
    void OnReload() override;
    void OnTask(const ConnGUID* conn_guid, ThreadInterface* source_thread, const void* data, size_t len) override;

    ///////////////////////// TimerSinkInterface /////////////////////////
    void OnTimer(TimerID timer_id, void* data, size_t len, int times);

private:
    int LoadBurdenCommonLogic();
    int LoadBurdenLogicGroup();

private:
    BurdenLogicArgs burden_logic_args_;

    ModuleLoader burden_common_logic_loader_;
    burden::CommonLogicInterface* burden_common_logic_;
    BurdenLogicItemVec burden_logic_item_vec_;

    burden::Scheduler scheduler_;
    burden::MsgDispatcher msg_dispatcher_;
};
}

#endif // BURDEN_THREADS_SRC_WORK_LOGIC_COMMON_LOGIC_H_
