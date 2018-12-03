#include "rand_str_test.h"

RandStrTest::RandStrTest()
{

}

RandStrTest::~RandStrTest()
{

}

void RandStrTest::Test001()
{
    const char CHARS[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const int CHARS_LEN = sizeof(CHARS) - 1;

    const int LEN = 32;
    char s[LEN + 1] = "";

    srand(time(NULL));

    for (int i = 0; i < LEN; ++i)
    {
        s[i] = CHARS[rand() / (RAND_MAX / CHARS_LEN)];
    }

    std::cout << s << std::endl;
}

void RandStrTest::Test002()
{
    const char CHARS[] = "0123456789";
    const int CHARS_LEN = sizeof(CHARS) - 1;

    const int LEN = 10;
    char s[LEN + 1] = "";

    srand(time(NULL));

    for (int i = 0; i < LEN; ++i)
    {
        s[i] = CHARS[rand() / (RAND_MAX / CHARS_LEN)];
    }

    std::cout << s << std::endl;
}

void RandStrTest::Test003()
{
    const char CHARS[] = "0123456789";
    const int CHARS_LEN = sizeof(CHARS) - 1;

    const int LEN = 6;
    char s[LEN + 1] = "";

    srand(time(NULL));

    for (int i = 0; i < LEN; ++i)
    {
        s[i] = CHARS[rand() / (RAND_MAX / CHARS_LEN)];
    }

    std::cout << s << std::endl;
}

void RandStrTest::Test004()
{
    const char CHARS[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const int CHARS_LEN = sizeof(CHARS) - 1;

    const int LEN = 32;
    char s[LEN + 1] = "";

    srand(time(NULL));

    for (int i = 0; i < LEN; ++i)
    {
        s[i] = CHARS[rand() % CHARS_LEN];
    }

    std::cout << s << std::endl;
}

void RandStrTest::Test005()
{
    const char CHARS[] = "0123456789";
    const int CHARS_LEN = sizeof(CHARS) - 1;

    const int LEN = 10;
    char s[LEN + 1] = "";

    srand(time(NULL));

    for (int i = 0; i < LEN; ++i)
    {
        s[i] = CHARS[rand() % CHARS_LEN];
    }

    std::cout << s << std::endl;
}

void RandStrTest::Test006()
{
    const char CHARS[] = "0123456789";
    const int CHARS_LEN = sizeof(CHARS) - 1;

    const int LEN = 6;
    char s[LEN + 1] = "";

    srand(time(NULL));

    for (int i = 0; i < LEN; ++i)
    {
        s[i] = CHARS[rand() % CHARS_LEN];
    }

    std::cout << s << std::endl;
}

ADD_TEST_F(RandStrTest, Test001);
ADD_TEST_F(RandStrTest, Test002);
ADD_TEST_F(RandStrTest, Test003);
ADD_TEST_F(RandStrTest, Test004);
ADD_TEST_F(RandStrTest, Test005);
ADD_TEST_F(RandStrTest, Test006);
