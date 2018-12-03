/**
 * @file module_interface.h
 * @brief ModuleInterface
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef BASE_INC_MODULE_INTERFACE_H_
#define BASE_INC_MODULE_INTERFACE_H_

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_ModuleInterface ModuleInterface
 * @{
 */

/**
 * @brief 模块接口
 */
class ModuleInterface
{
public:
    virtual ~ModuleInterface()
    {
    }

    /**
     * @brief 获取模块so的版本
     */
    virtual const char* GetVersion() const = 0;

    /**
     * @brief 获取最近的错误描述
     */
    virtual const char* GetLastErrMsg() const = 0;

    /**
     * @brief 销毁自身
     */
    virtual void Release() = 0;

    /**
     * @brief 初始化、分配内部资源
     * @param [in] ctx 初始化现场
     * @return =0表示成功，否则失败
     * @see Finalize
     */
    virtual int Initialize(const void* ctx) = 0;

    /**
     * @brief 释放内部资源
     * @see Initialize
     */
    virtual void Finalize() = 0;

    /**
     * @brief 建立与其它系统/模块的关联，其中可以使用其它系统/模块的指针
     * @return =0表示成功，否则失败
     * @see Freeze
     * @attention 与其它模块没有关联的基本模块不需要实现该方法
     */
    virtual int Activate() = 0;

    /**
     * @brief 断开与其它系统/模块的关联
     * @see Activate
     * @attention 与其它模块没有关联的基本模块不需要实现该方法
     */
    virtual void Freeze() = 0;
};

/** @} Module_ModuleInterface */
/** @} Module_Base */

#endif // BASE_INC_MODULE_INTERFACE_H_
