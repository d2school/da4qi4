#ifndef DAQI_ASIO_DEF_HPP
#define DAQI_ASIO_DEF_HPP

#include <boost/asio.hpp>

namespace da4qi4
{

#ifdef _USE_BOOST_VERSION_GE_1_66_
    #define HAS_IO_CONTEXT
    #define HAS_RESOLVER_RESULT
#endif

#ifdef HAS_IO_CONTEXT
    using IOC = boost::asio::io_context;
#else
    using IOC = boost::asio::io_service;
#endif

using Tcp = boost::asio::ip::tcp;

#ifdef HAS_RESOLVER_TYPE_RESULT
    typedef Tcp::resolver::results_type ResolverResultT;
#else
    typedef Tcp::resolver::iterator ResolverResultT;
#endif

}

#endif // DAQI_ASIO_DEF_HPP
