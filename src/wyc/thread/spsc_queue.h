/*-----------------------------------------------------------------------------
http://www.1024cores.net/home/lock-free-algorithms/queues/unbounded-spsc-queue

Unbounded single-producer/single-consumer node-based queue. 
Internal non-reducible cache of nodes is used. 
Dequeue operation is always wait-free. 
Enqueue operation is wait-free in common case (when there is available node in the cache), 
otherwise enqueue operation calls ::operator new(), so probably not wait-free. 
No atomic RMW operations nor heavy memory fences are used, i.e. enqueue and dequeue operations issue just several plain loads, 
several plain stores and one conditional branching. 
Cache-conscious data layout is used, so producer and consumer can work simultaneously causing no cache-coherence traffic.

Single-producer/single-consumer queue can be used for communication with thread which services hardware device (wait-free property is required), 
or when there are naturally only one producer and one consumer. 
Also N single-producer/single-consumer queues can be used to construct multi-producer/single-consumer queue, 
or N^2 queues can be used to construct fully-connected system of N threads (other partially-connected topologies are also possible).

Hardware platform: x86-32/64 
Compiler: Intel C++ Compiler 
-----------------------------------------------------------------------------*/

#ifndef __HEADER_WYC_SPSC_QUEUE
#define __HEADER_WYC_SPSC_QUEUE

#include "wyc/thread/thread.h"

namespace wyc
{

template<typename T>
class xspsc_queue 
{
	struct node_t
	{ 
		node_t *m_next; 
		T m_value; 
	}; 

	// consumer part 
	// accessed mainly by consumer, infrequently be producer 
	node_t* volatile m_tail; // tail of the queue 

	// delimiter between consumer part and producer part, 
	// so that they situated on different cache lines 
	char _cache_line_pad [64]; 

	// producer part 
	// accessed only by producer 
	node_t* m_head; // head of the queue 
	node_t* m_first; // last unused node (tail of node cache) 
	node_t* m_tail_copy; // helper (points somewhere between first_ and tail_) 
public:
	xspsc_queue() 
	{
		node_t *n=new node_t; 
		n->m_next=0; 
		m_tail=m_head=m_first=m_tail_copy=n; 
	}
	~xspsc_queue()
	{
		node_t* n=m_first, *next; 
		do 
		{ 
			next=n->m_next; 
			delete n; 
			n=next; 
		} 
		while(n); 
	}
	void push(T v)
	{
		node_t *n=alloc_node(); 
		n->m_next=0; 
		n->m_value=v; 
		_ReadWriteBarrier(); // compiler fence
		// x86: store never hoist
		m_head->m_next=n; // sync!
		m_head=n; 
	}
	bool pop(T &v)
	{
		node_t *next=m_tail->m_next;
		if (next) 
		{ 
			v=next->m_value; 
			_ReadWriteBarrier(); // compiler fence
			// x86: store never hoist
			m_tail=next; // sync!
			return true; 
		} 
		return false; 
	}
private:
	node_t* alloc_node() 
	{ 
		if (m_first==m_tail_copy) 
		{
			m_tail_copy=m_tail; // sync!
			if (m_first==m_tail_copy)
				return new node_t;
		}
		node_t *n=m_first; 
		m_first=m_first->m_next;
		return n; 
	} 
	xspsc_queue(const xspsc_queue&);
	xspsc_queue& operator = (const xspsc_queue&);
};

} // namespace wyc

#endif // __HEADER_WYC_SPSC_QUEUE

