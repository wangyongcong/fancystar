#ifndef __INLINE_WYC_CIRCUQUE
#define __INLINE_WYC_CIRCUQUE

namespace wyc
{

template<typename T>
xcircuque<T>::xcircuque() {
	m_que=0;
	m_cap=0;
	m_size=0;
}

template<typename T>
xcircuque<T>::xcircuque(unsigned sz) {
	m_que=0;
	m_cap=0;
	m_size=0;
	reserve(sz);
}

template<typename T>
xcircuque<T>::~xcircuque() {
	if(m_que) 
		delete [] m_que;
}

template<typename T>
bool xcircuque<T>::push(const T &elem, bool replace) {
	assert(m_que && "buffer is invalid");
	if(m_size==m_cap && !replace) 
		return false;
	*m_cursor++=elem;
	if(m_cursor>=m_que+m_cap)
		m_cursor=m_que;
	if(m_size==m_cap) 
		m_head=m_cursor;
	else ++m_size;
	return true;
}

template<typename T>
bool xcircuque<T>::pop(T &elem) {
	assert(m_que && "buffer is invalid");
	if(m_size==0)
		return false;
	elem=*m_head++;
	if(m_head>=m_que+m_cap)
		m_head=m_que;
	--m_size;
	return true;
}

template<typename T>
void xcircuque<T>::reserve(unsigned sz)
{
	if(m_cap>=sz)
		return;
	T *pnew=new T[sz];
	if(m_que) {
		if(m_size>0) {
			T *pdst=pnew;
			if(m_head>=m_cursor){
				T *pend=m_que+m_cap;
				while(m_head<pend)
					*pdst++=*m_head++;
				m_head=m_que;
			}
			while(m_head<m_cursor)
				*pdst++=*m_head++;
		}
		delete [] m_que;
	}
	m_que=pnew;
	m_cap=sz;
	m_head=pnew;
	m_cursor=pnew+m_size;
}

template<typename T>
void xcircuque<T>::clear()
{
	m_head=m_cursor=m_que;
	m_size=0;
}

template<typename T>
inline unsigned xcircuque<T>::capacity() const
{
	return m_cap;
}

template<typename T>
inline unsigned xcircuque<T>::size() const
{
	return m_size;
}

template<typename T>
inline const T& xcircuque<T>::front() const
{
	assert(m_size>0);
	return *m_head;
}

template<typename T>
inline T& xcircuque<T>::front()
{
	assert(m_size>0);
	return *m_head;
}

template<typename T>
inline const T& xcircuque<T>::back() const
{
	assert(m_size>0);
	return *m_cursor;
}

template<typename T>
inline T& xcircuque<T>::back()
{
	assert(m_size>0);
	return *m_cursor;
}

template<typename T>
inline typename xcircuque<T>::iterator xcircuque<T>::begin()
{
	iterator iter;
	iter.m_ptr=m_head;
	iter.m_idx=0;
	iter.m_container=this;
	return iter;
}

template<typename T>
inline typename xcircuque<T>::iterator xcircuque<T>::end()
{
	iterator iter;
	iter.m_ptr=m_cursor;
	iter.m_idx=m_size;
	iter.m_container=this;
	return iter;
}

template<typename T>
inline typename xcircuque<T>::const_iterator xcircuque<T>::begin() const
{
	const_iterator iter;
	iter.m_ptr=m_head;
	iter.m_idx=0;
	iter.m_container=(xcircuque*)this;
	return iter;
}

template<typename T>
inline typename xcircuque<T>::const_iterator xcircuque<T>::end() const
{
	const_iterator iter;
	iter.m_ptr=m_cursor;
	iter.m_idx=m_size;
	iter.m_container=(xcircuque*)this;
	return iter;
}

}; // namespace wyc

#endif // __INLINE_WYC_CIRCUQUE
