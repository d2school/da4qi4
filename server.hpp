#ifndef SERVER_HPP
#define SERVER_HPP

#include "asio_def.hpp"

namespace da4qi4 {

class Server
{
public:
    Server(boost::asio::io_context& ioc, short port);
    void Start();

private:
    void do_accept();
private:
    Tcp::acceptor _acceptor;
};

}

#endif // SERVER_HPP
