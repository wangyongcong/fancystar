#ifndef __INLINE_WYC_TLS
#define __INLINE_WYC_TLS

namespace wyc
{

inline xthread_local::xthread_local() : m_tls(TLS_OUT_OF_INDEXES)
{
}

inline xthread_local::~xthread_local()
{
	free();
}

inline xthread_local::operator bool() const
{
	return m_tls!=TLS_OUT_OF_INDEXES;
}

inline bool xthread_local::alloc()
{
	assert(m_tls==TLS_OUT_OF_INDEXES);
	m_tls=::TlsAlloc();
	return m_tls!=TLS_OUT_OF_INDEXES;
}

inline void xthread_local::free()
{
	if(m_tls!=TLS_OUT_OF_INDEXES) {
		::TlsFree(m_tls);
		m_tls=TLS_OUT_OF_INDEXES;
	}
}

inline void xthread_local::set_data(void *pdata)
{
	assert(m_tls!=TLS_OUT_OF_INDEXES);
	::TlsSetValue(m_tls,pdata);
}

inline void* xthread_local::get_data() const
{
	assert(m_tls!=TLS_OUT_OF_INDEXES);
	return ::TlsGetValue(m_tls);
}

} // namespace wyc

#endif // __INLINE_WYC_TLS

