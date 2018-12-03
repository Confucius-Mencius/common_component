/**
 * @file addr_port_util.h
 * @brief 解析格式为"addr:port"的字符串，获取其中的addr和port，其中addr为IP或域名均可。
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef BASE_INC_ADDR_PORT_UTIL_H_
#define BASE_INC_ADDR_PORT_UTIL_H_

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_AddrPortUtil AddrPortUilt
 * @{
 */

#include <string>

/**
 * @brief
 * @param [out] addr
 * @param [in] addr_buf_size
 * @param [out] port
 * @param [in] addr_port
 * @param [in] addr_port_len
 * @return =0表示成功，否则失败
 */
int ParseAddrPort(char* addr, size_t addr_buf_size, unsigned short& port, const char* addr_port, size_t addr_port_len);

/**
 * @brief
 * @param [out] addr
 * @param [in] addr_buf_size
 * @param [out] port
 * @param [in] addr_port
 * @return =0表示成功，否则失败
 */
int ParseHostPort(char* addr, size_t addr_buf_size, unsigned short& port, const std::string& addr_port);

/**
 * @brief
 * @param [out] addr
 * @param [out] port
 * @param [in] addr_port
 * @param [in] addr_port_len
 * @return =0表示成功，否则失败
 */
int ParseAddPort(std::string& addr, unsigned short& port, const char* addr_port, size_t addr_port_len);

/**
 * @brief
 * @param [out] addr
 * @param [out] port
 * @param [in] addr_port
 * @return =0表示成功，否则失败
 */
int ParseAddPort(std::string& addr, unsigned short& port, const std::string& addr_port);

/** @} Module_AddrPortUtil */
/** @} Module_Base */

#endif // BASE_INC_ADDR_PORT_UTIL_H_
