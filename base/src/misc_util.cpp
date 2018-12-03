#include "misc_util.h"
#include <stdio.h>
#include <string.h>
#include <string>
#include <sys/ipc.h>
#include "data_type.h"
#include "file_util.h"

bool IsLittleEndian()
{
    union
    {
        u16 d1;
        u8 d2[2];
    } d;

    const u16 high = UINT16_C(0x12);
    const u16 low = UINT16_C(0x34);
    d.d1 = ((high << 8) | low);

    return (d.d2[0] == UINT8_C(low));
}

int ExecShellCmd(char* buf, int buf_size, const char* cmd)
{
    if (NULL == cmd)
    {
        return -1;
    }

    // popen()会调用fork()产生子进程，然后从子进程中调用/bin/sh -c 来执行参数command的指令。
    // 参数type: "r"代表读取，"w"代表写入。
    // popen()会建立管道连到子进程的标准输出设备或标准输入设备，然后返回一个文件指针。随后进程便可利用此文件指针来读取子进程的标准输出设备或是写入到子进程的标准输入设备中。
    FILE* fp = popen(cmd, "r");
    if (NULL == fp)
    {
        return -1;
    }

    if (buf != NULL && buf_size > 0)
    {
        char buffer[4096];
        std::string result = "";

        while (!feof(fp))
        {
            memset(buffer, 0, sizeof(buffer));

            if (fgets(buffer, sizeof(buf), fp) != NULL)
            {
                result += buffer;
            }
        }

        // 去掉结尾的'\n'
        const int real_len = (int) result.size() <= buf_size - 1 ? result.size() - 1 : buf_size - 1;
        memcpy(buf, result.data(), real_len); // real_len可以等于0
        buf[real_len] = '\0';
    }

    (void) pclose(fp);
    return 0;
}

key_t FToKey(const char* file_path, char proj_id)
{
    if (NULL == file_path || 0 == strlen(file_path) || 0 == proj_id)
    {
        return -1;
    }

    // 文件不存在时自动创建
    if (!FileExist(file_path))
    {
        CreateFile(file_path);
    }

    key_t key = ftok(file_path, proj_id);
    if (-1 == key)
    {
        return -1;
    }

    return key;
}
