#include <iostream>
#include <fstream>

#include "spdlog/spdlog.h"
#include "spdlog/cfg/env.h"
#include "spdlog/sinks/syslog_sink.h"
#include "gflags/gflags.h"

using namespace std;
using namespace spdlog;

namespace shc {

std::shared_ptr<spdlog::logger> syslog_logger = nullptr;

spdlog::level::level_enum sfc_string_2_loglevel(const std::string &loglevel){
    static std::unordered_map<std::string, spdlog::level::level_enum> log_map = {
        {"trace", spdlog::level::trace},
        {"debug", spdlog::level::debug},
        {"info",  spdlog::level::info},
        {"warn",  spdlog::level::warn},
        {"err",   spdlog::level::err},
        {"crit",  spdlog::level::critical}
    };

    auto iter = log_map.find(loglevel);
    if ( log_map.end() != iter ) {
        return iter->second;
    }
    return spdlog::level::info;
}

DEFINE_string(loglevel, "info", "Log level.");

int log_init(void) {
    spdlog::set_level(sfc_string_2_loglevel(FLAGS_loglevel)); // Set global log level
    spdlog::set_pattern("[%H:%M:%S %z] [%^%L%$] [thread %t] %v");
    std::string ident = "sfc";
    syslog_logger = spdlog::syslog_logger_mt("syslog", ident, LOG_PID);
    syslog_logger->info("Init loglevel: {}.", FLAGS_loglevel);
    return 0;
}

} // namespace shc
