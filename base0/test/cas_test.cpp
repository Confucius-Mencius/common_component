#include "test_util.h"

/**
 在原子操作的基础上提出了著名的 CAS（Compare - And - Swap）操作来实现 Lock-Free 算法，Intel 实现了一条类似该操作的指令：cmpxchg8。
 CAS 原语负责将某处内存地址的值（1 个字节）与一个期望值进行比较，如果相等，则将该内存地址处的值替换为新值，CAS 操作伪码描述如下：
 */
namespace cas_test
{
typedef int T;

bool CAS(T* addr, T expected, T value)
{
    if (*addr == expected)
    {
        *addr = value;
        return true;
    }

    return false;
}

/**
 * 在实际开发过程中，利用 CAS 进行同步，代码如下所示：
 */
//    do{
//            备份旧数据；
//            基于旧数据构造新数据；
//     } while(!CAS( 内存地址，备份的旧数据，新数据 ))

void Test001()
{
    T a = 1;
    T b = 2;
    T c = 3;

    CAS(&a, b, c);
}

ADD_TEST(CASTest, Test001);
} /* namespace cas_test */
