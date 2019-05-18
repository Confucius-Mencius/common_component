#include "num_util.h"
#include "str_util.h"
#include "test_util.h"

namespace str_util_test
{
void ToStrTest()
{
    const char str[] = TO_STR(hello);
    const char expect[] = "hello";
    ASSERT_STREQ(expect, str);
}

void ExpandMacroTest()
{
    const char str[] = EXPAND_MACRO(MIN(a, b));
    const char expect[] = "(((a) < (b)) ? (a) : (b))";
    ASSERT_STREQ(expect, str);
}

void TokenCatTest()
{
    int i1 = 1;
    int i2 = TOKEN_CAT(i, 1);
    ASSERT_EQ(i2, i1);
}

void TokenCatTwiceTest()
{
    int i1 = 1;
    int i2 = TOKEN_CAT(i, 1);
    ASSERT_EQ(i2, i1);
}

//loop:
//将实参代入文本中
//if 在某个实参之前有符号“#”（字符串化）或“##”（连接）then
//    对当前的文本做一次字符串化或者连接
//
//    if 结果是另外一个宏名 then
//        替换一次
//    end
//
//    结束当前的处理
//else
//    foreach 实参 do
//        if 是宏 then
//            goto loop
//        end
//    end
//
//    if 文本中不包含任何宏 then
//        if 结果是另外一个宏名 then
//            替换一次
//        end
//
//        结束全部处理
//    else
//        goto loop
//    end
//end
void MacroTest()
{
    std::cout << EXPAND_MACRO(TO_STR(x)) << std::endl;
    std::cout << EXPAND_MACRO(TOKEN_CAT(x, y)) << std::endl;
    std::cout << EXPAND_MACRO(TOKEN_CAT_TWICE(x, y)) << std::endl;
    std::cout << std::endl;

    std::cout << TO_STR(TO_STR(x)) << std::endl;
    std::cout << TO_STR(TOKEN_CAT(x, y)) << std::endl;
    std::cout << TO_STR(TOKEN_CAT_TWICE(x, y)) << std::endl;
    std::cout << std::endl;

    std::cout << EXPAND_MACRO(TO_STR(TO_STR(x))) << std::endl;
    std::cout << EXPAND_MACRO(TO_STR(TOKEN_CAT(x, y))) << std::endl;
    std::cout << EXPAND_MACRO(TO_STR(TOKEN_CAT_TWICE(x, y))) << std::endl;
    std::cout << std::endl;

//    std::cout << EXPAND_MACRO(TOKEN_CAT(TOKEN_CAT(1, 2), 3)) << std::endl; // 编译报错
//    std::cout << EXPAND_MACRO(TOKEN_CAT(TOKEN_CAT_TWICE(1, 2), 3)) << std::endl; // 编译报错
    std::cout << EXPAND_MACRO(TOKEN_CAT_TWICE(TOKEN_CAT(1, 2), 3)) << std::endl;
    std::cout << EXPAND_MACRO(TOKEN_CAT_TWICE(TOKEN_CAT_TWICE(1, 2), 3)) << std::endl;
    std::cout << std::endl;

    std::cout << EXPAND_MACRO(MAX(1, 2)) << std::endl;
}

void StrLenTest()
{
    const char str[] = "12345678";
    const size_t size = sizeof(str);
    size_t len = StrLen(str, size);
    EXPECT_EQ(sizeof(str) - 1, len);
}

void StrCpyTest()
{
    char buf[8] = "";
    const size_t size_left = sizeof(buf);

    // src比dst短
    memset(buf, 0, size_left);
    const char src1[7] = "123456";
    StrCpy(buf, size_left, src1);
    EXPECT_STREQ(src1, buf);

    // src与dst长度相等
    memset(buf, 0, size_left);
    const char src2[8] = "1234567";
    StrCpy(buf, size_left, src2);
    EXPECT_STREQ(src2, buf);

    // src比dst长，结果被截断，不会溢出
    memset(buf, 0, size_left);
    const char src3[9] = "12345678";
    StrCpy(buf, size_left, src3);
    EXPECT_STREQ("1234567", buf);
}

void StrCatTest()
{
    char buf[8] = "";
    const size_t size_left = sizeof(buf);

    // src比dst的剩余空间短
    memset(buf, 0, size_left);
    const char src1[7] = "123456";
    StrCat(buf, size_left, src1);
    EXPECT_STREQ(src1, buf);

    // src与dst的剩余空间长度相等
    memset(buf, 0, size_left);
    const char src2[8] = "1234567";
    StrCat(buf, size_left, src2);
    EXPECT_STREQ(src2, buf);

    // src比dst的剩余空间长，结果被截断，不会溢出
    memset(buf, 0, size_left);
    const char src3[9] = "12345678";
    StrCat(buf, size_left, src3);
    EXPECT_STREQ("1234567", buf);

    // 连接多个字符串
    memset(buf, 0, size_left);
    const char src4_0[] = "0";
    const size_t src4_0Len = StrLen(src4_0, sizeof(src4_0));
    const char src4_1[6] = "12345";
    StrCat(buf, size_left, src4_0);
    StrCat(buf, size_left - src4_0Len, src4_1);
    EXPECT_STREQ("012345", buf);

    // 连接多个字符串
    memset(buf, 0, size_left);
    const char src5_0[] = "0";
    const size_t src5_0_len = StrLen(src5_0, sizeof(src5_0));
    const char src5_1[7] = "123456";
    StrCat(buf, size_left, src5_0);
    StrCat(buf, size_left - src5_0_len, src5_1);
    EXPECT_STREQ("0123456", buf);

    // 连接多个字符串
    memset(buf, 0, size_left);
    const char src6_0[] = "0";
    const size_t src6_0Len = StrLen(src6_0, sizeof(src6_0));
    const char src6_1[8] = "1234567";
    StrCat(buf, size_left, src6_0);
    StrCat(buf, size_left - src6_0Len, src6_1);
    EXPECT_STREQ("0123456", buf);

    // 连接多个字符串
    memset(buf, 0, size_left);
    const char src7_0[] = "0";
    const size_t src7_0Len = StrLen(src7_0, sizeof(src7_0));
    const char src7_1[9] = "12345678";
    StrCat(buf, size_left, src7_0);
    StrCat(buf, size_left - src7_0Len, src7_1);
    EXPECT_STREQ("0123456", buf);
}

void StrCaseEQTest()
{
    char s1[] = "123abc";
    char s2[] = "123abc"; // 相等
    char s3[] = "123abcd"; // 不等，有包含关系，长度长一些
    char s4[] = "0123abc"; // 不等，长度长一些
    char s5[] = "123ab"; // 不等，有包含关系，长度短一些
    char s6[] = "23abc"; // 不等，有包含关系，长度短一些
    char s7[] = "0123ab"; // 不等，但是长度相等
    char s8[] = "23abcd"; // 不等，但是长度相等
    char s9[] = "123ABC"; // 不等，大小写有区别

    EXPECT_TRUE(StrCaseEQ(s1, s2, MIN(sizeof(s1), sizeof(s2))));
    EXPECT_FALSE(StrCaseEQ(s1, s3, MIN(sizeof(s1), sizeof(s3))));
    EXPECT_FALSE(StrCaseEQ(s1, s4, MIN(sizeof(s1), sizeof(s4))));
    EXPECT_FALSE(StrCaseEQ(s1, s5, MIN(sizeof(s1), sizeof(s5))));
    EXPECT_FALSE(StrCaseEQ(s1, s6, MIN(sizeof(s1), sizeof(s6))));
    EXPECT_FALSE(StrCaseEQ(s1, s7, MIN(sizeof(s1), sizeof(s7))));
    EXPECT_FALSE(StrCaseEQ(s1, s8, MIN(sizeof(s1), sizeof(s8))));
    EXPECT_FALSE(StrCaseEQ(s1, s9, MIN(sizeof(s1), sizeof(s9))));
}

void StrNoCaseEQTest()
{
    char s1[] = "123abc";
    char s2[] = "123abc"; // 相等
    char s3[] = "123abcd"; // 不等，有包含关系，长度长一些
    char s4[] = "0123abc"; // 不等，长度长一些
    char s5[] = "123ab"; // 不等，有包含关系，长度短一些
    char s6[] = "23abc"; // 不等，有包含关系，长度短一些
    char s7[] = "0123ab"; // 不等，但是长度相等
    char s8[] = "23abcd"; // 不等，但是长度相等
    char s9[] = "123ABC"; // 相等，大小写有区别

    EXPECT_TRUE(StrNoCaseEQ(s1, s2, MIN(sizeof(s1), sizeof(s2))));
    EXPECT_FALSE(StrNoCaseEQ(s1, s3, MIN(sizeof(s1), sizeof(s3))));
    EXPECT_FALSE(StrNoCaseEQ(s1, s4, MIN(sizeof(s1), sizeof(s4))));
    EXPECT_FALSE(StrNoCaseEQ(s1, s5, MIN(sizeof(s1), sizeof(s5))));
    EXPECT_FALSE(StrNoCaseEQ(s1, s6, MIN(sizeof(s1), sizeof(s6))));
    EXPECT_FALSE(StrNoCaseEQ(s1, s7, MIN(sizeof(s1), sizeof(s7))));
    EXPECT_FALSE(StrNoCaseEQ(s1, s8, MIN(sizeof(s1), sizeof(s8))));
    EXPECT_TRUE(StrNoCaseEQ(s1, s9, MIN(sizeof(s1), sizeof(s9))));
}

void StrPrintfTest()
{
    char buf[8] = "";
    const size_t size_left = sizeof(buf);

    // src比dst的剩余空间短
    memset(buf, 0, size_left);
    const char src1[7] = "123456";
    StrPrintf(buf, size_left, "%s", src1);
    EXPECT_STREQ(src1, buf);

    // src与dst的剩余空间长度相等
    memset(buf, 0, size_left);
    const char src2[8] = "1234567";
    StrPrintf(buf, size_left, "%s", src2);
    EXPECT_STREQ(src2, buf);

    // src比dst的剩余空间长，结果被截断，不会溢出
    memset(buf, 0, size_left);
    const char src3[9] = "12345678";
    StrPrintf(buf, size_left, "%s", src3);
    EXPECT_STREQ("1234567", buf);

    // 格式化多个字符串到一个目标缓冲区中，未超出目标缓冲区的大小
    memset(buf, 0, size_left);
    const char src4[6] = "12345";
    i32 n = 0;
    n += StrPrintf(buf + n, size_left - n, "%d", 0);
    n += StrPrintf(buf + n, size_left - n, "%s", src4);
    EXPECT_STREQ("012345", buf);

    // 格式化多个字符串到一个目标缓冲区中，未超出目标缓冲区的大小
    memset(buf, 0, size_left);
    const char src5[7] = "123456";
    n = 0;
    n += StrPrintf(buf + n, size_left - n, "%d", 0);
    n += StrPrintf(buf + n, size_left - n, "%s", src5);
    EXPECT_STREQ("0123456", buf);

    // 格式化多个字符串到一个目标缓冲区中，超出目标缓冲区的大小，结果被截断
    memset(buf, 0, size_left);
    const char src6[8] = "1234567";
    n = 0;
    n += StrPrintf(buf + n, size_left - n, "%d", 0);
    n += StrPrintf(buf + n, size_left - n, "%s", src6);
    EXPECT_STREQ("0123456", buf);

    // 格式化多个字符串到一个目标缓冲区中，超出目标缓冲区的大小，结果被截断
    memset(buf, 0, size_left);
    char* p = buf;
    size_t left_size = size_left;

    n = StrPrintf(p, left_size, "%d", 0);
    p += n;
    left_size -= n;

    EXPECT_EQ(1, n);
    EXPECT_STREQ("0", buf);
    EXPECT_EQ((size_t) 7, left_size);

    // 这里结果被截断
    const char src7[9] = "12345678";
    n = StrPrintf(p, left_size, "%s", src7);
    EXPECT_EQ(6, n);
    EXPECT_STREQ("0123456", buf);

    p += n;
    left_size -= n;

    // 目标缓冲区中只剩下一个空间，供结束符\0使用，此时就格式化不进任何东西了
    const char src8[] = "1";
    n = StrPrintf(p, left_size, "%s", src8);
    EXPECT_EQ(0, n);
    EXPECT_STREQ("0123456", buf);
}

void StrTrimTest()
{
    char s1[] = "  \r \n \t \t abc  \t \n \r  ";
    char buf1[8] = "";
    int n1 = StrTrim(buf1, sizeof(buf1), s1, StrLen(s1, sizeof(s1)));
    EXPECT_GT(n1, 0);
    EXPECT_STREQ("abc", buf1);

    char s2[] = "  \r \n \t \t a \r \n    \t b \t c  \t \n \r  ";
    char buf2[24] = "";
    int n2 = StrTrim(buf2, sizeof(buf2), s2, StrLen(s2, sizeof(s2)));
    EXPECT_GT(n2, 0);
    EXPECT_STREQ("a \r \n    \t b \t c", buf2);

    char s3[] = "\"' a b \t \"'";
    char buf3[24] = "";
    int n3 = StrTrim(buf3, sizeof(buf3), s3, StrLen(s3, sizeof(s3)), "\"'");
    EXPECT_GT(n3, 0);
    EXPECT_STREQ(" a b \t ", buf3);
}

void StrReplaceTest()
{
    char s1[] = "xxabcyyabczz";
    char buf1[32] = "";
    int n1 = StrReplace(buf1, sizeof(buf1), s1, "abc", "12");
    EXPECT_GT(n1, 0);
    EXPECT_STREQ("xx12yy12zz", buf1);

    char s2[] = "xxabcyyabczz";
    char buf2[32] = "";
    int n2 = StrReplace(buf2, sizeof(buf2), s2, "abc", "1234");
    EXPECT_GT(n2, 0);
    EXPECT_STREQ("xx1234yy1234zz", buf2);
}

void StrReverseTest()
{
    char s[] = "123456789";
    StrReverse(s, sizeof(s) - 1);
    std::cout << s << std::endl;
}

void WildcardCmpTest()
{
    EXPECT_EQ(0, MatchWithAsteriskW("x.y.a.com", strlen("x.y.a.com"), "*.a.com", strlen("*.a.com")));
    EXPECT_EQ(0, MatchWithAsteriskW("x.y.a.com.p", strlen("x.y.a.com.p"), "*.a.com*", strlen("*.a.com*")));
    EXPECT_EQ(0, MatchWithAsteriskW("x.y.a.com.p", strlen("x.y.a.com.p"), "x.y.a.com*", strlen("*.a.com*")));
    EXPECT_EQ(0, MatchWithAsteriskW("x.y.a.com.p", strlen("x.y.a.com.p"), "*.?.??m*", strlen("*.a.com*")));
}

void strtok_test()
{
    char* str;
    char* token;
    char* saveptr;
    int i;

    const char s[] = "a&b&c";

    for (i = 1, str = (char*) s;; ++i, str = nullptr)
    {
        token = strtok_r(str, "&", &saveptr);
        if (nullptr == token)
        {
            break;
        }

        std::cout << i << " " << token << std::endl;
    }
}

void StrBeginEndWithTest()
{
    const char str[] = "hello, world";
    EXPECT_TRUE(StrBeginWith(str, "hello"));
    EXPECT_FALSE(StrBeginWith(str, "Hello"));

    EXPECT_TRUE(StrEndWith(str, "world"));
    EXPECT_FALSE(StrEndWith(str, "World"));
}

class O
{
public:
    int func(int i)
    {
        return i;
    }
};

/**
 * 执行o的成员函数f，输出成员函数名和结果
 */
#define PRINT_OF(o, f) std::cout << #f ": " << o.f << std::endl

void PrintOfTest()
{
    O o;
    PRINT_OF(o, func(0));
    PRINT_OF(o, func(1));
}

ADD_TEST(StrUtilTest, ToStrTest);
ADD_TEST(StrUtilTest, ExpandMacroTest);
ADD_TEST(StrUtilTest, TokenCatTest);
ADD_TEST(StrUtilTest, TokenCatTwiceTest);
ADD_TEST(StrUtilTest, MacroTest);
ADD_TEST(StrUtilTest, StrLenTest);
ADD_TEST(StrUtilTest, StrCpyTest);
ADD_TEST(StrUtilTest, StrCatTest);
ADD_TEST(StrUtilTest, StrCaseEQTest);
ADD_TEST(StrUtilTest, StrNoCaseEQTest);
ADD_TEST(StrUtilTest, StrPrintfTest);
ADD_TEST(StrUtilTest, StrTrimTest);
ADD_TEST(StrUtilTest, StrReplaceTest);
ADD_TEST(StrUtilTest, StrReverseTest);
ADD_TEST(StrUtilTest, WildcardCmpTest);
ADD_TEST(StrUtilTest, strtok_test);
ADD_TEST(StrUtilTest, StrBeginEndWithTest);
ADD_TEST(StrUtilTest, PrintOfTest);
} // namespace str_util_test
