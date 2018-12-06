#include "global_test_env.h"
#include "simple_log.h"

GlobalTestEnv::GlobalTestEnv()
{
}

GlobalTestEnv::~GlobalTestEnv()
{
}

void GlobalTestEnv::SetUp()
{
    LOG_CPP("this funtion execute before all test suite executing");
}

void GlobalTestEnv::TearDown()
{
    LOG_CPP("this function execute after all test suite executed");
}
