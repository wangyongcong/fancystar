#ifndef __HEADER_WYC_WSBUCKET
#define __HEADER_WYC_WSBUCKET

#include <cassert>
#include <iterator>
#include <new>
#include "wyc/basedef.h"
#include "wyc/config.h"

namespace wyc
{

// write static bucket

template<class T>
class xwsbucket 
{
	struct BUCKET {
		size_t size;
		T data[1];
	};
	BUCKET *m_pbuckets, *m_ptail;
	T *m_cursor;
	size_t m_pagesz;
	size_t m_size;
public:
	class iterator;
	class const_iterator;
	xwsbucket(size_t page_size=32);
	~xwsbucket();
	void push_back(const T &elem);
	T* push_back();
	void clear();
	void set_page_size(unsigned sz);
	size_t page_size() const;
	size_t size() const;
	iterator begin();
	iterator end();
	const_iterator begin() const;
	const_iterator end() const;
private:
	void new_bucket();
	void destroy();

	class iterator : public std::iterator<std::forward_iterator_tag,T>
	{
		friend class xwsbucket;
	public:
		inline T& operator * () const {return *m_elem;}
		inline T* operator ->() const {return m_elem;}
		inline iterator& operator ++ () {
			assert(m_elem<m_end);
			++m_elem;
			if(m_elem==m_end) {
				BUCKET *pbucket=*(BUCKET**)m_end;
				if(pbucket) {
					m_elem=pbucket->data;
					m_end=pbucket->data+pbucket->size;
				}
			}
			return *this;
		}
		inline iterator operator ++ (int) {
			assert(m_pbucket && m_elem<m_end);
			iterator itmp=*this; 
			++(*this);
			return itmp;
		}
		inline bool operator == (const iterator &iter) const {
			return m_elem==iter.m_elem;
		}
		inline bool operator != (const iterator &iter) const {
			return m_elem!=iter.m_elem;
		}
	protected:
		T *m_elem, *m_end;
	};

	class const_iterator : public iterator
	{
		friend class xwsbucket;
	public:
		const_iterator() {}
		const_iterator(const iterator &iter) : iterator(iter) {}
		const_iterator& operator = (const iterator &iter) {
			iterator::operator=(iter);
			return *this;
		}
		inline const T& operator * () const {return *m_elem;}
		inline const T* operator ->() const {return m_elem;}
	};
};

}; // namespace wyc

#include "wyc/util/wsbucket.inl"

#endif // __HEADER_WYC_WSBUCKET

