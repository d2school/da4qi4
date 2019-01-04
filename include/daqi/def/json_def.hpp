#ifndef DAQI_JSON_DEF_HPP
#define DAQI_JSON_DEF_HPP

#include "nlohmann/json.hpp"

#include <ctime>

namespace da4qi4
{

using Json = nlohmann::json;
extern Json theEmptyJson;

namespace Valuetool
{

extern double const zero_compare_value;

bool is_exists(Json const& j, std::string const& item_name);
bool is_exists(Json const& j, std::vector<std::string>const& name_pathes);

bool boolean_value(Json const& j, bool default_value = false);
bool boolean_value(Json const& j, std::string const& item_name, bool default_value = false);
bool boolean_value(Json const& j, std::vector<std::string>const& name_pathes, bool default_value = false);

int integer_value(Json const& j, int default_value = 0);
int integer_value(Json const& j, std::string const& item_name, int default_value = 0);
int integer_value(Json const& j, std::vector<std::string>const& name_pathes, int default_value = 0);

std::size_t size_value(Json const& j, std::size_t default_value = 0);
std::size_t size_value(Json const& j, std::string const& item_name, std::size_t default_value = 0);
std::size_t size_value(Json const& j, std::vector<std::string>const& name_pathes, std::size_t default_value = 0);

std::time_t time_value(Json const& j, std::time_t default_value = 0);
std::time_t time_value(Json const& j, std::string const& item_name, std::time_t default_value = 0);
std::time_t time_value(Json const& j, std::vector<std::string>const& name_pathes, std::time_t default_value = 0);

double double_value(Json const& j, double default_value = 0.0);
double double_value(Json const& j, std::string const& item_name, double default_value = 0.0);
double double_value(Json const& j, std::vector<std::string>const& name_pathes, double default_value = 0.0);

std::string string_value(Json const& j);
std::string string_value(Json const& j, std::string const& item_name
                         , std::string const& default_value = "");
std::string string_value(Json const& j, std::vector<std::string>const& name_pathes,
                         std::string const& default_value = "");

} // namespace Valuetool


} //namespace da4qi4

#endif // DAQI_JSON_DEF_HPP
