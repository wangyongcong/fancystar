#include "winpch.h"
#include <vector>
#include <algorithm>
#include "wyc/util/util.h"
#include "wyc/util/time.h"
#include "wyc/util/strutil.h"
#include "wyc/obj/object.h"
#include "wyc/thread/asyncstack.h"
#include "wyc/obj/scheduler.h"

using namespace wyc;

extern void thread_init();

// client的数量,总和不要超过硬件线程数量-1 (预留一个给server)
#define NUM_CLIENT 3

#define RESOURCE_COUNT 32

namespace 
{

class xtask_test : public xobject
{
	float m_file;
public:
	xtask_test()
	{
		m_file=wyc::random();
	}
	void load_file()
	{
		wyc_print("loading file [%f]...",m_file);
		Sleep(33);
	}
	void on_file_ok()
	{
		wyc_print("file OK [%f]",m_file);
	}
};

void worker_process (void *args) 
{
	xtask_test *ta=(xtask_test*)args;
	ta->load_file();
}

void worker_callback (void *args)
{
	xtask_test *ta=(xtask_test*)args;
	ta->on_file_ok();
}

struct xtest_context
{
	xworker_server *m_svr;
	volatile unsigned *m_serverExit;
	volatile unsigned *m_clientExit;
};
	
unsigned WINAPI server_thread (void *args)
{
	thread_init();
	unsigned threadID=::GetCurrentThreadId();
	xtest_context *pctx=(xtest_context*)args;
//	xobject_context *pobjctx=xobject::get_context();
	wyc_print("[%d] server: start",threadID);
	double last_time=xtime_source::singleton().get_time(), cur_time;
	while(!*(pctx->m_serverExit))
	{
		cur_time=xtime_source::singleton().get_time();
		pctx->m_svr->update();
		last_time=cur_time;
		Sleep(1);
	}
	pctx->m_svr->shut_down();
	wyc_print("[%d] server exit",threadID);
	return 0;
}

unsigned WINAPI client_thread (void *args)
{
	thread_init();
	unsigned threadID=::GetCurrentThreadId();
	xtest_context *pctx=(xtest_context*)args;
	if(!pctx->m_svr->connect()) {
		wyc_error("can't link resource server");
		return 1;
	}
	xtask_test *ta;
	std::vector<xrefobj*> loaders;
	loaders.reserve(RESOURCE_COUNT);
	for(size_t i=0; i<RESOURCE_COUNT; ++i) {
		ta=wycnew xtask_test;
		ta->incref();
		loaders.push_back(ta);
	}
	size_t idx=0;
	wyc_print("[%d] client: start",threadID);
	while(!*(pctx->m_clientExit))
	{
		if(idx<loaders.size()) {
			pctx->m_svr->send_request(worker_process,loaders[idx++],worker_callback);
		}
		pctx->m_svr->listen();
		Sleep(33);
	}
	pctx->m_svr->disconnect();
	for(size_t i=0, cnt=loaders.size(); i<cnt; ++i)
		loaders[i]->decref();
	loaders.clear();
	wyc_print("[%d] client exit",threadID);
	return 0;
}

}; // anonymous namespace

void test_scheduler()
{
	xworker_server svr;
	volatile unsigned serverExit=0;
	volatile unsigned clientExit=0;
	wyc::xcode_timer ct;
	xtest_context ctx;
	ctx.m_svr=&svr;
	ctx.m_serverExit=&serverExit;
	ctx.m_clientExit=&clientExit;
	HANDLE hclient[NUM_CLIENT], hsvr;
	unsigned client_id[NUM_CLIENT], svr_id;
	hsvr=(HANDLE)_beginthreadex(NULL,0,server_thread,&ctx,0,&svr_id);	
	for(unsigned i=0; i<NUM_CLIENT; ++i) 
	{
		hclient[i]=(HANDLE)_beginthreadex(NULL,0,client_thread,&ctx,0,&client_id[i]);
	}
	wyc_print("Running test suit...(press [Q] to exit)");
	ct.start();
	while(true) {
		char c=(char)getchar();
		if(c=='q' || c=='Q') {
			clientExit=1;
			getchar();
			break;
		}
	}
	WaitForMultipleObjects(NUM_CLIENT,hclient,TRUE,INFINITE);
	serverExit=1;
	WaitForSingleObject(hsvr,INFINITE);
	ct.stop();
	wyc_print("Time: %.4f",ct.get_time());
	// clean up
	for(unsigned i=0; i<NUM_CLIENT; ++i)
	{
		::CloseHandle(hclient[i]);
	}
	CloseHandle(hsvr);
}

