#include "wyc/util/cvtutf.h"

#ifdef _MSC_VER
#pragma warning (disable:4244)
#endif //_MSC_VER

namespace wyc 
{

#define HALF_SHIFT	10
#define HALF_BASE	0x0010000UL
#define HALF_MASK	0x3FFUL
#define SURROGATE_HIGH_START	0xD800UL
#define SURROGATE_HIGH_END		0xDBFFUL
#define SURROGATE_LOW_START		0xDC00UL
#define SURROGATE_LOW_END		0xDFFFUL

const uint32_t OFFSET_UTF8[6] = {
	0x00000000UL, 0x00003080UL, 0x000E2080UL, 
	0x03C82080UL, 0xFA082080UL, 0x82082080UL,
};

const uint8_t EXTRA_BYTES_UTF8[256] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5,
};

const uint8_t FIRST_BYTE_MASK[7] = {0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC};

unsigned chars_gbk(const uint8_t *psrc, const uint8_t *psrc_end)
{
	register unsigned len=0;
	register uint8_t ch;
	while (psrc<psrc_end)
	{
		if(!(ch=*psrc++))
			break;
		if(ch>GBK_SINGLE_MAX)
			++psrc;
		++len;
	}
	return len;
}

unsigned chars_utf8(const uint8_t *psrc, const uint8_t *psrc_end)
{
	register unsigned len=0;
	register uint8_t ch;
	while (psrc<psrc_end)
	{
		if(!(ch=*psrc++))
			break;
		psrc+=EXTRA_BYTES_UTF8[ch];
		++len;
	}
	return len;
}

unsigned chars_utf16(const uint16_t *psrc, const uint16_t *psrc_end)
{
	register unsigned len=0;
	register uint16_t ch;
	while (psrc<psrc_end)
	{
		if(!(ch=*psrc++))
			break;
		if (ch>=SURROGATE_HIGH_START && ch<=SURROGATE_HIGH_END) 
			++psrc;
		++len;
	}
	return len;
}

unsigned ucs4_to_utf8(const uint32_t *psrc, const uint32_t *psrc_end)
{
	register unsigned bytes=0;
	register uint32_t ch;
	while (psrc<psrc_end) {
		if(!(ch=*psrc++))
			break;
		if (ch < 0x80)				bytes += 1;
		else if (ch < 0x800)		bytes += 2;
		else if (ch < 0x10000)		bytes += 3;
		else if (ch < 0x200000)		bytes += 4;
		else if (ch < 0x4000000)	bytes += 5;
		else if (ch <= MAX_UCS4)	bytes += 6;
		else 						bytes += 2;
	}
	return bytes;
}

unsigned ucs4_to_utf8(uint8_t *pdst, uint32_t ch)
{
	uint16_t bytesToWrite;
	if (ch < 0x80)				bytesToWrite = 1;
	else if (ch < 0x800)		bytesToWrite = 2;
	else if (ch < 0x10000)		bytesToWrite = 3;
	else if (ch < 0x200000)		bytesToWrite = 4;
	else if (ch < 0x4000000)	bytesToWrite = 5;
	else if (ch <= MAX_UCS4)	bytesToWrite = 6;
	else {						bytesToWrite = 2;
								ch = REPLACEMENT_CHAR;
	}
	if(pdst) {
		pdst+=bytesToWrite;
		switch (bytesToWrite) {
			case 6:	*--pdst = (ch | 0x80) & 0xBF; ch >>= 6;
			case 5:	*--pdst = (ch | 0x80) & 0xBF; ch >>= 6;
			case 4:	*--pdst = (ch | 0x80) & 0xBF; ch >>= 6;
			case 3:	*--pdst = (ch | 0x80) & 0xBF; ch >>= 6;
			case 2:	*--pdst = (ch | 0x80) & 0xBF; ch >>= 6;
			case 1:	*--pdst =  ch | FIRST_BYTE_MASK[bytesToWrite];
		};
	}
	return bytesToWrite;
}

unsigned ucs4_to_utf16(const uint32_t *psrc, const uint32_t *psrc_end)
{
	register unsigned bytes=0;
	register uint32_t ch;
	while (psrc<psrc_end) {
		if(!(ch=*psrc++))
			break;
		bytes+=ch>MAX_UCS2?2:1;
	}
	return bytes;
}

