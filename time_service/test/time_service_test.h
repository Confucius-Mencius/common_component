#ifndef TIME_SERVICE_TEST_TIME_SERVICE_TEST_H_
#define TIME_SERVICE_TEST_TIME_SERVICE_TEST_H_

#include "module_loader.h"
#include "test_util.h"
#include "time_service_interface.h"

class TimeServiceTest : public GTest
{
public:
    TimeServiceTest();
    virtual ~TimeServiceTest();

    virtual void SetUp();
    virtual void TearDown();

    void Time001();
    void TimeDiff001();
    void TimeToStr001();
    void TimeFromStr001();
    void TimeInSameDay001();
    void TimeInSameDay002();
    void TimeBetween001();
    void TimeBetween002();
    void Timeval001();
    void TimevalDiff001();
    void TimevalToStr001();
    void TimevalFromStr001();
    void TimevalInSameDay001();
    void TimevalInSameDay002();
    void TimevalBetween001();
    void TimevalBetween002();
    void MakeTime001();
    void MakeTime002();
    void MakeTime003();
    void MakeTime004();
    void MakeTime005();
    void MakeTime006();
    void MakeTime007();
    void MakeTime008();
    void MakeTime009();
    void MakeTimeD001();
    void MakeTimeD002();
    void MakeTimeD003();
    void MakeTimeD004();
    void MakeTimeW001();
    void MakeTimeW002();
    void MakeTimeW003();
    void MakeTimeW004();
    void MakeTimeW005();
    void GetTickCount001();
    void LeapYear001();
    void LeapYear002();

private:
    ModuleLoader loader_;
    TimeServiceInterface* time_service_;
};

#endif // TIME_SERVICE_TEST_TIME_SERVICE_TEST_H_
