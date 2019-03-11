#include "daqi/def/json_def.hpp"

#include <string>
#include <vector>

namespace da4qi4
{

Json theNullJson;

namespace Valuetool
{

double const zero_compare_value = 0.0000000001;

bool is_exists(Json const& j, std::string const& item_name)
{
    return j.find(item_name) != j.cend();
}

bool is_exists(Json const& j, std::vector<std::string> const& name_pathes)
{
    if (!j.is_object())
    {
        return false;
    }

    if (name_pathes.empty())
    {
        return false;
    }

    auto parent = j.find(name_pathes[0]);

    if (parent == j.cend())
    {
        return false;
    }

    for (size_t i = 1; i < name_pathes.size(); ++i)
    {
        if (!parent->is_object())
        {
            return false;
        }

        auto child = parent->find(name_pathes[i]);

        if (child == parent->cend())
        {
            return false;
        }

        parent = child;
    }

    return true;
}

bool boolean_value(Json const& j, bool default_value)
{
    if (j.is_discarded())
    {
        return default_value;
    }

    if (j.is_boolean())
    {
        return j.get<bool>();
    }

    if (j.is_null())
    {
        return false;
    }

    if (j.is_array())
    {
        return !j.empty();
    }

    if (j.is_number_integer())
    {
        return j.get<std::size_t>() != 0;
    }

    if (j.is_number_float())
    {
        auto const d = j.get<double>();
        return (d < -zero_compare_value || d > zero_compare_value);
    }

    return default_value;
}

bool boolean_value(Json const& j, std::string const& item_name, bool default_value)
{
    if (item_name.empty())
    {
        return boolean_value(j, default_value);
    }

    if (!j.is_object())
    {
        return default_value;
    }

    auto it = j.find(item_name);

    if (it == j.cend())
    {
        return default_value;
    }

    return boolean_value(*it, default_value);
}

bool boolean_value(Json const& j, std::vector<std::string>const& name_pathes, bool default_value)
{
    if (name_pathes.empty())
    {
        return boolean_value(j, default_value);
    }

    if (!j.is_object())
    {
        return default_value;
    }

    auto parent = j.find(name_pathes[0]);

    if (parent == j.cend())
    {
        return default_value;
    }

    for (size_t i = 1; i < name_pathes.size(); ++i)
    {
        if (!parent->is_object())
        {
            return default_value;
        }

        auto child = parent->find(name_pathes[i]);

        if (child == parent->cend())
        {
            return default_value;
        }

        parent = child;
    }

    return boolean_value(*parent, default_value);
}

int integer_value(Json const& j, int default_value)
{
    if (j.is_discarded())
    {
        return default_value;
    }

    if (j.is_number_integer())
    {
        return j.get<int>();
    }

    if (j.is_null())
    {
        return 0;
    }

    if (j.is_boolean())
    {
        return (j.get<bool>()) ? 1 : 0;
    }

    if (j.is_number_float())
    {
        auto const d = j.get<double>();
        return static_cast<int>(d);
    }

    return default_value;
}

int integer_value(Json const& j, std::string const& item_name, int default_value)
{
    if (item_name.empty())
    {
        return integer_value(j, default_value);
    }

    if (!j.is_object())
    {
        return default_value;
    }

    auto it = j.find(item_name);

    if (it == j.cend())
    {
        return default_value;
    }

    return integer_value(*it, default_value);
}

int integer_value(Json const& j, std::vector<std::string>const& name_pathes, int default_value)
{
    if (name_pathes.empty())
    {
        return integer_value(j, default_value);
    }

    if (!j.is_object())
    {
        return default_value;
    }

    auto parent = j.find(name_pathes[0]);

    if (parent == j.cend())
    {
        return default_value;
    }

    for (size_t i = 1; i < name_pathes.size(); ++i)
    {
        if (!parent->is_object())
        {
            return default_value;
        }

        auto child = parent->find(name_pathes[i]);

        if (child == parent->cend())
        {
            return default_value;
        }

        parent = child;
    }

    return integer_value(*parent, default_value);
}

double double_value(Json const& j, double default_value)
{
    if (j.is_discarded())
    {
        return default_value;
    }

    if (j.is_number_float())
    {
        return j.get<double>();
    }

    if (j.is_null())
    {
        return 0.0;
    }

    if (j.is_boolean())
    {
        return (j.get<bool>()) ? 1 : 0;
    }

    if (j.is_number_integer())
    {
        auto const i = j.get<int>();
        return i;
    }

    return default_value;
}

double double_value(Json const& j, std::string const& item_name, double default_value)
{
    if (item_name.empty())
    {
        return double_value(j, default_value);
    }

    if (!j.is_object())
    {
        return default_value;
    }

    auto it = j.find(item_name);

    if (it == j.cend())
    {
        return default_value;
    }

    return double_value(*it, default_value);
}

double double_value(Json const& j, std::vector<std::string>const& name_pathes, double default_value)
{
    if (name_pathes.empty())
    {
        return double_value(j, default_value);
    }

    if (!j.is_object())
    {
        return default_value;
    }

    auto parent = j.find(name_pathes[0]);

    if (parent == j.cend())
    {
        return default_value;
    }

    for (size_t i = 1; i < name_pathes.size(); ++i)
    {
        if (!parent->is_object())
        {
            return default_value;
        }

        auto child = parent->find(name_pathes[i]);

        if (child == parent->cend())
        {
            return default_value;
        }

        parent = child;
    }

    return double_value(*parent, default_value);
}

std::string string_value(Json const& j)
{
    if (j.is_discarded())
    {
        return "";
    }

    if (j.is_string())
    {
        return j.get<std::string>();
    }

    if (j.is_number_float())
    {
        return std::to_string(j.get<double>());
    }

    if (j.is_null())
    {
        return "null";
    }

    if (j.is_boolean())
    {
        return (j.get<bool>()) ? "true" : "false";
    }

    if (j.is_number_integer())
    {
        auto const i = j.get<int>();
        return std::to_string(i);
    }

    return j.dump(2);
}

std::string string_value(Json const& j, std::string const& item_name, std::string const& default_value)
{
    if (item_name.empty())
    {
        return string_value(j);
    }

    if (!j.is_object())
    {
        return default_value;
    }

    auto it = j.find(item_name);

    if (it == j.cend())
    {
        return default_value;
    }

    return string_value(*it, default_value);
}

std::string string_value(Json const& j, std::vector<std::string>const& name_pathes,
                         std::string const& default_value)
{
    if (name_pathes.empty())
    {
        return string_value(j, default_value);
    }

    if (!j.is_object())
    {
        return default_value;
    }

    auto parent = j.find(name_pathes[0]);

    if (parent == j.cend())
    {
        return default_value;
    }

    for (size_t i = 1; i < name_pathes.size(); ++i)
    {
        if (!parent->is_object())
        {
            return default_value;
        }

        auto child = parent->find(name_pathes[i]);

        if (child == parent->cend())
        {
            return default_value;
        }

        parent = child;
    }

    return string_value(*parent, default_value);
}

std::size_t size_value(Json const& j, std::size_t default_value)
{
    if (j.is_discarded())
    {
        return default_value;
    }

    if (j.is_number_integer())
    {
        return j.get<std::size_t>();
    }

    if (j.is_null())
    {
        return 0;
    }

    if (j.is_boolean())
    {
        return (j.get<bool>()) ? 1 : 0;
    }

    if (j.is_number_float())
    {
        auto const d = j.get<double>();
        return static_cast<std::size_t>(d);
    }

    return default_value;
}

std::size_t size_value(Json const& j, std::string const& item_name, std::size_t default_value)
{
    if (item_name.empty())
    {
        return size_value(j, default_value);
    }

    if (!j.is_object())
    {
        return default_value;
    }

    auto it = j.find(item_name);

    if (it == j.cend())
    {
        return default_value;
    }

    return size_value(*it, default_value);
}

std::size_t size_value(Json const& j, std::vector<std::string>const& name_pathes, std::size_t default_value)
{
    if (name_pathes.empty())
    {
        return size_value(j, default_value);
    }

    if (!j.is_object())
    {
        return default_value;
    }

    auto parent = j.find(name_pathes[0]);

    if (parent == j.cend())
    {
        return default_value;
    }

    for (size_t i = 1; i < name_pathes.size(); ++i)
    {
        if (!parent->is_object())
        {
            return default_value;
        }

        auto child = parent->find(name_pathes[i]);

        if (child == parent->cend())
        {
            return default_value;
        }

        parent = child;
    }

    return size_value(*parent, default_value);
}


std::time_t time_value(Json const& j, std::time_t default_value)
{
    if (j.is_discarded())
    {
        return default_value;
    }

    if (j.is_number_integer())
    {
        return j.get<std::time_t>();
    }

    if (j.is_null())
    {
        return 0;
    }

    if (j.is_boolean())
    {
        return (j.get<bool>()) ? 1 : 0;
    }

    if (j.is_number_float())
    {
        auto const d = j.get<double>();
        return static_cast<std::time_t>(d);
    }

    return default_value;
}

std::time_t time_value(Json const& j, std::string const& item_name, std::time_t default_value)
{
    if (item_name.empty())
    {
        return time_value(j, default_value);
    }

    if (!j.is_object())
    {
        return default_value;
    }

    auto it = j.find(item_name);

    if (it == j.cend())
    {
        return default_value;
    }

    return time_value(*it, default_value);
}

std::time_t time_value(Json const& j, std::vector<std::string>const& name_pathes, std::time_t default_value)
{
    if (name_pathes.empty())
    {
        return time_value(j, default_value);
    }

    if (!j.is_object())
    {
        return default_value;
    }

    auto parent = j.find(name_pathes[0]);

    if (parent == j.cend())
    {
        return default_value;
    }

    for (size_t i = 1; i < name_pathes.size(); ++i)
    {
        if (!parent->is_object())
        {
            return default_value;
        }

        auto child = parent->find(name_pathes[i]);

        if (child == parent->cend())
        {
            return default_value;
        }

        parent = child;
    }

    return time_value(*parent, default_value);
}

} // namespace Valuetool

} //namespace da4qi4