unsigned ucs4_to_utf16(uint16_t *pdst, uint32_t ch)
{
	if (pdst==0)
	{
		if (ch<=MAX_UCS2 || ch>MAX_UTF16)
			return 1;
		return 2;
	}
	if (ch<=MAX_UCS2) {
		*pdst = ch;
		return 1;
	}
	if (ch>MAX_UTF16) {
		*pdst = REPLACEMENT_CHAR;
		return 1;
	}
	ch-=HALF_BASE;
	*pdst = (ch>>HALF_SHIFT)+SURROGATE_HIGH_START;
	*(pdst+1) = (ch&HALF_MASK)+SURROGATE_LOW_START;
	return 2;
}

CONVERT_RET ucs4_to_utf8(uint8_t **pdst_beg, const uint8_t *pdst_end, uint32_t **psrc_beg, const uint32_t *psrc_end)
{
	CONVERT_RET ret=OK;
	register uint32_t *psrc=*psrc_beg;
	register uint8_t *pdst=*pdst_beg;
	register uint16_t bytesToWrite=0;
	register uint32_t ch;
	while (psrc < psrc_end) {
		ch=*psrc++;
		if (ch < 0x80)				bytesToWrite = 1;
		else if (ch < 0x800)		bytesToWrite = 2;
		else if (ch < 0x10000)		bytesToWrite = 3;
		else if (ch < 0x200000)		bytesToWrite = 4;
		else if (ch < 0x4000000)	bytesToWrite = 5;
		else if (ch <= MAX_UCS4)	bytesToWrite = 6;
		else {						bytesToWrite = 2;
									ch = REPLACEMENT_CHAR;
		}
		pdst+=bytesToWrite;
		if (pdst>pdst_end) {
			pdst-=bytesToWrite;
			ret=DST_OVERFLOW;
			break;
		};
		switch (bytesToWrite) {
			case 6:	*--pdst = (ch | 0x80) & 0xBF; ch >>= 6;
			case 5:	*--pdst = (ch | 0x80) & 0xBF; ch >>= 6;
			case 4:	*--pdst = (ch | 0x80) & 0xBF; ch >>= 6;
			case 3:	*--pdst = (ch | 0x80) & 0xBF; ch >>= 6;
			case 2:	*--pdst = (ch | 0x80) & 0xBF; ch >>= 6;
			case 1:	*--pdst =  ch | FIRST_BYTE_MASK[bytesToWrite];
		};
		pdst+=bytesToWrite;
	};
	*psrc_beg=psrc;
	*pdst_beg=pdst;
	return ret;
}

CONVERT_RET ucs4_to_utf16(uint16_t **pdst_beg, const uint16_t *pdst_end, uint32_t **psrc_beg, const uint32_t *psrc_end)
{
	CONVERT_RET ret=OK;
	register uint32_t *psrc=*psrc_beg;
	register uint16_t *pdst=*pdst_beg;
	register uint32_t ch;
	while (psrc<psrc_end) {
		if (pdst>=pdst_end) {
			ret=DST_OVERFLOW; 
			break;
		};
		ch=*psrc++;
		if (ch<=MAX_UCS2)
			*pdst++ = ch;
		else if (ch>MAX_UTF16)
			*pdst++ = REPLACEMENT_CHAR;
		else {
			if (pdst+1 >= pdst_end) {
				ret=DST_OVERFLOW; 
				break;
			};
			ch-=HALF_BASE;
			*pdst++ = (ch>>HALF_SHIFT)+SURROGATE_HIGH_START;
			*pdst++ = (ch&HALF_MASK)+SURROGATE_LOW_START;
		}
	}
	*psrc_beg=psrc;
	*pdst_beg=pdst;
	return ret;
}

unsigned ucs2_to_utf8(const uint16_t *psrc, const uint16_t *psrc_end)
{
	register unsigned bytes=0;
	register uint32_t ch;
	while (psrc<psrc_end)
	{
		if(!(ch=*psrc++))
			break;
		if (ch < 0x80)			bytes += 1;
		else if (ch < 0x800)	bytes += 2;
		else					bytes += 3; // MAX_UCS2<0x10000
	}
	return bytes;
}

