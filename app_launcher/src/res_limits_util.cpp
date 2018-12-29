#include "res_limits_util.h"
#include <string.h>
#include <sys/resource.h>
#include "log_util.h"
#include "num_util.h"
#include "shell_util.h"
#include "str_util.h"

namespace app_launcher
{
static const int MAX_LINE = 1024; /* max line length */

void PrintResLimits(char* name, int resource)
{
    struct rlimit limit;

    if (getrlimit(resource, &limit) < 0)
    {
        const int err = errno;
        LOG_ERROR("failed to getrlimit, errno: " << err << ", err msg: " << strerror(err));
        return;
    }

    char buf[MAX_LINE] = "";
    size_t offset = 0;

    int n = StrPrintf(buf + offset, sizeof(buf) - offset, "%-24s", name);
    offset += n;

    if (limit.rlim_cur == RLIM_INFINITY)
    {
        n = StrPrintf(buf + offset, sizeof(buf) - offset, "(infinite)     ");
        offset += n;
    }
    else
    {
        n = StrPrintf(buf + offset, sizeof(buf) - offset, "%-15ld", limit.rlim_cur);
        offset += n;
    }

    if (limit.rlim_max == RLIM_INFINITY)
    {
        n = StrPrintf(buf + offset, sizeof(buf) - offset, "(infinite)     ");
        offset += n;
    }
    else
    {
        n = StrPrintf(buf + offset, sizeof(buf) - offset, "%-15ld", limit.rlim_max);
        offset += n;
    }

    LOG_INFO(buf);
}

struct ResEntry
{
    char name[24];
    int id;
};

static ResEntry res_entries[] =
{
    { "LIMIT_AS",          RLIMIT_AS }, // The maximum size of the process's virtual memory (address space) in bytes
    { "RLIMIT_CORE",       RLIMIT_CORE }, // Maximum size of a core file
    { "RLIMIT_CPU",        RLIMIT_CPU }, // CPU time limit in seconds
    { "RLIMIT_DATA",       RLIMIT_DATA }, // The  maximum  size  of  the process's data segment (initialized data, uninitialized data, and heap)
    { "RLIMIT_FSIZE",      RLIMIT_FSIZE }, // The maximum size of files that the process may create
    { "RLIMIT_LOCKS",      RLIMIT_LOCKS }, // A limit on the combined number of flock(2) locks and fcntl(2) leases that this process may establish
    { "RLIMIT_MEMLOCK",    RLIMIT_MEMLOCK }, // The  maximum  number  of  bytes of memory that may be locked into RAM
    { "RLIMIT_MSGQUEUE",   RLIMIT_MSGQUEUE }, // Specifies  the  limit on the number of bytes that can be allocated for POSIX message queues for the real user ID of the calling process
    { "RLIMIT_NICE",       RLIMIT_NICE }, // Specifies  a  ceiling  to  which the process's nice value can be raised using setpriority(2) or nice(2)
    { "RLIMIT_NOFILE",     RLIMIT_NOFILE }, // Specifies a value one greater than the maximum file descriptor number that can be opened by this process
    { "RLIMIT_NPROC",      RLIMIT_NPROC }, // The maximum number of processes (or, more precisely on Linux, threads) that can be created for the real user ID of the calling process
    { "RLIMIT_RSS",        RLIMIT_RSS }, // Specifies the limit (in bytes) of the process's resident set (the number of virtual pages resident in RAM)
    { "RLIMIT_RTPRIO",     RLIMIT_RTPRIO }, // Specifies a ceiling on the real-time priority that may be set for this process using sched_setscheduler(2) and sched_setparam(2)
    { "RLIMIT_RTTIME",     RLIMIT_RTTIME }, // Specifies  a limit (in microseconds) on the amount of CPU time that a process scheduled under a real-time scheduling policy may consume without making a blocking system call
    { "RLIMIT_SIGPENDING", RLIMIT_SIGPENDING }, // Specifies the limit on the number of signals that may be queued for the real user ID of the calling process
    { "RLIMIT_STACK",      RLIMIT_STACK } // The maximum size of the process stack, in bytes
};

// 进程资源限制
void PrintAllResLimits()
{
    char buf[MAX_LINE] = "";
    StrPrintf(buf, sizeof(buf), "%-24s%-15s%-15s", "[name]", "[soft limit]", "[hard limit]");
    LOG_INFO(buf);

    for (int i = 0; i < COUNT_OF(res_entries); ++i)
    {
        PrintResLimits(res_entries[i].name, res_entries[i].id);
    }

    ExecShellCmd(buf, sizeof(buf), "cat /proc/sys/kernel/core_pattern");
    LOG_INFO("cat /proc/sys/kernel/core_pattern: " << buf);

    ExecShellCmd(buf, sizeof(buf), "cat /proc/sys/kernel/core_uses_pid");
    LOG_INFO("cat /proc/sys/kernel/core_uses_pid: " << buf);
}
}
