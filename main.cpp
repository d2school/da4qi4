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
    if (!init_server_logger())
    {
        std::cerr << "Create server logger fail." << std::endl;
        return -1;
    }

    server_logger()->info("Wecome to {}", "da4qi4");

    auto svc = Server::Supply(4099);

    auto web = Application::Customize("d2school"
                                      , "/"
                                      , "../d2_daqi/static/"
                                      , "../d2_daqi/view/"
                                      , "../d2_daqi/upload/"
                                     );

    Intercepter::StaticFile static_file;
    static_file.SetCacheMaxAge(600).AddEntry("static/", "/");
    web->AddIntercepter(static_file);

    Intercepter::SessionOnRedis session_redis;
    session_redis.SetHttpOnly(Cookie::HttpOnly::for_http_only);
    web->AddIntercepter(session_redis);

    web->AddHandler(_GET_, "/", [](Context ctx)
    {
        ctx->RenderWithoutData();
        ctx->Pass();
    });

    svc->Mount(web);

    RedisPool().CreateClients(svc->GetIOContextPool());

    try
    {
        svc->Run();
    }
    catch (std::exception const& e)
    {
        server_logger()->error("Run exception. {}.", e.what());
    }

    RedisPool().Stop();
    svc.reset();
    server_logger()->info("Bye.");

    return 0;
}
