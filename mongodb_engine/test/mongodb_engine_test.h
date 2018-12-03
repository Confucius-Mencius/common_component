#ifndef MONGODB_ENGINE_TEST_MONGODB_ENGINE_TEST_H_
#define MONGODB_ENGINE_TEST_MONGODB_ENGINE_TEST_H_

#include "test_util.h"
#include "module_loader.h"
#include "mongodb_engine_interface.h"

class MongodbEngineTest : public GTest
{
public:
    MongodbEngineTest();
    virtual ~MongodbEngineTest();

    static void SetUpTestCase();
    static void TearDownTestCase();

    virtual void SetUp();
    virtual void TearDown();

    void Test001();
    void Test002();
    void Test003();
    void Test004();
    void Test005();
    void Test006();
    void Test007();
    void Test008();

private:
    static std::pair<mongocxx::instance*, mongocxx::pool*> mongodb_ctx_;
    ModuleLoader mongodb_engine_loader_;
    MongodbEngineInterface* mongodb_engine_;
};

#endif // MONGODB_ENGINE_TEST_MONGODB_ENGINE_TEST_H_
