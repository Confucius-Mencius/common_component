#ifndef CONF_CENTER_TEST_CONF_CENTER_TEST_H_
#define CONF_CENTER_TEST_CONF_CENTER_TEST_H_

#include "conf_center_interface.h"
#include "module_loader.h"
#include "test_util.h"

namespace conf_center_test
{
class ConfCenterTest : public GTest
{
public:
    ConfCenterTest();
    virtual ~ConfCenterTest();

    virtual void SetUp();
    virtual void TearDown();

    void Test001();
    void Test002();
    void Test003();
    void Test004();
    void Test005();
    void Test006();

private:
    ModuleLoader loader_;
    ConfCenterInterface* conf_center_;
};
}

#endif // CONF_CENTER_TEST_CONF_CENTER_TEST_H_
