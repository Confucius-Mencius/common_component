#include "daemonize.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <unistd.h>
#include "str_util.h"

namespace app_launcher
{
static const int MAX_LINE = 1024; /* max line length */

/*
 * Print a message and return to caller.
 * Caller specifies "errnoflag".
 */
static void err_doit(int errnoflag, int err, const char* fmt, va_list ap)
{
    char buf[MAX_LINE];

    vsnprintf(buf, MAX_LINE - 1, fmt, ap);
    if (errnoflag)
    {
        snprintf(buf + strlen(buf), MAX_LINE - strlen(buf) - 1, ". errno: %d, err msg: %s", err, strerror(err));
    }

    strcat(buf, "\n");
    fflush(stdout);        /* in case stdout and stderr are the same */
    fputs(buf, stderr);
    fflush(NULL);        /* flushes all stdio output streams */
}

/*
 * Fatal error unrelated to a system call.
 * Print a message and terminate.
 */
static void err_quit(const char* fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    err_doit(1, errno, fmt, ap);
    va_end(ap);

    exit(1);
}

void Daemonize(const char* cmd, int chdir_to_root)
{
    int i, fd0, fd1, fd2;
    pid_t pid;
    struct rlimit rl;
    struct sigaction sa;

    /*
     * Clear file creation mask.
     */
    umask(0);

    /*
     * Get maximum number of file descriptors.
     */
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
    {
        err_quit("%s: can't get file limit", cmd);
    }

    /*
     * Become a session leader to lose controlling TTY.
     */
    if ((pid = fork()) < 0)
    {
        err_quit("%s: can't fork", cmd);
    }
    else if (pid != 0) /* parent */
    {
        exit(0);
    }

    setsid();

    /*
     * Ensure future opens won't allocate controlling TTYs.
     */
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGHUP, &sa, NULL) < 0)
    {
        err_quit("%s: can't ignore SIGHUP", cmd);
    }

    if ((pid = fork()) < 0)
    {
        err_quit("%s: can't fork", cmd);
    }
    else if (pid != 0) /* parent */
    {
        exit(0);
    }

    /*
    * Change the current working directory to the root so
    * we won't prevent file systems from being unmounted.
    */
    if (chdir_to_root)
    {
        if (chdir("/") < 0)
        {
            err_quit("%s: can't change directory to /", cmd);
        }
    }

    /*
     * Close all open file descriptors.
     */
    if (rl.rlim_max == RLIM_INFINITY)
    {
        rl.rlim_max = 1024;
    }

    for (i = 0; i < (int) rl.rlim_max; i++)
    {
        close(i);
    }

    /*
     * Attach file descriptors 0, 1, and 2 to /dev/null.
     */

    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);

    /*
     * Initialize the log file.
     */
    openlog(cmd, LOG_CONS, LOG_DAEMON);
    if (fd0 != 0 || fd1 != 1 || fd2 != 2)
    {
        err_quit("%s: unexpected file descriptors %d %d %d", cmd, fd0, fd1, fd2);
    }
}

int GetAppName(char* buf, size_t buf_size, const char* argv0)
{
    if (NULL == buf || buf_size < 2 || NULL == argv0)
    {
        return -1;
    }

    const char* last_slash = strrchr(argv0, '/');
    if (NULL == last_slash)
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
