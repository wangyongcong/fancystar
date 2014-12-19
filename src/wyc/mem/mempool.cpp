#include <cassert>
#include <cstring>
#include "wyc/util/util.h"
#include "wyc/mem/mempool.h"

#ifdef _MSC_VER
#pragma warning (disable: 4996)
#endif // _MSC_VER

#define MEM_POOL_PACKED (sizeof(void*)-1)

#define DEFAULT_MEM_POOL_PAGE_SIZE 32

namespace wyc
{

xmempool::xmempool(unsigned elem_size, unsigned page_size, unsigned num_reserved, const char *pname) {
	m_elemsize=elem_size>sizeof(mem_cookie)?elem_size:sizeof(mem_cookie);
	// 确保内存对齐，避免mem_cookie::pnext指针在边界处被撕裂
	assert((MEM_POOL_PACKED & (MEM_POOL_PACKED+1))==0);
	m_elemsize=(m_elemsize+MEM_POOL_PACKED)&(~MEM_POOL_PACKED);
	m_pagesize=page_size==0?DEFAULT_MEM_POOL_PAGE_SIZE:page_size;
	m_pmem=0;
	m_used=0;
	if(num_reserved>0)
		reserve(num_reserved);
	if (pname)
	{
		unsigned len=strlen(pname);
		m_pname=new char[len+1];
		strcpy(m_pname,pname);
	}
	else m_pname=0;
}

xmempool::~xmempool() {
	__free_pages();
	if (m_pname) {
		delete [] m_pname;
	}
}

void* xmempool::alloc() {
	if(!m_pmem) {
		__alloc_page();
	}
	uint8_t* pelem=m_pmem->pdata;
	m_pmem=m_pmem->pnext;
	m_used+=1;
	return pelem;
}

void xmempool::free(void *pelem) {
	mem_cookie* pcookie=(mem_cookie*)pelem;
	pcookie->pnext=m_pmem;
	m_pmem=pcookie;
	m_used-=1;
}

void xmempool::clear() {
#ifdef _DEBUG
	if(m_used>0) {
		if (m_pname) {
			wyc_error("%s: memory leaks!! [%d] unfree",m_pname,m_used);
		}
		else {
			wyc_error("mempool: memory leaks!! [%d] unfree",m_used);
		}
	}
	mem_check();
#endif
	__free_pages();
	m_pmem=0;
	m_used=0;
}

void xmempool::reserve(unsigned objnum) 
{
	unsigned num_page=(objnum+m_pagesize-1)/m_pagesize;
	for(unsigned i=0; i<num_page; ++i)
		__alloc_page();
}


void xmempool::statistics(unsigned &cap, unsigned &used, unsigned &mem_alloc) const
{
	cap=m_pages.size()*m_pagesize;
	used=m_used;
	mem_alloc=m_elemsize*cap;
}

void xmempool::__alloc_page() {
	uint8_t *pmem=new uint8_t[m_elemsize*m_pagesize];
	register unsigned cnt=m_pagesize;
	register mem_cookie* ptr=(mem_cookie*)pmem;
	while(cnt-->1) {
		ptr->pnext=(mem_cookie*)((uint8_t*)ptr+m_elemsize);
		ptr=ptr->pnext;
	}
	if(m_pmem==0) {
		m_pmem=(mem_cookie*)pmem;
		ptr->pnext=0;
	}
	else {
		ptr->pnext=m_pmem;
		m_pmem=(mem_cookie*)pmem;
	}
	m_pages.push_back(pmem);
}

void xmempool::__free_pages()
{
	PAGELIST::iterator iter=m_pages.begin(), end=m_pages.end();
	while(iter!=end) 
		delete [] (uint8_t*)(*(iter++));
	m_pages.clear();
}

bool xmempool::is_valid(void *ptr)
{
	unsigned pagelen=m_elemsize*m_pagesize;
	PAGELIST::iterator iter=m_pages.begin(), end=m_pages.end();
	while(iter!=end) {
		if(ptr>=*iter && ptr<((uint8_t*)(*iter)+pagelen))
			return true;
		++iter;
	}
	return false;
}

void xmempool::mem_check()
{
	unsigned nfree=0;
	mem_cookie* ptr=m_pmem;
	while(ptr) {
		if(!is_valid(ptr))
			assert(("mempool check: memory collapse!!",0));
		nfree+=1;
		ptr=ptr->pnext;
	}
	unsigned total=m_pagesize*m_pages.size();
	if(m_pname) {
		wyc_print("mempool check: %s %d/%d (%d used)",m_pname,nfree,total,m_used);
	}
	else {
		wyc_print("mempool check: anonym %d/%d (%d used)",nfree,total,m_used);
	}
	assert(("mempool check: objects mismatch",nfree+m_used==total));
}

//===============================================================

xrefmem_pool::xrefmem_pool(unsigned elem_size, unsigned page_size, unsigned num_reserved, const char *pname) : xmempool(elem_size+sizeof(uint32_t),page_size,num_reserved,pname)
{
}

void* xrefmem_pool::alloc()
{
	RMEM_PREF pref=(RMEM_PREF)xmempool::alloc();
	*pref=1;
	return pref+1;
}

void xrefmem_pool::free(void *pelem)
{
	RMEM_PREF pref=RMEM_REFPTR(pelem);
	if(*pref<=1)
		xmempool::free(pref);
	else --(*pref);
}

void* xrefmem_pool::clone(void *pelem)
{
	RMEM_PREF pref=RMEM_REFPTR(pelem);
	if(*pref==0) {
		wyc_error("xrefmem_pool: try to clone locked element");
		return 0;
	}
	++(*pref);
	return pelem;
}

bool xrefmem_pool::lock(void *pelem)
{
	RMEM_PREF pref=RMEM_REFPTR(pelem);
	if(*pref>1)
		return false;
	*pref=0;
	return true;
}

void xrefmem_pool::unlock(void *pelem)
{
	RMEM_PREF pref=RMEM_REFPTR(pelem);
	if(*pref==0)
		*pref=1;
}

} // namespace wyc

