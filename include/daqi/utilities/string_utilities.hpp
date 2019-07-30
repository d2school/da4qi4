#ifndef DAQI_STRING_UTILITIES_HPP
#define DAQI_STRING_UTILITIES_HPP

#include <ctime>
#include <string>
#include <vector>
#include <map>

namespace da4qi4
{
namespace Utilities
{

extern std::string theEmptyString;

extern char const* dt_fmt_gmt;
extern char const* dt_fmt_yyyy_mm_dd_hh_mm_ss;
extern char const* dt_fmt_yyyy_mm_dd;
extern char const* dt_fmt_yyyy_mm_dd_hh_mm_ss_CN;
extern char const* dt_fmt_yyyy_mm_dd_CN;

std::string GMTFormatTime(std::time_t t);
std::string FormatDateTime(std::time_t t, char const* fmt = dt_fmt_yyyy_mm_dd_hh_mm_ss);

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

enum class TrimOptions {keep_space, trim_all, trim_left, trim_right};
std::vector<std::string> Split(std::string const& m, char c, TrimOptions opt = TrimOptions::keep_space);
std::vector<std::string> SplitByLine(std::string const& m, TrimOptions opt = TrimOptions::keep_space);

void Trim(std::string& m);
std::string TrimCopy(std::string const& m);

void TrimLeft(std::string& m);
std::string TrimLeftCopy(std::string const& m);

void TrimRight(std::string& m);
std::string TrimRightCopy(std::string const& m);

void TrimOnOptions(std::string& m, TrimOptions opt);
std::string TrimOnOptionsCopy(std::string const& m, TrimOptions opt);

std::string GetUUID(std::string const& prefix = theEmptyString);

} //namespace Utilities
} //namespace da4qi4

#endif // DAQI_STRING_UTILITIES_HPP
