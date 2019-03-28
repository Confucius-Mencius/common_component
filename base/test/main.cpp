#include "global_test_env.h"
#include "simple_log.h"

GlobalTestEnv* g_global_test_env;

int main(int argc, char* argv[])
{
    g_global_test_env = new GlobalTestEnv();
    if (nullptr == g_global_test_env)
    {
        LOG_CPPE("failed to create global test env");
        return -1;
    }

    testing::AddGlobalTestEnvironment(g_global_test_env);
    testing::InitGoogleMock(&argc, argv);

    int ret = 0;

#if (defined(__linux__))
    ret = RUN_ALL_TESTS();
#elif (defined(_WIN32) || defined(_WIN64))
    ret = RUN_ALL_TESTS();
    system("pause");
#endif

//    delete g_global_test_env; // 不需要释放
    return ret;
}
