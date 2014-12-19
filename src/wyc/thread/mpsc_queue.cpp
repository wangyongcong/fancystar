#include "winpch.h"
#include "wyc/thread/mpsc_queue.h"

namespace wyc
{

xmpsc_queue::xmpsc_queue() 
{
	m_head=m_tail=&m_stub;
	m_stub.m_next=0;
}

void xmpsc_queue::push(xasync_node* n)
{
	n->m_next = 0;
	xasync_node* prev = (xasync_node*) InterlockedExchangePointer(&m_head, n);
	//(*)
	prev->m_next = n;
}

xasync_node* xmpsc_queue::pop()
{
	xasync_node* tail = m_tail;
	xasync_node* next = m_tail->m_next;
	if (tail == &m_stub)
	{
		if (0 == next)
			return 0;
		m_tail = next;
		tail = next;
		next = next->m_next;
	}
	if (next)
	{
		m_tail = next;
		return tail;
	}
	if (tail != m_head)
		return 0;
	push(&m_stub);
	next = tail->m_next;
	if (next)
	{
		m_tail = next;
		return tail;
	}
	return 0;
}

};
