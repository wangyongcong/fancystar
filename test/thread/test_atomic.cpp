#include "winpch.h"
#include <cassert>
#include "wyc/util/time.h"
#include "wyc/util/util.h"
#include "wyc/thread/atomic.h"

using namespace wyc;

#define THREAD_COUNT 4
#define REFERENCE_ROUND 100000
#define CRITICAL_ROUND 10000

namespace {

unsigned int WINAPI acquire_reference(void *ctx)
{
	xatomic<int> &ref=*(xatomic<int>*)ctx;
	for(int i=0; i<REFERENCE_ROUND; ++i) {
		ref.increase();
	}
	printf("[%d] acquire end: %d\n",GetCurrentThreadId(),int(ref));
	return 0;
}

unsigned int WINAPI release_reference(void *ctx)
{
	xatomic<int> &ref=*(xatomic<int>*)ctx;
	for(int i=0; i<REFERENCE_ROUND; ++i) {
		ref.decrease();
	}
	printf("[%d] release end: %d\n",GetCurrentThreadId(),int(ref));
	return 0;
}

struct SHARE_DATA {
	xatomic<int> m_lock;
	unsigned m_data[2];
	SHARE_DATA() {
		m_data[0]=0;
		m_data[1]=0;
	}
};

unsigned int WINAPI access_critical_section(void *ctx) 
{
	SHARE_DATA &ce=*(SHARE_DATA*)ctx;
	int cmp, cnt=0, conflict=0;
	double wait=0;
	double total=wyc::xtime_source::singleton().get_time();
	wyc::xcode_timer ct;
	while(cnt<CRITICAL_ROUND) {
		ct.start();
		while(ce.m_lock.load()!=0);
		cmp=0;
		if(!ce.m_lock.compare_exchange(cmp,1)) {
			conflict+=1;
			continue;
		}
		ct.stop();
		wait+=ct.get_time();
		// enter critical section
		ce.m_data[0]+=1;
		ce.m_data[1]-=1;
		// leave critical section
		ce.m_lock.store_rel(0);
		cnt+=1;
	}
	total=wyc::xtime_source::singleton().get_time()-total;
	wyc_print("[%4d] %.6f/%.6f (%.2f%s) %4d",GetCurrentThreadId(),wait,total,wait/total*100.0f,"%%",conflict);
	return 0;
}

struct peterson_lock
{
	volatile bool m_demand[2];
	volatile int m_possession;
	int m_share;
	char *m_buff;
	size_t m_progress;
	const size_t m_round;
	const size_t m_step;
	peterson_lock(size_t round=1000000) : m_round(round), m_step((round+500-1)/500) {
		m_demand[0]=false;
		m_demand[1]=false;
		m_possession=0;
		m_share=0;
		m_buff=new char[1001];
		memset(m_buff,0,1001);
		m_progress=0;
	};
	~peterson_lock() {
		delete [] m_buff;
	}
	typedef std::pair<int,peterson_lock*> peterson_context;
	static unsigned int WINAPI test_peterson_lock(void *ctx)
	{
		int me=((peterson_context*)ctx)->first;
		assert(me==0 || me==1);
		int other=1-me;
		peterson_lock *lock=((peterson_context*)ctx)->second;
		size_t wait=0, round=lock->m_round, step=lock->m_step;
		size_t tick, progress=0;
		for(size_t i=0; i<round; ++i) {
			lock->m_demand[me]=true;
			lock->m_possession=other;
			::MemoryBarrier();
			tick=::GetTickCount();
			while(lock->m_demand[other] && lock->m_possession==other);
			wait+=::GetTickCount()-tick;
			// enter critical section
			if(me==0) 
				lock->m_share+=1;
			else 
				lock->m_share-=1;
			progress+=1;
			if(progress>=step) {
				lock->m_buff[lock->m_progress]=me==0?'+':'=';
				lock->m_progress+=1;
				progress-=step;
			}
			// leave critical section
			lock->m_demand[me]=false;
		}
		wyc_print("thread[%d] exit: wait %f",me,double(wait)/round);
		return 0;
	}
	static void run_test() {
		peterson_lock peterson(1000000);
		peterson_context ctx0(0,&peterson), ctx1(1,&peterson);
		HANDLE th[2];
		th[0]=(HANDLE)::_beginthreadex(NULL,0,test_peterson_lock,&ctx0,CREATE_SUSPENDED,0);
		th[1]=(HANDLE)::_beginthreadex(NULL,0,test_peterson_lock,&ctx1,CREATE_SUSPENDED,0);
		ResumeThread(th[0]);
		ResumeThread(th[1]);
		WaitForMultipleObjects(2,th,TRUE,INFINITE);
		CloseHandle(th[0]);
		CloseHandle(th[1]);
		wyc_print(peterson.m_buff);
		wyc_print("share data: %d",peterson.m_share);
		unsigned p0=0, p1=0;
		char *iter=peterson.m_buff;
		while(*iter!=0) {
			if(*iter=='+')
				++p0;
			else
				++p1;
			++iter;
		}
		wyc_print("share buffer: %d (%d+%d)",p0+p1,p0,p1);
		assert(peterson.m_share==0);
		wyc_print("peterson lock is OK");
	}
private:
	peterson_lock(const peterson_lock&);
	peterson_lock& operator = (const peterson_lock&);
};

} // anonymous namespace

void test_peterson_lock()
{
	peterson_lock::run_test();
}

void test_reference_counter()
{
	xatomic<int> ref=0;
	HANDLE p[THREAD_COUNT];
	for(int i=0; i<THREAD_COUNT; ++i)
		p[i]=NULL;
	for(int i=1; i<THREAD_COUNT; i+=2) {
		p[i-1]=(HANDLE)::_beginthreadex(NULL,0,acquire_reference,&ref,CREATE_SUSPENDED,0);
		p[i]=(HANDLE)::_beginthreadex(NULL,0,release_reference,&ref,CREATE_SUSPENDED,0);
	}
	for(int i=0; i<THREAD_COUNT; ++i) {
		if(p[i]!=NULL) 
			ResumeThread(p[i]);
	}
	WaitForMultipleObjects(THREAD_COUNT,p,TRUE,INFINITE);
	for(int i=0; i<THREAD_COUNT; ++i) {
		if(p[i]!=NULL) 
			CloseHandle(p[i]);
	}
	assert(0==ref);
}

void test_critical_section()
{
	SHARE_DATA ce;
	HANDLE p[THREAD_COUNT];
	for(int i=0; i<THREAD_COUNT; ++i) {
		p[i]=(HANDLE)::_beginthreadex(NULL,0,access_critical_section,&ce,CREATE_SUSPENDED,0);
	}
	for(int i=0; i<THREAD_COUNT; ++i) {
		ResumeThread(p[i]);
	}
	WaitForMultipleObjects(THREAD_COUNT,p,TRUE,INFINITE);
	for(int i=0; i<THREAD_COUNT; ++i) {
		CloseHandle(p[i]);
	}
	assert(0==ce.m_lock);
	assert(0==ce.m_data[0]+ce.m_data[1]);
}

