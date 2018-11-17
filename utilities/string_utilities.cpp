#include "string_utilities.hpp"

#include <ctime>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

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

bool StartsWith(std::string const& m, std::string const& s)
{
    return boost::algorithm::starts_with(m, s);
}

bool iEndsWith(std::string const& m, std::string const& s)
{
    return boost::algorithm::iends_with(m, s);
}

bool EndsWith(std::string const& m, std::string const& s)
{
    return boost::algorithm::ends_with(m, s);
}

bool iEquals(std::string const& l, std::string const& r)
{
    return boost::algorithm::iequals(l, r);
}

bool iLess(std::string const& l, std::string const& r)
{
    return boost::algorithm::ilexicographical_compare(l, r);
}

std::string GMTFormatTime(std::time_t t)
{
    struct tm gmt = *std::gmtime(&t);
    char buff[30] = "\0";
    strftime(buff, sizeof(buff), "%a, %d %b %Y %H:%M:%S %Z", &gmt);
    return buff;
}

std::string ReplaceAll(std::string const& m, std::string const& bef, std::string const& aft)
{
    return boost::algorithm::replace_all_copy(m, bef, aft);
}

std::vector<std::string> Split(std::string const& m, char c)
{
    std::vector<std::string> parts;
    char spe[] = {c, '\0'};
    boost::algorithm::split(parts, m, boost::is_any_of(spe));
    return parts;
}

std::vector<std::string> SplitByLine(std::string const& m)
{
    std::stringstream ss(m);
    std::vector<std::string> parts;

    while (ss)
    {
        std::string line;
        std::getline(ss, line);
        parts.push_back(line);
    }

    return parts;
}

void Trim(std::string& m)
{
    boost::trim(m);
}

std::string TrimCopy(std::string const& m)
{
    return boost::trim_copy(m);
}

} //namespace Utilities
} //namespace da4qi4

