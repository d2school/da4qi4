#ifndef DAQI_LOG_DEF_HPP
#define DAQI_LOG_DEF_HPP

#include <memory>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/sinks/file_sinks.h"

namespace da4qi4
{

using LoggerPtr = std::shared_ptr<spdlog::logger>;

bool InitServerLogger(std::string const& log_dir);

inline LoggerPtr ServerLogger()
{
    return spdlog::get("server");
}

LoggerPtr CreateApplicationLoger(std::string const& application_name
                                 , std::string const& application_root_log);

LoggerPtr AppLogger(std::string const& application_name);
}
#endif // DAQI_LOG_DEF_HPP
