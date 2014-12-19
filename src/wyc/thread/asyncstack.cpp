#include "winpch.h"
#include "wyc/thread/asyncstack.h"

namespace wyc
{

xasync_stack::~xasync_stack()
{
	entry_t *pentry=(entry_t*)m_stack.flush(), *pdel;
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

void xasync_stack::push(void *pdata) 
{
	assert(pdata);
	entry_t *pentry=(entry_t*)m_cache.pop_front();
	if(pentry==0) 
		pentry=new entry_t;
#ifdef _DEBUG
	else assert(0==pentry->m_pdata);
#endif
	pentry->m_pdata=pdata;
	m_stack.push_front(pentry);
}

void xasync_stack::clear()
{
	entry_t *pentry=(entry_t*)m_stack.flush(), *pdel;
	while(pentry) {
		pdel=pentry;
		pentry=(entry_t*)pentry->next();
		m_cache.push_front(pdel);
	}
}


} // namespace wyc

