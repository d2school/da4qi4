#ifndef DAQI_TEMPLATES_HPP
#define DAQI_TEMPLATES_HPP

#include <string>
#include <mutex>
#include <vector>
#include <unordered_map>

#include "def/inja_def.hpp"
#include "def/log_def.hpp"

namespace da4qi4
{

class Templates
{
public:
    Templates(std::string const& template_root, std::string const& app_url_root)
        : _root(template_root), _app_prefix(app_url_root)
    {
        if (!_app_prefix.empty() && _app_prefix[0] == '/')
        {
            _app_prefix = _app_prefix.substr(1);
        }
    }

    bool Preload(log::LoggerPtr app_logger);

    TemplatePtr const Get(std::string const& name);

    bool ReloadIfUpdate();

private:
    bool reload();

private:
    size_t load_templates(std::string const& template_ext, std::string const& key_ext);
    bool try_load_template(std::string const& key
                           , std::string const& template_filename
                           , std::string const& full_template_filename);

    struct Item
    {
        TemplatePtr templ;
        std::string filename;
    };

    std::mutex _m;  // for _templates reload and get
    log::LoggerPtr _app_logger;

    std::time_t _loaded_time;
    std::unordered_map<std::string, Item> _templates;
    std::vector<std::string> _includes;

    std::string _root, _app_prefix;
};

} //namesapce da4qi4

#endif // DAQI_TEMPLATE_LIBRARY_HPP
