#ifndef DAQI_STRING_UTILITIES_HPP
#define DAQI_STRING_UTILITIES_HPP

#include <ctime>
#include <string>
#include <vector>
#include <map>

#include "daqi/utilities/base64.hpp"

namespace da4qi4
{
namespace Utilities
{

extern std::string theEmptyString;

std::string GMTFormatTime(std::time_t t);

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

struct CompareDESC
{
    bool operator()(std::string const& l, std::string const& r) const
    {
        return r < l; /* DESC : swap l, r */
    }
};

bool iStartsWith(std::string const& m, std::string const& s);
bool iEquals(std::string const& l, std::string const& r);
bool iLess(std::string const& l, std::string const& r);
bool iEndsWith(std::string const& m, std::string const& s);

bool StartsWith(std::string const& m, std::string const& s);
bool EndsWith(std::string const& m, std::string const& s);

std::string ReplaceAll(std::string const& m, std::string const& bef, std::string const& aft);
std::vector<std::string> Split(std::string const& m, char c);
std::vector<std::string> SplitByLine(std::string const& m); // \n or \r\n
void Trim(std::string& m);
std::string TrimCopy(std::string const& m);

std::string GetUUID(std::string const& prefix = theEmptyString);

} //namespace Utilities
} //namespace da4qi4

#endif // DAQI_STRING_UTILITIES_HPP
