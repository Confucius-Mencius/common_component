/**
 * @file dbc.h
 * @brief 基于契约设计（DBC：design by contract）
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef BASE_INC_DBC_H_
#define BASE_INC_DBC_H_

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_DBC DBC相关验证宏
 * @{
 */

#include "exception_util.h"

DEFINE_EXCEPTION(DBCException);

/**
 * @brief DBC-前置条件验证。
 * @param [in] exp 待验证的表达式。如果表达式不满足，则抛出DBCException。
 * @param [in] msg 异常描述信息。
 * @par 示例：
 * @code
 *  DBC_REQUIRE(width > 0, "参数width必须大于0");
 * @endcode
 * @hideinitializer
 */
#define DBC_REQUIRE(exp, msg)\
    do {\
        if (exp) break;\
        THROW_EXCEPTION(DBCException, "** DBC REQUIRE FAILED **: " << msg);\
    } while (0)

/**
 * @brief DBC-后置条件验证。
 * @param [in] exp 待验证的表达式。如果表达式不满足，则抛出DBCException。
 * @param [in] msg 异常描述信息。
 * @par 示例：
 * @code
 * DBC_ENSURE(rect->GetWidth() == width, "width未被正确赋值");
 * DBC_ENSURE(rect->GetHeight() == oldHeight, "height应该保持原来的值不变");
 * @endcode
 * @hideinitializer
 */
#define DBC_ENSURE(exp, msg)\
    do {\
        if (exp) break;\
        THROW_EXCEPTION(DBCException, "** DBC ENSURE FAILED **: " << msg);\
    } while (0)

/** @} Module_DBC */
/** @} Module_Base */

#endif // BASE_INC_DBC_H_
