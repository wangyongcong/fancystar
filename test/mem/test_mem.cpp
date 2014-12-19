#include <string>
#include <cassert>
#include "wyc/util/util.h"
#include "wyc/util/time.h"
#include "wyc/mem/mempool.h"
#include "wyc/mem/pagebuffer.h"

using namespace wyc;

#define print_mem_pool(mp) \
	mp.statistics(cap,used,mem_alloc);\
	wyc_print("cap:\t%d/%d\t(total %d bytes)",used,cap,mem_alloc);\

void test_mempool(unsigned elem_size, unsigned page_size)
{
	xmempool mp(elem_size,page_size);
	wyc_print("create memory pool");
	unsigned cap, used, mem_alloc;
	mp.statistics(cap,used,mem_alloc);
	wyc_print("elem:%d page:%d cap:%d mem:%d used:%d",mp.elem_size(),mp.page_size(),cap,mem_alloc,used);
	unsigned n=page_size+9;
	std::vector<void*> pelems;
	wyc_print("\n============alloc elems============\n");
	for(unsigned i=0; i<n; ++i)
	{
		pelems.push_back(mp.alloc());
		print_mem_pool(mp);
	}
	assert(pelems.size()==used);
	wyc_print("\n============free elems=============\n");
	for(std::vector<void*>::iterator iter=pelems.begin(), end=pelems.end(); iter!=end; ++iter)
	{
		mp.free(*iter);
		print_mem_pool(mp);
	}
	pelems.clear();
	void *ptr=mp.alloc();
	mp.free(ptr);
	void *ptr2=mp.alloc();
	mp.free(ptr2);
	assert(ptr==ptr2);
	mp.clear();
}

void test_refmem_pool(unsigned elem_size, unsigned page_size)
{
	xrefmem_pool mp(elem_size,page_size);
	wyc_print("create memory refrence pool");
	unsigned cap, used, mem_alloc;
	mp.statistics(cap,used,mem_alloc);
	wyc_print("elem:%d page:%d cap:%d mem:%d used:%d",mp.elem_size(),mp.page_size(),cap,mem_alloc,used);
	unsigned n=page_size+9;
	std::vector<void*> pelems, pclones;
	std::vector<void*>::iterator iter, end;
	wyc_print("\n============alloc elems============\n");
	for(unsigned i=0; i<n; ++i)
	{
		pelems.push_back(mp.alloc());
		print_mem_pool(mp);
	}
	assert(pelems.size()==used);
	for(iter=pelems.begin(), end=pelems.end(); iter!=end; ++iter) {
		wyc_print("%p: %d",*iter,mp.getref(*iter));
	}
	wyc_print("\n============clone elems============\n");
	for(iter=pelems.begin(), end=pelems.end(); iter!=end; ++iter)
	{
		unsigned oldref=mp.getref(*iter);
		void *pclone=mp.clone(*iter);
		pclones.push_back(pclone);
		assert(pclone==*iter);
		assert(oldref+1==mp.getref(*iter));
		wyc_print("clone: %p==%p (%d)",*iter,pclone,mp.getref(pclone));
	}
	wyc_print("\n============free elems=============\n");
	for(iter=pelems.begin(), end=pelems.end(); iter!=end; ++iter)
	{
		mp.free(*iter);
		print_mem_pool(mp);
	}
	pelems.clear();
	for(iter=pclones.begin(), end=pclones.end(); iter!=end; ++iter) {
		wyc_print("%p: %d",*iter,mp.getref(*iter));
	}
	wyc_print("\n============free elems=============\n");
	for(iter=pclones.begin(), end=pclones.end(); iter!=end; ++iter) {
		mp.free(*iter);
		print_mem_pool(mp);
	}
	pclones.clear();
	wyc_print("\n============test macro=============\n");
	void *pelem=mp.alloc();
	RMEM_PREF pref=RMEM_REFPTR(pelem);
	wyc_print("init:\tref=%d",*pref);
	RMEM_INCREF(pelem);
	wyc_print("inc:\tref=%d",*pref);
	RMEM_DECREF(pelem);
	wyc_print("dec:\tref=%d",*pref);
	RMEM_LOCK(pelem);
	wyc_print("lock:\tref=%d",*pref);
	RMEM_UNLOCK(pelem);
	wyc_print("unlock:\tref=%d",*pref);
	RMEM_FREE(mp,pelem);
	mp.clear();
	wyc_print("exit...");
}

