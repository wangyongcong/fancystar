#include "winpch.h"
#include "wyc/util/util.h"
#include "wyc/util/time.h"
#include "wyc/obj/object.h"
#include "wyc/thread/tls.h"
#include "wyc/mem/memtracer.h"

#ifdef _DEBUG
 #pragma comment(lib, "fslog_d.lib")
 #pragma comment(lib, "fsutil_d.lib")
 #pragma comment(lib, "fsmemory_d.lib")
 #pragma comment(lib, "fsthread_d.lib")
 #pragma comment(lib, "fsobject_d.lib")
#else
 #pragma comment(lib, "fslog.lib")
 #pragma comment(lib, "fsutil.lib")
 #pragma comment(lib, "fsmemory.lib")
 #pragma comment(lib, "fsthread.lib")
 #pragma comment(lib, "fsobject.lib")
#endif

static wyc::xthread_local s_tlsObjctx;
static wyc::xobject_context s_objctx;

wyc::xobject_context* get_thread_context()
{
	return (wyc::xobject_context*)s_tlsObjctx.get_data();
}

void thread_init()
{
	wyc::xdate dt;
	dt.get_date();
	wyc::random_seed(dt.hour()*10000+dt.minute()*100+dt.second());
	s_tlsObjctx.set_data(&s_objctx);
}

extern void test_packet();
extern void test_object_base();
extern void test_ressvr();
extern void test_scheduler();

int main(int, char**)
{
	// initialize memory tracer singleton
	wyc::xmemtracer::singleton();
	wyc::xobject::get_context=&get_thread_context;
	s_tlsObjctx.alloc();
	thread_init();
	assert(wyc::xrefobj::object_count()==0);

//	test_object_base();
//	test_packet();
//	test_ressvr();
	test_scheduler();

	s_objctx.clear();
	s_tlsObjctx.free();
	wyc::xmemtracer::singleton().report();
	printf("Press [Enter] to continue...");
	getchar();
	return 0;
}


