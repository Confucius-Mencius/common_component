#include "time_service.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "version.h"

namespace time_service
{
static const int BASE_YEAR = 1900;
static const int BASE_MONTH = 1;

TimeService::TimeService()
{
}

TimeService::~TimeService()
{
}

const char* TimeService::GetVersion() const
{
    return TIME_SERVICE_TIME_SERVICE_VERSION;
}

const char* TimeService::GetLastErrMsg() const
{
    return NULL;
}

void TimeService::Release()
{
    delete this;
}

int TimeService::Initialize(const void* ctx)
{
    return 0;
}

void TimeService::Finalize()
{
}

int TimeService::Activate()
{
    return 0;
}

void TimeService::Freeze()
{
}

time_t TimeService::Time() const
{
    return time(NULL);
}

time_t TimeService::TimeDiff(const time_t& t1, const time_t& t2) const
{
    return t2 - t1;
}

int TimeService::TimeToStr(char* buf, int& buf_len, const time_t& t) const
{
    struct tm stm;

    if (NULL == localtime_r(&t, &stm))
    {
        return -1;
    }

    int n = snprintf(buf, buf_len, "%04d-%02d-%02d %02d:%02d:%02d", stm.tm_year + BASE_YEAR, stm.tm_mon + BASE_MONTH,
                     stm.tm_mday, stm.tm_hour, stm.tm_min, stm.tm_sec);
    buf[n] = '\0';

    buf_len = n;
    return 0;
}

time_t TimeService::TimeFromStr(const char* str) const
{
    if (NULL == str || strnlen(str, MAX_TIME_STR_LEN) != 19)
    {
        return -1;
    }

    struct tm stm;
    time_t t;

    int n = sscanf(str, "%4d-%2d-%2d %2d:%2d:%2d", &stm.tm_year, &stm.tm_mon, &stm.tm_mday, &stm.tm_hour, &stm.tm_min,
                   &stm.tm_sec);
    (void) n;

    stm.tm_year -= BASE_YEAR;
    stm.tm_mon--;
    stm.tm_isdst = -1;

    t = mktime(&stm);
    return t;
}

bool TimeService::TimeInSameDay(const time_t& t1, const time_t& t2) const
{
    struct tm stm1;
    struct tm stm2;

    if (NULL == localtime_r(&t1, &stm1))
    {
        return false;
    }

    if (NULL == localtime_r(&t2, &stm2))
    {
        return false;
    }

    if ((stm1.tm_year == stm2.tm_year) && (stm1.tm_yday == stm2.tm_yday))
    {
        return true;
    }

    return false;
}

int TimeService::Timeval(struct timeval* tv, struct timezone* tz) const
{
    if (NULL == tv)
    {
        return -1;
    }

#if (defined(__linux__))
    int ret = gettimeofday(tv, tz);
    if (ret != 0)
    {
        return -1;
    }
#elif (defined(_WIN32) || defined(_WIN64))
    // Define a structure to receive the current Windows filetime
    FILETIME ft;

    // Initialize the present time to 0 and the timezone to UTC
    unsigned __int64 tmpres = 0;
    static int tzflag = 0;

    if (NULL != tv)
    {
        GetSystemTimeAsFileTime(&ft);

        // The GetSystemTimeAsFileTime returns the number of 100 nanosecond
        // intervals since Jan 1, 1601 in a structure. Copy the high bits to
        // the 64 bit tmpres, shift it left by 32 then or in the low 32 bits.
        tmpres |= ft.dwHighDateTime;
        tmpres <<= 32;
        tmpres |= ft.dwLowDateTime;

        // Convert to microseconds by dividing by 10
        tmpres /= 10;

        // The Unix epoch starts on Jan 1 1970.  Need to subtract the difference
        // in seconds from Jan 1 1601.
        tmpres -= DELTA_EPOCH_IN_MICROSECS;

        // Finally change microseconds to seconds and place in the seconds value.
        // The modulus picks up the microseconds.
        tv->tv_sec = (long) (tmpres / 1000000UL);
        tv->tv_usec = (long) (tmpres % 1000000UL);
    }

    if (NULL != tz)
    {
        if (!tzflag)
        {
            _tzset();
            tzflag++;
        }

        // Adjust for the timezone west of Greenwich
        tz->tz_minuteswest = _timezone / 60;
        tz->tz_dsttime = _daylight;
    }
#endif

    return 0;
}

struct timeval TimeService::TimevalDiff(const struct timeval& tv1, const struct timeval& tv2) const
{
    struct timeval tv;

    tv.tv_sec = tv2.tv_sec - tv1.tv_sec;
    tv.tv_usec = tv2.tv_usec - tv1.tv_usec;

    return tv;
}

int TimeService::TimevalToStr(char* buf, int& buf_len, const struct timeval& tv) const
{
    time_t t = (time_t) tv.tv_sec;
    struct tm stm;

    if (NULL == localtime_r(&t, &stm))
    {
        return -1;
    }

    int n = snprintf(buf, buf_len, "%04d-%02d-%02d %02d:%02d:%02d %ld", stm.tm_year + BASE_YEAR,
                     stm.tm_mon + BASE_MONTH, stm.tm_mday, stm.tm_hour, stm.tm_min, stm.tm_sec, tv.tv_usec);
    buf[n] = '\0';

    buf_len = n;
    return 0;
}

struct timeval TimeService::TimevalFromStr(const char* str) const
{
    struct timeval tv;

