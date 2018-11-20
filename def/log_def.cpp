#include "log_def.hpp"

#include <iostream>

namespace da4qi4
{

bool init_server_logger()
{
    try
    {
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                                     "../d2_daqi/logs/server.log", 1048576 * 5, 9);

        auto console_sink = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();

        std::vector<spdlog::sink_ptr> sinks {file_sink, console_sink};

        auto server_logger = std::make_shared<spdlog::logger>("server"
                                                              , std::begin(sinks)
                                                              , std::end(sinks));
        spdlog::register_logger(server_logger);

        return true;
    }
    catch (std::exception const& e)
    {
        std::cerr << e.what() << std::endl;
        return false;
    }
}

}
