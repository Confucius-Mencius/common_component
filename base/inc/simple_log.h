/**
 * @file simple_log.h
 * @brief 日志宏
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef BASE_INC_SIMPLE_LOG_H_
#define BASE_INC_SIMPLE_LOG_H_

#include <sys/time.h>
#include <pthread.h>

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_SimpleLog SimpleLog
 * @{
 */

extern pthread_mutex_t g_simple_log_mutex;

/**
 * @brief C语言日志宏
 * @details 使用方式同printf，其中会打印文件名、行号、函数名，并自动换行
 * @param [in] format 格式化字符串，同printf
 * @param [in] ... 变参
 * @par 示例：
 * @code
 *  LOG_C("%d %s", 1, "hello");
 * @endcode
 * @hideinitializer
 */
#define LOG_C(format, ...)\
do {\
    struct timeval tv;\
    gettimeofday(&tv, NULL);\
    struct tm* p = localtime(&tv.tv_sec);\
    pthread_mutex_lock(&g_simple_log_mutex);\
    fprintf(stdout, "<%04d-%02d-%02d %02d:%02d:%02d %03ld %s:%d %s %lu> " format "\n", (1900 + p->tm_year), (1 + p->tm_mon), \
        p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, tv.tv_usec / 1000, __FILE__, __LINE__, __PRETTY_FUNCTION__, pthread_self(), ##__VA_ARGS__);\
    pthread_mutex_unlock(&g_simple_log_mutex);\
} while (0)

#define LOG_CE(format, ...)\
do {\
    struct timeval tv;\
    gettimeofday(&tv, NULL);\
    struct tm* p = localtime(&tv.tv_sec);\
    pthread_mutex_lock(&g_simple_log_mutex);\
    fprintf(stderr, "<%04d-%02d-%02d %02d:%02d:%02d %03ld %s:%d %s %lu> " format "\n", (1900 + p->tm_year), (1 + p->tm_mon), \
        p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, tv.tv_usec / 1000, __FILE__, __LINE__, __PRETTY_FUNCTION__, pthread_self(), ##__VA_ARGS__);\
} while (0)

/**
 * @brief C++语言日志宏
 * @details 使用方式同cout，其中会打印文件名、行号、函数名，并自动换行
 * @param [in] info_ostream 要打印的信息，是一个输出流的形式
 * @par 示例：
 * @code
 *  LOG_CPP(1 << "hello");
 * @endcode
 * @hideinitializer
 */
#define LOG_CPP(info_ostream)\
do {\
    struct timeval tv;\
    gettimeofday(&tv, NULL);\
    struct tm* p = localtime(&tv.tv_sec);\
    std::ostringstream result("");\
    result << std::setfill('0') << "<" << std::setw(4) << (1900 + p->tm_year) << "-" << std::setw(2) << (1 + p->tm_mon) << "-" << std::setw(2) << p->tm_mday\
        << " " << std::setw(2) << p->tm_hour << ":" << std::setw(2) << p->tm_min << ":" << std::setw(2) << p->tm_sec << " " << std::setw(3) << tv.tv_usec / 1000\
        << " " << __FILE__ << ":" << __LINE__ << " " << __PRETTY_FUNCTION__ <<  " " << pthread_self() << "> " << info_ostream;\
    pthread_mutex_lock(&g_simple_log_mutex);\
    std::cout << result.str() << std::endl;\
    pthread_mutex_unlock(&g_simple_log_mutex);\
} while (0)

#define LOG_CPPE(info_ostream)\
do {\
    struct timeval tv;\
    gettimeofday(&tv, NULL);\
    struct tm* p = localtime(&tv.tv_sec);\
    std::ostringstream result("");\
    result << std::setfill('0') << "<" << std::setw(4) << (1900 + p->tm_year) << "-" << std::setw(2) << (1 + p->tm_mon) << "-" << std::setw(2) << p->tm_mday\
        << " " << std::setw(2) << p->tm_hour << ":" << std::setw(2) << p->tm_min << ":" << std::setw(2) << p->tm_sec << " " << std::setw(3) << tv.tv_usec / 1000\
        << " " << __FILE__ << ":" << __LINE__ << " " << __PRETTY_FUNCTION__ << " " << pthread_self() << "> " << info_ostream;\
    pthread_mutex_lock(&g_simple_log_mutex);\
    std::cerr << result.str() << std::endl;\
    pthread_mutex_unlock(&g_simple_log_mutex);\
} while (0)

/** @} Module_SimpleLog */
/** @} Module_Base */

#endif // BASE_INC_SIMPLE_LOG_H_
