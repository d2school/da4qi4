#include  "server.hpp"

#include <functional>

#include "def/boost_def.hpp"
#include "utilities/asio_utilities.hpp"

#include "connection.hpp"

namespace da4qi4
{

Server::Server(Tcp::endpoint endpoint, size_t thread_count)
    : _acceptor(_ioc_for_self), _signals(_ioc_for_self), _ioc_pool_for_connections(thread_count)
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

void Server::Run()
{
    _ioc_pool_for_connections.Run();

    this->start_accept();

    while (!_stopping)
    {
        try
        {
            errorcode e;
            _ioc_for_self.run(e);

            if (e)
            {
                std::cerr << e.message() << std::endl;
            }
        }
        catch (std::exception const& e)
        {
            std::cerr << e.what() << std::endl;
        }
        catch (...)
        {
            std::cerr << "unknown exception." << std::endl;
        }
    }

    std::cout << "Waitting for connections stopped..." << std::endl;
    _ioc_pool_for_connections.Wait();
    std::cout << "Bye" << std::endl;
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
Application* ServerAddHandler(Server* s, M m, R r, Handler h)
{
    Application* app = s->PrepareApp(r.s);

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

Application* Server::AddHandler(HandlerMethod m, router_equals r, Handler h)
{
    return ServerAddHandler(this, m, r, h);
}

Application* Server::AddHandler(HandlerMethod m, router_starts r, Handler h)
{
    return ServerAddHandler(this, m, r, h);
}

Application* Server::AddHandler(HandlerMethod m, router_regex r, Handler h)
{
    return ServerAddHandler(this, m, r, h);
}

Application* Server::AddHandler(HandlerMethods ms, router_equals r, Handler h)
{
    return ServerAddHandler(this, ms, r, h);
}

Application* Server::AddHandler(HandlerMethods ms, router_starts r, Handler h)
{
    return ServerAddHandler(this, ms, r, h);
}

Application* Server::AddHandler(HandlerMethods ms, router_regex r, Handler h)
{
    return ServerAddHandler(this, ms, r, h);
}

Application* Server::PrepareApp(std::string const& url)
{
    make_default_app_if_need();

    auto app = AppMgr()->FindByURL(url);

    if (!app)
    {
        std::cerr << "no found a app for URL. " << url << std::endl;
        return nullptr;
    }

    return app;
}

void Server::make_default_app_if_need()
{
    if (AppMgr()->IsEmpty())
    {
        AppMgr()->CreateDefaultIfEmpty();
    }
}

void Server::start_accept()
{
    make_default_app_if_need();
    do_accept();
}

void Server::do_accept()
{
    ConnectionPtr cnt = Connection::Create(_ioc_pool_for_connections.GetIOContext());

    auto self = shared_from_this();
    _acceptor.async_accept(cnt->GetSocket()
                           , [self, cnt](errorcode ec)
    {
        if (ec)
        {
            std::cerr << ec.message() << std::endl;
        }
        else
        {
            cnt->StartRead();
        }

        self->do_accept();
    });
}

void Server::do_stop()
{
    std::cout << "\r\nReceives an instruction to stop the service." << std::endl;

    _stopping = true;
    _acceptor.get_executor().context().stop();
    _ioc_pool_for_connections.Stop();
}

}//namespace da4qi4
