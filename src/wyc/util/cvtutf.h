#ifndef __HEADER_WYC_CVTUTF
#define __HEADER_WYC_CVTUTF

#include "wyc/basedef.h"

#define REPLACEMENT_CHAR	0x0000FFFDUL

#define MAX_UCS4 	0x7FFFFFFFUL
#define MAX_UCS2 	0x0000FFFFUL
#define MAX_UTF32	0x0010FFFFUL
#define MAX_UTF16	0x0010FFFFUL

#define GBK_SINGLE_MAX	128

namespace wyc
{
	enum CONVERT_RET {
		OK=0,
		SRC_OVERFLOW,
		DST_OVERFLOW,
	};
	
	/// count characters
	unsigned chars_gbk(const uint8_t *psrc, const uint8_t *psrc_end=(uint8_t*)-1);
	unsigned chars_utf8(const uint8_t *psrc, const uint8_t *psrc_end=(uint8_t*)-1);
	unsigned chars_utf16(const uint16_t *psrc, const uint16_t *psrc_end=(uint16_t*)-1);

	/// index of the nth character
	unsigned index_gbk(const uint8_t *psrc, const uint8_t *psrc_end, unsigned n);
	unsigned index_utf8(const uint8_t *psrc, const uint8_t *psrc_end, unsigned n);

	//==UCS-4 conversion===============================

	/// get the required size, in characters, for the given conversion
	unsigned ucs4_to_utf8(const uint32_t *psrc, const uint32_t *psrc_end=(uint32_t*)-1);
	unsigned ucs4_to_utf16(const uint32_t *psrc, const uint32_t *psrc_end=(uint32_t*)-1);
	/// single character conversion
	unsigned ucs4_to_utf8(uint8_t *pdst, uint32_t ch);
	unsigned ucs4_to_utf16(uint16_t *pdst, uint32_t ch);
	/// string conversion
	CONVERT_RET ucs4_to_utf8(uint8_t **pdst_beg, const uint8_t *pdst_end, uint32_t **psrc_beg, const uint32_t *psrc_end);
	CONVERT_RET ucs4_to_utf16(uint16_t **pdst_beg, const uint16_t *pdst_end, uint32_t **psrc_beg, const uint32_t *psrc_end);

	//==UCS-2 conversion===============================

	/// get the required size, in characters, for the given conversion
	unsigned ucs2_to_utf8(const uint16_t *psrc, const uint16_t *psrc_end=(uint16_t*)-1);
	/// single character conversion
	inline unsigned ucs2_to_utf8(uint8_t *pdst, uint16_t ch) {
		return ucs4_to_utf8(pdst,ch);
	}
	/// string conversion
	CONVERT_RET ucs2_to_utf8(uint8_t **pdst_beg, const uint8_t *pdst_end, uint16_t **psrc_beg, const uint16_t *psrc_end);

	//==UTF-8 conversion===============================

	/// get the required size, in characters, for the given conversion
	unsigned utf8_to_utf16(const uint8_t *psrc, const uint8_t *psrc_end=(uint8_t*)-1);
	/// string conversion
	CONVERT_RET utf8_to_ucs4(uint32_t **pdst_beg, const uint32_t *pdst_end, uint8_t **psrc_beg, const uint8_t *psrc_end);
	CONVERT_RET utf8_to_ucs2(uint16_t **pdst_beg, const uint16_t *pdst_end, uint8_t **psrc_beg, const uint8_t *psrc_end);
	CONVERT_RET utf8_to_utf16(uint16_t **pdst_beg, const uint16_t *pdst_end, uint8_t **psrc_beg, const uint8_t *psrc_end);

	//==UTF-16 conversion===============================

	/// get the required size, in characters, for the given conversion
	unsigned utf16_to_utf8(const uint16_t *psrc, const uint16_t *psrc_end=(uint16_t*)-1);
	/// string conversion
	CONVERT_RET utf16_to_ucs4(uint32_t **pdst_beg, const uint32_t *pdst_end, uint16_t **psrc_beg, const uint16_t *psrc_end);
	CONVERT_RET utf16_to_ucs2(uint16_t **pdst_beg, const uint16_t *pdst_end, uint16_t **psrc_beg, const uint16_t *psrc_end);
	CONVERT_RET utf16_to_utf8(uint8_t **pdst_beg, const uint8_t *pdst_end, uint16_t **psrc_beg, const uint16_t *psrc_end);

};

#endif // end of __HEADER_WYC_CVTUTF

