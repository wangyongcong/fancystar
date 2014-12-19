#include "fscorepch.h"
#include "wyc/util/util.h"
#include "wyc/util/time.h"
#include "wyc/world/gameobj.h"
#include "wyc/mem/memtracer.h"

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

extern void test_gameobj_base();

int main(int, char**)
{
	wyc::xobject::get_context=&get_thread_context;
	s_tlsObjctx.alloc();
	thread_init();

	test_gameobj_base();

	s_objctx.clear();
	s_tlsObjctx.free();
	wyc::xmemtracer::singleton().report();
	printf("Press [Enter] to continue...");
	getchar();
	return 0;
}


