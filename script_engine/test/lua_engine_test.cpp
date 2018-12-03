#include "lua_engine_test.h"
#include "mem_util.h"
#include "variant.h"

#define TOLUAPP_ENABLED 0

#if TOLUAPP_ENABLED
#include <tolua++.h>
#include "c_api_toluapp.h"
#endif

static const char* OK_LUA_PATH = "./data/test_ok.lua";
static const char* NOT_EXIST_LUA_PATH = "./data/not_exist.lua";
static const char* ERROR_LUA_PATH = "./data/test_err.lua";
static const char* OK_LUA_STR = "print('hello' .. ' world')";
static const char* ERR_LUA_STR = "UnknownFunc(a)";
static const char* OK_LUA_FUNC = "OKFunc";
static const char* NOT_EXIST_LUA_FUNC = "NotExistFunc";
static const char* ERR_LUA_FUNC = "ErrFunc";

LuaEngineTest::LuaEngineTest() : loader_()
{
    script_engine_ = NULL;
}

LuaEngineTest::~LuaEngineTest()
{
}

void LuaEngineTest::SetUp()
{
    if (loader_.Load("../libscript_engine.so") != 0)
    {
        FAIL() << loader_.GetLastErrMsg();
    }

    script_engine_ = (ScriptEngineInterface*) loader_.GetModuleInterface(0);
    if (NULL == script_engine_)
    {
        FAIL() << loader_.GetLastErrMsg();
    }

    ScriptEngineCtx script_engine_ctx;

#if TOLUAPP_ENABLED
    script_engine_ctx.tolua_open_vec.push_back(tolua_c_api_toluapp_open);
#endif
    script_engine_ctx.lua_stack_size = 0xff;

    if (script_engine_->Initialize(&script_engine_ctx) != 0)
    {
        FAIL() << script_engine_->GetLastErrMsg();
    }

    if (script_engine_->Activate() != 0)
    {
        FAIL() << script_engine_->GetLastErrMsg();
    }
}

void LuaEngineTest::TearDown()
{
    SAFE_DESTROY_MODULE(script_engine_, loader_);
}

/**
 * @brief 执行一个存在且内容正确的脚本文件
 * @details
 *  - Set Up:
 1，脚本文件存在
 2，脚本文件的内容正确
 *  - Expect:
 1，执行成功
 *  - Tear Down:

 * @attention

 */
void LuaEngineTest::ExecFileTest001()
{
    if (script_engine_->RunScriptFile(OK_LUA_PATH) != 0)
    {
        FAIL() << script_engine_->GetLastErrMsg();
    }
}

/**
 * @brief 执行一个不存在的脚本文件
 * @details
 *  - Set Up:
 1，脚本文件不存在
 *  - Expect:
 1，执行失败，能提供可用于定位的错误信息
 *  - Tear Down:

 * @attention

 */
void LuaEngineTest::ExecFileTest002()
{
    if (script_engine_->RunScriptFile(NOT_EXIST_LUA_PATH) != 0)
    {
        std::cout << script_engine_->GetLastErrMsg() << std::endl;
    }
    else
    {
        FAIL();
    }
}

/**
 * @brief 执行一个存在的脚本文件，但是文件的内容有错误
 * @details
 *  - Set Up:
 1，脚本文件存在
 2，脚本文件的内容有错误
 *  - Expect:
 1，执行失败，能提供可用于定位的错误信息
 *  - Tear Down:

 * @attention

 */
void LuaEngineTest::ExecFileTest003()
{
    if (script_engine_->RunScriptFile(ERROR_LUA_PATH) != 0)
    {
        std::cout << script_engine_->GetLastErrMsg() << std::endl;
    }
    else
    {
        FAIL();
    }
}

/**
 * @brief 执行一个正确的脚本字符串
 * @details
 *  - Set Up:
 1，脚本字符串无错误
 *  - Expect:
 1，执行成功
 *  - Tear Down:

 * @attention

 */
