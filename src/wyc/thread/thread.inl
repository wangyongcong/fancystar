#ifndef __INLINE_WYC_THREADSYNC
#define __INLINE_WYC_THREADSYNC

namespace wyc 
{

//------------------------------------------------------------------------------
// global functions
//------------------------------------------------------------------------------

inline unsigned get_thread_id() {
	return ::GetCurrentThreadId();
}

//------------------------------------------------------------------------------
// class xcritical_section
//------------------------------------------------------------------------------

inline xcritical_section::xcritical_section() 
{
	InitializeCriticalSectionAndSpinCount(&m_ce,4000);
	m_lockcnt=0;
	m_owner=0;
}

inline xcritical_section::~xcritical_section() 
{
	DeleteCriticalSection(&m_ce);
}

inline void xcritical_section::lock() {
	EnterCriticalSection(&m_ce);
	m_owner=GetCurrentThreadId();
	++m_lockcnt;
}

inline void xcritical_section::unlock() {
	assert(m_owner==GetCurrentThreadId());
	assert(m_lockcnt>0);
	--m_lockcnt;
	if(m_lockcnt==0) 
		m_owner=0;
	LeaveCriticalSection(&m_ce);
}

inline unsigned xcritical_section::lock_count() const {
	return m_lockcnt;
}

inline unsigned long xcritical_section::owner_thread() const {
	return m_owner;
}

//------------------------------------------------------------------------------
// class xatom_signal
//------------------------------------------------------------------------------

inline xatom_signal::xatom_signal() {
	m_sign = false;
}

inline void xatom_signal::flag() {
	m_sign.store(true,std::memory_order_release);
}

inline void xatom_signal::clear() {
	m_sign.store(false,std::memory_order_release); 
}

inline xatom_signal::operator bool () const {
	return m_sign.load(std::memory_order_acquire);
}

//------------------------------------------------------------------------------
// class xevent_signal
//------------------------------------------------------------------------------

inline xevent_signal::xevent_signal() 
{
	m_eveFlag=CreateEvent(NULL,TRUE,FALSE,NULL);
	m_eveClear=CreateEvent(NULL,TRUE,FALSE,NULL);
}

inline xevent_signal::~xevent_signal() 
{
	CloseHandle(m_eveFlag);
	CloseHandle(m_eveClear);
}

inline void xevent_signal::flag() {
	::ResetEvent(m_eveClear);
	::SetEvent(m_eveFlag);
}

inline void xevent_signal::clear() {
	::ResetEvent(m_eveFlag);
	::SetEvent(m_eveClear);
}

inline xevent_signal::operator bool () const {
	return WaitForSingleObject(m_eveFlag,0)==WAIT_OBJECT_0;
}

inline void xevent_signal::wait(bool b) const {
	DWORD ret;
	b?ret=WaitForSingleObject(m_eveFlag,INFINITE):
		ret=WaitForSingleObject(m_eveClear,INFINITE);
	assert(ret==WAIT_OBJECT_0);
}

}; // namespace wyc

#endif // __INLINE_WYC_THREADSYNC
