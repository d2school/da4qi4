#include "session.hpp"

namespace da4qi4
{

std::string SessionData::ToString() const
{
    Json root;
    root["c"] = cookie;
    root["d"] = data;

    int const indent = 4;
    return root.dump(indent);
}

bool SessionData::FromString(std::string const& str)
{
    try
    {
        Json root = Json::parse(str);
        root.at("c").get_to(cookie);
        data = root.at("d");
        return true;
    }
    catch (Json::parse_error const& e)
    {
        std::cerr << e.what() << std::endl; //HINT : can get more detail info from e.
    }
    catch (std::exception const& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return false;
}


} //namespace da4qi4
