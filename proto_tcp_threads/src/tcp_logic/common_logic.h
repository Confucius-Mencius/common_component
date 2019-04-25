#ifndef PROTO_TCP_THREADS_SRC_TCP_LOGIC_COMMON_LOGIC_H_
#define PROTO_TCP_THREADS_SRC_TCP_LOGIC_COMMON_LOGIC_H_

#include <vector>
#include "module_loader.h"
#include "msg_dispatcher.h"
#include "part_msg_mgr.h"
#include "proto_logic_args.h"
#include "proto_msg_codec.h"
#include "proto_tcp_logic_interface.h"
#include "scheduler.h"
#include "tcp_logic_interface.h"

namespace tcp
{
struct ProtoLogicItem
{
    std::string logic_so_path;
    ModuleLoader logic_loader;
    tcp::proto::LogicInterface* logic;

    ProtoLogicItem() : logic_so_path(), logic_loader()
    {
        logic = nullptr;
    }
};

typedef std::vector<ProtoLogicItem> ProtoLogicItemVec;

class ProtoLogic : public LogicInterface, public TimerSinkInterface
{
public:
    ProtoLogic();
    virtual ~ProtoLogic();

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
    void OnClientMsg(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                     const void* msg_body, size_t msg_body_len);

private:
    ProtoLogicArgs proto_logic_args_;

    ModuleLoader proto_tcp_common_logic_loader_;
    tcp::proto::CommonLogicInterface* proto_tcp_common_logic_;
    ProtoLogicItemVec proto_tcp_logic_item_vec_;

    ::proto::MsgCodec msg_codec_;
    tcp::proto::Scheduler scheduler_;
    tcp::proto::MsgDispatcher msg_dispatcher_;

    tcp::proto::PartMsgMgr part_msg_mgr_;
};
}

#endif // PROTO_TCP_THREADS_SRC_CP_COMMON_LOGIC_COMMON_LOGIC_H_
