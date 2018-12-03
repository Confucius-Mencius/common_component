/**
 * @file time_service_interface.h
 * @brief 时间相关接口
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef TIME_SERVICE_INC_TIME_SERVICE_INTERFACE_H_
#define TIME_SERVICE_INC_TIME_SERVICE_INTERFACE_H_

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_TimeService 时间服务
 * @{
 */

#include <sys/time.h>
#include <time.h>
#include "module_interface.h"

#if (defined(_WIN32) || defined(_WIN64))
struct timezone
{
    int tz_minuteswest; // minutes W of Greenwich
    int tz_dsttime;// type of dst correction
};
#endif

enum Weekday
{
    WEEKDAY_MIN = 0,
    WEEKDAY_SUN = WEEKDAY_MIN,
    WEEKDAY_MON,
    WEEKDAY_TUE,
    WEEKDAY_WED,
    WEEKDAY_THU,
    WEEKDAY_FRI,
    WEEKDAY_SAT,
    WEEKDAY_MAX
};

class TimeServiceInterface : public ModuleInterface
{
public:
    static const time_t SECS_PER_MIN = 60;
    static const time_t SECS_PER_HOUR = 3600;
    static const time_t SECS_PER_DAY = 86400;
    static const int DAYS_ONE_WEEK = WEEKDAY_MAX;
    static const int MAX_TIME_STR_LEN = 31;

public:
    virtual ~TimeServiceInterface()
    {
    }

    /**
     * @brief 获取当前时间的格林尼治表示
     * @return 格林尼治时间，即自1970-01-01 00:00:00以来的秒数
     */
    virtual time_t Time() const = 0;

    /**
     * @brief 计算两个格林尼治时间的差值
     * @param t1 时间值1
     * @param t2 时间值2
     * @return 时间差值(t2 - t1)
     */
    virtual time_t TimeDiff(const time_t& t1, const time_t& t2) const = 0;

    /**
     * @brief 获取指定格林尼治时间的字符串表示
     * @param buf 指定时间的字符串表示，格式为：YYYY-MM-DD HH:MM:SS
     * @param buf_len 时间字符串的长度
     * @param t 格林尼治时间
     * @return 返回0表示成功
     * @see TimeFromStr
     */
    virtual int TimeToStr(char* buf, int& buf_len, const time_t& t) const = 0;

    /**
     * @brief 将时间字符串（通过TimeToStr得到的）转换成格林尼治时间
     * @param str 格式说明：YYYY-MM-DD HH:MM:SS
     * @return 指定时间字符串的格林尼治表示，如果参数不是一个合法的时间字符串表示，则返回-1
     * @see TimeToStr
     */
    virtual time_t TimeFromStr(const char* str) const = 0;

    /**
     * @brief 检查两个格林尼治时间是否是同一天的
     * @param t1 时间1
     * @param t2 时间2
     * @return 如果两个时间是同一天的则返回true，否则返回false
     */
    virtual bool TimeInSameDay(const time_t& t1, const time_t& t2) const = 0;

    /**
     * @brief 判断指定的格林尼治时间是否在指定的时分秒时间段之间
     * @param t 指定时间
     * @param hour1 起始时，取值范围：[0, 23]
     * @param min1 起始分，取值范围：[0, 59]
     * @param sec1 起始秒，取值范围：[0, 59]
     * @param hour2 结束时，取值范围：[0, 23]
     * @param min2 结束分，取值范围：[0, 59]
     * @param sec2 结束秒，取值范围：[0, 59]
     * @return 如果t在两个时间之间则返回true
     * @note 前后都是闭区间
     */
    virtual bool TimeBetween(const time_t& t, int hour1, int min1, int sec1, int hour2, int min2, int sec2) const = 0;

    /**
     * @brief 获取struct timeval表示的格林尼治时间，精度更高，可以到微妙
     * @param tv 格林尼治时间
     * @param tz 时区，可以为NULL
     * @return 返回0表示成功
     */
    virtual int Timeval(struct timeval* tv, struct timezone* tz) const = 0;

    /**
     * @brief 计算两个struct timeval表示的格林尼治时间的差值
     * @param tv1 时间值1
     * @param tv2 时间值2
     * @return 时间差值(tv2 - tv1)
     */
    virtual struct timeval TimevalDiff(const struct timeval& tv1, const struct timeval& tv2) const = 0;

