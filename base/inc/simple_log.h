/**
 * @file simple_log.h
 * @brief 日志宏
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef BASE_INC_SIMPLE_LOG_H_
#define BASE_INC_SIMPLE_LOG_H_

#include <pthread.h>
#include <sys/time.h>
#include <iomanip>

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_SimpleLog SimpleLog
 * @{
 */

/**
  * 是否打印上下文，包括文件名、行号、函数名、线程id等。
  * 如果不想打印这些信息，在#include "simplg_log.h"之前#define LOG_WITH_CONTEXT 0即可。
  */
#ifndef LOG_WITH_CONTEXT
#define LOG_WITH_CONTEXT 1
#endif // LOG_WITH_CONTEXT

extern pthread_mutex_t g_simple_log_mutex;

#if LOG_WITH_CONTEXT
/**
 * @brief C语言日志宏
 * @details 使用方式同printf，其中会打印文件名、行号、函数名，自动换行。输出到stdout
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
        fprintf(stdout, "<%#lX %04d-%02d-%02d %02d:%02d:%02d %03ld %s:%d %s> " format "\n", pthread_self(), (1900 + p->tm_year), (1 + p->tm_mon), \
                p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, tv.tv_usec / 1000, basename(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__);\
        pthread_mutex_unlock(&g_simple_log_mutex);\
    } while (0)

/**
  * @details 功能同LOG_C，但是输出到stderr
  */
#define LOG_CE(format, ...)\
    do {\
        struct timeval tv;\
        gettimeofday(&tv, NULL);\
        struct tm* p = localtime(&tv.tv_sec);\
        pthread_mutex_lock(&g_simple_log_mutex);\
        fprintf(stderr, "<%#lX %04d-%02d-%02d %02d:%02d:%02d %03ld %s:%d %s> " format "\n", pthread_self(), (1900 + p->tm_year), (1 + p->tm_mon), \
                p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, tv.tv_usec / 1000, basename(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__);\
        pthread_mutex_unlock(&g_simple_log_mutex);\
    } while (0)

/**
 * @brief C++语言日志宏
 * @details 使用方式同cout，其中会打印文件名、行号、函数名，自动换行。输出到stdout
 * @param [in] msg 要打印的信息，是一个输出流的形式
 * @par 示例：
 * @code
 *  LOG_CPP(1 << "hello");
 * @endcode
 * @hideinitializer
 */
#define LOG_CPP(msg)\
    do {\
        struct timeval tv;\
        gettimeofday(&tv, NULL);\
        struct tm* p = localtime(&tv.tv_sec);\
        std::ostringstream result("");\
        result << "<" << std::hex << std::showbase << setiosflags(std::ios::uppercase) << pthread_self() << " "\
               << std::dec << std::setfill('0') << std::setw(4) << (1900 + p->tm_year) << "-" << std::setw(2) << (1 + p->tm_mon) << "-" << std::setw(2) << p->tm_mday\
               << " " << std::setw(2) << p->tm_hour << ":" << std::setw(2) << p->tm_min << ":" << std::setw(2) << p->tm_sec << " " << std::setw(3) << tv.tv_usec / 1000\
               << " " << basename(__FILE__) << ":" << __LINE__ << " " << __FUNCTION__ << "> " << msg;\
        pthread_mutex_lock(&g_simple_log_mutex);\
        std::cout << result.str() << std::endl;\
        pthread_mutex_unlock(&g_simple_log_mutex);\
    } while (0)

/**
  * @details 功能同LOG_CPP，但是输出到stderr
  */
#define LOG_CPPE(msg)\
    do {\
        struct timeval tv;\
        gettimeofday(&tv, NULL);\
        struct tm* p = localtime(&tv.tv_sec);\
        std::ostringstream result("");\
        result << "<" << std::hex << std::showbase << setiosflags(std::ios::uppercase) << pthread_self() << " "\
               << std::dec << std::setfill('0') << std::setw(4) << (1900 + p->tm_year) << "-" << std::setw(2) << (1 + p->tm_mon) << "-" << std::setw(2) << p->tm_mday\
               << " " << std::setw(2) << p->tm_hour << ":" << std::setw(2) << p->tm_min << ":" << std::setw(2) << p->tm_sec << " " << std::setw(3) << tv.tv_usec / 1000\
               << " " << basename(__FILE__) << ":" << __LINE__ << " " << __FUNCTION__ << "> " << msg;\
        pthread_mutex_lock(&g_simple_log_mutex);\
        std::cerr << result.str() << std::endl;\
        pthread_mutex_unlock(&g_simple_log_mutex);\
    } while (0)
#else
/**
 * @brief C语言日志宏
 * @details 使用方式同printf，其中不会打印文件名、行号、函数名，自动换行。输出到stdout
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
        pthread_mutex_lock(&g_simple_log_mutex);\
        fprintf(stdout, format "\n", ##__VA_ARGS__);\
        pthread_mutex_unlock(&g_simple_log_mutex);\
    } while (0)

/**
  * @details 功能同LOG_C，但是输出到stderr
  */
#define LOG_CE(format, ...)\
    do {\
        pthread_mutex_lock(&g_simple_log_mutex);\
        fprintf(stderr, format "\n", ##__VA_ARGS__);\
        pthread_mutex_unlock(&g_simple_log_mutex);\
    } while (0)

/**
 * @brief C++语言日志宏
 * @details 使用方式同cout，其中不会打印文件名、行号、函数名，自动换行。输出到stdout
 * @param [in] msg 要打印的信息，是一个输出流的形式
 * @par 示例：
 * @code
 *  LOG_CPP(1 << "hello");
 * @endcode
 * @hideinitializer
 */
#define LOG_CPP(msg)\
    do {\
        std::ostringstream result("");\
        result << msg;\
        pthread_mutex_lock(&g_simple_log_mutex);\
        std::cout << result.str() << std::endl;\
        pthread_mutex_unlock(&g_simple_log_mutex);\
    } while (0)

/**
  * @details 功能同LOG_CPP，但是输出到stderr
  */
#define LOG_CPPE(msg)\
    do {\
        std::ostringstream result("");\
        result << msg;\
        pthread_mutex_lock(&g_simple_log_mutex);\
        std::cerr << result.str() << std::endl;\
        pthread_mutex_unlock(&g_simple_log_mutex);\
    } while (0)
#endif

/** @} Module_SimpleLog */
/** @} Module_Base */

#endif // BASE_INC_SIMPLE_LOG_H_
