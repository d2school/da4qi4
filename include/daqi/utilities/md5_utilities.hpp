#ifndef DAQI_MD5_UTILITIES_HPP
#define DAQI_MD5_UTILITIES_HPP

#include <string>

namespace da4qi4
{
namespace Utilities
{

enum class MD5ResultEncoding
{
    raw, hex
};

std::string MD5(const std::string& src, MD5ResultEncoding encoding = MD5ResultEncoding::hex);

} //namespace Utilities
} //namespace da4qi4
#endif // DAQI_MD5_UTILITIES_HPP
