#include "def/log_def.hpp"

#include <iostream>

#include "spdlog/sinks/null_sink.h"

#include "def/def.hpp"
#include "application.hpp"

namespace da4qi4
{

namespace
{

std::shared_ptr<spdlog::logger> the_null_log;

struct InitNullLogger
{
    InitNullLogger()
    {
        auto null_sink = std::make_shared<spdlog::sinks::null_sink_mt>();
        the_null_log = std::make_shared<spdlog::logger>("null", null_sink);
    }
};

}

namespace log
{

LoggerPtr Null()
{
    static InitNullLogger _init_null_log_;
    return the_null_log;
}

bool InitServerLogger(const std::string& log_dir, Level level, size_t max_file_size_kb, size_t max_file_count)
{
    try
    {
        std::string log_filename = log_dir + "server.log";
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                                     log_filename, 1024 * max_file_size_kb, max_file_count);

        auto console_sink = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();

        std::vector<spdlog::sink_ptr> sinks {file_sink, console_sink};

        auto server_log = std::make_shared<spdlog::logger>("server"
                                                           , std::begin(sinks)
                                                           , std::end(sinks));
        server_log->set_level(level);
        spdlog::register_logger(server_log);

        server_log->info("Welcome to {}.", the_daqi_name);

        return true;
    }
    catch (std::exception const& e)
    {
        std::cerr << e.what() << std::endl;
        return false;
    }
}

LoggerPtr CreateAppLogger(std::string const& application_name
                          , std::string const& application_root_log
                          , Level level
                          , size_t max_file_size_kb, size_t max_file_count)
{
    try
    {
        std::string log_filename = application_root_log + application_name + ".log";
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                                     log_filename, 1024 * max_file_size_kb, max_file_count);

        auto console_sink = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();

        std::vector<spdlog::sink_ptr> sinks {file_sink, console_sink};

        auto app_log = std::make_shared<spdlog::logger>(application_name
                                                        , std::begin(sinks)
                                                        , std::end(sinks));
        app_log->set_level(level);
        spdlog::register_logger(app_log);

        return app_log;
    }
    catch (std::exception const& e)
    {
        std::cerr << e.what() << std::endl;
        return nullptr;
    }
}

LoggerPtr Server()
{
    auto log = spdlog::get("server");
    return (log ? log : Null());
}

LoggerPtr App(std::string const& application_name)
{
    return AppMgr().GetApplicationLogger(application_name);
}

void SetLogLevel(Level level)
{
    spdlog::set_level(level);
}

void SetServerLogLevel(Level level)
{
    Server()->set_level(level);
}

void SetAppLogLevel(std::string const& application_name, Level level)
{
    App(application_name)->set_level(level);
}


} // namespace log
} //namespace da4qi4
