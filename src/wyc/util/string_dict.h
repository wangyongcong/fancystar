#ifndef __HEADER_WYC_STRING_DICT
#define __HEADER_WYC_STRING_DICT

#include <cassert>
#include <string>
#include "wyc/util/hash_base.h"

namespace wyc
{

template<typename T, typename OP>
class xbasic_string_dict
{
public:
	typedef T char_t;
	typedef std::basic_string<char_t> string_t;
	struct key_t {
		const char_t *str;
		unsigned hash;
		inline bool operator == (const key_t& r) const {
			return hash==r.hash && 0==OP::compare_string(str,r.str);
		}
	};
	typedef void* value_t;
	typedef std::pair<key_t,value_t> entry_t;
	typedef struct
	{
		static inline size_t hash (const key_t &key)
		{
			return key.hash;
		}
		static inline const key_t& get_key (const entry_t &item)
		{
			return item.first;
		}
		static inline bool is_empty (const entry_t &item)
		{
			return 0==item.first.str;
		}
		static inline bool is_deleted (const entry_t &item)
		{
			return HT_DUMMY_POINTER==(void*)item.first.str;
		}
		static inline void set_empty (entry_t &item)
		{
			item.first.str=0;
		}
		static inline void set_deleted (entry_t &item)
		{
			item.first.str=(char_t*)HT_DUMMY_POINTER;
		}
	} hasher_t;
	typedef xhashtable<key_t,entry_t,hasher_t> table_t;
	typedef typename table_t::iterator iterator;
	typedef typename table_t::const_iterator const_iterator;
	class proxy_t {
		friend class xbasic_string_dict;
		table_t *m_table;
		size_t m_pos;
		key_t m_key;
		bool m_ownership;
		proxy_t(table_t* tab, key_t key, size_t pos, size_t len=0) : 
			m_table(tab), m_key(key), m_pos(pos) 
			{
				assert(tab);
				if(HT_NULL_POS==pos) {
					m_key.str = OP::copy_string(key.str,len);
					m_ownership = true;
				}
				else {
					m_ownership = false;
#ifdef _DEBUG
					const entry_t &item = (*m_table)[m_pos];
					assert(!table_t::hasher_type::is_empty(item) && 
					!table_t::hasher_type::is_deleted(item));
#endif // _DEBUG
				}
			}
		proxy_t(const proxy_t& right);
		proxy_t& operator = (const proxy_t& right);
	public:
		~proxy_t () {
			if(m_ownership)
			{
				if(m_key.str)
					OP::free_string(m_key.str);
			}
		}
		proxy_t& operator = (void *p) {
			if(HT_NULL_POS!=m_pos)
				(*m_table)[m_pos].second=p;
			else {
				m_pos=m_table->set_default(m_key,entry_t(m_key,p));
				m_ownership = HT_NULL_POS==m_pos;
			}
			return *this;
		}
		template<typename T>
		operator T* () const {
			if(HT_NULL_POS==m_pos)
				return 0;
			return (T*)(*m_table)[m_pos].second;
		}
	};
//-------------------------------------------------------------
	xbasic_string_dict(size_t capacity=0);
	~xbasic_string_dict();
	void reserve(size_t size);
	void clear();
	void* operator[] (const char_t *s) const;
	proxy_t operator[] (const char_t *s);
	void* operator[] (const string_t &str) const;
	proxy_t operator[] (const string_t &str);
//-------------------------------------------------------------
#ifdef STRING_DICT_MEMORY_DEBUG
	static inline unsigned detect_leaks() {
		return OP::leaks();
	}
#endif
private:
	table_t m_table;
};

struct xastring_op 
{
	static inline int compare_string (const char *left, const char *right)
	{
		return strcmp(left,right);
	}

	static inline const char* copy_string (const char *s, size_t len=0)
	{
		if(!len) len=strlen(s);
		len+=1;
		char *new_s = new char[len];
		strcpy_s(new_s,len,s);
#ifdef STRING_DICT_MEMORY_DEBUG
		ms_mem_used += sizeof(char)*len;
#endif
		return new_s;
	}

