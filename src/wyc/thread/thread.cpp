#include "winpch.h"
#include "wyc/thread/thread.h"

namespace wyc
{

/*********************************************************
 class xatom_signal
/********************************************************/

void xatom_signal::wait(bool b) const
{
	int cnt=0;
	while(m_sign.load(std::memory_order_acquire)!=b) {
		++cnt;
		if(cnt<1000) 
			Sleep(0);
		else {
			cnt=0;
			Sleep(1);
		}
		continue;
	}
}

//------------------------------------------------------------------------------
// class xpeterson_lock
//------------------------------------------------------------------------------

xpeterson_lock::xpeterson_lock()
{
	m_flag[0] = 0;
	m_flag[1] = 0;
	m_turn = 0;
}

void xpeterson_lock::lock(unsigned idx)
{
	assert(idx==0 || idx==1);
	unsigned t = 1-idx;
	m_flag[idx].store(1, std::memory_order_relaxed);
	m_turn.exchange(t, std::memory_order_acq_rel);

	while (m_flag[t].load(std::memory_order_acquire)
		&& t == m_turn.load(std::memory_order_relaxed))
		Sleep(0);
}

void xpeterson_lock::unlock(unsigned idx)
{
	assert(idx==0 || idx==1);
	m_flag[idx].store(0, std::memory_order_release);
}

}; // namespace wyc
