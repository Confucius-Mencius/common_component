#include "test_util.h"
#include "type2type.h"

namespace type2type_test
{
const int ret1 = 100;
const int ret2 = 200;

// 区别重载函数
int Func(Type2Type<int>)
{
    return ret1;
}

int Func(Type2Type<char>)
{
    return ret2;
}

void Test001()
{
    EXPECT_EQ(ret1, Func(Type2Type<int>()));
    EXPECT_EQ(ret2, Func(Type2Type<char>()));
}

ADD_TEST(Type2TypeTest, Test001);
}
/* namespace type2type_test */
