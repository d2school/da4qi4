#ifndef DAQI_HTML_UTILITIES_HPP
#define DAQI_HTML_UTILITIES_HPP

#include <string>
#include <map>
#include <unordered_map>

namespace da4qi4
{
namespace Utilities
{

std::string const& GetMIMEType(std::string const&  extension);
std::string HTMLEscape(std::string const& s);

} //Utilities
} //da4qi4

#endif // DAQI_HTML_UTILITIES_HPP
