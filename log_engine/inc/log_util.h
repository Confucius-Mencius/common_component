#ifndef LOG_ENGINE_INC_LOG_UTIL_H_
#define LOG_ENGINE_INC_LOG_UTIL_H_

#include <log4cplus/loggingmacros.h>
#include "log_engine_interface.h"

extern LogEngineInterface* g_log_engine;

/**
 * @defgroup Module_Log 日志打印宏
 * @{
 */

#ifndef LOG_TRACE
/**
 * @brief 打印trace日志
 * @details trace事件，一般用于进入和退出某个方法或函数
 * @param msg 要打印的信息，是一个输出流的形式
 * @par 示例：
 * @code
 *  LOG_TRACE(1 << "hello");
 * @endcode
 * @hideinitializer
 */
#define LOG_TRACE(msg) LOG4CPLUS_TRACE(g_log_engine->GetLogger(), msg)
#endif // LOG_TRACE

#ifndef LOG_DEBUG
/**
 * @brief 打印debug日志
 * @details debug事件，较细粒度，对调试应用程序非常有用
 * @param msg 要打印的信息
 * @see LOG_TRACE
 * @hideinitializer
 * @attention 为避免与debug/release版本混淆，不推荐使用
 */
#define LOG_DEBUG(msg) LOG4CPLUS_DEBUG(g_log_engine->GetLogger(), msg)
#endif // LOG_DEBUG

#ifndef LOG_INFO
/**
 * @brief 打印info日志
 * @details info事件，以较粗粒度描述应用程序运行情况
 * @param msg 要打印的信息
 * @see LOG_TRACE
 * @hideinitializer
 */
#define LOG_INFO(msg) LOG4CPLUS_INFO(g_log_engine->GetLogger(), msg)
#endif // LOG_INFO

#ifndef LOG_WARN
/**
 * @brief 打印warn日志
 * @details 告警事件，可能引发错误
 * @param msg 要打印的信息
 * @see LOG_TRACE
 * @hideinitializer
 */
#define LOG_WARN(msg) LOG4CPLUS_WARN(g_log_engine->GetLogger(), msg)
#endif // LOG_WARN

#ifndef LOG_ERROR
/**
 * @brief 打印error日志
 * @details 错误事件，仍然允许应用程序继续运行
 * @param msg 要打印的信息
 * @see LOG_TRACE
 * @hideinitializer
 */
#define LOG_ERROR(msg) LOG4CPLUS_ERROR(g_log_engine->GetLogger(), msg)
#endif // LOG_ERROR

#ifndef LOG_FATAL
/**
 * @brief 打印fatal日志
 * @details 非常严重的错误事件，会导致应用程序中止
 * @param msg 要打印的信息
 * @see LOG_TRACE
 * @hideinitializer
 * @attention 会导致程序退出，不推荐使用
 */
#define LOG_FATAL(msg) LOG4CPLUS_FATAL(g_log_engine->GetLogger(), msg)
#endif // LOG_FATAL

#ifndef LOG_ALWAYS
/**
 * @brief 不受日志级别影响的日志宏
 * @details 不管配置什么级别都会一直打印的日志
 * @param msg 要打印的信息
 * @see LOG_TRACE
 * @hideinitializer
 */
#define LOG_ALWAYS(msg) LOG4CPLUS_ALWAYS(g_log_engine->GetLogger(), msg)
#endif // LOG_ALWAYS

/** @} Module_Log */

#endif // LOG_ENGINE_INC_LOG_UTIL_H_
