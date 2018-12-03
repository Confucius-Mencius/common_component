#include "file_util.h"
#include "common_define.h"

#if (defined(__linux__))

#include <unistd.h>

#elif (defined(_WIN32) || defined(_WIN64))
#include <io.h>
#endif

#include <dirent.h>
#include <sys/uio.h>
#include <fstream>
#include <sstream>
#include "misc_util.h"

bool FileExist(const char* file_path)
{
    if (NULL == file_path)
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
    DIR* dir = opendir(file_path);
    if (NULL == dir)
    {
        return false;
    }

    struct dirent entry;
    struct dirent* result = NULL;
    bool has_children = false;

    for (;;)
    {
        if (readdir_r(dir, &entry, &result) != 0)
        {
            closedir(dir);
            return false;
        }

        // readdir_r returns NULL in *result if the end of the directory stream is reached
        if (NULL == result)
        {
            break; // 没有了
        }

        if (0 == strcmp(entry.d_name, ".") || 0 == strcmp(entry.d_name, ".."))
        {
            continue;
        }

        has_children = true;
        break;
    }

    closedir(dir);

    return !has_children;
}

int CreateFile(const char* file_path, mode_t mode)
{
    if (NULL == file_path)
    {
        return -1;
    }

    char file_dir[MAX_PATH_LEN + 1] = "";
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

    return creat(file_path, mode);
}

int CreateDir(const char* file_path)
{
    if (NULL == file_path)
    {
        return -1;
    }

    std::ostringstream cmd;
    cmd << "mkdir -p " << file_path;

    return ExecShellCmd(NULL, 0, cmd.str().c_str());
}

int DelFile(const char* file_path)
{
    if (NULL == file_path)
    {
        return -1;
    }

    std::ostringstream cmd;
    cmd << "rm -rf " << file_path;

    return ExecShellCmd(NULL, 0, cmd.str().c_str());
}

int GetFileName(char* buf, int buf_size, const char* file_path)
{
    if (NULL == buf || buf_size < 1 || NULL == file_path)
    {
        return -1;
    }

    std::ostringstream cmd;
    cmd << "basename " << file_path;

    if (ExecShellCmd(buf, buf_size, cmd.str().c_str()) != 0)
    {
        return -1;
    }

    return 0;
}

int GetFileDir(char* buf, int buf_size, const char* file_path)
{
    if (NULL == buf || buf_size < 1 || NULL == file_path)
    {
        return -1;
    }

    std::ostringstream cmd;
    cmd << "dirname " << file_path;

    if (ExecShellCmd(buf, buf_size, cmd.str().c_str()) != 0)
    {
        return -1;
    }

    return 0;
}

int WriteBinFile(const char* file_path, const void* data, size_t len)
{
    if (NULL == file_path || NULL == data || len < 1)
    {
        return -1;
    }

    /* 文件不存在则创建 */
    std::fstream fs;
    fs.open(file_path, std::fstream::out | std::fstream::binary | std::fstream::trunc);
    (void) fs.write((const char*) data, len);
    fs.close();

    return 0;
}

int ReadBinFile(const char* file_path, void* data, size_t& len)
{
    if (NULL == file_path || NULL == data)
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
    fs.open(file_path, std::fstream::binary | std::fstream::in);
    (void) fs.read((char*) data, file_size);
    len = fs.gcount();
    fs.close();

    return 0;
}

int WriteTxtFile(const char* file_path, const void* data, size_t len)
{
    if (NULL == file_path || NULL == data || len < 1)
    {
        return -1;
    }

    /* 文件不存在则创建 */
    std::fstream fs;
    fs.open(file_path, std::fstream::out | std::fstream::trunc);
    (void) fs.write((const char*) data, len);
    fs.close();

    return 0;
}

int ReadTxtFile(const char* file_path, void* data, size_t& len)
{
    if (NULL == file_path || NULL == data)
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
    fs.open(file_path, std::fstream::in);
    (void) fs.read((char*) data, file_size);
    len = fs.gcount();
    fs.close();

    return 0;
}

int OpenFileWithRetry(const char* file_path, int flags, mode_t mode, int nretrys, int sleep_ms)
{
    if (NULL == file_path || nretrys < 0 || sleep_ms <= 0)
    {
        return -1;
    }

    int fd = open(file_path, flags, mode);
    if (fd >= 0)
    {
        return fd;
    }

    int n = 0;
    while (n < nretrys)
    {
        int fd = open(file_path, flags, mode);
        if (fd >= 0)
        {
            return fd;
        }

        ++n;
        usleep(1000 * sleep_ms);
    }

    return -1;
}

int WriteFileWithRetry(int fd, const void* data, size_t len, int nretrys, int sleep_ms)
{
    if (fd < 0 || NULL == data || len < 1 || nretrys < 0 || sleep_ms <= 0)
    {
        return -1;
    }

    if (write(fd, data, len) == (ssize_t) len)
    {
        return 0;
    }

    int n = 0;
    while (n < nretrys)
    {
        if (write(fd, data, len) == (ssize_t) len)
        {
            return 0;
        }

        ++n;
        usleep(1000 * sleep_ms);
    }

    return -1;
}

int WriteFileWithRetry(int fd, const struct iovec* iov, int iovcnt, size_t total_data_len, int nretrys, int sleep_ms)
{
    if (fd < 0 || NULL == iov || iovcnt <= 0 || iovcnt > 1024 || total_data_len <= 0 || nretrys < 0 || sleep_ms <= 0)
    {
        return -1;
    }

    if (writev(fd, iov, iovcnt) == (ssize_t) total_data_len)
    {
        return 0;
    }

    int n = 0;
    while (n < nretrys)
    {
        if (writev(fd, iov, iovcnt) == (ssize_t) total_data_len)
        {
            return 0;
        }

        ++n;
        usleep(1000 * sleep_ms);
    }

    return -1;
}
