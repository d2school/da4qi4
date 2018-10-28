#ifndef SERVER_HPP
#define SERVER_HPP

#include "def/asio_def.hpp"

#include "application.hpp"
#include "handler.hpp"

namespace da4qi4
{

class Server
    : public std::enable_shared_from_this<Server>
{
    Server(boost::asio::io_context& ioc, short port);
public:
    using Ptr = std::shared_ptr<Server>;
    
    static Ptr Supply(boost::asio::io_context& ioc, short port);
    
public:
    void Start();
    void Stop();
    
public:
    bool AddApp(Application const& app)
    {
        return AppMgr()->Add(app);
    }
    
public:
    Application* AddHandler(HandlerMethod m, router_equals r, Handler h);
    Application* AddHandler(HandlerMethod m, router_starts r, Handler h);
    Application* AddHandler(HandlerMethod m, router_regex r, Handler h);
    
    Application* AddHandler(HandlerMethods ms, router_equals r, Handler h);
    Application* AddHandler(HandlerMethods ms, router_starts r, Handler h);
    Application* AddHandler(HandlerMethods ms, router_regex r, Handler h);
    
    Application* PrepareApp(std::string const& r);
    
private:
    void do_accept();
    
private:
    void make_default_app_if_need();
    
private:
    Tcp::acceptor _acceptor;
    
private:
};

}

#endif // SERVER_HPP
