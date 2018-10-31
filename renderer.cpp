#include "renderer.hpp"

#include "def/boost_def.hpp"

namespace da4qi4
{

size_t TemplateLibrary::Preload()
{
    fs::path root(_root);
    errorcode ec;

    try
    {
        if (!fs::is_directory((root)) || !fs::exists(root))
        {
            return 0;
        }

        fs::recursive_directory_iterator iter(root);
        fs::recursive_directory_iterator end_iter;

        for (; iter != end_iter; ++iter)
        {
            if (fs::is_regular_file(*iter))
            {
                if (fs::extension(*iter) == ".HTML")
                {
                    ;
                }
            }
        }
    }
    catch (fs::filesystem_error const& ec)
    {

    }
}

inja::Template const* TemplateLibrary::Get(std::string const& name)
{
    return nullptr;
}


}
