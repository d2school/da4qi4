#include "daqi/net-detail/net_detail_server.hpp"

namespace da4qi4
{
namespace net_detail
{

SocketInterface::~SocketInterface() {}

IOC& SocketInterface::get_ioc()
{
    return _ioc;
}

Socket::~Socket() {}
SocketWithSSL::~SocketWithSSL() {}

} //namespace net_detail
} // namespace da4qi4
