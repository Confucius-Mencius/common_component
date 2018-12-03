/**
 * @file std_int.h
 * @brief
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef BASE_INC_STD_INT_H_
#define BASE_INC_STD_INT_H_

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
#ifndef __STDC_CONSTANT_MACROS
/**
 * @brief 宏INTn_C(val)定义在stdint.h头文件中，只有\n
 * （1）在纯C语言中，或者\n
 * （2）在C++语言中，且在'#include <stdint.h>'之前定义了宏__STDC_LIMIT_MACROS时\n
 * 才有效，否则编译报错。\n
 * @attention INT8_C,INT16_C,INT32_C,UINT8_C,UINT16_C这几个宏是无用的，不会对整数字面量加任何后缀。如INT8_C(0x80)的结果为128，不是-128。
 */
#define __STDC_CONSTANT_MACROS
#endif /* __STDC_CONSTANT_MACROS */
#ifdef __cplusplus
}
#endif /* __cplusplus */

#include <stdint.h>

#endif // BASE_INC_STD_INT_H_
