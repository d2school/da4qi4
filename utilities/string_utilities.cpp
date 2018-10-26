#include "string_utilities.hpp"

#include <boost/algorithm/string/predicate.hpp>

namespace da4qi4
{
namespace Utilities
{

std::string const& EmptyString()
{
    static std::string _empty_string_;
    return _empty_string_;    
}

bool IgnoreCaseCompare::operator () (std::string const& l, std::string const& r) const
{
    return boost::algorithm::ilexicographical_compare(l, r);
}

bool iStartsWith(std::string const& m, std::string const& s)
{
    return boost::algorithm::istarts_with(m, s);
}


} //namespace Utilities
} //namespace da4qi4

