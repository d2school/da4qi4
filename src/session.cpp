#include "daqi/session.hpp"

namespace da4qi4
{

Json ToJson(Cookie const& cookie, Json const& data)
{
    Json node;
    node["cookie"] = cookie;
    node["data"] = data;

    return node;
}

bool FromJson(Json const& node, Cookie& cookie, Json& data)
{
    try
    {
        cookie = node.at("cookie");
        data = node.at("data");
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
