#include "lua_engine.h"
#include <errno.h>
#include "version.h"

namespace script_engine
{
LuaEngine::LuaEngine() : last_err_msg_(), script_engine_ctx_()
{
    lua_state_ = NULL;
}

LuaEngine::~LuaEngine()
{
}

const char* LuaEngine::GetVersion() const
{
    return SCRIPT_ENGINE_SCRIPT_ENGINE_VERSION;
}

const char* LuaEngine::GetLastErrMsg() const
{
    return last_err_msg_.What();
}

void LuaEngine::Release()
{
    delete this;
}

int LuaEngine::Initialize(const void* ctx)
{
    if (NULL == ctx)
    {
        return -1;
    }

    script_engine_ctx_ = *((ScriptEngineCtx*) ctx);

    lua_state_ = luaL_newstate();
    if (NULL == lua_state_)
    {
        const int err = errno;
        SET_LAST_ERR_MSG(&last_err_msg_, "failed to create lua state, errno: " << err
                         << ", err msg: " << strerror(err));
        return -1;
    }

    luaL_openlibs(lua_state_); // 加载lua系统库，包括io、math等

    // 加载外部库，即实际项目中c++导出给lua的接口
    for (ToluaOpenVec::const_iterator it = script_engine_ctx_.tolua_open_vec.begin();
            it != script_engine_ctx_.tolua_open_vec.end(); ++it)
    {
        if ((*it) != NULL)
        {
            (*it)(lua_state_);
        }
    }

    // 设置栈的大小
    if (!lua_checkstack(lua_state_, script_engine_ctx_.lua_stack_size))
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "failed to set lua stack size to " << script_engine_ctx_.lua_stack_size);
        return -1;
    }

    return 0;
}

void LuaEngine::Finalize()
{
    if (lua_state_ != NULL)
    {
        lua_close(lua_state_);
        lua_state_ = NULL;
    }
}

int LuaEngine::Activate()
{
    return 0;
}

void LuaEngine::Freeze()
{
}

int LuaEngine::RunScriptFile(const char* script_file_path)
{
    if (NULL == script_file_path)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "invalid param");
        return -1;
    }

    if (NULL == lua_state_)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "lua state is null");
        return -1;
    }

    const int old_stack = lua_gettop(lua_state_);

    int ret = luaL_dofile(lua_state_, script_file_path);
    if (ret != 0)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "failed to run lua file " << script_file_path
                         << ", " << lua_tostring(lua_state_, -1));
        lua_settop(lua_state_, old_stack);
        return -1;
    }
    else
    {
        lua_settop(lua_state_, old_stack);
        return 0;
    }
}

int LuaEngine::RunScriptStr(const char* script_str)
{
    if (NULL == script_str)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "invalid param");
        return -1;
    }

    if (NULL == lua_state_)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "lua state is null");
        return -1;
    }

    const int old_stack = lua_gettop(lua_state_);

    if (luaL_dostring(lua_state_, script_str) != 0)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "failed to run lua string " << script_str
                         << ", " << lua_tostring(lua_state_, -1));
        lua_settop(lua_state_, old_stack);
        return -1;
    }
    else
    {
        lua_settop(lua_state_, old_stack);
        return 0;
    }
}

