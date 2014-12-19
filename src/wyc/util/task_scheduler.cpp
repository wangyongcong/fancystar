#include <cassert>
#include "wyc/util/task_scheduler.h"
#include "wyc/util/time.h"

namespace wyc 
{

xtask_scheduler::xtask_scheduler(float timeslice)
{
	m_taskli=0;
	m_taskcnt=0;
	m_timeslice=timeslice;
	m_accumtime=0;
}

xtask_scheduler::~xtask_scheduler()
{
	xtask *pdel;
	while(m_taskli) {
		pdel=m_taskli;
		m_taskli=m_taskli->m_next;
		pdel->delthis();
	}
}

unsigned xtask_scheduler::add_task(xtask *ptask, int priority, bool suspend)
{
	ptask->m_id=uintptr_t(ptask);
	ptask->m_st=priority&TASK_PRIORITY_MASK;
	if(suspend) 
		ptask->m_st|=TASK_ST_SUSPEND;
	ptask->m_next=m_taskli;
	m_taskli=ptask;
	m_taskcnt+=1;
	return ptask->m_id;
}

void xtask_scheduler::execute()
{
	if(!m_taskcnt || m_timeslice<0)
		return;
	assert(m_taskli);
	if(m_accumtime<0) {
		m_accumtime+=m_timeslice;
		if(m_accumtime<0)
			return;
	}
	else {
		m_accumtime=m_timeslice;
	}
	unsigned taskleft=m_taskcnt;
	double timeslice, timeused;
	TASK_RET ret=TASK_YIELD;
	wyc::xcode_timer ct;
	xtask *iter=m_taskli, **prev=&m_taskli;
	while(iter) {
		if(iter->m_st&TASK_ST_REMOVED) {
			goto REMOVE_TASK;
		}
		if(iter->is_suspend()) {
			goto NEXT_TASK;
		}	
		timeslice=m_accumtime/taskleft;
		timeused=0;
		while(timeused<timeslice) {
			ct.start();
			ret=iter->process(this);
			ct.stop();
			timeused+=ct.get_time();
			if(ret!=TASK_CONTINUE)
				break;
		}
		m_accumtime+=timeslice-timeused;
		if(ret==TASK_YIELD || ret==TASK_CONTINUE) {
NEXT_TASK:
			prev=&iter->m_next;
			iter=iter->m_next;
		}
		else { // TASK_FINISHED || TASK_ERROR
			while(*prev!=iter) {
				// new tasks was added
				assert(*prev);
				prev=&(*prev)->m_next;
			}
REMOVE_TASK:
			*prev=iter->m_next;
			iter->delthis();
			iter=*prev;
			m_taskcnt-=1;
		}
		if(m_accumtime<0) {
			timeslice=m_timeslice*60;
			if(timeslice+m_accumtime<0)
				m_accumtime=-timeslice;
			break;
		}
	}
}

xtask_scheduler::xtask* xtask_scheduler::find_task(unsigned task_id)
{
	xtask *iter=m_taskli;
	while(iter) {
		if(iter->m_id==task_id) {
			return 0==(iter->m_st&TASK_ST_REMOVED)?iter:0;
		}
		iter=iter->m_next;
	}
	return 0;
}


} // namespace wyc


