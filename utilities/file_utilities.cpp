#include "file_utilities.hpp"

#include <fstream>

#include "def/debug_def.hpp"
#include "def/boost_def.hpp"

namespace da4qi4
{
namespace Utilities
{

bool SaveDataToFile(std::string const& data, std::string const& dir, std::string const& filename)
{
    errorcode ec;

    if (fs::create_directories(fs::path(dir), ec))
    {
        fs::path temp_fn = fs::path(dir) / filename;
        std::ofstream ofs(temp_fn.c_str(), std::ios_base::binary);

        if (!ofs)
        {
            return false;
        }

        ofs.write(data.data(), data.size());
        ofs.close();
        return true;
    }
    else
    {
        std::cerr << ec.message() << std::endl;
    }

    return false;
}

} //namesapce Utilities
}//namespace da4qi4
