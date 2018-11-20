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
    std::string www_root = "../d2_daqi/";

    if (!InitServerLogger(www_root + "logs/"))
    {
        std::cerr << "Create server logger fail." << std::endl;
        return -1;
    }

    ServerLogger()->info("Wecome to {}", "da4qi4");

    auto svc = Server::Supply(4099);

    auto web = Application::Customize("d2school"
                                      , "/"
                                      , www_root + "logs/"
                                      , www_root + "static/"
                                      , www_root + "view/"
                                      , www_root + "upload/"
                                     );

    if (!web->Init())
    {
        std::cerr << "Init application " << web->GetName() << " fail." << std::endl;
        return -2;
    }

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

    if (!svc->Mount(web))
    {
        std::cerr << "Mount application " << web->GetName() << " fail." << std::endl;
        return -2;
    }

    RedisPool().CreateClients(svc->GetIOContextPool());

    try
    {
        svc->Run();
    }
    catch (std::exception const& e)
    {
        ServerLogger()->error("Run exception. {}.", e.what());
    }

    RedisPool().Stop();
    svc.reset();
    ServerLogger()->info("Bye.");

    return 0;
}
