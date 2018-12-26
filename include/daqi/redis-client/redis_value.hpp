#ifndef DAQI_REDIS_VALUE_HPP
#define DAQI_REDIS_VALUE_HPP

#include <string>
#include <stack>
#include <vector>
#include <utility>

#include <boost/variant.hpp>

namespace da4qi4 
{

class RedisValue {
public:
    struct ErrorTag {};

    RedisValue();
    RedisValue(RedisValue &&other);
    RedisValue(int64_t i);
    RedisValue(const char *s);
    RedisValue(const std::string &s);
    RedisValue(std::vector<char> buf);
    RedisValue(std::vector<char> buf, struct ErrorTag);
    RedisValue(std::string const& custom_error, struct ErrorTag);
    RedisValue(std::vector<RedisValue> array);


    RedisValue(const RedisValue &) = default;
    RedisValue& operator = (const RedisValue &) = default;
    RedisValue& operator = (RedisValue &&) = default;

    // Return the value as a std::string if
    // type is a byte string; otherwise returns an empty std::string.
    std::string ToString() const;

    // Return the value as a std::vector<char> if
    // type is a byte string; otherwise returns an empty std::vector<char>.
    std::vector<char> ToByteArray() const;

    // Return the value as a std::vector<RedisValue> if
    // type is an int; otherwise returns 0.
    int64_t ToInt() const;

    // Return the value as an array if type is an array;
    // otherwise returns an empty array.
    std::vector<RedisValue> ToArray() const;

    // Return the string representation of the value. Use
    // for dump content of the value.
    std::string Inspect() const;

    // Return true if value not a error
    bool IsOk() const;
    // Return true if value is a error
    bool IsError() const;

    // Return true if this is a null.
    bool IsNull() const;
    // Return true if type is an int
    bool IsInt() const;
    // Return true if type is an array
    bool IsArray() const;
    // Return true if type is a string/byte array. Alias for IsString();
    bool IsByteArray() const;
    // Return true if type is a string/byte array. Alias for IsByteArray().
    bool IsString() const;

    // Methods for increasing perfomance
    // Throws: boost::bad_get if the type does not match
    std::vector<char> &GetByteArray();
    const std::vector<char> &GetByteArray() const;
    std::vector<RedisValue> &GetArray();
    const std::vector<RedisValue> &GetArray() const;

    bool operator == (const RedisValue &rhs) const;
    bool operator != (const RedisValue &rhs) const;

protected:
    template<typename T>
     T cast_to() const;

    template<typename T>
    bool type_eq() const;

private:
    struct NullTag 
    {
        inline bool operator == (const NullTag &) const 
        {
            return true;
        }
    };

    boost::variant<NullTag, int64_t, std::vector<char>, std::vector<RedisValue> > _value;
    bool _error;
};


template<typename T>
T RedisValue::cast_to() const
{
    return (_value.type() == typeid(T))? boost::get<T>(_value) : T();
}

template<typename T>
bool RedisValue::type_eq() const
{
    return (_value.type() == typeid(T));
}

} // namespace da4qi4

#endif // DAQI_REDIS_VALUE_HPP
