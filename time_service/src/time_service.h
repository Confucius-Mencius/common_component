#ifndef TIME_SERVICE_SRC_TIME_SERVICE_H_
#define TIME_SERVICE_SRC_TIME_SERVICE_H_

#include "time_service_interface.h"

#if (defined(_WIN32) || defined(_WIN64))
#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif
#endif

namespace time_service
{
class TimeService : public TimeServiceInterface
{
public:
    TimeService();
    virtual ~TimeService();

    ///////////////////////// ModuleInterface /////////////////////////
    virtual const char* GetVersion() const;
    virtual const char* GetLastErrMsg() const;
    virtual void Release();
    virtual int Initialize(const void* ctx);
    virtual int Activate();
    virtual void Freeze();
    virtual void Finalize();

    ///////////////////////// TimeServiceInterface /////////////////////////
    virtual time_t Time() const;
    virtual time_t TimeDiff(const time_t& t1, const time_t& t2) const;
    virtual int TimeToStr(char* buf, int& buf_len, const time_t& t) const;
    virtual time_t TimeFromStr(const char* str) const;
    virtual bool TimeInSameDay(const time_t& t1, const time_t& t2) const;
    virtual bool TimeBetween(const time_t& t, int hour1, int min1, int sec1, int hour2, int min2, int sec2) const;
    virtual int Timeval(struct timeval* tv, struct timezone* tz) const;
    virtual struct timeval TimevalDiff(const struct timeval& tv1, const struct timeval& tv2) const;
    virtual int TimevalToStr(char* buf, int& buf_len, const struct timeval& tv) const;
    virtual struct timeval TimevalFromStr(const char* str) const;
    virtual bool TimevalInSameDay(const struct timeval& tv1, const struct timeval& tv2) const;
    virtual bool TimevalBetween(const struct timeval& tv, int hour1, int min1, int sec1, int hour2, int min2,
                                int sec2) const;
    virtual time_t MakeTime(int year, int mon, int day, int hour, int min, int sec) const;
    virtual time_t MakeTime(const time_t& t, int hour, int min, int sec) const;
    virtual time_t MakeTime(const time_t& t, Weekday weekday, int hour, int min, int sec) const;
    virtual time_t GetTickCount() const;
    virtual bool LeapYear(int year) const;
    virtual void SleepSec(int sec);
    virtual void SleepMillisec(int millisec);
};
}

#endif // TIME_SERVICE_SRC_TIME_SERVICE_H_
