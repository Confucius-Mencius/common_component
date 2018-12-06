/**
 * @file ipc_util.h
 * @brief ipc util
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef BASE_INC_IPC_UTIL_H_
#define BASE_INC_IPC_UTIL_H_

#include <sys/types.h>

/**
 * @brief 根据file_path和proj_id创建一个key值
 * @param file_path 绝对路径
 * @param proj_id
 * @return
 * @attention 要求file_path是绝对路径，不能是相对路径，否则结果是不确定的
 */
key_t FToKey(const char* file_path, char proj_id);

#endif // BASE_INC_IPC_UTIL_H_
