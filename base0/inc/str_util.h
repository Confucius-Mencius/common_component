/**
 * @file str_util.h
 * @brief 字符串操作接口和宏
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef BASE_INC_STR_UTIL_H_
#define BASE_INC_STR_UTIL_H_

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_StrUtil StrUtil
 * @{
 */

#include <string>

/**
 * @brief #的功能是将其后面的宏参数字符串化
 * @hideinitializer
 */
#define TO_STR(x) #x

/**
 * @brief 获取一个宏调用展开后的字符串形式
 * @hideinitializer
 */
#define EXPAND_MACRO(m) TO_STR(m)

/**
 * @brief 将两个token（编译器能够识别的最小语法单元）连接为一个token
 * @hideinitializer
 */
#define TOKEN_CAT(x, y) x##y

/**
 * @brief token cat twice
 * @hideinitializer
 */
#define TOKEN_CAT_TWICE(x, y) TOKEN_CAT(x, y)

/**
 * @brief 执行o的成员函数f，输出成员函数名和结果
 * @hideinitializer
 */
#define PRINT_OF(o, f) std::cout << #f ": " << o.f << std::endl

/**
 * @brief 获取C语言字符串的长度
 * @param str C语言字符串
 * @param buf_size 字符串缓冲区的大小
 * @return 字符串的长度
 */
int StrLen(const char* str, size_t buf_size);

/**
 * @brief C语言字符串拷贝
 * @param str 源字符串
 * @param buf 目标缓冲区
 * @param buf_size 目标缓冲区的剩余长度（包括结尾的\0占用的空间），必须大于1
 * @return 目标缓冲区。当参数有误时返回NULL
 * @attention 最多拷贝(dstBufByteCountLeft - 1)个字符。当源字符串比目标缓冲区的剩余长度长或相等时，结果会被截断
 */
char* StrCpy(char* buf, size_t buf_size, const char* str);

/**
 * @brief C语言字符串连接，向destBuf后面追加src字符串
 * @param str 源字符串
 * @param buf 目标缓冲区
 * @param buf_size_left 目标缓冲区的剩余长度（包括结尾的\0占用的空间），必须大于1
 * @return 目标缓冲区。当参数有误时返回NULL
 * @attention 当源字符串比目标缓冲区的剩余长度长或相等时，结果会被截断
 */
char* StrCat(char* buf, size_t buf_size_left, const char* str);

/**
 * @brief C语言字符串比较，区分大小写
 * @param str1 字符串1
 * @param str2 字符串2
 * @param min_buf_size s1和s2中较小的字符串缓冲区大小（包括结尾的\0占用的空间）
 * @return true：s1和s2相等，false：s1和s2不相等
 */
bool StrCaseEQ(const char* str1, const char* str2, size_t min_buf_size);

/**
 * @brief C语言字符串比较，忽略大小写
 * @param str1 字符串1
 * @param str2 字符串2
 * @param minStrBufByteCount s1和s2中较小的字符串缓冲区大小（包括结尾的\0占用的空间）
 * @return true：s1和s2相等，false：s1和s2不相等
 */
bool StrNoCaseEQ(const char* str1, const char* str2, size_t min_buf_size);

bool StrCaseBeginWith(const char* str, const char* needle);
bool StrNoCaseBeginWith(const char* str, const char* needle);

bool StrCaseEndWith(const char* str, const char* needle);
bool StrNoCaseEndWith(const char* str, const char* needle);

/**
 * @brief C语言字符串格式化到指定缓冲区中
 * @param buf 目标缓冲区
 * @param buf_size_Left 目标缓冲区的剩余长度（包括结尾的\0占用的空间），必须大于1
 * @param fmt 格式化串
 * @param ... 变参
 * @return
 *  - 返回格式化的字符个数（不包括结尾的\0），结尾会自动添加\0
 *  - 返回值<=0表示失败
 * @attention
 *  - 当源字符串比目标缓冲区的剩余长度长或者相等时，结果会被截断
 *  - 当bufSizeLeft为1时，只能存放一个结束符\0，此时就格式化不进任何字符了
 */
int StrPrintf(char* buf, size_t buf_size, const char* fmt, ...);

/**
 * @brief 去掉字符串两边的空白
 * @param buf
 * @param buf_size
 * @param str
 * @param len
 * @param delims 空白符号列表
 * @return
 */
int StrTrim(char* buf, int buf_size, const char* str, int len, const char* delims = " \t\r\n");

/**
 * @brief 替换字符串中的子串
 * @param buf
 * @param buf_size
 * @param str
 * @param search 子串
 * @param replace 替换成
 * @return
 */
int StrReplace(char* buf, int buf_size, const char* str, const char* search, const char* replace);

void StrReverse(char str[], int len);

/**
 * @brief 通配符匹配算法
 * @param str
 * @param len1
 * @param pattern ?代表一个任意的字符，*代表0个或多个字符
 * @param len2
 * @return =0表示匹配成功，否则失败
 */
int MatchWithAsteriskW(const char* str, int len1, const char* pattern, int len2);

/**
 * @brief 获取path相对于cur_work_dir的绝对路径
 * @param cur_work_dir 必须是绝对路径
 * @param path 可以是相对路径，也可以是绝对路径
 * @return
 */
std::string GetAbsolutePath(const char* path, const char* cur_work_dir);

/** @} Module_StrUtil */
/** @} Module_Base */

#endif // BASE_INC_STR_UTIL_H_
