#ifndef RAW_TCP_THREADS_SRC_CONN_MGR_H_
#define RAW_TCP_THREADS_SRC_CONN_MGR_H_

#include "base_conn.h"
#include "record_timeout_mgr.h"

namespace tcp
{
namespace raw
{
class ThreadSink;

struct ConnMgrCtx
{
    TimerAxisInterface* timer_axis;
    struct timeval inactive_conn_check_interval;
    int inactive_conn_life;
    int storm_interval;
    int storm_threshold;

    ConnMgrCtx()
    {
        timer_axis = NULL;
        inactive_conn_check_interval.tv_sec = inactive_conn_check_interval.tv_usec = 0;
        inactive_conn_life = 0;
        storm_interval = 0;
        storm_threshold = 0;
    }
};

class ConnMgr : public RecordTimeoutMgr<ConnID, std::hash<ConnID>, BaseConn*>
{
public:
    ConnMgr();
    virtual ~ConnMgr();

    void Release();
    int Initialize(const ConnMgrCtx* ctx);
    void Finalize();
    int Activate();
    void Freeze();

    void SetThreadSink(ThreadSink* sink)
    {
        thread_sink_ = sink;
    }

    BaseConn* CreateConn(int io_thread_idx, const char* ip, unsigned short port, int sock_fd);
    void DestroyConn(int sock_fd);

    BaseConn* GetConn(int sock_fd) const;
    BaseConn* GetConnByID(ConnID conn_id) const;

    int UpdateConnStatus(ConnID conn_id);

private:
    ///////////////////////// RecordTimeoutMgr<ConnID, std::hash<ConnID>, BaseConn*> /////////////////////////
    void OnTimeout(const ConnID& k, BaseConn* const& v, int timeout_sec) override; // 空闲连接管理

    // 清理相关数据结构
    void Clear(BaseConn* conn);

private:
    ConnMgrCtx conn_mgr_ctx_;
    ThreadSink* thread_sink_;

    struct ClientCtx
    {
        BaseConn* conn;
        time_t start_time; // 统计计时
        int recv_count; // 读回调次数

        ClientCtx()
        {
            conn = NULL;
            start_time = 0;
            recv_count = 0;
        }
    };

    typedef __hash_map<int, ClientCtx> ConnHashMap; // socket fd ->
    ConnHashMap conn_hash_map_;

    typedef __hash_map<ConnID, BaseConn*> ConnIDHashMap;
    ConnIDSeq conn_id_seq_;
    ConnIDHashMap conn_id_hash_map_;

    int max_online_conn_count_; // 每个负责IO的线程中同时在线的conn最大值，统计用
};
}
}

#endif // RAW_TCP_THREADS_SRC_CONN_MGR_H_
