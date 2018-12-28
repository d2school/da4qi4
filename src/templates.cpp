#include "daqi/templates.hpp"

#include "inja/inja.hpp"

#include "daqi/def/log_def.hpp"
#include "daqi/def/boost_def.hpp"
#include "daqi/utilities/string_utilities.hpp"
#include "daqi/utilities/file_utilities.hpp"

namespace da4qi4
{

namespace
{
std::string const daqi_HTML_template_ext = ".daqi.HTML";
}

void init_template_env(inja::Environment& env)
{
    env.set_element_notation(inja::ElementNotation::Dot);

    auto placeholder_find = [](inja::Parsed::Arguments, inja::json)
    {
        return Utilities::theEmptyString;
    };
    auto placeholder_exists = [](inja::Parsed::Arguments, inja::json)
    {
        return false;
    };

    env.add_callback("_PARAMETER_", 1, placeholder_find);
    env.add_callback("_IS_PARAMETER_EXISTS_", 1, placeholder_exists);

    env.add_callback("_HEADER_", 1, placeholder_find);
    env.add_callback("_IS_HEADER_EXISTS_", 1, placeholder_exists);

    env.add_callback("_URL_PARAMETER_", 1, placeholder_find);
    env.add_callback("_IS_URL_PARAMETER_EXISTS_", 1, placeholder_exists);

    env.add_callback("_PATH_PARAMETER_", 1, placeholder_find);
    env.add_callback("_IS_PATH_PARAMETER_EXISTS_", 1, placeholder_exists);

    env.add_callback("_FORM_DATA_", 1, placeholder_find);
    env.add_callback("_IS_FORM_DATA_EXISTS_", 1, placeholder_exists);

    env.add_callback("_COOKIE_", 1, placeholder_find);
    env.add_callback("_IS_COOKIE_EXISTS_", 1, placeholder_exists);
}

bool Templates::try_load_template(std::string const& key
                                  , std::string const& template_filename
                                  , std::string const& full_template_filename)
{
    try
    {
        inja::Environment env(_root);
        init_template_env(env);

        std::string tmpl_src = env.load_global_file(template_filename);

        if (tmpl_src.empty())
        {
            _app_logger->error("Load template file {} fail.", full_template_filename);
            return false;
        }

        TemplatePtr templ = TemplatePtr(new Template(env.parse(tmpl_src)));
        Item item {templ, full_template_filename};
        _templates.insert(std::make_pair(key, item));

        _app_logger->info("Template {} loaded.", template_filename);

        return true;
    }
    catch (std::exception const& e)
    {
        std::cerr << e.what() << std::endl;
        return false;
    }
}

bool is_deprecated(fs::path const& path)
{
    return (path.string().find("_deprecated/") != std::string::npos
            || path.string().find("_deprecated\\") != std::string::npos
            || path.string().find(".deprecated.") != std::string::npos);
}

bool is_include_dir(fs::path const& path)
{
    return (path.native().find("/i/") != std::string::npos
            || path.native().find("\\i\\") != std::string::npos);
}

std::size_t Templates::load_templates(std::string const& template_ext, std::string const& key_ext)
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
                if (is_deprecated(path))
                {
                    continue;
                }

                if (is_include_dir(path))
                {
                    _includes.push_back(path.string());
                    continue;
                }

                std::string::size_type len = path.size();
                std::string::size_type root_len = root.size();
                std::string mpath = path.native().substr(root_len, len - root_len - template_ext.size());

                if (!mpath.empty())
                {
                    std::string template_filename = mpath + template_ext;
                    std::string key = _app_prefix + mpath + key_ext;

                    if (try_load_template(key, template_filename, path.string()))
                    {
                        ++count;
                    }
                }
            }
        }
    }

    return count;
}

bool Templates::reload()
{
    return (!_app_logger) ? false : Preload(_app_logger);
}

bool Templates::Preload(log::LoggerPtr app_logger)
{
    if (_app_logger != app_logger)
    {
        _app_logger = app_logger;
    }

    std::lock_guard<std::mutex> _guard_(_m);

    _templates.clear();
    _includes.clear();

    try
    {
        std::size_t count = load_templates(daqi_HTML_template_ext, Utilities::theEmptyString);
        _loaded_time = std::time(nullptr);
        app_logger->info("All ({}) template(s) loaded.", count);

        return true;
    }
    catch (fs::filesystem_error const& ec)
    {
        _app_logger->error("Template file \"{}\" load exception.", ec.what());
    }
    catch (std::exception const& ec)
    {
        _app_logger->error("Template file \"{}\" load exception.", ec.what());
    }
    catch (...)
    {
        _app_logger->error("Template file load unknown exception.");
    }

    return false;
}

