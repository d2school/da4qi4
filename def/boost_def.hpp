#ifndef FILESYSTEM_DEF_HPP
#define FILESYSTEM_DEF_HPP

#include <string>

#include <boost/optional.hpp>
#include <boost/filesystem.hpp>


namespace da4qi4
{

extern boost::none_t NoneObject;

using errorcode = boost::system::error_code;
namespace fs = boost::filesystem;

using OptionalString = boost::optional<std::string>;
using OptionalStringRef = boost::optional<std::string&>;
using OptionalStringRefConst = boost::optional<std::string const&>;

} //namesapce da4qi4

#endif // FILESYSTEM_DEF_HPP
