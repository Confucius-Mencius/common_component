#include "time_service_test.h"
#include "simple_log.h"
#include "mem_util.h"

TimeServiceTest::TimeServiceTest() : loader_()
{
    time_service_ = NULL;
}

TimeServiceTest::~TimeServiceTest()
{
}

void TimeServiceTest::SetUp()
{
    if (loader_.Load("../libtime_service.so") != 0)
    {
        FAIL() << loader_.GetLastErrMsg();
    }

    time_service_ = (TimeServiceInterface*) loader_.GetModuleInterface();
    if (NULL == time_service_)
    {
        FAIL() << loader_.GetLastErrMsg();
    }

    ASSERT_EQ(0, time_service_->Initialize(NULL));
    ASSERT_EQ(0, time_service_->Activate());
}

void TimeServiceTest::TearDown()
{
    SAFE_DESTROY_MODULE(time_service_, loader_);
}

/**
 * @brief 获取格林尼治时间
 * @details
 *  - Set Up:
 无
 *  - Expect:
 1，成功
 *  - Tear Down:
 无
 * @attention
 无
 */
void TimeServiceTest::Time001()
{
    time_t t = time_service_->Time();
    LOG_CPP("now time: " << t);
}

/**
 * @brief 计算两个格林尼治时间的差值
 * @details
 *  - Set Up:
 无
 *  - Expect:
 1，成功
 *  - Tear Down:
 无
 * @attention
 无
 */
void TimeServiceTest::TimeDiff001()
{
    time_t t = time_service_->Time();
    time_t t1 = time_service_->MakeTime(t, 0, 0, 0);
    time_t t2 = time_service_->MakeTime(t, 23, 59, 59);

    time_t diff = time_service_->TimeDiff(t1, t2);
    LOG_CPP("time diff: " << diff);
    EXPECT_EQ(TimeServiceInterface::SECS_PER_DAY - 1, diff);
}

/**
 * @brief 获取格林尼治时间的字符串表示
 * @details
 *  - Set Up:
 无
 *  - Expect:
 1，成功
 *  - Tear Down:
 无
 * @attention
 无
 */
void TimeServiceTest::TimeToStr001()
{
    time_t t = time_service_->Time();

    char buf[TimeServiceInterface::MAX_TIME_STR_LEN + 1];
    int len = sizeof(buf);

    EXPECT_EQ(0, time_service_->TimeToStr(buf, len, t));
    EXPECT_EQ(19, len);
    LOG_CPP("now time str: " << buf);
}

/**
 * @brief 获取时间字符串表示的格林尼治时间-成功
 * @details
 *  - Set Up:
 1，时间字符串格式正确
 2，时间字符串内容合法
 *  - Expect:
 1，成功
 *  - Tear Down:
 无
 * @attention
 无
 */
void TimeServiceTest::TimeFromStr001()
{
    char s[] = "2013-10-02 09:00:00";
    time_t t1 = time_service_->MakeTime(2013, 10, 2, 9, 0, 0);
    time_t t2 = time_service_->TimeFromStr(s);
    EXPECT_EQ(t1, t2);
}

/**
 * @brief 检查两个格林尼治时间是否是同一天的-同一天的
 * @details
 *  - Set Up:
 1，两个参数是同一天的
 *  - Expect:
 1，返回true
 *  - Tear Down:
 无
 * @attention
 无
 */
void TimeServiceTest::TimeInSameDay001()
{
    time_t t1 = time_service_->MakeTime(2013, 10, 2, 9, 0, 0);
    time_t t2 = time_service_->MakeTime(2013, 10, 2, 10, 0, 0);
    EXPECT_TRUE(time_service_->TimeInSameDay(t1, t2));
}

/**
 * @brief 检查两个格林尼治时间是否是同一天的-不是同一天的
 * @details
 *  - Set Up:
 1，两个参数是不是同一天的
 *  - Expect:
 1，返回false
 *  - Tear Down:
 无
 * @attention
 无
 */
