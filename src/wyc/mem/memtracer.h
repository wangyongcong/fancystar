#ifndef __HEADER_WYC_MEMTRACER
#define __HEADER_WYC_MEMTRACER

#include "wyc/thread/thread.h"
#include "wyc/util/hash.h"
#include "wyc/mem/mempool.h"

namespace wyc
{

class xmemtracer
{
	struct MEMREC;
	struct FLY {
		FLY *prev, *next;
		MEMREC *prec;
		size_t size;
		size_t time;
	};
#pragma pack(push)
#pragma pack(1)
	struct MEMREC {
		unsigned key;
		size_t total;
		size_t ref;
		size_t minlife;
		size_t maxlife;
		size_t life;
		FLY *flys;
		MEMREC *next;
		char info[1];
	};
#pragma pack(pop)
	xdict m_recmap;
	MEMREC *m_precs;
	xmempool m_flypool;
	enum {
		MAX_INFO_SIZE=255,
	};
	xcritical_section m_reclock;
public:
	static xmemtracer& singleton();
	uintptr_t addrec(size_t sz, const char *file="", size_t line=0, const char *function="");
	void rmrec(uintptr_t recid);
	void report();
private:
	xmemtracer();
	~xmemtracer();
	xmemtracer& operator = (const xmemtracer &mt);
	MEMREC* alloc_record(size_t extra);
	FLY* newfly();
	void delfly(FLY *pfly);
};

inline xmemtracer& xmemtracer::singleton()
{
	// TODO: 这种初始化方式在多线程下会有问题
	// 当N个线程同时调用singleton的时候会怎样呢?
	// 第一个调用线程会触发构造函数
	// 而其他线程可能获得一个尚未构造完全的指针
	static xmemtracer *ls_ptracer=new xmemtracer;
	return *ls_ptracer;
}

inline xmemtracer::FLY* xmemtracer::newfly()
{
	return (FLY*)m_flypool.alloc();
}

inline void xmemtracer::delfly(FLY *pfly)
{
	m_flypool.free(pfly);
}

inline xmemtracer::MEMREC* xmemtracer::alloc_record(size_t extra)
{
	return (MEMREC*)malloc(sizeof(MEMREC)+extra);
}

} // namespace wyc

#endif // end of __HEADER_WYC_MEMTRACER
