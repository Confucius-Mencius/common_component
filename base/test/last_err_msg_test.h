#ifndef BASE_TEST_LAST_ERR_MSG_TEST_H_
#define BASE_TEST_LAST_ERR_MSG_TEST_H_

#include "last_err_msg.h"
#include "test_util.h"

namespace last_err_msg_test
{
class LastErrMsgTest : public GTest
{
private:
    LastErrMsg last_err_msg_;

public:
    LastErrMsgTest();
    virtual ~LastErrMsgTest();

    virtual void SetUp();
    virtual void TearDown();

    void Test001();
    void Test002();
};
}

#endif // BASE_TEST_LAST_ERR_MSG_TEST_H_
