#include <gmock/gmock.h>

int main(int argc, char* argv[])
{
    testing::InitGoogleMock(&argc, argv);

#if (defined(__linux__))
    return RUN_ALL_TESTS();
#elif (defined(_WIN32) || defined(_WIN64))
    int ret = RUN_ALL_TESTS();
    system("pause");
    return ret;
#endif
}
