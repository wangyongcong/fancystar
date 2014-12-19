#ifndef __INLINE_WYC_WSBUCKET
#define __INLINE_WYC_WSBUCKET

// Ä¬ÈÏbucket size
#define DEFAULT_BUCKET_SIZE 32

namespace wyc {

template<typename T>
xwsbucket<T>::xwsbucket(size_t page_size) : m_pbuckets(0), m_ptail(0), m_cursor(0), 
	m_pagesz(page_size?page_size:DEFAULT_BUCKET_SIZE), m_size(0)
{
}

template<typename T>
xwsbucket<T>::~xwsbucket()
{
	if(m_pbuckets)
		destroy();
}

template<typename T>
inline void xwsbucket<T>::set_page_size(unsigned sz)
{
	m_pagesz=sz==0?DEFAULT_BUCKET_SIZE:sz;
}

template<typename T>
inline size_t xwsbucket<T>::page_size() const
{
	return m_pagesz;
}

template<typename T>
inline size_t xwsbucket<T>::size() const
{
	return m_size;
}

template<typename T>
void xwsbucket<T>::push_back(const T &elem) {
	if(m_ptail==0 || m_cursor==m_ptail->data+m_ptail->size) 
		new_bucket();
	new(m_cursor) T(elem);
	++m_cursor;
	++m_size;
}

template<typename T>
T* xwsbucket<T>::push_back() {
	if(m_ptail==0 || m_cursor==m_ptail->data+m_ptail->size) 
		new_bucket();
	T *pelem=new(m_cursor) T;
	++m_cursor;
	++m_size;
	return pelem;
}

template<typename T>
inline void xwsbucket<T>::clear() 
{
	if(m_pbuckets) {
		destroy();
		m_pbuckets=0;
		m_ptail=0;
		m_cursor=0;
		m_size=0;
	}
}

template<typename T>
inline typename xwsbucket<T>::iterator xwsbucket<T>::begin()
{
	iterator iter;
	if(m_pbuckets) {
		iter.m_elem=m_pbuckets->data;
		iter.m_end=m_pbuckets->data+m_pbuckets->size;
	}
	else iter.m_elem=iter.m_end=0;
	return iter;
}

template<typename T>
inline typename xwsbucket<T>::iterator xwsbucket<T>::end()
{
	iterator iter;
	if(m_pbuckets) {
		iter.m_elem=m_cursor;
		iter.m_end=m_pbuckets->data+m_pbuckets->size;
	}
	else iter.m_elem=iter.m_end=0;
	return iter;
}

template<typename T>
inline typename xwsbucket<T>::const_iterator xwsbucket<T>::begin() const
{
	const_iterator iter;
	if(m_pbuckets) {
		iter.m_elem=m_pbuckets->data;
		iter.m_end=m_pbuckets->data+m_pbuckets->size;
	}
	else iter.m_elem=iter.m_end=0;
	return iter;
}

template<typename T>
inline typename xwsbucket<T>::const_iterator xwsbucket<T>::end() const
{
	const_iterator iter;
	if(m_pbuckets) {
		iter.m_elem=m_cursor;
		iter.m_end=m_pbuckets->data+m_pbuckets->size;
	}
	else iter.m_elem=iter.m_end=0;
	return iter;
}

template<typename T>
void xwsbucket<T>::new_bucket() 
{
	size_t memsz=(m_pagesz-1)*sizeof(T)+sizeof(BUCKET)+sizeof(BUCKET*);
	memsz=MEMORY_PADDING(memsz);
	BUCKET *pbucket=(BUCKET*)new uint8_t[memsz];
	m_cursor=pbucket->data;
	pbucket->size=m_pagesz;
	*(BUCKET**)(pbucket->data+pbucket->size)=0;
	if(m_ptail) {
		*(BUCKET**)(m_ptail->data+m_ptail->size)=pbucket;
		m_ptail=pbucket;
	}
	else {
		m_pbuckets=m_ptail=pbucket;
	}
}

template<typename T>
void xwsbucket<T>::destroy() 
{
	assert(m_pbuckets);
	BUCKET *del;
	T *beg, *end;
	while(m_pbuckets!=m_ptail) {
		del=m_pbuckets;
		beg=del->data;
		end=beg+del->size;
		while(beg!=end) {
			beg->~T();
			++beg;
		}
		m_pbuckets=*(BUCKET**)end;
		delete [] (uint8_t*)del;
	}
	beg=m_ptail->data;
	while(beg!=m_cursor) {
		beg->~T();
		++beg;
	}
	delete [] (uint8_t*)m_ptail;
}


} // namespace wyc

#endif // __INLINE_WYC_WSBUCKET
