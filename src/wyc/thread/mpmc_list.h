#ifndef __HEADER_WYC_MPMC_LIST
#define __HEADER_WYC_MPMC_LIST

#include "wyc/thread/thread.h"
#include "wyc/thread/hazard.h"

namespace wyc
{

class xmpmc_list
{
	xasync_node* volatile m_head;
public:
	xmpmc_list();
	void push(xasync_node *n);
	xasync_node* pop();
	xasync_node* pop(xhazard_local<xasync_node> *loc);
	xasync_node* flush();
	inline bool empty() const {
		return 0==m_head;
	}
private:
	xmpmc_list(const xmpmc_list&);
	xmpmc_list& operator = (const xmpmc_list&);
};

}; // namespace wyc

#endif // __HEADER_WYC_MPMC_LIST