void TimeServiceTest::TimeInSameDay002()
{
    time_t t1 = time_service_->MakeTime(2013, 10, 2, 9, 0, 0);
    time_t t2 = time_service_->MakeTime(2013, 10, 3, 10, 0, 0);
    EXPECT_FALSE(time_service_->TimeInSameDay(t1, t2));
}

/**
 * @brief 判断指定的格林尼治时间是否在指定的时分秒时间段之间-在时间段之间
 * @details
 *  - Set Up:
 1，指定时间在时间段之间
 *  - Expect:
 1，返回true
 *  - Tear Down:
 无
 * @attention
 无
 */
void TimeServiceTest::TimeBetween001()
{
    time_t t1 = time_service_->MakeTime(2013, 10, 2, 9, 0, 0);
    EXPECT_TRUE(time_service_->TimeBetween(t1, 8, 0, 0, 10, 0, 0));
}

/**
 * @brief 判断指定的格林尼治时间是否在指定的时分秒时间段之间-不在时间段之间
 * @details
 *  - Set Up:
 1，指定时间不在时间段之间
 *  - Expect:
 1，返回false
 *  - Tear Down:
 无
 * @attention
 无
 */
void TimeServiceTest::TimeBetween002()
{
    time_t t1 = time_service_->MakeTime(2013, 10, 2, 9, 0, 0);
    EXPECT_FALSE(time_service_->TimeBetween(t1, 7, 0, 0, 8, 0, 0));
    EXPECT_FALSE(time_service_->TimeBetween(t1, 9, 30, 0, 11, 30, 0));
}

/**
 * @brief 获取高精度的格林尼治时间
 * @details
 *  - Set Up:
 无
 *  - Expect:
 1，成功
 *  - Tear Down:
 无
 * @attention
 无
 */
void TimeServiceTest::Timeval001()
{
    struct timeval tv;
    EXPECT_EQ(0, time_service_->Timeval(&tv, NULL));
    LOG_CPP("tv_sec: " << tv.tv_sec << ", tv_usec: " << tv.tv_usec);
}

/**
 * @brief 计算两个高精度格林尼治时间的差值
 * @details
 *  - Set Up:
 无
 *  - Expect:
 1，成功
 *  - Tear Down:
 无
 * @attention
 无
 */
void TimeServiceTest::TimevalDiff001()
{
    struct timeval tv1;
    struct timeval tv2;
    EXPECT_EQ(0, time_service_->Timeval(&tv1, NULL));
    time_service_->SleepSec(1);
    time_service_->SleepMillisec(100);
    EXPECT_EQ(0, time_service_->Timeval(&tv2, NULL));

    struct timeval diff = time_service_->TimevalDiff(tv1, tv2);
    LOG_CPP(
        "timeval diff -> " << "tv_sec: " << diff.tv_sec << ", tv_usec: " << diff.tv_usec);
}

/**
 * @brief 获取高精度格林尼治时间的字符串表示
 * @details
 *  - Set Up:
 无
 *  - Expect:
 1，成功
 *  - Tear Down:
 无
 * @attention
 无
 */
void TimeServiceTest::TimevalToStr001()
{
    struct timeval tv;
    EXPECT_EQ(0, time_service_->Timeval(&tv, NULL));

    char buf[TimeServiceInterface::MAX_TIME_STR_LEN + 1];
    int len = sizeof(buf);

    EXPECT_EQ(0, time_service_->TimevalToStr(buf, len, tv));
    EXPECT_GT(len, 20);
    LOG_CPP("now timeval str: " << buf);
}

/**
 * @brief 获取时间字符串表示的timeval-成功
 * @details
 *  - Set Up:
 1，时间字符串格式正确
 2，时间字符串内容合法
 *  - Expect:
 1，成功
 *  - Tear Down:
 无
 * @attention
 无
 */
void TimeServiceTest::TimevalFromStr001()
{
    char s[] = "2013-10-02 09:00:00 1234";
    time_t t1 = time_service_->MakeTime(2013, 10, 2, 9, 0, 0);
    struct timeval tv2 = time_service_->TimevalFromStr(s);
    EXPECT_EQ(t1, tv2.tv_sec);
    EXPECT_EQ(1234, tv2.tv_usec);
}

