/**
 * @file zip_util.h
 * @brief zip文件操作
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef BASE_INC_ZIP_UTIL_H_
#define BASE_INC_ZIP_UTIL_H_

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_ZipUtil ZipUtil
 * @{
 */

#include "std_int.h"

uint32_t GetLocalFileHeaderLen(const char* file_name, uint64_t file_size);
int BuildLocalFileHeader(char* buf, uint32_t buf_len, const char* file_name, uint64_t file_size, uint32_t file_crc32);

uint32_t GetCentralDirectoryLen(const char* file_name, uint64_t file_size, uint64_t local_file_header_offset);
int BuildCentralDirectory(char* buf, uint32_t buf_len, const char* file_name, uint64_t file_size,
                          uint64_t local_file_header_offset, uint32_t file_crc32);

uint32_t GetZip64CentralDirectoryEndLen();
int BuildZip64CentralDirectoryEnd(char* buf, uint32_t buf_len, uint16_t central_directory_count,
                                  uint32_t total_central_directory_size, uint64_t first_central_directory_offset);

uint32_t GetZip64CentralDirectoryLocatorLen();
int BuildZip64CentralDirectoryLocator(char* buf, uint32_t buf_len, uint64_t zip64_central_directory_end_offset);

uint32_t GetCentralDirectoryEndLen();
int BuildCentralDirectoryEnd(char* buf, uint32_t buf_len, uint16_t central_directory_count,
                             uint32_t total_central_directory_size, uint64_t first_central_directory_offset);

/** @} Module_ZipUtil */
/** @} Module_Base */

#endif // BASE_INC_ZIP_UTIL_H_
