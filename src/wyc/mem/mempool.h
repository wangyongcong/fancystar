#ifndef __HEADER_WYC_XMEMPOOL
#define __HEADER_WYC_XMEMPOOL

#include <vector>
#include "wyc/basedef.h"

namespace wyc
{

/// 小对象内存池
class xmempool
{
	struct mem_cookie {
		union{
			mem_cookie *pnext;
			uint8_t pdata[1];
		};
	};
	unsigned m_elemsize;
	unsigned m_pagesize;
	typedef std::vector<void*> PAGELIST;
	PAGELIST m_pages;
	mem_cookie *m_pmem;
	unsigned m_used;
	char *m_pname;
public:
	xmempool(unsigned elem_size, unsigned page_size, unsigned num_reserved=0, const char *pname=0);
	~xmempool();
	void* alloc();
	void free(void *pelem);
	void clear();
	void reserve(unsigned objnum);
	inline const char* name() const {
		return m_pname;
	}
	inline unsigned elem_size() const {
		return m_elemsize;
	}
	inline unsigned page_size() const {
		return m_pagesize;
	}
	bool is_valid(void *ptr);
	void statistics(unsigned &cap, unsigned &used, unsigned &mem_alloc) const;
	void mem_check();
private:
	xmempool& operator = (const xmempool &mp);
	void __alloc_page();
	void __free_pages();
};

/// 带引用计数的内存池
class xrefmem_pool : public xmempool
{
public:
	xrefmem_pool(unsigned elem_size, unsigned page_size, unsigned num_reserved=0, const char *pname=0);
	void* alloc();
	void free(void *pelem);
	static void* clone(void *pelem);
	static unsigned getref(void *pelem);
	static void incref(void *pelem);
	static void decref(void *pelem);
	static bool lock(void *pelem);
	static void unlock(void *pelem);
};

typedef uint32_t* RMEM_PREF;
#define RMEM_REFPTR(pelem) (RMEM_PREF(pelem)-1)
#define RMEM_GETREF(pelem) (*RMEM_REFPTR(pelem))
#define RMEM_DECREF(pelem) (--RMEM_GETREF(pelem))
#define RMEM_INCREF(pelem) (++RMEM_GETREF(pelem))
#define RMEM_LOCK(pelem) (RMEM_GETREF(pelem)=0)
#define RMEM_UNLOCK(pelem) (RMEM_GETREF(pelem)=1)
#define RMEM_FREE(pool,pelem) (((xmempool*)(&pool))->free(RMEM_REFPTR(pelem)))

inline unsigned xrefmem_pool::getref(void *pelem) {
	return RMEM_GETREF(pelem);
}

inline void xrefmem_pool::incref(void *pelem) {
	RMEM_INCREF(pelem);
}

inline void xrefmem_pool::decref(void *pelem) {
	RMEM_DECREF(pelem);
}

} // namespace wyc

#endif // end of __HEADER_WYC_XMEMPOOL

