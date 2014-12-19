#ifndef __HEADER_WYC_XPOINTER
#define __HEADER_WYC_XPOINTER

namespace wyc
{

template<typename T>
class xpointer
{
	T *m_ptr;
public:
	xpointer(T *ptr=0) : m_ptr(ptr) {
		if(m_ptr)
			m_ptr->incref();
	}
	xpointer(const xpointer& sp) : m_ptr(0) {
		*this=sp;
	}
	~xpointer() {
		if(m_ptr)
			m_ptr->decref();
	}
	inline xpointer& operator = (const xpointer& sp) {
		if(m_ptr!=sp.m_ptr) {
			if(m_ptr)
				m_ptr->decref();
			m_ptr=sp.m_ptr;
			if(m_ptr)
				m_ptr->incref();
		}
		return *this;
	}
	inline xpointer& operator = (T *ptr) {
		if(m_ptr!=ptr) {
			if(m_ptr) 
				m_ptr->decref();
			m_ptr=ptr;
			if(m_ptr)
				m_ptr->incref();
		}
		return *this;
	}
	inline operator T* () const {
		return m_ptr;
	}
	inline T& operator* () const {
		return *m_ptr;
	}
	inline T* operator-> () const {
		return m_ptr;
	}
	inline bool operator == (const xpointer& sp) const {
		return m_ptr==sp.m_ptr;
	}
	inline bool operator != (const xpointer& sp) const {
		return m_ptr!=sp.m_ptr;
	}
	inline bool operator == (T *ptr) const {
		return m_ptr==ptr;
	}
	inline bool operator != (T *ptr) const {
		return m_ptr!=ptr;
	}
	inline bool operator < (const xpointer& sp) const {
		return m_ptr<sp.m_ptr;
	}
	inline bool operator > (const xpointer& sp) const {
		return m_ptr>sp.m_ptr;
	}
	inline bool operator < (T *ptr) const {
		return m_ptr<ptr;
	}
	inline bool operator > (T *ptr) const {
		return m_ptr>ptr;
	}
	inline void swap (xpointer &sp) {
		T *tmp=m_ptr;
		m_ptr=sp.m_ptr;
		sp.m_ptr=tmp;
	}
	inline void set (T *ptr) {
		m_ptr=ptr;
	}
	// be careful! the caset is unsafe
	// make sure you know the actual type
	template<typename T2>
	inline operator T2* () const {
		return (T2*)m_ptr;
	}
};

} // namespace wyc

#endif // __HEADER_WYC_XPOINTER

