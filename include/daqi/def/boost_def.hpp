#ifndef DAQI_BOOST_DEF_HPP
#define DAQI_BOOST_DEF_HPP

#include <string>

#include <boost/optional.hpp>
#include <boost/filesystem.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/asio/placeholders.hpp>

namespace da4qi4
{

extern boost::none_t NoneObject;
using errorcode = boost::system::error_code;
namespace fs = boost::filesystem;

using OptionalString = boost::optional<std::string>;
using OptionalStringRef = boost::optional<std::string&>;
using OptionalStringRefConst = boost::optional<std::string const&>;

namespace asio_placeholders = boost::asio::placeholders;

} //namesapce da4qi4

#endif // DAQI_BOOST_DEF_HPP
