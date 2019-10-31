# 零、几个原则
## 0.1 自己的狗粮自己吃
官网 [第2学堂 www.d2school.com](http://www.d2school.com) 后台使用 da4qi4作为Web Server开发。（nginx + da4qi4 + redis + mysql）。 

## 0.2 站在巨人的肩膀上

使用成熟的，广泛应用（最好有大公司参与）的开源项目作为框架基础组成部件。

da4qi4 Web 框架优先使用成熟的、C/C++开源项目的搭建。其中：
- HTTP 基础协议解析： Node.JS / http-parser， 纯C语言  [nodejs/http-parser](https://github.com/nodejs/http-parser) 
- HTTP multi-part  : multipart-parsr [multipart-parser-c](https://github.com/iafonov/multipart-parser-c)
- 网络异步框架： C++ boost.asio [boostorg/asio](https://github.com/boostorg/asio) （预计进入C++标准库）
- JSON  :  [nlohmann-json JSON for Modern C++](https://github.com/nlohmann/json) (github 上搜索JSON出来的第一个)
- 日志： [splogs](https://github.com/gabime/spdlog) 一个高性能的C++日志库 （微软公司将它绑定到 Node.JS）
- 模板引擎： [inja](https://github.com/pantor/inja) 是模板引擎 [Jinja](https://palletsprojects.com/p/jinja/) 的 C++ 实现版本，和 nlohmann-json 完美配合实现C++内嵌的动态数据结构 
- Redis 客户端： 基于[nekipelov/redisclient](https://github.com/nekipelov/redisclient)，为以类node.js访问redis进行专门优化（实现单线程异步访问，去锁）。 ，da4qi4默认使用redis缓存session等信息，以优先支持负载均衡下的节点无状态横向扩展。
- TLS/加密： OpenSSL
- 静态文件服务： da4qi4自身支持静态文件（包括前端缓存支持）。实际项目部署建议与nginx配合。由nginx提供更高性能、更安全的接入及提从静态文件服务。

数据库访问方式不作绑定，建议使用 Oracle 官方 C++ Connector。

## 0.3 易用优于性能

使用C++开发，基于异步框架，目的就是为了有一个较好的原生性能起点，开发者不要过于费心性能。框架易用性设计高于性能设计。
暂时仅与 Tomcat 做了一个比较。由于Tomcat似乎是“Per Connection Per Thread”，所以这个对比非常“胜之不武”；但考虑到Tomcat曾经被广泛使用，所以和它对比的数据反倒更容易让读者知道da4qi4框架的性能基准。

 **基准测试环境：** 
- ubuntu 18.04 
- 4核心8线程 、8G内存
- 测试工具： Jmeter
- 测试工具和服务端运行于同一机器（显然会影响服务端性能，不过本次测试重点是做相对性的对比）
- 后台无业务，不访问数据库，简单返回数百字节
- 不走nginx等Web Server的反向代理

**Tomcat 运行配置**
- JVM 1.8G 内存
- 最大线程数：10000
- 最大连接数：20000
- 最大等待队列长度 200

 _对 Tomcat不算熟，因此以上配置基本照着网上的相关测试指南设置，有不合理之处，望指正。_ 

| -  | 并发数 | 平均响应（ms） | 响应时间中位数  | 99% 用户响应时间 |最小响应 |最大响应|错误率|吞吐量(s)|每秒接收字节(KB）|
|--- |  ---   | ---            | ---             | ---              | ---     | ---    | ---  | ---     | ---             |
|tomcat| 1000 |  350 | 337     |  872   |  1   |  879   |  0   | 886.7 | 273  |
|da4qi4| 1000 |  1   | 1       |  27    |  0   |  24    |  0   | 1233  | 286.6|

另，官网 www.d2school.com 一度以 1M带度、1核CPU、1G 内存的一台服务器作为运行环境（即：同时还运行MySQL、redis服务）；后因线上编译太慢，做了有限的升级。

后续会给出与其他Web Server的更多对比。但总体上，da4qi4 的当前阶段开发，基本不会以极端性能提升作为目标。


# 一、快速了解

## 1.1 一个空转的Web Server
像所有的C++程序，我们至少需要一个C++文件，假设名字还是熟悉的“main.cpp”，在本例中，它的内容如下：

```C++
#include "daqi/da4qi4.hpp"

using namespace da4qi4;

int main()
{
    auto svc = Server::Supply(4098);
    svc->Run();
}
```

不到10行代码，我们创建了一个空转的，似乎不干活的Web Server。虽然被污蔑为不干活，但其实个Web Server是在正常运行中，它之所以表现成不干活，只是因为它要忠于创建它的主人，也就是我们（程序员）：我们没有指示它如何响应，所以它对所有的请求，都只能回应一句：“Not Found”（没错，就是404）。

编译、运行，然后在浏览器地址栏输入：http://127.0.0.1:4098 ，而后回车，浏览器将显示一个页面，上面写着：

```
Not Found
```

> 小提示：代码中的“Supply(4098)”调用，如果不提供4098这个入参，那么Web Server将在HTTP默认的80端口监听。我们使用4098是考虑到在许多程序员的开发电脑上，80端口可能已经被别的应用占用了。


## 1.2 Hello World!
不管访问什么都回一句“Not Found”这令人沮丧。接下来实现这么一个功能：当访问网站的根路径时，它能答应一声：“Hello World!”。

### 1.2.1 针对指定URL的响应

这需要我们大代码中指示框架遇上访问网站根路径时，做出必要的响应。响应可以是函数指针、std::function或C++11引入的lambda，我们使用最后者：

```C++
#include "daqi/da4qi4.hpp"

using namespace da4qi4;

int main()
{
    auto svc = Server::Supply(4098);

    svc->AddHandler(_GET_, "/", [](Context ctx)
    {
        ctx->Res().ReplyOk("Hello World!");
        ctx->Pass();
    });

    svc->Run();
}
```

例中使用到的Server类的AddHandler()方法，并提供三个入参：

1. 指定的HTTP访问方法： \_GET\_;

2. 指定的访问URL： /，即根路径 ;

3. 匿名的lambda表达式。

三个入参以及方法名，表达一个意思：如果用户以GET方法访问网站的根路径，框架就调用lambda 表达式以做出响应。

编译、运行。现在用浏览器访问 http://127.0.0.1:4098 ，将看到：

```
Hello World!
```

### 1.2.2 返回HTML 
以上代码返回给浏览器纯文本内容，接下来，应该来返回HTML格式的内容。出于演示目的，我们干了一件有“恶臭”的事：直接在代码中写HTML字符串。放心，后面很快会演示正常的做法：使用静态文件，或者基于网页模板文件来定制网页的页面内容；但现在，让我们来修改第11行代码调用ReplyOK()函数的入参，原来是“Hello World!”，现在将它改成一串HTML：

```c++
……
   ctx->Res().ReplyOk("<html><body><h1>Hello World!</h1></body></html>");
……
```

## 1.3 处理请求
接下来，我们希望请求和响应的内容都能够有点变化，并且二者的变化存在一定的匹配关系。具体是：在请求的URL中，加一个参数，假设是“name=Tom”，则我们希望后台能返回“Hello Tom!”。

这就需要用到“Request/请求”和“Response/响应”：

```C++
#include "daqi/da4qi4.hpp"

using namespace da4qi4;

int main()
{
    auto svc = Server::Supply(4098);

    svc->AddHandler(_GET_, "/", [](Context ctx)
    {
        std::string name = ctx->Req("name");
        std::string html = "<html><body><h1>Hello " + name + "!</h1></body></html>";
        ctx->Res().ReplyOk(html);
        ctx->Pass();
    });

    svc->Run();
}
```

编译、运行。通过浏览器访问 “http://127.0.0.1:4098/?name=Tom” ，将得到带有HTML格式控制的 “Hello Tom!”。


> 小提示：试试看将“Tom”改为汉字，比如“张三”，通常你的浏览器会在“Hello ”后面显示成一团乱码；这不是大器框架的问题，这是我们手工写的那段html内容不够规范。

## 1.4 引入“Application”
Server代表一个Web 服务端，但同一个Web Server系统很可能可分成多个不同的人群。

比如写一个在线商城，第一类用户，也是主要的用户，当然就是来商城在线购物的买家，第二类用户则是卖家和商城的管理员。这种区别，也可以称作是：一个服务端，多个应用。在大器框架中，应用以Application表达，

就当前而言，还不到演示一个Server上挂接多个Application的复杂案例，那我们为什么要开始介绍Application呢？Application才是负责应后台行为的主要实现者。在前面的例子中，虽然没有在代码中虽然只看到Server，但背后是由Server帮我们创建一个默认的 Application 对象，然后依靠该默认对象以实现演示中的相关功能。

现在我们要做的是：显式创建一个Application对象，并代替Server对象来实前面最后一个例子的功能。

```C++
#include "daqi/da4qi4.hpp"

using namespace da4qi4;

int main()
{
    auto svc = Server::Supply(4098);
    auto web_app = Application::Customize("web", "/", "./log");

    web_app->AddHandler(_GET_, "/", [](Context ctx)
    {
        std::string name = ctx->Req("name");
        std::string html 
             = "<html><body><h1>Hello " + name + "!</h1></body></html>";
        ctx->Res().ReplyOk(html);
        ctx->Pass();
    });

    svc->Mount(web_app);
    svc->Run();
}
```

发生的变化：使用Aplication类的静态成员函数，定制（Customize）了一个应用，例中命名为web_app；然后改用它来添加前端以GET方法访问网站根URL路径时的处理方法。最后在svc运行之前，需要先将该应用挂接（Mount）上去。

这段代码和前面没有显式引入Application的代码，功能一致，输出效果也一致。但为什么我们一定要引入Application呢？除了前述的，为将来一个Server对应多个Application做准备之外，从设计及运维上讲，还有一个目的：让Server和Application各背责任。Application负责较为高层的逻辑，重点是具体的某类业务，而Server则负责服务器较基础的逻辑，重点是网络方面的功能。下一小节将要讲到日志，正好是二者分工的一个典型体现。

## 1.5 运行日志
一个Web Server在运行时，当然容易遇到或产生各种问题。这时候后台能够输出、存储运行时的各种日志是大有必要的功能。

结合前面所说的Server与Application的分工。日志在归集上就被分成两大部分：服务日志和应用日志。

- 服务层日志：记录底层网络、相关的周边运行支撑环境(缓存/Redis、数据库/MySQL)等基础设施的运行状态。

- 应用层日志：记录具体应用的运行日志。

其中，相对底层的Server日志全局唯一，由框架自动创建；而应用层日志自然是每个应用对应一套日志。程序可以为服务层和应用层日志创建不同的日志策略。事实上，如果有多个应用，那自然可以为每个应用定制不同的日志策略。

在前面例子中，服务层日志对象已经存在，只是我们没有主动配置它，也没有主动使用它记录日志。而唯一的应用web_app则采用默认的日记策略：即没有日志。没错，应用允许不记录日志。

为了使用服务层日志，我们需要在程序启动后，服务还未创建时，就初始化服务层的日志对象，这样才有机会记录服务创建过程中的日志（比如最常见的，服务端口被占用的问题）。假设初始化日志这一步都失败，记录失败信息的人，肯定不能是日志对象，只能是我们熟悉的std::cerr或std::cout对象。

```C++
#include "daqi/da4qi4.hpp"

using namespace da4qi4;

int main()
{
    if (!log::InitServerLogger("/home/zhuangyan/Projects/CPP/daqi_demo/www/logs", log::Level::debug))
    {
        std::cerr << "Create server logger fail." << std::endl;
        return -1;
    }

    auto svc = Server::Supply(4098);
    log::Server()->info("准备web应用中……");

    auto web_app = Application::Customize("web", "/", "./log");

    web_app->AddHandler(_GET_, "/", [](Context ctx)
    {
        std::string name = ctx->Req("name");
        std::string html = "<html><body><h1>Hello " + name + "!</h1></body></html>";
        ctx->Res().ReplyOk(html);
        ctx->Pass();
    });

    svc->Mount(web_app);
    svc->Run();
}
```

> 小提示：为方便演示，上面代码暂时使用绝对路径以指定服务层日志文件的存储位置，实际项目通常以相对路径，或读取外部配置的方式以方便部署。

一旦“InitServerLogger()”调用成功，并且设置低于INFO的日志输出级别（例中为DEBUG级别，见该函数的第2个入参：log::Level::debug），框架中有关服务层的许多日志，就会打印到屏幕（控制台）上及相应的日志文件里。

## 1.6 HTML 模板
是时候解决在代码中直接写HTML的问题了。

用户最终看到的网页的内容，有一些在系统设计阶段就很清楚，有一些则必须等用户访问时才知道。比如前面的例子中，在设计时就清楚的有：页面字体格式，以及“Hello, _ _ _ _ !”；而需要在运行时用户访问后才能知道的，就是当中的下划线处所要填写的内容。

下面是适用于本例的，一个相当简单的的HTMl网页模板：

```html
<!DOCTYPE html>
<html lang="zh">
<head>
    <title>首页</title>
    <meta content="text/html; charset=UTF-8">
</head>
<body>
    <h1>你好，{=name=}！</h1>
</body>
</html>
```

“你好，”后面的特定格式{=name=}，将会被程序的模板解析引擎识别，并填写上运行时的提供的name的值。使用模板后，现在用于产生的网页内容的完整C++代码如下：

```C++
#include "daqi/da4qi4.hpp"

using namespace da4qi4;

int main()
{
    //网站根目录 （同样，我们暂使用绝对路径）
    std::string www_root = "/home/zhuangyan/Projects/CPP/daqi_demo/www/";

    //初始化服务层日志
    if (!log::InitServerLogger(www_root + "logs", log::Level::debug))
    {
        std::cerr << "Create server logger fail." << std::endl;
        return -1;
    }

    auto svc = Server::Supply(4098);
    log::Server()->info("准备web应用中……"); //纯粹是为了演示日志……

    auto web_app = Application::Customize("web", "/",
                                          www_root + "logs/",
                                          www_root + "static/",
                                          www_root + "view/",
                                          www_root + "upload/");

    //初始化web应用日志
    if (!web_app->Init(Application::ActualLogger::yes, log::Level::debug))
    {
        log::Server()->critical("Init application {} fail.", web_app->GetName()); 
        return -2;
    }

    //添加请求处理
    web_app->AddHandler(_GET_, "/", [](Context ctx)
    {
        std::string name = ctx->Req().GetUrlParameter("name");
        ctx->ModelData()["name"] = name;
        ctx->Render().Pass();
    });

    svc->Mount(web_app);
    svc->Run();
}
```

由于我们所写的模板文件正确地指定了相关编码，所以现在如果访问 http://127.0.0.1:4098?name=大器da4qi4 。将得到带有HTML格式的：

```
你好，大器da4qi4！
```

框架提供的模板引擎，不仅能替换数据，也支持基本的条件判断、循环、自定函数等功能，类似一门“脚本”。

> 小提示：大多数情况下，我们写的C++程序用以高性能地、从各种来源（数据库、缓存、文件、网络等）、以各种花样（同步、异步）获取数据、处理数据。而HTML模板引擎在C++程序中以解释的方式运行，因此正常的做法是不要让一个模板引擎干太复杂的，毕竟，在C++这种“彪形大汉”的语言面，它就是个小孩子。

## 1.7 更多
前面未提及的，但框架本身集成功能还包括：

1. cookie支持

2. 前端（浏览器）缓存支持

3. Redis 缓存支持

4. Session 支持

5. 静态文件

6. 模板文件更新检测

7. HTTP 客户端组件

8. POST响应支持

9. 文件上传、下载

10. 访问限流

11. HTTPS

12. JSON

13. 纯数据输出的API接口，与前端AJAX配合

14. 框架全方式集成：(a) 基于源代码集成、(b) 基于动态库集成、(c) 基于静态库集成

15. ……

而框架外围当前可供集成或实现的功能有：

1. 数据库访问

2. 和nginx配合（实现负载均衡的快速横向扩展）

3. 阿里短信云异步客户端

4. 微信扫一扫登录异步客户端

5. 基于OpenSSL的数据加密工具

6. 常用字符串处理

7. 常用编码转换（UTF-8、UCS、GBK、GB18030）

8. ……

