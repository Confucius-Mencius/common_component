#include "global_test_env.h"

GlobalTestEnv::GlobalTestEnv()
{
}

GlobalTestEnv::~GlobalTestEnv()
{
}

void GlobalTestEnv::SetUp()
{
    std::cout << "this funtion execute before all test suite executing" << std::endl;
}

void GlobalTestEnv::TearDown()
{
    std::cout << "this function execute after all test suite executed" << std::endl;
}
