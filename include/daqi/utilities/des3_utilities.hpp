#ifndef DAQI_DES3_UTILITIES_HPP
#define DAQI_DES3_UTILITIES_HPP

#include <string>

namespace da4qi4
{
namespace Utilities
{

std::string DESEncrypt(std::string const& clearText, std::string const& key);
std::string DESDecrypt(std::string const& cipherText, std::string const& key);

} // namespace Utilities
} // namespace da4qi4

#endif // DES3_UTILITIES_HPP
