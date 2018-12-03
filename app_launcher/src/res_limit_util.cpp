#include "res_limit_util.h"
#include <string.h>
#include <sys/resource.h>
#include "log_util.h"
#include "misc_util.h"
#include "num_util.h"
#include "str_util.h"

void PrintResLimit(char* name, int resource)
{
    struct rlimit limit;

    if (getrlimit(resource, &limit) < 0)
    {
        const int err = errno;
        LOG_ERROR("failed to getrlimit, errno: " << err << ", err msg: " << strerror(err));
        return;
    }

    char buf[1024] = "";
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

struct ResourceEntry
{
    char name[24];
    int id;
};

// 进程资源限制
void PrintAllResLimits()
{
    ResourceEntry resource_entries[] =
    {
        {"LIMIT_AS",          RLIMIT_AS},
        {"RLIMIT_CORE",       RLIMIT_CORE},
        {"RLIMIT_CPU",        RLIMIT_CPU},
        {"RLIMIT_DATA",       RLIMIT_DATA},
        {"RLIMIT_FSIZE",      RLIMIT_FSIZE},
        {"RLIMIT_LOCKS",      RLIMIT_LOCKS},
        {"RLIMIT_MEMLOCK",    RLIMIT_MEMLOCK},
        {"RLIMIT_MSGQUEUE",   RLIMIT_MSGQUEUE},
        {"RLIMIT_NICE",       RLIMIT_NICE},
        {"RLIMIT_NOFILE",     RLIMIT_NOFILE},
        {"RLIMIT_NPROC",      RLIMIT_NPROC},
        {"RLIMIT_RSS",        RLIMIT_RSS},
        {"RLIMIT_RTPRIO",     RLIMIT_RTPRIO},
        // { "RLIMIT_RTTIME",     RLIMIT_RTTIME },
        {"RLIMIT_SIGPENDING", RLIMIT_SIGPENDING},
        {"RLIMIT_STACK",      RLIMIT_STACK}
    };

    char buf[1024] = "";
    StrPrintf(buf, sizeof(buf), "%-24s%-15s%-15s", "[name]", "[soft limit]", "[hard limit]");
    LOG_INFO(buf);

    for (int i = 0; i < COUNT_OF(resource_entries); ++i)
    {
        PrintResLimit(resource_entries[i].name, resource_entries[i].id);
    }

    ExecShellCmd(buf, sizeof(buf), "cat /proc/sys/kernel/core_pattern");
    LOG_INFO("cat /proc/sys/kernel/core_pattern: " << buf);

    ExecShellCmd(buf, sizeof(buf), "cat /proc/sys/kernel/core_uses_pid");
    LOG_INFO("cat /proc/sys/kernel/core_uses_pid: " << buf);
}
