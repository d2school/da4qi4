#include <iostream>

#include "server.hpp"
#include "application.hpp"

#include "utilities/captcha_utilities.hpp"

int main()
{
    std::srand(std::time(nullptr));
    using namespace da4qi4;

    for (int i = 0; i < 100; ++i)
    {
        std::string captcha_text = Utilities::get_random_words_for_captcha(4, true);
        std::cout << captcha_text << std::endl;
        std::string fn;

        for (auto c : captcha_text)
        {
            if (Utilities::is_symbol_captcha_char(c))
            {
                c = '_';
            }

            fn.push_back(c);
        }

        fn = "./tmp/" + std::to_string(i) + "-" + fn + ".gif";
        Utilities::make_captcha_image(captcha_text, fn);
    }

    return -1;
    boost::asio::io_context ioc;

    try
    {
        auto svc = Server::Supply(ioc, 4099);
        Application app1("d2school", "/");
        app1.AddHandler(_GET_, ""_router_starts, [](Context ctx)
        {
            ctx->Res().AddHeader("Context-Type", "text/html; charset=utf-8");
            ctx->Res().SetBody("<!DOCTYPE html><html lang=\"zh-cn\"><body><h1>D2SCHOOL</h1></body></html>");
            ctx->Bye();
        });
        app1.AddHandler(_GET_, "test/"_router_starts, [](Context ctx)
        {
            ctx->Res().AddHeader("Context-Type", "text/html; charset=utf-8");
            ctx->Res().SetBody("<!DOCTYPE html><html lang=\"zh-cn\"><body><h2>D2SCHOOL/TEST</h2></body></html>");
            ctx->Bye();
        });
        Application app2("d2school-admin", "/admin/");
        app2.AddHandler(_GET_, ""_router_starts, [](Context ctx)
        {
            ctx->Res().AddHeader("Context-Type", "text/html; charset=utf-8");
            ctx->Res().SetBody("<!DOCTYPE html><html lang=\"zh-cn\"><body><h1>D2SCHOOL-ADMIN</h1></body></html>");
            ctx->Bye();
        });
        svc->AddApp(app1);
        svc->AddApp(app2);
        svc->AddHandler(_GET_, "/admin/test/"_router_starts, [](Context ctx)
        {
            ctx->Res().AddHeader("Context-Type", "text/html; charset=utf-8");
            ctx->Res().SetBody("<!DOCTYPE html><html lang=\"zh-cn\"><body><h1>D2SCHOOL-ADMIN/TEST</h1></body></html>");
            ctx->Bye();
        });
        svc->AddHandler(_POST_, "/"_router_starts, [](Context ctx)
        {
            ctx->Res().AddHeader("Context-Type", "text/html; charset=utf-8");
            ctx->Res().SetBody("<!DOCTYPE html><html lang=\"zh-cn\"><body><h1>D2SCHOOL-POST</h1></body></html>");
            ctx->Bye();
        });
        svc->Start();
        ioc.run();
    }
    catch (std::exception const& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
