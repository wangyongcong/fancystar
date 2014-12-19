#ifndef __HEADER_WYC_XASYNCQUEUE
#define __HEADER_WYC_XASYNCQUEUE

#include "asynclist.h"

namespace wyc
{

//
// “Ï≤Ω∂”¡–
//
class xasync_queue
{
	struct xqueue_entry : public xasync_entry 
	{
		void *m_pdata;
	};
	xasync_slist m_que, m_cache;
public:
	typedef xqueue_entry entry_t;
	~xasync_queue();
	void push(void *data);
	entry_t* flush();
	void free(entry_t *pentry);
	void clear();
};

inline void xasync_queue::free(entry_t *pentry)
{
	m_cache.push_front(pentry);
}

}; // namespace wyc

#endif // __HEADER_WYC_XASYNCQUEUE

