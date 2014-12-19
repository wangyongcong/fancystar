#ifndef __HEADER_WYC_SPSC_RING
#define __HEADER_WYC_SPSC_RING

#include <atomic>
#include "wyc/basedef.h"
#include "wyc/config.h"
#include "wyc/util/util.h"

namespace wyc
{

#define CONFIRM_CACHE_ALIGN(ptr) assert(IS_CACHE_ALIGN(ptr))

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4324)
#endif
template<typename T>
class WYC_CACHE_ALIGN xspsc_ring
{
	// share variable
	std::atomic_uint32_t m_read;
	std::atomic_uint32_t m_write;
	// consumer local
	WYC_CACHE_ALIGN uint32_t m_local_write;
	uint32_t m_next_read;
	uint32_t m_batch_read;
	// producer only
	WYC_CACHE_ALIGN uint32_t m_local_read;
	uint32_t m_next_write;
	uint32_t m_batch_write;
	// constant info
	WYC_CACHE_ALIGN uint32_t m_capacity_mask;
	uint32_t m_batch_size;
	// data array
	WYC_CACHE_ALIGN T *m_elements;
public:
	xspsc_ring(unsigned capacity, unsigned batch_size=0)
	{
#ifdef _DEBUG
		CONFIRM_CACHE_ALIGN(this);
		CONFIRM_CACHE_ALIGN(&m_local_write);
		CONFIRM_CACHE_ALIGN(&m_local_read);
		CONFIRM_CACHE_ALIGN(&m_capacity_mask);
		CONFIRM_CACHE_ALIGN(&m_elements);
#endif
		assert(batch_size<capacity);
		capacity=power2(capacity);
		assert(capacity);
		m_read=0;
		m_write=0;
		m_local_write=m_next_read=m_batch_read=0;
		m_local_read=m_next_write=m_batch_write=0;
		m_capacity_mask=capacity-1;
		m_batch_size=batch_size;
		m_elements=new T[capacity];
	}
	~xspsc_ring()
	{
		delete [] m_elements;
	}
	// producer
	bool push(const T &elem) {
		uint32_t after_next=(m_next_write+1)&m_capacity_mask;
		if(after_next==m_local_read) {
			uint32_t read_pos=m_read.load(std::memory_order_acquire);
			if(after_next==read_pos)
				// queue is full
				return false;
			m_local_read=read_pos;
		}
		m_elements[m_next_write]=elem;
		m_next_write=after_next;
		m_batch_write+=1;
		if(m_batch_write>=m_batch_size) {
			m_batch_write=0;
			m_write.store(m_next_write,std::memory_order_release);
		}
		return true;
	}
	void flush_write_batch()
	{
		m_batch_write=0;
		m_write.store(m_next_write,std::memory_order_release);
	}
	// consumer
	bool pop(T &elem) {
		if(m_next_read==m_local_write) {
			uint32_t write_pos=m_write.load(std::memory_order_acquire);
			if(m_next_read==write_pos)
				// queue is empty
				return false;
			m_local_write=write_pos;
		}
		elem=m_elements[m_next_read];
		m_next_read=(m_next_read+1)&m_capacity_mask;
		m_batch_read+=1;
		if(m_batch_read>=m_batch_size) {
			m_batch_read=0;
			m_read.store(m_next_read,std::memory_order_release);
		}
		return true;
	}
	unsigned capacity() const {
		return m_capacity_mask+1;
	}
	unsigned batch_size() const {
		return m_batch_size;
	}
};
#ifdef _MSC_VER
#pragma warning(pop)
#endif

}; // namespace wyc

#endif // __HEADER_WYC_SPSC_RING
