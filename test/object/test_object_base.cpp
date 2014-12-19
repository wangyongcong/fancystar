#include "winpch.h"
#include <vector>
#include <algorithm>
#include "wyc/util/util.h"
#include "wyc/util/time.h"
#include "wyc/obj/object.h"
#include "wyc/thread/asyncstack.h"

using wyc::xasync_stack;
using wyc::xobject_context;
using wyc::xobject;

extern void thread_init();

// producer/consumer的数量,根据queue的类型进行调整
// 总和不要超过硬件线程数量
#define NUM_PRODUCER 1
#define NUM_CONSUMER 1

namespace 
{
	
struct xtest_context
{
	xasync_stack *m_stack;
	volatile unsigned *m_producerExit;
	volatile unsigned *m_consumerExit;
	volatile unsigned *m_cleanerExit;
};

unsigned WINAPI cleaner_thread (void *args)
{
	thread_init();
	unsigned threadID=::GetCurrentThreadId();
	xtest_context *pctx=(xtest_context*)args;
	xobject_context *pobjctx=xobject::get_context();
	wyc_print("[%d] cleaner: start",threadID);
	while(!*(pctx->m_cleanerExit))
	{
		pobjctx->update();
	}
	wyc_print("[%d] cleaner exit",threadID);
	return 0;
}

unsigned WINAPI producer_thread (void *args)
{
	thread_init();
	unsigned threadID=::GetCurrentThreadId();
	xtest_context *pctx=(xtest_context*)args;
	wyc_print("[%d] producer: start",threadID);
	unsigned cnt, stride, total=0, frame=0;
	std::vector<xobject*> objlist;
	std::vector<xobject*>::iterator last;
	objlist.reserve(256);
	xobject *pobj;
	while(!*(pctx->m_producerExit))
	{
		cnt=unsigned(8+wyc::random()*24);
		total+=cnt;
		while(cnt>0) {
			pobj=wycnew xobject;
			assert(0==((uintptr_t)pobj & WYC_MEMORY_ALIGNMENT_MASK));
			pobj->incref();
			objlist.push_back(pobj);
			pobj->incref();
			assert(pobj->refcount()==2);
			pctx->m_stack->push(pobj);
			--cnt;
		}
		cnt=objlist.size();
		stride=unsigned(2+wyc::random()*4);
		// producer
		for(unsigned i=0; i<cnt; i+=stride) {
			pobj=objlist[i];
			objlist[i]=0;
			pobj->decref();
		}
		last=std::remove(objlist.begin(),objlist.end(),(xobject*)0);
		objlist.erase(last,objlist.end());
		frame+=1;
	//	Sleep(1);
	}
	cnt=objlist.size();
	wyc_print("[%d] producer: %d objects left",threadID,cnt);
	for(unsigned i=0; i<cnt; i+=1) {
		objlist[i]->decref();
		objlist[i]=0;
	}
	objlist.clear();
	wyc_print("[%d] producer exit: create %d objects",threadID,total);
	return 0;
}

unsigned WINAPI consumer_thread (void *args)
{
	thread_init();
	unsigned threadID=::GetCurrentThreadId();
	xtest_context *pctx=(xtest_context*)args;
	wyc_print("[%d] consumer: start",threadID);
	xasync_stack::entry_t *entry, *del;
	unsigned cnt, stride, total=0, frame=0;
	std::vector<xobject*> objlist;
	std::vector<xobject*>::iterator last;
	objlist.reserve(256);
	xobject *pobj;
	while(!*(pctx->m_consumerExit))
	{
		entry=pctx->m_stack->flush();
		while(entry) {
			del=entry;
			entry=(xasync_stack::entry_t*)entry->next();
			objlist.push_back((xobject*)del->m_pdata);
			pctx->m_stack->free(del);
			total+=1;
		}
		// cnosumer
		cnt=objlist.size();
		stride=unsigned(2+wyc::random()*4);
		for(unsigned i=0; i<cnt; i+=stride) {
			pobj=objlist[i];
			objlist[i]=0;
			pobj->decref();
		}
		last=std::remove(objlist.begin(),objlist.end(),(xobject*)0);
		objlist.erase(last,objlist.end());
		frame+=1;
	//	Sleep(1);
	}
	cnt=objlist.size();
	wyc_print("[%d] consumer: %d objects left",threadID,cnt);
	for(unsigned i=0; i<cnt; i+=1) {
		objlist[i]->decref();
		objlist[i]=0;
	}
	objlist.clear();
	wyc_print("[%d] consumer exit: receive %d objects",threadID,total);
	return 0;
}

}; // anonymous namespace

void test_object_base()
{
	volatile unsigned producerExit=0;
	volatile unsigned consumerExit=0;
	volatile unsigned cleanerExit=0;
	wyc::xcode_timer ct;
	xtest_context ctx;
	wyc::xasync_stack stack;
	ctx.m_producerExit=&producerExit;
	ctx.m_consumerExit=&consumerExit;
	ctx.m_cleanerExit=&cleanerExit;
	ctx.m_stack=&stack;
	HANDLE hp[NUM_PRODUCER], hc[NUM_CONSUMER], hgc;
	unsigned producer_id[NUM_PRODUCER], consumer_id[NUM_CONSUMER], gc_id;
	for(unsigned i=0; i<NUM_PRODUCER; ++i) 
	{
		hp[i]=(HANDLE)_beginthreadex(NULL,0,producer_thread,&ctx,CREATE_SUSPENDED,&producer_id[i]);
	}

	for(unsigned i=0; i<NUM_CONSUMER; ++i) 
	{
		hc[i]=(HANDLE)_beginthreadex(NULL,0,consumer_thread,&ctx,CREATE_SUSPENDED,&consumer_id[i]);
	}
	hgc=(HANDLE)_beginthreadex(NULL,0,cleaner_thread,&ctx,0,&gc_id);
	
	wyc_print("Running test suit...(press [Q] to exit)");
	ct.start();
	for(unsigned i=0; i<NUM_CONSUMER; ++i)
	{
		::ResumeThread(hc[i]);
	}
	for(unsigned i=0; i<NUM_PRODUCER; ++i)
	{
		::ResumeThread(hp[i]);
	}
	while(true) {
		char c=(char)getchar();
		if(c=='q' || c=='Q') {
			producerExit=1;
			getchar();
			break;
		}
	}
	WaitForMultipleObjects(NUM_PRODUCER,hp,TRUE,INFINITE);
	consumerExit=1;
	WaitForMultipleObjects(NUM_CONSUMER,hc,TRUE,INFINITE);
	cleanerExit=1;
	WaitForSingleObject(hgc,INFINITE);
	ct.stop();
	wyc_print("Time: %.4f",ct.get_time());

	for(unsigned i=0; i<NUM_CONSUMER; ++i)
	{
		::CloseHandle(hc[i]);
	}
	for(unsigned i=0; i<NUM_PRODUCER; ++i)
	{
		::CloseHandle(hp[i]);
	}
	CloseHandle(hgc);
}

