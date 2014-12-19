#include "winpch.h"
#include "wyc/util/util.h"
#include "wyc/util/time.h"
#include "wyc/thread/thread.h"

using namespace wyc;

static xsignal s_exit, s_renderStart, s_renderFinish;
static unsigned s_round=1000000, s_round2;
static unsigned s_counter=0, s_counter2=0;
static double s_logicWait, s_rendererWait;

unsigned int _stdcall thread_logic(void*)
{
	wyc_print("logic start...");
	wyc::xcode_timer ct;
	s_logicWait=0;
	unsigned total=s_round/100;
	while(total-->0) {
		s_counter=0;
		s_counter2=0;
		s_renderStart.flag();
		while(++s_counter<s_round);
		s_renderFinish.flag();
		ct.start();
		s_renderStart.wait(false);
		ct.stop();
		s_logicWait+=ct.get_time();
		assert(s_round2==s_counter2);
		s_renderFinish.clear();
	}
	wyc_print("logic stop");
	s_exit.flag();
	s_renderStart.flag();
	return 0;
}

unsigned int _stdcall thread_render(void*) 
{
	wyc_print("renderer start...");
	wyc::xcode_timer ct;
	while(true) {
		ct.start();
		s_renderStart.wait(true);
		ct.stop();
		s_rendererWait+=ct.get_time();
		if(s_exit)
			break;
		assert(s_counter2==0);
		while(true) {
			++s_counter2;
			if(s_renderFinish) {
				assert(s_counter==s_round);
				break;
			}
		}
		s_round2=s_counter2;
		s_renderStart.clear();
	}
	wyc_print("renderer stop");
	return 0;
}

void test_threadsync () 
{
	HANDLE ht1, ht2;
	ht1=(HANDLE)_beginthreadex(NULL,0,thread_logic,(void*)0,0,0);
	ht2=(HANDLE)_beginthreadex(NULL,0,thread_render,(void*)0,0,0);
	if(NULL==ht1 || NULL==ht2)
	{
		wyc_error("fail to create thread");
		return;
	}
	wyc::xcode_timer task;
	task.start();
	::WaitForSingleObject(ht1,INFINITE);
	::WaitForSingleObject(ht2,INFINITE);
	task.stop();
	wyc_print("all threads are done");
	double taskTime=task.get_time();
	wyc_print("total: %.4f",taskTime);
	wyc_print("logic wait: %.4f (%.4f%%)",s_logicWait,s_logicWait*100/taskTime);
	wyc_print("renderer wait: %.4f (%.4f%%)",s_rendererWait,s_rendererWait*100/taskTime);
}

