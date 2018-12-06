#include "ipc_util.h"
#include <string.h>
#include <sys/ipc.h>
#include "file_util.h"

key_t FToKey(const char* file_path, char proj_id)
{
    if (NULL == file_path || strlen(file_path) < 1 || 0 == proj_id)
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
