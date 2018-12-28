#ifndef DAQI_FILE_UTILITIES_HPP
#define DAQI_FILE_UTILITIES_HPP

#include <string>
#include "daqi/def/boost_def.hpp"

namespace da4qi4
{
namespace Utilities
{

bool SaveDataToFile(std::string const& data, std::string const& filename_with_path);
bool SaveDataToFile(std::string const& data, fs::path const& filename_with_path);

bool IsFileExists(fs::path const& fullpath);
bool IsFileExists(std::string const& fullpath);

} //namesapce Utilities
} //namespace da4qi4

#endif // DAQI_FILE_UTILITIES_HPP
