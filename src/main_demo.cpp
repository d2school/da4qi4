#include <iostream>

#include "daqi/da4qi4.hpp"

#include "daqi/intercepters/static_file.hpp"
#include "daqi/intercepters/session_redis.hpp"

using namespace da4qi4;

#ifdef _BUILD_DAQI_DEMO_SERVER_

int main()
{
    std::string www_root = "../d2_daqi/";

    if (!log::InitServerLogger(www_root + "logs/", log::Level::debug))
    {
        std::cerr << "Create server logger fail." << std::endl;
        return -1;
    }

    auto svc = Server::Supply(4099);

    std::string app_root = www_root;
    auto web = Application::Customize("web"
                                      , "/"
                                      , app_root + "logs/"
                                      , app_root + "static/"
                                      , app_root + "view/"
                                      , app_root + "upload/"
                                     );

    if (!web->Init(Application::ActualLogger::yes))
    {
        std::cerr << "Init application " << web->GetName() << " fail." << std::endl;
        return -2;
    }

    Intercepter::StaticFile static_file;
    static_file.SetCacheMaxAge(600).AddEntry("css/").AddEntry("js/").AddEntry("img/")
    .AddEntry("favicon.ico", "img/favicon.ico");

    web->AddIntercepter(static_file);

    Intercepter::SessionOnRedis session_redis;
    web->AddIntercepter(session_redis);

    web->AddHandler(_GET_, "/", [](Context ctx)
    {
        ctx->RenderWithoutData();
        ctx->Pass();
    });

    web->AddHandler(_GET_, "/login", [](Context ctx)
    {
        ctx->RenderWithoutData();
        ctx->Pass();
    });
    web->AddHandler(_GET_, "/regist", [](Context ctx)
    {
        ctx->RenderWithoutData();
        ctx->Pass();
    });

    if (!svc->Mount(web))
    {
        std::cerr << "Mount application " << web->GetName() << " fail." << std::endl;
        return -2;
    }

    svc->SetIdleTimerInterval(5);
    svc->EnableDetectTemplates();

    RedisPool().CreateClients(svc->GetIOContextPool());

    try
    {
        svc->Run();
    }
    catch (std::exception const& e)
    {
        log::Server()->error("Run exception. {}.", e.what());
    }

    RedisPool().Stop();
    svc.reset();
    log::Server()->info("Bye.");

    return 0;
}

#endif
