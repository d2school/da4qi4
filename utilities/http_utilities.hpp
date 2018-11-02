#ifndef DAQI_HTTP_UTILITIES_HPP
#define DAQI_HTTP_UTILITIES_HPP

#include <string>
#include <map>

namespace da4qi4
{
namespace Utilities
{

bool IsUrlEncoded(const std::string& value);
std::string UrlEncode(const std::string& value);
std::string UrlDecode(const std::string& value);
std::string UrlDecodeIfEncoded(std::string const& value);

std::string DecIntToHexStr(std::size_t num);

std::map<std::string, std::string> ParseQueryParameters(std::string const& query);
std::map<std::string, std::string> ParsePlainTextFormData(std::string const& body); //from html 4.01

} //Utilities
} //da4qi4

#endif // DAQI_HTTP_UTILITIES_HPP
