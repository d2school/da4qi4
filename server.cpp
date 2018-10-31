#include  "server.hpp"
#include "connection.hpp"

#include "def/boost_def.hpp"

namespace da4qi4
{

Server::Ptr Server::Supply(boost::asio::io_context& ioc, short port)
{
    return Ptr(new Server(ioc, port));
}

Server::Server(boost::asio::io_context& ioc, short port)
    : _acceptor(ioc, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
{
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

Application* Server::PrepareApp(std::string const& r)
{
    make_default_app_if_need();
    auto app = AppMgr()->FindByURL(r);

    if (!app)
    {
        std::cerr << "no found a app for URL. " << r << std::endl;
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

void Server::Start()
{
    make_default_app_if_need();
    this->do_accept();
}

void Server::do_accept()
{
    auto self = this->shared_from_this();

    this->_acceptor.async_accept(
                [self](errorcode ec, boost::asio::ip::tcp::socket socket)
    {
        if (ec)
        {
            std::cerr << ec.message() << std::endl;
        }
        else
        {
            //todo :make cnt have owner socket with a io_context
            auto cnt = std::make_shared<Connection>(std::move(socket));
            cnt->Start();
        }

        self->do_accept();
    });
}

}//namespace da4qi4
