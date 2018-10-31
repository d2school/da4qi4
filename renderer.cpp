#include "renderer.hpp"

#include "inja/inja.hpp"

#include "def/debug_def.hpp"
#include "def/boost_def.hpp"
#include "utilities/string_utilities.hpp"

namespace da4qi4
{

namespace
{
std::string const daqi_HTML_template_ext = ".daqi.HTML";
std::string::size_type const len_of_daqi_HTML = daqi_HTML_template_ext.size();
}

void TemplateEngine::Preload()
{
    _templates.clear();

    fs::path root(_root);

    try
    {
        if (!fs::is_directory((root)) || !fs::exists(root))
        {
            return;
        }

        fs::recursive_directory_iterator iter(root);
        fs::recursive_directory_iterator end_iter;

        for (; iter != end_iter; ++iter)
        {
            if (fs::is_regular_file(*iter))
            {
                fs::path const& path = *iter;

                if (Utilities::EndsWith(path.filename().native(), daqi_HTML_template_ext))
                {
                    if (path.native().find("/i/") == std::string::npos
                        || path.native().find("\\i\\") == std::string::npos)
                    {
                        continue;
                    }

                    std::string::size_type len = path.size();
                    std::string::size_type root_len = root.size();
                    std::string key = path.native().substr(root_len, len - root_len - len_of_daqi_HTML);

                    if (!key.empty())
                    {
                        try
                        {
                            std::string tmpl_src = _env.load_global_file(key + daqi_HTML_template_ext);

                            if (!tmpl_src.empty())
                            {
                                inja::Template templ = _env.parse(tmpl_src);
                                _templates.insert(std::make_pair(key, templ));
                            }
                        }
                        catch (std::exception const& e)
                        {
                            std::cerr << e.what() << std::endl;
                        }

                    }
                }
            }
        }
    }
    catch (fs::filesystem_error const& ec)
    {
        std::cerr << ec.what() << std::endl;
    }
}

inja::Template const* TemplateEngine::Get(std::string const& name)
{
    auto it = _templates.find(name);

    if (it == _templates.end())
    {
        return nullptr;
    }

    return &(it->second);
}


}
