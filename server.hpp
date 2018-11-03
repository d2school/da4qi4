#ifndef DAQI_SERVER_HPP
#define DAQI_SERVER_HPP

#include "def/asio_def.hpp"

#include "engine.hpp"
#include "application.hpp"
#include "handler.hpp"

namespace da4qi4
{

class Server
    : public std::enable_shared_from_this<Server>
{
    Server(Tcp::endpoint endpoint, size_t thread_count);
public:
    using Ptr = std::shared_ptr<Server>;

    static Ptr Supply(unsigned short port, size_t thread_count = 0);
    static Ptr Supply(std::string const& host, unsigned short port, size_t thread_count = 0);

public:
    void Run();
    void Stop();

public:
    bool AddApp(Application& app)
    {
        return AppMgr()->Add(app);
    }

public:
    Application* AddHandler(HandlerMethod m, std::string const& url, Handler h)
    {
        return AddHandler(m, router_equals(url), h);
    }

    Application* AddHandler(HandlerMethod m, router_equals r, Handler h);
    Application* AddHandler(HandlerMethod m, router_starts r, Handler h);
    Application* AddHandler(HandlerMethod m, router_regex r, Handler h);

    Application* AddHandler(HandlerMethods ms, std::string const& url, Handler h)
    {
        return AddHandler(ms, router_equals(url), h);
    }
    Application* AddHandler(HandlerMethods ms, router_equals r, Handler h);
    Application* AddHandler(HandlerMethods ms, router_starts r, Handler h);
    Application* AddHandler(HandlerMethods ms, router_regex r, Handler h);

public:
    Application* PrepareApp(std::string const& url);

private:
    void start_accept();
    void do_accept();
    void do_stop();
private:
    void make_default_app_if_need();

private:
    bool _stopping = false;
    boost::asio::io_context _ioc_for_self;
    Tcp::acceptor _acceptor;
    boost::asio::signal_set _signals;

    IOContextPool _ioc_pool_for_connections;
};

}

#endif // DAQI_SERVER_HPP
