#include "shell_util.h"
#include <stdio.h>
#include <string.h>
#include <string>

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

    if (buf != NULL && buf_size > 1)
    {
        char tmp_buf[4096];
        std::string result = "";

        while (!feof(fp))
        {
            memset(tmp_buf, 0, sizeof(tmp_buf));

            if (fgets(tmp_buf, sizeof(tmp_buf), fp) != NULL)
            {
                result += tmp_buf;
            }
        }

        // 去掉结尾的'\n'
        const int real_len = (int) result.length() <= buf_size - 1 ? result.length() - 1 : buf_size - 1;
        memcpy(buf, result.data(), real_len); // real_len可以等于0
        buf[real_len] = '\0';
    }

    (void) pclose(fp);
    return 0;
}
