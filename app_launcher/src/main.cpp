#include <errno.h>
#include <syslog.h>
#include <unistd.h>
#include <gflags/gflags.h>
#include "app_launcher.h"
#include "str_util.h"
#include "daemonize.h"

DEFINE_string(log_conf_file_path, "xx_server_log_conf.properties", "log conf file path");
DEFINE_string(logger_name, "xx_server", "logger name in the log conf file");
DEFINE_string(app_conf_file_path, "xx_server_conf.xml", "app conf file path");
DEFINE_string(common_component_dir, "common_component_dir", "common component dir");
DEFINE_bool(daemon, true, "whether run as daemon");
DEFINE_bool(chdir_to_root, false, "whether change working dir to '/'");

int main(int argc, char* argv[])
{
    char buf[1024] = "";

    if (StrPrintf(buf, sizeof(buf), "%s -log_conf_file_path=? -logger_name=? -app_conf_file_path=? "
                  "-common_component_dir=? -daemon=true/false chdir_to_root=ture/false", argv[0]) <= 0)
    {
        return -1;
    }

    gflags::SetUsageMessage(buf);

    if (argc != 7)
    {
        gflags::ShowUsageWithFlags(argv[0]);
        return -1;
    }

    gflags::ParseCommandLineFlags(&argc, &argv, false);

    app_launcher::AppLauncherCtx app_launcher_ctx;
    app_launcher_ctx.argc = argc;
    app_launcher_ctx.argv = argv;

    StrCpy(app_launcher_ctx.common_component_dir, sizeof(app_launcher_ctx.common_component_dir),
           FLAGS_common_component_dir.c_str());

    StrCpy(app_launcher_ctx.log_conf_file_path, sizeof(app_launcher_ctx.log_conf_file_path),
           FLAGS_log_conf_file_path.c_str());

    StrCpy(app_launcher_ctx.logger_name, sizeof(app_launcher_ctx.logger_name), FLAGS_logger_name.c_str());

    StrCpy(app_launcher_ctx.app_conf_file_path, sizeof(app_launcher_ctx.app_conf_file_path),
           FLAGS_app_conf_file_path.c_str());

    bool run_as_daemon = FLAGS_daemon;
    bool chdir_to_root = FLAGS_chdir_to_root;

    gflags::ShutDownCommandLineFlags();

    if (run_as_daemon)
    {
        app_launcher::Daemonize(argv[0], chdir_to_root);
    }

    if (NULL == getcwd(app_launcher_ctx.cur_working_dir, sizeof(app_launcher_ctx.cur_working_dir)))
    {
        const int err = errno;
        syslog(LOG_ERR, "getcwd failed, errno: %d, err msg: %s", err, strerror(err));
        return -1;
    }

    if (app_launcher::GetAppName(app_launcher_ctx.app_name, sizeof(app_launcher_ctx.app_name), argv[0]) != 0)
    {
        return -1;
    }

    // 可能传入的是相对路径，转为绝对路径
    char common_component_dir[MAX_PATH_LEN] = "";
    GetAbsolutePath(common_component_dir, sizeof(common_component_dir), app_launcher_ctx.common_component_dir, app_launcher_ctx.cur_working_dir);
    StrCpy(app_launcher_ctx.common_component_dir, sizeof(app_launcher_ctx.common_component_dir), common_component_dir);

    char log_conf_file_path[MAX_PATH_LEN] = "";
    GetAbsolutePath(log_conf_file_path, sizeof(log_conf_file_path), app_launcher_ctx.log_conf_file_path, app_launcher_ctx.cur_working_dir);
    StrCpy(app_launcher_ctx.log_conf_file_path, sizeof(app_launcher_ctx.log_conf_file_path), log_conf_file_path);

    char app_conf_file_path[MAX_PATH_LEN] = "";
    GetAbsolutePath(app_conf_file_path, sizeof(app_conf_file_path), app_launcher_ctx.app_conf_file_path, app_launcher_ctx.cur_working_dir);
    StrCpy(app_launcher_ctx.app_conf_file_path, sizeof(app_launcher_ctx.app_conf_file_path), app_conf_file_path);

    app_launcher::AppLauncher* app_launcher = app_launcher::AppLauncher::Create();
    if (NULL == app_launcher)
    {
        // the information will write to
        // '/var/log/syslog' on ubuntu
        // or '/var/log/messages' on centos
        syslog(LOG_ERR, "failed to create app launcher");
        return -1;
    }

    int ret = -1;

    do
    {
        if (app_launcher->Initialize(&app_launcher_ctx) != 0)
        {
            syslog(LOG_ERR, "failed to init app launcher: %s", app_launcher->GetLastErrMsg());
            break;
        }

        if (app_launcher->Activate() != 0)
        {
            break;
        }

        ret = 0;
    } while (0);

    if (ret != 0)
    {
        app_launcher->OnStop();
    }

    SAFE_DESTROY(app_launcher);
    closelog();

    return ret;
}
