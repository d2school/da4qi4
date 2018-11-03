#include "utilities/asio_utilities.hpp"

namespace da4qi4
{
namespace Utilities
{

Tcp::endpoint make_endpoint(char const* host, unsigned short port)
{
    boost::asio::ip::address adr = boost::asio::ip::address::from_string(host);
    return Tcp::endpoint(adr, port);
}

Tcp::endpoint make_endpoint(std::string const& host, unsigned short port)
{
    return make_endpoint(host.c_str(), port);
}

} //namespace Utilities
} //namespace da4qi4