int LuaEngine::CallScriptFunc(const char* script_func_name, const Variant* param_array, int param_count,
                              Variant* ret_array, int ret_count)
{
    if (NULL == script_func_name)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "invalid param");
        return -1;
    }

    if (NULL == lua_state_)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "lua state is null");
        return -1;
    }

    const int old_stack = lua_gettop(lua_state_);

    PushByName(script_func_name); // 将函数压栈
    if (!IsFunction(-1)) // -1表示栈顶
    {
        SET_LAST_ERR_MSG(&last_err_msg_, script_func_name << " is not a lua function");
        lua_settop(lua_state_, old_stack);
        return -1;
    }

    for (int i = 0; i < param_count; ++i)
    {
        const Variant& data = param_array[i];
        const Variant::Type type = data.GetType();

        switch (type)
        {
            case Variant::TYPE_I32:
            {
                Push(data.GetValue(Type2Type<int>()));
            }
            break;

            case Variant::TYPE_I64:
            {
                Push(data.GetValue(Type2Type<long>()));
            }
            break;

            case Variant::TYPE_F32:
            {
                Push(data.GetValue(Type2Type<float>()));
            }
            break;

            case Variant::TYPE_F64:
            {
                Push(data.GetValue(Type2Type<double>()));
            }
            break;

            case Variant::TYPE_STR:
            {
                Push(data.GetValue(Type2Type<const char*>()).data);
            }
            break;

            default:
            {
                SET_LAST_ERR_MSG(&last_err_msg_, "invalid param type: " << type << ", idx: " << i);
                lua_settop(lua_state_, old_stack);
                return -1;
            }
            break;
        }
    }

    if (lua_pcall(lua_state_, param_count, ret_count, 0) != 0)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "failed to run lua function " << script_func_name
                         << ", " << lua_tostring(lua_state_, -1));
        lua_settop(lua_state_, old_stack);
        return -1;
    }

    /*
     * 在lua中如果一个函数【return 1, '!'】返回两个值，那么在函数调用后，
     * '!'在栈顶，1在栈底，所以下面的代码是反过来取值的
     */
    for (int i = ret_count - 1; i >= 0; --i)
    {
        const Variant::Type type = ret_array[i].GetType();

        switch (type)
        {
            case Variant::TYPE_I32:
            {
                ret_array[i] = Get(Type2Type<int>(), -1);
            }
            break;

            case Variant::TYPE_I64:
            {
                ret_array[i] = Get(Type2Type<long>(), -1);
            }
            break;

            case Variant::TYPE_F32:
            {
                ret_array[i] = Get(Type2Type<float>(), -1);
            }
            break;

            case Variant::TYPE_F64:
            {
                ret_array[i] = Get(Type2Type<double>(), -1);
            }
            break;

            case Variant::TYPE_STR:
            {
                const char* s = Get(Type2Type<const char*>(), -1);
                ret_array[i] = Variant(Variant::TYPE_STR, s, strlen(s));
            }
            break;

            default:
            {
                SET_LAST_ERR_MSG(&last_err_msg_, "invalid ret type: " << type << ", idx: " << i);
                lua_settop(lua_state_, old_stack);
                return -1;
            }
            break;
        }

        lua_pop(lua_state_, 1);
    }

    lua_settop(lua_state_, old_stack);
    return 0;
}

//lua_State* LuaEngine::GetLuaState() const
//{
//    return lua_state_;
//}

void LuaEngine::PushByName(const char* name)
{
    lua_getglobal(lua_state_, name);
}

bool LuaEngine::IsFunction(int idx) const
{
    return (LUA_TFUNCTION == lua_type(lua_state_, idx));
}

void LuaEngine::Push(int val)
{
    lua_pushinteger(lua_state_, val); // or lua_pushnumber
}

void LuaEngine::Push(long val)
{
    lua_pushinteger(lua_state_, val); // or lua_pushnumber
}

void LuaEngine::Push(float val)
{
    lua_pushnumber(lua_state_, val);
}

void LuaEngine::Push(double val)
{
    lua_pushnumber(lua_state_, val);
}

void LuaEngine::Push(const char* val)
{
    lua_pushstring(lua_state_, val);
}

bool LuaEngine::TypeMatched(Type2Type<int> type, int idx) const
{
    return (LUA_TNUMBER == lua_type(lua_state_, idx));
}

bool LuaEngine::TypeMatched(Type2Type<long>, int idx) const
{
    return (LUA_TNUMBER == lua_type(lua_state_, idx));
}

bool LuaEngine::TypeMatched(Type2Type<float> type, int idx) const
{
    return (LUA_TNUMBER == lua_type(lua_state_, idx));
}

bool LuaEngine::TypeMatched(Type2Type<double>, int idx) const
{
    return (LUA_TNUMBER == lua_type(lua_state_, idx));
}

bool LuaEngine::TypeMatched(Type2Type<const char*>, int idx) const
{
    return (LUA_TSTRING == lua_type(lua_state_, idx));
}

int LuaEngine::Get(Type2Type<int> type, int idx) const
{
    return (int) (lua_tointeger(lua_state_, idx));
}

long LuaEngine::Get(Type2Type<long>, int idx) const
{
    return (long) (lua_tointeger(lua_state_, idx));
}

float LuaEngine::Get(Type2Type<float> type, int idx) const
{
    return (float) (lua_tonumber(lua_state_, idx));
}

double LuaEngine::Get(Type2Type<double>, int idx) const
{
    return (double) (lua_tonumber(lua_state_, idx));
}

const char* LuaEngine::Get(Type2Type<const char*>, int idx) const
{
    return (const char*) (lua_tostring(lua_state_, idx));
}
}
