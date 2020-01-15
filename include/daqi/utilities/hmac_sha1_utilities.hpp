#ifndef DAQI_HMAC_SHA1_UTILITIES_HPP
#define DAQI_HMAC_SHA1_UTILITIES_HPP

#include <cstdint>
#include <vector>
#include <string>

namespace da4qi4
{
namespace Utilities
{

std::vector<std::uint8_t> HMA_SHA1(std::string const& key, std::string const& src);
std::vector<std::uint8_t> HMA_SHA1(char const* key, int key_size, std::string const& src);
std::vector<std::uint8_t> HMA_SHA1(char const* key, int key_size, char const* src,  std::size_t src_size);

std::vector<std::uint8_t> SHA1(std::string const& src);
std::vector<std::uint8_t> SHA1(unsigned char const*& src, std::size_t src_size);
std::vector<std::uint8_t> SHA1(std::vector<unsigned char> const& src);

} // namespace Utilities
} // namespace da4qi4

#endif // DAQI_HMAC_SHA1_UTILITIES_HPP
