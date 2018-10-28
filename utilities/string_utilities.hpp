#ifndef STRING_UTILITIES_HPP
#define STRING_UTILITIES_HPP

#include <string>
#include <map>

namespace da4qi4
{
namespace Utilities
{

extern std::string theEmptyString;

struct IgnoreCaseCompare
{
    bool operator()(std::string const& l, std::string const& r) const;
};

struct IgnoreCaseCompareDESC
{
    bool operator()(std::string const& l, std::string const& r) const
    {
        IgnoreCaseCompare compare;
        return compare(r, l); /* DESC : swap l, r */
    }
};

bool iStartsWith(std::string const& m, std::string const& s);
bool iEquals(std::string const& l, std::string const& r);
bool iLess(std::string const& l, std::string const& r);

} //namespace Utilities
} //namespace da4qi4

#endif // STRING_UTILITIES_HPP
