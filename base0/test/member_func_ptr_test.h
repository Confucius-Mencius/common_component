#ifndef BASE_TEST_MEMBER_FUNC_PTR_TEST_H_
#define BASE_TEST_MEMBER_FUNC_PTR_TEST_H_

#include "test_util.h"

namespace member_func_ptr_test
{
struct Data
{
};

class Adder
{
public:
    Adder();
    ~Adder();

public:
    int AddNothing(const Data* d);
    int AddMonsterKilledData(const Data* d);
    int AddSkillCastedData(const Data* d);
};

typedef int (Adder::*DataAdder)(const Data* d);

////////////////////////////////////////////////////////////////////////////////
enum AdderType
{
    ADDER_TYPE_MIN = 0,
    ADDER_TYPE_NOTHING = ADDER_TYPE_MIN,
    ADDER_TYPE_KILL_MONSTER,
    ADDER_TYPE_CAST_SKILL,
    ADDER_TYPE_MAX
};

class MemberFuncPtrTest : public GTest
{
public:
    void Test001();

private:
    Adder adder_;
    DataAdder add_func_[ADDER_TYPE_MAX];
};
} /* namespace member_func_ptr_test */

#endif // BASE_TEST_MEMBER_FUNC_PTR_TEST_H_
