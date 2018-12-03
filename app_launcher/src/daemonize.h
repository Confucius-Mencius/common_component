#ifndef APP_LAUNCHER_SRC_DAEMONIZE_H_
#define APP_LAUNCHER_SRC_DAEMONIZE_H_

#include <stddef.h>

namespace app_launcher
{
void Daemonize(const char* cmd, int chdir_to_root);
int GetAppName(char* buf, size_t buf_size, const char* argv0);
int SingleRunCheck(const char* lock_file_path);
int RecordPID(const char* pid_file_path);
}

#endif // APP_LAUNCHER_SRC_DAEMONIZE_H_
