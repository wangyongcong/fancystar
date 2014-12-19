//************************************************************
// str_util inline function implementation
//************************************************************

#ifndef __INLINE_WYC_STRUTIL
#define __INLINE_WYC_STRUTIL

namespace wyc
{

inline void copystr(std::string &ret, const std::wstring &wstr)
{
	copystr(ret,wstr.c_str(),wstr.size());
}

inline void copystr(std::wstring &ret, const std::string &str)
{
	copystr(ret,str.c_str(),str.size());
}

inline bool str2wstr(std::wstring &ret, const std::string &str) 
{
	return str2wstr(ret,str.c_str(),str.size());
}

inline bool wstr2str(std::string &ret, const std::wstring &wstr) 
{
	return wstr2str(ret,wstr.c_str(),wstr.size());
}

inline bool str2wstr_utf8(std::wstring &ret, const std::string &str) 
{
	return str2wstr_utf8(ret,str.c_str(),str.size());
}

inline bool wstr2str_utf8(std::string &ret, const std::wstring &wstr)
{
	return wstr2str_utf8(ret,wstr.c_str(),wstr.size());
}

inline int str2int(const std::string &str)
{
	return atoi(str.c_str());
}

inline int str2int(const std::wstring &str)
{
	wchar_t *pend;
	return ::wcstol(str.c_str(),&pend,10);
}

inline unsigned str2uint(const std::string &str)
{
	char *pend;
	return ::strtoul(str.c_str(),&pend,10);
}

inline unsigned str2uint(const std::wstring &str)
{
	wchar_t *pend;
	return ::wcstoul(str.c_str(),&pend,10);
}

inline float str2float(const std::string &str)
{
	return float(::atof(str.c_str()));
}

inline float str2float(const std::wstring &str)
{
	wchar_t *pend;
	return float(::wcstod(str.c_str(),&pend));
}

inline unsigned str2hex(const std::wstring &str)
{
	wchar_t *pend;
	return ::wcstoul(str.c_str(),&pend,16);
}

inline unsigned str2hex(const std::string &str)
{
	char *pend;
	return ::strtoul(str.c_str(),&pend,16);
}

template<class str_t>
void _lower_case(str_t &str)
{
	size_t pos=0, len=str.size();
	while(pos<len) {
		str_t::value_type &c=str[pos++];
		if(c>='A'&&c<='Z')
			c+=32;
	}
}

template<class str_t>
void _upper_case(str_t &str)
{
	size_t pos=0, len=str.size();
	while(pos<len) {
		str_t::value_type &c=str[pos++];
		if(c>='a'&&c<='z')
			c-=32;
	}
}

inline void to_lower(std::string &str) 
{
	_lower_case<std::string>(str);
}

inline void to_upper(std::string &str) 
{
	_upper_case<std::string>(str);
}

inline void to_lower(std::wstring &str) 
{
	_lower_case<std::wstring>(str);
}

inline void to_upper(std::wstring &str) 
{
	_upper_case<std::wstring>(str);
}

template<typename str_t>
void _strip(str_t &str)
{
	size_t beg=0, end=str.size();
	while(beg<end) {
		if(!::isspace(str[beg]))
			break;
		++beg;
	}
	while(end>beg) {
		if(!::isspace(str[--end])) {
			break;
		}
	}
	if(beg==end)
		str.clear();
	else
		str=str.substr(beg,end-beg+1);
}

template<typename str_t>
void _lstrip(str_t &str)
{
	size_t beg=0, end=str.size();
	while(beg<end) {
		if(!::isspace(str[beg]))
			break;
		++beg;
	}
	if(beg==end)
		str.clear();
	else
		str=str.substr(beg,end-beg);
}

template<typename str_t>
void _rstrip(str_t &str)
{
	size_t beg=0, end=str.size();
	while(end>beg) {
		if(!::isspace(str[--end])) 
			break;
	}
	if(beg==end)
		str.clear();
	else
		str=str.substr(beg,end-beg+1);
}

inline void strip(std::string &str) {
	_strip<std::string>(str);
}

inline void strip(std::wstring &str) {
	_strip<std::wstring>(str);
}

inline void lstrip(std::string &str) {
	_lstrip<std::string>(str);
}

inline void lstrip(std::wstring &str) {
	_lstrip<std::wstring>(str);
}

inline void rstrip(std::string &str) {
	_rstrip<std::string>(str);
}

inline void rstrip(std::wstring &str) {
	_rstrip<std::wstring>(str);
}

}; // namespace wyc

#endif // __INLINE_WYC_STRUTIL

