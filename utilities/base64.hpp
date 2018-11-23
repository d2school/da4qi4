#ifndef DAQI_BASE64_HPP
#define DAQI_BASE64_HPP

#include <cstdint>
#include <string>

namespace da4qi4
{
namespace Utilities
{

std::string Base64Encode(std::uint8_t const* value, std::size_t len);
std::string Base64Encode(std::string const& value);
std::string Base64Decode(std::string const& base64);

} // namespace Utilities
} // namespace da4qi4

#endif // DAQI_BASE64_HPP
