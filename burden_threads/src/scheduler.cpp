#include "scheduler.h"
#include "num_util.h"
#include "task_type.h"
#include "thread_sink.h"

namespace burden
{
Scheduler::Scheduler()
{
    thread_sink_ = nullptr;
    msg_codec_ = nullptr;
    threads_ctx_ = nullptr;
    related_thread_groups_ = nullptr;
    last_burden_thread_idx_ = 0;
}

Scheduler::~Scheduler()
{
}

int Scheduler::Initialize(const void* ctx)
{
    if (nullptr == ctx)
    {
        return -1;
    }

    threads_ctx_ = static_cast<const ThreadsCtx*>(ctx);
    return 0;
}

void Scheduler::Finalize()
{
}

void Scheduler::SetRelatedThreadGroups(RelatedThreadGroups* related_thread_group)
{
    related_thread_groups_ = related_thread_group;

    const int burden_thread_count = thread_sink_->GetBurdenThreadGroup()->GetThreadCount();
    if (burden_thread_count > 0)
    {
        last_burden_thread_idx_ = rand() % burden_thread_count;
    }
}

int Scheduler::SendToGlobalThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                                  const void* msg_body, size_t msg_body_len)
{
    return SendToThread(THREAD_TYPE_GLOBAL, conn_guid, msg_head, msg_body, msg_body_len, 0);
}

int Scheduler::SendToBurdenThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                                  const void* msg_body, size_t msg_body_len, int burden_thread_idx)
{
    return SendToThread(THREAD_TYPE_BURDEN, conn_guid, msg_head, msg_body, msg_body_len, burden_thread_idx);
}

int Scheduler::GetScheduleBurdenThreadIdx(int burden_thread_idx)
{
    const int burden_thread_count = thread_sink_->GetBurdenThreadGroup()->GetThreadCount();

    if (INVALID_IDX(burden_thread_idx, 0, burden_thread_count))
    {
        burden_thread_idx = last_burden_thread_idx_;
        last_burden_thread_idx_ = (last_burden_thread_idx_ + 1) % burden_thread_count;
    }

    return burden_thread_idx;
}

int Scheduler::SendToThread(int thread_type, const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                            const void* msg_body, size_t msg_body_len, int thread_idx)
{
    ThreadGroupInterface* thread_group = nullptr;
    ThreadInterface* thread = nullptr;

    switch (thread_type)
    {
        case THREAD_TYPE_GLOBAL:
        {
            thread = related_thread_groups_->global_thread;
        }
        break;

        case THREAD_TYPE_BURDEN:
        {
            thread_group = thread_sink_->GetBurdenThreadGroup();
            if (nullptr == thread_group)
            {
                LOG_ERROR("no such threads, thread type: " << thread_type);
                return -1;
            }

            int real_thread_idx = GetScheduleBurdenThreadIdx(thread_idx);
            thread = thread_group->GetThread(real_thread_idx);

        }
        break;

        default:
        {
        }
        break;
    }

    if (nullptr == thread)
    {
        LOG_ERROR("no such thread, thread type: " << thread_type);
        return -1;
    }

    std::unique_ptr<char []> buf(new char[MIN_DATA_LEN + msg_body_len + 1]);
    if (nullptr == buf)
    {
        LOG_ERROR("failed to alloc memory");
        return -1;
    }

    char* data = buf.get();
    size_t len;

    if (msg_codec_->EncodeMsg(&data, len, msg_head, msg_body, msg_body_len) != 0)
    {
        return -1;
    }

    ThreadTask* task = new ThreadTask(TASK_TYPE_NORMAL, thread_sink_->GetThread(), conn_guid, data + TOTAL_MSG_LEN_FIELD_LEN, len - TOTAL_MSG_LEN_FIELD_LEN); // 内部的消息不发送4个字节的长度字段
    if (nullptr == task)
    {
        const int err = errno;
        LOG_ERROR("failed to create task, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    thread->PushTask(task);
    return 0;
}
}
