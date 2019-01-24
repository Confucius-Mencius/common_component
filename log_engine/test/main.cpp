#include <log4cplus/initializer.h>
#include <gmock/gmock.h>

int main(int argc, char* argv[])
{
    log4cplus::Initializer initializer;
    testing::InitGoogleMock(&argc, argv);

#if (defined(__linux__))
    return RUN_ALL_TESTS();
#elif (defined(_WIN32) || defined(_WIN64))
    int ret = RUN_ALL_TESTS();
    system("pause");
    return ret;
#endif
}
