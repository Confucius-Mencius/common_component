#include "nullptr_test.h"

//nullptr关键字用于标识空指针，是std::nullptr_t类型的（constexpr）变量。
//它可以转换成任何指针类型和bool布尔类型（主要是为了兼容普通指针可以作为条件判断语句的写法），但是不能被转换为整数。

NullptrTest::NullptrTest()
{

}

NullptrTest::~NullptrTest()
{

}

void NullptrTest::Test001()
{
    char* p1 = nullptr;     // 正确
    (void) p1;

    int* p2 = nullptr;     // 正确
    (void) p2;

//    bool b = nullptr;       // g++ compile error
//    int a = nullptr;        // error
}
