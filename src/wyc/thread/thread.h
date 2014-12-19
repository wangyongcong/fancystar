#ifndef __HEADER_WYC_THREADSYNC
#define __HEADER_WYC_THREADSYNC

#include <cassert>
#include <atomic>
#include "wyc/platform.h"

namespace wyc 
{

struct xasync_node
{
	xasync_node* volatile m_next;
};


//------------------------------------------------------------------------------
// global functions
//------------------------------------------------------------------------------

unsigned get_thread_id();

//------------------------------------------------------------------------------
// class xcritical_section
//------------------------------------------------------------------------------

class xcritical_section
{
	CRITICAL_SECTION m_ce;
	unsigned m_lockcnt;
	unsigned long m_owner; 
public:
	xcritical_section();
	~xcritical_section();
	void lock();
	void unlock();
	unsigned lock_count() const;
	unsigned long owner_thread() const;
private:
	xcritical_section(const xcritical_section&);
	xcritical_section& operator = (const xcritical_section&);
};

/*********************************************************
  class xatom_signal
  使用原子操作实现的信号量
/********************************************************/

class xatom_signal
{
	std::atomic_bool m_sign;
public:
	xatom_signal();
	void flag();
	void clear();
	operator bool () const;
	void wait(bool b) const;
};

/*********************************************************
  class xevent_signal
  使用操作系统事件机制实现的信号量
/********************************************************/

class xevent_signal
{
	HANDLE m_eveFlag, m_eveClear;
public:
	xevent_signal();
	~xevent_signal();
	void flag();
	void clear();
	operator bool () const;
	void wait(bool b) const;
};

typedef xatom_signal xsignal;

/*********************************************************
  Peterson Lock, Dmitriy algorithm
/********************************************************/

class xpeterson_lock
{
	std::atomic_uint m_flag[2], m_turn;
public:
	xpeterson_lock();
	void lock(unsigned idx);
	void unlock(unsigned idx);
};

} // namespace wyc


#include "wyc/thread/thread.inl"

#endif // __HEADER_WYC_THREADSYNC

