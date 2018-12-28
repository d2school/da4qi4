#include "daqi/utilities/file_utilities.hpp"

#include <fstream>

#include "daqi/def/debug_def.hpp"
#include "daqi/def/boost_def.hpp"

namespace da4qi4
{
namespace Utilities
{

bool SaveDataToFile(std::string const& data, std::string const& filename_with_path)
{
    fs::path f(filename_with_path);
    return SaveDataToFile(data, f);
}

bool SaveDataToFile(std::string const& data, fs::path const& filename_with_path)
{
    bool dir_exists = false;

    try
    {
        fs::path dir_path = filename_with_path.parent_path();
        dir_exists = (fs::exists(dir_path)) || fs::create_directories(dir_path);
    }
    catch (std::exception const& e)
    {
        std::cerr << e.what() << std::endl;
        return false;
    }

    if (!dir_exists)
    {
        return false;
    }

    std::ofstream ofs(filename_with_path.native().c_str(), std::ios_base::binary);

    if (!ofs)
    {
        return false;
    }

    ofs.write(data.data(), data.size());
    ofs.close();

    return true;
}

bool IsFileExists(fs::path const& fullpath)
{
    errorcode ec;
    bool exists_still = fs::exists(fs::status(fullpath, ec));
    return (!ec && exists_still);
}

bool IsFileExists(std::string const& fullpath)
{
    auto fp = fs::path(fullpath);
    return IsFileExists(fp);
}

} //namesapce Utilities
}//namespace da4qi4
