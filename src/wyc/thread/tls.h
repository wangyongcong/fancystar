#ifndef __HEADER_WYC_TLS
#define __HEADER_WYC_TLS

#include <cassert>
#include "wyc/platform.h"

namespace wyc
{

class xthread_local
{
	DWORD m_tls;
public:
	xthread_local();
	~xthread_local();
	bool alloc();
	void free();
	operator bool() const;
	void set_data(void *pdata);
	void* get_data() const;
};

} // namespace wyc

#include "wyc/thread/tls.inl"

#endif // __HEADER_WYC_TLS


