#include  "daqi/server.hpp"

#include <functional>
#include <boost/date_time/posix_time/ptime.hpp>

#include "daqi/def/log_def.hpp"
#include "daqi/def/boost_def.hpp"
#include "daqi/utilities/asio_utilities.hpp"

#include "daqi/connection.hpp"

namespace da4qi4
{

#ifdef NDEBUG
    int const _detect_templates_interval_seconds_ =  15 * 60; //15 minutes
#else
    int const _detect_templates_interval_seconds_ =  1 * 60;  //1 minutes
#endif

int const _first_idle_interval_seconds_ = 10;            //10 seconds

Server::Server(Tcp::endpoint endpoint, size_t thread_count)
    : _idle_interval_seconds(_first_idle_interval_seconds_)
    , _running(false), _stopping(false)
    , _ioc_pool(thread_count)
    , _acceptor(_ioc_pool.GetIOContext())
    , _signals(_ioc_pool.GetIOContext())
    , _idle_running(false)
    , _detect_templates(false)
    , _idle_timer(_ioc_pool.GetIOContext())
{
    _signals.add(SIGINT);
    _signals.add(SIGTERM);

#if defined(SIGQUIT)
    _signals.add(SIGQUIT);
#endif
    _signals.async_wait(std::bind(&Server::do_stop, this));

    _acceptor.open(endpoint.protocol());
    _acceptor.set_option(Tcp::acceptor::reuse_address(true));
    _acceptor.bind(endpoint);
    _acceptor.listen();

    log::Server()->info("Supping on {}:{}, {} thread(s).",
                        endpoint.address().to_string()
                        , endpoint.port()
                        , _ioc_pool.Size()
                       );
}

Server::Ptr Server::Supply(unsigned short port, size_t thread_count)
{
    return Ptr(new Server({Tcp::v4(), port}, thread_count));
}

Server::Ptr Server::Supply(std::string const& host, unsigned short port, size_t thread_count)
{
    return Ptr(new Server(Utilities::make_endpoint(host, port), thread_count));
}

Server::Ptr Server::Supply(std::string const& host, unsigned short port)
{
    return Ptr(new Server(Utilities::make_endpoint(host, port), 0));
}

Server::Ptr Server::Supply(unsigned short port)
{
    return Ptr(new Server({Tcp::v4(), port}, 0));
}

Server::~Server()
{
    log::Server()->info("Destroied.");
}

bool Server::Mount(ApplicationPtr app)
{
    if (AppMgr().MountApplication(app))
    {
        log::Server()->info("Application {} mounted.", app->GetName());
        return true;
    }

    log::Server()->error("Application {} mount fail.", app->GetName());
    return false;
}

void Server::Run()
{
    assert(!_running);

    if (_running)
    {
        log::Server()->critical("Server is running already.");
        return;
    }

    _running = true;

    AppMgr().Mount();

    _idle_running = true;
    start_idle_timer();

    start_accept();

    _ioc_pool.Run();

    log::Server()->info("Stopped.");
}

void Server::Stop()
{
    do_stop();
}

std::string extract_app_path(std::string const& app_root, std::string const& path)
{
    assert((Utilities::iStartsWith(path, app_root)) && "URL MUST STARTSWITH APPLICATION ROOT");

    return path.substr(app_root.size());
}

template<typename R, typename M>
ApplicationPtr ServerAddHandler(Server* s, M m, R r, Handler h)
{
    ApplicationPtr app = s->PrepareApp(r.s);

    if (app)
    {
        r.s = extract_app_path(app->GetUrlRoot(), r.s);

        if (app->AddHandler(m, r, h))
        {
            return app;
        }
    }

    return nullptr;
}

ApplicationPtr Server::AddHandler(HandlerMethod m, router_equals r, Handler h)
{
    return ServerAddHandler(this, m, r, h);
}

ApplicationPtr Server::AddHandler(HandlerMethod m, router_starts r, Handler h)
{
    return ServerAddHandler(this, m, r, h);
}

ApplicationPtr Server::AddHandler(HandlerMethod m, router_regex r, Handler h)
{
    return ServerAddHandler(this, m, r, h);
}

ApplicationPtr Server::AddHandler(HandlerMethods ms, router_equals r, Handler h)
{
    return ServerAddHandler(this, ms, r, h);
}

ApplicationPtr Server::AddHandler(HandlerMethods ms, router_starts r, Handler h)
{
    return ServerAddHandler(this, ms, r, h);
}

ApplicationPtr Server::AddHandler(HandlerMethods ms, router_regex r, Handler h)
{
    return ServerAddHandler(this, ms, r, h);
}

bool Server::AddEqualsRouter(HandlerMethod m, std::vector<std::string> const& urls, Handler h)
{
    for (auto a : urls)
    {
        if (!AddHandler(m, router_equals(a), h))
        {
            return false;
        }
    }

    return true;
}

bool Server::AddStartsRouter(HandlerMethod m, std::vector<std::string> const& urls, Handler h)
{
    for (auto a : urls)
    {
        if (!AddHandler(m, router_starts(a), h))
        {
            return false;
        }
    }

    return true;
}

bool Server::AddRegexRouter(HandlerMethod m, std::vector<std::string> const& urls, Handler h)
{
    for (auto a : urls)
    {
        if (!AddHandler(m, router_regex(a), h))
        {
            return false;
        }
    }

    return true;
}

void Server::AppendIdleFunction(int interval_seconds, IdleFunction func)
{
    assert(!_idle_running && "Idle-function is running.");
    assert(interval_seconds > 0);

    _idle_functions.emplace_back(interval_seconds, func);
}

ApplicationPtr Server::PrepareApp(std::string const& url)
{
    make_default_app_if_need();

    auto app = AppMgr().FindByURL(url);

    if (!app)
    {
        log::Server()->warn("Application on url \"{}\" no found.", url);
        return nullptr;
    }

    return app;
}

void Server::make_default_app_if_need()
{
    if (AppMgr().IsEmpty())
    {
        AppMgr().CreateDefaultIfEmpty();
    }
}

void Server::start_accept()
{
    make_default_app_if_need();
    do_accept();
}

void Server::do_accept()
{
    auto ioc_ctx = _ioc_pool.GetIOContextAndIndex();

    ConnectionPtr cnt = Connection::Create(ioc_ctx.first /* ioc */, ioc_ctx.second /* index */);

    _acceptor.async_accept(cnt->GetSocket()
                           , [this, cnt](errorcode ec)
    {
        if (ec)
        {
            log::Server()->error("Acceptor error: {}", ec.message());

            if (_stopping)
            {
                return;
            }
        }
        else
        {
            cnt->StartRead();
        }

        do_accept();
    });
}

void Server::do_stop()
{
    _stopping = true;

    log::Server()->info("Stopping...");

    stop_idle_timer();
    _ioc_pool.Stop();
}

void Server::start_idle_timer()
{
    if (!_idle_running)
    {
        return;
    }

    errorcode ec;
    _idle_timer.expires_from_now(boost::posix_time::seconds(_idle_interval_seconds), ec);

    if (ec)
    {
        log::Server()->error("Idle timer set expires fail.");
        return;
    }

    _idle_timer.async_wait(std::bind(&Server::on_idle_timer, this, std::placeholders::_1));
}

void appmgr_check_templates_update()
{
    AppMgr().CheckTemplatesUpdate();
}

int Server::call_idle_function_if_timeout(std::time_t now, IdleFunctionStatus& status)
{
    int distance_seconds = static_cast<int>(status.next_timepoint - now);

    if (distance_seconds <= 0)
    {
        status.func();

        now = std::time(nullptr);
        status.next_timepoint = now + status.interval_seconds;
        return status.interval_seconds;
    }

    return distance_seconds;
}

void update_min_distance_seconds(int& min_distance_seconds, int a_distance_seconds)
{
    if (min_distance_seconds <= 0 || min_distance_seconds > a_distance_seconds)
    {
        min_distance_seconds = a_distance_seconds;
    }
}

void Server::on_idle_timer(errorcode const& ec)
{
    if (ec)
    {
        if (ec != boost::system::errc::operation_canceled)
        {
            log::Server()->error("Idle timer exception. {}", ec.message());
        }

        _idle_running = false;
        return;
    }

    std::time_t now = std::time(nullptr);
    int min_distance_seconds = -1;

    if (_detect_templates)
    {
        if (!_detect_templates_status.func)
        {
            _detect_templates_status.func = appmgr_check_templates_update;
        }

        if (_detect_templates_status.interval_seconds <= 0)
        {
            _detect_templates_status.interval_seconds = _detect_templates_interval_seconds_;
        }

        update_min_distance_seconds(min_distance_seconds,
                                    call_idle_function_if_timeout(now, _detect_templates_status));
    }

    for (auto& ss : _idle_functions)
    {
        update_min_distance_seconds(min_distance_seconds,
                                    call_idle_function_if_timeout(now, ss));
    }

    if (min_distance_seconds <= 0)
    {
        _idle_running = false;
    }
    else
    {
        if (_idle_interval_seconds != min_distance_seconds)
        {
            _idle_interval_seconds = min_distance_seconds;
        }

        start_idle_timer();
    }
}

void Server::stop_idle_timer()
{
    errorcode ec;

    _idle_timer.cancel(ec);

    if (ec)
    {
        log::Server()->error("Cancel idle timer exception. {}", ec.message());
    }
}

void Server::PauseIdleTimer()
{
    if (_idle_running)
    {
        _idle_running = false;
    }
}

void Server::ResumeIdleTimer()
{
    if (_idle_running)
    {
        return;
    }

    _idle_running = true;
    start_idle_timer();
}

}//namespace da4qi4
