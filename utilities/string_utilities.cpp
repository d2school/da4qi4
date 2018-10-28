#include "string_utilities.hpp"

#include <boost/algorithm/string/predicate.hpp>

namespace da4qi4
{
namespace Utilities
{

std::string theEmptyString;

bool IgnoreCaseCompare::operator()(std::string const& l, std::string const& r) const
{
    return iLess(l, r);
}

bool iStartsWith(std::string const& m, std::string const& s)
{
    return boost::algorithm::istarts_with(m, s);
}

bool iEquals(std::string const& l, std::string const& r)
{
    return boost::algorithm::iequals(l, r);
}

bool iLess(std::string const& l, std::string const& r)
{
    return boost::algorithm::ilexicographical_compare(l, r);
}


} //namespace Utilities
} //namespace da4qi4