void profile_mempool(unsigned elem_size, unsigned page_size, bool page_alloc)
{
	xmempool mp(elem_size,page_size);
	wyc_print("Memory pool time compare");
	elem_size=mp.elem_size();
	unsigned n;
	void *ptr;
	n=page_size*2+(page_size>>1);
	if(!page_alloc) {
		mp.reserve(n);
	}
	unsigned cap, used, mem_alloc;
	mp.statistics(cap,used,mem_alloc);
	wyc_print("elem:%d page:%d cap:%d mem:%d used:%d",mp.elem_size(),mp.page_size(),cap,mem_alloc,used);
	std::vector<void*> elems_mp, elems_new;
	xcode_timer ct;
	double t=0, tmax=0, total=0;
	for(unsigned i=0; i<n; ++i)
	{
		ct.start();
		ptr=mp.alloc();
		ct.stop();
		t=ct.get_time();
		if(t>tmax) {
			tmax=t;
			wyc_print("peak at %d",i);
		}
		total+=t;
		elems_mp.push_back(ptr);
	}
	wyc_print("pool alloc:\ttotal=%f\tmax=%f\tave=%f",total,tmax,total/n);
	total=0, tmax=0;
	for(unsigned i=0; i<n; ++i)
	{
		ct.start();
		ptr=new uint8_t[elem_size];
		ct.stop();
		t=ct.get_time();
		if(t>tmax)
			tmax=t;
		total+=t;
		elems_new.push_back(ptr);
	}
	wyc_print("C++ new:\ttotal=%f\tmax=%f\tave=%f",total,tmax,total/n);
	std::vector<void*>::iterator iter, end;
	total=0, tmax=0;
	for(iter=elems_mp.begin(), end=elems_mp.end(); iter!=end; ++iter) {
		ptr=*iter;
		ct.start();
		mp.free(ptr);
		ct.stop();
		t=ct.get_time();
		if(t>tmax)
			tmax=t;
		total+=t;
	}
	elems_mp.clear();
	wyc_print("pool free:\ttotal=%f\tmax=%f\tave=%f",total,tmax,total/n);
	total=0, tmax=0;
	for(iter=elems_new.begin(), end=elems_new.end(); iter!=end; ++iter) {
		ptr=*iter;
		ct.start();
		delete [] (uint8_t*)ptr;
		ct.stop();
		t=ct.get_time();
		if(t>tmax)
			tmax=t;
		total+=t;
	}
	elems_new.clear();
	wyc_print("C++ free:\ttotal=%f\tmax=%f\tave=%f",total,tmax,total/n);
}


void test_pagebuffer() 
{
	std::vector<uint8_t*> pool;
	pool.reserve(256);
	size_t elemsize=sizeof(size_t);
	xpagebuffer page(1024);
	uint8_t *ptr;
	unsigned cnt=1024/(sizeof(size_t)<<1);
	for(unsigned i=0; i<cnt; ++i) {
		ptr=page.alloc(elemsize);
		*(size_t*)ptr=i;
		assert(ptr);
		pool.push_back(ptr);
	}
	for(unsigned i=0; i<10; ++i) {
		ptr=pool.back();
		pool.pop_back();
		wyc_print("free: %d",*(size_t*)ptr);
		page.free(ptr);
	}
	for(unsigned i=0; i<10; ++i) {
		ptr=page.alloc(elemsize);
		assert(ptr);
		pool.push_back(ptr);
		wyc_print("alloc: %d",*(size_t*)ptr);
	}
	std::vector<uint8_t*>::iterator iter, end;
	for(iter=pool.begin(), end=pool.end(); iter!=end; ++iter) {
		page.free(*iter);
	}
	page.clear();	
	wyc_print("test page buffer...OK");
}

extern void test_memobj();

int main(int argc, char **argv)
{
	print_libinfo();
//	profile_mempool(16,256,false);
//	test_mempool(5,16);
//	test_refmem_pool(3,16);
//	test_pagebuffer();
	test_memobj();
	wyc_print("Press [Enter] to continue");
	getchar();
	return 0;
}

