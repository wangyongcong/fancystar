#ifndef __INLINE_WYC_ASYNCLIST
#define __INLINE_WYC_ASYNCLIST

namespace wyc
{

inline xasync_entry* xasync_entry::next() const
{
	return (xasync_entry*)Next;
}

inline xasync_entry** xasync_entry::nextptr() const
{
	return (xasync_entry**)(&Next);
}

inline void xasync_entry::set_next(xasync_entry *pentry)
{
	Next=pentry;
}

inline void* xasync_entry::operator new (size_t size) throw()
{
	assert(size>0);
	return ::_aligned_malloc(size,MEMORY_ALLOCATION_ALIGNMENT);
}

inline void* xasync_entry::operator new[] (size_t) throw()
{
	assert(!"不能以数组形式分配xasync_entry,因为无法保证每个xasync_entry起始于MEMORY_ALLOCATION_ALIGNMENT");
	return 0;
}

inline void xasync_entry::operator delete (void *ptr, size_t)
{
	::_aligned_free(ptr);
}

inline void xasync_entry::operator delete[] (void*)
{
	assert(0);
}

inline void* xasync_entry::operator new (size_t, void *pmem)
{
	return pmem;
}

inline void* xasync_entry::operator new[] (size_t, void *pmem)
{
	return pmem;
}

inline void xasync_entry::operator delete (void*, void*)
{
	// do nothing
}

inline void xasync_entry::operator delete[] (void*, void*)
{
	// do nothing
}

inline xasync_slist::xasync_slist()
{
	m_pheader=(PSLIST_HEADER)_aligned_malloc(sizeof(SLIST_HEADER),MEMORY_ALLOCATION_ALIGNMENT);
	assert(m_pheader);
	InitializeSListHead(m_pheader);
}

inline xasync_slist::~xasync_slist()
{
	xasync_entry *piter=flush(), *pdel;
	_aligned_free(m_pheader);
	while(piter) {
		pdel=piter;
		piter=piter->next();
		delete piter;
	}
}

inline void xasync_slist::push_front(xasync_entry *pentry)
{
	InterlockedPushEntrySList(m_pheader,(PSLIST_ENTRY)pentry);
}

inline xasync_entry* xasync_slist::pop_front()
{
	return (xasync_entry*)InterlockedPopEntrySList(m_pheader);
}

inline xasync_entry* xasync_slist::flush()
{
	return (xasync_entry*)InterlockedFlushSList(m_pheader);
}

inline unsigned xasync_slist::size() const
{
	return QueryDepthSList(m_pheader);
}

}; // namespace wyc

#endif //__INLINE_WYC_ASYNCLIST

