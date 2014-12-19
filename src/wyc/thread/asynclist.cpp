#include "winpch.h"
#include <cstdio>
#include "wyc/util/util.h"
#include "wyc/thread/asynclist.h"

namespace wyc 
{

// inline binary search for entry size
// size should be multiple of 8
// [8,16,24,32,48,64,96,128,192,256]
const uint16_t xasync_entry_cache::ms_fixedNodeSize[CACHE_LEVEL]={
	8,16,24,32,48,64,96,128,192,256,
};

xasync_entry_cache::xasync_entry_cache() {
#ifdef _DEBUG
	memset(m_cacheCount,0,sizeof(m_cacheCount));
#endif
}

xasync_entry_cache::~xasync_entry_cache() {
	clear();
}

xasync_entry* xasync_entry_cache::alloc(size_t sz) {
	if(sz>ms_fixedNodeSize[CACHE_LEVEL-1]) {
		return (xasync_entry*)::_aligned_malloc(sz,MEMORY_ALLOCATION_ALIGNMENT);
	}
	int idx;
	binary_search<uint16_t>(ms_fixedNodeSize,CACHE_LEVEL,uint16_t(sz),idx);
	assert(idx<CACHE_LEVEL);
//	INLINE_SEARCH(sz,idx);
	xasync_entry *pentry=m_cache[idx].pop_front();
	if(pentry==NULL) {
		pentry=(xasync_entry*)::_aligned_malloc(sz,MEMORY_ALLOCATION_ALIGNMENT);
#ifdef _DEBUG
		m_cacheCount[idx]++;
#endif
	}
	return pentry;
}

void xasync_entry_cache::free(xasync_entry* pentry, size_t sz) {
	if(sz>ms_fixedNodeSize[CACHE_LEVEL-1]) {
		::_aligned_free(pentry);
		return;
	}
	int idx;
	binary_search<uint16_t>(ms_fixedNodeSize,CACHE_LEVEL,uint16_t(sz),idx);
	assert(idx<CACHE_LEVEL);
//	INLINE_SEARCH(sz,idx);
	m_cache[idx].push_front(pentry);
}

void xasync_entry_cache::clear() {
#ifdef _DEBUG
	unsigned short cache_size[CACHE_LEVEL]={8,16,24,32,48,64,96,128,192,256};
#endif
	xasync_entry *pentry, *pdel;
	for(unsigned i=0; i<CACHE_LEVEL; ++i) {
		pentry=m_cache[i].flush();
#ifdef _DEBUG
		unsigned cnt=0;
#endif
		while(pentry) {
			pdel=pentry;
			pentry=pentry->next();
			::_aligned_free(pdel);
#ifdef _DEBUG
			++cnt;
#endif
		}
#ifdef _DEBUG
		printf("Cache[%03d]: %d\n",cache_size[i],m_cacheCount[i]);
		assert(cnt==m_cacheCount[i]);
#endif
	}
#ifdef _DEBUG
	memset(m_cacheCount,0,sizeof(m_cacheCount));
#endif
}

}; // namespace wyc

