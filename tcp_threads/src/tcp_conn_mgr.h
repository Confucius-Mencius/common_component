#ifndef TCP_THREADS_SRC_TCP_CONN_MGR_H_
#define TCP_THREADS_SRC_TCP_CONN_MGR_H_

#include <set>
#include "base_conn.h"
#include "record_timeout_mgr.h"

namespace tcp
{
class ConnMgr : public RecordTimeoutMgr<ConnID, std::hash<ConnID>, BaseConn*>
{
public:
    ConnMgr();
    virtual ~ConnMgr();

    void Clear(ConnInterface* conn);

    void Release();
    int Initialize(const void* ctx);
    void Finalize();
    int Activate();
    void Freeze();

#if defined(USE_BUFFEREVENT)
    ConnInterface* CreateBufferEventConn(int io_thread_idx, int sock_fd, struct bufferevent* buf_event, const char* ip, unsigned short port);
#else
    ConnInterface* CreateNormalConn(int io_thread_idx, int sock_fd, struct event* read_event, const char* ip, unsigned short port);
#endif

    void DestroyConn(int sock_fd);
    BaseConn* GetConn(int sock_fd) const;
    BaseConn* GetConnByID(ConnID conn_id) const;
    void UpdateConnStatus(ConnID conn_id);
    int AddConnInactiveSink(ConnInactiveSinkInterface* sink);
    void RemoveConnInactiveSink(ConnInactiveSinkInterface* sink);

protected:
    ///////////////////////// RecordTimeoutMgr<ConnID, std::hash<ConnID>, ConnInterface*> /////////////////////////
    void OnTimeout(const ConnID& k, ConnInterface* const& v, int timeout_sec) override;

private:
    ConnCenterCtx conn_center_ctx_;

    typedef __hash_map<int, BaseConn*> ConnHashMap; // socket fd ->
    ConnHashMap conn_hash_map_;

    typedef __hash_map<ConnID, BaseConn*> ConnIDHashMap;
    ConnIDSeq conn_id_seq_;
    ConnIDHashMap conn_id_hash_map_;

    int max_online_conn_count_; // 每个负责IO的线程中同时在线的conn最大值，统计用

    typedef std::set<ConnInactiveSinkInterface*> ConnInactiveSinkSet;
    ConnInactiveSinkSet conn_inactive_sink_set_;
};
}

#endif // TCP_THREADS_SRC_TCP_CONN_MGR_H_