/**
 * @brief 检查两个timeval是否是同一天的-同一天的
 * @details
 *  - Set Up:
 1，两个参数是同一天的
 *  - Expect:
 1，返回true
 *  - Tear Down:
 无
 * @attention
 无
 */
void TimeServiceTest::TimevalInSameDay001()
{
    char s1[] = "2013-10-02 09:00:00 1234";
    char s2[] = "2013-10-02 10:00:00 1234";
    struct timeval tv1 = time_service_->TimevalFromStr(s1);
    struct timeval tv2 = time_service_->TimevalFromStr(s2);
    EXPECT_TRUE(time_service_->TimevalInSameDay(tv1, tv2));
}

/**
 * @brief 检查两个timeval是否是同一天的-不是同一天的
 * @details
 *  - Set Up:
 1，两个参数是不是同一天的
 *  - Expect:
 1，返回false
 *  - Tear Down:
 无
 * @attention
 无
 */
void TimeServiceTest::TimevalInSameDay002()
{
    char s1[] = "2013-10-02 09:00:00 1234";
    char s2[] = "2013-10-03 09:00:00 1234";
    struct timeval tv1 = time_service_->TimevalFromStr(s1);
    struct timeval tv2 = time_service_->TimevalFromStr(s2);
    EXPECT_FALSE(time_service_->TimevalInSameDay(tv1, tv2));
}

void TimeServiceTest::TimevalBetween001()
{
    char s[] = "2013-10-02 09:00:00 1234";
    struct timeval tv1 = time_service_->TimevalFromStr(s);
    EXPECT_TRUE(time_service_->TimevalBetween(tv1, 8, 0, 0, 10, 0, 0));
}

void TimeServiceTest::TimevalBetween002()
{
    char s[] = "2013-10-02 09:00:00 1234";
    struct timeval tv1 = time_service_->TimevalFromStr(s);
    EXPECT_FALSE(time_service_->TimevalBetween(tv1, 7, 0, 0, 8, 0, 0));
}

/**
 * @brief 根据年月日时分秒来构造一个格林尼治时间-成功
 * @details
 *  - Set Up:
 1，参数均合法
 *  - Expect:
 1，成功
 *  - Tear Down:
 无
 * @attention
 无
 */
void TimeServiceTest::MakeTime001()
{
    char s[] = "2013-10-02 09:00:00";
    time_t t = time_service_->MakeTime(2013, 10, 2, 9, 0, 0);
    LOG_CPP("time of " << s << ": " << t);

    char buf[TimeServiceInterface::MAX_TIME_STR_LEN + 1];
    int len = sizeof(buf);

    EXPECT_EQ(0, time_service_->TimeToStr(buf, len, t));
    LOG_CPP("str of time '" << t << "': " << buf);
    EXPECT_STREQ(s, buf);
}

/**
 * @brief 根据年月日时分秒来构造一个格林尼治时间-年不合法
 * @details
 *  - Set Up:
 1，年不合法
 *  - Expect:
 1，失败，返回-1
 *  - Tear Down:
 无
 * @attention
 无
 */
void TimeServiceTest::MakeTime002()
{
    time_t t = time_service_->MakeTime(1969, 10, 2, 9, 0, 0);
    EXPECT_EQ((time_t) -1, t);
}

/**
 * @brief 根据年月日时分秒来构造一个格林尼治时间-月不合法
 * @details
 *  - Set Up:
 1，月不合法
 *  - Expect:
 1，失败，返回-1
 *  - Tear Down:
 无
 * @attention
 无
 */
void TimeServiceTest::MakeTime003()
{
    time_t t = time_service_->MakeTime(2013, 0, 2, 9, 0, 0);
    EXPECT_EQ((time_t) -1, t);
}

/**
 * @brief 根据年月日时分秒来构造一个格林尼治时间-日不合法
 * @details
 *  - Set Up:
 1，日不合法
 *  - Expect:
 1，失败，返回-1
 *  - Tear Down:
 无
 * @attention
 无
 */
