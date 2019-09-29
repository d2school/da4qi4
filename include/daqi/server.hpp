#ifndef DAQI_SERVER_HPP
#define DAQI_SERVER_HPP

#include <atomic>

#include <boost/asio/deadline_timer.hpp>

#include "daqi/def/asio_def.hpp"

#include "daqi/server_engine.hpp"
#include "daqi/application.hpp"
#include "daqi/handler.hpp"

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
    static Ptr Supply(unsigned short port = 80);

    ~Server();

    IOContextPool* GetIOContextPool()
    {
        return &_ioc_pool;
    }
public:
    void Run();
    void Stop();

public:
    void EnableIdleTimer(std::size_t interval_seconds = 0);
    void DisableIdleTimer();
    void SetIdleTimerInterval(std::size_t seconds = 20);

    void EnableDetectTemplates()
    {
        _detect_templates = true;
    }

    void DisableDetectTemplates()
    {
        _detect_templates = false;
    }

    bool IsEnabledDetetTemplates() const
    {
        return _detect_templates;
    }

public:
    bool Mount(ApplicationPtr app);

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

    bool AddEqualsRouter(HandlerMethod m, std::vector<std::string> const& urls, Handler h);
    bool AddStartsRouter(HandlerMethod m, std::vector<std::string> const& urls, Handler h);
    bool AddRegexRouter(HandlerMethod m, std::vector<std::string> const& urls, Handler h);

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

    static int const _defalut_idle_interval_seconds_ = 25;

    std::atomic_bool _detect_templates;
    int _idle_interval_seconds;
    boost::asio::deadline_timer _idle_timer;
};

}

#endif // DAQI_SERVER_HPP
