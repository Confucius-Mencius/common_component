#ifndef THREAD_CENTER_INC_THREAD_TASK_H_
#define THREAD_CENTER_INC_THREAD_TASK_H_

#include <errno.h>
#include <string.h>
#include "conn.h"
#include "log_util.h"

class ThreadInterface;

typedef int TaskType;

class ThreadTask
{
public:
    ThreadTask() : conn_guid_(), data_()
    {
        task_type_ = -1;
        source_thread_ = NULL;
    }

    ThreadTask(int task_type, ThreadInterface* source_thread, const ConnGUID* conn_guid, const void* data, size_t len)
    {
        task_type_ = task_type;
        source_thread_ = source_thread;

        if (conn_guid != NULL)
        {
            conn_guid_ = *conn_guid;
        }

        if (data != NULL && len > 0)
        {
            data_.assign((const char*) data, len);
        }
    }

    ThreadTask(int task_type, ThreadInterface* source_thread, const ConnGUID* conn_guid, const std::string& data)
    {
        task_type_ = task_type;
        source_thread_ = source_thread;

        if (conn_guid != NULL)
        {
            conn_guid_ = *conn_guid;
        }

        data_.assign(data);
    }

    ~ThreadTask()
    {
    }

    void Release()
    {
        delete this;
    }

    ThreadTask* Clone()
    {
        ThreadTask* task = new ThreadTask(task_type_, source_thread_, &conn_guid_, data_);
        if (NULL == task)
        {
            const int err = errno;
            LOG_ERROR("failed to create task, err: " << err << ", err msg: " << strerror(err));
            return NULL;
        }

        return task;
    }

    TaskType GetType() const
    {
        return task_type_;
    }

    ThreadInterface* GetSourceThread() const
    {
        return source_thread_;
    }

    const ConnGUID* GetConnGUID() const
    {
        return &conn_guid_;
    }

    const std::string& GetData() const
    {
        return data_;
    }

private:
    TaskType task_type_;
    ThreadInterface* source_thread_;
    ConnGUID conn_guid_; // io线程管理的客户端连接
    std::string data_;
};

#endif // THREAD_CENTER_INC_THREAD_TASK_H_
