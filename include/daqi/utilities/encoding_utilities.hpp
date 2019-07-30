#ifndef ENCODING_UTILITIES_HPP
#define ENCODING_UTILITIES_HPP

#include <iconv.h>
#include <string>

namespace da4qi4
{
namespace Utilities
{

namespace iconvpp
{

class Converter
{
public:
    Converter(char const* in_encode, char const* out_encode,
              bool _ignore_error = false, std::size_t const& buf_size = 5);

    ~Converter();

    std::string const& Error() const
    {
        return _err;
    };

    operator bool () const noexcept
    {
        return _err.empty() && _iconv && (_iconv != reinterpret_cast<iconv_t>(-1));
    }

    bool operator !() const noexcept
    {
        return !_err.empty() || !_iconv || (_iconv == reinterpret_cast<iconv_t>(-1));
    }

    std::size_t IgnoreCount() const noexcept
    {
        return _ignore_count;
    }

    bool Convert(std::string const& input, std::string& output);

private:
    iconv_t _iconv;
    bool _ignore_error;
    const size_t _buf_size;
    std::string _err;
    std::size_t _ignore_count;
};

} //namespace iconvpp

std::wstring FromUTF8(std::string const& utf8str, std::string& err);
std::string ToUTF8(std::wstring const& wstr, std::string& err);

std::wstring FromUTF8(std::string const& utf8str);
std::string ToUTF8(std::wstring const& wstr);

std::string ToGBK(std::string const& utf8str, std::string& err);
std::string ToGBK(std::string const& utf8str);

std::string FromGBK(std::string const& gbkstr, std::string& err);
std::string FromGBK(std::string const& gbkstr);

} //namespace Utilities
} //namespace da4qi4

#endif // ENCODING_UTILITIES_HPP