CONVERT_RET ucs2_to_utf8(uint8_t **pdst_beg, const uint8_t *pdst_end, uint16_t **psrc_beg, const uint16_t *psrc_end)
{
	CONVERT_RET ret=OK;
	register uint16_t *psrc=*psrc_beg;
	register uint8_t *pdst=*pdst_beg;
	register unsigned bytesToWrite=0;
	register uint16_t ch;
	while (psrc<psrc_end)
	{
		ch=*psrc++;
		if (ch < 0x80)			bytesToWrite = 1;
		else if (ch < 0x800)	bytesToWrite = 2;
		else					bytesToWrite = 3; // MAX_UCS2<0x10000
		pdst+=bytesToWrite;
		if(pdst>pdst_end) {
			pdst-=bytesToWrite;
			ret=DST_OVERFLOW;
			break;
		}
		switch(bytesToWrite) {
			case 3:	*--pdst = (ch | 0x80) & 0xBF; ch >>= 6;
			case 2:	*--pdst = (ch | 0x80) & 0xBF; ch >>= 6;
			case 1:	*--pdst =  ch | FIRST_BYTE_MASK[bytesToWrite];
		}
		pdst+=bytesToWrite;
	}
	*psrc_beg=psrc;
	*pdst_beg=pdst;
	return ret;
}

unsigned utf8_to_utf16(const uint8_t *psrc_beg, const uint8_t *psrc_end)
{
	register const uint8_t *psrc=psrc_beg;
	register uint16_t extraBytes;
	register uint32_t ch, bytesToWrite=0;
	while (psrc<psrc_end && *psrc!=0) {
		extraBytes=EXTRA_BYTES_UTF8[*psrc];
		if (psrc+extraBytes > psrc_end) 
			break;
		ch=0;
		switch(extraBytes) {
			case 5:	ch += *psrc++; ch <<= 6;
			case 4:	ch += *psrc++; ch <<= 6;
			case 3:	ch += *psrc++; ch <<= 6;
			case 2:	ch += *psrc++; ch <<= 6;
			case 1:	ch += *psrc++; ch <<= 6;
			case 0:	ch += *psrc++;
		};
		ch-=OFFSET_UTF8[extraBytes];
		if (ch <= MAX_UCS2 || ch>MAX_UTF16) 
			++bytesToWrite;
		else 
			bytesToWrite+=2;
	};
	return bytesToWrite;
}

CONVERT_RET utf8_to_ucs4(uint32_t **pdst_beg, const uint32_t *pdst_end, uint8_t **psrc_beg, const uint8_t *psrc_end)
{
	CONVERT_RET ret=OK;
	register uint8_t *psrc=*psrc_beg;
	register uint32_t *pdst=*pdst_beg;
	register uint16_t extraBytes;
	register uint32_t ch;
	while (psrc<psrc_end) {
		ch=0;
		extraBytes=EXTRA_BYTES_UTF8[*psrc];
		if (psrc+extraBytes > psrc_end) {
			ret=SRC_OVERFLOW; 
			break;
		}
		switch(extraBytes) {
			case 5:	ch += *psrc++; ch <<= 6;
			case 4:	ch += *psrc++; ch <<= 6;
			case 3:	ch += *psrc++; ch <<= 6;
			case 2:	ch += *psrc++; ch <<= 6;
			case 1:	ch += *psrc++; ch <<= 6;
			case 0:	ch += *psrc++;
		};
		ch-=OFFSET_UTF8[extraBytes];

		if (pdst>=pdst_end) {
			ret=DST_OVERFLOW; 
			break;
		}
		if (ch>MAX_UCS4)
			*pdst++ = REPLACEMENT_CHAR;
		else
			*pdst++ = ch;
	};
	*psrc_beg=psrc;
	*pdst_beg=pdst;
	return ret;
}

CONVERT_RET utf8_to_ucs2(uint16_t **pdst_beg, const uint16_t *pdst_end, uint8_t **psrc_beg, const uint8_t *psrc_end)
{
	CONVERT_RET ret=OK;
	register uint8_t* psrc=*psrc_beg;
	register uint16_t* pdst=*pdst_beg;
	register uint16_t extraBytes;
	register uint32_t ch;
	while (psrc<psrc_end) {
		ch=0;
		extraBytes=EXTRA_BYTES_UTF8[*psrc];
		if (psrc+extraBytes > psrc_end) {
			ret=SRC_OVERFLOW; 
			break;
		};
		switch(extraBytes) {
			case 5:	ch += *psrc++; ch <<= 6;
			case 4:	ch += *psrc++; ch <<= 6;
			case 3:	ch += *psrc++; ch <<= 6;
			case 2:	ch += *psrc++; ch <<= 6;
			case 1:	ch += *psrc++; ch <<= 6;
			case 0:	ch += *psrc++;
		};
		ch-=OFFSET_UTF8[extraBytes];

		if (pdst >= pdst_end) {
			ret=DST_OVERFLOW; 
			break;
		};
		if (ch <= MAX_UCS2) 
			*pdst++ = ch;
		else
			*pdst++ = REPLACEMENT_CHAR;
	};
	*psrc_beg=psrc;
	*pdst_beg=pdst;
	return ret;
}

