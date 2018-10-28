#ifndef CAPTCHA_UTILITIES_HPP
#define CAPTCHA_UTILITIES_HPP

#include <string>

namespace da4qi4
{
namespace Utilities
{

extern size_t const max_captcha_size;
extern bool is_symbol_captcha_char(char c);
extern std::string get_random_words_for_captcha(size_t len, bool allow_symbol = false);

extern void make_captcha_image(std::string const& captcha_text
                               , std::string const& filename, bool add_border = false);
extern void make_captcha_image(char const* captcha_text
                               , size_t len
                               , char const* filename, bool add_border = false);

} //namespace Utilities
} //namespace da4qi4

#endif // CAPTCHA_UTILITIES_HPP
