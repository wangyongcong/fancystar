#ifndef __HEADER_WYC_HAZARD
#define __HEADER_WYC_HAZARD

#include <atomic>
#include "wyc/util/util.h"
#include "wyc/thread/thread.h"
#include "wyc/thread/tls.h"

namespace wyc
{

// 最大hazard pointer=最大线程数*每线程最大指针数
#define HAZARD_MAX_SIZE 8

// 如果线程数量不定,开启以下选项
//#define HAZARD_DYNAMIC_THREAD

template<typename NODE>
struct xhazard
{
	void** m_hazptr;
	unsigned m_size;
	unsigned m_numPerThread;
	std::atomic_uint m_hazidx;
	xthread_local m_tlsHazard;
	unsigned m_threshold;
};

template<typename NODE>
struct xhazard_local
{
	void **m_hazloc;
	NODE *m_delist;
	unsigned m_delcnt;
};

template<typename NODE>
void hazard_process_init(xhazard<NODE> *hazard, unsigned threadCount, unsigned numPerThread=1, unsigned threshold=0) {
	if(hazard->m_tlsHazard) 
		return;
	hazard->m_size=threadCount*numPerThread;
	hazard->m_hazptr=new void*[hazard->m_size];
	memset(hazard->m_hazptr,0,sizeof(void*)*hazard->m_size);
	hazard->m_numPerThread=numPerThread;
	hazard->m_hazidx=0;
	hazard->m_tlsHazard.alloc();
	if(threshold<threadCount)
		threshold=2*threadCount+2;
	hazard->m_threshold=threshold;
}

template<typename NODE>
void hazard_thread_init(xhazard<NODE> *hazard) {
	unsigned idx=hazard->m_hazidx.fetch_add(hazard->m_numPerThread);
	assert(idx<hazard->m_size);
	xhazard_local<NODE> *loc=new xhazard_local<NODE>;
	loc->m_hazloc=hazard->m_hazptr+idx;
	loc->m_delist=0;
	loc->m_delcnt=0;
	hazard->m_tlsHazard.set_data(loc);
}

template<typename NODE, typename ALLOCATOR>
void hazard_scan(xhazard<NODE> *hazard, xhazard_local<NODE> *loc, ALLOCATOR *allocator) {
#ifdef HAZARD_DYNAMIC_THREAD
	void **plist=new void*[hazard->m_size];
#else
	assert(hazard->m_size<=HAZARD_MAX_SIZE);
	void *plist[HAZARD_MAX_SIZE];
#endif
	void **iter, **end;
	unsigned cnt=0;
	iter=hazard->m_hazptr;
	end=hazard->m_hazptr+hazard->m_size;
	while(iter!=end) {
		if(*iter)
			plist[cnt++]=*iter;
		++iter;
	}
	quick_sort<void*>(plist,0,cnt);
	NODE *node=0, *delist=loc->m_delist;
	loc->m_delist=0;
	loc->m_delcnt=0;
	int idx;
	while(delist) {
		node=delist;
		delist=node->m_next;
		if(binary_search<void*>(plist,cnt,node,idx)) {
			node->m_next=loc->m_delist;
			loc->m_delist=node;
			loc->m_delcnt+=1;
		}
		else {
			allocator->hazard_free_node(node);
		}
	}
#ifdef HAZARD_DYNAMIC_THREAD
	delete [] plist;
#endif
}

template<typename NODE>
inline xhazard_local<NODE>* hazard_get_local(xhazard<NODE> *hazard)
{
	return (xhazard_local<NODE>*)(hazard->m_tlsHazard.get_data());
}

template<typename NODE>
inline void hazard_set(xhazard_local<NODE> *loc, void *ptr) {
	// (*) XCHG
	InterlockedExchangePointer((volatile LONG*)loc->m_hazloc,ptr); 
}

template<typename NODE, typename ALLOCATOR>
void hazard_del(xhazard<NODE> *hazard, NODE *ptr, ALLOCATOR *allocator) {
	xhazard_local<NODE> *loc=hazard_get_local(hazard);
	ptr->m_next=loc->m_delist;
	loc->m_delist=ptr;
	loc->m_delcnt+=1;
	if(hazard->m_threshold<=loc->m_delcnt)
		hazard_scan(hazard,loc,allocator);
}

template<typename NODE, typename ALLOCATOR>
void hazard_clear(xhazard<NODE> *hazard, ALLOCATOR allocator)
{
	xhazard_local<NODE> *loc=hazard_get_local(hazard);
	if(0!=*(loc->m_hazloc))
		hazard_set(loc,0);
	unsigned spin=1;
	while(loc->m_delcnt && spin<256) {
		if(0==(spin&7)) {
			// release the rest CPU cycle
			SwitchToThread();
		}
		hazard_scan(hazard,loc,allocator);
		spin+=1;
	}
	if(loc->m_delcnt) {
		unsigned tid=::GetCurrentThreadId();
		wyc_error("Thread[%d] failed to clear hazard, [%d] node left",tid,loc->m_delcnt);
	}
}

}; // namespace wyc

#endif // __HEADER_WYC_HAZARD
