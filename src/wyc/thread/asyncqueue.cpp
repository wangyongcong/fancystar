#include "winpch.h"
#include "asyncqueue.h"

namespace wyc
{

xasync_queue::~xasync_queue()
{
	entry_t *pentry=(entry_t*)m_que.flush(), *pdel;
	while(pentry) {
		pdel=pentry;
		pentry=(entry_t*)pentry->next();
		delete pdel;
	}
	pentry=(entry_t*)m_cache.flush();
	while(pentry) {
		pdel=pentry;
		pentry=(entry_t*)pentry->next();
		delete pdel;
	}
}

void xasync_queue::push(void *pdata)
{
	entry_t *pentry=(entry_t*)m_cache.pop_front();
	if(pentry==0) 
		pentry=new entry_t;
	pentry->m_pdata=pdata;
	m_que.push_front(pentry);
}

xasync_queue::entry_t* xasync_queue::flush()
{
	entry_t *pentry=(entry_t*)m_que.flush(), *pnext, *pprev=0;
	if(pentry) {
		do {
			pnext=(entry_t*)pentry->next();
			pentry->set_next(pprev);
			pprev=pentry;
			pentry=pnext;
		} while(pentry);
	}
	return pprev;
}

void xasync_queue::clear()
{
	entry_t *pentry=(entry_t*)m_que.flush(), *pdel;
	while(pentry) {
		pdel=pentry;
		pentry=(entry_t*)pentry->next();
		m_cache.push_front(pdel);
	}
}

}; // namespace wyc

