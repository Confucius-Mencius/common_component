#include "file_util.h"
#include <dirent.h>
#include <string.h>

#if (defined(__linux__))
#include <unistd.h>
#elif (defined(_WIN32) || defined(_WIN64))
#include <io.h>
#endif

#include <fstream>
#include <sstream>
#include "str_util.h"
#include "shell_util.h"

bool FileExist(const char* file_path)
{
    if (nullptr == file_path)
    {
        return false;
    }

#if (defined(__linux__))
    if (::access(file_path, F_OK) != -1)
    {
        return true;
    }
#elif (defined(_WIN32) || defined(_WIN64))
    if (_access(file_path, 0) != -1)
    {
        return true;
    }
#endif

    return false;
}

int GetFileStat(FileStat& file_stat, const char* file_path)
{
    struct stat stat_buf;

    if (stat(file_path, & stat_buf) != 0)
    {
        return -1;
    }

    file_stat.file_size = stat_buf.st_size;
    file_stat.last_modify_time = stat_buf.st_mtime;

    return 0;
}

bool IsDirEmpty(const char* file_path)
{
    if (!FileExist(file_path))
    {
        return true;
    }

    DIR* dir = opendir(file_path);
    if (nullptr == dir)
    {
        return false;
    }

    struct dirent entry;
    struct dirent* result = nullptr;
    bool has_children = false;

    while (true)
    {
        if (readdir_r(dir, &entry, &result) != 0)
        {
            closedir(dir);
            return false;
        }

        // readdir_r returns NULL in *result if the end of the directory stream is reached
        if (nullptr == result)
        {
            break; // 没有了
        }

        if (0 == strcmp(entry.d_name, ".") || 0 == strcmp(entry.d_name, ".."))
        {
            continue; // 跳过.和..
        }

        has_children = true;
        break;
    }

    closedir(dir);
    return !has_children;
}

int CreateDir(const char* file_path)
{
    if (nullptr == file_path)
    {
        return -1;
    }

    std::ostringstream cmd;
    cmd << "mkdir -p " << file_path;

    return ExecShellCmd(nullptr, 0, cmd.str().c_str());
}

int CreateFile(const char* file_path, mode_t mode)
{
    if (nullptr == file_path)
    {
        return -1;
    }

    if (FileExist(file_path))
    {
        return 0;
    }

    char file_dir[MAX_PATH_LEN] = "";
    if (GetFileDir(file_dir, sizeof(file_dir), file_path) != 0)
    {
        return -1;
    }

    if (!FileExist(file_dir))
    {
        if (CreateDir(file_dir) != 0)
        {
            return -1;
        }
    }

    int fd = creat(file_path, mode);
    if (-1 == fd)
    {
        // const int err = errno;
        // (void) err;
        return -1;
    }

    return fd;
}

int DelFile(const char* file_path)
{
    if (nullptr == file_path)
    {
        return -1;
    }

    std::ostringstream cmd;
    cmd << "rm -rf " << file_path;

    return ExecShellCmd(nullptr, 0, cmd.str().c_str());
}

int GetFileDir(char* buf, int buf_size, const char* file_path)
{
    if (nullptr == buf || buf_size < 2 || nullptr == file_path)
    {
        return -1;
    }

    std::ostringstream cmd;
    cmd << "dirname " << file_path;

    return ExecShellCmd(buf, buf_size, cmd.str().c_str());
}

int GetFileName(char* buf, int buf_size, const char* file_path)
{
    if (nullptr == buf || buf_size < 2 || nullptr == file_path)
    {
        return -1;
    }

    strncpy(buf, basename(file_path), buf_size - 1);

//    std::ostringstream cmd;
//    cmd << "basename " << file_path;

//    return ExecShellCmd(buf, buf_size, cmd.str().c_str());

    return 0;
}

