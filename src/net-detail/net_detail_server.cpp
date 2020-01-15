#include "daqi/net-detail/net_detail_server.hpp"

namespace da4qi4
{
namespace net_detail
{

SocketInterface::~SocketInterface() {}
Socket::~Socket() {}
SocketWithSSL::~SocketWithSSL() {}

template <typename T>
IOC& get_io_context_from(T& socket_obj)
{
#ifdef HAS_IO_CONTEXT
    return socket_obj.get_executor().get_io_context();
#else
    return socket_obj.get_io_service();
#endif
}

IOC& Socket::get_ioc()
{
    return get_io_context_from(_socket);
}

IOC& SocketWithSSL::get_ioc()
{
    return get_io_context_from(_stream);
}

} //namespace net_detail
} // namespace da4qi4
