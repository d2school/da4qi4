#include "utilities/asio_utilities.hpp"

namespace da4qi4 {
namespace Utilites {

Tcp::endpoint make_endpoint(char const* host, unsigned short port)
{
	boost::asio::ip::address adr = boost::asio::ip::address::from_string(host);
	return Tcp::endpoint(adr, port);
}

} //namespace Utilites
} //namespace da4qi4
