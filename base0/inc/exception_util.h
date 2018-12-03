/**
 * @file exception_util.h
 * @brief 异常基类和辅助宏
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef BASE_INC_EXCEPTION_UTIL_H_
#define BASE_INC_EXCEPTION_UTIL_H_

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_ExceptionUtil 异常基类和辅助宏
 * @{
 */

#include <exception>
#include <sstream>

/**
 * @brief 异常基类
 */
class Exception : public std::exception
{
public:
    explicit Exception(const std::string& msg = "") throw()
    {
        msg_ = msg;
    }

    virtual ~Exception() throw()
    {
    }

    virtual const char* what() const throw()
    {
        return msg_.c_str();
    }

protected:
    std::string msg_;
};

/**
 * @brief 定义一个自己的异常类，它会自动继承上面的异常基类CException
 * @param [in] ExceptionType 自定义的异常类型名
 * @hideinitializer
 */
#define DEFINE_EXCEPTION(ExceptionType)\
class ExceptionType : public Exception\
{\
public:\
    explicit ExceptionType(const std::string& msg = "") throw () : Exception(msg) {}\
    virtual ~ExceptionType() throw () {}\
}

/**
 * @brief 抛出自定义的异常
 * @param [in] ExceptionType 自定义的异常类名
 * @param [in] msg 异常描述信息，是一个输出流的形式
 * @par 示例：
 * @code
 *  THROW_EXCEPTION(CDBCException, "DBC Require Failed: " << someOtherMsg);
 * @endcode
 * @hideinitializer
 */
#define THROW_EXCEPTION(ExceptionType, msg)\
do {\
    std::ostringstream oss("");\
    oss << "<" << __FILE__ << ":" << __LINE__ << " " << __PRETTY_FUNCTION__ << "> " << msg << std::endl;\
    throw ExceptionType(oss.str());\
} while (0)

/** @} Module_ExceptionUtil */
/** @} Module_Base */

#endif // BASE_INC_EXCEPTION_UTIL_H_
