#include "winpch.h"
#include <cassert>
#include "wyc/util/util.h"

using namespace wyc;

// fit in cache line
//#define SIZE_PRODUCER_DATA	16
// across cache line
#define SIZE_PRODUCER_DATA	27

#define DATA_COUNT 1000

namespace {

struct SHARE_DATA 
{
	int* m_shareData[DATA_COUNT];
	SHARE_DATA() 
	{
		memset(m_shareData,0,sizeof(void*)*DATA_COUNT);
	}
	~SHARE_DATA() 
	{
		for(int i=0; i<DATA_COUNT; ++i) {
			if(m_shareData[i])
				delete [] m_shareData[i];
		}
	}
};

unsigned int WINAPI producer(void *ctx)
{
	SHARE_DATA &sd=*(SHARE_DATA*)ctx;
	for(int cnt=0; cnt<DATA_COUNT; ++cnt) {
		int *p=new int[SIZE_PRODUCER_DATA];
		for(int i=0; i<SIZE_PRODUCER_DATA; ++i) {
			p[i]=i;
		}
		// insert sfence here...
		_ReadWriteBarrier();
		sd.m_shareData[cnt]=p;
		wyc_print("producer: %d",cnt);
	}
	return 0;
}

unsigned int WINAPI consumer(void *ctx)
{
	SHARE_DATA &sd=*(SHARE_DATA*)ctx;
	int cnt=0, wait=0;
	while(cnt<DATA_COUNT) {
		while(sd.m_shareData[cnt]==0)
			wait+=1;
		// insert lfence here...
		_ReadWriteBarrier();
		int *p=sd.m_shareData[cnt];
		// 假设是顺序store, 所以从后往前检查更容易捕获错误
		for(int i=SIZE_PRODUCER_DATA-1; i>=0; --i) {
			assert(p[i]==i);
		}
		wyc_print("consumer: %d (%d)",cnt,wait);
		++cnt;
		wait=0;
	}
	return 0;
}

} // anonymous namespace

void test_read_write_fence()
{
	SHARE_DATA sd;
	HANDLE p[2];
	p[0]=(HANDLE)_beginthreadex(NULL,0,producer,&sd,CREATE_SUSPENDED,0);
	p[1]=(HANDLE)_beginthreadex(NULL,0,consumer,&sd,CREATE_SUSPENDED,0);
	::SetThreadPriority(p[1],THREAD_PRIORITY_ABOVE_NORMAL);
	ResumeThread(p[0]);
	ResumeThread(p[1]);
	WaitForMultipleObjects(2,p,TRUE,INFINITE);
	CloseHandle(p[0]);
	CloseHandle(p[1]);
}


