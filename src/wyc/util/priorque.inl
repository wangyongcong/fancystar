#ifndef __INLINE_WYC_PRIORQUE
#define __INLINE_WYC_PRIORQUE

namespace wyc
{

template<typename T, typename Cmp>
T* xpriorque<T,Cmp>::__alloc(size_t sz)
{
	return (T*)new char[sizeof(T)*sz];
}

template<typename T, typename Cmp>
void xpriorque<T,Cmp>::__free(T *pmem)
{
	delete [] pmem;
}

template<typename T, typename Cmp>
bool xpriorque<T,Cmp>::__move_up(size_t pos)
{
	size_t cid=pos, pid;
	T tmpval;
	Cmp compare;
	while(cid>0) {
		pid=(cid-1)>>1;
		if(compare(m_que[pid],m_que[cid]))
			break;
		tmpval=m_que[cid];
		m_que[cid]=m_que[pid];
		m_que[pid]=tmpval;
		cid=pid;
	}
	return cid!=pos;
}

template<typename T, typename Cmp>
bool xpriorque<T,Cmp>::__move_down(size_t pos)
{
	size_t pid=pos, cid=(pid<<1)+1;
	T tmpval;
	Cmp compare;
	while(cid<m_size) {
		if(cid+1<m_size && compare(m_que[cid+1],m_que[cid]))
			cid+=1;
		if(!compare(m_que[cid],m_que[pid])) 
			break;
		tmpval=m_que[pid];
		m_que[pid]=m_que[cid];
		m_que[cid]=tmpval;
		pid=cid;
		cid=(pid<<1)+1;
	}
	return pid!=pos;
}

template<typename T, typename Cmp>
xpriorque<T,Cmp>::xpriorque(size_t sz)
{
	m_que=0;
	m_size=0;
	m_capacity=0;
	if(sz) 
		resize(sz);
}

template<typename T, typename Cmp>
xpriorque<T,Cmp>::~xpriorque()
{
	if(m_que)
		__free(m_que);
}

template<typename T, typename Cmp>
void xpriorque<T,Cmp>::push(const T &elem)
{
	if(m_size==m_capacity)
		resize((m_capacity<<1)+1);
	assert(m_capacity>m_size);
	m_que[m_size++]=elem;
	__move_up(m_size-1);
}

template<typename T, typename Cmp>
bool xpriorque<T,Cmp>::pop(T &elem)
{
	if(m_size==0)
		return false;
	elem=*m_que;
	*m_que=m_que[--m_size];
	__move_down(0);
	return true;
}

template<typename T, typename Cmp>
void xpriorque<T,Cmp>::refresh(const T &elem)
{
	size_t i;
	for(i=0; i<m_size; ++i) {
		if(m_que[i]==elem)
			break;
	}
	if(i==m_size)
		return;
	if(!__move_up(i)) 
		__move_down(i);
}

template<typename T, typename Cmp>
void xpriorque<T,Cmp>::resize(size_t sz)
{
	if(sz<m_capacity)
		return;
	T *pnew=__alloc(sz);
	if(m_que) {
		memcpy(pnew,m_que,sizeof(T)*m_size);
		__free(m_que);
	}
	m_que=pnew;
	m_capacity=sz;
}

template<typename T, typename Cmp>
inline void xpriorque<T,Cmp>::clear()
{
	m_size=0;
}

template<typename T, typename Cmp>
inline size_t xpriorque<T,Cmp>::capacity() const {
	return m_capacity;
}

template<typename T, typename Cmp>
inline size_t xpriorque<T,Cmp>::size() const {
	return m_size;
}

template<typename T, typename Cmp>
inline bool xpriorque<T,Cmp>::empty() const {
	return m_size==0;
}

}; // namespace wyc

#endif // __INLINE_WYC_PRIORQUE
