#include "templates.hpp"

#include "inja/inja.hpp"

#include "def/debug_def.hpp"
#include "def/boost_def.hpp"
#include "utilities/string_utilities.hpp"

namespace da4qi4
{

namespace
{
std::string const daqi_HTML_template_ext = ".daqi.HTML";
//std::string const daqi_PLAIN_template_ext = ".daqi.PLAIN";
//std::string const daqi_JSON_template_ext = ".daqi.JSON";
//std::string const daqi_XML_template_ext = ".daqi.XML";
}

void init_template_env(inja::Environment& env)
{
    env.set_element_notation(inja::ElementNotation::Dot);

    auto placeholder_find = [](inja::Parsed::Arguments, inja::json)
    {
        return Utilities::theEmptyString;
    };
    auto placeholder_exist = [](inja::Parsed::Arguments, inja::json)
    {
        return false;
    };

    env.add_callback("_PARAMETER_", 1, placeholder_find);
    env.add_callback("_IS_PARAMETER_EXISTS_", 1, placeholder_exist);

    env.add_callback("_HEADER_", 1, placeholder_find);
    env.add_callback("_IS_HEADER_EXISTS_", 1, placeholder_exist);

    env.add_callback("_URL_PARAMETER_", 1, placeholder_find);
    env.add_callback("_IS_URL_PARAMETER_EXISTS_", 1, placeholder_exist);

    env.add_callback("_PATH_PARAMETER_", 1, placeholder_find);
    env.add_callback("_IS_PATH_PARAMETER_EXISTS_", 1, placeholder_exist);

    env.add_callback("_FORM_DATA_", 1, placeholder_find);
    env.add_callback("_IS_FORM_DATA_EXISTS_", 1, placeholder_exist);

    env.add_callback("_COOKIE_", 1, placeholder_find);
    env.add_callback("_IS_COOKIE_EXISTS_", 1, placeholder_exist);
}

bool Templates::try_load_template(std::string const& key, std::string const& template_filename)
{
    try
    {
        inja::Environment env(_root);
        init_template_env(env);

        std::string tmpl_src = env.load_global_file(template_filename);

        if (tmpl_src.empty())
        {
            return false;
        }

        inja::Template templ = env.parse(tmpl_src);
        _templates.insert(std::make_pair(key, templ));
        return true;
    }
    catch (std::exception const& e)
    {
        std::cerr << e.what() << std::endl;
        return false;
    }
}

size_t Templates::load_templates(std::string const& template_ext, std::string const& key_ext)
{
    fs::path root(_root);

    if (!fs::is_directory((root)) || !fs::exists(root))
    {
        return 0;
    }

    size_t count = 0;

    fs::recursive_directory_iterator iter(root);
    fs::recursive_directory_iterator end_iter;

    for (; iter != end_iter; ++iter)
    {
        if (fs::is_regular_file(*iter))
        {
            fs::path const& path = *iter;

            if (Utilities::EndsWith(path.filename().native(), template_ext))
            {
                if (path.native().find("/i/") != std::string::npos
                    || path.native().find("\\i\\") != std::string::npos)
                {
                    continue;
                }

                std::string::size_type len = path.size();
                std::string::size_type root_len = root.size();
                std::string mpath = path.native().substr(root_len, len - root_len - template_ext.size());

                if (!mpath.empty())
                {
                    std::string template_filename = mpath + template_ext;
                    std::string key = mpath + key_ext;

                    if (try_load_template(key, template_filename))
                    {
                        ++count;
                    }
                }
            }
        }
    }

    return count;
}

size_t Templates::Preload()
{
    _templates.clear();
    size_t count = 0;

    try
    {
        count += load_templates(daqi_HTML_template_ext, Utilities::theEmptyString);
        //        count += load_templates(daqi_PLAIN_template_ext, ".plain");
        //        count += load_templates(daqi_JSON_template_ext, ".json");
        //        count += load_templates(daqi_XML_template_ext, ".xml");
        return count;
    }
    catch (fs::filesystem_error const& ec)
    {
        std::cerr << ec.what() << std::endl;
    }
    catch (std::exception const& ec)
    {
        std::cerr << ec.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "unknown exception" << std::endl;
    }

    return 0;
}

inja::Template const* Templates::Get(std::string const& name)
{
    std::string const* pname = &name;
    std::string tmp;

    if (Utilities::EndsWith(name, ".html"))
    {
        size_t const len_of_ext_html = 5;
        tmp = name.substr(0, name.size() - len_of_ext_html);
        pname = &tmp;
    }

    auto it = _templates.find(*pname);

    if (it == _templates.end())
    {
        return nullptr;
    }

    return &(it->second);
}

}
