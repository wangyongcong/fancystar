#ifndef __HEADER_WYC_XHASH
#define __HEADER_WYC_XHASH

#include "wyc/util/hash_base.h"

namespace wyc 
{

// CRC32 hash function
unsigned hash_crc32(const char *str, unsigned len);

/*-----------------------------------------------------------------------------------------------
 *  字典
 *  key: 任意整数
 *  value: (HT_EMPTY_POINTER,HT_DUMMY_POINTER)之外的指针
------------------------------------------------------------------------------------------------*/

class xdict
{
public:
	typedef uintptr_t key_t;
	typedef void* value_t;
	typedef std::pair<key_t,value_t> entry_t;
	typedef struct
	{
		static inline size_t hash (key_t key)
		{
			return key;
		}
		static inline key_t get_key (const entry_t &item)
		{
			assert(item.second!=HT_EMPTY_POINTER && item.second!=HT_DUMMY_POINTER);
			return item.first;
		}
		static inline bool is_empty (const entry_t &item)
		{
			return HT_EMPTY_POINTER==item.second;
		}
		static inline bool is_deleted (const entry_t &item)
		{
			return HT_DUMMY_POINTER==item.second;
		}
		static inline void set_empty (entry_t &item)
		{
			item.second=HT_EMPTY_POINTER;
		}
		static inline void set_deleted (entry_t &item)
		{
			assert(item.second!=HT_EMPTY_POINTER && item.second!=HT_DUMMY_POINTER);
			item.second=HT_DUMMY_POINTER;
		}
	} hasher_t;
	typedef xhashtable<key_t,entry_t,hasher_t> table_t;
	typedef table_t::iterator iterator;
	typedef table_t::const_iterator const_iterator;
	class proxy_t {
		friend class xdict;
		table_t *m_table;
		size_t m_pos;
		key_t m_key;
		proxy_t(table_t* tab, key_t key, size_t pos) : 
			m_table(tab), m_key(key), m_pos(pos) 
			{
#ifdef _DEBUG
				assert(tab);
				if(HT_NULL_POS!=pos) {
					const entry_t &item = (*m_table)[m_pos];
					assert(!table_t::hasher_type::is_empty(item) && 
					!table_t::hasher_type::is_deleted(item));
				}
#endif // _DEBUG
			}
		// no copy constructor and assignment
		proxy_t(const proxy_t& right);
		proxy_t& operator = (const proxy_t& right);
	public:
		proxy_t& operator = (void *p) {
			if(HT_NULL_POS!=m_pos)
				(*m_table)[m_pos].second=p;
			else 
				m_pos = m_table->set_default(m_key,entry_t(m_key,p));
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
	xdict(size_t capacity=0);
	void reserve(size_t size);
	void clear();
	bool add(key_t key, value_t data);
	bool del(key_t key);
	value_t pop(key_t key, value_t def=0);
	value_t get(key_t key, value_t def=0) const;
	void set(key_t key, value_t data);
	bool contain(key_t key) const;
	size_t size() const;
	size_t capacity() const;
	proxy_t operator [] (key_t key);
	void* operator [] (key_t key) const;
	iterator begin();
	const_iterator begin() const;
	iterator end();
	const_iterator end() const;
	void set_auto_shrink(bool b);
	bool is_auto_shrink() const;
//-------------------------------------------------------------
	static bool is_valid(value_t v);
private:
	table_t m_table;
};

/*-----------------------------------------------------------------------------------------------
 *  集合
 *  value: (HT_EMPTY_POINTER,HT_DUMMY_POINTER)之外的指针
------------------------------------------------------------------------------------------------*/

class xset
{
public:
	typedef void* key_t;
	typedef void* value_t;
	typedef struct 
	{
		static inline size_t hash (key_t key)
		{
			assert(key!=HT_EMPTY_POINTER && key!=HT_DUMMY_POINTER);
			return (uintptr_t)key;
		}
		static inline key_t get_key (value_t item)
		{
			assert(item!=HT_EMPTY_POINTER && item!=HT_DUMMY_POINTER);
			return item;
		}
		static inline bool is_empty (value_t item)
		{
			return item==HT_EMPTY_POINTER;
		}
		static inline bool is_deleted (value_t item)
		{
			return item==HT_DUMMY_POINTER;
		}
		static inline void set_empty (value_t &item)
		{
			item=HT_EMPTY_POINTER;
		}
		static inline void set_deleted (value_t &item)
		{
			assert(item!=HT_EMPTY_POINTER && item!=HT_DUMMY_POINTER);
			item=HT_DUMMY_POINTER;
		}
	} hasher_t;
	typedef xhashtable<key_t,value_t,hasher_t> table_t;
	typedef table_t::const_iterator const_iterator;
//-------------------------------------------------------------
	xset(size_t capacity=0);
	void reserve(size_t size);
	void clear();
	bool add(value_t data);
	bool del(value_t data);
	bool contain(value_t data) const;
	size_t size() const;
	size_t capacity() const;
	const_iterator begin() const;
	const_iterator end() const;
	void set_auto_shrink(bool b);
	bool is_auto_shrink() const;
private:
	table_t m_table;
};

} // namespace wyc

#include "wyc/util/hash.inl"

#endif // end of __HEADER_WYC_XHASH

