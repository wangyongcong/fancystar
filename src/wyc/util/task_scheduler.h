#ifndef __XTASK_SCHEDULER
#define __XTASK_SCHEDULER

namespace wyc 
{

class xtask_scheduler {
public:
	enum TASK_STATE {
		TASK_ST_SUSPEND=0x10,
		TASK_ST_REMOVED=0x20,
		TASK_PRIORITY_MASK=0xF,
	};
	enum TASK_RET {
		TASK_CONTINUE,
		TASK_YIELD,
		TASK_FINISHED,
		TASK_ERROR,
	};
	class xtask {
		friend class xtask_scheduler;
	public:
		virtual void delthis()=0;
		virtual TASK_RET process(xtask_scheduler *pscheduler)=0;
		unsigned id() const {
			return m_id;
		}
		void suspend() {
			m_st|=TASK_ST_SUSPEND;
		}
		void resume() {
			m_st&=~TASK_ST_SUSPEND;
		}
		void remove() {
			m_st|=TASK_ST_REMOVED;
		}
		bool is_suspend() const {
			return 0!=(m_st&TASK_ST_SUSPEND);
		}
		bool is_removed() const {
			return 0!=(m_st&TASK_ST_REMOVED);
		}
	private:
		unsigned m_id;
		xtask *m_next;
		unsigned m_st;
	};
	xtask_scheduler(float timeslice);
	~xtask_scheduler();
	void execute();
	unsigned add_task(xtask *ptask, int priority=0, bool suspend=false);
	void del_task(unsigned task_id);
	xtask* find_task(unsigned task_id);
	void set_timeslice(float t);
	float timeslice() const;
	unsigned task_count() const;
private:
	xtask *m_taskli;
	unsigned m_taskcnt;
	float m_timeslice;
	double m_accumtime;
};


// inline function implementation

inline void xtask_scheduler::del_task(unsigned task_id)
{
	xtask *ptask=find_task(task_id);
	if(ptask) ptask->remove();
}

inline void xtask_scheduler::set_timeslice(float t)
{
	m_timeslice=t;
}

inline float xtask_scheduler::timeslice() const
{
	return m_timeslice;
}

inline unsigned xtask_scheduler::task_count() const
{
	return m_taskcnt;
}

} // namespace wyc

#ifndef _LIB

extern void test_timeslice_scheduler();

#endif // !_LIB

#endif // __XTASK_SCHEDULER
