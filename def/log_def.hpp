#ifndef DAQI_LOG_DEF_HPP
#define DAQI_LOG_DEF_HPP

#include <memory>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/sinks/file_sinks.h"

namespace da4qi4
{

namespace log
{

using Level = spdlog::level::level_enum;
using LoggerPtr = std::shared_ptr<spdlog::logger>;

bool InitServerLogger(std::string const& log_dir, Level level = Level::info,
                      size_t max_file_size_kb = 5 * 1024,
                      size_t max_file_count = 9);

LoggerPtr CreateAppLogger(std::string const& application_name,
                          std::string const& application_root_log,
                          Level level,
                          size_t max_file_size_kb,
                          size_t max_file_count);

LoggerPtr Null();
LoggerPtr Server();
LoggerPtr App(std::string const& application_name);

void SetLogLevel(Level level);
void SetServerLogLevel(Level level);
void SetAppLogLevel(std::string const& application_name, Level level);

} // namespace log

}
#endif // DAQI_LOG_DEF_HPP
