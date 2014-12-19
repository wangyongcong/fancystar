#include <cstring>
#include <cassert>
#include "wyc/util/strcodec.h"

#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif //_MSC_VER

namespace wyc
{

string_codec::string_codec() 
{
	m_nchar=0;
	m_pustr=0;
}

string_codec::~string_codec() 
{
	if(m_pustr) 
		delete [] m_pustr;
}

bool string_codec::decode_ansi(const char *pstr, unsigned)
{
	assert(pstr);
	m_nchar=mbstowcs(0,pstr,0);
	if(m_nchar==unsigned(-1))
		return false;
	pstrucs pwstr=new ucschar_t[m_nchar+1];
	mbstowcs((wchar_t*)pwstr,pstr,m_nchar);
	pwstr[m_nchar]=0;
	if(m_pustr)
		delete [] m_pustr;
	m_pustr=pwstr;
	return true;
}

bool string_codec::decode_utf8(const char *pstr, unsigned len)
{
	assert(pstr);
	if(len==0)
		len=strlen(pstr);
	m_nchar=chars_utf8((uint8_t*)pstr);
	if(!len || !m_nchar)
		return false;
	pstrucs pwstr=new ucschar_t[m_nchar+1];
	pstrucs pwdst=pwstr;
	uint8_t *psrc=(uint8_t*)pstr;
	CONVERT_RET ret=utf8_to_ucs(&pwdst,pwdst+m_nchar,&psrc,psrc+len);
	if(ret!=OK) {
		delete [] pwstr;
		return false;
	}
	*pwdst=0;
	if(m_pustr) 
		delete [] m_pustr;
	m_pustr=pwstr;
	return true;
}

template<typename T>
unsigned ucslen(const T *pstr) 
{
	register unsigned len=0;
	register const T *piter=pstr;
	while(*piter++!=0)
		++len;
	return len;
}

bool string_codec::decode_ucs2(const uint16_t *pstr, unsigned len)
{
	assert(pstr);
	if(len==0) 
		len=ucslen(pstr);
	if(!len)
		return false;
	register uint16_t *psrc=(uint16_t*)pstr;
	register pstrucs pdst=new ucschar_t[len+1];
	register pstrucs pdst_end=pdst+len;
	while(pdst<pdst_end)
		*pdst++=(ucschar_t)(*psrc++);
	*pdst=0;
	if(m_pustr)
		delete [] m_pustr;
	m_pustr=pdst;
	return true;
}

bool string_codec::decode_ucs4(const uint32_t *pstr, unsigned len)
{
	assert(pstr);
	if(len==0) 
		len=ucslen(pstr);
	if(!len)
		return false;
	register pstrucs pdst=new ucschar_t[len+1];
#if XSTR_WCHAR_ENCODING==XSTR_ENCODE_UCS2
	register pstrucs pdst_end=pdst+len;
	register uint32_t *psrc=(uint32_t*)pstr;
	register uint32_t ch;
	while(pdst<pdst_end) {
		ch=*psrc++;
		if(ch>MAX_UCS2) 
			ch=REPLACEMENT_CHAR;
		*pdst++=(ucschar_t)ch;
	}
	*pdst=0;
#elif XSTR_WCHAR_ENCODING==XSTR_ENCODE_UCS4
	memcpy(pdst,pstr,len);
	pdst[len]=0;
#endif
	if(m_pustr)
		delete [] m_pustr;
	m_pustr=pdst;
	return true;
}

bool string_codec::encode_utf8(char *pdst, unsigned size)
{
	pstrucs pwstr=m_pustr;
	uint8_t *pstr=(uint8_t*)pdst;
	CONVERT_RET ret=ucs_to_utf8(&pstr,pstr+size,&pwstr,pwstr+m_nchar);
	if(ret!=OK)
		return false;
	return true;
}

bool string_codec::encode_ucs2(uint16_t *pdst, unsigned size)
{
	if(size>m_nchar)
		size=m_nchar;
#if XSTR_WCHAR_ENCODING==XSTR_ENCODE_UCS2
	memcpy(pdst,m_pustr,size<<1);
#elif XSTR_WCHAR_ENCODING==XSTR_ENCODE_UCS4
	register uint16_t *piter=(uint16_t*)pdst;
	register pstrucs pbeg=m_pustr;
	register pstrucs pend=m_pustr+size;
	register uint32_t ch;
	while(pbeg<pend) {
		ch=*pbeg++;
		if(ch>MAX_UCS2)
			ch=REPLACEMENT_CHAR;
		*piter++=(uint16_t)ch;
	}
#endif
	return true;
}

bool string_codec::encode_ucs4(uint32_t *pdst, unsigned size)
{
	if(size>m_nchar)
		size=m_nchar;
	register uint32_t *piter=pdst;
	register pstrucs pbeg=m_pustr;
	register pstrucs pend=m_pustr+size;
	while(pbeg<pend)
		*piter++=(uint32_t)(*pbeg++);
	return true;
}

} // namespace wyc



