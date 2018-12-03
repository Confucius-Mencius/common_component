#ifndef SCRIPT_ENGINE_TEST_LUA_ENGINE_TEST_H_
#define SCRIPT_ENGINE_TEST_LUA_ENGINE_TEST_H_

#include "module_loader.h"
#include "script_engine_interface.h"
#include "test_util.h"

class LuaEngineTest : public GTest
{
public:
    LuaEngineTest();
    virtual ~LuaEngineTest();

    virtual void SetUp();
    virtual void TearDown();

    void ExecFileTest001();
    void ExecFileTest002();
    void ExecFileTest003();
    void ExecStrTest001();
    void ExecStrTest002();
    void ExecFuncTest001();
    void ExecFuncTest002();
    void ExecFuncTest003();
    void ExecFuncTest004();

private:
    ModuleLoader loader_;
    ScriptEngineInterface* script_engine_;
};

#endif // SCRIPT_ENGINE_TEST_LUA_ENGINE_TEST_H_
