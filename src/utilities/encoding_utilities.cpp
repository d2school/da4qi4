#include "daqi/utilities/encoding_utilities.hpp"

#include <vector>

namespace da4qi4
{
namespace Utilities
{

namespace iconvpp
{

Converter::Converter(char const* in_encode, char const* out_encode,
                     bool _ignore_error, std::size_t const& buf_size)
    : _iconv(nullptr), _ignore_error(_ignore_error), _buf_size(buf_size), _ignore_count(0)
{
    if (buf_size < 4 || buf_size >= 10 * 1024 * 1024)
    {
        _err = "bad convert buffer size : " + std::to_string(buf_size) + ".";
        return;
    }

    iconv_t h = ::iconv_open(out_encode, in_encode);

    if (h == reinterpret_cast<iconv_t>(-1))
    {
        if (errno == EINVAL)
        {
            _err = std::string("not supported from ") + in_encode + " to " + out_encode;
        }
        else
        {
            _err = "unknown error on iconv_open.";
        }

        return;
    }

    _iconv = h;
}

Converter::~Converter()
{
    if (_iconv && (_iconv != reinterpret_cast<iconv_t>(-1)))
    {
        iconv_close(_iconv);
    }
}

bool Converter::Convert(std::string const& input, std::string& output)
{
    if (!*this)
    {
        return false;
    }

    _ignore_count = 0;
    std::vector<char> in_buf(input.begin(), input.end());

    char* src_ptr = &in_buf[0];
    size_t src_size = input.size();

    std::vector<char> buf(_buf_size);
    std::string dst;

    while (src_size > 0)
    {
        char* dst_ptr = &buf[0];
        size_t dst_size = buf.size();
        size_t res = ::iconv(_iconv, &src_ptr, &src_size, &dst_ptr, &dst_size);

        if (res == static_cast<size_t>(-1))
        {
            if (errno == E2BIG)
            {
                ;// ignore this error
            }
            else if (_ignore_error)
            {
                ++src_ptr;
                --src_size;
                ++_ignore_count;
            }
            else if (errno == EILSEQ)
            {
                _err = "invalid multibyte chars : EILSEQ.";
                break;
            }
            else if (errno == EINVAL)
            {
                _err = "invalid multibyte chars : EINVAL.";
                break;
            }
        }

        dst.append(&buf[0], _buf_size - dst_size);
    }

    if (!(*this))
    {
        return false;
    }

    dst.swap(output);
    return true;
}

} //namesapce iconvpp

constexpr bool WCHAR_HAS_4_BYTES()
{
    return sizeof(wchar_t) == 4;
}

constexpr bool WCHAR_HAS_2_BYTES()
{
    return sizeof(wchar_t) == 2;
}

//bool IS_LITTER_ENDIAN()
//{
//    union UN
//    {
//        char c;
//        int i;
//    } un;
//    un.i = 1;

//    return un.c == 1;
//}

std::wstring FromUTF8(std::string const& utf8str, std::string& err)
{
    if (!WCHAR_HAS_4_BYTES() && !WCHAR_HAS_2_BYTES())
    {
        err = "only support wchat_t with 2 or 4 bytes.";
        return std::wstring();
    }

    iconvpp::Converter cvter("UTF-8"
                             , ((WCHAR_HAS_4_BYTES()) ? "UCS-4" : "UCS-2")
                             , true);

    if (!cvter)
    {
        err = cvter.Error();
        return std::wstring();
    }

    std::string output;

    if (!cvter.Convert(utf8str, output))
    {
        err = cvter.Error();
        return std::wstring();
    }

    std::wstring ws;
    ws.reserve(output.size() / sizeof(wchar_t));

    for (std::size_t i = 0; i < output.size(); i += sizeof(wchar_t))
    {
        wchar_t wc = static_cast<wchar_t>(0);

        if (WCHAR_HAS_4_BYTES())
        {
            wchar_t b4 = output[i];
            wchar_t b3 = output[i + 1];
            wchar_t b2 = output[i + 2];
            wchar_t b1 = output[i + 3];
            wc = static_cast<wchar_t>((static_cast<unsigned int>(b4 << 24) & 0xFF000000)
                                      | ((b3 << 16) & 0x00FF0000)
                                      | ((b2 << 8) & 0x0000FF00)
                                      | (b1 & 0x000000FF)); //TODO : how about BIG-ENDIAN?
        }
        else
        {
            wchar_t b2 = output[i];
            wchar_t b1 = output[i + 1];
            wc = (b2 << 8) | b1;
        }

        ws.push_back(wc);
    }

    return ws;
}

std::string ToUTF8(std::wstring const& wstr, std::string& err)
{
    if (!WCHAR_HAS_4_BYTES() && !WCHAR_HAS_2_BYTES())
    {
        err = "only support wchat_t with 2 or 4 bytes.";
        return std::string();
    }

    iconvpp::Converter cvter(((WCHAR_HAS_4_BYTES()) ? "UCS-4" : "UCS-2"), "UTF-8", true);

    if (!cvter)
    {
        err = cvter.Error();
        return std::string();
    }

    std::string input;
    input.reserve(wstr.size() * sizeof(wchar_t));

    for (std::size_t i = 0; i < wstr.size(); ++i)
    {
        wchar_t wc = wstr[i];

        if (WCHAR_HAS_4_BYTES())
        {
            char b4 = static_cast<char>((static_cast<unsigned int>(wc) & 0xFF000000) >> 24);
            char b3 = static_cast<char>((wc & 0x00FF0000) >> 16);
            char b2 = static_cast<char>((wc & 0x0000FF00) >> 8);
            char b1 = static_cast<char>((wc & 0x000000FF));
            input.push_back(b4);
            input.push_back(b3);
            input.push_back(b2);
            input.push_back(b1);
        }
        else
        {
            char b2 = static_cast<char>((static_cast<unsigned int>(wc) & 0xFF00) >> 8);
            char b1 = static_cast<char>((wc & 0x00FF));
            input.push_back(b2);
            input.push_back(b1);
        }
    }

    std::string output;

    if (!cvter.Convert(input, output))
    {
        err = cvter.Error();
        return std::string();
    }

    return output;
}

std::wstring FromUTF8(std::string const& utf8str)
{
    std::string err;
    return FromUTF8(utf8str, err);
}

std::string ToUTF8(std::wstring const& wstr)
{
    std::string err;
    return ToUTF8(wstr, err);
}

std::string ToGBK(std::string const& utf8str, std::string& err)
{
    iconvpp::Converter cvter("UTF-8", "GBK", true);

    if (!cvter)
    {
        err = cvter.Error();
        return std::string();
    }

    std::string output;

    if (!cvter.Convert(utf8str, output))
    {
        err = cvter.Error();
        return std::string();
    }

    return output;
}

std::string ToGBK(std::string const& utf8str)
{
    std::string err;
    return ToGBK(utf8str, err);
}

std::string FromGBK(std::string const& gbkstr, std::string& err)
{
    iconvpp::Converter cvter("GBK", "UTF-8", true);

    if (!cvter)
    {
        err = cvter.Error();
        return std::string();
    }

    std::string output;

    if (!cvter.Convert(gbkstr, output))
    {
        err = cvter.Error();
        return std::string();
    }

    return output;
}

std::string FromGBK(std::string const& gbkstr)
{
    std::string err;
    return FromGBK(gbkstr, err);
}

} //namespace Utilities
} //namespace da4qi4
