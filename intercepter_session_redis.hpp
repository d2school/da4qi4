#ifndef INTERCEPTER_SESSION_HPP
#define INTERCEPTER_SESSION_HPP

#include <string>

#include "nlohmann_json/json.hpp"

namespace da4qi4
{

class RedisSession
{
public:

private:
    std::string _id;
    std::string _cookie_name;

};

}

#endif // INTERCEPTER_SESSION_HPP
