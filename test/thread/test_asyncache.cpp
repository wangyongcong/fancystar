#include "winpch.h"

#include <vector>
#include "wyc/util/util.h"
#include "wyc/util/time.h"
#include "wyc/util/hash.h"
#include "wyc/thread/async_cache.h"

using wyc::xcached_node;
using wyc::xasync_node_cache;

#define NUM_CORE 4
#define PRESSURE_ALLOC	0
#define RANDOM_ALLOC	0

#define MEM_ALIGN_MASK (MEMORY_ALLOCATION_ALIGNMENT-1)

namespace
{

struct xtest_context
{
	xasync_node_cache *m_cache;
	volatile unsigned *m_exit;
	unsigned m_seed;
	size_t m_alloc;
};

struct node_t : public xcached_node
{
	char m_dataField[1];
};

unsigned WINAPI proc_alloc_only(void *param)
{
	unsigned threadID=::GetCurrentThreadId();
	xtest_context *pctx=(xtest_context*)param;
	pctx->m_cache->on_thread_start();
	size_t sz, direct=0, total_size=0;
	float bigsz=0;
	std::vector<xcached_node*> data;
	node_t *node;
	wyc::random_seed(pctx->m_seed);
	wyc_print("[%4d] alloc begin...",threadID);
	while(!*(pctx->m_exit) && data.size()<PRESSURE_ALLOC) {
		sz=unsigned(4+wyc::random()*316);
		if(sz>256) {
			direct+=1;
			bigsz+=sz;
		}
		else total_size+=sz;
		node=(node_t*)pctx->m_cache->alloc(sz);
		assert(node);
		// 确保内存边界对齐, 原子操作依赖于此
		assert(0==(uintptr_t(node)&MEM_ALIGN_MASK)); 
		memset(node->m_dataField,'A',sz);
		node->m_dataField[sz-1]=0;
		data.push_back(node);
	}
	sz=data.size();
	for(size_t i=0; i<sz; ++i)
		pctx->m_cache->free(data[i]);
	int unit_idx=wyc::format_memory_size(bigsz);
	wyc_print("[%4d] finish: alloc [%d] nodes (with %d direct alloc: %.2f %s)",\
		threadID,sz,direct,bigsz,wyc::memory_unit(unit_idx));
	pctx->m_alloc=total_size;
	pctx->m_cache->on_thread_end();
	return 0;
}

unsigned WINAPI proc_alloc_and_free(void *param)
{
	unsigned threadID=::GetCurrentThreadId();
	xtest_context *pctx=(xtest_context*)param;
	pctx->m_cache->on_thread_start();
	size_t sz, cnt, total=0, round=0, total_size=0;
	std::vector<xcached_node*> data;
	node_t *node;
	wyc::random_seed(pctx->m_seed);
	wyc_print("[%4d] random alloc & free",threadID);
#if (RANDOM_ALLOC==0)
	while(!*(pctx->m_exit)) {
#else
	while(!*(pctx->m_exit) && round<RANDOM_ALLOC) {
#endif
		cnt=unsigned(4+wyc::random()*28);
		total+=cnt;
		while(cnt>0) {
			sz=unsigned(1+wyc::random()*255);
			total_size+=sz;
			node=(node_t*)pctx->m_cache->alloc(sz);
			assert(node);
			// 确保内存边界对齐, 原子操作依赖于此
			assert(0==(uintptr_t(node)&MEM_ALIGN_MASK));
			memset(node->m_dataField,'R',sz);
			node->m_dataField[sz-1]=0;
			data.push_back(node);
			--cnt;
		}
		sz=data.size();
		for(size_t i=0; i<sz; ++i) {
			pctx->m_cache->free(data[i]);
		}
		data.clear();
		++round;
	}
	wyc_print("[%4d] finish: alloc [%d] nodes",threadID,total);
	pctx->m_alloc=total_size;
	pctx->m_cache->on_thread_end();
	return 0;
}

}; // anonymous namespace

void test_async_cache() 
{
	wyc::xhazard<wyc::xasync_node> hazard;
	hazard_process_init(&hazard,NUM_CORE);
	xasync_node_cache cache(&hazard);
	volatile unsigned exit=0;
	wyc::xcode_timer ct;
	HANDLE ht[NUM_CORE];
	xtest_context ctx[NUM_CORE];
	wyc::xdate dt;
	dt.get_date();
	unsigned seed=dt.hour()*10000+dt.minute()*100+dt.second();
#if (PRESSURE_ALLOC==0)
	for(unsigned i=0; i<NUM_CORE; ++i) 
	{
		ctx[i].m_cache=&cache;
		ctx[i].m_exit=&exit;
	//	ctx[i].m_seed=seed;
		ctx[i].m_seed=seed^(seed*i);
		ctx[i].m_alloc=0;
		ht[i]=(HANDLE)_beginthreadex(NULL,0,proc_alloc_and_free,&ctx[i],CREATE_SUSPENDED,0);
	}
#else
	for(unsigned i=0; i<NUM_CORE; i+=2) 
	{
		ctx[i].m_cache=&cache;
		ctx[i].m_exit=&exit;
	//	ctx[i].m_seed=seed;
		ctx[i].m_seed=seed^(seed*i);
		ctx[i].m_alloc=0;
		ht[i]=(HANDLE)_beginthreadex(NULL,0,proc_alloc_only,&ctx[i],CREATE_SUSPENDED,0);
	}
	for(unsigned i=1; i<NUM_CORE; i+=2) 
	{
		ctx[i].m_cache=&cache;
		ctx[i].m_exit=&exit;
	//	ctx[i].m_seed=seed;
		ctx[i].m_seed=seed^(seed*i);
		ctx[i].m_alloc=0;
		ht[i]=(HANDLE)_beginthreadex(NULL,0,proc_alloc_and_free,&ctx[i],CREATE_SUSPENDED,0);
	}
#endif // (PRESSURE_ALLOC==0)

	wyc_print("running test...(enter 'q' to quit)");
	ct.start();
	for(unsigned i=0; i<NUM_CORE; ++i)
	{
		::ResumeThread(ht[i]);
	}
#if (RANDOM_ALLOC==0)
	while(true) {
		char c=(char)getchar();
		if(c=='q' || c=='Q') {
			exit=1;
			getchar();
			break;
		}
	}
#endif // (RANDOM_ALLOC==0)
	WaitForMultipleObjects(NUM_CORE,ht,TRUE,INFINITE);
	ct.stop();
	wyc_print("Time: %.4f",ct.get_time());
	float total=0;
	for(unsigned i=0; i<NUM_CORE; ++i) 
		total+=ctx[i].m_alloc;
	int unit_idx=wyc::format_memory_size(total);
	wyc_print("Total used: %.2f %s",total,wyc::memory_unit(unit_idx));
	cache.clear();
}

