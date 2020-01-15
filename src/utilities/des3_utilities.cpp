#include "daqi/utilities/des3_utilities.hpp"

#include <cstdlib>
#include <cstring>
#include <vector>

#include "openssl/des.h"

namespace da4qi4
{
namespace Utilities
{

std::string DESEncrypt(std::string const& clearText, std::string const& key)
{
    DES_cblock keyEncrypt;
    std::memset(keyEncrypt, 0, 8);

    if (key.length() <= 8)
    {
        std::memcpy(keyEncrypt, key.c_str(), key.length());
    }
    else
    {
        std::memcpy(keyEncrypt, key.c_str(), 8);
    }

    DES_key_schedule keySchedule;
    DES_set_key_unchecked(&keyEncrypt, &keySchedule);

    const_DES_cblock inputText;
    DES_cblock outputText;
    std::vector<unsigned char> vecCiphertext;
    unsigned char tmp[8];

    for (std::size_t i = 0; i < clearText.length() / 8; i++)
    {
        std::memcpy(inputText, clearText.c_str() + i * 8, 8);
        DES_ecb_encrypt(&inputText, &outputText, &keySchedule, DES_ENCRYPT);
        std::memcpy(tmp, outputText, 8);

        for (int j = 0; j < 8; j++)
        {
            vecCiphertext.push_back(tmp[j]);
        }
    }

    if (clearText.length() % 8 != 0)
    {
        std::size_t tmp1 = clearText.length() / 8 * 8;
        std::size_t tmp2 = clearText.length() - tmp1;
        std::memset(inputText, 0, 8);
        std::memcpy(inputText, clearText.c_str() + tmp1, tmp2);

        DES_ecb_encrypt(&inputText, &outputText, &keySchedule, DES_ENCRYPT);
        std::memcpy(tmp, outputText, 8);

        for (int j = 0; j < 8; j++)
        {
            vecCiphertext.push_back(tmp[j]);
        }
    }

    std::string cipherText;
    cipherText.reserve(vecCiphertext.size());
    cipherText.assign(vecCiphertext.begin(), vecCiphertext.end());

    return cipherText;
}

std::string DESDecrypt(std::string const& cipherText, std::string const& key)
{
    DES_cblock keyEncrypt;
    std::memset(keyEncrypt, 0, 8);

    if (key.length() <= 8)
    {
        std::memcpy(keyEncrypt, key.c_str(), key.length());
    }
    else
    {
        std::memcpy(keyEncrypt, key.c_str(), 8);
    }

    DES_key_schedule keySchedule;
    DES_set_key_unchecked(&keyEncrypt, &keySchedule);

    const_DES_cblock inputText;
    DES_cblock outputText;
    std::vector<unsigned char> vecCleartext;
    unsigned char tmp[8];

    for (std::size_t i = 0; i < cipherText.length() / 8; i++)
    {
        std::memcpy(inputText, cipherText.c_str() + i * 8, 8);
        DES_ecb_encrypt(&inputText, &outputText, &keySchedule, DES_DECRYPT);
        std::memcpy(tmp, outputText, 8);

        for (std::size_t j = 0; j < 8 && tmp[j]; j++)
        {
            vecCleartext.push_back(tmp[j]);
        }
    }

    if (cipherText.length() % 8 != 0)
    {
        std::size_t tmp1 = cipherText.length() / 8 * 8;
        std::size_t tmp2 = cipherText.length() - tmp1;

        std::memset(inputText, 0, 8);
        std::memcpy(inputText, cipherText.c_str() + tmp1, tmp2);
        DES_ecb_encrypt(&inputText, &outputText, &keySchedule, DES_DECRYPT);
        std::memcpy(tmp, outputText, 8);

        for (int j = 0; j < 8 && tmp[j]; j++)
        {
            vecCleartext.push_back(tmp[j]);
        }
    }

    std::string clearText;
    clearText.reserve(vecCleartext.size());
    clearText.assign(vecCleartext.begin(), vecCleartext.end());

    return clearText;
}

} // namespace Utilities
} // namespace da4qi4
