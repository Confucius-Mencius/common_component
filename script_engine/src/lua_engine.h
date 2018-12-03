#ifndef SCRIPT_ENGINE_SRC_LUA_ENGINE_H_
#define SCRIPT_ENGINE_SRC_LUA_ENGINE_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#ifdef __cplusplus
}
#endif

#include "last_err_msg.h"
#include "misc_util.h"
#include "script_engine_interface.h"
#include "variant.h"

namespace script_engine
{
class LuaEngine : public ScriptEngineInterface
{
    DISALLOW_COPY_AND_ASSIGN(LuaEngine);

public:
    LuaEngine();
    virtual ~LuaEngine();

    ///////////////////////// ModuleInterface /////////////////////////
    virtual const char* GetVersion() const;
    virtual const char* GetLastErrMsg() const;
    virtual void Release();
    virtual int Initialize(const void* ctx);
    virtual void Finalize();
    virtual int Activate();
    virtual void Freeze();

    ///////////////////////// ScriptEngineInterface /////////////////////////
    virtual int RunScriptFile(const char* script_file_path);
    virtual int RunScriptStr(const char* script_str);
    virtual int CallScriptFunc(const char* script_func_name, const Variant* param_array, int param_count,
                               Variant* ret_array, int ret_count);
//    virtual lua_State* GetLuaState() const;

private:
    // 通过变量名将变量压栈
    void PushByName(const char* name);

    // 检查栈中指定位置是否是函数
    bool IsFunction(int idx) const;

    // 将数值压栈
    void Push(int val);
    void Push(long val);
    void Push(float val);
    void Push(double val);
    void Push(const char* val);

    // 检查栈中指定位置数据的类型
    bool TypeMatched(Type2Type<int>, int idx) const;
    bool TypeMatched(Type2Type<long>, int idx) const;
    bool TypeMatched(Type2Type<float>, int idx) const;
    bool TypeMatched(Type2Type<double>, int idx) const;
    bool TypeMatched(Type2Type<const char*>, int idx) const;

    // 获取栈中指定位置的变量的值，注意类型要匹配才行
    int Get(Type2Type<int>, int idx) const;
    long Get(Type2Type<long>, int idx) const;
    float Get(Type2Type<float>, int idx) const;
    double Get(Type2Type<double>, int idx) const;
    const char* Get(Type2Type<const char*>, int idx) const;

private:
    LastErrMsg last_err_msg_;
    ScriptEngineCtx script_engine_ctx_;
    lua_State* lua_state_;
};
}

#endif // SCRIPT_ENGINE_SRC_LUA_ENGINE_H_