    if (NULL == str || strnlen(str, MAX_TIME_STR_LEN) <= 20)
    {
        tv.tv_sec = -1;
        tv.tv_usec = -1;
        return tv;
    }

    struct tm stm;

    int n = sscanf(str, "%4d-%2d-%2d %2d:%2d:%2d %ld", &stm.tm_year, &stm.tm_mon, &stm.tm_mday, &stm.tm_hour,
                   &stm.tm_min, &stm.tm_sec, &tv.tv_usec);
    (void) n;

    stm.tm_year -= BASE_YEAR;
    stm.tm_mon--;
    stm.tm_isdst = -1;

    tv.tv_sec = mktime(&stm);
    return tv;
}

bool TimeService::TimevalInSameDay(const struct timeval& tv1, const struct timeval& tv2) const
{
    const time_t t1 = tv1.tv_sec;
    const time_t t2 = tv2.tv_sec;

    return TimeInSameDay(t1, t2);
}

bool TimeService::TimevalBetween(const struct timeval& tv, int hour1, int min1, int sec1, int hour2, int min2,
                                 int sec2) const
{
    const time_t t = tv.tv_sec;
    return TimeBetween(t, hour1, min1, sec1, hour2, min2, sec2);
}

time_t TimeService::MakeTime(int year, int mon, int day, int hour, int min, int sec) const
{
    // 参数检查，必须要在1970年1月1日00::00:00之后，时分秒注意是24小时制
    if (year < 1970 || mon < 1 || mon > 12 || day < 1 || day > 31 || hour < 0 || hour > 23 || min < 0
            || min > 59 || sec < 0 || sec > 59)
    {
        return -1;
    }

    struct tm stm;

    // 检查年月日是否是合法的一天
    const int month_days_leap[13] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    const int month_days_no_leap[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    int month_days = (LeapYear(year) ? month_days_leap[mon] : month_days_no_leap[mon]);
    if (day > month_days)
    {
        return -1;
    }

    memset(&stm, 0, sizeof(stm));
    stm.tm_year = year - BASE_YEAR;
    stm.tm_mon = mon - BASE_MONTH;
    stm.tm_mday = day;
    stm.tm_hour = hour;
    stm.tm_min = min;
    stm.tm_sec = sec;

    return mktime(&stm);
}

time_t TimeService::MakeTime(const time_t& t, int hour, int min, int sec) const
{
    // 时分秒参数检查，注意是24小时制
    if (hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 || sec > 59)
    {
        return -1;
    }

    struct tm stm;

    if (NULL == localtime_r(&t, &stm))
    {
        return -1;
    }

    stm.tm_hour = hour;
    stm.tm_min = min;
    stm.tm_sec = sec;

    return mktime(&stm);
}

time_t TimeService::MakeTime(const time_t& t, Weekday weekday, int hour, int min, int sec) const
{
    if (weekday < WEEKDAY_MIN || weekday >= WEEKDAY_MAX)
    {
        return -1;
    }

    if (hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 || sec > 59)
    {
        return -1;
    }

    struct tm base;

    if (NULL == localtime_r(&t, &base))
    {
        return -1;
    }

    // weekday保持不变，获取修改时、分、秒后的time_t值
    struct tm stm = base;
    stm.tm_hour = hour;
    stm.tm_min = min;
    stm.tm_sec = sec;
    time_t v = mktime(&stm);

    // TODO 跨月跨年要考虑,这里可能有bug

    const int base_weekday = base.tm_wday; // 星期 – 取值区间为[0,6]，其中0代表星期天，1代表星期一，以此类推
    int delta_days = 0; // 相差的天数

    if (weekday == base_weekday)
    {
        if (v <= t)
        {
            delta_days = DAYS_ONE_WEEK;
        }
        else
        {
            delta_days = 0;
        }
    }
    else if (weekday < base_weekday)
    {
        delta_days = DAYS_ONE_WEEK - (base_weekday - weekday);
    }
    else
    {
        delta_days = weekday - base_weekday;
    }

    return v + delta_days * SECS_PER_DAY;
}

bool TimeService::TimeBetween(const time_t& t, int hour1, int min1, int sec1, int hour2, int min2, int sec2) const
{
    time_t t1 = MakeTime(t, hour1, min1, sec1);
    time_t t2 = MakeTime(t, hour2, min2, sec2);

    if (t >= t1 && t <= t2)
    {
        return true;
    }

    return false;
}

time_t TimeService::GetTickCount() const
{
#if (defined(__linux__))
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * ((int64_t) 1000) + ts.tv_nsec / ((int64_t) 1000000);
#elif (defined(_WIN32) || defined(_WIN64))
    return ::GetTickCount64();
#endif
}

bool TimeService::LeapYear(int year) const
{
    if (year < 0)
    {
        return false;
    }

    return ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0));
}

void TimeService::SleepSec(int sec)
{
    sleep(sec);
}

void TimeService::SleepMillisec(int millisec)
{
    if (millisec >= 1000)
    {
        return;
    }

#if (defined(__linux__))
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 1000000 * millisec;
    nanosleep(&ts, NULL);
#elif (defined(_WIN32) || defined(_WIN64))
    Sleep(millisec);
#endif
}
}
