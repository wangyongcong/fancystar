#include "wyc/mem/memtracer.h"
#include "wyc/util/util.h"
#include "wyc/util/time.h"

#ifdef _MSC_VER
#pragma warning (disable:4996)
#endif //_MSC_VER

namespace wyc
{

xmemtracer::xmemtracer() : m_recmap(512), m_flypool(sizeof(FLY),512,512,"memory fly")
{
	m_precs=0;
}

xmemtracer::~xmemtracer()
{
	MEMREC *pdel;
	while(m_precs) {
		pdel=m_precs;
		m_precs=m_precs->next;
		delete pdel;
	}
}

inline size_t make_info(char *sinfo, unsigned max_size, const char *file, uint32_t line, const char *function)
{
	size_t len;
#if defined(_WIN32) || defined(_WIN64)
	len=sprintf_s(sinfo,max_size,"%s(%d): %s",file,line,function);
	if(max_size==len) {
		sinfo[max_size-1]=0;
		len-=1;
	}
#else
	size_t fileLen=strlen(file);
	size_t funcLen=strlen(function);
	len=fileLen+funcLen+16; // 0xFFFFFFFF(转换为十进制最多10位数字)
	if(max_size<=len) {
		if(len-funcLen<max_size) {
			len=sprintf(sinfo,"%s(%d)",file,line);
		}
		else {
			strncpy(sinfo,function,max_size);
			sinfo[max_size-1]=0;
			len=max_size-1;
		}
	}
	else {
		len=sprintf(sinfo,"%s(%d): %s",file,line,function);
	}
#endif // WIN32
	return len;
}

uintptr_t xmemtracer::addrec(size_t sz, const char *file, size_t line, const char *function)
{
	const char *pstr=strrchr(file,'\\');
	if(pstr)
		file=pstr+1;
	char sinfo[MAX_INFO_SIZE];
	size_t len=make_info(sinfo,MAX_INFO_SIZE,file,line,function);
	unsigned key=strhash(sinfo);
	MEMREC *prec;
	m_reclock.lock();
	prec = (MEMREC*)m_recmap.get(key);
	if(!prec) {
		prec=alloc_record(len);
		prec->key=key;
		prec->flys=0;
		prec->ref=0;
		prec->total=0;
		prec->life=0;
		prec->maxlife=0;
		prec->minlife=size_t(-1);
		memcpy(prec->info,sinfo,len);
		prec->info[len]=0;
		prec->next=m_precs;
		m_precs=prec;
		m_recmap.add(key,prec);
	}
	FLY *pfly=newfly();
	pfly->size=sz;
	pfly->time=clock();
	pfly->prec=prec;
	pfly->prev=0;
	pfly->next=prec->flys;
	if(prec->flys)
		prec->flys->prev=pfly;
	prec->flys=pfly;
	prec->total+=sz;
	prec->ref+=1;
	m_reclock.unlock();
	return (uintptr_t)pfly;
}

void xmemtracer::rmrec(uintptr_t recid)
{
	m_reclock.lock();
	FLY *pfly=(FLY*)recid;
	assert(m_flypool.is_valid(pfly));
	if(pfly->prev)
		pfly->prev->next=pfly->next;
	if(pfly->next)
		pfly->next->prev=pfly->prev;
	MEMREC *prec=pfly->prec;
	if(prec->flys==pfly)
		prec->flys=pfly->next;
	size_t life=clock()-pfly->time;
	if(life<prec->minlife)
		prec->minlife=life;
	if(life>prec->maxlife)
		prec->maxlife=life;
	prec->life+=life;
	delfly(pfly);
	m_reclock.unlock();
}

void xmemtracer::report() 
{
	m_reclock.lock();
	MEMREC *prec=m_precs;
	if(prec==0) {
		m_reclock.unlock();
		wyc_print("[tracer] no memory records");
		return;
	}
	wyc_print(log_splitter());
	wyc_print("| memory records ");
	wyc_print(log_splitter());
	unsigned totalLeaks=0;
	float mems, totlaMems=0;
	unsigned unit;
	while(prec) {
		unsigned leakCount=0;
		FLY *pfly=prec->flys;
		while(pfly) {
			++leakCount;
			pfly=pfly->next;
		}
		mems=float(prec->total);
		totlaMems+=mems;
		if(leakCount>0) {
			totalLeaks+=leakCount;
			unit=format_memory_size(mems);
			wyc_print("| %s",prec->info);
			wyc_print("| calls=%d leaks=%d used=%.2f%s",prec->ref,leakCount,mems,memory_unit(unit));
			wyc_print(log_splitter());
		}
		prec=prec->next;
	}
	unit=format_memory_size(totlaMems);
	wyc_print("| TotalUsed=%.2f%s TotalLeaks=%d",totlaMems,memory_unit(unit),totalLeaks);
	wyc_print(log_splitter());
	m_reclock.unlock();
	assert(totalLeaks==0);
}


} // namespace wyc

