#include "daqi/utilities/string_utilities.hpp"

#include <iconv.h>

#include <ctime>
#include <codecvt>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

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

char const* dt_fmt_gmt = "%a, %d %b %Y %H:%M:%S %Z";
char const* dt_fmt_yyyy_mm_dd_hh_mm_ss = "%Y-%m-%d %H:%M:%S";
char const* dt_fmt_yyyy_mm_dd = "%Y-%m-%d";
char const* dt_fmt_yyyy_mm_dd_hh_mm_ss_CN = "%Y年%m月%d日 %H时%M分%S秒";
char const* dt_fmt_yyyy_mm_dd_CN = "%Y年%m月%d日";

std::string GMTFormatTime(std::time_t t)
{
    struct tm gmt = *std::gmtime(&t);
    char buff[48] = "\0";
    strftime(buff, sizeof(buff), dt_fmt_gmt, &gmt);

    return buff;
}

std::string FormatDateTime(std::time_t t, char const* fmt)
{
    struct tm gmt = *std::localtime(&t);
    char buff[48] = "\0";
    strftime(buff, sizeof(buff), fmt, &gmt);

    return buff;
}

std::string ReplaceAll(std::string const& m, std::string const& bef, std::string const& aft)
{
    return boost::algorithm::replace_all_copy(m, bef, aft);
}

void TrimOnOptions(std::string& m, TrimOptions opt)
{
    switch (opt)
    {
        case TrimOptions::trim_all :
            Trim(m);
            break;

        case TrimOptions::trim_left:
            TrimLeft(m);
            break;

        case TrimOptions::trim_right:
            TrimRight(m);
            break;

        case TrimOptions::keep_space:
            break;
    }
}

std::string TrimOnOptionsCopy(std::string const& m, TrimOptions opt)
{
    if (opt == TrimOptions::keep_space)
    {
        return m;
    }

    std::string mcopy = m;
    TrimOnOptions(mcopy, opt);
    return mcopy;
}

std::vector<std::string> Split(std::string const& m, char c, TrimOptions opt)
{
    std::vector<std::string> parts;
    char spe[] = {c, '\0'};
    boost::algorithm::split(parts, m, boost::is_any_of(spe));

    if (opt != TrimOptions::keep_space)
    {
        for (auto& part : parts)
        {
            TrimOnOptions(part, opt);
        }
    }

    return parts;
}

std::vector<std::string> SplitByLine(std::string const& m, TrimOptions opt)
{
    std::stringstream ss(m);
    std::vector<std::string> parts;

    while (ss)
    {
        std::string line;
        std::getline(ss, line);
        TrimOnOptions(line, opt);
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

void TrimLeft(std::string& m)
{
    boost::trim_left(m);
}

std::string TrimLeftCopy(std::string const& m)
{
    return boost::trim_copy(m);
}

void TrimRight(std::string& m)
{
    boost::trim_right(m);
}

std::string TrimRightCopy(std::string const& m)
{
    return boost::trim_right_copy(m);
}

std::string GetUUID(const std::string& prefix)
{
    boost::uuids::random_generator gen; //NOTE: too slow

    boost::uuids::uuid uid = gen();
    std::stringstream ss;
    ss << prefix << uid;

    return ss.str();
}

} //namespace Utilities
} //namespace da4qi4

