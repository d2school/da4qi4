#include <iostream>

#include "server.hpp"
#include "application.hpp"

#include "def/log_def.hpp"
#include "utilities/captcha_utilities.hpp"

using namespace da4qi4;

int main()
{
    auto console = spdlog::stdout_color_mt("console");
    console->info("Wecome to da4qi4");

    try
    {
        boost::asio::io_context ioc;
        auto svc = Server::Supply(ioc, 4099);
        console->info("Server start at {}", 4099);
        Application app1("d2school", "/");
        app1.AddHandler(_GET_, ""_router_starts, [](Context ctx)
        {
            ctx->Res().Ok("<!DOCTYPE html><html lang=\"zh-cn\"><body><h1>D2SCHOOL</h1></body></html>");
            ctx->Bye();
        });
        app1.AddHandler(_GET_, "test/"_router_starts, [](Context ctx)
        {
            ctx->Res().Ok("<!DOCTYPE html><html lang=\"zh-cn\"><body><h2>D2SCHOOL/TEST</h2></body></html>");
            ctx->Bye();
        });
        Application app2("d2school-admin", "/admin/");
        app2.AddHandler(_GET_, ""_router_starts, [](Context ctx)
        {
            ctx->Res().Ok("<!DOCTYPE html><html lang=\"zh-cn\"><body><h1>D2SCHOOL-ADMIN</h1></body></html>");
            ctx->Bye();
        });
        app1.AddHandler(_POST_, "post/"_router_starts, [](Context ctx)
        {
            std::string p1 = ctx->Req().GetParameter("aaa_who");
            std::string p2 = ctx->Req().GetParameter("bbb_how");
            std::string data = "<p><b>aaa_who : </b></p><p>" + p1 +
                               "</p><p><b>bbb_how : </b></p><p>" + p2 + "</p>";
            data = "<!DOCTYPE html><html lang=\"zh-cn\"><body><h1>D2SCHOOL-POST</h1>" + data + "</body></html>";
            ctx->Res().SetContentType("text/html");
            ctx->Res().Ok(data);
            ctx->Bye();
        });
        svc->AddApp(app1);
        console->info("App {} regist!", app1.GetName());
        svc->AddApp(app2);
        console->info("App {} regist!", app2.GetName());
        svc->AddHandler(_GET_, "/admin/test/"_router_starts, [](Context ctx)
        {
            ctx->Res().Ok("<!DOCTYPE html><html lang=\"zh-cn\"><body><h1>D2SCHOOL-ADMIN/TEST</h1></body></html>");
            ctx->Bye();
        });
        svc->AddHandler(_POST_, "/"_router_starts, [](Context ctx)
        {
            ctx->Res().Ok("<!DOCTYPE html><html lang=\"zh-cn\"><body><h1>D2SCHOOL-POST</h1></body></html>");
            ctx->Bye();
        });
        svc->AddHandler(_GET_, "/favicon.ico", [](Context ctx)
        {
            ctx->Res().Nofound();
            ctx->Bye();
        });
        svc->AddHandler(_GET_, "/plain-text", [](Context ctx)
        {
            ctx->Res().SetContentType("text/plain");
            ctx->Res().AppendHeader("Content-Language", "zh-CN");
            ctx->Res().SetBody("!!!这是纯纯的纯文本!!!");
            ctx->Bye();
        });
        svc->Start();
        ioc.run();
    }
    catch (std::exception const& e)
    {
        console->error("Server Start Fail ! {}", e.what());
    }

    return 0;
}