int GetAbsolutePath(char* buf, int buf_size, const char* path, const char* cur_working_dir)
{
    if (nullptr == buf || buf_size < 2 || nullptr == path)
    {
        return -1;
    }

    std::string absolute_path;

    if (path[0] != '/')
    {
        if (nullptr == cur_working_dir)
        {
            return -1;
        }

        if (0 == strcmp(path, "."))
        {
            // 当前目录，返回cur_working_dir即可
            absolute_path.append(cur_working_dir);
        }
        else
        {
            // 避免结尾是//
            if (StrEndWith(cur_working_dir, "/"))
            {
                absolute_path.append(cur_working_dir);
            }
            else
            {
                absolute_path.append(cur_working_dir);
                absolute_path.append("/");
            }

            // 避免结果中有./
            if (StrBeginWith(path, "./"))
            {
                absolute_path.append(path + 2);
            }
            else
            {
                absolute_path.append(path);
            }
        }
    }
    else
    {
        // 本身就是绝对路径，直接返回
        absolute_path.append(path);
    }

    if (buf_size - 1 < (int) absolute_path.length())
    {
        return -1;
    }

    strncpy(buf, absolute_path.data(), absolute_path.length());
    buf[absolute_path.length()] = '\0';

    return 0;
}

int WriteBinFile(const char* file_path, const void* data, size_t len)
{
    if (nullptr == file_path || nullptr == data || len < 1)
    {
        return -1;
    }

    /* 文件不存在则创建 */
    std::fstream fs;
    fs.open(file_path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!fs)
    {
        return -1;
    }

    (void) fs.write((const char*) data, len);
    fs.close();

    return 0;
}

int AppendBinFile(const char* file_path, const void* data, size_t len)
{
    if (nullptr == file_path || nullptr == data || len < 1)
    {
        return -1;
    }

    /* 文件不存在则创建 */
    std::fstream fs;
    fs.open(file_path, std::fstream::out | std::fstream::binary | std::ios::app);
    if (!fs)
    {
        return -1;
    }

    (void) fs.write((const char*) data, len);
    fs.close();

    return 0;
}

int ReadBinFile(void* data, size_t len, const char* file_path)
{
    if (nullptr == data || nullptr == file_path)
    {
        return -1;
    }

    if (!FileExist(file_path))
    {
        return -1;
    }

    struct stat stat_buf;
    stat(file_path, &stat_buf);
    size_t file_size = stat_buf.st_size;

    if (len < file_size)
    {
        return -1;
    }

    std::fstream fs;
    fs.open(file_path, std::ios::in | std::ios::binary);
    if (!fs)
    {
        return -1;
    }

    (void) fs.read((char*) data, file_size);
//    len = fs.gcount();
    fs.close();

    return 0;
}

int WriteTxtFile(const char* file_path, const void* data, size_t len)
{
    if (nullptr == file_path || nullptr == data || len < 1)
    {
        return -1;
    }

    /* 文件不存在则创建 */
    std::fstream fs;
    fs.open(file_path, std::ios::out | std::ios::trunc);
    if (!fs)
    {
        return -1;
    }

    (void) fs.write((const char*) data, len);
    fs.close();

    return 0;
}

int AppendTxtFile(const char* file_path, const void* data, size_t len)
{
    if (nullptr == file_path || nullptr == data || len < 1)
    {
        return -1;
    }

    /* 文件不存在则创建 */
    std::fstream fs;
    fs.open(file_path, std::ios::out | std::ios::app);
    if (!fs)
    {
        return -1;
    }

    (void) fs.write((const char*) data, len);
    fs.close();

    return 0;
}

int ReadTxtFile(void* data, size_t len, const char* file_path)
{
    if (nullptr == file_path || nullptr == data)
    {
        return -1;
    }

    if (!FileExist(file_path))
    {
        return -1;
    }

    struct stat stat_buf;
    stat(file_path, &stat_buf);
    size_t file_size = stat_buf.st_size;

    if (len < file_size)
    {
        return -1;
    }

    std::fstream fs;
    fs.open(file_path, std::ios::in);
    if (!fs)
    {
        return -1;
    }

    (void) fs.read((char*) data, file_size);
//    len = fs.gcount();
    fs.close();

    return 0;
}
