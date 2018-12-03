#include "test_util.h"
#include <chrono>

namespace time_test
{
void Test001()
{
//    std::chrono::duration 表示一段时间
//    template <class Rep, class Period = ratio<1> > class duration;
//    Rep表示一种数值类型，用来表示Period的数量，比如int float double
//    Period是ratio类型，用来表示【用秒表示的时间单位】比如second milisecond
//    常用的duration<Rep,Period>已经定义好了，在std::chrono::duration下：
//    ratio<3600, 1>                hours
//    ratio<60, 1>                    minutes
//    ratio<1, 1>                      seconds
//    ratio<1, 1000>               microseconds
//    ratio<1, 1000000>         microseconds
//    ratio<1, 1000000000>    nanosecons
//
//    template <intmax_t N, intmax_t D = 1> class ratio;
//    N代表分子，D代表分母，所以ratio表示一个分数值。

    typedef std::chrono::duration<int> seconds_type;
    typedef std::chrono::duration<int, std::milli> milliseconds_type;
    typedef std::chrono::duration<int, std::ratio<60 * 60>> hours_type;

    hours_type h_oneday(24);                  // 24h
    seconds_type s_oneday(60 * 60 * 24);          // 86400s
    milliseconds_type ms_oneday(s_oneday);    // 86400000ms

    seconds_type s_onehour(60 * 60);            // 3600s
    //hours_type h_onehour (s_onehour);          // NOT VALID (type truncates), use:
    hours_type h_onehour(std::chrono::duration_cast<hours_type>(s_onehour));
    milliseconds_type ms_onehour(s_onehour);  // 3600000ms (ok, no type truncation)

    std::cout << ms_onehour.count() << "ms in 1h" << std::endl;


//    duration还有一个成员函数count()返回Rep类型的Period数量，看代码：
    using namespace std::chrono;
    // std::chrono::milliseconds is an instatiation of std::chrono::duration:
    milliseconds foo(1000); // 1 second
    foo *= 60;

    std::cout << "duration (in periods): ";
    std::cout << foo.count() << " milliseconds.\n";

    std::cout << "duration (in seconds): ";
    std::cout << foo.count() * milliseconds::period::num / milliseconds::period::den;
    std::cout << " seconds.\n";
}

void Test002()
{
//    std::chrono::time_point 表示一个具体时间，一个time point必须有一个clock计时
//    template <class Clock, class Duration = typename Clock::duration>  class time_point;
    using namespace std::chrono;

    system_clock::time_point tp_epoch;    // epoch value

    time_point<system_clock, duration<int>> tp_seconds(duration<int>(1));

    system_clock::time_point tp(tp_seconds);

    std::cout << "1 second since system_clock epoch = ";
    std::cout << tp.time_since_epoch().count();
    std::cout << " system_clock periods." << std::endl;

    // display time_point:
    std::time_t tt = system_clock::to_time_t(tp);
    std::cout << "time_point tp is: " << ctime(&tt);

//    time_point有一个函数time_from_eproch()用来获得1970年1月1日到time_point时间经过的duration。
//    举个例子，如果timepoint以天为单位，函数返回的duration就以天为单位。
    typedef duration<int, std::ratio<60 * 60 * 24>> days_type;

    time_point<system_clock, days_type> today = time_point_cast<days_type>(system_clock::now());

    std::cout << today.time_since_epoch().count() << " days since epoch" << std::endl;
}

void Test003()
{
//    std::chrono::system_clock 它表示当前的系统时钟，系统中运行的所有进程使用now()得到的时间是一致的。
//    每一个clock类中都有确定的time_point, duration, Rep, Period类型。
//    操作有：
//    now() 当前时间time_point
//    to_time_t() time_point转换成time_t秒
//    from_time_t() 从time_t转换成time_point

    using std::chrono::system_clock;

    std::chrono::duration<int, std::ratio<60 * 60 * 24> > one_day(1);

    system_clock::time_point today = system_clock::now();
    system_clock::time_point tomorrow = today + one_day;

    std::time_t tt;

    tt = system_clock::to_time_t(today);
    std::cout << "today is: " << ctime(&tt);

    tt = system_clock::to_time_t(tomorrow);
    std::cout << "tomorrow will be: " << ctime(&tt);
}

void Test004()
{
//    system_clock不适合来计算时间间隔，因为系统时间可以改变的。
//    std::chrono::steady_clock 为了表示稳定的时间间隔，后一次调用now()得到的时间总是比前一次的值大
// （这句话的意思其实是，如果中途修改了系统时间，也不影响now()的结果），每次tick都保证过了稳定的时间间隔。
    using namespace std::chrono;

    steady_clock::time_point t1 = steady_clock::now();

    std::cout << "printing out 1000 stars...\n";
    for (int i = 0; i < 1000; ++i)
    { std::cout << "*"; }
    std::cout << std::endl;

    steady_clock::time_point t2 = steady_clock::now();

    duration<double> time_span = duration_cast<duration<double>>(t2 - t1);

    std::cout << "It took me " << time_span.count() << " seconds.";
    std::cout << std::endl;

//    steady_clock::period是个有理数，表示steady_clock计时最短时间间隔，单位为秒
    std::cout << steady_clock::period::num << std::endl; //num 和 den分别表示分子(numerator)和分母(denominator)
    std::cout << steady_clock::period::den << std::endl; // 这里打印出来的是1和10000000，也就是1/10,000,000之一秒，100 nano seconds。
}

void Test005()
{
//    std::chrono::high_resolution_clock 顾名思义，这是系统可用的最高精度的时钟。
    std::chrono::high_resolution_clock hrclock;
    hrclock.now();
}

void Test006()
{
    using heartbeats = std::chrono::duration<int, std::ratio<3, 4>>; // std::ratio < 3, 4 >表示每四分之三秒跳一次。

//    定义一个用浮点计数的minutes类型，计算跳动123次等于多少分钟。
    using minutes = std::chrono::duration<double, std::ratio<60> >;
    heartbeats beat(123);
    std::cout << minutes(beat).count() << std::endl;

//    用std::get_time和std::put_time处理时间和字符串。
}

ADD_TEST(TimeTest, Test001);
ADD_TEST(TimeTest, Test002);
ADD_TEST(TimeTest, Test003);
ADD_TEST(TimeTest, Test004);
ADD_TEST(TimeTest, Test005);
ADD_TEST(TimeTest, Test006);
}
