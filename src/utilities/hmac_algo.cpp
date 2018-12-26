#include "utilities/hmac_algo.hpp"

#include <openssl/hmac.h>

#include <cassert>
#include <algorithm>

namespace da4qi4
{
namespace Utilities
{

std::vector<std::uint8_t> HMA_SHA1(std::string const& key, std::string const& src)
{
    return HMA_SHA1(key.c_str(), key.size(), src.c_str(), src.size());
}

std::vector<std::uint8_t> HMA_SHA1(char const* key, int key_size, std::string const& src)
{
    return HMA_SHA1(key, key_size, src.c_str(), src.size());
}

std::vector<std::uint8_t> HMA_SHA1(char const* key, int key_size,
                                   char const* src,  std::size_t src_size)
{
    unsigned int dst_size = 0;
    unsigned char dst_buf[20 * sizeof(std::uint8_t)]; //20 byte = 160bit

    HMAC(EVP_sha1(),
         key, key_size,
         reinterpret_cast<unsigned char const*>(src), src_size,
         dst_buf, &dst_size);

    std::vector<std::uint8_t> result;
    result.resize(dst_size);

    assert(dst_size <= 20);

    std::copy(dst_buf, dst_buf + dst_size, result.begin());

    return result;
}

} // namespace Utilities
} // namespace da4qi4
