#ifndef BASE_TEST_ZK_TEST_H_
#define BASE_TEST_ZK_TEST_H_

#include "test_util.h"
#include "zookeeper.h"

class ZkTest : public GTest
{
public:
    ZkTest();
    virtual ~ZkTest();

    virtual void SetUp();
    virtual void TearDown();

    void Test001(); // init close
    void Test002(); // 异步api
    void Test003(); // 同步api
    void Test004(); // multi原子性地一次提交多个操作
    void Test005(); // version
    void Test006(); // acl测试

private:
    zhandle_t* zkhandle_;
};

#endif // BASE_TEST_ZK_TEST_H_
