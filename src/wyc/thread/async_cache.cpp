#include "winpch.h"
#include "wyc/util/util.h"
#include "wyc/thread/async_cache.h"

namespace wyc
{

// size为8的倍数, 每个node按照8 byte边界对齐
const uint16_t xasync_node_cache::ms_fixedNodeSize[CACHE_LEVEL]={
	16, 24, 32, 48, 64, 96, 128, 192, 256,
};

const uint8_t xasync_node_cache::ms_bucketSize[CACHE_LEVEL]={
	16, 16, 16, 8, 8, 4, 4, 2, 2,
};

const size_t xasync_node_cache::ms_bucketHeadSize=\
	(std::max<size_t>(sizeof(xcached_node),MEMORY_ALLOCATION_ALIGNMENT)\
	+(MEMORY_ALLOCATION_ALIGNMENT-1))&~(MEMORY_ALLOCATION_ALIGNMENT-1);

xasync_node_cache::xasync_node_cache(xhazard<xasync_node> *hazard) : m_set(1000), m_hazard(hazard)
{
	assert(m_hazard);
}

xasync_node_cache::~xasync_node_cache() 
{
	assert(m_bucket.empty());
}

xcached_node* xasync_node_cache::alloc(size_t sz) {
	xcached_node *node;
	sz+=sizeof(xcached_node);
	if(sz>ms_fixedNodeSize[CACHE_LEVEL-1]) 
	{
		node=(xcached_node*)::_aligned_malloc(sz,MEMORY_ALLOCATION_ALIGNMENT);
		node->m_nodeInfo=0xFF;
	}
	else {
		int idx;
		binary_search<uint16_t>(ms_fixedNodeSize,CACHE_LEVEL,uint16_t(sz),idx);
		assert(idx<CACHE_LEVEL);
		node=(xcached_node*)m_cache[idx].pop(hazard_get_local(m_hazard));
		if(!node) {
			node=_alloc_node(idx);
		}
	}
	m_lock.lock();
	assert(!m_set.contain(node));
	m_set.add(node);
	m_lock.unlock();
	/*
	if(0!=(node->m_nodeInfo&0xFF00))
	{
		wyc_error("[%p]: alloc 0x%X, =>[%p] ",node,node->m_nodeInfo,node->m_next,new_alloc?"(new)":"");
		assert(0);
	}
	node->m_nodeInfo|=0xFF00;*/
	return node;
}

void xasync_node_cache::free(xcached_node* node) {
	m_lock.lock();
	assert(m_set.contain(node));
	m_set.del(node);
	m_lock.unlock();
//	assert(0xFF00==(node->m_nodeInfo&0xFF00));
//	node->m_nodeInfo&=0xFF;
	if(0xFF==node->m_nodeInfo) {
		::_aligned_free(node);
	}
	else {
		assert(node->m_nodeInfo<CACHE_LEVEL);
		hazard_del(m_hazard,(xasync_node*)node,this);
	//	m_cache[node->m_nodeInfo].push(node);
	}
}

xcached_node* xasync_node_cache::_alloc_node(unsigned idx)
{
	unsigned sz=ms_fixedNodeSize[idx];
	xcached_node* node;
	unsigned cnt=ms_bucketSize[idx];
	uint8_t* iter=(uint8_t*)::_aligned_malloc(sz*cnt+ms_bucketHeadSize,MEMORY_ALLOCATION_ALIGNMENT);
	node=(xcached_node*)iter;
	node->m_nodeInfo=idx;
	m_bucket.push(node);
	iter+=ms_bucketHeadSize;
	node=(xcached_node*)iter;
	node->m_nodeInfo=idx;
	for(unsigned i=1; i<cnt; ++i)
	{
		iter+=sz;
		((xcached_node*)iter)->m_nodeInfo=idx;
		m_cache[idx].push((xcached_node*)iter);
		assert(((xcached_node*)iter)->m_next!=((xcached_node*)iter));
	}
	return node;
}

void xasync_node_cache::clear() 
{
	unsigned nodeCount[CACHE_LEVEL];
	xasync_node *node, *pdel;
	for(int i=0; i<CACHE_LEVEL; ++i) {
		node=m_cache[i].flush();
		nodeCount[i]=0;
		while(node) {
			nodeCount[i]+=1;
			node=node->m_next;
		}
	}
	unsigned cacheSize[CACHE_LEVEL];
	memset(cacheSize,0,sizeof(cacheSize));
	node=m_bucket.flush();
	while(node) {
		cacheSize[((xcached_node*)node)->m_nodeInfo]+=1;
		pdel=node;
		node=node->m_next;
		::_aligned_free(pdel);
	}
	assert(0==report(nodeCount,cacheSize));
}

unsigned xasync_node_cache::report(unsigned *nodeCount, unsigned *cacheSize)
{
	char splitLine[39];
	memset(splitLine,'-',sizeof(splitLine));
	splitLine[38]=0;
	wyc_print(splitLine);
	wyc_print("| async node cache report");
	wyc_print(splitLine);
	wyc_print("|  node\t leak\ttotal\tmemory");
	unsigned totalCount, leakCount, totalLeak=0;
	float totalSize, totalAlloc=0;
	int unit_idx;
	for(int i=0; i<CACHE_LEVEL; ++i) {
		totalCount=cacheSize[i]*ms_bucketSize[i];
		leakCount=totalCount-nodeCount[i];
		totalSize=float(totalCount*ms_fixedNodeSize[i]);
		totalAlloc+=totalSize;
		unit_idx=format_memory_size(totalSize);
		wyc_print("| %5d\t%5d\t%5d\t%.2f %s",ms_fixedNodeSize[i],leakCount,totalCount,totalSize,memory_unit(unit_idx));
		totalLeak+=leakCount;
	}
	wyc_print(splitLine);
	unit_idx=format_memory_size(totalAlloc);
	wyc_print("| Total: %.2f %s",totalAlloc,memory_unit(unit_idx));
	wyc_print(splitLine);
	return leakCount;
}

}; // namespace wyc

