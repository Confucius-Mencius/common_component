#ifndef BASE_INC_SPD_LOG_UTIL_H_
#define BASE_INC_SPD_LOG_UTIL_H_

// TODO 修改spdlog源码，日志级别字符串要大写，线程id获取也不对

#include <spdlog/spdlog.h>

struct SpdLogCtx
{
    const char* logger_file_path;
    const char* logger_name;

    SpdLogCtx()
    {
        logger_file_path = NULL;
        logger_name = NULL;
    }
};

int SpdLogInitialize(const SpdLogCtx* ctx);
void SpdLogFinalize();

#define SPDLOG_STR_H(x) #x
#define SPDLOG_STR_HELPER(x) SPDLOG_STR_H(x)

#define SPD_LOG_TRACE(logger_name, format, ...)\
    do {\
        std::ostringstream f("");\
        f << basename(__FILE__) << ":" << SPDLOG_STR_HELPER(__LINE__) << "] " << format;\
        spdlog::get(logger_name)->trace(f.str().c_str(), ##__VA_ARGS__);\
    } while (0)

#define SPD_LOG_DEBUG(logger_name, format, ...)\
    do {\
        std::ostringstream f("");\
        f << basename(__FILE__) << ":" << SPDLOG_STR_HELPER(__LINE__) << "] " << format;\
        spdlog::get(logger_name)->debug(f.str().c_str(), ##__VA_ARGS__);\
    } while (0)

#define SPD_LOG_INFO(logger_name, format, ...)\
    do {\
        std::ostringstream f("");\
        f << basename(__FILE__) << ":" << SPDLOG_STR_HELPER(__LINE__) << "] " << format;\
        spdlog::get(logger_name)->info(f.str().c_str(), ##__VA_ARGS__);\
    } while (0)

#define SPD_LOG_WARN(logger_name, format, ...)\
    do {\
        std::ostringstream f("");\
        f << basename(__FILE__) << ":" << SPDLOG_STR_HELPER(__LINE__) << "] " << format;\
        spdlog::get(logger_name)->warn(f.str().c_str(), ##__VA_ARGS__);\
    } while (0)

#define SPD_LOG_ERROR(logger_name, format, ...)\
    do {\
        std::ostringstream f("");\
        f << basename(__FILE__) << ":" << SPDLOG_STR_HELPER(__LINE__) << "] " << format;\
        spdlog::get(logger_name)->error(f.str().c_str(), ##__VA_ARGS__);\
    } while (0)

#endif // BASE_INC_SPD_LOG_UTIL_H_
