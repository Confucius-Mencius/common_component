#ifndef TCP_THREADS_SRC_TCP_CONN_CENTER_H_
#define TCP_THREADS_SRC_TCP_CONN_CENTER_H_

#include <set>
#include "record_timeout_mgr.h"

namespace tcp
{
class ConnMgr : public RecordTimeoutMgr<ConnID, std::hash<ConnID>, ConnInterface*>
{
public:
    ConnMgr();
    virtual ~ConnMgr();

    void Clear(ConnInterface* conn);

    ///////////////////////// ModuleInterface /////////////////////////
    const char* GetVersion() const override;
    const char* GetLastErrMsg() const override;
    void Release() override;
    int Initialize(const void* ctx) override;
    void Finalize() override;
    int Activate() override;
    void Freeze() override;

    ///////////////////////// ConnCenterInterface /////////////////////////
#if defined(USE_BUFFEREVENT)
    ConnInterface* CreateBufferEventConn(int io_thread_idx, int sock_fd, struct bufferevent* buf_event, const char* ip,
                                         unsigned short port) override;
#else
    ConnInterface* CreateNormalConn(int io_thread_idx, int sock_fd, struct event* read_event, const char* ip,
                                    unsigned short port) override;
#endif

    void DestroyConn(int sock_fd) override;
    ConnInterface* GetConn(int sock_fd) const override;
    ConnInterface* GetConnByID(ConnID conn_id) const override;
    void UpdateConnStatus(ConnID conn_id) override;
    int AddConnInactiveSink(ConnInactiveSinkInterface* sink) override;
    void RemoveConnInactiveSink(ConnInactiveSinkInterface* sink) override;

protected:
    ///////////////////////// RecordTimeoutMgr<ConnID, std::hash<ConnID>, ConnInterface*> /////////////////////////
    void OnTimeout(const ConnID& k, ConnInterface* const& v, int timeout_sec) override;

private:
    ConnCenterCtx conn_center_ctx_;

    typedef __hash_map<int, ConnInterface*> ConnHashMap; // socket fd ->
    ConnHashMap conn_hash_map_;

    typedef __hash_map<ConnID, ConnInterface*> ConnIDHashMap;
    ConnIDSeq conn_id_seq_;
    ConnIDHashMap conn_id_hash_map_;

    int max_online_conn_count_; // 每个负责IO的线程中同时在线的conn最大数，统计用

    typedef std::set<ConnInactiveSinkInterface*> ConnInactiveSinkSet;
    ConnInactiveSinkSet conn_inactive_sink_set_;
};
}

#endif // TCP_THREADS_SRC_TCP_CONN_CENTER_H_
