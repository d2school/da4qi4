#ifndef DEF_HPP
#define DEF_HPP

#include <string>
#include <map>
#include <functional>
#include <memory>
#include <list>
#include <vector>

#include "utilities/string_utilities.hpp"

namespace da4qi4
{

using Headers = std::map<std::string, std::string>;
using ICHeaders = std::map<std::string, std::string, Utilities::IgnoreCaseCompare>;
using UrlParameters = std::map<std::string, std::string>;

}

#endif // DEF_HPP
