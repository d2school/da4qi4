- [零、几个原则](#零几个原则)
  * [0.1 自己的狗粮自己吃](#01-自己的狗粮自己吃)
  * [0.2 站在巨人的肩膀上](#02-站在巨人的肩膀上)
  * [0.3 易用优于性能](#03-易用优于性能)
  * [0.4 简单胜过炫技](#04-简单胜过炫技)
  * [0.5 紧跟国内生产环境](#05-紧跟国内生产环境)
- [一、快速了解](#一快速了解)
  * [1.1 一个空转的Web Server](#11-一个空转的web-server)
  * [1.2 Hello World!](#12-hello-world)
    + [1.2.1 针对指定URL的响应](#121-针对指定url的响应)
    + [1.2.2 返回HTML](#122-返回html)
  * [1.3 处理请求](#13-处理请求)
  * [1.4 引入Application](#14-引入application)
  * [1.5 运行日志](#15-运行日志)
  * [1.6 HTML模板](#16-html模板)
  * [1.7 WebSocket](#17-websocket)
    * [1.7.1 HTTP对比WebSocket](#171-http-对比-websocket)
    * [1.7.2 大器WebSocket后台实现特性](#172-大器WebSocket后台实现特性)
    * [1.7.3 使用示例](#173-使用示例)
  * [1.8 更多](#18-更多)
    + [1.8.1 框架更多集成功能](#181-框架更多集成功能)
    + [1.8.2 框架外围可供集成的工具](#182-框架外围可供集成的工具)
- [二、如何构建](#二如何构建)
  * [2.1 基于生产环境构建](#21-基于生产环境构建)
  * [2.2 准备编译工具](#22-准备编译工具)
  * [2.3 准备第三方库](#23-准备第三方库)
  * [2.4 下载大器源代码](#24-下载大器源代码)
  * [2.5 编译“大器”库](#25-编译大器库)
  * [2.6 在你的项目中使用 da4qi4库](#26-在你的项目中使用da4qi4)
- [三、运行时外部配套系统](#三运行时外部配套系统)
  * [3.1 运行时依赖说明](#31-运行时依赖说明)
  * [3.2 Redis的安装](#32-redis的安装)
  * [3.3 数据库](#33-数据库)

# 零、几个原则

## 0.1 自己的狗粮自己吃

官网 [第2学堂 www.d2school.com](http://www.d2school.com) 后台使用 da4qi4作为Web Server开发。（nginx + da4qi4 + redis + mysql）。 
给一个在手机上运行的网站效果：

![第2学堂手机版](https://images.gitee.com/uploads/images/2019/1114/123659_60286a85_1463463.png "手机屏幕截图.png")

样式丑，但这只和差劲的UI师，也就是我的美感有关，和后台使用什么Web框架没有关系。

## 0.2 站在巨人的肩膀上

da4qi4 Web 框架优先使用成熟的、C/C++开源项目的搭建。它的关键组成：

- HTTP 基础协议解析：[Node.JS/llhttp](https://github.com/nodejs/llhttp)。 一直使用Node.JS底层的HTTP解析器，Node.JS v12 之前是[nodejs/http-parser](https://github.com/nodejs/http-parser)；之后升级迁移到 [llhttp](https://github.com/nodejs/llhttp) 。Node.JS 官方说法解析性能提升156%；
- HTTP multi-part  : multipart-parsr [multipart-parser-c](https://github.com/iafonov/multipart-parser-c) ；
- 网络异步框架： C++ boost.asio [boostorg/asio](https://github.com/boostorg/asio) （可能进入C++2x标准库）
- JSON  :  [nlohmann-json JSON for Modern C++](https://github.com/nlohmann/json) (github上搜索JSON结果中第一个)；
- 日志： [splogs](https://github.com/gabime/spdlog) 高性能的C++日志库 (微软公司选择将它绑定到 Node.JS 作日志库)；
- 模板引擎： [inja](https://github.com/pantor/inja) 模板引擎 [Jinja](https://palletsprojects.com/p/jinja/) 的 C++ 实现版本，名气不大，但能和nlohmann-json完美配合实现C++内嵌的动态数据结构，加上我为它解决过bug，比较熟悉、放心。
- TLS/SSL/数据加密： [OpenSSL](https://www.openssl.org/) （TLS）；
- Redis 客户端： 基于[nekipelov/redisclient](https://github.com/nekipelov/redisclient)，为以类node.js访问redis进行专门优化（实现单线程异步访问，去锁）。 da4qi4默认使用redis缓存session等信息(以优先支持负载均衡下的节点无状态横向扩展)。
- 静态文件服务： da4qi4自身支持静态文件（包括前端缓存逻辑）。实际项目部署建议与nginx配合。由nginx提供更高性能、更安全的接入及提从静态文件服务。

注：

1. 框架未绑定数据库访问方式。用户可使用 Oracle 官方 C++ Connector，或MySQL++，或 [三、运行时外部配套系统](#三运行时外部配套系统)提及的各类数据库连接器；
2. 框架自身使用 redis 作为默认的（可跨进程的）SESSION支持。上层应用可选用框架的redis接口，也可以使用自己喜欢、 顺手 的redis C++客户端。

## 0.3 易用优于性能

使用C++开发，基于异步框架，目的就是为了有一个较好的原生性能起点，开发者不要过于费心性能。当然，性能也不能差，因为性能差必将影响产品的易用性）。
暂时仅与 Tomcat 做了一个比较。由于Tomcat似乎是“Per Connection Per Thread/每连接每线程”，所以这个对比会有些胜之不武；但考虑到Tomcat曾广泛应用于实际系统，所以和它的对比数据有利于表明da4qi4在性能上的可用性。

 **基准测试环境：** 

- ubuntu 18.04 
- 4核心8线程 、8G内存
- 测试工具： Jmeter
- 测试工具和服务端运行于同一机器（显然会影响服务端性能，不过本次测试重点是做相对性的对比）
- 后台无业务，不访问数据库，仅返回简短字符串（造成吞吐量严重不足）
- 不走nginx等Web Server的反向代理

**Tomcat 运行配置**

- JVM 1.8G 内存

- 最大线程数：10000

- 最大连接数：20000

- 最大等待队列长度 200
  
  _对 Tomcat不算熟，因此以上配置基本照着网上的相关测试指南设置，有不合理之处，望指正。_ 

| -      | 并发数  | 平均响应（ms） | 响应时间中位数（ms） | 99% 用户响应时间（ms） | 最小响应（ms） | 最大响应（ms） | 错误率 | 吞吐量(s) | 每秒接收字节(KB） |
| ------ | ---- | -------- | ----------- | -------------- | -------- | -------- | --- | ------ | ---------- |
| tomcat | 1000 | 350      | 337         | 872            | 1        | 879      | 0   | 886.7  | 273        |
| da4qi4 | 1000 | 1        | 1           | 20             | 0        | 24       | 0   | 1233   | 286.6      |

另，官网 www.d2school.com 一度以 1M带度、1核CPU、1G 内存的一台服务器作为运行环境（即：同时还运行MySQL、redis服务）；后因线上编译太慢，做了有限的升级。

后续会给出与其他Web Server的更多对比。但总体上，da4qi4 的当前阶段开发，基本不会以极端性能提升作为目标。

## 0.4 简单胜过炫技

众所周知C++语言很难，非常适于C++程序员“炫技”；所以有一票C++开源项目虽然技术上很优秀，但却很容易吓跑普通的C++程序员。比如，超爱用“模板元”……da4qi4 的代码强烈克制了这种“炫技”冲动，尽量代码看上去毫无技巧，特别是对外接口，遵循KISS原则，不会让你产生任何“惊奇”（头回看到程序员把无技可炫写得这么清新脱俗？）。

不管怎样，在C++所大范围支持的“面向过程”、“基于对象”、“面向对象”和“泛型”等编程模式中，你只需熟悉“面向过程”，并且会一点“基于对象”，就可以放心地用这个库。

## 0.5 紧跟国内生产环境

用哪个版本的C++？用哪个版本的boost库？用哪个版本的OpenSSL？用哪个版本的CMake？

就一个标准：当前国内主要云计算提供商，已经提供哪些现成的版本，我们就用那个版本——这意味着你几乎只需编译好你写的代码可以完成在线构建、部署了。不用编译boost、不用编译OpenSSL、不用下载编译新版的CMake……

[阿里云](https://cn.aliyun.com/)、[腾讯云](https://cloud.tencent.com/)、[百度云](https://cloud.baidu.com/)、[华为云](https://www.huaweicloud.com/)、[七牛云](https://www.qiniu.com/)……无论哪家，只要你在上面申请一台Ubuntu 18.04 （或更高版本）的服务器，简单向行指令就能在线编译、部署好，让它成为一台跑着“大器 INSIDE”的WEB 服务器，为你的用户提供网站服务。对服务器配置的最低要求是：4G内存、1M带宽、1核CPU。

# 一、快速了解

## 1.1 一个空转的Web Server

我们需要一个C++文件，假设名为“main.cpp”，内容如下：

```C++
#include "daqi/da4qi4.hpp"

using namespace da4qi4;

int main()
{
    auto svc = Server::Supply(4098);
    svc->Run();
}
```

不到10行代码，我们创建了一个空转的，似乎不干活的Web Server。

编译、运行，然后在浏览器地址栏输入：http://127.0.0.1:4098 ，而后回车，浏览器将显示一个页面，上面写着：

```
Not Found
```

虽然说它“不干活”，但这个Web Server的运行完全合乎逻辑：我们没有为它配备任何资源或响应操作、，所以对它的任何访问，都返回404页面：“Not Found”。

## 1.2 Hello World!

接下来实现这么一个功能：当访问网站的根路径时，它能响应：“Hello World!”。

### 1.2.1 针对指定URL的响应

这需要我们大代码中指示框架遇上访问网站根路径时，做出必要的响应。响应可以是函数指针、std::function、类方法或C++11引入的lambda，我们先来使用最后者：

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

三个入参以及方法名合起来表达：如果用户以GET方法访问网站的根路径，框架就调用lambda表达式以做出响应。

编译、运行。现在用浏览器访问 http://127.0.0.1:4098 ，将看到：

```
Hello World!
```

作为对比，下面给出同样功能使用自由函数的实现：

```C++
#include "daqi/da4qi4.hpp"

using namespace da4qi4;

void hello(Context ctx)
{
    ctx->Res().ReplyOk("Hello World!");
    ctx->Pass();
}

int main()
{
    auto svc = Server::Supply(4098);

    svc->AddHandler(_GET_, "/",  hello); 
    svc->Run();
}
```

为节省代码篇幅，后续演示均使用lambda表达式来表达HTTP的响应操作。实际系统显然不可能将代码全塞在main()函数中，因此平实的自由函数会用得更多。不仅lambda不是必需，实际是连“class/类”都很少使用——这符号Web Server基本的要求：尽量不要带状态；自由函数相比类的成员函数（或称方法），更“天然的”不带状态。

### 1.2.2 返回HTML

以上代码返回给浏览器纯文本内容，接下来，应该来返回HTML格式的内容。出于演示目的，我们干了一件有“恶臭”的事：直接在代码中写HTML字符串。后面很快会演示正常的做法：使用静态文件，或者基于网页模板文件来定制网页的页面内容；但现在，让我们来修改第11行代码调用ReplyOK()函数的入参，原来是“Hello World!”，现在将它改成一串HTML：

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

> 重要： 这里为方便演示而使用 lambda 表达式，但实际系统不可能把所有代码都放在main()函数中写。所以肯定是一个个函数。用编程语言中最最基础的函数并不丢人，因为，我们要的是实用，而不是非在代码秀一下“我会lambda哦！”。（参看：[0.4 简单胜过炫技](#04-简单胜过炫技)）

编译、运行。通过浏览器访问 “http://127.0.0.1:4098/?name=Tom” ，将得到带有HTML格式控制的 “Hello Tom!”。

## 1.4 引入Application

编译、运行。通过浏览器访问 “http://127.0.0.1:4098/?name=Tom” ，将得到带有HTML格式控制的 “Hello Tom!”。

Server代表一个Web 服务端，但同一个Web Server系统很可能可分成多个不同的人群。

> 举例：比如写一个在线商城，第一类用户，也是主要的用户，当然就是来商城在线购物的买家，第二类用户则是卖家和商城的管理员。这种区别，也可以称作是：一个服务端，多个应用。在大器框架中，应用以Application表达。

就当前而言，还不到演示一个Server上挂接多个Application的复杂案例，那我们为什么要开始介绍Application呢？Application才是负责应后台行为的主要实现者。在前面的例子中，虽然没有在代码中虽然只看到Server，但背后是由Server帮我们创建一个默认的 Application 对象，然后依靠该默认对象以实现演示中的相关功能。

下面我们就通过“Server | 服务”对象，取出这个“Application | 应用”，并代替前者实现前面最后一个例子的功能。

```C++
#include "daqi/da4qi4.hpp"

using namespace da4qi4;

int main()
{
    auto svc = Server::Supply(4098);

    auto app = svc->DefaultApp(); //取出自动生成的默认应用对象
    app->AddHandler(_GET_, "/", [](Context ctx)
    {
        std::string name = ctx->Req("name");
        std::string html = "<html><body><h1>Hello " + name + "!</h1></body></html>";
        ctx->Res().ReplyOk(html);
        ctx->Pass();
    });

    svc->Run();
}
```

除了“AddHandler()”的实施对象以前是svc，现在是“app”以外，基本没有什么变化。代码和前面没有显式引入Application之前功能一致。但为什么我们一定要引入Application呢？除了前述的，为将来一个Server对应多个Application做准备之外，从设计及运维上讲，还有一个目的：让Server和Application各背责任。 **Application负责较为高层的逻辑，重点是具体的某类业务，而Server则负责服务器较基础的逻辑，重点是网络方面的功能** 。下一小节将要讲到日志，正好是二者分工的一个典型体现。

## 1.5 运行日志

一个Web Server在运行时，当然容易遇到或产生各种问题。这时候后台能够输出、存储运行时的各种日志是大有必要的功能。并且，最最重要的是，如果你写一个服务端程序，运行大半年没有什么屏幕输出，看起来实在是“不够专业”，很有可能会影响你的工资收入……

结合前面所说的Server与Application的分工。日志在归集上就被分成两大部分：服务日志和应用日志。

- 服务层日志：全局唯一，记录底层网络、相关的周边运行支撑环境(缓存/Redis、数据库/MySQL)等基础设施的运行状态。

- 应用层日志：每个应用都对应一个日志记录器，记录该应用的运行日志。

其中，相对底层的Server日志由框架自动创建；而应用层日志自然是每个应用对应一套日志。程序可以为服务层和应用层日志创建不同的日志策略。事实上，如果有多个应用，那自然可以为每个应用定制不同的日志策略。如果不主动为某个应用创建日志记录器，则该应用只管全速运行，不输出任何日志——听起很酷，但你不应该对自己写的代码这么有信心。

```C++
#include "daqi/da4qi4.hpp"

using namespace da4qi4;

int main()
{
    //初始化服务日志，需指定日志文件要存在哪里？以及日志记录的最低级别
    log::InitServerLogger("你希望/服务日志文件/要存储的/目录/" 
                        , log::Level::debug));        

    auto svc = Server::Supply(4098);
    log::Server()->info("服务已成功加载.");          //强行输出一条服务日志

    auto app = svc->DefaultApp();

    //再来初始化应用日志
    app->InitLogger("你希望/当前应用的/要存储的/目录/");

    app->AddHandler(_GET_, "/", [](Context ctx)
    {
        std::string name = ctx->Req("name");
        std::string html = "<html><body><h1>Hello " + name + "!</h1></body></html>";
        ctx->Res().ReplyOk(html);
        ctx->Pass();
    });

    svc->Run();
    log::Server()->info("再见！");                  //强行再输出一条服务日志
}
```

所有日志功能都在“log::”名字空间之下。以上日志配置不仅会将信息输出到终端（控制台），也会自动输出指定目录下的文件中，服务日志和各应用日志是独立的文件。文件带有默认的最大尺寸和最大个数限制。实际在linux服务器上运行时，程序通常在后台运行并将本次运行的屏幕输出重定向到某个文件。

日志的输出控制，支持常见的：跟踪(trace)、调试(debug)、信息(info)、警告(warn)、错误(err)、致命错误(critical)等级别。例中对“app->InitLogger()” 使用默认级别：info。

下面是运行日志截图示例。

![输入图片说明](https://images.gitee.com/uploads/images/2019/1114/123346_2e261b35_1463463.jpeg "da4qi4运行日志界面")

看起来有点像个后台程序，可以申请领导过来视察你的工作成果了。

## 1.6 HTML模板

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
    <h1>你好，{=_URL_PARAMETER_("name")=} ！</h1>
    <p>您正在使用的浏览器： {=_HEADER_("User-Agent")=}</p>
    <p>您正在通过该网址访问本站：{=_HEADER_("Host")=}</p>
</body>
</html>
```

“你好，”后面的特定格式{=_URL_PARAMETER_("name")=} ，将会被程序的模板解析引擎识别，并填写上运行时的提供的name的值。

解释：

- \_URL_PARAMETER\_() 是网页模板脚本内置提供的一个函数，它将自动得取浏览器地址栏输入URL后，所带的参数。
- \_HEADER\_() 同样是网页模板脚本内置的一个函数，用以获得当前HTTP请求的报头数据项。在本例中无实际业务作用，常用于辅助页面调试。

假设这个文件被存放在 “你的/网页模板/目录”。下面代码中的 “app->SetTemplateRoot()”将用到这个目录的路径。

```C++
#include "daqi/da4qi4.hpp"

using namespace da4qi4;

int main()
{
    log::InitServerLogger("你希望/服务日志文件/要存储的/目录/", log::Level::debug));

    auto svc = Server::Supply(4098);
    log::Server()->info("服务已成功加载.");

    auto app = svc->DefaultApp();

    //新增的两行：
    app->SetTemplateRoot("你的/网页模板/目录/"); //模板文件根目录
    app->InitTemplates(); //加载并将模板文件“编译成” 字节码

    app->InitLogger("你希望/当前应用的/要存储的/目录/");

    //下面这行让sever定时检测模板文件的变动（包括新增）
    svc->EnableDetectTemplates(5); //5秒，实际项目请设置较大间隔，如10分钟

    svc->Run();
    log::Server()->info("再见！");
}
```

现在，使用火狐浏览器访问URL并带上“name”参数：http://127.0.0.1:4098?name=大器da4qi4 ，将得到以下HTML内容：

```html
<h1>你好，大器da4qi4 ！</h1>
<p>您正在使用的浏览器：Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:71.0) Gecko/20100101 Firefox/71.0 </p>
<p>您正在通过该网址访问本站：127.0.0.1:4098</p>
```

小提示：“为什么代码更短了？” 你应该注意到，基于模板响应后，代码原有“AddHandler()” 都不见了。因为这个例子没有实质业务逻辑：用户访问一个URL地址，并且带参数，服务依据事先定义的模板样式，将这个参数原样展现出来。实际业务系统当然不可能这么简单（否则要我们后端程序员干什么？），但是，当我们在快速搭建一个系统时，在初始开发过程中，这种情况非常常见，不需要修改源代码，不需要重启服务程序，就能直接看到新增或修改的网页内容，带给我们很大的方便。

框架提供的模板引擎，不仅能替换数据，也支持基本的条件判断、循环、自定函数等功能，类似一门“脚本”。

> 重要：多数情况下我们写C++程序用以高性能地、从各种来源（数据库、缓存、文件、网络等）、以各种花样（同步、异步）获取数据、处理数据。而HTML模板引擎在C++程序中以解释的方式运行，因此正常的做法是不要让一个模板引擎干太复杂的，毕竟，在C++这种 “彪形大汉”的语言面，页面模板引擎“语言”无论在功能还是性能上，都只能算是一个小孩子。

接下来，我们应该有一个带业务逻辑的例子。这个业务逻辑非常的复杂，并且严重依赖于CPU的计算速度……我们要做一个加法器。用户在浏览器地址栏输入:

```html
http://127.0.0.1:4098/add?a=1&b=2
```

浏览器将显示a+b的结果。显然，业务逻辑就是计算两个整数相加，我们的强大的，计算力过剩的C++语言终于可以派上用场。

首先，准备一个用于显示加法结果的页面模板，文件名为 “add.daqi.HTML”：

```html
<!DOCTYPE html>
<html lang="zh">
<head>
    <title>加法</title>
    <meta content="text/html; charset=UTF-8">
</head>
<body>
    <p>
       {=c=} 
    </p>
</body>
</html>
```

重点在 “{=c=}”身上。 {==} 仍然用来标识一个可变内容，但其内不再是一个内置函数，而是一个普通的变量名称：c。为此我们在C++代码中要做的事变成两件：一是计算 a 加 b的和，二是将和以 c 为名字，填入模板对应位置。

然后需要一个add的自由函数：

```C++
void add(Context ctx)
{
    //第一步：取得用户输入的参数 a 和 b:
    std::string a = ctx->Req().GetUrlParameter("a");
    std::string b = ctx->Req().GetUrlParameter("b");

    //第二步：把字符串转换为整数:
    int na = std::stoi(a);  //stoi 是 C++11新标中的字符串转换整数的函数
    int nb = std::stoi(b);

    //第三步：核心核心核心业务逻辑：加法计算
    int c = na + nb;

    //第四步：把结果按模板指定的名字"c"，设置到“Model”数据中：
    ctx->ModelData()["c"] = c;

    //最后一步：渲染，并把最终页面数据传回浏览器： （即：输出结果 = 模板 + 数据）
    ctx->Render().Pass();  //Render 是动词：渲染
}
```

暂时为了简化，我们不写日志、不作错误处理，现在，除了add函数的内部实现外，完整的main.cpp文件内容是：

```C++
#include "daqi/da4qi4.hpp"

using namespace da4qi4;

void add(Context ctx)
{ 
      /* 实现见上 */
}

int main()
{
    auto svc = Server::Supply("127.0.0.1", 4098);
    auto app = svc->DefaultApp(); 

    app->SetTemplateRoot("你的/网页模板/目录/"); 
    app->InitTemplates();

    //AddHandler 又回来了：
    app->AddHandler(_GET_, "/add", add);

    svc->EnableDetectTemplates(5);
    svc->Run();
}
```

如前所述通过浏览器访问  .../add?a=1&b=2 ，将看一个简单的3。

甲方说这也太不人性化了，好歹显示一个 “1 + 2 = 3” 啊！ 太好了，我们正好借此演示如何不修改代码，不重启服务程序就达成目标。

需要修改的是模板文件：“add.daqi.HTML”：

```html
<!DOCTYPE html>
<html lang="zh">
<head>
    <title>加法</title>
    <meta content="text/html; charset=UTF-8">
</head>
<body>
    <p>
        <!-- 展示内容类似：1 + 2 = 3  -->
        {=_URL_PARAMETER_("a")=}  +  {=_URL_PARAMETER_("b")=} = {=c=} 
    </p>
</body>
</html>
```

修改、保存，5秒过后再访问，就看到新成果了。

会有人担心C++写的程序容易出错，并且一出错就直接挂掉——上面程序，如果用户无意有意或干脆就是恶意搞破坏，输入 “.../add?a=A&b=BBBB”……会怎样呢？ add 函数中的 “std::stoi()” 调用可能抛出异常？不管怎样，请放心，程序并不会挂掉，它会继续运行，只是：

- 一来、用户只会看到一个典型的HTTP 500 错误 “Internal Server Error  ”  (即：服务内部错误),这对用户来说，不太友好。

- 二来，后台什么日志记录也没有，对系统的维护人员来说，也不友好。

很简单，对add的业务逻辑加上异常处理，出现异常时，向客户回复一句相对友好点的内容，并且留下应用日志即可。以下是关注异常后的add函数：

```C++
#include "daqi/da4qi4.hpp"

using namespace da4qi4;

void add(Context ctx)
{
    try
    {
        std::string a = ctx->Req().GetUrlParameter("a");
        std::string b = ctx->Req().GetUrlParameter("b");

        int na = std::stoi(a); 
        int nb = std::stoi(b);

        int c = na + nb;

        ctx->ModelData()["c"] = c;
    }
   catch(std::exception const& e)
   {
        ctx->Logger()->warn("hand add exception. {}. {}. {}.", a, b, e.what());
        ctx->ModelData()["c"] = std::string("同学，不要乱输入加数嘛！") + e.what();
   }

   ctx->Render().Pass();
}
```

关键在异常处理。第一行是：

```C++
ctx->Logger()->warn("hand add exception. {}. {}. {}.", a, b, e.what());
```

三个重点：

- 一是如何通过上下文（Context）得到当前应用的日志记录器：ctx->Logger()。它实际上是 ctx->App().GetLogger() 的简写。

- 二是得益于“spdlog”的语法，记日志就这么简单：要显示三个信息：a、b和异常，就在前面的格式字符串中，写上三个 {} ，最终就可以在日志中看到一行完整的内容。

- 三是我们使用“warn()”，而不是“error()”，这体现了服务程序在此刻的淡定内心：不就是用户输入错误嘛？有什么因结什么果。用户输入错误，就返回给他一行出错信息。何事慌张？警告而已。

第二行是：

```C++
ctx->ModelData()["c"] = std::string("同学，不要乱输入加数嘛！") + e.what();
```

重点在于：赋值操作的右值，是一个字符串。“ModelData”，即“模型数据”在这里指的是即将写往页面模板的数据，和C++的强类型相比，页面上的数据不用太区分类型。所以，“c” 本是a+b之和，按理说是整数类型，但我们却可以往里写入一行字符串，这样，当用户捣乱造成 a + b 无法执行时，他就会看到一行出错信息。

> main() 函数中如何初始化日志，已经演示过，不再给出代码。

## 1.7 WebSocket

### 1.7.1 HTTP 对比 WebSocket

先简单说下在业务与技术上，传统HTTP访问和WebSocket访问的核心区别。

HTTP访问讲究“无状态”，当然，一个业务系统怎么可能无状态，只不过是将状态都放在数据中（缓存、数据库），所谓的无状态是指业务逻辑相关的“类/class”应该无状态——这正好和“class/类”或“object/对象”本质是一个“状态机”相冲突——幸好C++支持多范式开发，所以在前面的例子中，我们几乎不设计“class”，而是使用天生无状态的自由函数。“类与对象”想写成不带状态的状态，难；而自由函数想写出带“状态”来，还真不简单。

到了WebSocket，长连接，通常这时候就有状态——甚至此时底层的网络连接保持或断开本身就是一种状态。比如使用WebSocket写一个页面聊天室，有人连线，就是上线了（进聊天室）；有人断线，那就是下线了（出聊天室）。再比如，假设我们的“聊天室”要限制“潜水”用户，就至少得记录每个用户这些状态：上线多久一直没有说话？反过来，如果要限制话痨用户，也至少需要记录一个用户说话记录的记数——这些都是状态。

结论：HTTP访问后端讲究无状态，所以很适合使用“面向过程”的自由函数，而WebSocket 的后端往往需要保持状态，所以这时候“面向对象”比较合适。da4qi4的WebSocket 在保持对无状态的支持下，增加并且主要使用“有状态”的类设计做支持。

### 1.7.2 大器WebSocket后台实现特性

- 支持直接接入WebSokcet支持，也支持从nginx继续反向代理。

- 支持一个端口同时响应HTTP和WebSocket请求。

- 支持ws和wss。

- 支持服务端推送（其实是WebSocket的要求）。

- 支持大报文分段传输（其实也是WebSocket的要求）。

- 支持群发。

- 保持和HTTP相对一致的概念与设计，比如上下文：Context。

- WebSocket连接时，可以方便获得连接升级(upgrade)前的Cookie、HTTP报头、URL等信息。

### 1.7.3 使用示例

一、先演示面向对象思路的写法。

1. 先写一个类，派生自 da4qi4::Websocket::EventsHandler。

```C++
using namespace da4qi4;

class MyEventsHandler : public Websocket::EventsHandler
{
public:
    bool Open(Websocket::Context ctx)　{ return　true; }   //允许该ws连接
    
    void OnText(Websocket::Context ctx, std::string&& data, bool isfinish)
    {
        ctx->Logger()->info("收到： {}.", data);
        ctx->SendText("已阅!"); 
    }
    
    void OnBinary(Websocket::Context ctx, std::string&& data, bool isfinish)
    {
        //此时data是二进制数据，比如图片什么的，可以保存下来...
    }
    
    void OnError(Websocket::Context ctx
                , Websocket::EventOn evt //在哪个环节出错，读或写？
                , int code //出错编号
                , std::string const& msg //出错信息
                )    
    {
        ctx->Logger()->error("出错了. {} - {}.", code, msg);
    }
    
    void OnClose(Websocket::Context ctx, Websocket::EventOn evt) 
    {
        ctx->Logger()->info("Websocket连接已经关闭.");
    }   
};
```

2. 在主函数中，在某个“Application”上注册一个WebSocket的后台处理方法。这个方法用来创建(new)出刚刚定义的那个“MyEventsHandler”的对象，我们使用lambda实现：

```C++
#include "daqi/da4qi4.hpp"
#include "daqi/websocket/websocket.hpp" //引入websocket相关定义

using namespace da4qi4;

class MyEventsHandler : public Websocket::EventsHandler
{
     //见上
};

int main()
{
    auto svc = Server::Supply(4098);
    auto app = svc->DefaultApp();
    app->InitLogger("log/");
　
    //在某个app的指定URL下，挂接一个websocket响应处理
    app->RegistWebSocket("/ws", UrlFlag::url_full_path, 
           [](){ return new MyEventsHandler; }
    );

    svc->Run();
}
```

现在，让你的前端开发人员，在ＨＴＭＬ页面里，用JS写一段代码，类似于：

```javascript
var ws = new WebSocket("ws://127.0.0.1:4098/ws");

ws.onopen = function(evt) {
    this.send("Hello WebSocket.");
}

ws.onmessage = function (evt) {
    console.log(evt.data);
}
……
```

前后端就可以聊起来了。

二、如果后台业务逻辑确实很简单，那写一个类，还派生什么的确实显得很笨。此时也可以使用简单的函数、labmbda来快速响应。

方法是定义一个大器预定的　“ Websocket::EventHandleFunctor”　变量：

```C++
#include "daqi/da4qi4.hpp"
#include "daqi/websocket/websocket.hpp" //引入websocket相关定义

using namespace da4qi4;

/*不需要类定义了*/

int main()
{
    auto svc = Server::Supply(4098);
    auto app = svc->DefaultApp();
    app->InitLogger("log/");

    Websocket::EventHandleFunctor functor;
    functor.DoOnText = [] (Websocket::Context ctx, std::string&& data, bool isfinished)
    {
        ctx->Logger()->info("收到： {}.", data);
        ctx->SendText("已阅!");
    }

    app->RegistWebSocket("/ws", UrlFlag::url_full_path, functor);

    svc->Run();
}
```

## 1.8 更多

### 1.8.1 框架更多集成功能

1. cookie支持

2. 前端（浏览器）缓存支持

3. Redis 缓存支持

4. Session 支持

5. 静态文件

6. 模板文件更新检测及热加载

7. HTTP/HTTPS 客户端组件（已基于此实现微信扫码登录、阿里云短信的C++SDK，见下）

8. POST响应支持

9. 文件上传、下载

10. 访问限流

11. JSON

12. 纯数据输出的API接口，与前端AJAX配合

13. 框架全方式集成：(a) 基于源代码集成、(b) 基于动态库集成、(c) 基于静态库集成

14. 常用编码转换（UTF-8、UCS、GBK、GB18030）

15. ……

### 1.8.2 框架外围可供集成的工具

1. 数据库访问

2. 和nginx配合（实现负载均衡的快速横向扩展）

3. 阿里短信云异步客户端

4. 微信扫一扫登录异步客户端

5. 基于OpenSSL的数据加密工具

6. 常用字符串处理

7. ……

# 二、如何构建

## 2.1 基于生产环境构建

尽管使用的组件都支持跨平台，但大器当前仅支持在Linux下环境编译；大多数实际项目的服务，都运行在Linux下。

> Web 服务端部署在Linux上，这是有原因的： (1) 前述的外围组件：nginx、mysql、redis 在Linux下安装都是一行命令的事，远比Windows方便。(2) 如果你使用当前非常流行的Docker，更是如此。 (3) 事实上Web 端的开源大杀器都是先提供Linux版，然后再考虑出Windows版，甚至有官方拒绝出Windows版（比如Redis 作者就“无情”地拒绝了微软提供的，将Redis变成也可以在Windows执行的补丁包）。

大器框架同样会在后续某个时间，提供Windows版本。

当前国内各云计算提供商，均提供 Ubuntu Server 版本为 18.04 LTS 版本。如“[0.5 紧跟国内生产环境](#05-紧跟国内生产环境)” 小节所述，你有一台2019年或更新的Ubuntu云服务器，那么在其上构建大器，则所需的软件、依赖库等，暂时只有一个用于中文编码转换的 iconv 库，需要手动下载编译之外，其它的都可以从Ubuntu 软件仓库中获取。

以下内容均以 Ubuntu 18.04  为例，考虑日常开发不会直接使用Server版，因此严格讲，以下内容均假设系统环境为 Ubuntu 18.04 桌面版。

> 小提示-服务器与开发机的区别：
> 
> 开发机（桌面版）为安装组件时，需临时提升用户权限，即相关指令前面多出个“sudo ”。如果是在服务版的Ubuntu操作，默认就是拥有更高管理权限的根用户，因此不需要该指令。例如：
> 开发机：sudo apt install git
> 服务器：apt install git

## 2.2 准备编译工具

1. 如果未安装或不知道有没有安装（以下简称为“准备”） GCC 编译器：

```shell
sudo apt install build-essential
```

2. 准备 CMake构建套件，请：

```shell
sudo apt install cmake
```

## 2.3 准备第三方库

1. 准备 boost 开发库：

```shell
sudo apt install libboost-dev libboost-filesystem libboost-system
```

2. 准备openssl及其开发库：

```shell
sudo apt install openssl libssl-dev
```

3. 准备 libiconv 库

我们使用汉字，而汉字有多种编码方案，因此，汉字的编码转换，是开发包括Web应用在内各种软件系统的常见需求。大器框架通过集成iconv库用以实现支持多国语言多种编码的转换功能。

先下载：https://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.16.tar.gz

在Ubuntu图形界面中，双击该.gz文件，再点击其内.tar文件，解压后得到 “libiconv-1.16”文件夹。在终端进入该文件夹，依次输入以下三行，完成安装：

```shell
./configure --prefix=/usr/local
make
sudo make install
```

4. 最后，确保生效所安装的动态库在系统中能被找到：

```shell
sudo ldconfig
```

## 2.4 下载大器源代码

通常你应该安装 git 工具，如果没有或不确定，请打开终端（Ctrl + Alt + T），按如下指令安装。

```shell
sudo apt install git 
```

然后在本地新建一文件夹，假设命名为 daqi，打开终端进入该目录，从github克隆代码：

```shell
git clone https://github.com/d2school/da4qi4.git
```

或者从国内的gitee克隆代码（速度比较快）：

```shell
git  clone  https://gitee.com/zhuangyan-stone/da4qi4_public.git
```

最终，你**将在前述的“daqi”目录下，得到一个子目录“da4qi4”(也可能是别的，看git源)**。大器项目的代码位于后者内，其内你应该能看到“src”、“include”等多个子目录。

> 感谢你看到这里。如有余力，建议在以上两个网站均为本开源项目打个星 。

## 2.5 编译“大器”库

**重要：以下假设大器源代码位于“daqi/da4qi4”目录下。**

1. 准备构建目录

请在“daqi”之下（和“da4qi4”平级）的位置，新建一目录，名为“build”：

```shell
mkdir build
```

进入该当目录：

```shell
cd build
```

2. 执行CMake

> 如果你使用的是1.66或更高版本的boost库，请先打开项目下的CMakefile.txt文件，找到第13行：set(USE_LOCAL_BOOST_VERSION OFF) 将OFF改为ON： set(USE_LOCAL_BOOST_VERSION ON)


```shell
cmake -D_DAQI_TARGET_TYPE_=SHARED_LIB -DCMAKE_BUILD_TYPE=Release ../da4qi4/
```

将生成目标为“发行版（Release）”的大器“动态库（SHARED_LIB）”。

* 如果希望生成调试版本，请将“Release”替换为“Debug”;
* 如果希望生成静态库版本，请将“SHARED_LIB”替换为“STATIC_LIB”;
* 更多编译目标设置，请到本项目官网“www.d2school.com”

一切正常的看，将看到终端上输出“Generating done”等字样。其中更多内容中，包含有boost库的版本号、库所在路径，以及一行“\~BUILD DAQI AS SHARED LIB\~”字样以指示大器的编译形式（SHARED LIB)。

3. 开始编译

```shell
make
```

> 小提示：并行编译
> 如果你的电脑拥有多核CPU，并且内存足够大（至少8G），可以按如下方式并行编译（其中 -j 后面的数字，指明并行编译的核数，以下以四核数例）：
> make -j4 

完成make之后，以上过程将在build目录内，得到“libda4qi4.so”；如果是调试版，将得到 “libda4qi4_d.so”。 如果是静态库，则相应的扩展名为 “.a”。

以上工作都是一次性的，以后你再使用da4qi4开发新项目，都从下面的步骤开始。

## 2.6 在你的项目中使用da4qi4库

现在，你可以使用你熟悉IDE（Code::Blocks、Qt Creator、CodeLite等）中，构建你的项目，然后以类型使用其它开发库的方式，添加大器的库文件（就是前一步构建所得的.so或.a文件），及大器的头文件。

1. da4qi4库文件。  即前面编译大器库得到的库文件，如“libda4qi4.so”或“libda4qi4.a”，“libda4qi4_d.so”、“libda4qi4_d.a”等文件。
2. da4qi4库依赖的文件。 在Linux下，它们是 pthread、ssl、crypto、boost_filesystem、boost_system
3. da4qi4头文件：“大器项目目录”、“大器项目目录/include”及“大器项目目录/nlohmann_json/include/”

下面以CMake的CMakefiles.txt为例：

```cmake
cmake_minimum_required(VERSION 3.5)

project(hello_daqi LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall")

# 此处设置大器项目目录
set(_DAQI_PROJECT_PATH_ "你的大器项目所在目录")
# 此处设置大器项目编译后得到的 .so 文件所在目录
set(_DAQI_LIBRARY_PATH_ "你的大器项目动态库所在目录")

include_directories(${_DAQI_PROJECT_PATH_})
include_directories(${_DAQI_PROJECT_PATH_}/include)
include_directories(${_DAQI_PROJECT_PATH_}/nlohmann_json/include/)

find_package(Boost 1.65.0 REQUIRED COMPONENTS filesystem system)
link_directories(${_DAQI_LIBRARY_PATH_})

link_libraries(da4qi4)

link_libraries(pthread)
link_libraries(ssl)
link_libraries(crypto)
link_libraries(boost_filesystem)
link_libraries(boost_system)

add_executable(hello_daqi main.cpp)
```

现在你可以从之前 [1.1 一个空转的Web Server](#11-一个空转的web-server) 重新看起。

# 三、运行时外部配套系统

## 3.1 运行时依赖说明

一个Web系统常用到缓存系统和数据库系统。大器框架对二者依赖情况如下：

* 完全不依赖数据库；
* 简单例子不依赖缓存系统，但一旦需要用到Web 系统常见的“会话/SESSION”功能，则需要依赖redis缓存库。

## 3.2 Redis的安装

显然，这已经不是本开源项目的自己的说明内容。不过，反正在Ubuntu Linux下安装Redis就一行话：

```C++
sudo apt install redis-server
```

这不仅会安装redis服务，而且会顺便在本机redis的命令行客户端 redis-cli。

* 有关如何在你写的大器Web Server中实现SESSION，请参看本项目官网www.d2school.com 相关（免费视频）课程；
* 有关Redis的学习，请关注www.d2school.com 课程。

## 3.3 数据库

* 可以使用 mysql 官方的 MySQL C++ Connector；
* 新人强烈推荐： 相对传统的C++封装 ： [MySQL++](https://tangentsoft.com/mysqlpp/home) （注：欢迎关注《白话 C++》下册，有详细的 MySQL 数据库及 MySQL++使用的章节；
* 新人推荐： [CppDB](http://cppcms.com/sql/cppdb/) 
* 到 [github](https://github.com)上，搜索 “MySQL C++”，你将找到大量国内或国外的MySQL C++连接库；
* 有经验的C++程序员推荐：[sqlpp11](https://github.com/rbock/sqlpp11)

更多课程（视频课程、文字课程），请到 [第2学堂](https://www.d2school.com)查看。
谢谢。
