#ifndef PROTO_TCP_THREADS_SRC_RAW_TCP_COMMON_LOGIC_COMMON_LOGIC_H_
#define PROTO_TCP_THREADS_SRC_RAW_TCP_COMMON_LOGIC_COMMON_LOGIC_H_

#include <vector>
#include "module_loader.h"
#include "msg_dispatcher.h"
#include "part_msg_mgr.h"
#include "proto_args.h"
#include "proto_msg_codec.h"
#include "proto_tcp_logic_interface.h"
#include "raw_tcp_logic_interface.h"
#include "scheduler.h"

namespace tcp
{
namespace raw
{
struct LogicItem
{
    std::string logic_so_path;
    ModuleLoader logic_loader;
    tcp::proto::LogicInterface* logic;

    LogicItem() : logic_so_path(), logic_loader()
    {
        logic = nullptr;
    }
};

typedef std::vector<LogicItem> LogicItemVec;

class ProtoCommonLogic : public CommonLogicInterface, public TimerSinkInterface
{
public:
    ProtoCommonLogic();
    virtual ~ProtoCommonLogic();

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
    void OnClientConnected(const ConnGUID* conn_guid) override;
    void OnClientClosed(const ConnGUID* conn_guid) override;
    void OnRecvClientData(const ConnGUID* conn_guid, const void* data, size_t len) override;
    void OnTask(const ConnGUID* conn_guid, ThreadInterface* source_thread, const void* data, size_t len) override;

    ///////////////////////// TimerSinkInterface /////////////////////////
    void OnTimer(TimerID timer_id, void* data, size_t len, int times);

private:
    int LoadProtoTCPCommonLogic();
    int LoadProtoTCPLogicGroup();
    void OnRecvClientMsg(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                         const void* msg_body, size_t msg_body_len);

private:
    ProtoArgs proto_tcp_logic_args_;

    ModuleLoader proto_tcp_common_logic_loader_;
    tcp::proto::CommonLogicInterface* proto_tcp_common_logic_;
    LogicItemVec proto_tcp_logic_item_vec_;

    ::proto::MsgCodec msg_codec_;
    tcp::proto::Scheduler scheduler_;
    tcp::proto::MsgDispatcher msg_dispatcher_;
    tcp::proto::PartMsgMgr part_msg_mgr_;
};
}
}

#endif // PROTO_TCP_THREADS_SRC_RAW_TCP_COMMON_LOGIC_COMMON_LOGIC_H_
