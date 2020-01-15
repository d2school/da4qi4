#include "daqi/utilities/md5_utilities.hpp"

#include <cstring>
#include <openssl/md5.h>

namespace da4qi4
{
namespace Utilities
{

void MD5(const std::string& srcStr, std::string& encodedStr, std::string& encodedHexStr)
{
    unsigned char mdStr[33] = {0};
    ::MD5(reinterpret_cast<const unsigned char*>(srcStr.c_str()), srcStr.length(), mdStr);

    encodedStr = std::string(reinterpret_cast<char*>(mdStr));

    char buf[65] = {0};
    char tmp[3] = {0};

    for (int i = 0; i < 32; i++)
    {
        sprintf(tmp, "%02x", mdStr[i]);
        strcat(buf, tmp);
    }

    buf[32] = '\0';
    encodedHexStr = std::string(buf);
}

std::string MD5(const std::string& src, MD5ResultEncoding encoding)
{
    std::string dst, dst_hex;
    MD5(src, dst, dst_hex);

    return (MD5ResultEncoding::hex == encoding) ? dst_hex : dst;
}

} //namespace Utilities
} //namespace da4qi4