void LuaEngineTest::ExecStrTest001()
{
    if (script_engine_->RunScriptStr(OK_LUA_STR) != 0)
    {
        FAIL() << script_engine_->GetLastErrMsg();
    }
}

/**
 * @brief 执行一个有错误的脚本字符串
 * @details
 *  - Set Up:
 1，脚本字符串有错误
 *  - Expect:
 1，执行失败，能提供可用于定位的错误信息
 *  - Tear Down:

 * @attention

 */
void LuaEngineTest::ExecStrTest002()
{
    if (script_engine_->RunScriptStr(ERR_LUA_STR) != 0)
    {
        std::cout << script_engine_->GetLastErrMsg() << std::endl;
    }
    else
    {
        FAIL();
    }
}

/**
 * @brief 执行一个无错误的脚本函数
 * @details
 *  - Set Up:
 1，脚本函数存在，且没有错误
 *  - Expect:
 1，执行成功
 *  - Tear Down:

 * @attention

 */
void LuaEngineTest::ExecFuncTest001()
{
    if (script_engine_->RunScriptFile(OK_LUA_PATH) != 0)
    {
        FAIL() << script_engine_->GetLastErrMsg();
    }

    Variant param_array[3];
    param_array[0] = 1; // int
    param_array[1] = Variant(Variant::TYPE_STR, "!", strlen("!")); // str
    param_array[2] = 2.0F; // float

    // 这里要设置好返回值的类型
    Variant ret_array[3];
    ret_array[0].SetType(Variant::TYPE_I32);
    ret_array[1].SetType(Variant::TYPE_STR);
    ret_array[2].SetType(Variant::TYPE_F32);

    if (script_engine_->CallScriptFunc(OK_LUA_FUNC, param_array, sizeof(param_array) / sizeof(param_array[0]),
                                       ret_array,
                                       sizeof(ret_array) / sizeof(ret_array[0])) != 0)
    {
        FAIL() << script_engine_->GetLastErrMsg();
    }

    std::cout << ret_array[0].GetValue(Type2Type<i32>()) << " " << ret_array[1].GetValue(Type2Type<const char*>()).data
              << " " << ret_array[2].GetValue(Type2Type<f32>()) << std::endl;

    EXPECT_TRUE(ret_array[0].TypeMatch(Variant::TYPE_I32));
    EXPECT_EQ(param_array[0].GetValue(Type2Type<i32>()), ret_array[0].GetValue(Type2Type<i32>()));

    EXPECT_TRUE(ret_array[1].TypeMatch(Variant::TYPE_STR));
    EXPECT_STREQ(param_array[1].GetValue(Type2Type<const char*>()).data,
                 ret_array[1].GetValue(Type2Type<const char*>()).data);

    EXPECT_TRUE(ret_array[2].TypeMatch(Variant::TYPE_F32));
    EXPECT_FLOAT_EQ(param_array[2].GetValue(Type2Type<f32>()), ret_array[2].GetValue(Type2Type<f32>()));
}

