#include "strutil.h"
#include <cassert>
#include <cstdio>
#include <cstdarg>
#include <sstream>
#include "wyc/util/util.h"
#include "wyc/util/strcodec.h"

#ifdef _MSC_VER
#pragma warning (disable:4996)
#endif //_MSC_VER

namespace wyc
{

#define SIZE_CHAR_BUFFER 256

void copystr(std::string &ret, const wchar_t *pwstr, size_t len)
{
	if(len==0)
		len=wcslen(pwstr);
	ret.resize(len);
	for(size_t i=0; i<len; ++i) 
		ret[i]=(char)pwstr[i];
}

void copystr(std::wstring &ret, const char *pstr, size_t len)
{
	if(len==0)
		len=strlen(pstr);
	ret.resize(len);
	for(size_t i=0; i<len; ++i)
		ret[i]=(wchar_t)pstr[i];
}

bool str2wstr(std::wstring &ret, const char *pstr, size_t len)
{
	assert(pstr);
	if(len<SIZE_CHAR_BUFFER) {
		wchar_t buff[SIZE_CHAR_BUFFER];
		len=mbstowcs(buff,pstr,SIZE_CHAR_BUFFER);
		if(len==-1)
			return false;
		if(len<SIZE_CHAR_BUFFER) {
			buff[len]=0;
			ret.assign(buff,len);
			return true;
		}
		len=mbstowcs(0,pstr,0);
		if(len==-1)
			return false;
		wyc_error("to_wstring: size overflow [%d]",len);
	}
	wchar_t *pdst=new wchar_t[len+1];
	len=mbstowcs(pdst,pstr,len);
	assert(len>=0);
	pdst[len]=0;
	ret.assign(pdst,len);
	delete [] pdst;
	return true;
}

bool wstr2str(std::string &ret, const wchar_t *pwstr, size_t len)
{
	assert(pwstr);
	if(len<((SIZE_CHAR_BUFFER*3)>>1)) {
		char buff[SIZE_CHAR_BUFFER];
		len=wcstombs(buff,pwstr,SIZE_CHAR_BUFFER);
		if(len==-1)
			return false;
		if(len<SIZE_CHAR_BUFFER) {
			buff[len]=0;
			ret.assign(buff,len);
			return true;
		}
		len=wcstombs(0,pwstr,0);
		if(len==-1)
			return false;
		wyc_error("to_astring: size overflow [%d]",len);
	}
	else len<<=1;
	char *pdst=new char[len+1];
	len=wcstombs(pdst,pwstr,len);
	assert(len>=0);
	pdst[len]=0;
	ret.assign(pdst,len);
	delete [] pdst;
	return true;
}

bool str2wstr_utf8(std::wstring &ret, const char *pstr, size_t len)
{
	string_codec codec;
	if(!codec.decode_utf8(pstr,len))
		return false;
	unsigned n=codec.chars();
	ret.assign(codec.buffer(),n);
	return true;
}

bool wstr2str_utf8(std::string &ret, const wchar_t *pwstr, size_t len)
{
	uint16_t *pucs=(uint16_t*)pwstr;
	if(len==0)
		len=wcslen(pwstr);
	unsigned size=ucs_to_utf8(pucs);
	char *pdst=new char[size];
	uint8_t *pstr=(uint8_t*)pdst;
	if(ucs_to_utf8(&pstr,pstr+size,&pucs,pucs+len)!=OK) {
		delete [] pdst;
		return false;
	}
	ret.assign(pdst,size);
	delete [] pdst;
	return true;
}

void int2str(std::string &ret, int val)
{
#if (UINT_MAX > 0xffffffff) // 64bit int system: (+/-)9223372036854775807
	char buff[21];
#else // 32bit int system: (+/-)2147483647
	char buff[12];
#endif
	::itoa(val,buff,10);
	ret=buff;
}

void int2str(std::wstring &ret, int val)
{
	std::wstringstream ss;
	ss<<val;
	ret=ss.str();
}

void uint2str(std::string &ret, unsigned val)
{
#if (UINT_MAX > 0xffffffff) // 64bit int system
	char buff[22];
#else // 32bit int system: 4294967295
	char buff[11];
#endif
	::ultoa(val,buff,10);
	ret=buff;
}

void uint2str(std::wstring &ret, unsigned val)
{
	std::wstringstream ss;
	ss<<val;
	ret=ss.str();
}

void float2str(std::string &ret, float f)
{
	std::stringstream ss;
	// 默认精度不足,FLT_MAX会产生很大的误差
	ss.precision(10);
	ss<<f;
	ret=ss.str();
}

void float2str(std::wstring &ret, float f)
{
	std::wstringstream ss;
	ss.precision(10);
	ss<<f;
	ret=ss.str();
}

void hex2str(std::string &ret, unsigned val)
{
#if (UINT_MAX > 0xffffffff) // 64bit int system: 0xFFFFFFFFFFFFFFFF
	char buff[19];
#else // 32bit int system: 0xFFFFFFFF
	char buff[11];
#endif
	::sprintf(buff,"0x%X",val);
	ret=buff;
}

void hex2str(std::wstring &ret, unsigned val)
{
	std::wstringstream ss;
	ss<<"0x"<<std::hex<<std::uppercase<<val;
	ret=ss.str();
}

template<class char_t>
unsigned _str2hex(const char_t *pstr, const char_t *pend)
{
	register char_t c;
	register unsigned ret=0;
	while(pstr<pend) {
		c=*pstr++;
		if(c>=48 && c<=57) 
			ret=(ret<<4)+c-48;
		else if(c>=65 && c<=70)
			ret=(ret<<4)+c-55;
		else if(c>=97 && c<=102)
			ret=(ret<<4)+c-87;
		else
			break;
	}
	return ret;
}

bool get_word(const std::wstring &in, std::wstring &sret, wchar_t delim, size_t &begpos)
{
	if(begpos>=in.size()) {
		sret=L"";
		return false;
	}
	size_t end=in.find(delim,begpos);
	if(end==std::wstring::npos) {
		sret=in.substr(begpos);
		begpos=std::wstring::npos;
	}
	else {
		sret=in.substr(begpos,end-begpos);
		begpos=end+1;
	}
	return true;
}

bool get_word(const std::string &in, std::string &sret, char delim, size_t &begpos)
{
	if(begpos>=in.size()) {
		sret="";
		return false;
	}
	size_t end=in.find(delim,begpos);
	if(end==std::string::npos) {
		sret=in.substr(begpos);
		begpos=std::string::npos;
	}
	else {
		sret=in.substr(begpos,end-begpos);
		begpos=end+1;
	}
	return true;
}

void time2str(std::wstring &str, double t) 
{
	std::wstringstream ss;
	ss<<int(t*1000)<<"."<<int(t*100000+0.5)%100;
	str=ss.str();
}

void time2str(std::string &str, double t) 
{
	std::stringstream ss;
	ss<<int(t*1000)<<"."<<int(t*100000+0.5)%100;
	str=ss.str();
}

bool format(std::string &ret, const char *strFormat, ...)
{
	va_list arglist;
	va_start(arglist,strFormat);
	char buff[SIZE_CHAR_BUFFER];
	int len;
	len=::vsprintf_s(buff,SIZE_CHAR_BUFFER,strFormat,arglist);
	if(len<0) {
		va_end(arglist);
		return false;
	}
	if(len<SIZE_CHAR_BUFFER) {
		buff[len]=0;
		ret.assign(buff,len);
		va_end(arglist);
		return true;
	}
	len=::vsprintf_s(0,0,strFormat,arglist);
	if(len<=0)
		return false;
	wyc_error("format string: buffer overflow [%d]",len);
	char *pdst=new char[len];
	::vsprintf_s(pdst,len,strFormat,arglist);
	va_end(arglist);
	ret.assign(pdst,len);
	delete [] pdst;
	return true;
}

bool format(std::wstring &ret, const wchar_t *strFormat, ...)
{
	va_list arglist;
	va_start(arglist,strFormat);
	wchar_t buff[SIZE_CHAR_BUFFER];
	int len;
	len=::vswprintf_s(buff,SIZE_CHAR_BUFFER,strFormat,arglist);
	if(len<0) {
		va_end(arglist);
		return false;
	}
	if(len<SIZE_CHAR_BUFFER) {
		buff[len]=0;
		ret.assign(buff,len);
		va_end(arglist);
		return true;
	}
	len=::vswprintf_s(0,0,strFormat,arglist);
	if(len<=0)
		return false;
	wyc_error("format string: buffer overflow [%d]",len);
	wchar_t *pdst=new wchar_t[len];
	::vswprintf_s(pdst,len,strFormat,arglist);
	va_end(arglist);
	ret.assign(pdst,len);
	delete [] pdst;
	return true;
}

} // namespace wyc
