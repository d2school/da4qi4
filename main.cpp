#include <ctime>

#include <iostream>

#include "server.hpp"

#include "application.hpp"
#include "intercepter_staticfile.hpp"

#include "def/log_def.hpp"
#include "utilities/captcha_utilities.hpp"

using namespace da4qi4;

int main()
{
    auto console = spdlog::stdout_color_mt("console");
    console->info("Wecome to da4qi4");

    auto svc = Server::Supply(4099);
    console->info("Server start at {}", 4099);

    Application app1("d2school" //name
                     , "/"      //url root
                     , "/home/zhuangyan/projects/CPP/test_web_root/static/" //static root
                     , "/home/zhuangyan/projects/CPP/test_web_root/view/"   //template root
                     , "/home/zhuangyan/projects/CPP/test_web_root/upload/" //upload root
                    );

    Intercepter::StaticFileIntercepter sfi(60);
    sfi.AddEntry("html/", "").AddDefaultFileNames({"index.html", "index.htm"});
    app1.PushBackIntercepter(sfi);

    app1.AddHandler(_GET_, "", [](Context ctx)
    {
        using namespace nlohmann;
        json data;

        data["names"] = {"南老师", "林校长", "张校花"};

        std::clock_t beg = std::clock();
        data["time"]["start"] = beg;

        std::clock_t end = std::clock();
        data["time"]["end"] = end;
        data["time"]["long"] = std::to_string((end - beg) * 1000 / CLOCKS_PER_SEC) + " 毫秒";

        ctx->Render(data);
        ctx->Bye();
    });

    app1.AddHandler(_GET_, "/chunked", [](Context ctx)
    {
        ctx->Res().SetContentType("text/plain");
        ctx->StartChunkedResponse();
        ctx->ContinueChunkedResponse("我是一个兵\r\n");
        ctx->ContinueChunkedResponse("来自老百姓！\r\n");
        ctx->Bye();
    });

    app1.AddHandler(_GET_, "usr/{{name}}/{{age}}/regist"_router_regex, [](Context ctx)
    {
        ctx->RenderWithoutData("user/regist");
        ctx->Bye();
    });
    app1.AddHandler(_GET_, "find"_router_starts, [](Context ctx)
    {
        ctx->RenderWithoutData();
        ctx->Bye();
    });
    app1.AddHandler(_GET_, "cookie/", [](Context ctx)
    {
        ctx->Res().SetCookie("is_new", "YE\"S!");
        ctx->Res().SetCookie("do_you_love_me", "NO", 15, Cookie::HttpOnly::for_http_only);
        ctx->Res().ReplyOk("<!DOCTYPE html><html lang=\"zh-cn\"><body><h2>D2SCHOOL/COOKIE</h2></body></html>");
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
        ctx->Res().ReplyOk(html);
        ctx->Bye();
    });
    app1.AddHandler(_GET_, "cookie/delete"_router_starts, [](Context ctx)
    {
        ctx->Res().SetCookieExpiredImmediately("is_new");
        ctx->Res().SetCookieExpiredImmediately("do_you_love_me");
        ctx->Res().ReplyOk("<!DOCTYPE html><html lang=\"zh-cn\"><body><h2>D2SCHOOL/COOKIE/DELETE</h2></body></html>");
        ctx->Bye();
    });

    app1.AddHandler(_GET_, "form/get", [](Context ctx)
    {
        ctx->Res().CacheControlMaxAge(600);
        ctx->RenderWithoutData();
        ctx->Bye();
    });
    app1.AddHandler(_GET_, "form/post", [](Context ctx)
    {
        ctx->Res().CacheControlMaxAge(600);
        ctx->RenderWithoutData();
        ctx->Bye();
    });
    app1.AddHandler({_GET_, _POST_}, "form/result", [](Context ctx)
    {
        ctx->RenderWithoutData();
        ctx->Bye();
    });

    app1.AddHandler(_GET_, "post/upload", [](Context ctx)
    {
        ctx->RenderWithoutData();
        ctx->Bye();
    });
    app1.AddHandler(_POST_, "post/upload_result", [](Context ctx)
    {
        ctx->RenderWithoutData();
        ctx->Bye();
    });

    svc->AddApp(app1);
    console->info("App {} regist!", app1.GetName());

    svc->AddHandler(_GET_, "/favicon.ico", [](Context ctx)
    {
        ctx->Res().ReplyNofound();
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
        svc->Run();
    }
    catch (std::exception const& e)
    {
        console->error("Server Run Fail ! {}", e.what());
    }

    return 0;
}
