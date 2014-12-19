#include "winpch.h"
#include "wyc/util/util.h"
#include "wyc/util/time.h"
#include "wyc/util/hash.h"
#include "wyc/thread/mpmc_list.h"
#include "wyc/thread/mpsc_queue.h"
#include "wyc/thread/spsc_queue.h"

using wyc::xasync_node;
using wyc::xmpmc_list;
using wyc::xmpsc_queue;
using wyc::xspsc_queue;

//--------------------------------------------------------
// 使用producer-consumer模型测试异步队列
// 以下为测试参数配置

// queue的类型
// 1 - mpmc
// 2 - mpsc
// 3 - spsc
#define QUEUE_TYPE 2

// 根据queue的类型配置producer/consumer的数量
// 总和不要超过硬件线程数量
#if QUEUE_TYPE==1
	typedef xmpmc_list queue_t;
	#define NUM_PRODUCER 2
	#define NUM_CONSUMER 2
#elif QUEUE_TYPE==2
	typedef xmpsc_queue queue_t;
	#define NUM_PRODUCER 3
	#define NUM_CONSUMER 1
#endif

// 每个producer产生的数据量
#define DATA_COUNT 40000
#define BUFFER_SIZE (DATA_COUNT*NUM_PRODUCER)

//--------------------------------------------------------

namespace 
{

struct xshare_data_t : public xasync_node
{
	unsigned m_producer;
	unsigned m_consumer;
	unsigned m_data;
};

#if QUEUE_TYPE==3

class xspsc_queue_wrapper
{
	xspsc_queue<xshare_data_t*> m_que;
public:
	void push(xshare_data_t *val)
	{
		m_que.push(val);
	}
	xshare_data_t* pop() 
	{
		xshare_data_t *ret=0;
		m_que.pop(ret);
		return ret;
	}
};
typedef xspsc_queue_wrapper queue_t;
#define NUM_PRODUCER 1
#define NUM_CONSUMER 1

#endif // QUEUE_TYPE==3

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
	while(iter!=end) {
		iter->m_producer=threadID;
		iter->m_consumer=0;
		iter->m_data=ppc->m_idx++;
		ppc->m_que->push(iter);
		++iter;
	}
	wyc_print("producer[%d] done",threadID);
	return 0;
}

struct consumer_context_t
{
	queue_t *m_que;
	unsigned m_ret;
	volatile unsigned *m_exit;
};

unsigned int WINAPI consumer (void* pctx)
{
	unsigned threadID=::GetCurrentThreadId();
	wyc_print("consumer[%d] start...",threadID);
	consumer_context_t *pcc=(consumer_context_t*)pctx;
	xshare_data_t *pdata;
	unsigned count=0;
	while(1) 
	{
		pdata=(xshare_data_t*)pcc->m_que->pop();
		if(pdata) {
			pdata->m_consumer=threadID;
			++count;
		}
		else if(*pcc->m_exit)
			break;
	}
	pcc->m_ret=count;
	wyc_print("consumer[%d] done: %d",threadID,count);
	return 0;
}

} // namespace

#ifdef _DEBUG
	#define WYC_ASSERT assert
#else
	#define WYC_ASSERT(expression) if(!(expression)) {wyc_error(#expression);}
#endif

void test_mpmc_queue ()
{
	volatile unsigned exit=0;
	wyc::xcode_timer ct;
	queue_t que;
	HANDLE hp[NUM_PRODUCER], hc[NUM_CONSUMER];
	xproducer_context_t ctx_producer[NUM_PRODUCER];
	consumer_context_t ctx_consumer[NUM_CONSUMER];
	xshare_data_t *buffer=new xshare_data_t[BUFFER_SIZE];
	unsigned producer_id[NUM_PRODUCER], consumer_id[NUM_CONSUMER];
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
		ctx_consumer[i].m_exit=&exit;
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
	exit=1;
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

