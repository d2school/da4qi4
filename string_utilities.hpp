#ifndef STRING_UTILITIES_HPP
#define STRING_UTILITIES_HPP

#include <string>
#include <map>

#include <boost/optional.hpp>

namespace da4qi4
{
namespace Utilities
{

std::string const& EmptyString();

struct IgnoreCaseCompare
{
    bool operator () (std::string const& l, std::string const& r) const;
};

using OptionalString = boost::optional<std::string>;
using OptionalStringRef = boost::optional<std::string&>;
using OptionalStringRefConst = boost::optional<std::string const&>;

bool iStartsWith(std::string const& m, std::string const& s);

} //namespace Utilities
} //namespace da4qi4

#endif // STRING_UTILITIES_HPP
