#include "last_err_msg_test.h"

#define LOG_WITH_CONTEXT 0
#include "simple_log.h"

namespace last_err_msg_test
{
LastErrMsgTest::LastErrMsgTest()
{
}

LastErrMsgTest::~LastErrMsgTest()
{
}

void LastErrMsgTest::SetUp()
{
}

void LastErrMsgTest::TearDown()
{
}

void LastErrMsgTest::Test001()
{
    SET_LAST_ERR_MSG(&last_err_msg_, "last err msg test! " << 101 << ", hello, world!");
    LOG_CPP(last_err_msg_.What());

    SET_LAST_ERR_MSG(&last_err_msg_, "last err msg test 1234! " << 101 << ", hello, world!");
    LOG_CPP(last_err_msg_.What());

    SET_LAST_ERR_MSG(&last_err_msg_, "last err msg test abc! " << 101 << ", hello, world!");
    LOG_CPP(last_err_msg_.What());
}

// 超出长度的测试
void LastErrMsgTest::Test002()
{
    char buf[MAX_ERR_MSG_LEN * 2 + 1] = "";

    for (int i = 0; i < (int) sizeof(buf); ++i)
    {
        buf[i] = 'a';
    }

    SET_LAST_ERR_MSG(&last_err_msg_, buf);
    LOG_CPP(last_err_msg_.What());
}

ADD_TEST_F(LastErrMsgTest, Test001);
ADD_TEST_F(LastErrMsgTest, Test002);
}
