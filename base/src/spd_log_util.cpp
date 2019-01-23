#include "spd_log_util.h"
//#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>

int SPDLogInitialize(const SPDLogCtx* ctx)
{
    if (NULL == ctx || NULL == ctx->logger_file_path || NULL == ctx->logger_name)
    {
        return -1;
    }

    // 只允许trace -> err这几种日志级别
    if (ctx->log_level < spdlog::level::trace || ctx->log_level > spdlog::level::err)
    {
        return -1;
    }

    spdlog::set_level((spdlog::level::level_enum) ctx->log_level);
    spdlog::set_pattern("[%l %n %t %Y:%m:%d %T %e %v");

//    auto logger = spdlog::rotating_logger_mt(ctx->logger_name, ctx->logger_file_path, 10485760, 10);
    auto logger = spdlog::daily_logger_mt(ctx->logger_name, ctx->logger_file_path, 0, 0);

    logger->flush_on(spdlog::level::info);

    return 0;
}

void SPDLogFinalize()
{
    spdlog::drop_all();
}

int SPDLogSetLevel(int level)
{
    if (level < spdlog::level::trace || level > spdlog::level::err)
    {
        return -1;
    }

    spdlog::set_level((spdlog::level::level_enum) level);
    return 0;
}
