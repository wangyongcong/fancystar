#include "winpch.h"
#include "wyc/util/util.h"
#include "wyc/util/time.h"
#include "wyc/thread/asyncqueue.h"

using wyc::xasync_queue;

struct PRODUCER_DATA
{
	xasync_queue *queue;
	uintptr_t beg, end;
	PRODUCER_DATA(xasync_queue *que, uintptr_t beg, uintptr_t end) {
		queue=que;
		this->beg=beg;
		this->end=end;
	}
};

struct CONSUMER_DATA
{
	xasync_queue *queue;
	unsigned count;
	CONSUMER_DATA(xasync_queue *que, unsigned cnt) {
		queue=que;
		count=cnt;
	}
};

unsigned int __stdcall producer (void* pdata)
{
	PRODUCER_DATA *pd=(PRODUCER_DATA*)pdata;
	for(uintptr_t i=pd->beg; i<=pd->end; ++i) {
		printf("[P] %d\n",i);
		pd->queue->push((void*)i);
		Sleep(0);
	}
	return 0;
}

unsigned int __stdcall consumer (void* pdata)
{
	CONSUMER_DATA *pd=(CONSUMER_DATA*)pdata;
	unsigned cnt=0;
	while(cnt<pd->count) {
		wyc::xasync_queue::entry_t *pentry=pd->queue->flush(), *pdel;
		while(pentry) {
			printf("[C] %d\n",(uintptr_t)(pentry->m_pdata));
			pdel=pentry;
			pentry=(wyc::xasync_queue::entry_t*)pentry->next();
			pd->queue->free(pdel);
			++cnt;
		}
		Sleep(0);
	}
	return 0;
}

void test_asyncque() 
{
	xasync_queue queue;
	HANDLE hp[3], hc;
	PRODUCER_DATA data1(&queue,1,100), data2(&queue,101,200), data3(&queue,201,300);
	CONSUMER_DATA cdata(&queue,300);
	hp[0]=(HANDLE)_beginthreadex(NULL,0,producer,&data1,0,0);
	assert(hp[0]!=NULL);
	hp[1]=(HANDLE)_beginthreadex(NULL,0,producer,&data2,0,0);
	assert(hp[1]!=NULL);
	hp[2]=(HANDLE)_beginthreadex(NULL,0,producer,&data3,0,0);
	assert(hp[2]!=NULL);
	hc=(HANDLE)_beginthreadex(NULL,0,consumer,&cdata,0,0);
	assert(hc!=NULL);
	WaitForMultipleObjects(3,hp,TRUE,INFINITE);
	WaitForSingleObject(hc,INFINITE);
}

