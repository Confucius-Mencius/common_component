/**
 * @file test_util.h
 * @brief
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_TestUtil TestUtil
 * @{
 */

#ifndef BASE_INC_TEST_UTIL_H_
#define BASE_INC_TEST_UTIL_H_

#include <gtest/gtest.h>

/**
 * @brief 测试基类
 */
typedef testing::Test GTest;

/**
 * @brief 添加全局测试函数
 * @param _TEST_SUITE_NAME_ 用例组名，有意义的名字即可。该参数在google test中称为test case name，在XUnit中称为suite
 * @param _GLOBAL_TEST_FUNC_NAME_ 全局测试用例函数名，要求该函数无参数且无返回值。该参数在google test中称为test name，在XUnit中称为test case
 * @hideinitializer
 */
#define ADD_TEST(_TEST_SUITE_NAME_, _GLOBAL_TEST_FUNC_NAME_)\
    TEST(_TEST_SUITE_NAME_, _GLOBAL_TEST_FUNC_NAME_) { _GLOBAL_TEST_FUNC_NAME_(); }

/**
 * @brief 添加GTest子类的成员函数
 * @param _TEST_CLASS_NAME_ 测试类名，必须为GTest的子类
 * @param _MEMBER_TEST_FUNC_NAME_ 测试用例函数名，是测试类的成员函数，要求该函数无参数且无返回值
 * @see GTest
 * @hideinitializer
 */
#define ADD_TEST_F(_TEST_CLASS_NAME_, _MEMBER_TEST_FUNC_NAME_)\
    TEST_F(_TEST_CLASS_NAME_, _MEMBER_TEST_FUNC_NAME_) { _MEMBER_TEST_FUNC_NAME_(); }

/** @} Module_TestUtil */
/** @} Module_Base */

#endif // BASE_INC_TEST_UTIL_H_
