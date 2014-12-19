#ifndef __HEADER_WYC_STRCODEC
#define __HEADER_WYC_STRCODEC

#include <cstdlib>
#include "wyc/util/cvtutf.h"

// 编码格式常量
#define XSTR_ENCODE_ANSI	1
#define XSTR_ENCODE_UTF8	2
#define XSTR_ENCODE_UCS2	3
#define XSTR_ENCODE_UCS4	4

// xstring内部编码格式
#define XSTR_ENCODING	XSTR_ENCODE_ANSI
//#define XSTR_ENCODING	XSTR_ENCODE_UTF8
//#define XSTR_ENCODING	XSTR_ENCODE_UCS2
//#define XSTR_ENCODING	XSTR_ENCODE_UCS4

// 当前系统的字符串编码
// 单字节字符串：VC默认为ANSI, GCC默认为UTF-8
// 宽字符串：Windows为UCS-2, Linux为UCS-4
#ifdef _MSC_VER
	#define XSTR_CHAR_ENCODING XSTR_ENCODE_ANSI
#elif __GNUC__
	#define XSTR_CHAR_ENCODING XSTR_ENCODE_UTF8
#endif

#if defined(_MSC_VER) || defined(__MINGW32__)
	#define XSTR_WCHAR_ENCODING XSTR_ENCODE_UCS2
#else
	#define XSTR_WCHAR_ENCODING XSTR_ENCODE_UCS4
#endif

#ifdef _MSC_VER
	#pragma warning(push)
	#pragma warning(disable: 4996)
#endif //_MSC_VER

namespace wyc
{

#if XSTR_WCHAR_ENCODING==XSTR_ENCODE_UCS4
	#define ucs_to_utf8	ucs4_to_utf8
	#define utf8_to_ucs utf8_to_ucs4
	typedef uint32_t ucschar_t;
	typedef uint32_t* pstrucs;
#elif XSTR_WCHAR_ENCODING==XSTR_ENCODE_UCS2
	#define ucs_to_utf8	ucs2_to_utf8
	#define utf8_to_ucs utf8_to_ucs2
	typedef uint16_t ucschar_t;
	typedef uint16_t* pstrucs;
#endif

class string_codec
{
	unsigned m_nchar;
	pstrucs m_pustr;
public:
	string_codec();
	~string_codec();
	bool decode_ansi(const char *pstr, unsigned len=0);
	bool decode_utf8(const char *pstr, unsigned len=0);
	bool decode_ucs2(const uint16_t *pstr, unsigned len=0);
	bool decode_ucs4(const uint32_t *pstr, unsigned len=0);
	bool encode_ansi(char *pdst, unsigned size);
	bool encode_utf8(char *pdst, unsigned size);
	bool encode_ucs2(uint16_t *pdst, unsigned size);
	bool encode_ucs4(uint32_t *pdst, unsigned size);
	unsigned size_ansi() const;
	unsigned size_utf8() const;
	unsigned size_ucs2() const;
	unsigned size_ucs4() const;
	unsigned chars() const;
	const wchar_t* buffer() const;
};

inline bool string_codec::encode_ansi(char *pdst, unsigned size) 
{
	if(wcstombs(pdst,(wchar_t*)m_pustr,size)==unsigned(-1))
		return false;
	return true;
}

inline unsigned string_codec::size_ansi() const {
	return wcstombs(0,(wchar_t*)m_pustr,0);
}

inline unsigned string_codec::size_utf8() const {
	return ucs_to_utf8(m_pustr);
}

inline unsigned string_codec::size_ucs2() const {
	return m_nchar<<1;
}

inline unsigned string_codec::size_ucs4() const {
	return m_nchar<<2;
}

inline unsigned string_codec::chars() const {
	return m_nchar;
}

inline const wchar_t* string_codec::buffer() const {
	return (wchar_t*)m_pustr;
}

} // namespace wyc

#ifdef _MSC_VER
#pragma warning(pop)
#endif //_MSC_VER

#endif // end of __HEADER_WYC_STRCODEC
