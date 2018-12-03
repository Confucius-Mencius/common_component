/**
 * @file script_engine_interface.h
 * @brief 脚本引擎
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef SCRIPT_ENGINE_INC_SCRIPT_ENGINE_INTERFACE_H_
#define SCRIPT_ENGINE_INC_SCRIPT_ENGINE_INTERFACE_H_

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_ScriptEngine 脚本引擎
 * @{
 */

#include <vector>
#include "module_interface.h"

struct lua_State;
struct Variant;

typedef int (*ToluaOpen)(lua_State* tolua_S);
typedef std::vector<ToluaOpen> ToluaOpenVec;

struct ScriptEngineCtx
{
    ToluaOpenVec tolua_open_vec; /**< 外部库初始化函数，即实际项目中c++导出给lua的接口 */
    int lua_stack_size;

    ScriptEngineCtx() : tolua_open_vec()
    {
        lua_stack_size = 0xff;
    }
};

class ScriptEngineInterface : public ModuleInterface
{
public:
    virtual ~ScriptEngineInterface()
    {
    }

    /**
     * @brief 执行一个脚本文件
     * @param script_file_path 脚本文件全路径名
     * @return 返回0表示执行成功，可以通过GetLastErrMsg获取最近一次的出错信息
     */
    virtual int RunScriptFile(const char* script_file_path) = 0;

    /**
     * @brief 执行一段脚本字符串
     * @param script_str 脚本字符串
     * @return 返回0表示执行成功，可以通过GetLastErrMsg获取最近一次的出错信息
     */
    virtual int RunScriptStr(const char* script_str) = 0;

    /**
     * @brief 执行一个脚本函数
     * @param script_func_name 脚本函数名
     * @param param_array 脚本函数的参数（入参）列表
     * @param param_count 参数（入参）个数
     * @param ret_array 脚本函数的返回值列表，调用者事先必须将数组各个元素的类型填好
     * @param ret_count 返回值个数
     * @return 返回0表示执行成功，可以通过GetLastErrMsg获取最近一次的出错信息
     */
    virtual int CallScriptFunc(const char* script_func_name, const Variant* param_array, int param_count,
                               Variant* ret_array, int ret_count) = 0;

    /**
     * @brief 获取lua状态机
     */
//    virtual lua_State* GetLuaState() const = 0;
};

/** @} Module_ScriptEngine */
/** @} Module_Base */

#endif // SCRIPT_ENGINE_INC_SCRIPT_ENGINE_INTERFACE_H_
