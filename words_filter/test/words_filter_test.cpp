#include "words_filter_test.h"
#include "mem_util.h"

WordsFilterTest::WordsFilterTest() : loader_()
{
    words_filter_ = NULL;
}

WordsFilterTest::~WordsFilterTest()
{
}

void WordsFilterTest::SetUp()
{
    if (loader_.Load("../libwords_filter.so") != 0)
    {
        FAIL() << loader_.GetLastErrMsg();
    }

    words_filter_ = (WordsFilterInterface*) loader_.GetModuleInterface();
    if (NULL == words_filter_)
    {
        FAIL() << loader_.GetLastErrMsg();
    }

    ASSERT_EQ(0, words_filter_->Initialize(NULL));
    ASSERT_EQ(0, words_filter_->Activate());
}

void WordsFilterTest::TearDown()
{
    SAFE_DESTROY_MODULE(words_filter_, loader_);
}

/**
 * @brief 加载keywords文件成功
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void WordsFilterTest::LoadKeywordsFileTest001()
{
    int ret = words_filter_->LoadKeywordsFile("./keywords.txt");
    if (ret != 0)
    {
        FAIL() << words_filter_->GetLastErrMsg();
    }
}

/**
 * @brief 加载keywords文件失败，文件不存在
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void WordsFilterTest::LoadKeywordsFileTest002()
{
    EXPECT_FALSE(0 == words_filter_->LoadKeywordsFile("./no_such_file.txt"));
    std::cout << words_filter_->GetLastErrMsg() << std::endl;
}

/**
 * @brief 输入字符串中存在关键字，可以成功检查到
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void WordsFilterTest::CheckTest001()
{
    ASSERT_EQ(0, words_filter_->LoadKeywordsFile("./keywords.txt"));

    const char str1[] = "1江1青大海gcdfuck春暖肏花开";
    bool has_keywords = false;

    EXPECT_EQ(0, words_filter_->Check(has_keywords, str1));
    EXPECT_TRUE(has_keywords);

    const char str2[] = "江22澤22民大海gcdfuck春暖肏花开";
    has_keywords = false;

    EXPECT_EQ(0, words_filter_->Check(has_keywords, str2));
    EXPECT_TRUE(has_keywords);
}

/**
 * @brief 输入字符串中存在繁体关键字，可以成功检查到
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void WordsFilterTest::CheckTest002()
{
    ASSERT_EQ(0, words_filter_->LoadKeywordsFile("./keywords.txt"));

    const char str1[] = "1江1青大海gcdfuck春暖肏花开胡錦濤胡锦涛";
    bool has_keywords = false;

    EXPECT_EQ(0, words_filter_->Check(has_keywords, str1));
    EXPECT_TRUE(has_keywords);

    const char str2[] = "江22澤22民大海gcdfuck春暖肏花开胡錦濤&毛，澤，東，同志";
    has_keywords = false;

    EXPECT_EQ(0, words_filter_->Check(has_keywords, str2));
    EXPECT_TRUE(has_keywords);
}

/**
 * @brief 输入字符串中不存在关键字
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void WordsFilterTest::CheckTest003()
{
    ASSERT_EQ(0, words_filter_->LoadKeywordsFile("./keywords.txt"));

    const char str1[] = "大1海春暖花开哈哈哈";
    bool has_keywords = false;

    EXPECT_EQ(0, words_filter_->Check(has_keywords, str1));
    EXPECT_FALSE(has_keywords);

    const char str2[] = "大1海春暖##花开哈哈哈7777我（愛）你（）";
    has_keywords = false;

    EXPECT_EQ(0, words_filter_->Check(has_keywords, str2));
    EXPECT_FALSE(has_keywords);
}

/**
 * @brief 输入字符串中存在关键字，可以成功替换掉
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void WordsFilterTest::FilterTest001()
{
    ASSERT_EQ(0, words_filter_->LoadKeywordsFile("./keywords.txt"));

    const char str1[] = "1江1青大海gcdfuck春暖肏花开";
    bool has_keywords = false;

    EXPECT_EQ(0, words_filter_->Check(has_keywords, str1));
    EXPECT_TRUE(has_keywords);

    char* dst = NULL;
    size_t dst_len = 0;

    EXPECT_EQ(0, words_filter_->Filter(&dst, dst_len, str1, '*'));
    std::cout << "dst: " << dst << ", dst len: " << dst_len << std::endl;

    words_filter_->FilterRelease(&dst);

    const char str2[] = "江22澤22民大海gcdfuck春暖肏花开";
    has_keywords = false;

    EXPECT_EQ(0, words_filter_->Check(has_keywords, str2));
    EXPECT_TRUE(has_keywords);

    if (words_filter_->Filter(&dst, dst_len, str2, '*') != 0)
    {
        std::cout << words_filter_->GetLastErrMsg() << std::endl;
        FAIL();
    }

    std::cout << "dst: " << dst << ", dst len: " << dst_len << std::endl;
    words_filter_->FilterRelease(&dst);
}

void WordsFilterTest::FilterTest002()
{
    ASSERT_EQ(0, words_filter_->LoadKeywordsFile("./keywords.txt"));

    const char str1[] = "1江1青大海gcdfuck春暖肏花开胡錦濤胡锦涛我操tmd";
    bool has_keywords = false;

    EXPECT_EQ(0, words_filter_->Check(has_keywords, str1));
    EXPECT_TRUE(has_keywords);

    char* dst = NULL;
    size_t dst_len = 0;

    EXPECT_EQ(0, words_filter_->Filter(&dst, dst_len, str1, '*'));
    std::cout << "dst: " << dst << ", dst len: " << dst_len << std::endl;
    words_filter_->FilterRelease(&dst);

    const char str2[] = "江22澤22民大海gcdfuck春暖肏花开胡錦濤&毛，澤，東，同志大家TMD好TNND";
    has_keywords = false;

    EXPECT_EQ(0, words_filter_->Check(has_keywords, str2));
    EXPECT_TRUE(has_keywords);

    if (words_filter_->Filter(&dst, dst_len, str2, '*') != 0)
    {
        std::cout << words_filter_->GetLastErrMsg() << std::endl;
        FAIL();
    }

    std::cout << "dst: " << dst << ", dst len: " << dst_len << std::endl;
    words_filter_->FilterRelease(&dst);
}

ADD_TEST_F(WordsFilterTest, LoadKeywordsFileTest001);
ADD_TEST_F(WordsFilterTest, LoadKeywordsFileTest002);
ADD_TEST_F(WordsFilterTest, CheckTest001);
ADD_TEST_F(WordsFilterTest, CheckTest002);
ADD_TEST_F(WordsFilterTest, CheckTest003);
ADD_TEST_F(WordsFilterTest, FilterTest001);
ADD_TEST_F(WordsFilterTest, FilterTest002);
