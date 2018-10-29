#ifndef CONTAINER_UTILITIES_HPP
#define CONTAINER_UTILITIES_HPP

#include "def/def.hpp"
#include "def/boost_def.hpp"
#include "utilities/string_utilities.hpp"

namespace da4qi4
{
namespace Utilities
{

template<typename HeadersT>
bool IsExistsHeader(HeadersT const& headers, std::string const& field)
{
    auto const it = headers.find(field);
    return it != headers.cend();
}

template<typename HeadersT>
std::string const& GetHeader(HeadersT const& headers, std::string const& field)
{
    auto const it = headers.find(field);
    return (it != headers.cend() ? it->second : Utilities::theEmptyString);
}

template<typename HeadersT>
OptionalStringRefConst TryGetHeader(HeadersT const& headers, std::string const& field)
{
    auto const it = headers.find(field);
    return (it == headers.cend() ? OptionalStringRefConst(NoneObject)
            : OptionalStringRefConst(it->second));
}

} //namespace Utilities
} //namespace da4qi4

#endif // CONTAINER_UTILITIES_HPP

