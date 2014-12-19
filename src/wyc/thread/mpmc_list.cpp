#include "winpch.h"
#include "wyc/thread/mpmc_list.h"

namespace wyc
{

xmpmc_list::xmpmc_list() 
{
	m_head=0;
}

void xmpmc_list::push(xasync_node *n)
{
	xasync_node *next=m_head;
	n->m_next=next;
	while(InterlockedCompareExchangePointer((volatile PVOID*)&m_head, n, next)!=next) {
		next=m_head;
		n->m_next=next;
	}
}

xasync_node* xmpmc_list::pop() 
{
	xasync_node *head=m_head;
	while(head) 
	{
		// 在CAS之前,如果head所指向的结点被其他线程pop & push
		// 则head->m_next可能已被修改,而m_head==head依然成立
		// 此时CAS成功,但m_head获得的却是旧的m_next值
		if(InterlockedCompareExchangePointer((volatile PVOID*)&m_head, head->m_next, head)==head) {
			return head;
		}
		head=m_head;
	}
	return 0;
}

xasync_node* xmpmc_list::pop(xhazard_local<xasync_node> *loc) 
{
	xasync_node *head=m_head;
	while(head) 
	{
		hazard_set(loc,head);
		if(head==m_head) {
			if(InterlockedCompareExchangePointer((volatile PVOID*)&m_head, head->m_next, head)==head) {
				break;
			}
		}
		head=m_head;
	}
	hazard_set(loc,0);
	return head;
}

xasync_node* xmpmc_list::flush() 
{
	xasync_node *head=m_head;
	while(head) {
		if(InterlockedCompareExchangePointer((volatile PVOID*)&m_head, 0, head)==head) {
			return head;
		}
		head=m_head;
	}
	return 0;
}

}; // namespace wyc

