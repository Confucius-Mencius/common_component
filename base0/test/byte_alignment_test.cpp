#include "test_util.h"

/*
 GCC的字节对齐规则：
 1，默认对齐模数为4
 2，char的对齐模数为1，short的对齐模数是2，而其它所有超过2字节的数据类型（比如long,double）都以4为对齐模数。
 3，尾部填充使得整个结构的大小是对齐模数[默认的或指定的]的倍数（????）。

 VS的规则：（待验证）
 1，默认对齐模数为8。
 2，类型T的对齐模数为sizeof(T)。
 3，结构体的对齐模数为其最大成员所占的字节的整数倍（但是不能超过默认对齐模数）。
 4，尾部填充使得整个结构的大小是对齐模数[默认的或指定的]的倍数（可以为0倍）。
 */

/*
The following typical alignments are valid for compilers from Microsoft (Visual C++), Borland/CodeGear (C++Builder), Digital Mars (DMC) and GNU (GCC) when compiling for 32-bit x86:
A char (one byte) will be 1-byte aligned.
A short (two bytes) will be 2-byte aligned.
An int (four bytes) will be 4-byte aligned.
A long (four bytes) will be 4-byte aligned.
A float (four bytes) will be 4-byte aligned.
A double (eight bytes) will be 8-byte aligned on Windows and 4-byte aligned on Linux (8-byte with -malign-double compile time option).
A long long (eight bytes) will be 8-byte aligned.
A long double (ten bytes with C++Builder and DMC, eight bytes with Visual C++, twelve bytes with GCC) will be 8-byte aligned with C++Builder, 2-byte aligned with DMC, 8-byte aligned with Visual C++ and 4-byte aligned with GCC.
Any pointer (four bytes) will be 4-byte aligned. (e.g.: char*, int*)
The only notable difference in alignment for a 64-bit system when compared to a 32-bit system is:
A long (eight bytes) will be 8-byte aligned.
A double (eight bytes) will be 8-byte aligned.
A long double (eight bytes with Visual C++, sixteen bytes with GCC) will be 8-byte aligned with Visual C++ and 16-byte aligned with GCC.
Any pointer (eight bytes) will be 8-byte aligned.
*/

namespace byte_alignment_test
{
struct S1
{
    char x1;
    short x2;
    float x3;
    char x4;
};

#pragma pack(1) // 让编译器对这个结构作1字节对齐
struct S2
{
    char x1;
    short x2;
    float x3;
    char x4;
};
#pragma pack() // 取消1字节对齐，恢复默认对齐

struct A
{
    int a;
    char b;
    short c;
};

struct B
{
    char a;
    int b;
    short c;
};

#pragma pack (2) // 指定按2字节对齐
struct C
{
    char a;
    int b;
    short c;
};
#pragma pack () // 取消指定对齐，恢复默认对齐

#pragma pack (1) // 指定按1字节对齐
struct D
{
    char a;
    int b;
    short c;
};
#pragma pack () // 取消指定对齐，恢复默认对齐

struct E
{
    char a;
    double b;
};

struct F
{
    char a;
};// TODO 尾部不填充？

struct G
{
    char a;
    char b;
};

struct H
{
    char a;
    char b;
    char c;
};

struct I
{
    int a;
    double b;
    float c;
};

struct J
{
    char a[2];
    int b;
    double c;
    short d;
    struct I e;
};

struct K
{
    int a;
    char b[3]; // 元素类型为char，按1字节对齐
    short c[3];
    double d[3];
    struct I e;
};

struct L
{
    char a;
    char b[3];
    struct I c;
    char d[5];
    int e;
    double f;
    short g;
};

struct M
{
    char a;
    short c;
};

struct N
{
    char a;
    struct M b;
};// TODO 尾部不填充？

struct O
{
    char a;
    struct F b;
};

// 32位机器
void Test001()
{
    if ((size_t) 4 == sizeof(void*))
    {
        EXPECT_EQ((size_t) 12, sizeof(S1));
        EXPECT_EQ((size_t) 8, sizeof(S2));
        EXPECT_EQ((size_t) 8, sizeof(A));
        EXPECT_EQ((size_t) 12, sizeof(B));
        EXPECT_EQ((size_t) 8, sizeof(C));
        EXPECT_EQ((size_t) 7, sizeof(D));

#if (defined(__linux__))
        EXPECT_EQ((size_t) 12, sizeof(E));
#elif (defined(_WIN32) || defined(_WIN64))
        EXPECT_EQ((size_t ) 16, sizeof(E));
#endif /* */

        EXPECT_EQ((size_t) 1, sizeof(F));

#if (defined(__linux__))
        EXPECT_EQ((size_t) 4, sizeof(G));
        EXPECT_EQ((size_t) 4, sizeof(H));
        EXPECT_EQ((size_t) 16, sizeof(I));
        EXPECT_EQ((size_t) 36, sizeof(J));
        EXPECT_EQ((size_t) 56, sizeof(K));
        EXPECT_EQ((size_t) 44, sizeof(L));
#elif (defined(_WIN32) || defined(_WIN64))
        EXPECT_EQ((size_t ) 2, sizeof(G));
            EXPECT_EQ((size_t ) 3, sizeof(H));
            EXPECT_EQ((size_t ) 24, sizeof(I));
            EXPECT_EQ((size_t ) 48, sizeof(J));
            EXPECT_EQ((size_t ) 64, sizeof(K));
            EXPECT_EQ((size_t ) 64, sizeof(L));
#endif /* */

        EXPECT_EQ((size_t) 4, sizeof(M));
        EXPECT_EQ((size_t) 6, sizeof(N));
        EXPECT_EQ((size_t) 2, sizeof(O));
    }
}

ADD_TEST(ByteAlignmentTest, Test001);
}
