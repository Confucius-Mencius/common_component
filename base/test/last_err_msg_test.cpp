#include "last_err_msg_test.h"

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
    std::cout << last_err_msg_.What() << std::endl;

    SET_LAST_ERR_MSG(&last_err_msg_, "last err msg test 1234! " << 101 << ", hello, world!");
    std::cout << last_err_msg_.What() << std::endl;

    SET_LAST_ERR_MSG(&last_err_msg_, "last err msg test abc! " << 101 << ", hello, world!");
    std::cout << last_err_msg_.What() << std::endl;
}

ADD_TEST_F(LastErrMsgTest, Test001);
