/**
 * @file last_err_msg.h
 * @brief 最近的错误描述
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef BASE_INC_LAST_ERR_MSG_H_
#define BASE_INC_LAST_ERR_MSG_H_

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_LastErrMsg LastErrMsg
 * @{
 */

#include <stdio.h>
#include <sstream>

/**
  * 超过这个最大长度的err msg会被截断。
  */
#define MAX_ERR_MSG_LEN 1023

/**
 * @brief 最近一次的出错信息
 */
class LastErrMsg
{
public:
    LastErrMsg()
    {
        what_[0] = '\0';
    }

    ~LastErrMsg()
    {
    }

    void SetWhat(const std::ostringstream& what)
    {
        const int n = snprintf(what_, sizeof(what_), "%s", what.str().c_str());
        what_[n] = '\0';
    }

    const char* What() const
    {
        return what_;
    }

private:
    char what_[MAX_ERR_MSG_LEN + 1];
};

/**
 * @brief 设置最近的错误猫叔
 * @param [in,out] obj_ptr LastErrMsg对象指针
 * @param [in] err_msg 最近的错误描述，是一个输入流（<<）的形式
 * @hideinitializer
 */
#define SET_LAST_ERR_MSG(obj_ptr, err_msg)\
    do {\
        std::ostringstream what;\
        what << err_msg;\
        (obj_ptr)->SetWhat(what);\
    } while (0)

/** @} Module_LastErrMsg */
/** @} Module_Base */

#endif // BASE_INC_LAST_ERR_MSG_H_
