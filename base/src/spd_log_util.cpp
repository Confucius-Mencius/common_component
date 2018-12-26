#include "spd_log_util.h"
//#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>

void SpdLogInitialize(const SpdLogCtx* ctx)
{
    spdlog::set_level(spdlog::level::trace);
    spdlog::set_pattern("[%l %n %t %Y:%m:%d %T %e %v");

//    auto logger = spdlog::rotating_logger_mt(SPD_LOGGER_NAME, ctx->logger_file_path, 10485760, 10);
    auto logger = spdlog::daily_logger_mt(SPD_LOGGER_NAME, ctx->logger_file_path, 0, 0);

    logger->flush_on(spdlog::level::info);
}

void SpdLogFinalize()
{
    spdlog::drop_all();
}
