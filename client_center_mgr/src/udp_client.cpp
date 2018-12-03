#include "udp_client.h"
#include <arpa/inet.h>
#include <iomanip>
#include "log_util.h"
#include "msg_codec_interface.h"

namespace udp
{
void Client::ReadCallback(evutil_socket_t fd, short events, void* arg)
{
    Client* client = (Client*) arg;
    LOG_TRACE("Client::ReadCallback, socket fd: " << fd << ", events: " << setiosflags(std::ios::showbase) << std::hex
                  << events << ", client: " << client << ", " << client->peer_);

    char* data_buf = client->GetRecvBuf();
    struct sockaddr_in peer_addr;
    socklen_t peer_addr_len = sizeof(peer_addr);

    ssize_t data_len = recvfrom(fd, data_buf, client->client_center_ctx_->max_msg_body_len, 0,
                                (struct sockaddr*) &peer_addr, &peer_addr_len);
    const int err = EVUTIL_SOCKET_ERROR();

    LOG_TRACE("recv len: " << data_len);

    if (-1 == data_len)
    {
        LOG_ERROR("failed to read, errno: " << err << ", err msg: " << evutil_socket_error_to_string(err) << ", "
                      << client->peer_);
    }
    else if (0 == data_len)
    {
        LOG_INFO("connection closed, socket_fd: " << fd << ", " << client->peer_); // udp是无连接的，不会走到这里来
    }
    else if (data_len > (ssize_t) TOTAL_MSG_LEN_FIELD_LEN)
    {
        client->OnRecvPeerMsg(fd, &peer_addr, data_buf + TOTAL_MSG_LEN_FIELD_LEN,
                              data_len - TOTAL_MSG_LEN_FIELD_LEN);
    }
    else
    {
        LOG_ERROR("not a whole msg, len: " << data_len << ", " << client->peer_);
    }
}

Client::Client() : peer_()
{
    client_center_ctx_ = NULL;
    send_buf_ = NULL;
    recv_buf_ = NULL;
    sock_fd_ = -1;
    udp_event_ = NULL;
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
    if (NULL == ctx)
    {
        return -1;
    }

    client_center_ctx_ = (ClientCenterCtx*) ctx;

    send_buf_ = (char*) malloc(TOTAL_MSG_LEN_FIELD_LEN + MIN_TOTAL_MSG_LEN + client_center_ctx_->max_msg_body_len + 1);
    if (NULL == send_buf_)
    {
        const int err = errno;
        LOG_ERROR("failed to create send buf, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    recv_buf_ = (char*) malloc(MIN_TOTAL_MSG_LEN + client_center_ctx_->max_msg_body_len + 1);
    if (NULL == recv_buf_)
    {
        const int err = errno;
        LOG_ERROR("failed to create recv buf, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    if (InitSocket() != 0)
    {
        return -1;
    }

    return 0;
}

void Client::Finalize()
{
    if (udp_event_ != NULL)
    {
        event_del(udp_event_);
        event_free(udp_event_);
        udp_event_ = NULL;
    }

    if (sock_fd_ != -1)
    {
        evutil_closesocket(sock_fd_);
        sock_fd_ = -1;
    }

    if (recv_buf_ != NULL)
    {
        free(recv_buf_);
        recv_buf_ = NULL;
    }

    if (send_buf_ != NULL)
    {
        free(send_buf_);
        send_buf_ = NULL;
    }
}

int Client::Activate()
{
    return 0;
}

void Client::Freeze()
{

}

TransId Client::Send(const MsgHead& msg_head, const void* msg_body, size_t msg_body_len,
                     const base::AsyncCtx* async_ctx)
{
    if (msg_body_len > client_center_ctx_->max_msg_body_len)
    {
        LOG_ERROR("msg body len too large: " << msg_body_len << ", limit is: " << client_center_ctx_->max_msg_body_len);
        return INVALID_TRANS_ID;
    }

    TransCtx trans_ctx;
    trans_ctx.peer = peer_;
    trans_ctx.passback = msg_head.passback;

    if (async_ctx != NULL)
    {
        trans_ctx.timeout_sec = async_ctx->timeout_sec;
        trans_ctx.sink = async_ctx->sink;
        trans_ctx.async_data = (char*) async_ctx->async_data;
        trans_ctx.async_data_len = async_ctx->async_data_len;
    }

    TransId trans_id = client_center_ctx_->trans_center->RecordTransCtx(&trans_ctx);
    if (INVALID_TRANS_ID == trans_id)
    {
        return trans_id;
    }

    MsgHead trans_msg_head = msg_head;
    trans_msg_head.passback = trans_id;

    size_t data_len = 0;

    if (client_center_ctx_->msg_codec->EncodeMsg(&send_buf_, data_len, trans_msg_head, msg_body, msg_body_len) != 0)
    {
        client_center_ctx_->trans_center->CancelTrans(trans_id);
        return INVALID_TRANS_ID;
    }

    LOG_TRACE("data len: " << data_len);
    send_buf_[data_len] = '\0';

    if (-1 == sendto(sock_fd_, send_buf_, data_len, 0, (struct sockaddr*) &peer_addr_, sizeof(peer_addr_)))
    {
        const int err = EVUTIL_SOCKET_ERROR();
        LOG_ERROR("failed to send to " << peer_ << ", errno: " << err
                      << ", err msg: " << evutil_socket_error_to_string(err));
        client_center_ctx_->trans_center->CancelTrans(trans_id);
        return INVALID_TRANS_ID;
    }

    return trans_id;
}

void Client::SetPeer(const Peer& peer)
{
    peer_ = peer;

    memset(&peer_addr_, 0, sizeof(peer_addr_));

    peer_addr_.sin_family = AF_INET;
    peer_addr_.sin_addr.s_addr = inet_addr(peer_.addr.c_str());
    peer_addr_.sin_port = htons(peer_.port);
}

int Client::InitSocket()
{
    sock_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd_ < 0)
    {
        const int err = EVUTIL_SOCKET_ERROR();
        LOG_ERROR("failed to create socket, errno: " << err << ", err msg: " << evutil_socket_error_to_string(err));
        return -1;
    }

    if (evutil_make_socket_nonblocking(sock_fd_) != 0)
    {
        const int err = EVUTIL_SOCKET_ERROR();
        LOG_ERROR("failed to set socket non blocking, errno: " << err
                      << ", err msg: " << evutil_socket_error_to_string(err));
        evutil_closesocket(sock_fd_);
        return -1;
    }

    udp_event_ = event_new(client_center_ctx_->thread_ev_base, sock_fd_, EV_READ | EV_PERSIST,
                           Client::ReadCallback, this);
    if (NULL == udp_event_)
    {
        const int err = errno;
        LOG_ERROR("failed to create udp event, errno: " << err << ", err msg: " << evutil_socket_error_to_string(err));
        return -1;
    }

    if (event_add(udp_event_, NULL) != 0)
    {
        const int err = errno;
        LOG_ERROR("failed to add udp event, errno: " << err << ", err msg: " << evutil_socket_error_to_string(err));
        return -1;
    }

    return 0;
}

void Client::OnRecvPeerMsg(evutil_socket_t fd, const struct sockaddr_in* peer_addr, const char* total_msg_buf,
                           size_t total_msg_len)
{
    MsgHead msg_head;
    char* msg_body = NULL;
    size_t msg_body_len = 0;
    MsgId err_msg_id = MSG_ID_OK;

    int ret = client_center_ctx_->msg_codec->DecodeMsg(err_msg_id, &msg_head, &msg_body, msg_body_len, total_msg_buf,
                                                       total_msg_len);
    if (ret != 0)
    {
        LOG_ERROR("failed to decode msg, err: " << err_msg_id);
        return;
    }

    TransId trans_id = msg_head.passback;
    TransCtx* trans_ctx = client_center_ctx_->trans_center->GetTransCtx(trans_id);
    if (NULL == trans_ctx)
    {
        LOG_WARN("failed to get trans ctx by trans id: " << trans_id << ", maybe canceled");
        return;
    }

    if (trans_ctx->sink != NULL)
    {
        MsgHead passback_msg_head = msg_head;
        passback_msg_head.passback = trans_ctx->passback;

        trans_ctx->sink->OnRecvRsp(trans_id, peer_, passback_msg_head, msg_body, msg_body_len,
                                   trans_ctx->async_data, trans_ctx->async_data_len);
    }

    client_center_ctx_->trans_center->CancelTrans(trans_id);
}
}
