#include <cassert>
#include "wyc/util/util.h"
#include "wyc/mem/memtracer.h"
#include "wyc/mem/memdef.h"
#include "wyc/mem/memobj.h"

namespace wyc 
{

#ifdef WYC_MEMDEBUG

#define WYC_HEADER_SIZE ((sizeof(uintptr_t)+WYC_MEMORY_ALIGNMENT_MASK)&WYC_MEMORY_ALIGNMENT)

void* xmemobj::operator new (size_t size, const char *file, size_t line, const char *function)
{
	if(size==0) size=1;
	void *ptr=::_aligned_malloc(size+WYC_HEADER_SIZE,WYC_MEMORY_ALIGNMENT);
	uintptr_t recid=xmemtracer::singleton().addrec(size,file,line,function);
	*(uintptr_t*)ptr=recid;
//	printf("wycnew: %p (%p, %d)\n",(uintptr_t*)ptr+1,ptr,recid);
	return (uint8_t*)ptr+WYC_HEADER_SIZE;
}

void* xmemobj::operator new[] (size_t size, const char *file, size_t line, const char *function)
{
	if(size==0) size=1;
	void *ptr=::_aligned_malloc(size+WYC_HEADER_SIZE,WYC_MEMORY_ALIGNMENT);
	uintptr_t recid=xmemtracer::singleton().addrec(size,file,line,function);
	*(uintptr_t*)ptr=recid;
	return (uint8_t*)ptr+WYC_HEADER_SIZE;
}

void xmemobj::operator delete (void*, const char*, size_t, const char*)
{
	// do nothing
}

void xmemobj::operator delete[] (void*, const char*, size_t, const char*)
{
	// do nothing
}

#endif // WYC_MEMDEBUG

void xmemobj::operator delete(void *ptr, size_t) {
	if(ptr==0) return;
#ifdef WYC_MEMDEBUG
	ptr=(uint8_t*)ptr-WYC_HEADER_SIZE;
	uintptr_t recid=*(uintptr_t*)ptr;
//	printf("wycdel: %p (%p, %d)\n",(uintptr_t*)ptr+1,ptr,recid);
	xmemtracer::singleton().rmrec(recid);
#endif // WYC_MEMDEBUG
	::_aligned_free(ptr);
}

void xmemobj::operator delete[] (void *ptr) {
	if(ptr==0) return;
#ifdef WYC_MEMDEBUG
	ptr=(uint8_t*)ptr-WYC_HEADER_SIZE;
	uintptr_t recid=*(uintptr_t*)ptr;
	xmemtracer::singleton().rmrec(recid);
#endif // WYC_MEMDEBUG
	::_aligned_free(ptr);
}

} // namespace wyc