	static inline void free_string(const char *s)
	{
#ifdef STRING_DICT_MEMORY_DEBUG
		size_t len = strlen(s)+1;
		ms_mem_used -= sizeof(char)*len;
#endif
		delete [] (char*)(s);
	}

#ifdef STRING_DICT_MEMORY_DEBUG
	static inline unsigned leaks()
	{
		return ms_mem_used;
	}
private:
	static unsigned ms_mem_used;
#endif
};

struct xwstring_op
{
	static inline int compare_string (const wchar_t *left, const wchar_t *right)
	{
		return wcscmp(left,right);
	}

	static inline const wchar_t* copy_string (const wchar_t *s, size_t len=0)
	{
		if(!len) len=wcslen(s);
		len+=1;
		wchar_t *new_s = new wchar_t[len];
		wcscpy_s(new_s,len,s);
#ifdef STRING_DICT_MEMORY_DEBUG
		ms_mem_used += sizeof(char)*len;
#endif
		return new_s;
	}

	static inline void free_string(const wchar_t *s)
	{
#ifdef STRING_DICT_MEMORY_DEBUG
		size_t len = wcslen(s)+1;
		ms_mem_used -= sizeof(char)*len;
#endif
		delete [] (wchar_t*)(s);
	}

#ifdef STRING_DICT_MEMORY_DEBUG
	static inline unsigned leaks()
	{
		return ms_mem_used;
	}
private:
	static unsigned ms_mem_used;
#endif
};

#define STRING_DICT_MEMORY_DEBUG_INIT() \
	unsigned xastring_op::ms_mem_used=0;\
	unsigned xwstring_op::ms_mem_used=0;

typedef xbasic_string_dict<char,xastring_op> xstring_dict;
typedef xbasic_string_dict<wchar_t,xwstring_op> xwstring_dict;

template<typename T, typename OP>
xbasic_string_dict<T,OP>::xbasic_string_dict(size_t capacity) : m_table(capacity)
{
}

template<typename CHAR, typename OP>
xbasic_string_dict<CHAR,OP>::~xbasic_string_dict()
{
	clear();
}

template<typename CHAR, typename OP>
inline void xbasic_string_dict<CHAR,OP>::reserve(size_t size)
{
	m_table.reserve(size);
}

template<typename CHAR, typename OP>
void xbasic_string_dict<CHAR,OP>::clear()
{
	for(iterator iter=m_table.begin(), end=m_table.end(); iter!=end; ++iter)
	{
		entry_t &item = *iter;
		OP::free_string((char_t*)item.first.str);
	}
	m_table.clear();
}

template<typename CHAR, typename OP>
void* xbasic_string_dict<CHAR,OP>::operator [] (const char_t *s) const
{
	entry_t item;
	item.first.str=s;
	item.first.hash=strhash(s);
	size_t pos= m_table.find(item.first);
	if(HT_NULL_POS==pos)
		return 0;
	return m_table[pos].second;
}

template<typename CHAR, typename OP>
typename xbasic_string_dict<CHAR,OP>::proxy_t 
	xbasic_string_dict<CHAR,OP>::operator [] (const char_t *s)
{
	entry_t item;
	item.first.str=s;
	item.first.hash=strhash(s);
	size_t pos=m_table.find(item.first);
	return proxy_t(&m_table,item.first,pos);
}

template<typename CHAR, typename OP>
inline void* xbasic_string_dict<CHAR,OP>::operator[] (const string_t &str) const
{
	return this->operator[](str.c_str());
}

template<typename CHAR, typename OP>
inline typename xbasic_string_dict<CHAR,OP>::proxy_t 
	xbasic_string_dict<CHAR,OP>::operator[] (const string_t &str)
{
	entry_t item;
	item.first.str=str.c_str();
	item.first.hash=strhash(str.c_str());
	size_t pos=m_table.find(item.first);
	return proxy_t(&m_table,item.first,pos,str.length());
}


} // namespace wyc

#endif // __HEADER_WYC_STRING_DICT

