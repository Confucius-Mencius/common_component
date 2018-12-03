/**
 * @file event_center_interface.h
 * @brief 事件中心
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef EVENT_CENTER_INC_EVENT_CENTER_INTERFACE_H_
#define EVENT_CENTER_INC_EVENT_CENTER_INTERFACE_H_

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_EventCenter 事件中心
 * @{
 */

#include "module_interface.h"

/**
 * @brief 投票事件回调对象
 * @details 模块M1发出一个投票事件E，关注这个事件的模块M2、M3（监听者）等对这个投票事件进行裁决
 */
class VoteEventSinkInterface
{
public:
    virtual ~VoteEventSinkInterface()
    {
    }

    /**
     * @brief 投票事件回调接口
     * @details 模块M1发出一个投票事件E，关注这个事件的模块M2、M3（监听者）等对这个投票事件进行裁决
     * @param event_category 事件类别
     * @param event_id 全局唯一的事件ID
     * @param event_provider 事件发出者，如具体的actor对象等
     * @param event_ctx 事件现场，是应用程序自定义的结构
     * @return 裁决该投票事件的结果，返回true表示通过该投票事件，否则表示不通过，否决原因可以通过在事件现场中增加一个错误码字段传出来给上层
     * @attention 如果返回值不为true，则不会将这个事件通知给下一个监听者
     * @see ActionEventSinkInterface IEventEngine
     */
    virtual bool OnVote(int event_category, int event_id, void* event_provider, void* event_ctx, int times) = 0;
};

/**
 * @brief 行为事件回调对象
 * @details 模块M1发出一个行为事件E，关注这个事件的模块M2、M3（监听者）等对这个行为事件进行处理
 */
class ActionEventSinkInterface
{
public:
    virtual ~ActionEventSinkInterface()
    {
    }

    /**
     * @brief 行为事件回调接口
     * @details 模块M1发出一个行为事件E，关注这个事件的模块M2、M3（监听者）等对这个行为事件进行处理
     * @param event_category 事件类别
     * @param event_id 全局唯一的事件ID
     * @param event_provider 事件发出者，如具体的actor对象等
     * @param event_ctx 事件现场，是应用程序自定义的结构
     * @see VoteEventSinkInterface IEventEngine
     */
    virtual void OnAction(int event_category, int event_id, void* event_provider, void* event_ctx, int times) = 0;
};

/**
 * Create接口的参数
 */
struct EventCenterCtx
{
    int ncategories; // category数目
};

/**
 * @brief 事件引擎
 * @details 服务器内部各个模块之间通过事件引擎订阅事件、退订事件、发布事件（包括投票事件和行为事件），减少模块之间的直接调用，降低模块之间的耦合
 */
class EventCenterInterface : public ModuleInterface
{
public:
    virtual ~EventCenterInterface()
    {
    }

    /**
     * @brief 登记事件类别父子关系
     * @details 例如actor类别的父类别是creature，则某个actor死亡（设事件ID为EVENT_ID_DIE）时，也应该通知那些订阅了creature死亡事件（事件ID也为EVENT_ID_DIE）的监听者
     * @param child 子事件类别，其值应该小于初始化时设置的事件类别的最大数量
     * @param parent 父事件类别，其值应该小于初始化时设置的事件类别的最大数量
     * @return 返回0表示登记成功。如果子类别已经有父类别了，则返回-1，参数非法也返回-1
     * @attention 未登记父子类别关系时，指定ID的事件只会在自身类别中发布（发布给监听指定事件发出者的sink和不关心事件发出者的sink）
     * @see Initialize FireVoteEvent FireActionEvent
     */
    virtual int SetCategoryRelation(int parent, int child) = 0;

    /**
     * @brief 检查投票事件监听者是否存在，即是否订阅过相应的投票事件
     * @param event_category 事件类别
     * @param event_id 全局唯一的事件ID
     * @param event_provider 事件发出者，如具体的actor对象等，也可以为NULL，表示该事件的所有发出者
     * @param event_sink 事件回调对象
     * @return 返回true表示订阅过相应的投票事件，否则表示未订阅
     * @attention 如果订阅过所有发出者（订阅时指定的事件发出者为NULL，即不关心事件发出者）的某个投票事件，则当参数provider是某个具体的且未订阅过的事件发出者时，该接口依然返回false
     * @see AddVoteListener VoteEventSinkInterface
     */
    virtual bool VoteListenerExist(int event_category, int event_id, void* event_provider,
                                   VoteEventSinkInterface* event_sink) const = 0;

