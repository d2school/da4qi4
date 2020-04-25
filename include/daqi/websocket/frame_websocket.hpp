//MIT
//copyright:  https://github.com/akzi/xwebsocket
//author:     https://github.com/akzi (fuwq, 82018309@qq.com, beijing, China)

#ifndef DAQI_WEBSOCKET_FRAME_HPP
#define DAQI_WEBSOCKET_FRAME_HPP

#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <functional>

namespace da4qi4
{
namespace Websocket
{

enum FrameType
{
    e_continuation = 0x00,
    e_text = 0x01,
    e_binary = 0x02,
    e_connection_close = 0x08,
    e_ping = 0x09,
    e_pong = 0xa
};

struct FrameHeader
{
    uint8_t FIN = 0; //是否是消息的结束帧(分片) 1位

    uint8_t RSV1 = 0;
    uint8_t RSV2 = 0;
    uint8_t RSV3 = 0;

    //   %x0 表示连续消息分片
    //   %x1 表示文本消息分片
    //   %x2 表未二进制消息分片
    //   %x3-7 为将来的非控制消息片断保留的操作码
    //   %x8 表示连接关闭
    //   %x9 表示心跳检查的ping
    //   %xA 表示心跳检查的pong
    //   %xB-F 为将来的控制消息片断的保留操作码

    FrameType OPCODE = e_continuation;

    uint8_t MASK = 0;
    uint8_t PAYLOAD_LEN = 0;
    uint16_t EXT_PAYLOAD_LEN_16 = 0; //extended payload length 16
    uint64_t EXT_PAYLOAD_LEN_64 = 0; //extended payload length 64
    uint64_t PAYLOAD_REALY_LEN = 0;
    uint32_t MASKING_KEY = 0;

    void Reset()
    {
        FIN = 0;
        RSV1 = RSV2 = RSV3 = 0;
        OPCODE =  e_continuation;
        MASK = 0;
        PAYLOAD_LEN = EXT_PAYLOAD_LEN_16 = EXT_PAYLOAD_LEN_64 = PAYLOAD_REALY_LEN = 0;
        MASKING_KEY = 0;
    }
};

class FrameBuilder
{
public:
    FrameBuilder() = default;

    FrameBuilder& ResetMaskingKey()
    {
        _frame_header.MASK = 0x00;
        return *this;
    }

    FrameBuilder& SetMaskingKey(uint32_t masking_key)
    {
        _frame_header.MASK = 0x80;
        _frame_header.MASKING_KEY = masking_key;
        return *this;
    }
    FrameBuilder& SetFIN(bool val)
    {
        _frame_header.FIN = val ? 0x80 : 0;
        return *this;
    }
    FrameBuilder& SetFrameType(FrameType type)
    {
        _frame_header.OPCODE = type;
        return *this;
    }

    std::string Build(std::string const& data)
    {
        return Build(data.c_str(), data.length());
    }
    std::string Build(const char* data, size_t len);
    std::string Build(const char* data)
    {
        size_t len = (data) ? std::strlen(data) : 0;
        return Build(data, len);
    }
    std::string Build()
    {
        return Build(nullptr, 0);
    }

private:
    FrameHeader _frame_header;
};

/*
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-------+-+-------------+-------------------------------+
    |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
    |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
    |N|V|V|V|       |S|             |   (if payload len==126/127)   |
    | |1|2|3|       |K|             |                               |
    +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
    |     Extended payload length continued, if payload len == 127  |
    + - - - - - - - - - - - - - - - +-------------------------------+
    |                               |Masking-key, if MASK set to 1  |
    +-------------------------------+-------------------------------+
    | Masking-key (continued)       |          Payload Data         |
    +-------------------------------- - - - - - - - - - - - - - - - +
    :                     Payload Data continued ...                :
    + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
    |                     Payload Data continued ...                |
    +---------------------------------------------------------------+
*/

class FrameParser
{
public:
    using MsgCallback = std::function < void (std::string &&, FrameType, bool) >;

    FrameParser()
    {
        reset();
    }

    ~FrameParser() = default;

    FrameParser(FrameParser&& parser)
    {
        move_reset(std::move(parser));
    }

    FrameParser& operator= (FrameParser&& parser)
    {
        move_reset(std::move(parser));
        return *this;
    }

    FrameParser& RegistMsgCallback(MsgCallback const& handle)
    {
        _msb_cb = handle;
        return *this;
    }

    std::pair<bool, std::string> Parse(void const* data, uint32_t len);

    void Reset()
    {
        reset();
    }
private:
    void reset();
    void move_reset(FrameParser&& parser);
    uint32_t parse_fixed_header(const char* data);
    uint32_t parse_payload_len(const char* data);
    uint32_t parse_extened_payload_len(const char* data, uint32_t len);
    void decode_extened_payload_len();
    uint32_t parse_masking_key(const char* data, uint32_t len);
    uint32_t parse_payload(const char* data, uint32_t len);

private:
    enum parser_step
    {
        e_fixed_header,
        e_payload_len,
        e_extened_payload_len,
        e_masking_key,
        e_payload_data,
    };

    parser_step _parser_step;

    //extended payload length write pos
    uint32_t _payload_len_offset;

    //masking key buffer pos
    uint32_t _masking_key_pos;

    std::string _payload;

    FrameHeader _frame_header;
    MsgCallback _msb_cb;
};

} // namespace Websocket
} // namespace da4qi4

#endif // DAQI_WEBSOCKET_FRAME_HPP