CONVERT_RET utf8_to_utf16(uint16_t **pdst_beg, const uint16_t *pdst_end, uint8_t **psrc_beg, const uint8_t *psrc_end)
{
	CONVERT_RET ret=OK;
	register uint8_t* psrc=*psrc_beg;
	register uint16_t* pdst=*pdst_beg;
	register uint16_t extraBytes;
	register uint32_t ch;
	while (psrc<psrc_end) {
		ch=0;
		extraBytes=EXTRA_BYTES_UTF8[*psrc];
		if (psrc+extraBytes > psrc_end) {
			ret=SRC_OVERFLOW; 
			break;
		};
		switch(extraBytes) {
			case 5:	ch += *psrc++; ch <<= 6;
			case 4:	ch += *psrc++; ch <<= 6;
			case 3:	ch += *psrc++; ch <<= 6;
			case 2:	ch += *psrc++; ch <<= 6;
			case 1:	ch += *psrc++; ch <<= 6;
			case 0:	ch += *psrc++;
		};
		ch-=OFFSET_UTF8[extraBytes];

		if (pdst >= pdst_end) {
			ret=DST_OVERFLOW; 
			break;
		};
		if (ch <= MAX_UCS2) 
			*pdst++ = ch;
		else if (ch>MAX_UTF16) 
			*pdst++ = REPLACEMENT_CHAR;
		else {
			if (pdst+1 >= pdst_end) {
				ret=DST_OVERFLOW;
				break;
			};
			ch-=HALF_BASE;
			*pdst++ = (ch>>HALF_SHIFT) + SURROGATE_HIGH_START;
			*pdst++ = (ch & HALF_MASK) + SURROGATE_LOW_START;
		};
	};
	*psrc_beg=psrc;
	*pdst_beg=pdst;
	return ret;
}

unsigned utf16_to_utf8(const uint16_t *psrc, const uint16_t *psrc_end)
{
	register unsigned bytes=0;
	register uint32_t ch;
	while (psrc<psrc_end)
	{
		ch=*psrc++;
		if (ch==0)
			break;
		if (ch>=SURROGATE_HIGH_START && ch<=SURROGATE_HIGH_END) {
			if (psrc<psrc_end) {
				register uint32_t ch2=*psrc;
				if (ch2>=SURROGATE_LOW_START && ch2<=SURROGATE_LOW_END) {
					ch=((ch-SURROGATE_HIGH_START)<<HALF_SHIFT)+(ch2-SURROGATE_LOW_START)+HALF_BASE;
					++psrc;
				}
				else ch=REPLACEMENT_CHAR;
			}
			else ch=REPLACEMENT_CHAR;
		};
		if (ch < 0x80)				bytes += 1;
		else if (ch < 0x800)		bytes += 2;
		else if (ch < 0x10000)		bytes += 3;
		else if (ch < 0x200000)		bytes += 4;
		else if (ch < 0x4000000)	bytes += 5;
		else if (ch <= MAX_UCS4)	bytes += 6;
		else 						bytes += 2;
	}
	return bytes;
}

CONVERT_RET utf16_to_ucs4(uint32_t **pdst_beg, const uint32_t *pdst_end, uint16_t **psrc_beg, const uint16_t *psrc_end)
{
	CONVERT_RET ret=OK;
	register uint16_t *psrc=*psrc_beg;
	register uint32_t *pdst=*pdst_beg;
	register uint32_t ch;
	while (psrc<psrc_end) {
		ch=*psrc++;
		if (ch>=SURROGATE_HIGH_START && ch<=SURROGATE_HIGH_END) {
			if (psrc<psrc_end) {
				register uint32_t ch2=*psrc++;
				if (ch2>=SURROGATE_LOW_START && ch2<=SURROGATE_LOW_END)
					ch=((ch-SURROGATE_HIGH_START)<<HALF_SHIFT)+(ch2-SURROGATE_LOW_START)+HALF_BASE;
				else ch=REPLACEMENT_CHAR;
			}
			else ch=REPLACEMENT_CHAR;
		}
		if (pdst>=pdst_end) {
			ret=DST_OVERFLOW; 
			break;
		}
		*pdst++ = ch;
	};
	*psrc_beg=psrc;
	*pdst_beg=pdst;
	return ret;
}

