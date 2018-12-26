#ifndef THREAD_CENTER_TEST_LOCK_FREE_QUEUE_TEST_H_
#define THREAD_CENTER_TEST_LOCK_FREE_QUEUE_TEST_H_

#include "test_util.h"

namespace thread_center_test
{
class LockFreeQueueTest : public GTest
{
public:
    LockFreeQueueTest();
    virtual ~LockFreeQueueTest();

    void Test001();
    void Test002();
};
}

#endif // THREAD_CENTER_TEST_LOCK_FREE_QUEUE_TEST_H_
