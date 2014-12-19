#ifndef __HEADER_WYC_STRUTIL
#define __HEADER_WYC_STRUTIL

#include <string>

namespace wyc
{
void copystr(std::string &ret, const wchar_t *pwstr, size_t len=0);
void copystr(std::wstring &ret, const char *pstr, size_t len=0);
void copystr(std::string &ret, const std::wstring &wstr);
void copystr(std::wstring &ret, const std::string &str);

bool str2wstr(std::wstring &ret, const char *pstr, size_t len=0);
bool str2wstr(std::wstring &ret, const std::string &str);
bool wstr2str(std::string &ret, const wchar_t *pwstr, size_t len=0);
bool wstr2str(std::string &ret, const std::wstring &wstr);

bool str2wstr_utf8(std::wstring &ret, const char *pstr, size_t len=0);
bool str2wstr_utf8(std::wstring &ret, const std::string &str);
bool wstr2str_utf8(std::string &ret, const wchar_t *pwstr, size_t len=0);
bool wstr2str_utf8(std::string &ret, const std::wstring &wstr);

void int2str(std::string &ret, int val);
void int2str(std::wstring &ret, int val);
int str2int(const std::string &str);
int str2int(const std::wstring &str);

void uint2str(std::string &ret, unsigned val);
void uint2str(std::wstring &ret, unsigned val);
unsigned str2uint(const std::string &str);
unsigned str2uint(const std::wstring &str);

void float2str(std::string &ret, float f);
void float2str(std::wstring &ret, float f);
float str2float(const std::string &str);
float str2float(const std::wstring &str);

unsigned str2hex(const std::wstring &str);
unsigned str2hex(const std::string &str);
void hex2str(std::string &ret, unsigned val);
void hex2str(std::wstring &ret, unsigned val);

void time2str(std::string &str, double t);
void time2str(std::wstring &str, double t);

void to_lower(std::string &str);
void to_upper(std::string &str);
void to_lower(std::wstring &str);
void to_upper(std::wstring &str);

void strip(std::string &str);
void strip(std::wstring &str);
void lstrip(std::string &str);
void lstrip(std::wstring &str);
void rstrip(std::string &str);
void rstrip(std::wstring &str);

bool get_word(const std::string &in, std::string &sret, char delim, size_t &begpos);
bool get_word(const std::wstring &in, std::wstring &sret, wchar_t delim, size_t &begpos);

bool format(std::string &ret, const char *strFormat, ...);
bool format(std::wstring &ret, const wchar_t *strFormat, ...);

template<typename char_t>
bool starts_with(const char_t *s, const char_t *prefix)
{
	register char_t c=*prefix;
	while(c) {
		if(c!=*s)
			return false;
		++s;
		++prefix;
		c=*prefix;
	}
	return true;
}

template<typename char_t>
bool ends_with(const char_t *s, const char_t *postfix)
{
	int off=strlen(s)-strlen(postfix);
	if(off<0) 
		return false;
	return starts_with(s+off,postfix);
}

} // namespace wyc

#include "wyc/util/strutil.inl"

#endif // __HEADER_WYC_STRUTIL

