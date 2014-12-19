#ifndef __INLINE_WYC_XVAR
#define __INLINE_WYC_XVAR

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4996)
#endif //_MSC_VER

namespace wyc
{

inline xvar::xvar()
{
	m_type=0;
	m_pData=0;
}

inline xvar::xvar(const xvar &var)
{
	m_type=var.m_type;
	if(m_type<XVAR_TYPE_PACKED) 
		m_pData=var.m_pData;
	else 
		m_pPacket=var.m_pPacket->clone();
}

inline xvar::xvar(int val) 
{
	m_type=XVAR_TYPE_INT;
	m_iVal=val;
}

inline xvar::xvar(unsigned val)
{
	m_type=XVAR_TYPE_UINT;
	m_iVal=val;
}

inline xvar::xvar(void *ptr)
{
	m_type=XVAR_TYPE_PTR;
	m_pData=ptr;
}

inline xvar::xvar(float val)
{
	m_type=XVAR_TYPE_FLOAT;
	m_fVal=val;
}

inline xvar::xvar(const char *str)
{
	m_type=0;
	m_pData=0;
	*this=str;
}

inline xvar::xvar(const wchar_t *str)
{
	m_type=0;
	m_pData=0;
	*this=str;
}

inline xvar::xvar(const std::string &str)
{
	m_type=0;
	m_pData=0;
	*this=str;
}

inline xvar::xvar(const std::wstring &str)
{
	m_type=0;
	m_pData=0;
	*this=str;
}

inline xvar::~xvar()
{
	clear();
}

inline void xvar::clear()
{
	clear_packet();
	m_type=0;
	m_pData=0;
}

inline void xvar::clear_packet() 
{
	if(m_type>=XVAR_TYPE_PACKED) {
		xpackdata::free_packed_data(m_pPacket);
	}
}

inline xvar& xvar::operator = (const xvar &var)
{
	clear_packet();
	m_type=var.m_type;
	if(m_type<XVAR_TYPE_PACKED) 
		m_pData=var.m_pData;
	else 
		m_pPacket=var.m_pPacket->clone();
	return *this;
}

template<typename T>
xvar& xvar::operator = (const T &val)
{
	clear_packet();
	m_type=XVAR_TYPE_PACKED;
	m_pPacket=xpackdata::alloc_packed_data(sizeof(T));
	new(m_pPacket->buffer()) T(val);
	return *this;
}

inline xvar& xvar::operator = (int val)
{
	clear_packet();
	m_type=XVAR_TYPE_INT;
	m_iVal=val;
	return *this;
}

inline xvar& xvar::operator = (unsigned val)
{
	clear_packet();
	m_type=XVAR_TYPE_UINT;
	m_uiVal=val;
	return *this;
}

inline xvar& xvar::operator = (float val)
{
	clear_packet();
	m_type=XVAR_TYPE_FLOAT;
	m_fVal=val;
	return *this;
}

inline xvar& xvar::operator = (void* ptr)
{
	clear_packet();
	m_type=XVAR_TYPE_PTR;
	m_pData=ptr;
	return *this;
}

template <> 
inline xvar& xvar::operator = <std::string> (const std::string &str)
{
	clear_packet();
	m_type=XVAR_TYPE_STR;
	unsigned size=(unsigned)str.size()+1;
	m_pPacket=xpackdata::alloc_packed_data(size);
	char* pstr=(char*)m_pPacket->buffer();
	strcpy(pstr,str.c_str());
	return *this;
}

template <> 
inline xvar& xvar::operator = <std::wstring> (const std::wstring &str)
{
	clear_packet();
	m_type=XVAR_TYPE_WSTR;
	unsigned size=(unsigned)str.size()+1;
	m_pPacket=xpackdata::alloc_packed_data(sizeof(wchar_t)*size);
	wchar_t* pstr=(wchar_t*)m_pPacket->buffer();
	wcscpy(pstr,str.c_str());
	return *this;
}

template<typename T>
inline bool xvar::get(T &data) const 
{
	if(m_type!=XVAR_TYPE_PACKED || m_pPacket->size()!=sizeof(T))
		return false;
	data=*(const T*)(m_pPacket->buffer());
	return true;
}

template<> 
inline bool xvar::get<int> (int &ival) const
{
	if(m_type!=XVAR_TYPE_INT)
		return false;
	ival=m_iVal;
	return true;
}

template<> 
inline bool xvar::get<unsigned> (unsigned &uval) const
{
	if(m_type!=XVAR_TYPE_UINT)
		return false;
	uval=m_uiVal;
	return true;
}

template<> bool xvar::get<float> (float &fval) const
{
	if(m_type!=XVAR_TYPE_FLOAT)
		return false;
	fval=m_fVal;
	return true;
}

template<typename T>
inline bool xvar::get(T *&ptr) const
{
	if(m_type!=XVAR_TYPE_PTR)
		return false;
	ptr=(T*)m_pData;
	return true;
}

template<> 
inline bool xvar::get<std::string> (std::string &str) const
{
	if(m_type!=XVAR_TYPE_STR)
		return false;
	str=(const char*)(m_pPacket->buffer());
	return true;
}

template<> 
inline bool xvar::get<std::wstring> (std::wstring &str) const
{
	if(m_type!=XVAR_TYPE_WSTR)
		return false;
	str=(const wchar_t*)(m_pPacket->buffer());
	return true;
}

template<typename T>
const T* xvar::packet() const
{
	if(m_type!=XVAR_TYPE_PACKED || m_pPacket->size()!=sizeof(T))
		return 0;
	return (const T*)m_pPacket->buffer();
}

template<typename T>
T* xvar::packet() 
{
	if(m_type!=XVAR_TYPE_PACKED || m_pPacket->size()!=sizeof(T))
		return 0;
	if(!m_pPacket->unique()) {
		xpackdata *pNewPack=xpackdata::alloc_packed_data(m_pPacket->size());
		new(pNewPack->buffer()) T(*(T*)m_pPacket->buffer());
		xpackdata::free_packed_data(m_pPacket);
		m_pPacket=pNewPack;
	}
	return (T*)m_pPacket->buffer();
}

template<typename T>
inline T* xvar::ptr() const
{
	return (m_type==XVAR_TYPE_PTR) ? (T*)m_pData : 0;
}

inline xvar::operator int () const
{
	if (m_type==XVAR_TYPE_INT || m_type==XVAR_TYPE_UINT) 
		return m_iVal;
	else if(m_type==XVAR_TYPE_FLOAT)
		return int(m_fVal);
	return 0;
}

inline xvar::operator unsigned () const
{
	if (m_type==XVAR_TYPE_UINT || m_type==XVAR_TYPE_INT) 
		return m_uiVal;
	else if(m_type==XVAR_TYPE_FLOAT)
		return unsigned(m_fVal);
	return 0;
}

inline xvar::operator float () const
{
	if(m_type==XVAR_TYPE_FLOAT) 
		return m_fVal;
	else if(m_type==XVAR_TYPE_INT)
		return float(m_iVal);
	else if(m_type==XVAR_TYPE_UINT)
		return float(m_uiVal);
	return 0;
}

inline xvar::operator void* () const
{
	return (m_type==XVAR_TYPE_PTR) ? m_pData : 0;
}

inline xvar::operator const char* () const
{
	return (m_type==XVAR_TYPE_STR) ? (const char*)m_pPacket->buffer() : 0;
}

inline xvar::operator const wchar_t* () const
{
	return (m_type==XVAR_TYPE_WSTR) ? (const wchar_t*)m_pPacket->buffer() : 0;
}

inline unsigned xvar::size() const
{
	return (m_type>=XVAR_TYPE_PACKED) ? m_pPacket->size() : 0;
}

//==================================================================================

inline xargs::xargs(int val) : m_array(1) 
{
	m_array.back()=val;
	m_readpos=0;
}

inline xargs::xargs(unsigned val) : m_array(1) 
{
	m_array.back()=val;
	m_readpos=0;
}

inline xargs::xargs(void *ptr) : m_array(1) 
{
	m_array.back()=ptr;
	m_readpos=0;
}

inline xargs::xargs(float val) : m_array(1) 
{
	m_array.back()=val;
	m_readpos=0;
}

inline xargs::xargs(const char *str) : m_array(1) 
{
	m_array.back()=str;
	m_readpos=0;
}

inline xargs::xargs(const wchar_t *str) : m_array(1) 
{
	m_array.back()=str;
	m_readpos=0;
}

inline xargs::xargs(const std::string &str) : m_array(1) 
{
	m_array.back()=str;
	m_readpos=0;
}

inline xargs::xargs(const std::wstring &str) : m_array(1) 
{
	m_array.back()=str;
	m_readpos=0;
}

inline xargs::~xargs() 
{
	m_array.clear(); 
}

inline void xargs::clear() 
{
	m_array.clear();
}

inline void xargs::resize(unsigned size)
{
	m_array.resize(size);
}

inline void xargs::reserve(unsigned size)
{
	m_array.reserve(size);
}

inline unsigned xargs::size() const 
{
	return (unsigned)m_array.size();
}

inline xvar& xargs::operator [] (unsigned pos)
{
	return m_array[pos];
}

inline const xvar& xargs::operator [] (unsigned pos) const 
{
	return m_array[pos];
}

inline void xargs::push_back(const xvar &var) 
{
	m_array.push_back(var);
}

inline void xargs::pop_back() 
{
	m_array.pop_back();
}

template<typename T>
inline xargs& xargs::operator << (const T &data)
{
	m_array.push_back(xvar());
	m_array.back()=data;
	return *this;
}

template<> inline xargs& xargs::operator << <xvar> (const xvar &val) {
	m_array.push_back(val);
	return *this;
}

inline xargs& xargs::operator << (int val) {
	m_array.push_back(xvar(val));
	return *this;
}

inline xargs& xargs::operator << (unsigned val) {
	m_array.push_back(xvar(val));
	return *this;
}

inline xargs& xargs::operator << (float val) {
	m_array.push_back(xvar(val));
	return *this;
}

template<typename T>
inline xargs& xargs::operator << (T *ptr)
{
	m_array.push_back(xvar(ptr));
	return *this;
}

template<> 
inline xargs& xargs::operator << <std::string> (const std::string &str)
{
	m_array.push_back(xvar(str));
	return *this;
}

template<> 
inline xargs& xargs::operator << <std::wstring> (const std::wstring &str)
{
	m_array.push_back(xvar(str));
	return *this;
}

template<typename T>
inline const xargs& xargs::operator >> (T &data) const {
	if(m_readpos<m_array.size()) {
		if(m_array[m_readpos++].get<T>(data))
			return *this;
	}
	m_readpos=unsigned(-1);
	return *this;
}

template<> 
inline const xargs& xargs::operator >> <xvar> (xvar &val) const
{
	if(m_readpos<m_array.size()) 
		val=m_array[m_readpos++];
	else m_readpos=unsigned(-1);
	return *this;
}

#define READ_SIMPLE_DATA(data)\
	if(m_readpos<m_array.size()) {\
		if(m_array[m_readpos++].get(data))\
			return *this;\
	}\
	m_readpos=unsigned(-1);\
	return *this;


template<> 
inline const xargs& xargs::operator >> <int> (int &ival) const
{
	READ_SIMPLE_DATA(ival);
}

template<> 
inline const xargs& xargs::operator >> <unsigned> (unsigned &uval) const
{
	READ_SIMPLE_DATA(uval);
}

template<> 
inline const xargs& xargs::operator >> <float> (float &fval) const
{
	READ_SIMPLE_DATA(fval);
}

template<typename T>
inline const xargs& xargs::operator >> (T* &ptr) const 
{
	READ_SIMPLE_DATA(ptr);
}

template<> 
inline const xargs& xargs::operator >> <std::string> (std::string &str) const
{
	READ_SIMPLE_DATA(str);
}

template<> 
inline const xargs& xargs::operator >> <std::wstring> (std::wstring &str) const
{
	READ_SIMPLE_DATA(str);
}

}; // namespace wyc

#ifdef _MSC_VER
#pragma warning (pop)
#endif //_MSC_VER

#endif // __INLINE_WYC_XVAR