void LuaEngineTest::ExecFuncTest002()
{
    if (script_engine_->RunScriptFile(OK_LUA_PATH) != 0)
    {
        FAIL() << script_engine_->GetLastErrMsg();
    }

    Variant param_array[3];
    param_array[0] = 4294967296L; // long
    param_array[1] = Variant(Variant::TYPE_STR, "!", strlen("!")); // str
    param_array[2] = 34000000000000000000000000000000000000000.0; // double

    // 这里要设置好返回值的类型
    Variant ret_array[3];
    ret_array[0].SetType(Variant::TYPE_I64);
    ret_array[1].SetType(Variant::TYPE_STR);
    ret_array[2].SetType(Variant::TYPE_F64);

    if (script_engine_->CallScriptFunc(OK_LUA_FUNC, param_array, sizeof(param_array) / sizeof(param_array[0]),
                                       ret_array,
                                       sizeof(ret_array) / sizeof(ret_array[0])) != 0)
    {
        FAIL() << script_engine_->GetLastErrMsg();
    }

    std::cout << ret_array[0].GetValue(Type2Type<i64>()) << " "
              << ret_array[1].GetValue(Type2Type<const char*>()).data << " " << ret_array[2].GetValue(Type2Type<f64>())
              << std::endl;

    EXPECT_TRUE(ret_array[0].TypeMatch(Variant::TYPE_I64));
    EXPECT_EQ(param_array[0].GetValue(Type2Type<i64>()), ret_array[0].GetValue(Type2Type<i64>()));

    EXPECT_TRUE(ret_array[1].TypeMatch(Variant::TYPE_STR));
    EXPECT_STREQ(param_array[1].GetValue(Type2Type<const char*>()).data,
                 ret_array[1].GetValue(Type2Type<const char*>()).data);

    EXPECT_TRUE(ret_array[2].TypeMatch(Variant::TYPE_F64));
    EXPECT_DOUBLE_EQ(param_array[2].GetValue(Type2Type<f64>()), ret_array[2].GetValue(Type2Type<f64>()));
}

/**
 * @brief 执行一个不存在的脚本函数
 * @details
 *  - Set Up:
 1，脚本函数不存在
 *  - Expect:
 1，执行失败，能提供可用于定位的错误信息
 *  - Tear Down:

 * @attention

 */
void LuaEngineTest::ExecFuncTest003()
{
    if (script_engine_->RunScriptFile(OK_LUA_PATH) != 0)
    {
        FAIL() << script_engine_->GetLastErrMsg();
    }

    Variant param_array[2];
    param_array[0] = Variant(Variant::TYPE_STR, "hello", strlen("hello"));
    param_array[1] = Variant(Variant::TYPE_STR, "world", strlen("world"));

    Variant ret_array[1];

    if (script_engine_->CallScriptFunc(NOT_EXIST_LUA_FUNC, param_array, sizeof(param_array) / sizeof(param_array[0]),
                                       ret_array, sizeof(ret_array) / sizeof(ret_array[0])) != 0)
    {
        std::cout << script_engine_->GetLastErrMsg() << std::endl;
    }
    else
    {
        FAIL();
    }
}

/**
 * @brief 执行一个存在的脚本函数，但是函数内容有错误
 * @details
 *  - Set Up:
 1，脚本函数存在，但是内容有错误
 *  - Expect:
 1，执行失败，能提供可用于定位的错误信息
 *  - Tear Down:

 * @attention

 */
void LuaEngineTest::ExecFuncTest004()
{
    if (script_engine_->RunScriptFile(OK_LUA_PATH) != 0)
    {
        FAIL() << script_engine_->GetLastErrMsg();
    }

    Variant param_array[2];
    param_array[0] = Variant(Variant::TYPE_STR, "hello", strlen("hello"));
    param_array[1] = 123;

    Variant ret_array[2];

    if (script_engine_->CallScriptFunc(ERR_LUA_FUNC, param_array, sizeof(param_array) / sizeof(param_array[0]),
                                       ret_array,
                                       sizeof(ret_array) / sizeof(ret_array[0])) != 0)
    {
        std::cout << script_engine_->GetLastErrMsg() << std::endl;
    }
    else
    {
        FAIL();
    }
}

ADD_TEST_F(LuaEngineTest, ExecFileTest001);
ADD_TEST_F(LuaEngineTest, ExecFileTest002);
ADD_TEST_F(LuaEngineTest, ExecFileTest003);
ADD_TEST_F(LuaEngineTest, ExecStrTest001);
ADD_TEST_F(LuaEngineTest, ExecStrTest002);
ADD_TEST_F(LuaEngineTest, ExecFuncTest001);
ADD_TEST_F(LuaEngineTest, ExecFuncTest002);
ADD_TEST_F(LuaEngineTest, ExecFuncTest003);
ADD_TEST_F(LuaEngineTest, ExecFuncTest004);
