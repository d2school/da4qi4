#ifndef DAQI_ASIO_UTILITIES_HPP
#define DAQI_ASIO_UTILITIES_HPP

#include <string>
#include <vector>
#include <functional>

#include "daqi/def/asio_def.hpp"
#include "daqi/def/boost_def.hpp"

namespace da4qi4
{
namespace Utilities
{

Tcp::endpoint make_endpoint(char const* host, unsigned short port);
Tcp::endpoint make_endpoint(std::string const& host, unsigned short port);

std::vector<Tcp::endpoint> from_http_host_sync(std::string const& host, IOC& ioc
                                               , std::string const& service //http ? https?
                                               , std::string& exception);

using HostResolveHandler = std::function<void (errorcode const& ec,  Tcp::resolver::results_type)>;
void from_host(std::string const& host
               , std::string const& service //http ? https?
               , Tcp::resolver& resolver, HostResolveHandler handler);

} //namespace Utilities
} //namespace da4qi4


#endif // DAQI_ASIO_UTILITIES_HPP
