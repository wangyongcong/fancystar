#ifndef __HEADER_WYC_FONT_UTIL
#define __HEADER_WYC_FONT_UTIL

#include "wyc/render/font.h"

namespace wyc
{

xfont* get_font(const char *font_name);

unsigned load_glyphs(xfont *font, const wchar_t *chars);

}; // namespace wyc

#endif // __HEADER_WYC_FONT_UTIL

