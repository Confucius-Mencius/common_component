/**
 * @file backtrace.h
 * @brief 获取函数调用栈
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef base_inc_backtrace_H_
#define base_inc_backtrace_H_

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_Backtrace Backtrace
 * @{
 */

/**
 * @brief 获取函数调用栈
 * @param nframes 想获取的调用栈层数
 * @return 格式良好的函数调用栈字符串
 * @note
 *  1，链接时需要-rdynamic选项，不会统计静态函数，详细说明如下（摘自man手册）：
 *  These functions make some assumptions about how a function's return address is stored on the stack.  Note the following:
 *  - Omission of the frame pointers (as implied by any of gcc(1)'s nonzero optimization levels) may cause these assumptions to be violated.
 *  - Inlined functions do not have stack frames.
 *  - Tail-call optimization causes one stack frame to replace another.
 *  The symbol names may be unavailable without the use of special linker options.  For systems using the GNU linker, it is necessary to use the -rdynamic linker option.  Note that names of "static" functions are not  exposed,
 *  and won't be available in the backtrace.
 *  2，该接口仅在调试或者出错需要跟踪函数调用关系时使用。
 * @see GetStackTrace.h (c) 2008, Timo Bingmann from http://idlebox.net/, published under the WTFPL v2.0
 */
int Backtrace(char* buf, int buf_size, int nframes = 50);

/** @} Module_Backtrace */
/** @} Module_Base */

#endif // base_inc_backtrace_H_
