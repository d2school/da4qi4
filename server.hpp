#ifndef DAQI_SERVER_HPP
#define DAQI_SERVER_HPP

#include <atomic>

#include <boost/asio/deadline_timer.hpp>

#include "def/asio_def.hpp"

#include "server_engine.hpp"
#include "application.hpp"
#include "handler.hpp"

namespace da4qi4
{

class Server
{
    Server(Tcp::endpoint endpoint, size_t thread_count);
public:
    using Ptr = std::unique_ptr<Server>;

    static Ptr Supply(std::string const& host, unsigned short port, size_t thread_count);
    static Ptr Supply(unsigned short port, size_t thread_count);

    static Ptr Supply(std::string const& host, unsigned short port);
    static Ptr Supply(unsigned short port);

    ~Server();

    IOContextPool* GetIOContextPool()
    {
        return &_ioc_pool;
    }
public:
    void Run();
    void Stop();

public:
    bool Mount(ApplicationPtr app)
    {
        return AppMgr().Add(app);
    }

public:
    ApplicationPtr AddHandler(HandlerMethod m, std::string const& url, Handler h)
    {
        return AddHandler(m, router_equals(url), h);
    }

    ApplicationPtr AddHandler(HandlerMethod m, router_equals r, Handler h);
    ApplicationPtr AddHandler(HandlerMethod m, router_starts r, Handler h);
    ApplicationPtr AddHandler(HandlerMethod m, router_regex r, Handler h);

    ApplicationPtr AddHandler(HandlerMethods ms, std::string const& url, Handler h)
    {
        return AddHandler(ms, router_equals(url), h);
    }
    ApplicationPtr AddHandler(HandlerMethods ms, router_equals r, Handler h);
    ApplicationPtr AddHandler(HandlerMethods ms, router_starts r, Handler h);
    ApplicationPtr AddHandler(HandlerMethods ms, router_regex r, Handler h);

public:
    ApplicationPtr PrepareApp(std::string const& url);

private:
    void start_accept();
    void do_accept();
    void do_stop();
private:
    void make_default_app_if_need();

    void start_idle_timer();
    void on_idle_timer(errorcode const& ec);
    void stop_idle_timer();
private:
    std::atomic_bool _stopping;

    IOContextPool _ioc_pool;
    Tcp::acceptor _acceptor;
    boost::asio::signal_set _signals;

    boost::asio::deadline_timer _idle_timer;
};

}

#endif // DAQI_SERVER_HPP
