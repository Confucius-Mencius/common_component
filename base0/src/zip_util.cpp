#include "zip_util.h"
#include <string.h>
#include "common_define.h"
#include "str_util.h"

uint32_t GetLocalFileHeaderLen(const char* file_name, uint64_t file_size)
{
    const uint16_t file_name_len = StrLen(file_name, MAX_PATH_LEN + 1);

    int extra_len = 0;
    if (file_size > 0xffffffff)
    {
        extra_len = 20;
    }

    uint32_t local_file_header_len = 30 + file_name_len + extra_len;
    return local_file_header_len;
}

int BuildLocalFileHeader(char* buf, uint32_t buf_len, const char* file_name, uint64_t file_size, uint32_t file_crc32)
{
    const uint16_t file_name_len = StrLen(file_name, MAX_PATH_LEN + 1);
    if (file_name_len > 0xffff)
    {
        return -1;
    }

    memset(buf, 0, buf_len);
    *((uint16_t*) (buf + 6)) = 0x0800; // utf8 file name and comment
    *((uint32_t*) buf) = 0x04034b50;
    *((uint32_t*) (buf + 14)) = file_crc32;

    if (file_size > 0xffffffff)
    {
        *((uint32_t*) (buf + 18)) = 0xffffffff;
        *((uint32_t*) (buf + 22)) = 0xffffffff;
        *((uint16_t*) (buf + 26)) = file_name_len;
        *((uint16_t*) (buf + 28)) = 20;

        memcpy(buf + 30, file_name, file_name_len);

        *((uint16_t*) (buf + 30 + file_name_len)) = 0x0001;
        *((uint16_t*) (buf + 30 + file_name_len + 2)) = 16; // 后面字节的长度
        *((uint64_t*) (buf + 30 + file_name_len + 4)) = file_size;
        *((uint64_t*) (buf + 30 + file_name_len + 12)) = file_size;
    }
    else
    {
        *((uint32_t*) (buf + 18)) = (uint32_t) file_size;
        *((uint32_t*) (buf + 22)) = (uint32_t) file_size;
        *((uint16_t*) (buf + 26)) = file_name_len;
        memcpy(buf + 30, file_name, file_name_len);
    }

    return 0;
}

uint32_t GetCentralDirectoryLen(const char* file_name, uint64_t file_size, uint64_t local_file_header_offset)
{
    const uint16_t file_name_len = StrLen(file_name, MAX_PATH_LEN + 1);

    int extra_len = 0;
    if (local_file_header_offset > 0xffffffff)
    {
        if (file_size > 0xffffffff)
        {
            extra_len = 28;
        }
        else
        {
            extra_len = 12;
        }
    }
    else
    {
        if (file_size > 0xffffffff)
        {
            extra_len = 20;
        }
    }

    uint32_t central_directory_len = 46 + file_name_len + extra_len;
    return central_directory_len;
}

