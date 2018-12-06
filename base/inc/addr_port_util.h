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
  * ip地址或者host名称的最大长度
  */
#define MAX_ADDR_LEN 255

#ifndef MAX_IPV4_ADDRESS_LEN
/**
 * @brief max ipv4 address len
 */
#define MAX_IPV4_ADDRESS_LEN 15 // INET_ADDRSTRLEN - 1
#endif // MAX_IPV4_ADDRESS_LEN

/**
 * @brief
 * @param [out] addr
 * @param [in] buf_size
 * @param [out] port
 * @param [in] addr_port
 * @param [in] len
 * @return =0表示成功，否则失败
 */
int ParseAddrPort(char* addr, size_t buf_size, unsigned short& port, const char* addr_port, size_t len);

/**
 * @brief
 * @param [out] addr
 * @param [in] buf_size
 * @param [out] port
 * @param [in] addr_port
 * @return =0表示成功，否则失败
 */
int ParseAddrPort(char* addr, size_t buf_size, unsigned short& port, const std::string& addr_port);

/** @} Module_AddrPortUtil */
/** @} Module_Base */

#endif // BASE_INC_ADDR_PORT_UTIL_H_
