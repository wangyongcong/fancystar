#ifndef __HEADER_WYC_PRIORQUE
#define __HEADER_WYC_PRIORQUE

#include <cstring>

namespace wyc
{

template<typename T, typename Cmp>
class xpriorque
{
	T *m_que;
	size_t m_size;
	size_t m_capacity;
public:
	xpriorque(size_t sz=0);
	~xpriorque();
	void push(const T &elem);
	bool pop(T &elem);
	void refresh(const T &elem);
	void resize(size_t sz);
	void clear();
	size_t capacity() const;
	size_t size() const;
	bool empty() const;
private:
	// not allow copy operations
	xpriorque(const xpriorque &que);
	xpriorque& operator = (const xpriorque &que);
	// memory alloc
	T *__alloc(size_t sz);
	void __free(T *pmem);
	// traversal
	bool __move_up(size_t cid);
	bool __move_down(size_t pid);
};

}; // namespace wyc

#include "wyc/util/priorque.inl"

#endif // __HEADER_WYC_PRIORQUE

