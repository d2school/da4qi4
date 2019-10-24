#ifndef DAQI_TEMPLATES_HPP
#define DAQI_TEMPLATES_HPP

#include <string>
#include <mutex>
#include <vector>
#include <unordered_map>

#include "daqi/def/inja_def.hpp"
#include "daqi/def/log_def.hpp"

namespace da4qi4
{
void init_template_env(inja::Environment& env);

using TemplatesEnv = inja::Environment;

class Templates
{
public:
    Templates() = default;

    Templates(std::string const& template_root, std::string const& app_url_root)
        : _root(template_root), _app_prefix(app_url_root)
    {
    }

    void InitPathes(std::string const& template_root, std::string const& app_url_root)
    {
        _root = template_root;
        _app_prefix = app_url_root;
    }

    bool Preload(log::LoggerPtr app_logger);

    TemplatePtr const Get(std::string const& name);

    void CopyIncludeTemplateTo(TemplatesEnv& env);

    bool ReloadIfFindUpdate();
    bool ReloadIfFindNew();

    std::string const& GetRoot() const
    {
        return _root;
    }

private:
    bool reload();

private:
    enum class TemplateFlag {for_normal, for_include};

    std::pair<size_t, size_t>
    load_templates(std::string const& template_ext, std::string const& key_ext);

    std::pair<size_t, size_t>
    load_templates(TemplatesEnv& env
                   , std::string const& template_ext
                   , std::string const& key_ext
                   , TemplateFlag flag);

    bool try_load_template(TemplatesEnv& env
                           , std::string const& key
                           , std::string const& template_filename
                           , std::string const& full_template_filename
                           , bool is_include_dir) noexcept;

    enum class TemplateUpdateAction
    {
        none, appended, modified, removed
    };

    TemplateUpdateAction check_exists_template();

    TemplateUpdateAction check_new_template(std::string const& template_ext
                                            , std::string const& key_ext);

    void hint_template_updated_found(TemplateUpdateAction action);
    void hint_template_reload_fail();

    struct Item
    {
        TemplatePtr templ;
        std::string filename;
    };

    TemplateUpdateAction check_exists_template(std::unordered_map<std::string, Item> const& templates);

private:
    std::mutex _m;  // for _templates reload and get
    log::LoggerPtr _app_logger;

    std::time_t _loaded_time;
    std::unordered_map<std::string, Item> _templates;
    std::unordered_map<std::string, Item> _includes_templates;

    std::string _root, _app_prefix;
};

} //namesapce da4qi4

#endif // DAQI_TEMPLATE_LIBRARY_HPP
