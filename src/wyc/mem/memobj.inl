///////////////////////////////////////////////////////////
// class xmemobj inline functions
///////////////////////////////////////////////////////////

#ifndef __INLINE_WYC_XMEMOBJ
#define __INLINE_WYC_XMEMOBJ

namespace wyc {

inline void* xmemobj::operator new (size_t size) throw() 
{
	if(size==0) size=1;
	return ::_aligned_malloc(size,WYC_MEMORY_ALIGNMENT);
}

inline void* xmemobj::operator new[] (size_t size) throw() 
{
	if(size==0) size=1;
	return ::_aligned_malloc(size,WYC_MEMORY_ALIGNMENT);
}


inline void* xmemobj::operator new (size_t, void *pmem)
{
	return pmem;
}

inline void* xmemobj::operator new[] (size_t, void *pmem)
{
	return pmem;
}

inline void xmemobj::operator delete (void*, void*)
{
	// do nothing
}

inline void xmemobj::operator delete[] (void*, void*)
{
	// do nothing
}

} // namespace wyc

#endif // __INLINE_WYC_XMEMOBJ

