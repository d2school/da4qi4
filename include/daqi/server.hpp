#ifndef DAQI_SERVER_HPP
#define DAQI_SERVER_HPP

#include <atomic>
#include <list>

#include <boost/asio/deadline_timer.hpp>

#include "daqi/def/asio_def.hpp"

#include "daqi/server_engine.hpp"
#include "daqi/application.hpp"
#include "daqi/handler.hpp"

namespace da4qi4
{

extern int const _detect_templates_interval_seconds_;

class Server
{
    Server(Tcp::endpoint endpoint, size_t thread_count);
public:
    using IdleFunction = std::function<void (void)>;

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
    void PauseIdleTimer();
    void ResumeIdleTimer();

    bool IsIdleTimerRunning() const
    {
        return _idle_running;
    }

    void EnableDetectTemplates(int interval_seconds = _detect_templates_interval_seconds_)
    {
        assert(interval_seconds > 0);

        _detect_templates_status.interval_seconds = interval_seconds;
        _detect_templates_status.next_timepoint = std::time(nullptr) + interval_seconds;

        _detect_templates = true;

        if (_running && !_idle_running)
        {
            ResumeIdleTimer();
        }
    }

    void DisableDetectTemplates()
    {
        _detect_templates = false;
    }

    bool IsEnabledDetetTemplates() const
    {
        return _detect_templates;
    }

    void AppendIdleFunction(int interval_seconds, IdleFunction func);

public:
    ApplicationPtr DefaultApp(std::string const& name = "");

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
    void make_default_app_if_empty();
    void make_default_app(std::string const& name);

    void start_idle_timer();
    void on_idle_timer(errorcode const& ec);
    void stop_idle_timer();

private:
    int _idle_interval_seconds;

private:
    std::atomic_bool _running;
    std::atomic_bool _stopping;

    IOContextPool _ioc_pool;
    Tcp::acceptor _acceptor;
    boost::asio::signal_set _signals;

    std::atomic_bool _idle_running;

    std::atomic_bool _detect_templates;
    boost::asio::deadline_timer _idle_timer;

    struct IdleFunctionStatus
    {
        IdleFunctionStatus()
            : interval_seconds(0), next_timepoint(static_cast<std::time_t>(0))
        {}

        IdleFunctionStatus(IdleFunctionStatus const&) = default;

        IdleFunctionStatus(int interval_seconds, IdleFunction func)
            : interval_seconds(interval_seconds),
              next_timepoint(std::time(nullptr) + interval_seconds), func(func)
        {}

        int interval_seconds;
        std::time_t next_timepoint;
        IdleFunction func;
    };

    IdleFunctionStatus _detect_templates_status;
    std::list<IdleFunctionStatus> _idle_functions;

private:
    static int call_idle_function_if_timeout(std::time_t now, IdleFunctionStatus& status);
};

}

#endif // DAQI_SERVER_HPP