void TimeServiceTest::MakeTime004()
{
    time_t t = time_service_->MakeTime(2013, 10, 32, 9, 0, 0);
    EXPECT_EQ((time_t) -1, t);
}

/**
 * @brief 根据年月日时分秒来构造一个格林尼治时间-时不合法
 * @details
 *  - Set Up:
 1，时不合法
 *  - Expect:
 1，失败，返回-1
 *  - Tear Down:
 无
 * @attention
 无
 */
void TimeServiceTest::MakeTime005()
{
    time_t t = time_service_->MakeTime(2013, 10, 2, 24, 0, 0);
    EXPECT_EQ((time_t) -1, t);
}

/**
 * @brief 根据年月日时分秒来构造一个格林尼治时间-分不合法
 * @details
 *  - Set Up:
 1，分不合法
 *  - Expect:
 1，成功，会自动推理到合法的值
 *  - Tear Down:
 无
 * @attention
 无
 */
void TimeServiceTest::MakeTime006()
{
    time_t t1 = time_service_->MakeTime(2013, 10, 2, 9, 60, 0);
    EXPECT_EQ((time_t) -1, t1);
}

/**
 * @brief 根据年月日时分秒来构造一个格林尼治时间-秒不合法
 * @details
 *  - Set Up:
 1，秒不合法
 *  - Expect:
 1，失败，返回-1
 *  - Tear Down:
 无
 * @attention
 无
 */
void TimeServiceTest::MakeTime007()
{
    time_t t = time_service_->MakeTime(2013, 10, 2, 9, 0, 60);
    EXPECT_EQ((time_t) -1, t);
}

/**
 * @brief 根据年月日时分秒来构造一个格林尼治时间-月日一起不合法，例如2月没有30号
 * @details
 *  - Set Up:
 1，月日一起不合法
 *  - Expect:
 1，失败，返回-1
 *  - Tear Down:
 无
 * @attention
 无
 */
void TimeServiceTest::MakeTime008()
{
    time_t t = time_service_->MakeTime(2013, 2, 30, 9, 0, 0);
    EXPECT_EQ((time_t) -1, t);
}

/**
 * @brief 根据年月日时分秒来构造一个格林尼治时间-年月日一起不合法，例如某年是平年，2月没有29号
 * @details
 *  - Set Up:
 1，年月日一起不合法
 *  - Expect:
 1，失败，返回-1
 *  - Tear Down:
 无
 * @attention
 无
 */
void TimeServiceTest::MakeTime009()
{
    time_t t = time_service_->MakeTime(2013, 2, 29, 9, 0, 0);
    EXPECT_EQ((time_t) -1, t);
}

/**
 * @brief 获取与指定格林尼治时间同一天的指定时分秒的格林尼治时间表示-成功
 * @details
 *  - Set Up:
 1，参数均合法
 *  - Expect:
 1，成功
 *  - Tear Down:
 无
 * @attention
 无
 */
void TimeServiceTest::MakeTimeD001()
{
    time_t t1 = time_service_->MakeTime(2013, 10, 2, 12, 10, 30);
    time_t t2 = time_service_->MakeTime(t1, 20, 10, 30);
    EXPECT_TRUE(time_service_->TimeInSameDay(t1, t2));
    EXPECT_EQ(8 * TimeServiceInterface::SECS_PER_HOUR, time_service_->TimeDiff(t1, t2));
}

/**
 * @brief 获取与指定格林尼治时间同一天的指定时分秒的格林尼治时间表示-时不合法
 * @details
 *  - Set Up:
 1，时不合法
 *  - Expect:
 1，失败，返回-1
 *  - Tear Down:
 无
 * @attention
 无
 */
void TimeServiceTest::MakeTimeD002()
{
    time_t t1 = time_service_->MakeTime(2013, 10, 2, 12, 10, 30);
    time_t t2 = time_service_->MakeTime(t1, 24, 10, 30);
    EXPECT_EQ(-1, t2);
}

