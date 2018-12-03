/**
 * @file byte_util.h
 * @brief 字节与位操作
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef BASE_INC_BYTE_UTIL_H_
#define BASE_INC_BYTE_UTIL_H_

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_ByteUitl ByteUtil
 * @{
 */

#include "std_int.h"

#ifndef SET_BIT
/**
 * @brief 将一个无符号整数的从右边起的第pos位设置为1，pos从1开始编号
 * @param [in,out] val 无符号整数
 * @param [in] pos 从右边起的第pos位，pos从1开始编号
 * @hideinitializer
 */
#define SET_BIT(val, pos) ((val) |= (UINT64_C(1) << ((pos) - 1)))
#endif

#ifndef CLR_BIT
/**
 * @brief 将一个无符号整数的从右边起的第pos位设置为0，pos从1开始编号
 * @param [in,out] val 无符号整数
 * @param [in] pos 从右边起的第pos位，pos从1开始编号
 * @hideinitializer
 */
#define CLR_BIT(val, pos) ((val) &= (~(UINT64_C(1) << ((pos) - 1))))
#endif

#ifndef BIT_ENABLED
/**
 * @brief 检查一个无符号整数从右边起的第pos位是否为1，pos从1开始编号
 * @param [in] val 无符号整数
 * @param [in] pos 从右边起的第pos位，pos从1开始编号
 * @return 返回true表示该位为1，否则表示该位为0
 * @hideinitializer
 */
#define BIT_ENABLED(val, pos) (((val) & (UINT64_C(1) << ((pos) - 1))) != 0)
#endif

/**
 * @brief 将两个8位无符号整数拼装成一个16位无符号整数，左高右低
 * @param [in] u8_high 高8位
 * @param [in] u8_low 低8位
 * @return 拼装生成的16位无符号整数
 * @see windows系统头文件WinDef.h
 * @hideinitializer
 */
#define MAKE_U16(u8_high, u8_low) ((((uint16_t)((u8_high) & 0xff)) << 8) | ((uint16_t)((u8_low) & 0xff)))

/**
 * @brief 取16位无符号整数的高8位，左高右低
 * @param [in] u16_val 16位无符号整数
 * @return 16位无符号整数的高8位
 * @hideinitializer
 */
#define U16_HIGH(u16_val) (uint8_t)(((u16_val) >> 8) & 0xff)

/**
 * @brief 取16位无符号整数的低8位，左高右低
 * @param [in] u16_val 16位无符号整数
 * @return 16位无符号整数的低8位
 * @hideinitializer
 */
#define U16_LOW(u16_val) (uint8_t)((u16_val) & 0xff)

/**
 * @brief 将两个16位无符号整数拼装成一个32位无符号整数，左高右低
 * @param [in] u16_high 高16位
 * @param [in] u16_low 低16位
 * @return 拼装生成的32位无符号整数
 * @hideinitializer
 */
#define MAKE_U32(u16_high, u16_low) ((((uint32_t)((u16_high) & 0xffff)) << 16) | ((uint32_t)((u16_low) & 0xffff)))

/**
 * @brief 取32位无符号整数的高16位，左高右低。
 * @param [in] u32_val 32位无符号整数
 * @return 32位无符号整数的高16位
 * @hideinitializer
 */
#define U32_HIGH(u32_val) (uint16_t)(((u32_val) >> 16) & 0xffff)

/**
 * @brief 取32位无符号整数的低16位，左高右低
 * @param [in] u32_val 32位无符号整数
 * @return 32位无符号整数的低16位
 * @hideinitializer
 */
#define U32_LOW(u32_val) (uint16_t)((u32_val) & 0xffff)

/**
 * @brief 将两个32位无符号整数拼装成一个64位无符号整数，左高右低
 * @param [in] u32_low 低32位
 * @param [in] u32_high 高32位
 * @return 拼装生成的64位无符号整数
 * @hideinitializer
 */
#define MAKE_U64(u32_high, u32_low) (((uint64_t)((u32_high) & 0xffffffff) << 32) | ((uint64_t)((u32_low) & 0xffffffff)))

/**
 * @brief 取64位无符号整数的高32位，左高右低
 * @param [in] u64_val 64位无符号整数
 * @return 64位无符号整数的高32位
 * @hideinitializer
 */
#define U64_HIGH(u64_val) (uint32_t)(((u64_val) >> 32) & 0xffffffff)

/**
 * @brief 取64位无符号整数的低32位，左高右低
 * @param [in] u64_val 64位无符号整数
 * @return 64位无符号整数的低32位
 * @hideinitializer
 */
#define U64_LOW(u64_val) (uint32_t)((u64_val) & 0xffffffff)

/** @} Module_ByteUitl */
/** @} Module_Base */

#endif // BASE_INC_BYTE_UTIL_H_
