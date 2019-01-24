#ifndef APP_LAUNCHER_SRC_UTIL_H_
#define APP_LAUNCHER_SRC_UTIL_H_

#include <stddef.h>

namespace app_launcher
{
int GetAppName(char* buf, size_t buf_size, const char* argv0);
int SingleRunCheck(const char* lock_file_path);
int RecordPID(const char* pid_file_path);
}

#endif // APP_LAUNCHER_SRC_UTIL_H_
