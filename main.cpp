#include <ctime>

#include <iostream>

#include "def/log_def.hpp"

#include "server.hpp"
#include "router.hpp"
#include "application.hpp"

#include "intercepters/static_file.hpp"
#include "intercepters/session_redis.hpp"

using namespace da4qi4;

int main()
{
    auto console = spdlog::stdout_color_st("console");
    console->info("Wecome to {}", "da4qi4");

    auto svc = Server::Supply(4099);

    console->info("Server start at {}", 4099);

    auto admin = Application::Customize("d2-admin"
                                        , "/admin/"
                                        , "../d2_daqi/static/"
                                        , "../d2_daqi/view/admin/"
                                        , "../d2_daqi/upload/admin/"
                                       );

    Intercepter::StaticFile static_file;
    static_file.SetCacheMaxAge(600).AddEntry("layui/", "layui/");
    admin->AddIntercepter(static_file);

    Intercepter::SessionOnRedis session_redis;
    session_redis.SetHttpOnly(Cookie::HttpOnly::for_http_only);
    admin->AddIntercepter(session_redis);

    admin->AddHandler(_GET_, "/new_lesson", [](Context ctx)
    {
        ctx->RenderWithoutData();
        ctx->Pass();
    });

    svc->Mount(admin);
    console->info("App {} mounted.", admin->GetName());

    RedisPool().CreateClients(svc->GetIOContextPool());

    try
    {
        svc->Run();
    }
    catch (std::exception const& e)
    {
        console->error("Server Run Fail ! {}", e.what());
    }

    RedisPool().Stop();
    svc.reset();
    std::cout << "Bye." << std::endl;

    return 0;
}
