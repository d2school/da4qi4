#ifndef DAQI_CONTAINER_UTILITIES_HPP
#define DAQI_CONTAINER_UTILITIES_HPP

#include <string>

#include "daqi/def/def.hpp"
#include "daqi/def/boost_def.hpp"
#include "daqi/utilities/string_utilities.hpp"

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

#endif // DAQI_CONTAINER_UTILITIES_HPP

