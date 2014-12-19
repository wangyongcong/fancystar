#ifndef __HEADER_WYC_RECYCLEBIN
#define __HEADER_WYC_RECYCLEBIN

#include "wyc/thread/asynclist.h"

namespace wyc
{

class xasync_stack
{
	struct xstack_entry : public xasync_entry 
	{
		void *m_pdata;
	};
	xasync_slist m_stack, m_cache;
public:
	typedef xstack_entry entry_t;
	~xasync_stack();
	void push(void *pdata);
	void* pop();
	entry_t* flush();
	void xasync_stack::free(entry_t *pentry);
	void clear();
};

inline xasync_stack::entry_t* xasync_stack::flush() {
	return (entry_t*)m_stack.flush();
}

inline void xasync_stack::free(entry_t *pentry) {
#ifdef _DEBUG
	pentry->m_pdata=0;
#endif
	m_cache.push_front(pentry);
}

inline void* xasync_stack::pop() {
	entry_t *pentry=(entry_t*)m_stack.pop_front();
	void *pret=pentry->m_pdata;
	m_cache.push_front(pentry);
	return pret;
}

}; // namespace wyc

#endif // __HEADER_WYC_RECYCLEBIN
