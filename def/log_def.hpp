#ifndef DAQI_LOG_DEF_HPP
#define DAQI_LOG_DEF_HPP

#include <memory>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/sinks/file_sinks.h"

namespace da4qi4
{

bool init_server_logger();

inline std::shared_ptr<spdlog::logger> server_logger()
{
    return spdlog::get("server");
}

}
#endif // DAQI_LOG_DEF_HPP
