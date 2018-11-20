#include "log_def.hpp"

#include <iostream>

namespace da4qi4
{

bool InitServerLogger(const std::string& log_dir)
{
    try
    {
        std::string log_filename = log_dir + "server.log";
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                                     log_filename, 1048576 * 5, 9);

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

LoggerPtr CreateApplicationLoger(std::string const& application_name
                                 , std::string const& application_root_log)
{
    try
    {
        std::string log_filename = application_root_log + application_name + ".log";
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                                     log_filename, 1048576 * 5, 9);

        auto console_sink = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();

        std::vector<spdlog::sink_ptr> sinks {file_sink, console_sink};

        auto app_logger = std::make_shared<spdlog::logger>(application_name
                                                           , std::begin(sinks)
                                                           , std::end(sinks));
        spdlog::register_logger(app_logger);

        return app_logger;
    }
    catch (std::exception const& e)
    {
        std::cerr << e.what() << std::endl;
        return nullptr;
    }
}

} //namespace da4qi4
