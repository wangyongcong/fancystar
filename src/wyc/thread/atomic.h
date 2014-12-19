#ifndef __HEADER_WYC_ATOMIC
#define __HEADER_WYC_ATOMIC

#include <intrin.h>
#include "wyc/basedef.h"
#include "wyc/config.h"

namespace wyc
{

WYC_ALIGN(4) class xatomic32
{
	volatile uint32_t m_val;
public:
	xatomic32() {
		// make sure it's 4 byte aligned
		assert(0==(uintptr_t(this)&(3)));
	}
	// relaxed load/store
	uint32_t load() const;
	void store(uint32_t v);
	// load acquire/store release
	uint32_t load_acq() const;
	void store_rel(uint32_t v);
	// CAS with total order
	// 返回是否成功,原来的值保存在cmp中
	bool compare_exchange(uint32_t &cmp, uint32_t v);
	// increase/decrease with total order
	// 返回修改后的值
	uint32_t increase();
	uint32_t decrease();
	// add with total order
	// 返回原来的值
	uint32_t add(int32_t v);
};

inline uint32_t xatomic32::load() const
{
	return m_val;
}

inline void xatomic32::store(uint32_t v)
{
	m_val=v;
}

inline uint32_t xatomic32::load_acq() const
{
	uint32_t v=m_val;
	_ReadWriteBarrier();
	return v;
}

inline void xatomic32::store_rel(uint32_t v)
{
	_ReadWriteBarrier();
	m_val=v;
}

inline bool xatomic32::compare_exchange(uint32_t &cmp, uint32_t xchg)
{
	uint32_t prev=InterlockedCompareExchange((volatile LONG*)&m_val,xchg,cmp);
	if(prev==cmp)
		return true;
	cmp=prev;
	return false;
}

inline uint32_t xatomic32::increase()
{
	return InterlockedIncrement((volatile LONG*)&m_val);
}

inline uint32_t xatomic32::decrease()
{
	return InterlockedDecrement((volatile LONG*)&m_val);
}

inline uint32_t xatomic32::add(int32_t v)
{
	return InterlockedExchangeAdd((volatile LONG*)&m_val,v);
}

template<typename T>
class xatomic;

template<>
class xatomic<unsigned> : public xatomic32
{
public:
	xatomic(unsigned ival=0) {
		xatomic32::store((uint32_t)ival);
	}
	inline xatomic& operator = (unsigned ival) {
		xatomic32::store((uint32_t)ival);
		return *this;
	}
	operator unsigned () const {
		return load();
	}
};

template<>
class xatomic<int> : private xatomic32
{
public:
	xatomic(int ival=0) {
		store(ival);
	}
	inline xatomic& operator = (int ival) {
		store(ival);
		return *this;
	}
	operator int () const {
		return load();
	}
	// relaxed load/store
	inline int load() const {
		return (int)xatomic32::load();
	}
	inline void store(int v) {
		xatomic32::store((uint32_t)v);
	}
	// load acquire/store release
	inline int load_acq() const {
		return (int)xatomic32::load_acq();
	}
	inline void store_rel(int v) {
		xatomic32::store_rel((uint32_t)v);
	}
	// CAS with total order
	inline bool compare_exchange(int &cmp, int v) {
		return xatomic32::compare_exchange((uint32_t&)cmp,(int)v);
	}
	// increase/decrease with total order
	inline int increase() {
		return (int)xatomic32::increase();
	}
	inline int decrease() {
		return (int)xatomic32::decrease();
	}
};

template<>
class xatomic<bool> : private xatomic32
{
public:
	xatomic(bool b=false) {
		store(b);
	}
	inline xatomic& operator = (bool b) {
		store(b);
		return *this;
	}
	operator bool () const {
		return load();
	}
	// relaxed load/store
	inline bool load() const {
		uint32_t v=xatomic32::load();
		return v==1;
	}
	inline void store(bool b) {
		uint32_t v=b?1:0;
		xatomic32::store(v);
	}
	// load acquire/store release
	inline bool load_acq() const {
		uint32_t v=xatomic32::load_acq();
		return v==1;
	}
	inline void store_rel(bool b) {
		uint32_t v=b?1:0;
		xatomic32::store_rel(v);
	}
	// CAS with total order
	inline bool compare_exchange(bool cmp, bool b) {
		uint32_t icmp=cmp?1:0;
		uint32_t v=b?1:0;
		return xatomic32::compare_exchange(icmp,v);
	}
};

}; // namespace wyc

#endif // __HEADER_WYC_ATOMIC

