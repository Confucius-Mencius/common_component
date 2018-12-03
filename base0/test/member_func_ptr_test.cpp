#include "member_func_ptr_test.h"

namespace member_func_ptr_test
{
Adder::Adder()
{
}

Adder::~Adder()
{
}

int Adder::AddNothing(const Data* d)
{
    return 0;
}

int Adder::AddMonsterKilledData(const Data* d)
{
    return 0;
}

int Adder::AddSkillCastedData(const Data* d)
{
    return 0;
}

void MemberFuncPtrTest::Test001()
{
    for (int i = ADDER_TYPE_MIN; i < ADDER_TYPE_MAX; ++i)
    {
        add_func_[i] = &Adder::AddNothing;
    }

    add_func_[ADDER_TYPE_KILL_MONSTER] = &Adder::AddMonsterKilledData;
    add_func_[ADDER_TYPE_CAST_SKILL] = &Adder::AddSkillCastedData;

    Data d;

    // 调用
    int ret = (adder_.*add_func_[ADDER_TYPE_KILL_MONSTER])(&d);
    EXPECT_EQ(0, ret);
}

ADD_TEST_F(MemberFuncPtrTest, Test001);
} /* namespace member_func_ptr_test */
