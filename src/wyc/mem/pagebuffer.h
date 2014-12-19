#ifndef __HEADER_WYC_XPAGEBUFFER
#define __HEADER_WYC_XPAGEBUFFER

#include <vector>
#include <set>
#include "wyc/basedef.h"

namespace wyc
{

class xpagebuffer
{
	size_t m_pagesize;
	uint8_t *m_pbuffer;
	size_t m_cursor;
	struct free_node {
		uint8_t *ptr;
	};
	struct compare_freenode {
		bool operator () (const free_node &n1, const free_node &n2) const {
			return *(size_t*)n1.ptr<*(size_t*)n2.ptr;
		}
	};
	typedef std::multiset<free_node,compare_freenode> freelist_t;
	freelist_t m_freelist;
	typedef std::vector<void*> PAGELIST;
	PAGELIST m_pages;
public:
	xpagebuffer(size_t page_size=0);
	~xpagebuffer();
	uint8_t* alloc(size_t sz);
	void free(void* ptr);
	void clear();
	bool is_valid(void *ptr);
	inline size_t pagesize() const {
		return m_pagesize;
	}
private:
	xpagebuffer& operator = (const xpagebuffer &ppb);
	uint8_t* search_free(size_t sz);
	void new_page();
}; // class xpagebuffer

}; // namespace wyc

#endif // end of __HEADER_WYC_XPAGEBUFFER