    /**
     * @brief 获取struct timeval表示的格林尼治时间的字符串表示
     * @param buf 指定时间的字符串表示，格式为：YYYY-MM-DD HH:MM:SS Microseconds
     * @param buf_len 时间字符串的长度
     * @param tv 格林尼治时间
     * @return 返回0表示成功
     * @see TimevalFromStr
     */
    virtual int TimevalToStr(char* buf, int& buf_len, const struct timeval& tv) const = 0;

    /**
     * @brief 将时间字符串（通过TimevalToStr得到的）转换成timeval结构
     * @param str 格式说明：YYYY-MM-DD HH:MM:SS Microseconds
     * @return 指定时间字符串的timeval表示，如果参数不是一个合法的时间字符串表示，则返回值中的字段均为-1
     * @see TimevalToStr
     */
    virtual struct timeval TimevalFromStr(const char* str) const = 0;

    /**
     * @brief 检查两个struct timeval表示的格林尼治时间是否是同一天的
     * @param tv1 时间1
     * @param tv2 时间2
     * @return 如果两个时间是同一天的则返回true
     */
    virtual bool TimevalInSameDay(const struct timeval& tv1, const struct timeval& tv2) const = 0;

    /**
     * @brief 判断指定的struct timeval是否在指定的时分秒时间段之间
     * @param tv 指定时间
     * @param hour1 起始时，取值范围：[0, 23]
     * @param min1 起始分，取值范围：[0, 59]
     * @param sec1 起始秒，取值范围：[0, 59]
     * @param hour2 结束时，取值范围：[0, 23]
     * @param min2 结束分，取值范围：[0, 59]
     * @param sec2 结束秒，取值范围：[0, 59]
     * @return 如果tv在两个时间之间则返回true
     * @note 前后都是闭区间
     */
    virtual bool TimevalBetween(const struct timeval& tv, int hour1, int min1, int sec1, int hour2, int min2,
                                int sec2) const = 0;

    /**
     * @brief 获取指定年月日时分秒的格林尼治时间表示
     * @param year 年，取值范围：[1970, ]
     * @param mon 月，取值范围：[1, 12]
     * @param day 日，取值范围：[1, 31]，还要看大月/小月/润月
     * @param hour 时，取值范围：[0, 23]
     * @param min 分，取值范围：[0, 59]
     * @param sec 秒，取值范围：[0, 59]
     * @return 指定时间的格林尼治表示，如果参数不是一个合法的时间表示，则返回-1
     */
    virtual time_t MakeTime(int year, int mon, int day, int hour, int min, int sec) const = 0;

    /**
     * @brief 获取与指定格林尼治时间同一天的指定时分秒的格林尼治时间表示
     * @param t 格林尼治时间
     * @param hour 时，取值范围：[0, 23]
     * @param min 分，取值范围：[0, 59]
     * @param sec 秒，取值范围：[0, 59]
     * @return 指定时间的格林尼治表示，如果参数不是一个合法的时间表示，则返回-1
     * @note 如果想获取后一天的指定时分秒，可以先获取当天的，然后加上1天的秒数
     */
    virtual time_t MakeTime(const time_t& t, int hour, int min, int sec) const = 0;

    /**
     * @brief 获取指定格林尼治时间之后的第一个指定的星期几、时分秒的格林尼治时间表示
     * @param t 格林尼治时间，从这个时间之后算起
     * @param weekday 星期几
     * @param hout 时，取值范围为：[0, 23]
     * @param min 分，取值范围为：[0, 59]
     * @param sec 秒，取值范围为：[0, 59]
     * @return 指定时间的格林尼治表示，如果参数不是一个合法的时间表示，则返回-1
     */
    virtual time_t MakeTime(const time_t& t, Weekday weekday, int hour, int min, int sec) const = 0;

    /**
     * @brief 获取自操作系统启动以来的毫秒数
     * @return 自操作系统启动以来的毫秒数，称之为tick
     * @note Link with -lrt on Linux.
     */
    virtual time_t GetTickCount() const = 0;

    /**
     * @brief 判断某年是否为闰年
     * @param year 年份
     * @return 如果为闰年则返回true
     */
    virtual bool LeapYear(int year) const = 0;

    /**
     * @brief sleep一定的秒数
     * @param sec 秒数
     * @attention sec
     */
    virtual void SleepSec(int sec) = 0;

    /**
     * @brief sleep一定的毫秒数
     * @param millisec 毫秒数
     * @attention millisec 必须小于1000，即1秒
     */
    virtual void SleepMillisec(int millisec) = 0;
};

/** @} Module_TimeService */
/** @} Module_Base */

#endif // TIME_SERVICE_INC_TIME_SERVICE_INTERFACE_H_
