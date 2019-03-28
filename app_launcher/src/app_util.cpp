#include "app_util.h"
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include "str_util.h"

namespace app_launcher
{
int GetAppName(char* buf, size_t buf_size, const char* argv0)
{
    if (nullptr == buf || buf_size < 2 || nullptr == argv0)
    {
        return -1;
    }

    const char* last_slash = strrchr(argv0, '/');
    if (nullptr == last_slash)
    {
        syslog(LOG_ERR, "can't find / in %s", argv0);
        return -1;
    }

    StrCpy(buf, buf_size, last_slash + 1);
    return 0;
}

int SingleRunCheck(const char* lock_file_path)
{
    int fd = open(lock_file_path, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd < 0)
    {
        const int err = errno;
        syslog(LOG_ERR, "failed to open file %s, errno: %d, err msg: %s", lock_file_path, err, strerror(err));
        return -1;
    }

    static struct flock fl;

    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_pid = -1;

    if (fcntl(fd, F_SETLK, &fl) < 0)
    {
        const int err = errno;
        syslog(LOG_ERR, "failed to lock file %s, errno: %d, err msg: %s", lock_file_path, err, strerror(err));
        close(fd);
        return -1;
    }

    // 不关闭文件，保持打开加锁状态
    return 0;
}

int RecordPID(const char* pid_file_path)
{
    int fd = open(pid_file_path, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd < 0)
    {
        const int err = errno;
        syslog(LOG_ERR, "failed to open file %s, errno: %d, err msg: %s", pid_file_path, err, strerror(err));
        return -1;
    }

    char buf[64] = "";
    int ret = -1;

    do
    {
        int len = StrPrintf(buf, sizeof(buf), "%lu", (unsigned long) getpid());
        if (len <= 0)
        {
            syslog(LOG_ERR, "failed to build pid string");
            break;
        }

        ssize_t n = write(fd, buf, len);
        if (n <= 0)
        {
            const int err = errno;
            syslog(LOG_ERR, "failed to write pid %s to file %s, errno: %d, err msg: %s", buf, pid_file_path,
                   err, strerror(err));
            break;
        }

        ret = 0;
    } while (0);

    close(fd);
    return ret;
}
}
