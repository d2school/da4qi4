#include "daqi/utilities/asio_utilities.hpp"

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

std::vector<Tcp::endpoint> from_http_host_sync(std::string const& host
                                               , std::string const& service
                                               , IOC& ioc, std::string& exception)
{
    Tcp::resolver resolver(ioc);
    Tcp::resolver::query query(host, service);

    std::vector<Tcp::endpoint> result;

    try
    {
        auto it = resolver.resolve(query);

        if (it == Tcp::resolver::iterator())
        {
            return result;
        }

        for (; it != Tcp::resolver::iterator(); ++it)
        {
            result.emplace_back(*it);
        }
    }
    catch (std::exception const& e)
    {
        exception = e.what();
    }

    return result;
}

void from_host(std::string const& host,  std::string const& service,
               Tcp::resolver& resolver,
               HostResolveHandler handler)
{
    Tcp::resolver::query query(host, service);
    resolver.async_resolve(query, handler);
}

} //namespace Utilities
} //namespace da4qi4