int BuildCentralDirectory(char* buf, uint32_t buf_len, const char* file_name, uint64_t file_size,
                          uint64_t local_file_header_offset, uint32_t file_crc32)
{
    const uint16_t file_name_len = StrLen(file_name, MAX_PATH_LEN + 1);

    if (file_name_len > 0xffff)
    {
        return -1;
    }

    memset(buf, 0, buf_len);

    *((uint32_t*) buf) = 0x02014b50;
    *((uint16_t*) (buf + 8)) = 0x0800; // utf8 file name and comment
    *((uint32_t*) (buf + 16)) = file_crc32; // 可选

    if (local_file_header_offset > 0xffffffff)
    {
        if (file_size > 0xffffffff)
        {
            *((uint32_t*) (buf + 20)) = 0xffffffff;
            *((uint32_t*) (buf + 24)) = 0xffffffff;
            *((uint16_t*) (buf + 28)) = file_name_len;
            *((uint16_t*) (buf + 30)) = 28;
            *((uint32_t*) (buf + 42)) = 0xffffffff;

            memcpy(buf + 46, file_name, file_name_len);

            *((uint16_t*) (buf + 46 + file_name_len)) = 0x0001;
            *((uint16_t*) (buf + 46 + file_name_len + 2)) = 24;
            *((uint64_t*) (buf + 46 + file_name_len + 4)) = file_size;
            *((uint64_t*) (buf + 46 + file_name_len + 12)) = file_size;
            *((uint64_t*) (buf + 46 + file_name_len + 20)) = local_file_header_offset;
        }
        else
        {
            *((uint32_t*) (buf + 20)) = (uint32_t) file_size;
            *((uint32_t*) (buf + 24)) = (uint32_t) file_size;
            *((uint16_t*) (buf + 28)) = file_name_len;
            *((uint16_t*) (buf + 30)) = 12;
            *((uint32_t*) (buf + 42)) = 0xffffffff;

            memcpy(buf + 46, file_name, file_name_len);

            *((uint16_t*) (buf + 46 + file_name_len)) = 0x0001;
            *((uint16_t*) (buf + 46 + file_name_len + 2)) = 8;
            *((uint64_t*) (buf + 46 + file_name_len + 4)) = local_file_header_offset;
        }
    }
    else
    {
        if (file_size > 0xffffffff)
        {
            *((uint32_t*) (buf + 20)) = 0xffffffff;
            *((uint32_t*) (buf + 24)) = 0xffffffff;
            *((uint16_t*) (buf + 28)) = file_name_len;
            *((uint16_t*) (buf + 30)) = 20;
            *((uint32_t*) (buf + 42)) = (uint32_t) local_file_header_offset;

            memcpy(buf + 46, file_name, file_name_len);

            *((uint16_t*) (buf + 46 + file_name_len)) = 0x0001;
            *((uint16_t*) (buf + 46 + file_name_len + 2)) = 16;
            *((uint64_t*) (buf + 46 + file_name_len + 4)) = file_size;
            *((uint64_t*) (buf + 46 + file_name_len + 12)) = file_size;
        }
        else
        {
            *((uint32_t*) (buf + 20)) = (uint32_t) file_size;
            *((uint32_t*) (buf + 24)) = (uint32_t) file_size;
            *((uint16_t*) (buf + 28)) = file_name_len;
            *((uint32_t*) (buf + 42)) = (uint32_t) local_file_header_offset;

            memcpy(buf + 46, file_name, file_name_len);
        }
    }

    return 0;
}

uint32_t GetZip64CentralDirectoryEndLen()
{
    return 56;
}

int BuildZip64CentralDirectoryEnd(char* buf, uint32_t buf_len, uint16_t central_directory_count,
                                  uint32_t total_central_directory_size, uint64_t first_central_directory_offset)
{
    memset(buf, 0, buf_len);

    *((uint32_t*) buf) = 0x06064b50;
    *((uint64_t*) (buf + 4)) = 56 - 12;
    *((uint64_t*) (buf + 24)) = central_directory_count;
    *((uint64_t*) (buf + 32)) = central_directory_count;
    *((uint64_t*) (buf + 40)) = total_central_directory_size;
    *((uint64_t*) (buf + 48)) = first_central_directory_offset;

    return 0;
}

uint32_t GetZip64CentralDirectoryLocatorLen()
{
    return 20;
}

int BuildZip64CentralDirectoryLocator(char* buf, uint32_t buf_len, uint64_t zip64_central_directory_end_offset)
{
    memset(buf, 0, buf_len);

    *((uint32_t*) buf) = 0x07064b50;
    *((uint64_t*) (buf + 8)) = zip64_central_directory_end_offset;
    *((uint32_t*) (buf + 16)) = 1;

    return 0;
}

uint32_t GetCentralDirectoryEndLen()
{
    return 22;
}

int BuildCentralDirectoryEnd(char* buf, uint32_t buf_len, uint16_t central_directory_count,
                             uint32_t total_central_directory_size, uint64_t first_central_directory_offset)
{
    memset(buf, 0, buf_len);

    *((uint32_t*) buf) = 0x06054b50;
    *((uint16_t*) (buf + 8)) = central_directory_count;
    *((uint16_t*) (buf + 10)) = central_directory_count;
    *((uint32_t*) (buf + 12)) = total_central_directory_size;
    *((uint32_t*) (buf + 16)) = 0xffffffff;

    return 0;
}