TemplatePtr const  Templates::Get(std::string const& name)
{
    std::lock_guard<std::mutex> _guard_(_m);

    auto it = _templates.find(name);

    if (it == _templates.end())
    {
        return nullptr;
    }

    return it->second.templ;
}

void Templates::hint_template_updated_found(TemplateUpdateAction action)
{
    switch (action)
    {
        case TemplateUpdateAction::appended:
            _app_logger->info("Detects template appended.");
            break;

        case TemplateUpdateAction::modified:
            _app_logger->info("Detects template modified.");
            break;

        case TemplateUpdateAction::removed:
            _app_logger->info("Detects template removed.");
            break;

        default:
            break;
    }
}

void Templates::hint_template_reload_fail()
{
    _app_logger->error("Templates reload fail.");
}

bool Templates::ReloadIfFindUpdate()
{
    auto r = check_exists_template();

    if (r == TemplateUpdateAction::none)
    {
        return false;
    }


    hint_template_updated_found(r);

    if (reload())
    {
        return true;
    }

    hint_template_reload_fail();
    return false;
}

Templates::TemplateUpdateAction Templates::check_exists_template()
{
    TemplateUpdateAction action = TemplateUpdateAction::none;

    std::string modified_file;

    for (auto item : _templates)
    {
        auto fp = fs::path(item.second.filename);

        if (!Utilities::IsFileExists(fp))
        {
            modified_file = item.second.filename;
            action = TemplateUpdateAction::removed;
            break;
        }

        errorcode ec;
        std::time_t t = fs::last_write_time(fp, ec);

        if (ec)
        {
            _app_logger->warn("Check templates file \"{}\" last-write-time exception. {}",
                              item.second.filename, ec.message());
        }
        else if (t > _loaded_time)
        {
            modified_file = item.second.filename;
            action = TemplateUpdateAction::modified;
            break;
        }
    }

    if (modified_file.empty())
    {
        for (auto filename : _includes)
        {
            auto fp = fs::path(filename);
            errorcode ec;
            std::time_t t = fs::last_write_time(fp, ec);

            if (ec)
            {
                _app_logger->warn("Check templates include file \"{}\" last-write-time exception. {}",
                                  filename, ec.message());
            }
            else if (t > _loaded_time)
            {
                modified_file = filename;
                action = TemplateUpdateAction::modified;
                break;
            }
        }
    }

    return action;
}

bool Templates::ReloadIfFindNew()
{
    auto r = check_new_template(daqi_HTML_template_ext, Utilities::theEmptyString);

    if (r == TemplateUpdateAction::none)
    {
        return false;
    }

    hint_template_updated_found(r);

    if (reload())
    {
        return true;
    }

    hint_template_reload_fail();
    return false;
}

Templates::TemplateUpdateAction Templates::check_new_template(std::string const& template_ext,
                                                              std::string const& key_ext)
{
    fs::path root(_root);

    if (!fs::is_directory((root)) || !fs::exists(root))
    {
        return TemplateUpdateAction::none;
    }

    fs::recursive_directory_iterator iter(root);
    fs::recursive_directory_iterator end_iter;

    for (; iter != end_iter; ++iter)
    {
        if (fs::is_regular_file(*iter))
        {
            fs::path const& path = *iter;

            if (Utilities::EndsWith(path.filename().native(), template_ext))
            {
                if (is_deprecated(path))
                {
                    continue;
                }

                if (is_include_dir(path))
                {
                    std::lock_guard<std::mutex> _guard_(_m);
                    auto fd = std::find(_includes.cbegin(), _includes.cend(), path.string());

                    if (fd == _includes.cend())
                    {
                        return TemplateUpdateAction::appended;
                    }
                }

                std::string::size_type len = path.size();
                std::string::size_type root_len = root.size();
                std::string mpath = path.native().substr(root_len, len - root_len - template_ext.size());

                if (!mpath.empty())
                {
                    std::string key = _app_prefix + mpath + key_ext;

                    std::lock_guard<std::mutex> _guard_(_m);

                    if (_templates.find(key) == _templates.cend())
                    {
                        return TemplateUpdateAction::appended;
                    }
                }
            }
        }
    }

    return TemplateUpdateAction::none;
}


} //namespace da4qi4
