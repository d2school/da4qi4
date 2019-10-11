#include "daqi/utilities/http_utilities.hpp"

#include "daqi/utilities/string_utilities.hpp"

namespace da4qi4
{
namespace Utilities
{


bool IsUrlEncoded(const std::string& value)
{
    for (auto c : value)
    {
        if (c == '%' || c == '+')
        {
            return true;
        }
    }
    
    return false;
}

std::string UrlDecodeIfEncoded(std::string const& value)
{
    return IsUrlEncoded(value) ? UrlDecode(value) : value;
}


std::string UrlEncode(const std::string& value)
{
    static auto hex_digt = "0123456789ABCDEF";
    
    std::string result;
    result.reserve(value.size() << 1);
    
    for (auto ch : value)
    {
        if ((ch >= '0' && ch <= '9')
            || (ch >= 'A' && ch <= 'Z')
            || (ch >= 'a' && ch <= 'z')
            || ch == '-' || ch == '_' || ch == '!'
            || ch == '\'' || ch == '(' || ch == ')'
            || ch == '*' || ch == '~' || ch == '.')  // !'()*-._~
        {
            result.push_back(ch);
        }
        else
        {
            result += std::string("%") +
                      hex_digt[static_cast<unsigned char>(ch) >> 4]
                      +  hex_digt[static_cast<unsigned char>(ch) & 15];
        }
    }
    
    return result;
}

std::string UrlDecode(const std::string& value)
{
    std::string result;
    result.reserve(value.size());
    
    for (std::size_t i = 0; i < value.size(); ++i)
    {
        auto ch = value[i];
        
        if (ch == '%' && (i + 2) < value.size())
        {
            auto hex = value.substr(i + 1, 2);
            auto dec = static_cast<char>(std::strtol(hex.c_str(), nullptr, 16));
            result.push_back(dec);
            i += 2;
        }
        else if (ch == '+')
        {
            result.push_back(' ');
        }
        else
        {
            result.push_back(ch);
        }
    }
    
    return result;
}

std::map<std::string, std::string> ParseQueryParameters(std::string const& query)
{
    std::map<std::string, std::string> result;
    std::vector<std::string> kvparts = Utilities::Split(query, '&', Utilities::TrimOptions::keep_space);
    
    for (auto kvpart : kvparts)
    {
        std::vector<std::string> kv = Utilities::Split(kvpart, '=', Utilities::TrimOptions::keep_space);
        size_t  c = kv.size();
        std::string k, v;
        
        switch (c)
        {
        case 2 :
            v = UrlDecodeIfEncoded(Utilities::TrimCopy(kv[1]));
            
            /* don't break; */
            
        case 1 :
            k = UrlDecodeIfEncoded(Utilities::TrimCopy(kv[0]));
            result.insert(std::make_pair(std::move(k), std::move(v)));
        }
    }
    
    return result;
}

std::map<std::string, std::string> ParsePlainTextFormData(std::string const& body)
{
    std::map<std::string, std::string> result;
    std::vector<std::string> kvparts = Utilities::SplitByLine(body
                                                              , Utilities::TrimOptions::keep_space);
    
    for (std::string kvpart : kvparts)
    {
        Utilities::Trim(kvpart);
        
        if (kvpart.empty())
        {
            continue;
        }
        
        std::vector<std::string> kv = Utilities::Split(kvpart
                                                       , '='
                                                       , Utilities::TrimOptions::keep_space);
        
        size_t  c = kv.size();
        std::string k, v;
        
        switch (c)
        {
        case 2 :
            v = Utilities::TrimCopy(kv[1]);
            
            /* don't break; */
            
        case 1 :
            k = Utilities::TrimCopy(kv[0]);
            result.insert(std::make_pair(std::move(k), std::move(v)));
        }
    }
    
    return result;
}


} //Utilities
} //da4qi4
