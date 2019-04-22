#ifndef PROTO_MSG_CODEC_INC_PROTO_MSG_HANDLER_INTERFACE_H_
#define PROTO_MSG_CODEC_INC_PROTO_MSG_HANDLER_INTERFACE_H_

#include "proto_msg.h"

struct ConnGUID;

namespace proto
{
class MsgHandlerInterface
{
public:
    MsgHandlerInterface() {}

    virtual ~MsgHandlerInterface() {}

    virtual void Release() = 0;
    virtual int Initialize(const void* ctx) = 0;
    virtual void Finalize() = 0;
    virtual int Activate() = 0;
    virtual void Freeze() = 0;

    virtual MsgID GetMsgID() = 0;
    virtual void OnMsg(const ConnGUID* conn_guid, const MsgHead& msg_head, const void* msg_body, size_t msg_body_len) = 0;
};

class MsgDispatcherInterface
{
public:
    virtual ~MsgDispatcherInterface()
    {
    }

    virtual int AttachMsgHandler(MsgID msg_id, MsgHandlerInterface* msg_handler) = 0;
    virtual void DetachMsgHandler(MsgID msg_id) = 0;
};

class MsgHandlerMgrInterface
{
public:
    MsgHandlerMgrInterface()
    {
        msg_dispatcher_ = nullptr;
    }

    virtual ~MsgHandlerMgrInterface()
    {
    }

    virtual int Initialize(const void* ctx) = 0;
    virtual void Finalize() = 0;
    virtual int Activate() = 0;
    virtual void Freeze() = 0;

protected:
    MsgDispatcherInterface* msg_dispatcher_;
};
}

#define MSG_DISPATCHER_DECL \
class MsgDispatcher : public ::proto::MsgDispatcherInterface \
{ \
public: \
    MsgDispatcher(); \
    virtual ~MsgDispatcher(); \
 \
    int AttachMsgHandler(::proto::MsgID msg_id, ::proto::MsgHandlerInterface* msg_handler) override; \
    void DetachMsgHandler(::proto::MsgID msg_id) override; \
    int DispatchMsg(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head, const void* msg_body, \
                    size_t msg_body_len); \
 \
protected: \
    typedef std::map<::proto::MsgID, ::proto::MsgHandlerInterface*> MsgHandlerMap; \
    MsgHandlerMap msg_handler_map_; \
};

#define MSG_DISPATCHER_IMPL \
MsgDispatcher::MsgDispatcher() : msg_handler_map_() {} \
MsgDispatcher::~MsgDispatcher() {} \
int MsgDispatcher::AttachMsgHandler(::proto::MsgID msg_id, ::proto::MsgHandlerInterface* msg_handler) \
{ \
    MsgHandlerMap::iterator it = msg_handler_map_.find(msg_id); \
    if (it != msg_handler_map_.end()) \
    { \
        LOG_ERROR("msg handler already exist, msg id: " << msg_id); \
        return -1; \
    } \
 \
    if (!msg_handler_map_.insert(MsgHandlerMap::value_type(msg_id, msg_handler)).second) \
    { \
        const int err = errno; \
        LOG_ERROR("failed to insert to map, msg id: " << msg_id << ", msg handler: " << msg_handler \
                  << ", errno: " << err << ", err msg: " << strerror(err)); \
        return -1; \
    } \
 \
    LOG_ALWAYS("attach msg handler ok, msg id: " << msg_id << ", msg handler: " << msg_handler); \
    return 0; \
} \
 \
void MsgDispatcher::DetachMsgHandler(::proto::MsgID msg_id) \
{ \
    MsgHandlerMap::iterator it = msg_handler_map_.find(msg_id); \
    if (it != msg_handler_map_.end()) \
    { \
        msg_handler_map_.erase(it); \
    } \
} \
 \
int MsgDispatcher::DispatchMsg(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head, const void* msg_body, size_t msg_body_len) \
{ \
    MsgHandlerMap::iterator it = msg_handler_map_.find(msg_head.msg_id); \
    if (it == msg_handler_map_.end()) \
    { \
        LOG_WARN("failed to get msg handler, msg id: " << msg_head.msg_id); \
        return -1; \
    } \
 \
    struct timeval begin_time; \
    gettimeofday(&begin_time, nullptr); \
    const long begin_millisecond = begin_time.tv_sec * 1000 + begin_time.tv_usec / 1000; \
 \
    it->second->OnMsg(conn_guid, msg_head, msg_body, msg_body_len); \
 \
    struct timeval end_time; \
    gettimeofday(&end_time, nullptr); \
    const long end_millisecond = end_time.tv_sec * 1000 + end_time.tv_usec / 1000; \
 \
    if (conn_guid != nullptr) \
    { \
        LOG_INFO("msg process time: " << end_millisecond - begin_millisecond << " milliseconds. " \
                  << *conn_guid << ", " << msg_head << ", msg body len: " << msg_body_len); \
    } \
    else \
    { \
        LOG_INFO("msg process time: " << end_millisecond - begin_millisecond << " milliseconds. " \
                  << msg_head << ", msg body len: " << msg_body_len); \
    } \
\
    return 0; \
}

#endif // PROTO_MSG_CODEC_INC_PROTO_MSG_HANDLER_INTERFACE_H_
