#ifndef DEF_HPP
#define DEF_HPP

#include <string>
#include <map>
#include <functional>
#include <memory>
#include <list>
#include <vector>
#include <set>

#include "utilities/string_utilities.hpp"

namespace da4qi4
{

using Headers = std::map<std::string, std::string>;
using ICHeaders = std::map<std::string, std::string, Utilities::IgnoreCaseCompare>;
using UrlParameters = std::map<std::string, std::string>;

struct UploadFileSaveOptions
{
    enum Strategy
    {
        always_save
        , alway_no_save
        , size_greater_than
        , size_lesser_than
        , extension_is
        , extension_is_not
    };

    bool IsNeedSave(std::string const& extension, size_t filesize_kb) const;

    Strategy strategy = always_save;
    size_t size_base_kb;
    std::set<std::string, Utilities::IgnoreCaseCompare> extensions;
};

} //namespace da4qi4

#endif // DEF_HPP
