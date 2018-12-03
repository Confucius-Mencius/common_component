/**
 * @file type2type.h
 * @brief
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef BASE_INC_TYPE2TYPE_H_
#define BASE_INC_TYPE2TYPE_H_

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_Type2Type Type2Type
 * @{
 */

/**
 * @brief 传给重载函数的轻量级ID，用于区分重载函数而不需要增加一个实类型的参数
 * @param T 任意类型名
 */
template<typename T>
struct Type2Type
{
    typedef T OriginalType;
};

/** @} Module_Type2Type */
/** @} Module_Base */

#endif // BASE_INC_TYPE2TYPE_H_
