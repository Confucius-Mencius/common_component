#include "spd_log_util.h"
//#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>

int SpdLogInitialize(const SpdLogCtx* ctx)
{
    if (NULL == ctx || NULL == ctx->logger_file_path || NULL == ctx->logger_name)
    {
        return -1;
    }

    spdlog::set_level(spdlog::level::trace);
    spdlog::set_pattern("[%l %n %t %Y:%m:%d %T %e %v");

//    auto logger = spdlog::rotating_logger_mt(ctx->logger_name, ctx->logger_file_path, 10485760, 10);
    auto logger = spdlog::daily_logger_mt(ctx->logger_name, ctx->logger_file_path, 0, 0);

    logger->flush_on(spdlog::level::info);

    return 0;
}

void SpdLogFinalize()
{
    spdlog::drop_all();
}
