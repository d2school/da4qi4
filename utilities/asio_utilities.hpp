#ifndef DAQI_ASIO_UTILITIES_HPP
#define DAQI_ASIO_UTILITIES_HPP

#include <string>

#include "def/asio_def.hpp"

namespace da4qi4
{
namespace Utilities
{

Tcp::endpoint make_endpoint(char const* host, unsigned short port);
Tcp::endpoint make_endpoint(std::string const& host, unsigned short port);

} //namespace Utilities
} //namespace da4qi4


#endif // DAQI_ASIO_UTILITIES_HPP
