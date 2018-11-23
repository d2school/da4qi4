#include <iostream>

#include "da4qi4.hpp"

#include "intercepters/static_file.hpp"
#include "intercepters/session_redis.hpp"

#include "tools/aliyun_sms/sms_client.hpp"

using namespace da4qi4;

int main()
{
    char const* accessKeyId = "LTAIUKE0VtpC1M2g";
    char const* secretAccessKey = "Qny6V8pQFYzMeIGgawwzHSFXiAKQiN";
    std::string code = "9845";
    IOC ioc;
    alisms::SmsClient::Ptr cli = alisms::SmsClient::Create(ioc, accessKeyId, secretAccessKey);
    cli->Request([](alisms::SmsClient::Ptr client, errorcode const & ec)
    {
        if (ec)
        {
            std::cout << ec.message() << std::endl;

            if (client->GetErrorStep())
            {
                std::cerr << client->GetErrorMsg() << std::endl;
            }

            return;
        }
        else
        {
            std::cout << client->GetResponseHeader() << "\r\n" << std::endl;
            std::cout << client->GetResponseBody() << std::endl;
        }
    }, "18650125181", "第2学堂", "SMS_127167744", "{\"code\":\"" + code + "\"}");

    // ioc.run();


    /////////////////////////////////////////////

    std::string www_root = "../d2_daqi/";

    if (!log::InitServerLogger(www_root + "logs/"))
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

    if (!web->Init(Application::NeedLogger::yes))
    {
        std::cerr << "Init application " << web->GetName() << " fail." << std::endl;
        return -2;
    }

    Intercepter::StaticFile static_file(web->GetName());
    static_file.SetCacheMaxAge(600).AddEntry("static/", "/");
    web->AddIntercepter(static_file);

    Intercepter::SessionOnRedis session_redis(web->GetName());
    session_redis.SetHttpOnly(Cookie::HttpOnly::for_http_only);
    web->AddIntercepter(session_redis);

    web->AddHandler(_GET_, "/", [](Context ctx)
    {
        Json data;
        data["names"] = {"张三", "李四", "王五"};
        ctx->Render(data);
        ctx->Pass();
    });

    if (!svc->Mount(web))
    {
        std::cerr << "Mount application " << web->GetName() << " fail." << std::endl;
        return -2;
    }

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
