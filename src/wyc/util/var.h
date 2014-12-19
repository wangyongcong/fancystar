#ifndef __HEADER_WYC_XVAR
#define __HEADER_WYC_XVAR

#include <vector>
#include <string>
#include "wyc/basedef.h"

namespace wyc 
{

class xpackdata
{
	unsigned m_nCounter;
	unsigned m_nSize;
	uint8_t m_pData[1];
public:
	static xpackdata* alloc_packed_data(unsigned size);
	static void free_packed_data(xpackdata *pData);
#ifdef _DEBUG
	static unsigned packet_num();
#endif
	inline xpackdata* clone() 
	{
		m_nCounter+=1;
		return this;
	}
	inline unsigned size() const
	{
		return m_nSize;
	}
	inline uint8_t* buffer() 
	{
		return m_pData;
	}
	inline bool unique() const 
	{
		return m_nCounter==1;
	}
private:
	xpackdata();
	xpackdata(const xpackdata &pd);
	xpackdata& operator = (const xpackdata &pd);
};

class xvar
{
	int m_type;
	union {
		int m_iVal;
		unsigned m_uiVal;
		float m_fVal;
		char *m_pStr;
		void *m_pData;
		xpackdata *m_pPacket;
	};
public:
	enum XVAR_TYPE {
		XVAR_TYPE_UNKNOWN=0,
		XVAR_TYPE_INT,
		XVAR_TYPE_UINT,
		XVAR_TYPE_FLOAT,
		XVAR_TYPE_PTR,
		XVAR_TYPE_PACKED,
		XVAR_TYPE_STR,
		XVAR_TYPE_WSTR,
		XVAR_TYPE_NUM
	};
	xvar();
	xvar(const xvar &var);
	xvar(int val);
	xvar(unsigned val);
	xvar(void *ptr);
	xvar(float val);
	xvar(const char *str);
	xvar(const wchar_t *str);
	xvar(const std::string &str);
	xvar(const std::wstring &str);
	~xvar();

	xvar& operator = (const xvar &var);
	xvar& operator = (int val);
	xvar& operator = (unsigned val);
	xvar& operator = (float val);
	xvar& operator = (void* ptr);
	xvar& operator = (const char* str);
	xvar& operator = (const wchar_t* str);
	// 因为无法保存T的类型信息,在析构时也就无法调用T的析构函数
	// 所以类型T必须为无需析构函数的简单类型
	// 否则就要特例化并添加新的类型支持
	// 例如: std::string
	template<typename T>
		xvar& operator = (const T &val);
	template<> xvar& operator = <std::string> (const std::string &str);
	template<> xvar& operator = <std::wstring> (const std::wstring &str);

	operator int () const;
	operator unsigned () const;
	operator float () const;
	operator void* () const;
	operator const char* () const;
	operator const wchar_t* () const;
	unsigned size() const;
	template<typename T>
		T* ptr() const;
	template<typename T>
		const T* packet() const;
	template<typename T>
		T* packet();
	template<typename T>
		bool get(T &data) const;
	template<> bool get<int> (int &ival) const;
	template<> bool get<unsigned> (unsigned &uval) const;
	template<> bool get<float> (float &fval) const;
	template<typename T>
		bool get(T *&ptr) const;
	template<> bool get<std::string> (std::string &str) const;
	template<> bool get<std::wstring> (std::wstring &str) const;

	void set(const void *pdata, size_t size);
	bool get(void *pdata, size_t size) const;

	inline int type() const {
		return m_type;
	}
	inline char name() const {
		static char s_fmts[XVAR_TYPE_NUM+1]="?dufpbsw";
		return s_fmts[m_type];
	}
	inline bool empty() const {
		return m_type==0;
	}
	inline bool is_type(XVAR_TYPE t) const {
		return m_type==t;
	}
	void clear();
protected:
	void clear_packet();
};

class xargs
{
	std::vector<xvar> m_array;
	mutable unsigned m_readpos;
	enum {
		XVA_GOOD=0,
		XVA_EOF=0x01,
		XVA_READ_FAILED=0x02
	};
public:
	xargs() {}
	xargs(int val);
	xargs(unsigned val);
	xargs(void *ptr);
	xargs(float val);
	xargs(const char *str);
	xargs(const wchar_t *str);
	xargs(const std::string &str);
	xargs(const std::wstring &str);
	~xargs();
	void clear();
	void resize(unsigned size);
	void reserve(unsigned size);
	unsigned size() const;
	xvar& operator [] (unsigned pos);
	const xvar& operator [] (unsigned pos) const;
	void push_back(const xvar &var);
	void pop_back();
	void extend(const wyc::xargs &args);
	void extend(const wyc::xargs &args, unsigned beg, unsigned end=-1);

	xargs& operator << (int val);
	xargs& operator << (unsigned val);
	xargs& operator << (float val);
	template<typename T>
		xargs& operator << (T *ptr);
	template<typename T>
		xargs& operator << (const T &data);
	template<> xargs& operator << <xvar> (const xvar &val);
	template<> xargs& operator << <> (const std::string &str);
	template<> xargs& operator << <> (const std::wstring &str);

	template<typename T>
		const xargs& operator >> (T &data) const;
	template<typename T>
		const xargs& operator >> (T* &ptr) const;
	template<> const xargs& operator >> <xvar> (xvar &val) const;
	template<> const xargs& operator >> <int> (int &ival) const;
	template<> const xargs& operator >> <unsigned> (unsigned &ival) const;
	template<> const xargs& operator >> <float> (float &fval) const;
	template<> const xargs& operator >> <std::string> (std::string &str) const;
	template<> const xargs& operator >> <std::wstring> (std::wstring &str) const;

	inline operator bool() const {
		return m_readpos<=m_array.size();
	}
	inline bool eof() const {
		return m_readpos==m_array.size();
	}
	inline bool fail() const {
		return m_readpos>m_array.size();
	}
	inline void reset() const {
		m_readpos=0;
	}
	bool check(const char *fmt) const;
	const char* str() const;
	bool scanf(const char *fmt,...);
	bool printf(const char *fmt,...);
};

} // namespace wyc

#include "wyc/util/var.inl"

#endif // end of __HEADER_WYC_XVAR

