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

/**
 If even this is too much work for you, you'll find the gmock_gen.py tool
 in Google Mock's scripts/generator/ directory (courtesy of the cppclean project) useful.
 This command-line tool requires that you have Python 2.4 installed.
 You give it a C++ file and the name of an abstract class defined in it,
 and it will print the definition of the mock class for you.
 Due to the complexity of the C++ language, this script may not always work,
 but it can be quite handy when it does.

 The Google Mock class generator is an application that is part of cppclean.
 For more information about cppclean, see the README.cppclean file or
 visit http://code.google.com/p/cppclean/

 cppclean requires Python 2.3.5 or later.  If you don't have Python installed
 on your system, you will also need to install it.  You can download Python
 from:  http://www.python.org/download/releases/

 To use the Google Mock class generator, you need to call it
 on the command line passing the header file and class for which you want
 to generate a Google Mock class.

 Make sure to install the scripts somewhere in your path.  Then you can
 run the program.

 gmock_gen.py header-file.h [ClassName]...

 If no ClassNames are specified, all classes in the file are emitted.

 To change the indentation from the default of 2, set INDENT in
 the environment.  For example to use an indent of 4 spaces:

 INDENT=4 gmock_gen.py header-file.h ClassName

 This version was made from SVN revision 281 in the cppclean repository.

 Known Limitations
 -----------------
 Not all code will be generated properly.  For example, when mocking templated
 classes, the template information is lost.  You will need to add the template
 information manually.

 Not all permutations of using multiple pointers/references will be rendered
 properly.  These will also have to be fixed manually.
 */

#include "gtest/gtest.h"

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
