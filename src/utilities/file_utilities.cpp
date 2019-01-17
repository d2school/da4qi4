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

std::pair<bool, std::string /*msg*/>
CopyFile(fs::path const& src, fs::path const& dst, FileOverwriteOptions overwrite)
{
    if (!IsFileExists(src))
    {
        return std::make_pair(false, "Source file is not exists.");
    }

    bool dst_exists = IsFileExists(dst);

    if (dst_exists && overwrite != FileOverwriteOptions::overwrite)
    {
        return std::make_pair(overwrite == FileOverwriteOptions::ignore_success
                              , "Destination was exists.");
    }

    errorcode ec;
    fs::copy_file(src, dst, fs::copy_option::overwrite_if_exists, ec);

    if (ec)
    {
        return std::make_pair(false, ec.message());
    }

    return std::make_pair(true, "");
}

std::pair<bool, std::string /*msg*/>
MoveFile(fs::path const& src, fs::path const& dst, FileOverwriteOptions overwrite)
{
    if (!IsFileExists(src))
    {
        return std::make_pair(false, "Sourse is not exists.");
    }

    bool dst_exists = IsFileExists(dst);

    if (dst_exists && overwrite != FileOverwriteOptions::overwrite)
    {
        return std::make_pair(overwrite == FileOverwriteOptions::ignore_success
                              , "Destination is exists.");
    }

    errorcode ec;
    fs::rename(src, dst, ec);

    if (ec)
    {
        return std::make_pair(false, ec.message());
    }

    return std::make_pair(true, "");
}

} //namesapce Utilities
}//namespace da4qi4
