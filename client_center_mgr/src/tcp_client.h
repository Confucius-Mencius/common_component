#ifndef CLIENT_CENTER_MGR_SRC_TCP_CLIENT_H_
#define CLIENT_CENTER_MGR_SRC_TCP_CLIENT_H_

#include <list>
#include <map>
#include <set>
#include <event2/bufferevent.h>
#include "mem_util.h"
#include "msg_codec_interface.h"
#include "tcp_client_center_interface.h"
#include "timer_axis_interface.h"
#include "trans_center_interface.h"

namespace tcp
{
class ClientCenter;

class Client
    : public ModuleInterface, public ClientInterface, public TimerSinkInterface, public TransCenterSinkInterface
{
    CREATE_FUNC(Client);

    static void EventCallback(struct bufferevent* buf_event, short events, void* arg);
    static void ReadCallback(struct bufferevent* buf_event, void* arg);

public:
    Client();
    virtual ~Client();

    ///////////////////////// ModuleInterface /////////////////////////
    const char* GetVersion() const override;
    const char* GetLastErrMsg() const override;
    void Release() override;
    int Initialize(const void* ctx) override;
    void Finalize() override;
    int Activate() override;
    void Freeze() override;

    ///////////////////////// ClientInterface /////////////////////////
    const Peer& GetPeer() const override
    {
        return peer_;
    }

    int AddNfySink(NfySinkInterface* sink) override;
    void RemoveNfySink(NfySinkInterface* sink) override;

    bool IsConnected() const override
    {
        return connected_;
    }

    TransId Send(const MsgHead& msg_head, const void* msg_body, size_t msg_body_len,
                 const base::AsyncCtx* async_ctx) override;

    void Close() override;

    ///////////////////////// TimerSinkInterface /////////////////////////
    void OnTimer(TimerID timer_id, const void* async_data, size_t async_data_len, int times) override;

    ///////////////////////// TransCenterSinkInterface /////////////////////////
    void OnTimeout(TransId trans_id) override;

public:
    void SetClientCenter(ClientCenter* client_center)
    {
        client_center_ = client_center;
    }

    void SetPeer(const Peer& peer)
    {
        peer_ = peer;
    }

    void SetConnected(bool flag, struct bufferevent* buf_event);

    char* GetRecvBuf() const
    {
        return recv_buf_;
    }

private:
    struct CacheMsg;

    int StartConnectTimer();
    void StopConnectTimer();
    int Connect();
    int InnerSend(const MsgHead& msg_head, const void* data, size_t data_len, int total_retry);
    int AddCacheMsg(TransId trans_id, const CacheMsg& cache_msg);
    void SendCacheMsg();

private:
    ClientCenter* client_center_;
    const ClientCenterCtx* client_center_ctx_;

    char* send_buf_;
    char* recv_buf_;

    Peer peer_;
    bool connected_;

    typedef std::set<struct bufferevent*> BufeventSet;
    BufeventSet bufevent_set_;

    struct bufferevent* buf_event_; // 连接上对端之后的buf event

    // 未连接时发消息，先把消息缓存起来
    struct CacheMsg
    {
        MsgHead msg_head;
        std::string msg_data;
        int total_retry;

        CacheMsg() : msg_head(), msg_data("")
        {
            total_retry = 0;
        }
    };

    typedef std::map<TransId, CacheMsg> CacheMsgMap;
    CacheMsgMap cache_msg_map_;

    typedef std::list<TransId> TransIdList;
    TransIdList cache_msg_trans_id_list_; // 按顺序排列

    typedef std::set<NfySinkInterface*> NfySinkSet;
    NfySinkSet nfy_sink_set_;

    int reconnect_failed_count_;
};
}

#endif // CLIENT_CENTER_MGR_SRC_TCP_CLIENT_H_
