#ifndef DAQI_APP_SETTING_HPP
#define DAQI_APP_SETTING_HPP

#include <string>
#include <set>

#include "utilities/string_utilities.hpp"

namespace da4qi4
{

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

}

#endif // DAQI_APP_SETTING_HPP