/**
 * @brief 获取与指定格林尼治时间同一天的指定时分秒的格林尼治时间表示-分不合法
 * @details
 *  - Set Up:
 1，分不合法
 *  - Expect:
 1，失败，返回-1
 *  - Tear Down:
 无
 * @attention
 无
 */
void TimeServiceTest::MakeTimeD003()
{
    time_t t1 = time_service_->MakeTime(2013, 10, 2, 12, 10, 30);
    time_t t2 = time_service_->MakeTime(t1, 23, 60, 30);
    EXPECT_EQ(-1, t2);
}

/**
 * @brief 获取与指定格林尼治时间同一天的指定时分秒的格林尼治时间表示-秒不合法
 * @details
 *  - Set Up:
 1，秒不合法
 *  - Expect:
 1，失败，返回-1
 *  - Tear Down:
 无
 * @attention
 无
 */
void TimeServiceTest::MakeTimeD004()
{
    time_t t1 = time_service_->MakeTime(2013, 10, 2, 12, 10, 30);
    time_t t2 = time_service_->MakeTime(t1, 23, 10, 60);
    EXPECT_EQ(-1, t2);
}

/**
 * @brief 获取指定格林尼治时间之后的第一个指定的星期几、时分秒的格林尼治时间表示-成功
 * @details
 *  - Set Up:
 1，参数均合法
 *  - Expect:
 1，成功
 *  - Tear Down:
 无
 * @attention
 无
 */
void TimeServiceTest::MakeTimeW001()
{
    time_t t1 = time_service_->MakeTime(2013, 10, 2, 12, 10, 30); // 星期三

    // 星期相同
    time_t t2 = time_service_->MakeTime(t1, WEEKDAY_WED, 13, 10, 30);
    EXPECT_EQ((time_t) TimeServiceInterface::SECS_PER_HOUR, time_service_->TimeDiff(t1, t2));

    time_t t3 = time_service_->MakeTime(t1, WEEKDAY_WED, 12, 10, 30);
    EXPECT_EQ(7 * TimeServiceInterface::SECS_PER_DAY, time_service_->TimeDiff(t1, t3));

    time_t t4 = time_service_->MakeTime(t1, WEEKDAY_WED, 11, 10, 30);
    EXPECT_EQ(7 * TimeServiceInterface::SECS_PER_DAY - TimeServiceInterface::SECS_PER_HOUR,
              time_service_->TimeDiff(t1, t4));

    // 星期在前-星期二
    time_t t21 = time_service_->MakeTime(t1, WEEKDAY_TUE, 13, 10, 30);
    EXPECT_EQ(6 * TimeServiceInterface::SECS_PER_DAY + TimeServiceInterface::SECS_PER_HOUR,
              time_service_->TimeDiff(t1, t21));

    time_t t31 = time_service_->MakeTime(t1, WEEKDAY_TUE, 12, 10, 30);
    EXPECT_EQ(6 * TimeServiceInterface::SECS_PER_DAY, time_service_->TimeDiff(t1, t31));

    time_t t41 = time_service_->MakeTime(t1, WEEKDAY_TUE, 11, 10, 30);
    EXPECT_EQ(6 * TimeServiceInterface::SECS_PER_DAY - TimeServiceInterface::SECS_PER_HOUR,
              time_service_->TimeDiff(t1, t41));

    // 星期在前-星期天
    time_t t22 = time_service_->MakeTime(t1, WEEKDAY_SUN, 13, 10, 30);
    EXPECT_EQ(4 * TimeServiceInterface::SECS_PER_DAY + TimeServiceInterface::SECS_PER_HOUR,
              time_service_->TimeDiff(t1, t22));

    time_t t32 = time_service_->MakeTime(t1, WEEKDAY_SUN, 12, 10, 30);
    EXPECT_EQ(4 * TimeServiceInterface::SECS_PER_DAY, time_service_->TimeDiff(t1, t32));

    time_t t42 = time_service_->MakeTime(t1, WEEKDAY_SUN, 11, 10, 30);
    EXPECT_EQ(4 * TimeServiceInterface::SECS_PER_DAY - TimeServiceInterface::SECS_PER_HOUR,
              time_service_->TimeDiff(t1, t42));

    // 星期在后-星期四
    time_t t23 = time_service_->MakeTime(t1, WEEKDAY_THU, 13, 10, 30);
    EXPECT_EQ(1 * TimeServiceInterface::SECS_PER_DAY + TimeServiceInterface::SECS_PER_HOUR,
              time_service_->TimeDiff(t1, t23));

    time_t t33 = time_service_->MakeTime(t1, WEEKDAY_THU, 12, 10, 30);
    EXPECT_EQ(1 * TimeServiceInterface::SECS_PER_DAY, time_service_->TimeDiff(t1, t33));

    time_t t43 = time_service_->MakeTime(t1, WEEKDAY_THU, 11, 10, 30);
    EXPECT_EQ(1 * TimeServiceInterface::SECS_PER_DAY - TimeServiceInterface::SECS_PER_HOUR,
              time_service_->TimeDiff(t1, t43));

    // 星期在后-星期六
    time_t t24 = time_service_->MakeTime(t1, WEEKDAY_SAT, 13, 10, 30);
    EXPECT_EQ(3 * TimeServiceInterface::SECS_PER_DAY + TimeServiceInterface::SECS_PER_HOUR,
              time_service_->TimeDiff(t1, t24));

    time_t t34 = time_service_->MakeTime(t1, WEEKDAY_SAT, 12, 10, 30);
    EXPECT_EQ(3 * TimeServiceInterface::SECS_PER_DAY, time_service_->TimeDiff(t1, t34));

    time_t t44 = time_service_->MakeTime(t1, WEEKDAY_SAT, 11, 10, 30);
    EXPECT_EQ(3 * TimeServiceInterface::SECS_PER_DAY - TimeServiceInterface::SECS_PER_HOUR,
              time_service_->TimeDiff(t1, t44));
}

