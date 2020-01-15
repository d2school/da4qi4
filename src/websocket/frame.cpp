#include "daqi/websocket/frame.hpp"

#include <cstring>

namespace da4qi4
{
namespace Websocket
{

std::string FrameBuilder::Build(char const* data, size_t len)
{
    std::string buffer;
    size_t externded_payload_len = (len <= 125 ? 0 : (len <= 65535 ? 2 : 8));
    size_t mask_key_len = ((len && _frame_header.MASK) ? 4 : 0);

    auto frame_size = static_cast<size_t>(2 + externded_payload_len + mask_key_len + len);
    buffer.resize(frame_size);

    uint8_t* ptr = reinterpret_cast<uint8_t*>(buffer.data());
    uint64_t offset = 0;

    ptr[0] |= _frame_header.FIN;
    ptr[0] |= _frame_header.OPCODE;

    if (len)
    {
        ptr[1] |= _frame_header.MASK;
    }

    ++offset;

    if (len <= 125)
    {
        ptr[offset++] |= static_cast<unsigned char>(len);
    }
    else if (len <= 65535)
    {
        ptr[offset++] |= 126;
        ptr[offset++] = static_cast<unsigned char>((len >> 8) & 0xFF);
        ptr[offset++] = len & 0xFF;
    }
    else
    {
        ptr[offset++] |= 127;
        ptr[offset++] = static_cast<unsigned char>(((static_cast<uint64_t>(len) >> 56) & 0xff));
        ptr[offset++] = static_cast<unsigned char>(((static_cast<uint64_t>(len) >> 48) & 0xff));
        ptr[offset++] = static_cast<unsigned char>(((static_cast<uint64_t>(len) >> 40) & 0xff));
        ptr[offset++] = static_cast<unsigned char>(((static_cast<uint64_t>(len) >> 32) & 0xff));
        ptr[offset++] = static_cast<unsigned char>(((static_cast<uint64_t>(len) >> 24) & 0xff));
        ptr[offset++] = static_cast<unsigned char>(((static_cast<uint64_t>(len) >> 16) & 0xff));
        ptr[offset++] = static_cast<unsigned char>(((static_cast<uint64_t>(len) >> 8) & 0xff));
        ptr[offset++] = static_cast<unsigned char>((static_cast<uint64_t>(len) & 0xff));
    }

    if (!len || !data)
    {
        return buffer;
    }

    if (_frame_header.MASK)
    {
        int mask_key = static_cast<int>(_frame_header.MASKING_KEY);

        ptr[offset++] = static_cast<unsigned char>(((mask_key >> 24) & 0xff));
        ptr[offset++] = static_cast<unsigned char>(((mask_key >> 16) & 0xff));
        ptr[offset++] = static_cast<unsigned char>(((mask_key >> 8) & 0xff));
        ptr[offset++] = static_cast<unsigned char>(((mask_key) & 0xff));

        unsigned char* mask = ptr + offset - 4;

        for (uint32_t i = 0; i < len; ++i)
        {
            ptr[offset++] = static_cast<uint8_t>(data[i] ^ mask[i % 4]);
        }
    }
    else
    {
        std::copy(data, data + len, reinterpret_cast<char*>(ptr + offset));
        offset += len;
    }

    assert(offset == frame_size);
    return buffer;
}

void FrameParser::reset()
{
    _parser_step = e_fixed_header;
    _masking_key_pos = 0;
    _payload_len_offset = 0;
    _payload.clear();

    memset(&_frame_header, 0, sizeof(_frame_header));
}

void FrameParser::move_reset(FrameParser&& parser)
{
    if (&parser == this)
    {
        return;
    }

    _parser_step = parser._parser_step;
    _payload_len_offset = parser._payload_len_offset;
    _masking_key_pos = parser._masking_key_pos;
    _payload = std::move(parser._payload);
    _frame_header = std::move(parser._frame_header);
    _msb_cb = std::move(parser._msb_cb);
}

uint32_t FrameParser::parse_fixed_header(const char* data)
{
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(data);
    memset(&_frame_header, 0, sizeof(_frame_header));

    _payload_len_offset = 0;

    _frame_header.FIN = ptr[0] & 0xf0;
    _frame_header.RSV1 = ptr[0] & 0x40;
    _frame_header.RSV2 = ptr[0] & 0x20;
    _frame_header.RSV3 = ptr[0] & 0x10;
    _frame_header.OPCODE = static_cast<FrameType>(ptr[0] & 0x0f);
    _parser_step = e_payload_len;

    return 1U;
}

uint32_t FrameParser::parse_payload_len(const char* data)
{
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(data);

    _frame_header.MASK = ptr[0] & 0x80;
    _frame_header.PAYLOAD_LEN = ptr[0] & (0x7f);

    if (_frame_header.PAYLOAD_LEN <= 125)
    {
        _frame_header.PAYLOAD_REALY_LEN = _frame_header.PAYLOAD_LEN;

        if (_frame_header.MASK)
        {
            _parser_step = e_masking_key;
        }
        else
        {
            _parser_step = e_payload_data;
        }
    }
    else if (_frame_header.PAYLOAD_LEN > 125)
    {
        _parser_step = e_extened_payload_len;
    }

    if (_frame_header.PAYLOAD_LEN == 0)
    {
        assert(_msb_cb);
        _msb_cb("", _frame_header.OPCODE, !!_frame_header.FIN);
        reset();
    }

    return 1U;
}

uint32_t FrameParser::parse_extened_payload_len(const char* data, uint32_t len)
{
    uint32_t offset = 0;
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(data);

    if (_frame_header.PAYLOAD_LEN == 126)
    {
        //Extended payload length is 16bit!
        uint32_t min_len = std::min<uint32_t>
                           (2 - _payload_len_offset, len - offset);

        memcpy(&_frame_header.EXT_PAYLOAD_LEN_16,
               ptr + offset, min_len);
        offset += min_len;
        _payload_len_offset += min_len;

        if (_payload_len_offset == 2)
        {
            decode_extened_payload_len();
        }
    }
    else if (_frame_header.PAYLOAD_LEN == 127)
    {
        //Extended payload length is 64bit!
        auto min_len = std::min<uint32_t>(8 - _payload_len_offset, len - offset);
        memcpy(&_frame_header.EXT_PAYLOAD_LEN_64, ptr + offset, static_cast<size_t>(min_len));
        offset += min_len;
        _payload_len_offset += min_len;

        if (_payload_len_offset == 8)
        {
            decode_extened_payload_len();
        }
    }

    return offset;
}

void FrameParser::decode_extened_payload_len()
{
    if (_frame_header.PAYLOAD_LEN == 126)
    {
        uint16_t tmp = _frame_header.EXT_PAYLOAD_LEN_16;
        uint8_t* buffer_ = reinterpret_cast<uint8_t*>(&tmp);
        _frame_header.PAYLOAD_REALY_LEN =
                    static_cast<uint64_t>(
                                (static_cast<uint16_t>(buffer_[0]) << 8) | static_cast<uint16_t>(buffer_[1]));

    }
    else if (_frame_header.PAYLOAD_LEN == 127)
    {
        uint64_t tmp = _frame_header.EXT_PAYLOAD_LEN_64;
        uint8_t* buffer_ = reinterpret_cast<uint8_t*>(&tmp);
        _frame_header.PAYLOAD_REALY_LEN =
                    (static_cast<uint64_t>(buffer_[0]) << 56) |
                    (static_cast<uint64_t>(buffer_[1]) << 48) |
                    (static_cast<uint64_t>(buffer_[2]) << 40) |
                    (static_cast<uint64_t>(buffer_[3]) << 32) |
                    (static_cast<uint64_t>(buffer_[4]) << 24) |
                    (static_cast<uint64_t>(buffer_[5]) << 16) |
                    (static_cast<uint64_t>(buffer_[6]) << 8) |
                    static_cast<uint64_t>(buffer_[7]);
    }

    if (_frame_header.MASK)
    {
        _parser_step = e_masking_key;
    }
    else
    {
        _parser_step = e_payload_data;
    }
}

uint32_t FrameParser::parse_masking_key(const char* data, uint32_t len)
{
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(data);
    auto min = std::min<uint32_t>(4 - _masking_key_pos, len);

    if (_parser_step == e_masking_key)
    {
        memcpy(&_frame_header.MASKING_KEY, ptr, static_cast<size_t>(min));

        _masking_key_pos += min;

        if (_masking_key_pos == 4)
        {
            _parser_step = e_payload_data;
        }
    }

    return min;
}

uint32_t FrameParser::parse_payload(const char* data, uint32_t len)
{
    if (_payload.empty() && _frame_header.PAYLOAD_REALY_LEN > 0)
    {
        _payload.reserve(static_cast<size_t>(_frame_header.PAYLOAD_REALY_LEN));
    }

    auto remain = static_cast<uint32_t>(_frame_header.PAYLOAD_REALY_LEN) - static_cast<uint32_t>(_payload.size());
    auto min_len = std::min<uint32_t>(remain, len);

    if (_frame_header.MASK)
    {
        unsigned char* mask = reinterpret_cast<unsigned char*>(&_frame_header.MASKING_KEY);

        for (size_t i = 0; i < min_len; i++)
        {
            _payload.push_back(static_cast<char>(data[i] ^ mask[i % 4]));
        }
    }
    else
    {
        _payload.append(data, min_len);
    }

    if (_payload.size() == _frame_header.PAYLOAD_REALY_LEN)
    {
        assert(_msb_cb);

        _msb_cb(std::move(_payload), _frame_header.OPCODE, !!_frame_header.FIN);

        reset();
    }

    return min_len;
}

std::pair<bool, std::string> FrameParser::Parse(void const* data, uint32_t len)
{
    assert(data != nullptr && len > 0);

    uint32_t offset = 0;
    uint32_t remain_len = len;

    try
    {
        do
        {
            if (_parser_step == e_fixed_header && remain_len)
            {
                offset += parse_fixed_header(static_cast<char const*>(data) + offset);
                remain_len = len - offset;
            }

            if (_parser_step == e_payload_len && remain_len)
            {
                offset += parse_payload_len(static_cast<char const*>(data) + offset);
                remain_len = len - offset;
            }

            if (_parser_step == e_extened_payload_len && remain_len)
            {
                offset += parse_extened_payload_len(static_cast<char const*>(data) + offset, remain_len);
                remain_len = len - offset;
            }

            if (_parser_step == e_masking_key && remain_len)
            {
                offset += parse_masking_key(static_cast<char const*>(data) + offset, remain_len);
                remain_len = len - offset;
            }

            if (_parser_step == e_payload_data && remain_len)
            {
                offset += parse_payload(static_cast<char const*>(data) + offset, remain_len);
                remain_len = len - offset;
            }

        }
        while (offset < len);
    }
    catch (std::exception const& e)
    {
        return {false, e.what()};
    }
    catch (...)
    {
        return {false, "unknown exception."};
    }

    return {true, ""};
}

} // namespace Websocket
} // namespace da4qi4
