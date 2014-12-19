#ifndef __HEADER_WYC_CIRCUQUE
#define __HEADER_WYC_CIRCUQUE

#include <cassert>
#include <iterator>

namespace wyc
{

template<typename T>
class xcircuque 
{
	T *m_que, *m_head, *m_cursor;
	unsigned m_cap;
	unsigned m_size;
public:
	class iterator : public std::iterator<std::forward_iterator_tag,T>
	{
		friend class xcircuque;
		T* m_ptr;
		unsigned m_idx;
		xcircuque *m_container;
	public:
		inline T& operator * () const {return *m_ptr;}
		inline T* operator ->() const {return m_ptr;}
		inline iterator& operator ++ () {
			++m_ptr;
			if(m_ptr>=m_container->m_que+m_container->m_cap)
				m_ptr=m_container->m_que;
			++m_idx;
			return *this;
		}
		inline iterator operator ++ (int) {
			iterator itmp=*this; 
			++(*this);
			return itmp;
		}
		inline bool operator == (const iterator &iter) const {
			assert(m_container==iter.m_container);
			return m_idx==iter.m_idx;
		}
		inline bool operator != (const iterator &iter) const {
			assert(m_container==iter.m_container);
			return m_idx!=iter.m_idx;
		}
	};
	class const_iterator : public iterator
	{
		friend class xcircuque;
	public:
		const_iterator& operator = (const iterator &iter) {
			iterator::operator=(iter);
			return *this;
		}
		inline const T& operator * () const {return *m_ptr;}
		inline const T* operator ->() const {return m_ptr;}
	};
	friend class iterator;
	friend class const_iterator;
	xcircuque();
	xcircuque(unsigned sz);
	~xcircuque();
	bool push(const T &elem, bool replace=true);
	bool pop(T &elem);
	void reserve(unsigned sz);
	void clear();
	unsigned capacity() const;
	unsigned size() const;
	const T& front() const;
	T& front();
	const T& back() const;
	T& back();
	iterator begin();
	iterator end();
	const_iterator begin() const;
	const_iterator end() const;
};

}; // namespace wyc

#include "wyc/util/circuque.inl"

#endif // __HEADER_WYC_CIRCUQUE

