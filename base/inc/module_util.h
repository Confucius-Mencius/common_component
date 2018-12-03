/**
 * @file module_util.h
 * @brief
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef BASE_INC_MODULE_UTIL_H_
#define BASE_INC_MODULE_UTIL_H_

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_ModuleUtil 模块相关方法
 * @{
 */

/**
 * @brief 模块实例创建函数原型
 * @param class_id 一般情况下，一个模块只导出一个接口，导出多个接口时，根据该参数（通常是某个枚举）来创建不同类的实例
 */
typedef void* (* GetModuleInterfaceFunc)(int type);

#define GET_MODULE_INTERFACE GetModuleInterface
#define GET_MODULE_INTERFACE_NAME "GetModuleInterface"

/**
 * @brief 声明模块实例创建函数
 * @hideinitializer
 */
#if (defined(__linux__))
#define MODULE_EXPORTER_DECL() \
extern "C" void* GET_MODULE_INTERFACE(int type)
#elif (defined(_WIN32) || defined(_WIN64))
#define MODULE_EXPORTER_DECL() \
extern "C" __declspec(dllexport) void* GET_MODULE_INTERFACE(int Type)
#endif

/**
 * @brief 实现模块实例创建函数
 * @param ImplType 接口的实现类
 * @attention
 - 要求类ImplType有一个无参的构造函数
 - 导出多个接口时，根据参数type（通常是某个枚举）来创建不同类的实例
 * @hideinitializer
 */
#if (defined(__linux__))
#define MODULE_EXPORTER_IMPL(ImplType) \
extern "C" void* GET_MODULE_INTERFACE(int type) \
{ \
    return (void*) new ImplType(); \
}
#elif (defined(_WIN32) || defined(_WIN64))
#define MODULE_EXPORTER_IMPL(ImplType) \
void* GET_MODULE_INTERFACE(int Type) \
{ \
    return (void*) new ImplType(); \
}
#endif

/** @} Module_ModuleUtil */
/** @} Module_Base */

#endif // BASE_INC_MODULE_UTIL_H_
