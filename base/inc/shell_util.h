/**
 * @file shell_util.h
 * @brief
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef BASE_INC_SHELL_UTIL_H_
#define BASE_INC_SHELL_UTIL_H_

/**
 * @brief 执行一个shell命令，将命令的输出写到一个buf中，以'\0'结尾
 * @param [out] buf shell命令的输出，如果不关心输出，传入NULL即可。
 * @param [in] buf_size
 * @param [in] cmd shell命令字符串，可以带各种选项和参数。
 * @return 返回0表示成功，其他表示失败。
 */
int ExecShellCmd(char* buf, int buf_size, const char* cmd);

#endif // BASE_INC_SHELL_UTIL_H_