    /**
     * @brief 订阅投票事件
     * @param event_category 事件类别
     * @param event_id 全局唯一的事件ID
     * @param event_provider 事件发出者，如具体的actor对象等，也可以为NULL，表示该事件的所有发出者
     * @param event_sink 事件回调对象
     * @return 如果sink不为空, 且还没有订阅过该事件，则返回0，表示添加成功
     * @see VoteEventSinkInterface
     */
    virtual int AddVoteListener(int event_category, int event_id, void* event_provider,
                                VoteEventSinkInterface* event_sink, int total_times) = 0;

    /**
     * @brief 退订投票事件
     * @param event_category 事件类别
     * @param event_id 全局唯一的事件ID
     * @param event_provider 事件发出者，如具体的actor对象等，也可以为NULL，表示该事件的所有发出者
     * @param event_sink 事件回调对象
     * @see VoteEventSinkInterface
     */
    virtual void RemoveVoteListener(int event_category, int event_id, void* event_provider,
                                    VoteEventSinkInterface* event_sink) = 0;

    /**
     * @brief 发布投票事件
     * @param event_category 事件类别
     * @param event_id 全局唯一的事件ID
     * @param event_provider 事件发出者，如具体的actor对象等
     * @param event_ctx 事件现场，是应用程序自定义的结构
     * @return 该事件的全部监听者的投票结果，如果全部监听者都通过投票，则返回true，只要有一个监听者否决了投票，则立即返回false，终止事件发布，后继的监听者都不会收到这个投票事件了
     * @see SetCatRelation VoteEventSinkInterface
     */
    virtual bool FireVoteEvent(int event_category, int event_id, void* event_provider, void* event_ctx) = 0;

    /**
     * @brief 检查行为事件监听者是否存在，即是否订阅过相应的行为事件
     * @param event_category 事件类别
     * @param event_id 全局唯一的事件ID
     * @param event_provider 事件发出者，如具体的actor对象等，也可以为NULL，表示该事件的所有发出者
     * @param event_sink 事件回调对象
     * @return 返回true表示订阅过相应的行为事件，否则表示未订阅
     * @note 如果订阅过所有发出者（订阅时指定的事件发出者为NULL，即不关心事件发出者）的该行为事件，则当参数provider是某个具体的且未订阅过的事件发出者时，该接口仍然返回false
     * @see AddActionListener ActionEventSinkInterface
     */
    virtual bool ActionListenerExist(int event_category, int event_id, void* event_provider,
                                     ActionEventSinkInterface* event_sink) const = 0;

    /**
     * @brief 订阅行为事件
     * @param event_category 事件类别
     * @param event_id 全局唯一的事件ID
     * @param event_provider 事件发出者，如具体的actor对象等，也可以为NULL，表示该事件的所有发出者
     * @param event_sink 事件回调对象
     * @return 如果sink不为空, 且没有订阅过这个事件，则返回0，表示添加成功
     * @see ActionEventSinkInterface
     */
    virtual int AddActionListener(int event_category, int event_id, void* event_provider,
                                  ActionEventSinkInterface* event_sink, int total_times) = 0;

    /**
     * @brief 退订行为事件
     * @param event_category 事件类别
     * @param event_id 全局唯一的事件ID
     * @param event_provider 事件发出者，如具体的actor对象等，也可以为NULL，表示该事件的所有发出者
     * @param event_sink 事件回调对象
     * @see ActionEventSinkInterface
     */
    virtual void RemoveActionListener(int event_category, int event_id, void* event_provider,
                                      ActionEventSinkInterface* event_sink) = 0;

    /**
     * @brief 发布行为事件
     * @param event_category 事件类别
     * @param event_id 全局唯一的事件ID
     * @param event_provider 事件发出者，如具体的actor对象等
     * @param event_ctx 事件现场，是应用程序自定义的结构
     * @see SetCatRelation ActionEventSinkInterface
     */
    virtual void FireActionEvent(int event_category, int event_id, void* event_provider, void* event_ctx) = 0;
};

/** @} Module_EventCenter */
/** @} Module_Base */

#endif // EVENT_CENTER_INC_EVENT_CENTER_INTERFACE_H_
