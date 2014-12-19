#ifndef __HEADER_WYC_SPARSE_ARRAY
#define __HEADER_WYC_SPARSE_ARRAY

#include <vector>
#include <cassert>

namespace wyc
{

class xsparse_array
{
	std::vector<void*> m_array;
	size_t m_deleted;
public:
	xsparse_array();
	void reserve(size_t cap);
	void clear();
	void push_back(void *pdata);
	void insert(void *pdata);
	void erase(size_t idx);
	void pack(float threshold=0.0f);
	void* get(size_t idx) const;
	void* operator [] (size_t idx) const;
	size_t capacity() const;
	size_t size() const;
	size_t gap() const;
	float vacancy() const;
	bool empty() const;
};

inline void xsparse_array::reserve(size_t cap) {
	m_array.reserve(cap);
}

inline void xsparse_array::clear() {
	m_array.clear();
	m_deleted=0;
}

inline void xsparse_array::push_back(void *pdata) {
	m_array.push_back(pdata);
}

inline void xsparse_array::erase(size_t idx) {
	assert(idx<m_array.size());
	if(m_array[idx]) {
		m_array[idx]=0;
		m_deleted+=1;
	}
}

inline size_t xsparse_array::capacity() const {
	return m_array.capacity();
}

inline size_t xsparse_array::size() const {
	return m_array.size();
}

inline void* xsparse_array::get(size_t idx) const {
	assert(idx<m_array.size());
	return m_array[idx];
}

inline void* xsparse_array::operator [] (size_t idx) const {
	return get(idx);
}

inline size_t xsparse_array::gap() const {
	return m_deleted;
}

inline bool xsparse_array::empty() const {
	return m_deleted==m_array.size();
}

inline float xsparse_array::vacancy() const {
	return float(m_deleted)/m_array.size();
}

}; // namespace wyc

#endif // end of __HEADER_WYC_SPARSE_ARRAY

