#include "winpch.h"
#include "wyc/util/util.h"
#include "wyc/util/time.h"
#include "wyc/util/hash.h"
#include "wyc/thread/spsc_ring.h"

using wyc::xspsc_ring;

//--------------------------------------------------------
// 测试参数配置

// ring buffer 容量
#define RING_BUFFER_CAPACITY 256
// ring buffer 批大小
#define RING_BUFFER_BATCH_SIZE 8

// 每个producer产生的数据量
#define DATA_COUNT 100000
#define BUFFER_SIZE (DATA_COUNT*NUM_PRODUCER)

#define NUM_PRODUCER 1
#define NUM_CONSUMER 1

#define SPIN_LOCK 4000

//--------------------------------------------------------

#ifdef _DEBUG
	#define WYC_ASSERT assert
#else
	#define WYC_ASSERT(expression) if(!(expression)) {wyc_error(#expression);}
#endif

namespace 
{

struct xshare_data_t
{
	unsigned m_producer;
	unsigned m_consumer;
	unsigned m_data;
};

typedef xspsc_ring<xshare_data_t*> queue_t;

struct xproducer_context_t
{
	queue_t *m_que;
	xshare_data_t *m_buffer;
	size_t m_size;
	size_t m_idx;
};

unsigned int WINAPI producer (void* pctx)
{
	unsigned threadID=::GetCurrentThreadId();
	wyc_print("producer[%d] start...",threadID);
	xproducer_context_t *ppc=(xproducer_context_t*)pctx;
	xshare_data_t *iter=ppc->m_buffer, 
		*end=ppc->m_buffer+ppc->m_size;
	unsigned spin, total_spin=0, suspend=0;
	while(iter!=end) {
		iter->m_producer=threadID;
		iter->m_consumer=0;
		iter->m_data=ppc->m_idx++;
		spin=0;
		while(!ppc->m_que->push(iter)) {
			++spin;
			if(spin>=SPIN_LOCK) {
				Sleep(0);
				spin=0;
				++suspend;
				total_spin+=SPIN_LOCK;
			}
		}
		total_spin+=spin;
		++iter;
	}
	while(!ppc->m_que->push(0))
		Sleep(0);
	ppc->m_que->flush_write_batch();
	wyc_print("producer[%d] done (spin: %d, average: %.4f, suspend: %d)",threadID,total_spin,float(total_spin)/ppc->m_size,suspend);
	return 0;
}

struct consumer_context_t
{
	queue_t *m_que;
	unsigned m_ret;
};

unsigned int WINAPI consumer (void* pctx)
{
	unsigned threadID=::GetCurrentThreadId();
	wyc_print("consumer[%d] start...",threadID);
	consumer_context_t *pcc=(consumer_context_t*)pctx;
	xshare_data_t *pdata;
	unsigned count=0, spin, total_spin=0, suspend=0;
	while(1) 
	{
		spin=0;
		while(!pcc->m_que->pop(pdata)) 
		{
			++spin;
			if(spin>=SPIN_LOCK) {
				Sleep(0);
				spin=0;
				++suspend;
				total_spin+=SPIN_LOCK;
			}
		}
		total_spin+=spin;
		if(pdata) {
			pdata->m_consumer=threadID;
			++count;
			continue;
		}
		break;
	}
	pcc->m_ret=count;
	wyc_print("consumer[%d] done (spin: %d, average: %.4f, suspend: %d)",threadID,total_spin, float(total_spin)/count,suspend);
	return 0;
}

} // namespace

void test_ring_queue ()
{
	wyc_print("test ring queue (size: %d)",sizeof(queue_t));
	wyc::xcode_timer ct;
	queue_t que(RING_BUFFER_CAPACITY,RING_BUFFER_BATCH_SIZE);
	HANDLE hp[NUM_PRODUCER], hc[NUM_CONSUMER];
	xproducer_context_t ctx_producer[NUM_PRODUCER];
	consumer_context_t ctx_consumer[NUM_CONSUMER];
	unsigned producer_id[NUM_PRODUCER], consumer_id[NUM_CONSUMER];
	xshare_data_t *buffer=new xshare_data_t[BUFFER_SIZE];
	wyc::xdict di;
	for(unsigned i=0, idx=0; i<NUM_PRODUCER; ++i, idx+=DATA_COUNT) 
	{
		ctx_producer[i].m_que=&que;
		ctx_producer[i].m_buffer=buffer+idx;
		ctx_producer[i].m_idx=idx;
		ctx_producer[i].m_size=DATA_COUNT;
		hp[i]=(HANDLE)_beginthreadex(NULL,0,producer,&ctx_producer[i],CREATE_SUSPENDED,&producer_id[i]);
		di.add(producer_id[i],0);
	}

	for(unsigned i=0; i<NUM_CONSUMER; ++i) 
	{
		ctx_consumer[i].m_que=&que;
		ctx_consumer[i].m_ret=0;
		hc[i]=(HANDLE)_beginthreadex(NULL,0,consumer,&ctx_consumer[i],CREATE_SUSPENDED,&consumer_id[i]);
		di.add(consumer_id[i],0);
	}

	ct.start();
	for(unsigned i=0; i<NUM_CONSUMER; ++i)
	{
		::ResumeThread(hc[i]);
	}
	for(unsigned i=0; i<NUM_PRODUCER; ++i)
	{
		::ResumeThread(hp[i]);
	}
	WaitForMultipleObjects(NUM_PRODUCER,hp,TRUE,INFINITE);
	WaitForMultipleObjects(NUM_CONSUMER,hc,TRUE,INFINITE);
	ct.stop();
	wyc_print("Time: %.4f",ct.get_time());

	void *p;
	for(unsigned i=0; i<BUFFER_SIZE; ++i) {
		WYC_ASSERT(buffer[i].m_data==i);
		WYC_ASSERT(buffer[i].m_consumer);
		p = di[buffer[i].m_producer];
		di[buffer[i].m_producer] = (void*)(uintptr_t(p)+1);
		p = di[buffer[i].m_consumer];
		di[buffer[i].m_consumer]= (void*)(uintptr_t(p)+1);
	}
	for(unsigned i=0; i<NUM_PRODUCER; ++i) {
		p=di[producer_id[i]];
		WYC_ASSERT(DATA_COUNT==uintptr_t(p));
	}
	unsigned total=0, count;
	for(unsigned i=0; i<NUM_CONSUMER; ++i) {
		count=(uintptr_t)(void*)di[consumer_id[i]];
		WYC_ASSERT(count==ctx_consumer[i].m_ret);
		total+=count;
	}
	WYC_ASSERT(total==BUFFER_SIZE);

	for(unsigned i=0; i<NUM_CONSUMER; ++i)
	{
		::CloseHandle(hc[i]);
	}
	for(unsigned i=0; i<NUM_PRODUCER; ++i)
	{
		::CloseHandle(hp[i]);
	}
	delete [] buffer;
}

