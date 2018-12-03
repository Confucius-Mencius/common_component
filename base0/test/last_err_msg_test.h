#ifndef BASE_TEST_LAST_ERR_MSG_TEST_H_
#define BASE_TEST_LAST_ERR_MSG_TEST_H_

#include "test_util.h"
#include "last_err_msg.h"

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
};

#endif // BASE_TEST_LAST_ERR_MSG_TEST_H_