CONVERT_RET utf16_to_ucs2(uint16_t **pdst_beg, const uint16_t *pdst_end, uint16_t **psrc_beg, const uint16_t *psrc_end)
{
	CONVERT_RET ret=OK;
	register uint16_t *psrc=*psrc_beg;
	register uint16_t *pdst=*pdst_beg;
	register uint32_t ch;
	while (psrc<psrc_end) {
		ch=*psrc++;
		if (ch>=SURROGATE_HIGH_START && ch<=SURROGATE_HIGH_END) {
			if (psrc<psrc_end) {
				register uint32_t ch2=*psrc++;
				if (ch2>=SURROGATE_LOW_START && ch2<=SURROGATE_LOW_END)
					ch=((ch-SURROGATE_HIGH_START)<<HALF_SHIFT)+(ch2-SURROGATE_LOW_START)+HALF_BASE;
				else ch=REPLACEMENT_CHAR;
			}
			else ch=REPLACEMENT_CHAR;
		}
		if (pdst>=pdst_end) {
			ret=DST_OVERFLOW; 
			break;
		}
		if (ch <= MAX_UCS2) 
			*pdst++ = ch;
		else
			*pdst++ = REPLACEMENT_CHAR;
	};
	*psrc_beg=psrc;
	*pdst_beg=pdst;
	return ret;
}

CONVERT_RET utf16_to_utf8(uint8_t **pdst_beg, const uint8_t *pdst_end, uint16_t **psrc_beg, const uint16_t *psrc_end)
{
	CONVERT_RET ret=OK;
	register uint16_t *psrc=*psrc_beg;
	register uint8_t *pdst=*pdst_beg;
	register unsigned bytesToWrite=0;
	register uint32_t ch;
	while (psrc<psrc_end)
	{
		ch=*psrc++;
		if (ch>=SURROGATE_HIGH_START && ch<=SURROGATE_HIGH_END) {
			if (psrc<psrc_end) {
				register uint32_t ch2=*psrc;
				if (ch2>=SURROGATE_LOW_START && ch2<=SURROGATE_LOW_END) {
					ch=((ch-SURROGATE_HIGH_START)<<HALF_SHIFT)+(ch2-SURROGATE_LOW_START)+HALF_BASE;
					++psrc;
				}
				else ch=REPLACEMENT_CHAR;
			}
			else ch=REPLACEMENT_CHAR;
		};
		if (ch < 0x80)				bytesToWrite = 1;
		else if (ch < 0x800)		bytesToWrite = 2;
		else if (ch < 0x10000)		bytesToWrite = 3;
		else if (ch < 0x200000)		bytesToWrite = 4;
		else if (ch < 0x4000000)	bytesToWrite = 5;
		else if (ch <= MAX_UCS4)	bytesToWrite = 6;
		else {						bytesToWrite = 2;
									ch=REPLACEMENT_CHAR;
		}
		pdst+=bytesToWrite;
		if(pdst>pdst_end) {
			pdst-=bytesToWrite;
			ret=DST_OVERFLOW;
			break;
		}
		switch(bytesToWrite) {
			case 6:	*--pdst = (ch | 0x80) & 0xBF; ch >>= 6;
			case 5:	*--pdst = (ch | 0x80) & 0xBF; ch >>= 6;
			case 4:	*--pdst = (ch | 0x80) & 0xBF; ch >>= 6;
			case 3:	*--pdst = (ch | 0x80) & 0xBF; ch >>= 6;
			case 2:	*--pdst = (ch | 0x80) & 0xBF; ch >>= 6;
			case 1:	*--pdst =  ch | FIRST_BYTE_MASK[bytesToWrite];
		}
		pdst+=bytesToWrite;
	}
	*psrc_beg=psrc;
	*pdst_beg=pdst;
	return ret;
}

unsigned index_gbk(const uint8_t *psrc, const uint8_t *psrc_end, unsigned nch)
{
	const uint8_t *iter=psrc;
	while(iter<psrc_end && nch>0) {
		if(*iter>GBK_SINGLE_MAX)
			iter+=2;
		else ++iter;
		--nch;
	}
	return iter>psrc_end?psrc_end-psrc:iter-psrc;
}

unsigned index_utf8(const uint8_t *psrc, const uint8_t *psrc_end, unsigned nch)
{
	const uint8_t *iter=psrc;
	while(iter<psrc_end && nch>0) {
		iter+=EXTRA_BYTES_UTF8[*iter];
		--nch;
	}
	return iter>psrc_end?psrc_end-psrc:iter-psrc;
}

}

