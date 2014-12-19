#include "wyc/platform.h"
#include <string>
#include <vector>
#include "wyc/util/util.h"
#include "wyc/util/time.h"
#include "wyc/util/task_scheduler.h"

using wyc::xtask_scheduler; 

class xtest_task : public wyc::xtask_scheduler::xtask
{
public:
	xtest_task() {
		m_execnt=0;
	}
	virtual void delthis() {
		wyc_sys("delete task: %s",m_name.c_str());
		delete this;
	}
	const char* get_name() const {
		return m_name.c_str();
	}
protected:
	std::string m_name;
	mutable int m_execnt;
	inline void on_execute() const {
		wyc_print("%s (%d)",m_name.c_str(),++m_execnt);
	}
};

class xtest_task1 : public xtest_task
{
public:
	xtest_task1(const char *name, int count, unsigned taskid, int *sum) {
		m_name=name;
		m_count=count;
		m_taskid=taskid;
		m_sum=sum;
		if(m_sum) 
			*m_sum=0;
		build_test_data();
	}
	virtual xtask_scheduler::TASK_RET process(wyc::xtask_scheduler *pscheduler)
	{
		xtest_task::on_execute();
		int total=0;
		for(size_t i=0; i<m_data.size(); ++i) {
			total+=m_data[i];
		}
		if(m_sum)
			*m_sum+=total;
		m_count-=1;
		if(m_count<1) {
			if(m_taskid) {
				xtask *ptask=pscheduler->find_task(m_taskid);
				if(ptask) ptask->resume();
			}
			return xtask_scheduler::TASK_FINISHED;
		}
		return xtask_scheduler::TASK_CONTINUE;
	}
private:
	int m_count;
	int *m_sum;
	unsigned m_taskid;
	std::vector<int> m_data;
	void build_test_data()
	{
		int num=int(float(abs(rand()))/RAND_MAX*100+0.5f);
		for(int i=0; i<num; ++i) {
			m_data.push_back(rand());
		}
	}
};

class xtest_task2 : public xtest_task
{
public:
	xtest_task2(const char *name, int count, int taskid) {
		m_name=name;
		m_count=count;
		m_taskid=taskid;
		m_start=false;
	}
	virtual xtask_scheduler::TASK_RET process(wyc::xtask_scheduler *pscheduler)
	{
		xtest_task::on_execute();
		if(m_start) {
			m_count-=1;
			if(m_count<=0) {
				xtask *ptask=pscheduler->find_task(m_taskid);
				if(ptask) ptask->resume();
				return xtask_scheduler::TASK_FINISHED;
			}
		}
		else {
			m_start=true;
			xtask *ptask=pscheduler->find_task(m_taskid);
			if(ptask) ptask->suspend();
		}
		return xtask_scheduler::TASK_YIELD;
	}
private:
	int m_count;
	unsigned m_taskid;
	bool m_start;
};

class xtest_task3 : public xtest_task
{
public:
	xtest_task3(const char *name, const char *subtask) {
		m_name=name;
		m_subtask=subtask;
		m_start=false;
	}
	virtual xtask_scheduler::TASK_RET process(wyc::xtask_scheduler *pscheduler) {
		xtest_task::on_execute();
		if(!m_start) {
			m_start=true;
			xtest_task *ptask=new xtest_task1(m_subtask.c_str(),32,id(),&m_sum);
			pscheduler->add_task(ptask);
			suspend();
			return xtask_scheduler::TASK_YIELD;
		}
		wyc_sys("sum=%d",m_sum);
		return xtask_scheduler::TASK_FINISHED;
	}
private:
	std::string m_subtask;
	int m_sum;
	bool m_start;
};

void test_task_scheduler() 
{
	srand(clock());
	wyc::xtask_scheduler scheduler(0.001f);

	xtest_task *ptask;
	
	ptask=new xtest_task1("Task01",16,0,0);
	scheduler.add_task(ptask);

	ptask=new xtest_task2("Task02",16,ptask->id());
	scheduler.add_task(ptask);

	ptask=new xtest_task3("Task03","Task04");
	scheduler.add_task(ptask);

	while(scheduler.task_count()) {
		scheduler.execute();
		::Sleep(33);
	}
}

