#include <cassert>
#include <cstring>
#include "wyc/util/util.h"
#include "wyc/mem/pagebuffer.h"

namespace wyc 
{

#define MAX_PAGE_SIZE 0xFFFF
#define ALLOC_EXPONENT 10 
#define ALLOC_GRANULARITY 1023 // (1<<ALLOC_EXPONENT)-1
#define ALIGN_BUFFER_SIZE(size) ((size+ALLOC_GRANULARITY)&~ALLOC_GRANULARITY)

xpagebuffer::xpagebuffer(size_t page_size)
{
	assert(page_size<MAX_PAGE_SIZE && "xpagebuffer: buffer size must be less than 0xFFFF");
	m_pagesize=page_size?ALIGN_BUFFER_SIZE(page_size):ALLOC_GRANULARITY+1;
	m_pbuffer=new uint8_t[m_pagesize];
	m_cursor=0;
	m_pages.push_back(m_pbuffer);
}

xpagebuffer::~xpagebuffer()
{
	clear();
}

void xpagebuffer::clear()
{
	m_pbuffer=0;
	m_cursor=0;
	m_freelist.clear();
	PAGELIST::iterator iter=m_pages.begin(), end=m_pages.end();
	while(iter!=end) {
		delete [] (uint8_t*)*iter;
		++iter;
	}
	m_pages.clear();
}

uint8_t* xpagebuffer::alloc(size_t sz)
{
	uint8_t *ptr=search_free(sz);
	if(!ptr) {
		size_t required=sz+sizeof(size_t);
		if(required>=m_pagesize)
			return 0;
		if(m_cursor+required>m_pagesize) {
			new_page();
		}
		*(size_t*)(m_pbuffer+m_cursor)=sz;
		ptr=m_pbuffer+m_cursor+sizeof(size_t);
		m_cursor+=required;
	}
	return ptr;
}

void xpagebuffer::free(void *ptr)
{
	assert(is_valid(ptr));
	free_node node;
	node.ptr=((uint8_t*)ptr)-sizeof(size_t);
	m_freelist.insert(node);
}

uint8_t* xpagebuffer::search_free(size_t sz)
{
	free_node node;
	node.ptr=(uint8_t*)&sz;
	freelist_t::iterator iter=m_freelist.find(node);
	if(iter==m_freelist.end())
		return 0;
	uint8_t *ptr=iter->ptr+sizeof(size_t);
	m_freelist.erase(iter);
	return ptr;
}

void xpagebuffer::new_page()
{
	size_t left=m_pagesize-m_cursor;
	if(left>=sizeof(size_t)) {
		free_node node;
		node.ptr=m_pbuffer+m_cursor;
		*(size_t*)node.ptr=left;
		m_freelist.insert(node);
	}
	m_pbuffer=new uint8_t[m_pagesize];
	m_cursor=0;
	m_pages.push_back(m_pbuffer);
}

bool xpagebuffer::is_valid(void *pdel)
{
	uint8_t *ptr=(uint8_t*)pdel;
	ptr-=sizeof(size_t);
	PAGELIST::iterator iter=m_pages.begin(), end=m_pages.end();
	while(iter!=end) {
		if(ptr>=*iter && ptr<((uint8_t*)(*iter)+m_pagesize))
			return true;
		++iter;
	}
	return false;
}

} // namespace wyc

