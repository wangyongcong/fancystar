#ifndef __HEADER_WYC_ASYNC_NODE
#define __HEADER_WYC_ASYNC_NODE

#include "wyc/util/hash.h"
#include "wyc/thread/mpmc_list.h"

namespace wyc
{

class xasync_node_cache;

class xcached_node : public xasync_node
{
	// 占用指针大小的size,尽管浪费
	// 但可以保证在32/64位系统下,数据起始地址于内存边界对齐
	uintptr_t m_nodeInfo;
	friend class xasync_node_cache;
};

class xasync_node_cache
{
	enum {CACHE_LEVEL=9};
	static const uint16_t ms_fixedNodeSize[CACHE_LEVEL];
	static const uint8_t ms_bucketSize[CACHE_LEVEL];
	static const size_t ms_bucketHeadSize;
	xmpmc_list m_cache[CACHE_LEVEL];
	xmpmc_list m_bucket;
	xhazard<xasync_node> *m_hazard;
	wyc::xcritical_section m_lock;
	wyc::xset m_set;
public:
	xasync_node_cache(xhazard<xasync_node> *hazard);
	~xasync_node_cache();
	xcached_node* alloc(size_t sz);
	void free(xcached_node* node);
	void clear();

	inline void on_thread_start() {
		assert(m_hazard);
		hazard_thread_init(m_hazard);
	}
	inline void on_thread_end() {
		assert(m_hazard);
		hazard_clear(m_hazard,this);
	}
	inline void hazard_free_node(xasync_node* node) {
		m_cache[((xcached_node*)node)->m_nodeInfo].push((xcached_node*)node);
	}
private:
	xcached_node* _alloc_node(unsigned idx);
	unsigned report(unsigned *nodeCount, unsigned *cacheSize);
};

}; // namespace wyc

#endif // __HEADER_WYC_ASYNC_NODE

