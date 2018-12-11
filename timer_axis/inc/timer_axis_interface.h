/**
 * @file timer_axis_interface.h
 * @brief 时间轴与定时器
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef TIMER_AXIS_INC_TIMER_AXIS_INTERFACE_H_
#define TIMER_AXIS_INC_TIMER_AXIS_INTERFACE_H_

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_TimerAxis 时间轴与定时器
 * @{
 */

#include <stddef.h>
#include "module_interface.h"

struct event_base;

struct TimerAxisCtx
{
    struct event_base* thread_ev_base;

    TimerAxisCtx()
    {
        thread_ev_base = NULL;
    }
};

typedef int TimerID; /**< TimerID类型 */

/**
 * @brief 定时回调接口
 */
class TimerSinkInterface
{
public:
    virtual ~TimerSinkInterface()
    {
    }

    /**
     * @brief 定时器回调接口
     * @param timer_id 定时器ID
     * @param data 回调时使用的异步数据，可以是一个对象的指针，也可以是一块内存区
     * @param len 如果data是一个对象的指针，len为0；如果时一块内存区，len为内存区的长度
     * @param 定时器执行的次数，从1开始
     */
    virtual void OnTimer(TimerID timer_id, void* data, size_t len, int times) = 0;
};

class TimerAxisInterface : public ModuleInterface
{
public:
    virtual ~TimerAxisInterface()
    {
    }

    /**
     * @brief 检查定时器是否存在
     * @param sink 定时器回调对象
     * @param timer_id 定时器ID，在回调对象内部唯一
     */
    virtual bool TimerExist(TimerSinkInterface* sink, TimerID timer_id) = 0;

    /**
     * @brief 添加定时器
     * @param sink 定时器回调对象
     * @param timer_id 定时器ID，在回调对象内部唯一
     * @param interval 定时器间隔
     * @param async_data 回调时使用的异步数据，必须是基本数据类型或者基本数据类型组合的结构体
     * @param async_data_len 异步数据的长度
     * @param total_times 定时器执行的总次数，=-1表示一直执行
     */
    virtual int SetTimer(TimerSinkInterface* sink, TimerID timer_id, const struct timeval& interval,
                         void* data = NULL, size_t len = 0, int total_times = -1) = 0;

    /**
     * @brief 删除定时器
     * @param sink 定时器回调对象
     * @param timer_id 定时器ID，在回调对象内部唯一
     */
    virtual void KillTimer(TimerSinkInterface* sink, TimerID timer_id) = 0;
};

/** @} Module_TimerAxis */
/** @} Module_Base */

#endif // TIMER_AXIS_INC_TIMER_AXIS_INTERFACE_H_
