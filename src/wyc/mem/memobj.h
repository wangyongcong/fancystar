#ifndef __HEADER_WYC_XMEMOBJ
#define __HEADER_WYC_XMEMOBJ

#include <cstdlib>
#include "memdef.h"

namespace wyc 
{

class xmemobj 
{
#ifdef WYC_MEMDEBUG
private:
	static void* operator new (size_t size) throw();
	static void* operator new[] (size_t size) throw();
public:
	static void* operator new (size_t size, const char *file, size_t line, const char *function);
	static void* operator new[] (size_t size, const char *file, size_t line, const char *function);
	static void operator delete (void*, const char*, size_t, const char*);
	static void operator delete[] (void*, const char*, size_t, const char*);
#else // !defined WYC_MEMDEBUG
public:
	static void* operator new (size_t size) throw();
	static void* operator new[] (size_t size) throw();
#endif // WYC_MEMDEBUG
	static void operator delete (void *ptr, size_t size);
	static void operator delete[] (void *ptr);
	// placement new/delete
	static void* operator new (size_t size, void *pmem);
	static void* operator new[] (size_t size, void *pmem);
	static void operator delete (void*, void*);
	static void operator delete[] (void*, void*);
};

}; // namespace wyc

#include "wyc/mem/memobj.inl"

#endif // __HEADER_WYC_XMEMOBJ
