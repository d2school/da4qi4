#include <ctime>

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

    boost::asio::io_context ioc;
    auto svc = Server::Supply(ioc, 4099);
    console->info("Server start at {}", 4099);
    Application app1("d2school", "/", "", "/home/zhuangyan/projects/CPP/build-da4qi4-Debug/view/");

    app1.AddHandler(_GET_, ""_router_starts, [](Context ctx)
    {
        using namespace nlohmann;
        json data;

        data["names"] = {"南老师", "林校长", "张校花"};

        std::clock_t beg = std::clock();
        data["time"]["start"] = beg;

        std::clock_t end = std::clock();
        data["time"]["end"] = end;
        data["time"]["long"] = std::to_string((end - beg) * 1000 / CLOCKS_PER_SEC) + " 毫秒";

        ctx->Render("index", data);
        ctx->Bye();
    });
    app1.AddHandler(_GET_, "test/"_router_starts, [](Context ctx)
    {
        ctx->Res().Ok("<!DOCTYPE html><html lang=\"zh-cn\"><body><h2>D2SCHOOL/TEST</h2></body></html>");
        ctx->Bye();
    });
    app1.AddHandler(_GET_, "cookie/", [](Context ctx)
    {
        ctx->Res().SetCookie("is_new", "YE\"S!");
        ctx->Res().SetCookie("do_you_love_me", "NO", 15, Cookie::HttpOnly::for_http_only);
        ctx->Res().Ok("<!DOCTYPE html><html lang=\"zh-cn\"><body><h2>D2SCHOOL/COOKIE</h2></body></html>");
        ctx->Bye();
    });
    app1.AddHandler(_GET_, "cookie/view", [](Context ctx)
    {
        std::string html = "<!DOCTYPE html><html lang=\"zh-cn\"><body><h2>D2SCHOOL/COOKIE/VIEW</h2>";

        for (auto c : ctx->Req().GetCookies())
        {
            html += "<h3>" + c.first + " = " + c.second + "</h3>";
        }

        html += "</body></html>";
        ctx->Res().Ok(html);
        ctx->Bye();
    });
    app1.AddHandler(_GET_, "cookie/global"_router_starts, [](Context ctx)
    {
        ctx->Res().SetCookie("is_global", "YES");
        ctx->Res().Ok("<!DOCTYPE html><html lang=\"zh-cn\"><body><h2>D2SCHOOL/COOKIE/GLOBAL</h2></body></html>");
        ctx->Bye();
    });
    app1.AddHandler(_GET_, "cookie/delete"_router_starts, [](Context ctx)
    {
        ctx->Res().SetCookieExpiredImmediately("is_new");
        ctx->Res().SetCookieExpiredImmediately("do_you_love_me");
        ctx->Res().Ok("<!DOCTYPE html><html lang=\"zh-cn\"><body><h2>D2SCHOOL/COOKIE/DELETE</h2></body></html>");
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
        std::string p1 = ctx->Req("aaa_who");
        std::string p2 = ctx->Req()["bbb_how"];
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

    try
    {
        svc->Start();

        for (;;)
        {
            try
            {
                ioc.run();
            }
            catch (std::exception const& e)
            {
                console->error("Server Fail ! {}", e.what());
            }
        }
    }
    catch (std::exception const& e)
    {
        console->error("Server Start Fail ! {}", e.what());
    }

    return 0;
}
