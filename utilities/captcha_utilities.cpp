#include "utilities/captcha_utilities.hpp"

#include "cimg/CImg.h"

#include "def/debug_def.hpp"

namespace da4qi4
{
namespace Utilities
{

using namespace cimg_library;

#ifdef min
    #undef min
#endif

#ifdef min
    #undef max
#endif

size_t const max_captcha_size = 8;

bool is_symbol_captcha_char(char c)
{
    return !((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z'));
}

std::string get_random_words_for_captcha(size_t len, bool allow_symbol)
{
    static std::string const m = "*AaCcdEeFfKkMmNtUuVvWwXxYZ#+34578";

    if (len > max_captcha_size)
    {
        len = max_captcha_size;
    }

    std::string s;
    s.resize(len);

    for (int i = 0; i < len; ++i)
    {
        int rand_index;

        while (true)
        {
            rand_index = std::rand() % m.size();
            char c = m[rand_index];

            if (!allow_symbol && is_symbol_captcha_char(c))
            {
                continue;
            }

            s[i] = c;
            break;
        }
    }

    return s;
}

void make_captcha_image(char const* captcha_text, size_t len, char const* filename, bool add_border)
{
    char letter[2] = {'\0', '\0'};

    if (len > max_captcha_size)
    {
        len = max_captcha_size;
    }

    double w = 6 + (40 * len) + 6;
    CImg<unsigned char> captcha(static_cast<int>(w), 64, 1, 3, 0), color(3);

    // Write colored and distorted text
    for (size_t k = 0; k < len; ++k)
    {
        CImg<unsigned char> tmp;
        *letter = captcha_text[k];
        cimg_forX(color, i) color[i] = (unsigned char)(128 + (std::rand() % 127));
        tmp.draw_text((int)(2 + 8 * cimg::rand()) //x
                      , (int)(12 * cimg::rand()) //y
                      , letter
                      , color.data()
                      , 0
                      , 1
                      , std::rand() % 2 ? 42 : 56).resize(-100, -100, 1, 3);
        unsigned int const dir = std::rand() % 4;
        unsigned int const wph = tmp.width() + tmp.height();
        cimg_forXYC(tmp, x, y, v)
        {
            const int val = (dir == 0) ? x + y
                            : ((dir == 1) ? x + tmp.height() - y
                               : ((dir == 2) ? y + tmp.width() - x
                                  : tmp.width() - x + tmp.height() - y));
            tmp(x, y, v) = (unsigned char)std::max(0.0f, std::min(255.0f, 2.8f * tmp(x, y, v) * val / wph));
        }

        if (std::rand() % 2)
        {
            tmp = (tmp.get_dilate(5) -= tmp);
        }

        tmp.blur((float)cimg::rand() * 0.8f).normalize(0, 255);
        float const sin_offset = (float)cimg::rand(-1, 1) * 3;
        float const sin_freq = (float)cimg::rand(-1, 1) / 7;
        cimg_forYC(captcha, y, v)
        {
            captcha.get_shared_row(y, 0, v).shift((int)(4 * std::cos(y * sin_freq + sin_offset)));
        }
        captcha.draw_image(6 + 40 * k, tmp);
    }

    // Add geometric and random noise
    CImg<unsigned char> copy = (+captcha).fill(0);

    for (unsigned int l = 0; l < 3; ++l)
    {
        if (l)
        {
            copy.blur(0.5f).normalize(80, 148);
        }

        for (unsigned int k = 0; k < 4; ++k)
        {
            cimg_forX(color, i) color[i] = (unsigned char)(128 + cimg::rand() * 127);

            if (cimg::rand() < 0.5f)
            {
                copy.draw_circle((int)(cimg::rand()*captcha.width()),
                                 (int)(cimg::rand()*captcha.height()),
                                 (int)(cimg::rand() * 30),
                                 color.data(), 0.6f, ~0U);
            }
            else
            {
                copy.draw_line((int)(cimg::rand()*captcha.width()),
                               (int)(cimg::rand()*captcha.height()),
                               (int)(cimg::rand()*captcha.width()),
                               (int)(cimg::rand()*captcha.height()),
                               color.data(), 0.6f);
            }
        }
    }

    captcha |= copy;
    captcha.noise(10, 2);

    if (add_border)
    {
        captcha.draw_rectangle(0, 0, captcha.width() - 1, captcha.height() - 1,
                               CImg<unsigned char>::vector(255, 255, 255).data(), 1.0f, ~0U);
    }

    captcha = (+captcha).fill(255) - captcha;
    captcha.save(filename);
}

void make_captcha_image(std::string const& captcha_text
                        , std::string const& filename, bool add_border)
{
    make_captcha_image(captcha_text.c_str(), captcha_text.size(), filename.c_str());
}

} //namespace Utilities
} //namespace da4qi4

