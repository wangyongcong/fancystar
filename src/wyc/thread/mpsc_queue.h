#ifndef __HEADER_WYC_MPSCQUEUE
#define __HEADER_WYC_MPSCQUEUE

#include "wyc/thread/thread.h"

namespace wyc
{

class xmpsc_queue 
{
	xasync_node* volatile m_head;
	xasync_node* m_tail;
	xasync_node  m_stub;
public:
	xmpsc_queue();
	void push(xasync_node *n);
	xasync_node* pop();
private:
	xmpsc_queue(const xmpsc_queue&);
	xmpsc_queue& operator = (const xmpsc_queue&);
};

}; // namespace wyc

#endif // __HEADER_WYC_MPSCQUEUE


