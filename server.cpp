#include  "server.hpp"

#include <functional>
#include <boost/date_time/posix_time/ptime.hpp>

#include "def/boost_def.hpp"
#include "utilities/asio_utilities.hpp"

#include "connection.hpp"

namespace da4qi4
{

Server::Server(Tcp::endpoint endpoint, size_t thread_count)
    : _stopping(false)
    , _ioc_pool(thread_count)
    , _acceptor(_ioc_pool.GetIOContext())
    , _signals(_ioc_pool.GetIOContext())
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
    std::cout << "server destory." << std::endl;
}

void Server::Run()
{
    AppMgr().Mount();
    start_idle_timer();
    start_accept();

    _ioc_pool.Run();

    std::cout << "server's running stopped." << std::endl;
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

ApplicationPtr Server::PrepareApp(std::string const& url)
{
    make_default_app_if_need();

    auto app = AppMgr().FindByURL(url);

    if (!app)
    {
        std::cerr << "no found a app for URL. " << url << std::endl;
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

    ConnectionPtr cnt = Connection::Create(ioc_ctx.first, ioc_ctx.second);

    _acceptor.async_accept(cnt->GetSocket()
                           , [this, cnt](errorcode ec)
    {
        if (ec)
        {
            std::cerr << ec.message() << std::endl;

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

    std::cout << "\r\nReceives an instruction to stop the service." << std::endl;
    stop_idle_timer();
    _ioc_pool.Stop();
}

void Server::start_idle_timer()
{
    errorcode ec;
    _idle_timer.expires_from_now(boost::posix_time::seconds(25), ec);

    if (ec)
    {
        std::cerr << "Set server's idle timer expires time fail." << std::endl;
    }
    else
    {
        _idle_timer.async_wait(std::bind(&Server::on_idle_timer, this
                                         , std::placeholders::_1));
    }
}

void Server::on_idle_timer(errorcode const& ec)
{
    if (ec)
    {
        std::cerr << "Server idle timer exception. " << ec.message() << std::endl;
        return;
    }

    AppMgr().CheckTemplatesUpdate();

    start_idle_timer();
}

void Server::stop_idle_timer()
{
    _idle_timer.cancel();
}


}//namespace da4qi4
