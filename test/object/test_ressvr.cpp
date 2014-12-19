#include "winpch.h"
#include <vector>
#include <algorithm>
#include "wyc/util/util.h"
#include "wyc/util/time.h"
#include "wyc/util/strutil.h"
#include "wyc/obj/object.h"
#include "wyc/thread/asyncstack.h"
#include "wyc/obj/ressvr.h"

using namespace wyc;

extern void thread_init();

// client的数量,总和不要超过硬件线程数量-1 (预留一个给server)
#define NUM_CLIENT 1

#define RESOURCE_COUNT 256

namespace 
{

class xres_dummy : public xresbase
{
	USE_RTTI;
	std::string m_name;
	unsigned m_load_count;
public:
	xres_dummy()
	{
		m_load_count=0;
	}
	virtual bool async_load(const char *res_name) 
	{
		wyc_print("load: %s",res_name);
		assert(m_load_count==0);
		m_name=res_name;
		m_load_count+=1;
		Sleep(1);
		return true;
	}
	virtual void on_async_complete()
	{
		assert(m_load_count==1);
		assert(m_name.size()>0);
	}
	virtual void unload() 
	{
		wyc_print("unload: %s (%d)",m_name.c_str(),refcount());
		assert(m_load_count==1);
		m_name="";
		m_load_count-=1;
		Sleep(1);
	}
	const std::string& name() const {
		return m_name;
	}
};

REG_RTTI(xres_dummy,xresbase);

class xobj_test : public xobject
{
	USE_RTTI;
	typedef std::vector<std::pair<xresbase*,float> > reslist_t;
	reslist_t m_res;
public:
	virtual void on_destroy()
	{
		reslist_t::iterator iter, end;
		for(iter=m_res.begin(), end=m_res.end(); iter!=end; ++iter)
			iter->first->decref();
		m_res.clear();
	}
	void update(double t) 
	{
		unsigned idx=0, last=m_res.size();
		while(idx!=last)
		{
			std::pair<xresbase*,float> &r=m_res[idx];
			r.second-=float(t);
			if(r.second<=0) {
				r.first->decref();
				last-=1;
				m_res[idx]=m_res[last];
			}
			else idx+=1;
		}
		if(last<m_res.size())
		{
			reslist_t::iterator iter=m_res.begin()+last;
			m_res.erase(iter,m_res.end());
		}
	}
	void add_res(xresbase *res)
	{
		float life=wyc::random()*4.0f+1.0f;
		std::pair<xresbase*,float> holder(res,life);
		m_res.push_back(holder);
		res->incref();
	}
	size_t res_count() const {
		return m_res.size();
	}
	static void on_async_ok (xrefobj *pobj, xresbase *res)
	{
		wyc_print("load OK [%X]: %s",res->id(),((xres_dummy*)res)->name().c_str());
		((xobj_test*)pobj)->add_res(res);
	}
};
REG_RTTI(xobj_test,xobject);

struct xtest_context
{
	xressvr *m_rsvr;
	xworker_server *m_worker;
	std::vector<std::string> m_res_pool;
	volatile unsigned *m_serverExit;
	volatile unsigned *m_clientExit;
};
	
unsigned WINAPI worker_thread (void *args)
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
		pctx->m_worker->update();
		last_time=cur_time;
		Sleep(33);
	}
	pctx->m_worker->shut_down();
	wyc_print("[%d] server exit",threadID);
	return 0;
}

unsigned WINAPI client_thread (void *args)
{
	thread_init();
	unsigned threadID=::GetCurrentThreadId();
	xtest_context *pctx=(xtest_context*)args;
	if(!pctx->m_rsvr->get_scheduler()->connect()) {
		wyc_error("can't link worker server");
		return 1;
	}
	unsigned res_count=pctx->m_res_pool.size()-1;
	unsigned res_type=xres_dummy::get_class()->id;
	unsigned idx;
	xresbase *res;
	xpointer<xobj_test> sp_consumer=wycnew xobj_test;
	double last_time=xtime_source::singleton().get_time(), cur_time;
	wyc_print("[%d] client: start",threadID);
	while(!*(pctx->m_clientExit))
	{
		cur_time=xtime_source::singleton().get_time();
		if(sp_consumer->res_count()<16) {
			idx=unsigned(wyc::random()*res_count);
			res=pctx->m_rsvr->async_request(res_type,pctx->m_res_pool[idx].c_str(),sp_consumer,xobj_test::on_async_ok);
			if(res->is_complete()) 
			{
				wyc_print("already loaded: %s",((xres_dummy*)res)->name().c_str());
				xobj_test::on_async_ok(sp_consumer,res);
			}
		}
		pctx->m_worker->listen();
		sp_consumer->update(cur_time-last_time);
		pctx->m_rsvr->update(cur_time-last_time);
		last_time=cur_time;
		Sleep(33);
	}
	pctx->m_worker->disconnect();
	sp_consumer=0;
	wyc_print("[%d] client exit",threadID);
	return 0;
}

}; // anonymous namespace

bool wyc::send_async_task(worker_function_t func, xrefobj *task, worker_callback_t cb, int hint)
{
	assert(0 && "Never arrive here!");
	return false;
}

void test_ressvr()
{
	xworker_server worker;
	xsimple_scheduler scheduler;
	scheduler.set_worker(&worker);
	xressvr rsvr;
	rsvr.set_scheduler(&scheduler);
	rsvr.create_group("xres_dummy",RESOURCE_COUNT,4);
	volatile unsigned serverExit=0;
	volatile unsigned clientExit=0;
	wyc::xcode_timer ct;
	xtest_context ctx;
	ctx.m_rsvr=&rsvr;
	ctx.m_worker=&worker;
	ctx.m_serverExit=&serverExit;
	ctx.m_clientExit=&clientExit;
	ctx.m_res_pool.reserve(RESOURCE_COUNT);
	std::string s;
	wyc::xset res_set(RESOURCE_COUNT);
	for(int i=0; i<RESOURCE_COUNT; ) {
		wyc::float2str(s,wyc::random()*10);
		unsigned resid=strhash(s.c_str());
		if(res_set.contain((void*)resid))
			continue;
		res_set.add((void*)resid);
		ctx.m_res_pool.push_back(s);
		++i;
	}
	HANDLE client, server;
	unsigned client_id, svr_id;
	server=(HANDLE)_beginthreadex(NULL,0,worker_thread,&ctx,0,&svr_id);	
	client=(HANDLE)_beginthreadex(NULL,0,client_thread,&ctx,0,&client_id);
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
	WaitForSingleObject(client,INFINITE);
	serverExit=1;
	WaitForSingleObject(server,INFINITE);
	rsvr.shut_down();
	ct.stop();
	wyc_print("Time: %.4f",ct.get_time());
	// clean up
	CloseHandle(client);
	CloseHandle(server);
}

