#ifndef BASE_INC_SPD_LOG_UTIL_H_
#define BASE_INC_SPD_LOG_UTIL_H_

#include <spdlog/spdlog.h>
#include "file_util.h"

#define SPD_LOGGER_NAME "spdlog"

struct SpdLogCtx
{
    char logger_file_path[MAX_PATH_LEN + 1];

    SpdLogCtx()
    {
        logger_file_path[0] = '\0';
    }
};

void SpdLogInitialize(const SpdLogCtx* ctx);
void SpdLogFinalize();

#define SPDLOG_STR_H(x) #x
#define SPDLOG_STR_HELPER(x) SPDLOG_STR_H(x)

#define SPD_LOG_TRACE(format, ...)\
    do {\
        std::ostringstream f("");\
        f << basename(__FILE__) << ":" << SPDLOG_STR_HELPER(__LINE__) << "] " << format;\
        spdlog::get(SPD_LOGGER_NAME)->trace(f.str().c_str(), ##__VA_ARGS__);\
    } while (0)

#define SPD_LOG_DEBUG(format, ...)\
    do {\
        std::ostringstream f("");\
        f << basename(__FILE__) << ":" << SPDLOG_STR_HELPER(__LINE__) << "] " << format;\
        spdlog::get(SPD_LOGGER_NAME)->debug(f.str().c_str(), ##__VA_ARGS__);\
    } while (0)

#define SPD_LOG_INFO(format, ...)\
    do {\
        std::ostringstream f("");\
        f << basename(__FILE__) << ":" << SPDLOG_STR_HELPER(__LINE__) << "] " << format;\
        spdlog::get(SPD_LOGGER_NAME)->info(f.str().c_str(), ##__VA_ARGS__);\
    } while (0)

#define SPD_LOG_WARN(format, ...)\
    do {\
        std::ostringstream f("");\
        f << basename(__FILE__) << ":" << SPDLOG_STR_HELPER(__LINE__) << "] " << format;\
        spdlog::get(SPD_LOGGER_NAME)->warn(f.str().c_str(), ##__VA_ARGS__);\
    } while (0)

#define SPD_LOG_ERROR(format, ...)\
    do {\
        std::ostringstream f("");\
        f << basename(__FILE__) << ":" << SPDLOG_STR_HELPER(__LINE__) << "] " << format;\
        spdlog::get(SPD_LOGGER_NAME)->error(f.str().c_str(), ##__VA_ARGS__);\
    } while (0)

#endif // BASE_INC_SPD_LOG_UTIL_H_
