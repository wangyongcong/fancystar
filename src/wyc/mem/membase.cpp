#include "wyc/mem/membase.h"
#include "wyc/mem/memtracer.h"

#ifdef _MSC_VER
#pragma warning (disable : 4100)
#endif // _MSC_VER

namespace wyc
{

void* debug_malloc(size_t sz, const char *file, size_t line, const char *function)
{
	if(sz==0) 
		sz=1;
	uintptr_t *ptr=(uintptr_t*)::operator new(sz+sizeof(uintptr_t));
	if(ptr) {
		uintptr_t recid=xmemtracer::singleton().addrec(sz,file,line,function);
		*ptr=recid;
		ptr+=1;
	}
	return ptr;
}

void debug_free(void *ptr)
{
	if(ptr==0) return;
	uintptr_t *prec=(uintptr_t*)ptr;
	prec-=1;
	xmemtracer::singleton().rmrec(*prec);
	::operator delete(prec);
}

} // namespace wyc
