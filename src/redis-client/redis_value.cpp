#include "daqi/redis-client/redis_value.hpp"

#include <string.h>

namespace da4qi4
{

RedisValue::RedisValue()
    : _value(NullTag()), _error(false)
{
}

RedisValue::RedisValue(RedisValue&& other)
    : _value(std::move(other._value)), _error(other._error)
{
}

RedisValue::RedisValue(int64_t i)
    : _value(i), _error(false)
{
}

RedisValue::RedisValue(const char* s)
    : _value(std::vector<char>(s, s + strlen(s))), _error(false)
{
}

RedisValue::RedisValue(std::string const& s)
    : _value(std::vector<char>(s.begin(), s.end())), _error(false)
{
}

RedisValue::RedisValue(std::vector<char> buf)
    : _value(std::move(buf)), _error(false)
{
}

RedisValue::RedisValue(std::vector<char> buf, struct ErrorTag)
    : _value(std::move(buf)), _error(true)
{
}

RedisValue::RedisValue(std::string const& custom_error, struct ErrorTag)
    : _value(std::vector<char>(custom_error.cbegin(), custom_error.cend()))
    , _error(true)
{

}

RedisValue::RedisValue(std::vector<RedisValue> array)
    : _value(std::move(array)), _error(false)
{
}

std::vector<RedisValue> RedisValue::ToArray() const
{
    return cast_to<std::vector<RedisValue>>();
}

std::string RedisValue::ToString() const
{
    const std::vector<char>& buf = ToByteArray();
    return std::string(buf.begin(), buf.end());
}

std::vector<char> RedisValue::ToByteArray() const
{
    return cast_to<std::vector<char>>();
}

int64_t RedisValue::ToInt() const
{
    return cast_to<int64_t>();
}

std::string RedisValue::Inspect() const
{
    if (IsError())
    {
        static std::string err = "_error: ";
        std::string result;

        result = err;
        result += ToString();

        return result;
    }
    else if (IsNull())
    {
        static std::string null = "(null)";
        return null;
    }
    else if (IsInt())
    {
        return std::to_string(ToInt());
    }
    else if (IsString())
    {
        return ToString();
    }
    else
    {
        std::vector<RedisValue> _values = ToArray();
        std::string result = "[";

        if (_values.empty() == false)
        {
            for (size_t i = 0; i < _values.size(); ++i)
            {
                result += _values[i].Inspect();
                result += ", ";
            }

            result.resize(result.size() - 1);
            result[result.size() - 1] = ']';
        }
        else
        {
            result += ']';
        }

        return result;
    }
}

bool RedisValue::IsOk() const
{
    return !IsError();
}

bool RedisValue::IsError() const
{
    return _error;
}

bool RedisValue::IsNull() const
{
    return type_eq<NullTag>();
}

bool RedisValue::IsInt() const
{
    return type_eq<int64_t>();
}

bool RedisValue::IsString() const
{
    return type_eq<std::vector<char>>();
}

bool RedisValue::IsByteArray() const
{
    return type_eq<std::vector<char>>();
}

bool RedisValue::IsArray() const
{
    return type_eq< std::vector<RedisValue>>();
}

std::vector<char>& RedisValue::GetByteArray()
{
    assert(IsByteArray());
    return boost::get<std::vector<char>>(_value);
}

const std::vector<char>& RedisValue::GetByteArray() const
{
    assert(IsByteArray());
    return boost::get<std::vector<char>>(_value);
}

std::vector<RedisValue>& RedisValue::GetArray()
{
    assert(IsArray());
    return boost::get<std::vector<RedisValue>>(_value);
}

const std::vector<RedisValue>& RedisValue::GetArray() const
{
    assert(IsArray());
    return boost::get<std::vector<RedisValue>>(_value);
}

bool RedisValue::operator == (const RedisValue& rhs) const
{
    return _value == rhs._value;
}

bool RedisValue::operator != (const RedisValue& rhs) const
{
    return !(_value == rhs._value);
}

} // namespace da4qi4
