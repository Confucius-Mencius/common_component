#include "web_util_test.h"
#include "log_util.h"

namespace tcp
{
namespace web
{
WebUtilTest::WebUtilTest()
{
}

WebUtilTest::~WebUtilTest()
{
}

void WebUtilTest::SetUp()
{
}

void WebUtilTest::TearDown()
{
}

/**
 * @brief
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void WebUtilTest::Test001()
{
    char s[] = "你好，世界！";
    const int len = strlen(s);

    int len1 = 0;
    char* new_s = url_encode(len1, s, len);
    LOG_DEBUG("after encode, s: " << new_s << ", len: " << len1);
    ASSERT_STREQ("%e4%bd%a0%e5%a5%bd%ef%bc%8c%e4%b8%96%e7%95%8c%ef%bc%81", new_s);
    ASSERT_EQ(54, len1);

    int len2 = url_decode(new_s, len1);
    LOG_DEBUG("after decode, s: " << new_s << ", len: " << len2);
    ASSERT_STREQ(s, new_s);
    ASSERT_EQ(len, len2);

    url_free(new_s);
}

void WebUtilTest::Test002()
{
    char s[] = "A你好，世界！B";
    const int len = strlen(s);

    int len1 = 0;
    char* new_s = url_encode(len1, s, len);
    LOG_DEBUG("after encode, s: " << new_s << ", len: " << len1);
    ASSERT_STREQ("A%e4%bd%a0%e5%a5%bd%ef%bc%8c%e4%b8%96%e7%95%8c%ef%bc%81B", new_s);
    ASSERT_EQ(56, len1);

    int len2 = url_decode(new_s, len1);
    LOG_DEBUG("after decode, s: " << new_s << ", len: " << len2);
    ASSERT_STREQ(s, new_s);
    ASSERT_EQ(len, len2);

    url_free(new_s);
}

void WebUtilTest::Test003()
{
    char s[] = "*-.0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz";
//    char s[] = "txt1=hello&txt2=world&a=你好，世界！";
    const int len = strlen(s);

    int len1 = 0;
    char* new_s = url_encode(len1, s, len);
    LOG_DEBUG("after encode, s: " << new_s << ", len: " << len1);
    ASSERT_STREQ("*-.0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz", new_s);
    ASSERT_EQ(len, len1);

    int len2 = url_decode(new_s, len1);
    LOG_DEBUG("after decode, s: " << new_s << ", len: " << len2);
    ASSERT_STREQ(s, new_s);
    ASSERT_EQ(len, len2);

    url_free(new_s);
}

ADD_TEST_F(WebUtilTest, Test001);
ADD_TEST_F(WebUtilTest, Test002);
ADD_TEST_F(WebUtilTest, Test003);
}
}
