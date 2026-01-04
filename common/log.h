#pragma once

#include <memory>
#include "spdlog/spdlog.h"

namespace shc {

extern std::shared_ptr<spdlog::logger> syslog_logger;

#define __FILENAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)
#define __FILE_LINE_FUNC__  __FILENAME__, __LINE__, __FUNCTION__

#define SHC_LOG_TRACE(format, ...)                 \
        syslog_logger->trace("{}:{} {} | " format, \
                              __FILE_LINE_FUNC__,  \
                              ##__VA_ARGS__)

#define SHC_LOG_DEBUG(format, ...)                 \
        syslog_logger->debug("{}:{} {} | " format, \
                              __FILE_LINE_FUNC__,  \
                              ##__VA_ARGS__)

#define SHC_LOG_INFO(format, ...)                  \
        syslog_logger->info("{}:{} {} | " format,  \
                              __FILE_LINE_FUNC__,  \
                              ##__VA_ARGS__)

#define SHC_LOG_WARN(format, ...)                  \
        syslog_logger->warn("{}:{} {} | " format,  \
                              __FILE_LINE_FUNC__,  \
                              ##__VA_ARGS__)

#define SHC_LOG_ERR(format, ...)                   \
        syslog_logger->err("{}:{} {} | " format,   \
                              __FILE_LINE_FUNC__,  \
                              ##__VA_ARGS__)

#define SHC_LOG_CRIT(format, ...)                  \
        syslog_logger->crit("{}:{} {} | " format,  \
                              __FILE_LINE_FUNC__,  \
                              ##__VA_ARGS__)

extern int log_init(void);

} // namespace shc
