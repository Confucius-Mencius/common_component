#ifndef THREAD_CENTER_INC_TASK_DEFINE_H_
#define THREAD_CENTER_INC_TASK_DEFINE_H_

#include <errno.h>
#include <string.h>
#include "common_define.h"
#include "log_util.h"
#include "msg_define.h"

class ThreadInterface;

enum TaskType
{
    TASK_TYPE_MIN = 0,
    TASK_TYPE_TCP_CONN_CONNECTED = TASK_TYPE_MIN,
    TASK_TYPE_TCP_CONN_CLOSED,
    TASK_TYPE_TCP_SEND_TO_CLIENT,
    TASK_TYPE_TCP_SEND_RAW_TO_CLIENT,
    TASK_TYPE_TCP_CLOSE_CONN,
    TASK_TYPE_UDP_SEND_TO_CLIENT,
    TASK_TYPE_UDP_CLOSE_CONN,

    TASK_TYPE_GLOBAL_RETURN_TO_HTTP,
    TASK_TYPE_WORK_RETURN_TO_HTTP,
    TASK_TYPE_BURDEN_RETURN_TO_HTTP,

    TASK_TYPE_NORMAL,
    TASK_TYPE_MAX,
};

struct TaskCtx
{
    TaskType task_type;
    ThreadInterface* source_thread;
    ConnGuid conn_guid; // io线程管理的客户端连接
    MsgHead msg_head;
    char* msg_body;
    size_t msg_body_len;

    TaskCtx() : msg_head()
    {
        task_type = TASK_TYPE_MAX;
        source_thread = NULL;
        msg_body = NULL;
        msg_body_len = 0;
    }
};

class Task
{
public:
    Task() : task_ctx_()
    {
    }

    ~Task()
    {
    }

    static Task* Create(const TaskCtx* ctx)
    {
        Task* task = new Task();
        if (NULL == task)
        {
            const int err = errno;
            LOG_ERROR("failed to create task, errno: " << err << ", err msg: " << strerror(err));
            return NULL;
        }

        if (NULL == ctx)
        {
            return task;
        }

        task->task_ctx_.source_thread = ctx->source_thread;
        task->task_ctx_.task_type = ctx->task_type;
        task->task_ctx_.conn_guid = ctx->conn_guid;

        task->task_ctx_.msg_head = ctx->msg_head;

        if (ctx->msg_body != NULL && ctx->msg_body_len > 0)
        {
            task->task_ctx_.msg_body = new char[ctx->msg_body_len + 1];
            if (NULL == task->task_ctx_.msg_body)
            {
                const int err = errno;
                LOG_ERROR("failed to create msg body buf, errno: " << err << ", err msg: " << strerror(errno));
                task->Release();
                return NULL;
            }

            memcpy(task->task_ctx_.msg_body, ctx->msg_body, ctx->msg_body_len);
            task->task_ctx_.msg_body[ctx->msg_body_len] = '\0';
            task->task_ctx_.msg_body_len = ctx->msg_body_len;
        }

        return task;
    }

    void Release()
    {
        if (task_ctx_.msg_body != NULL)
        {
            delete[] task_ctx_.msg_body;
            task_ctx_.msg_body = NULL;
            task_ctx_.msg_body_len = 0;
        }

        delete this;
    }

    Task* Clone() const
    {
        return Task::Create(&task_ctx_);
    }

    const TaskCtx* GetCtx() const
    {
        return &task_ctx_;
    }

private:
    TaskCtx task_ctx_;
};

#endif // THREAD_CENTER_INC_TASK_DEFINE_H_
