#include "fscorepch.h"
#include "wyc/game/frametime.h"

namespace wyc
{

#define MAX_DURATION 4.0	// 最多时间间隔,单位:秒

xframe_clock::xframe_clock()
{
	m_current_time=0;
	m_duration=0;
	m_elapsed_time=0;
}

void xframe_clock::reset(double time_delta) 
{
	m_current_time=xtime_source::singleton().get_time()+time_delta;
	m_duration=0;
	m_elapsed_time=0;
}

void xframe_clock::tick()
{
	double last_time = m_current_time;
	m_current_time = xtime_source::singleton().get_time();
	m_duration = m_current_time-last_time;
	/*
		On some systems QueryPerformanceCounter returns a bit different counter values
		on the different CPUs (contrary to what it's supposed to do), which can cause negative frame times
		if the thread is scheduled on the other CPU in the next frame. This can cause very jerky behavior and
		appear as if frames return out of order.
	*/
	if(m_duration<0) {
		m_current_time = last_time;
		m_duration = 0;
	}
	else if(m_duration>MAX_DURATION) {
		m_duration = MAX_DURATION;
	}
	m_elapsed_time += m_duration;
	
	for(timer_list_t::iterator iter = m_timers.begin(), end = m_timers.end();
		iter!=end; ++iter) {
			if(!(*iter)->is_paused())
				(*iter)->m_elapsed_time+=m_duration;
	}
}

} // namespace wyc

