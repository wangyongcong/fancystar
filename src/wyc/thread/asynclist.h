#ifndef __HEADER_WYC_ASYNCLIST
#define __HEADER_WYC_ASYNCLIST

#include <cassert>
#include <atomic>
#include "wyc/platform.h"
#include "wyc/thread/thread.h"

namespace wyc 
{

/*********************************************************
  async singly linked list
/********************************************************/

class xasync_entry : public SLIST_ENTRY
{
public:
	xasync_entry* next() const;
	xasync_entry** nextptr() const;
	// Notice: not thread safe!
	void set_next(xasync_entry *pentry);
	// only created in heap
	static void* operator new (size_t size) throw();
	static void* operator new[] (size_t size) throw();
	static void operator delete (void *ptr, size_t size);
	static void operator delete[] (void *ptr);
	static void* operator new (size_t size, void *pmem);
	static void* operator new[] (size_t size, void *pmem);
	static void operator delete (void*, void*);
	static void operator delete[] (void*, void*);
};

class xasync_slist
{
	PSLIST_HEADER m_pheader;
public:
	xasync_slist();
	~xasync_slist();
	void push_front(xasync_entry *pentry);
	xasync_entry* pop_front();
	xasync_entry* flush();
	unsigned size() const;
};

//--------------------------------------------
// 快速分配小型xasync_entry对象
// 使用缓存的效率约为直接new/delete的50倍
//--------------------------------------------
class xasync_entry_cache
{
	enum {CACHE_LEVEL=10};
	static const uint16_t ms_fixedNodeSize[CACHE_LEVEL];
	xasync_slist m_cache[CACHE_LEVEL];
#ifdef _DEBUG
	std::atomic_uint m_cacheCount[CACHE_LEVEL];
#endif
public:
	xasync_entry_cache();
	~xasync_entry_cache();
	xasync_entry* alloc(size_t sz);
	void free(xasync_entry* pentry, size_t sz);
	void clear();
};

} // namespace wyc

#include "wyc/thread/asynclist.inl"

#endif // __HEADER_WYC_ASYNCLIST

