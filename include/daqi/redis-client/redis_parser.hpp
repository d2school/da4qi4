#ifndef DAQI_REDIS_PARSER_HPP
#define DAQI_REDIS_PARSER_HPP

#include <vector>
#include <stack>

#include <boost/variant.hpp>

#include "daqi/redis-client/redis_value.hpp"

namespace da4qi4
{

class RedisParser
{
public:
    RedisParser();

    enum ParseResult
    {
        Completed,
        Incompleted,
        Error,
    };

    std::pair<size_t, ParseResult> Parse(const char* ptr, size_t size);

    RedisValue Result();

protected:
    std::pair<size_t, ParseResult> parse_chunk(const char* ptr, size_t size);

    inline bool is_char(int c)
    {
        return c >= 0 && c <= 127;
    }

    inline bool is_control(int c)
    {
        return (c >= 0 && c <= 31) || (c == 127);
    }

    long int buf_to_long(const char* str, size_t size);

private:
    enum State
    {
        Start = 0,
        StartArray = 1,

        String = 2,
        StringLF = 3,

        ErrorString = 4,
        ErrorLF = 5,

        Integer = 6,
        IntegerLF = 7,

        BulkSize = 8,
        BulkSizeLF = 9,
        Bulk = 10,
        BulkCR = 11,
        BulkLF = 12,

        ArraySize = 13,
        ArraySizeLF = 14,
    };

    std::stack<State> states;

    long int bulkSize;
    std::vector<char> buf;
    RedisValue redisValue;

    // temporary variables
    std::stack<long int> arraySizes;
    std::stack<RedisValue> arrayValues;

    static const char stringReply = '+';
    static const char errorReply = '-';
    static const char integerReply = ':';
    static const char bulkReply = '$';
    static const char arrayReply = '*';
};

} //namesapce da4qi4

#endif // DAQI_REDIS_PARSER_HPP
