#include "app_setting.hpp"

namespace da4qi4
{

bool UploadFileSaveOptions::IsNeedSave(std::string const& extension, size_t filesize_kb) const
{
    if (strategy == always_save)
    {
        return true;
    }

    if (strategy == alway_no_save)
    {
        return false;
    }

    if (strategy == size_greater_than)
    {
        return filesize_kb > size_base_kb;
    }

    if (strategy == size_lesser_than)
    {
        return filesize_kb < size_base_kb;
    }

    if (strategy == extension_is)
    {
        return extensions.find(extension) != extensions.cend();
    }

    if (strategy == extension_is_not)
    {
        return extensions.find(extension) == extensions.cend();
    }

    return false;
}

} //namespace da4qi4
