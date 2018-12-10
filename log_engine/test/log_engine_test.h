#ifndef CONF_CENTER_TEST_LOG_ENGINE_TEST_H_
#define CONF_CENTER_TEST_LOG_ENGINE_TEST_H_

#include "log_engine_interface.h"
#include "module_loader.h"
#include "test_util.h"

namespace log_engine_test
{
class LogEngineTest : public GTest
{
public:
    LogEngineTest();
    virtual ~LogEngineTest();

    virtual void SetUp();
    virtual void TearDown();

    void Test001();
    void Test002();
    void Test003();

    void ConsoleLogEngineTest();

private:
    ModuleLoader loader_;
    LogEngineInterface* log_engine_;
};
}

#endif // CONF_CENTER_TEST_LOG_ENGINE_TEST_H_