/**
 * @brief 获取指定格林尼治时间之后的第一个指定的星期几、时分秒的格林尼治时间表示-星期不合法
 * @details
 *  - Set Up:
 1，星期不合法
 *  - Expect:
 1，失败，返回-1
 *  - Tear Down:
 无
 * @attention
 无
 */
void TimeServiceTest::MakeTimeW002()
{
    time_t t1 = time_service_->MakeTime(2013, 10, 2, 12, 10, 30); // 星期三
    time_t t2 = time_service_->MakeTime(t1, WEEKDAY_MAX, 13, 10, 30);
    EXPECT_EQ(-1, t2);
}

/**
 * @brief 获取指定格林尼治时间之后的第一个指定的星期几、时分秒的格林尼治时间表示-时不合法
 * @details
 *  - Set Up:
 1，时不合法
 *  - Expect:
 1，失败，返回-1
 *  - Tear Down:
 无
 * @attention
 无
 */
void TimeServiceTest::MakeTimeW003()
{
    time_t t1 = time_service_->MakeTime(2013, 10, 2, 12, 10, 30); // 星期三
    time_t t2 = time_service_->MakeTime(t1, WEEKDAY_FRI, 25, 10, 30);
    EXPECT_EQ(-1, t2);
}

/**
 * @brief 获取指定格林尼治时间之后的第一个指定的星期几、时分秒的格林尼治时间表示-分不合法
 * @details
 *  - Set Up:
 1，分不合法
 *  - Expect:
 1，失败，返回-1
 *  - Tear Down:
 无
 * @attention
 无
 */
void TimeServiceTest::MakeTimeW004()
{
    time_t t1 = time_service_->MakeTime(2013, 10, 2, 12, 10, 30); // 星期三
    time_t t2 = time_service_->MakeTime(t1, WEEKDAY_FRI, 13, 65, 30);
    EXPECT_EQ(-1, t2);
}

/**
 * @brief 获取指定格林尼治时间之后的第一个指定的星期几、时分秒的格林尼治时间表示-秒不合法
 * @details
 *  - Set Up:
 1，秒不合法
 *  - Expect:
 1，失败，返回-1
 *  - Tear Down:
 无
 * @attention
 无
 */
