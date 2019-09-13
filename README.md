```cpp
int main(int argc, char* argv[])
{
    int concurrent_count = std::thread::hardware_concurrency(); //物理线程数

    auto svc = Server::Supply(80, concurrent_count); //在80端口

    std::string app_root = "./www";   //daqi网站的根目录
    auto web = Application::Customize("web"   //支持多个应用，这个是应用名称
                                      , "/"    //www root
                                      , app_root + "logs/"   //网站运行日志目录   
                                      , app_root + "static/"   //静态文件，实际网站交给 nginx 托管
                                      , app_root + "view/"     //网页模板文件 MVC中的 V
                                      , app_root + "upload/"   //上传文件时的临时目录
                                     );

    if (!web->Init(Application::ActualLogger::yes))  //要不要开启日志
    {
        std::cerr << "Init application " << web->GetName() << " fail." << std::endl;
        return -2;
    }

    //静态文件
    //如果不想让nginx或apache托管静态文件（实际项目基本需要），则自己管理：
    //下面演示凡是访问  css, js, img/目录下，以及 /favicon.ico，都直接走静态文件
    //静态文件并不会每次走到服务器读磁盘文件，实际会让浏览器做缓存（SetCacheMaxAge）,
    //当然，还是强烈推荐3分钟安装配置下nginx的好，因为处理静态文件，我们强不过nginx……
    Intercepter::StaticFile static_file;
    static_file.SetCacheMaxAge(600).AddEntry("css/").AddEntry("js/")
               .AddEntry("img/").AddEntry("favicon.ico", "img/favicon.ico");
    web->AddIntercepter(static_file);
   
    //配置session数据使用 redis。因此，哪天网站压力大了点，在 nginx之后一拖N，多部署几个这个程序
    //就可以简单负载分摊一下了。
    Intercepter::SessionOnRedis session_redis;
    web->AddIntercepter(session_redis);

    /* 以上配置了很多，不过我们这个例子只要一个 hello world, */
    web->AddHandler(_GET_, {"/", "/index"}, [](Context ctx)
    {
        std::string name = ctx->Req("name");
        if (name.empty()) name = "小明";

        ctx->Res().ReplyOk("<html><body>Hello " + name + "</body></html>")
        ctx->Pass();
    });

    svc->Mount(web); //一个"server" 可以 挂接 多个 "Application" 

    //因为session使用redis,所以需要初始化下redis池，da4qi4集成了C++ rds 的异步访问功能
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
}
```
