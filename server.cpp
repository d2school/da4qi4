#include  "server.hpp"
#include "connection.hpp"

namespace da4qi4
{

Server::Server(boost::asio::io_context& ioc, short port)
    : _acceptor(ioc, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
{
}

void Server::Start()
{
    this->do_accept();
}

void Server::do_accept()
{
    this->_acceptor.async_accept(
        [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket)
    {
        if (!ec)
        {
            auto cnt = std::make_shared<Connection>(std::move(socket));
            cnt->Start();
        }
    });
}

}//namespace da4qi4