void TimeServiceTest::MakeTimeW005()
{
    time_t t1 = time_service_->MakeTime(2013, 10, 2, 12, 10, 30); // 星期三
    time_t t2 = time_service_->MakeTime(t1, WEEKDAY_FRI, 13, 10, 80);
    EXPECT_EQ(-1, t2);
}

/**
 * @brief 获取自操作系统启动以来的毫秒数
 * @details
 *  - Set Up:
 无
 *  - Expect:
 1，成功
 *  - Tear Down:
 无
 * @attention
 无
 */
void TimeServiceTest::GetTickCount001()
{
    time_t tc = time_service_->GetTickCount();
    LOG_CPP("tick count: " << tc);
}

/**
 * @brief 判断某年是否为闰年-闰年
 * @details
 *  - Set Up:
 1，参数是闰年
 *  - Expect:
 1，返回true
 *  - Tear Down:
 无
 * @attention
 无
 */
void TimeServiceTest::LeapYear001()
{
    EXPECT_TRUE(time_service_->LeapYear(1996));
}

/**
 * @brief 判断某年是否为闰年-不是闰年
 * @details
 *  - Set Up:
 1，参数不是闰年
 *  - Expect:
 1，返回false
 *  - Tear Down:
 无
 * @attention
 无
 */
void TimeServiceTest::LeapYear002()
{
    EXPECT_FALSE(time_service_->LeapYear(2013));
}

ADD_TEST_F(TimeServiceTest, Time001);
ADD_TEST_F(TimeServiceTest, TimeDiff001);
ADD_TEST_F(TimeServiceTest, TimeToStr001);
ADD_TEST_F(TimeServiceTest, TimeFromStr001);
ADD_TEST_F(TimeServiceTest, TimeInSameDay001);
ADD_TEST_F(TimeServiceTest, TimeInSameDay002);
ADD_TEST_F(TimeServiceTest, TimeBetween001);
ADD_TEST_F(TimeServiceTest, TimeBetween002);
ADD_TEST_F(TimeServiceTest, Timeval001);
ADD_TEST_F(TimeServiceTest, TimevalDiff001);
ADD_TEST_F(TimeServiceTest, TimevalToStr001);
ADD_TEST_F(TimeServiceTest, TimevalFromStr001);
ADD_TEST_F(TimeServiceTest, TimevalInSameDay001);
ADD_TEST_F(TimeServiceTest, TimevalInSameDay002);
ADD_TEST_F(TimeServiceTest, TimevalBetween001);
ADD_TEST_F(TimeServiceTest, TimevalBetween002);
ADD_TEST_F(TimeServiceTest, MakeTime001);
ADD_TEST_F(TimeServiceTest, MakeTime002);
ADD_TEST_F(TimeServiceTest, MakeTime003);
ADD_TEST_F(TimeServiceTest, MakeTime004);
ADD_TEST_F(TimeServiceTest, MakeTime005);
ADD_TEST_F(TimeServiceTest, MakeTime006);
ADD_TEST_F(TimeServiceTest, MakeTime007);
ADD_TEST_F(TimeServiceTest, MakeTime008);
ADD_TEST_F(TimeServiceTest, MakeTime009);
ADD_TEST_F(TimeServiceTest, MakeTimeD001);
ADD_TEST_F(TimeServiceTest, MakeTimeD002);
ADD_TEST_F(TimeServiceTest, MakeTimeD003);
ADD_TEST_F(TimeServiceTest, MakeTimeD004);
ADD_TEST_F(TimeServiceTest, MakeTimeW001);
ADD_TEST_F(TimeServiceTest, MakeTimeW002);
ADD_TEST_F(TimeServiceTest, MakeTimeW003);
ADD_TEST_F(TimeServiceTest, MakeTimeW004);
ADD_TEST_F(TimeServiceTest, MakeTimeW005);
ADD_TEST_F(TimeServiceTest, GetTickCount001);
ADD_TEST_F(TimeServiceTest, LeapYear001);
ADD_TEST_F(TimeServiceTest, LeapYear002);
