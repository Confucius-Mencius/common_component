#include "proto_tcp_client.h"
#include <iomanip>
#include <unistd.h>
#include <event2/buffer.h>
#include <event2/event.h>
#include "log_util.h"
#include "proto_tcp_client_center.h"

namespace tcp
{
namespace proto
{
enum
{
    CONNECT_TIMER_ID = 1,
};

void Client::EventCallback(struct bufferevent* buf_event, short events, void* arg)
{
    const int err = EVUTIL_SOCKET_ERROR();
    const evutil_socket_t fd = bufferevent_getfd(buf_event);
    Client* client = static_cast<Client*>(arg);

    log4cplus::tostringstream tmp;
    tmp << std::hex << std::showbase << events;

    LOG_TRACE("Client::EventCallback, socket fd: " << fd << ", events: " << tmp.str()
              << ", client: " << client << ", " << client->peer_);

    do
    {
        if (events & BEV_EVENT_CONNECTED)
        {
            LOG_INFO("connect to " << client->peer_ << " ok, socket fd: " << fd);
            client->SetConnected(true, buf_event);
            return;
        }

        if (events & BEV_EVENT_EOF)
        {
            LOG_INFO("socket closed by peer: " << client->peer_ << ", fd: " << fd);
            break;
        }

        if (events & BEV_EVENT_ERROR)
        {
            if (err != 0)
            {
                LOG_ERROR("error occurred on socket, fd: " << fd << ", errno: " << err
                          << ", err msg: " << evutil_socket_error_to_string(err) << ", " << client->peer_);
            }

            break;
        }

        if (events & BEV_EVENT_READING)
        {
            LOG_TRACE("reading event occurred on socket, fd: " << fd);

            struct evbuffer* input_buf = bufferevent_get_input(buf_event);
            size_t input_buf_len = evbuffer_get_length(input_buf);
            LOG_TRACE("input buf len: " << input_buf_len);

            if (0 == input_buf_len)
            {
                // 这个事件等价于上面的EOF事件,都表示对端关闭了
                LOG_INFO("connection closed, socket fd: " << fd << ", " << client->peer_);
                break;
            }
        }

        if (events & BEV_EVENT_WRITING)
        {
            LOG_TRACE("writing event occurred on socket, fd: " << fd << ", " << client->peer_);
        }

        if (events & BEV_EVENT_TIMEOUT)
        {
            LOG_ERROR("timeout event occurred on socket, fd: " << fd << ", " << client->peer_); // TODO 什么时候会出现timeout？
        }

        return;
    } while (0);

    client->bufevent_set_.erase(buf_event);
    bufferevent_free(buf_event);
    client->SetConnected(false, nullptr);
}

void Client::ReadCallback(struct bufferevent* buf_event, void* arg)
{
    struct evbuffer* input_buf = bufferevent_get_input(buf_event);
    const size_t len = evbuffer_get_length(input_buf);
    const evutil_socket_t fd = bufferevent_getfd(buf_event);
    Client* client = static_cast<Client*>(arg);
    LOG_TRACE("Client::ReadCallback, socket fd: " << fd << ", input buf len: " << evbuffer_get_length(input_buf)
              << ", client: " << client << ", " << client->peer_);

    if (0  == len)
    {
        return;
    }

    ::proto::MsgCodec* msg_codec = client->client_center_ctx_->msg_codec;

    std::string& d = client->AppendRecvData(evbuffer_pullup(input_buf, -1), len);
    const char* dp = d.data();
    size_t dl = d.size();

    if (evbuffer_drain(input_buf, len) != 0)
    {
        const int err = EVUTIL_SOCKET_ERROR();
        LOG_ERROR("failed to drain data from input buffer, errno: " << err
                  << ", err msg: " << evutil_socket_error_to_string(err));
        return;
    }

    while (true)
    {
        ::proto::MsgHead msg_head;
        ::proto::MsgID err_msg_id = MSG_ID_OK;
        size_t total_msg_len = 0;

        if (!msg_codec->IsWholeMsg(err_msg_id, total_msg_len, dp, dl))
        {
            if (err_msg_id != MSG_ID_NOT_A_WHOLE_MSG)
            {
                client->ClearRecvedData();;

                msg_head.Reset();
                msg_head.msg_id = err_msg_id;
                client->Send(msg_head, nullptr, 0, nullptr);

                return;
            }

            break;
        }

        char* msg_body = nullptr;
        size_t msg_body_len = 0;

        msg_head.Reset();

        if (msg_codec->DecodeMsg(err_msg_id, &msg_head, &msg_body, msg_body_len, dp + TOTAL_MSG_LEN_FIELD_LEN, total_msg_len) != 0)
        {
            msg_head.Reset();
            msg_head.msg_id = err_msg_id;
            client->Send(msg_head, nullptr, 0, nullptr);

            return;
        }

        if (msg_head.passback != NFY_PASSBACK)
        {
            TransID trans_id = msg_head.passback;
            TransCtx* trans_ctx = client->client_center_ctx_->trans_center->GetTransCtx(trans_id);
            if (nullptr == trans_ctx)
            {
                LOG_WARN("failed to get trans ctx by trans id: " << trans_id << ", maybe canceled, " << client->peer_);
                return;
            }

            if (trans_ctx->sink != nullptr)
            {
                ::proto::MsgHead passback_msg_head = msg_head;
                passback_msg_head.passback = trans_ctx->passback;

                trans_ctx->sink->OnRecvRsp(trans_id, client->GetPeer(), passback_msg_head, msg_body, msg_body_len,
                                           trans_ctx->data, trans_ctx->len);
            }

            client->client_center_ctx_->trans_center->CancelTrans(trans_id);
        }
        else
        {
            // nfy消息
            for (NfySinkSet::iterator it = client->nfy_sink_set_.begin(); it != client->nfy_sink_set_.end(); ++it)
            {
                (*it)->OnRecvNfy(client->GetPeer(), msg_head, msg_body, msg_body_len);
            }
        }

        const size_t left = dl - TOTAL_MSG_LEN_FIELD_LEN - total_msg_len;
        if (left > 0)
        {
            d.assign(dp + TOTAL_MSG_LEN_FIELD_LEN + total_msg_len, left); // TODO 重叠assign是否安全？

            std::string& d1 = client->GetRecvedData();
            dp = d1.data();
            dl = d1.size();
        }
        else
        {
            client->ClearRecvedData();
            break;
        }
    }
}

Client::Client() : peer_(), bufevent_set_(), cache_msg_map_(), cache_msg_trans_id_list_()
{
    client_center_ = nullptr;
    client_center_ctx_ = nullptr;
    connected_ = false;
    buf_event_ = nullptr;
    reconnect_failed_count_ = 0;
}

Client::~Client()
{
}

const char* Client::GetVersion() const
{
    return nullptr;
}

const char* Client::GetLastErrMsg() const
{
    return nullptr;
}

void Client::Release()
{
    delete this;
}

int Client::Initialize(const void* ctx)
{
    if (nullptr == ctx)
    {
        return -1;
    }

    client_center_ctx_ = (ClientCenterCtx*) ctx;
    return 0;
}

void Client::Finalize()
{
    for (BufeventSet::iterator it = bufevent_set_.begin(); it != bufevent_set_.end(); ++it)
    {
        bufferevent_free(*it);
    }

    bufevent_set_.clear();
}

int Client::Activate()
{
    if (Connect() != 0)
    {
        return -1;
    }

    return 0;
}

void Client::Freeze()
{
    StopConnectTimer();
}

int Client::AddNfySink(NfySinkInterface* sink)
{
    if (nfy_sink_set_.find(sink) != nfy_sink_set_.end())
    {
        LOG_ERROR("nfy sink already exist: " << sink);
        return -1;
    }

    if (!nfy_sink_set_.insert(sink).second)
    {
        const int err = errno;
        LOG_ERROR("failed to insert to set, nfy sink: " << sink
                  << ", errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    LOG_TRACE("insert to set ok, nfy sink:: " << sink);
    return 0;
}

void Client::RemoveNfySink(NfySinkInterface* sink)
{
    NfySinkSet::iterator it = nfy_sink_set_.find(sink);
    if (it != nfy_sink_set_.end())
    {
        nfy_sink_set_.erase(it);
    }
    else
    {
        LOG_WARN("failed to find nfy sink: " << sink);
    }
}

// TODO 如果发送消息必须要按时间顺序，则如果已经存在一个cache msg，即使连接OK也不能直接发送新的消息，而是要先把全部的cache msg发送完毕。
// 这样也会引入cache msg的量如果达到一定的值就需要缓存到磁盘上去，以及定时发送cache msg等机制。
TransID Client::Send(const ::proto::MsgHead& msg_head, const void* msg_body, size_t msg_body_len, const AsyncCtx* async_ctx)
{
    TransCtx trans_ctx;
    trans_ctx.peer = peer_;
    trans_ctx.passback = msg_head.passback;

    if (async_ctx != nullptr)
    {
        trans_ctx.timeout_sec = async_ctx->timeout_sec;
        trans_ctx.sink = async_ctx->sink;
        trans_ctx.data = (char*) async_ctx->data;
        trans_ctx.len = async_ctx->len;
    }

    TransID trans_id = client_center_ctx_->trans_center->RecordTransCtx(&trans_ctx);
    if (INVALID_TRANS_ID == trans_id)
    {
        return trans_id;
    }

    std::unique_ptr<char[]> buf(new char[TOTAL_MSG_LEN_FIELD_LEN + MIN_TOTAL_MSG_LEN + msg_body_len + 1]);
    if (nullptr == buf)
    {
        const int err = errno;
        LOG_ERROR("failed to create send buf, errno: " << err << ", err msg: " << strerror(err));
        client_center_ctx_->trans_center->CancelTrans(trans_id);
        return -1;
    }

    char* send_buf = buf.get();

    ::proto::MsgHead trans_msg_head = msg_head;
    trans_msg_head.passback = trans_id;

    size_t data_len = 0;

    if (client_center_ctx_->msg_codec->EncodeMsg(&send_buf, data_len, trans_msg_head, msg_body, msg_body_len) != 0)
    {
        client_center_ctx_->trans_center->CancelTrans(trans_id);
        return INVALID_TRANS_ID;
    }

    LOG_TRACE("data len: " << data_len);
    send_buf[data_len] = '\0';

    if (!connected_)
    {
        LOG_TRACE("not connected, " << peer_ << ", cache msg first");

        // 将消息推到cache队列中，待连接建立后立即发送
        CacheMsg cache_msg;
        cache_msg.msg_head = trans_msg_head;
        cache_msg.msg_body.assign(send_buf, data_len);

        if (async_ctx != nullptr)
        {
            cache_msg.total_retries = async_ctx->total_retries;
        }

        if (AddCacheMsg(trans_id, cache_msg) != 0)
        {
            client_center_ctx_->trans_center->CancelTrans(trans_id);
            return INVALID_TRANS_ID;
        }
    }
    else
    {
        InnerSend(trans_msg_head, send_buf, data_len, async_ctx != nullptr ? async_ctx->total_retries : 0);
    }

    return trans_id;
}

void Client::Close()
{
    client_center_->RemoveClient(peer_);
}

void Client::OnTimer(TimerID timer_id, void* data, size_t len, int times)
{
    Connect();
}

void Client::OnTimeout(TransID trans_id)
{
    CacheMsgMap::iterator it_cache_msg = cache_msg_map_.find(trans_id);
    if (it_cache_msg != cache_msg_map_.end())
    {
        cache_msg_map_.erase(it_cache_msg);

        for (TransIDList::iterator it_cache_msg_trans_id = cache_msg_trans_id_list_.begin();
                it_cache_msg_trans_id != cache_msg_trans_id_list_.end();
                ++it_cache_msg_trans_id)
        {
            if (*it_cache_msg_trans_id == trans_id)
            {
                cache_msg_trans_id_list_.erase(it_cache_msg_trans_id);
                break;
            }
        }
    }
}

void Client::SetConnected(bool flag, struct bufferevent* buf_event)
{
    // 如果是连接断开了且不重连，则销毁
    if (!flag)
    {
        if (0 == client_center_ctx_->reconnect_interval.tv_sec && 0 == client_center_ctx_->reconnect_interval.tv_usec)
        {
            client_center_->RemoveClient(peer_);
            return;
        }
    }

    if (connected_ == flag)
    {
        if (!connected_)
        {
            ++reconnect_failed_count_;

            if (client_center_ctx_->reconnect_limit == reconnect_failed_count_)
            {
                // 重连100次都失败，销毁
                LOG_ERROR("failed to connect " << peer_ << " after " << reconnect_failed_count_ << " times retry, destroy the conn");
                client_center_->RemoveClient(peer_);
                return;
            }
        }

        return; // 状态未变化
    }
    else
    {
        if (flag)
        {
            reconnect_failed_count_ = 0;
        }
    }

    connected_ = flag;
    buf_event_ = buf_event;

    if (connected_)
    {
        StopConnectTimer();
        SendCacheMsg();
        client_center_ctx_->trans_center->OnConnected(peer_);
    }
    else
    {
        client_center_ctx_->trans_center->OnClosed(peer_);
        StartConnectTimer();
    }
}

int Client::StartConnectTimer()
{
    if (0 == client_center_ctx_->reconnect_interval.tv_sec && 0 == client_center_ctx_->reconnect_interval.tv_usec)
    {
        return 0;
    }

    TimerAxisInterface* timer_axis = client_center_ctx_->timer_axis;

    if (timer_axis->TimerExist(this, CONNECT_TIMER_ID))
    {
        return 0;
    }

    if (timer_axis->SetTimer(this, CONNECT_TIMER_ID, client_center_ctx_->reconnect_interval, nullptr, 0) != 0)
    {
        LOG_ERROR("failed to set timer: " << timer_axis->GetLastErrMsg());
        return -1;
    }

    return 0;
}

void Client::StopConnectTimer()
{
    client_center_ctx_->timer_axis->KillTimer(this, CONNECT_TIMER_ID);
}

int Client::Connect()
{
    char port[NI_MAXSERV];
    evutil_snprintf(port, sizeof(port), "%d", peer_.port);

    struct evutil_addrinfo ai;
    memset(&ai, 0, sizeof(ai));
    ai.ai_family = AF_INET;
    ai.ai_socktype = SOCK_STREAM;

    struct evutil_addrinfo* aitop = nullptr;
    int ret = evutil_getaddrinfo(peer_.addr.c_str(), port, &ai, &aitop);
    if (ret != 0)
    {
        LOG_ERROR("evutil_getaddrinfo failed: " << evutil_gai_strerror(ret) << ", " << peer_);
        return -1;
    }

    struct sockaddr* sa = aitop->ai_addr;
    int slen = aitop->ai_addrlen;

    evutil_socket_t fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        const int err = EVUTIL_SOCKET_ERROR();
        LOG_ERROR("failed to create socket, errno: " << err << ", err msg: " << evutil_socket_error_to_string(err));
        evutil_freeaddrinfo(aitop);
        return -1;
    }

    if (evutil_make_socket_nonblocking(fd) != 0)
    {
        const int err = EVUTIL_SOCKET_ERROR();
        LOG_ERROR("failed to set socket non blocking, errno: " << err
                  << ", err msg: " << evutil_socket_error_to_string(err));
        evutil_closesocket(fd);
        evutil_freeaddrinfo(aitop);
        return -1;
    }

    struct bufferevent* buf_event = bufferevent_socket_new(client_center_ctx_->thread_ev_base, fd,
                                    BEV_OPT_CLOSE_ON_FREE);
    if (nullptr == buf_event)
    {
        const int err = EVUTIL_SOCKET_ERROR();
        LOG_ERROR("failed to create buffer event, errno: " << err
                  << ", err msg: " << evutil_socket_error_to_string(err));
        evutil_closesocket(fd);
        evutil_freeaddrinfo(aitop);
        return -1;
    }

    bufferevent_setcb(buf_event, Client::ReadCallback, nullptr, Client::EventCallback, this);

    if (bufferevent_enable(buf_event, EV_READ | EV_WRITE | EV_PERSIST) != 0)
    {
        const int err = EVUTIL_SOCKET_ERROR();
        LOG_ERROR("failed to enable buffer event reading and writing, errno: " << err
                  << ", err msg: " << evutil_socket_error_to_string(err));
        bufferevent_free(buf_event);
        evutil_freeaddrinfo(aitop);
        return -1;
    }

    if (bufferevent_socket_connect(buf_event, sa, slen) != 0)
    {
        const int err = EVUTIL_SOCKET_ERROR();
        LOG_ERROR("failed to connect " << peer_ << "， errno: " << err
                  << ", err msg: " << evutil_socket_error_to_string(err));
        bufferevent_free(buf_event);
        evutil_freeaddrinfo(aitop);
        return -1;
    }

    evutil_freeaddrinfo(aitop);
    bufevent_set_.insert(buf_event);

    LOG_TRACE("start connecting to " << peer_ << ", socket fd: " << fd);
    return 0;
}

int Client::InnerSend(const ::proto::MsgHead& msg_head, const void* data, size_t len, int total_retry)
{
    int ret = -1;

    if (evbuffer_add(bufferevent_get_output(buf_event_), data, len) != 0)
    {
        int err = EVUTIL_SOCKET_ERROR();
        LOG_ERROR("failed to send to " << peer_ << ", errno: " << err
                  << ", err msg: " << evutil_socket_error_to_string(err) << ", total retry: " << total_retry);

        while (total_retry > 0)
        {
            usleep(1000); // 1000微妙

            if (0 == evbuffer_add(bufferevent_get_output(buf_event_), data, len))
            {
                ret = 0;
                break;
            }

            err = EVUTIL_SOCKET_ERROR();
            --total_retry;
            LOG_ERROR("failed to send to " << peer_ << ", errno: " << err
                      << "err msg: " << evutil_socket_error_to_string(err) << ", retry left: " << total_retry);
        }
    }
    else
    {
        ret = 0;
    }

    return ret;
}

int Client::AddCacheMsg(TransID trans_id, const Client::CacheMsg& cache_msg)
{
    if (!cache_msg_map_.insert(CacheMsgMap::value_type(trans_id, cache_msg)).second)
    {
        LOG_ERROR("failed to insert to map, trans id: " << trans_id);
        return -1;
    }

    cache_msg_trans_id_list_.push_back(trans_id);
    return 0;
}

void Client::SendCacheMsg()
{
    LOG_TRACE("send cached msg");

    for (TransIDList::iterator it_cache_msg_trans_id = cache_msg_trans_id_list_.begin();
            it_cache_msg_trans_id != cache_msg_trans_id_list_.end();)
    {
        CacheMsgMap::iterator it_cache_msg = cache_msg_map_.find(*it_cache_msg_trans_id);
        if (it_cache_msg == cache_msg_map_.end())
        {
            it_cache_msg_trans_id = cache_msg_trans_id_list_.erase(it_cache_msg_trans_id);
            continue;
        }

        CacheMsg& cache_msg = it_cache_msg->second;

        // 将发送成功的移除 [list可以在遍历过程中删除元素]
        if (0 == InnerSend(cache_msg.msg_head, cache_msg.msg_body.data(), cache_msg.msg_body.size(),
                           cache_msg.total_retries))
        {
            it_cache_msg_trans_id = cache_msg_trans_id_list_.erase(it_cache_msg_trans_id);
            cache_msg_map_.erase(it_cache_msg);
        }
        else
        {
            ++it_cache_msg_trans_id;
        }
    }
}
}
}
