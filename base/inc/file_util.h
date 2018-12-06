/**
 * @file file_util.h
 * @brief 文件操作常用接口
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef BASE_INC_FILE_UTIL_H_
#define BASE_INC_FILE_UTIL_H_

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_FileUtil FileUtil
 * @{
 */

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifndef MAX_PATH_LEN
/**
 * @brief max path len
 */
#define MAX_PATH_LEN 255
#endif // MAX_PATH_LEN

/**
 * @brief 文件属性结构体
 */
struct FileStat
{
    size_t file_size;
    time_t last_modify_time;

    FileStat()
    {
        file_size = 0;
        last_modify_time = 0;
    }

    /**
     * @brief 判断两个结构体的各个成员的值是否都相等
     * @param [in] rhs
     * @return
     */
    bool Equals(const FileStat& rhs) const
    {
        return (file_size == rhs.file_size) && (last_modify_time == rhs.last_modify_time);
    }
};

/**
 * @brief 检查文件是否存在
 * @param [in] file_path
 * @return
 * @attention 文件路径的三个概念：全路径名：FilePath，文件所在的目录名：FileDir，文件名：FileName \n
 *  例如对于文件"/usr/local/beauty.txt"而言，\n
 *  全路径名为：/usr/local/beauty.txt，文件所在的目录名为：/usr/local/，文件名为：beauty.txt
 */
bool FileExist(const char* file_path);

/**
 * @brief 获取文件属性
 * @param [out] file_stat
 * @param [in] file_path
 * @return =0表示成功，否则失败
 */
int GetFileStat(FileStat& file_stat, const char* file_path);

/**
 * @brief 检查目录是否为空
 * @param [in] file_path
 * @return 当目录为空，即目录中没有任何文件和子目录时返回true
 */
bool IsDirEmpty(const char* file_path);

/**
 * @brief 创建一个新文件
 * @param [in] file_path
 * @param [in] mode
 * @attention 如果上层目录不存在则自动创建，允许多级目录
 * @return 创建成功后返回文件的fd，否则返回-1
 */
int CreateFile(const char* file_path, mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);

/**
 * @brief 创建一个新目录
 * @param [in] file_path，允许多级
 * @return =0表示成功，否则失败
 */
int CreateDir(const char* file_path);

/**
 * @brief 删除文件或者目录
 * @param [in] file_path
 * @return =0表示成功，否则失败
 */
int DelFile(const char* file_path);

/**
 * 获取文件名
 * @param [out] buf
 * @param [in] buf_size
 * @param [in] file_path
 * @return =0表示成功，否则失败
 */
int GetFileName(char* buf, int buf_size, const char* file_path);

/**
 * @brief 获取文件所在的目录
 * @param [out] buf
 * @param [in] buf_size
 * @param [in] file_path
 * @return =0表示成功，否则失败
 */
int GetFileDir(char* buf, int buf_size, const char* file_path);

/**
 * @brief 获取path相对于cur_working_dir的绝对路径
 * @param cur_working_dir 必须是绝对路径
 * @param path 可以是相对路径，也可以是绝对路径
 * @return
 */
int GetAbsolutePath(char* buf, int buf_size, const char* path, const char* cur_working_dir);

/**
 * @brief 将一段二进制数据写入指定文件中，如果文件不存在则先创建
 * @param [in] file_path
 * @param [in] data 二进制数据
 * @param [in] len 数据长度
 * @return =0表示成功，否则失败
 */
int WriteBinFile(const char* file_path, const void* data, size_t len);

/**
 * @brief 从指定文件中读取二进制数据
 * @param [out] data 存放二进制数据的缓冲区
 * @param [in, out] len 输入时为缓冲区的大小，输出时为所读取数据的实际长度
 * @param [in] file_path
 * @return =0表示成功，否则失败
 */
int ReadBinFile(void* data, size_t& len, const char* file_path);

int AppendBinFile(const char* file_path, const void* data, size_t len);

/**
 * @brief 将一段文本数据写入指定文件中，如果文件不存在则先创建
 * @param [in] file_path
 * @param [in] data 文本数据
 * @param [in] len 数据长度
 * @return =0表示成功，否则失败
 */
int WriteTxtFile(const char* file_path, const void* data, size_t len);

/**
 * @brief 从指定文件中读取文本数据
 * @param [out] data 存放文本数据的缓冲区
 * @param [in, out] len 输入时为缓冲区的大小，输出时为所读取数据的实际长度
 * @param [in] file_path
 * @return =0表示成功，否则失败
 */
int ReadTxtFile(void* data, size_t& len, const char* file_path);

int AppendTxtFile(const char* file_path, const void* data, size_t len);

/** @} Module_FileUtil */
/** @} Module_Base */

#endif // BASE_INC_FILE_UTIL_H_
