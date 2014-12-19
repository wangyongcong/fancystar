#ifndef __HEADER_WYC_SHARE_BUFFER
#define __HEADER_WYC_SHARE_BUFFER

#include "basedef.h"
#include "xrefobj.h"

namespace wyc
{

class xshare_buffer : public xrefobj
{
	uint8_t *m_pbuff;
	size_t m_size;
public:
	xshare_buffer() {
		m_pbuff=0;
		m_size=0;
	}
	virtual ~xshare_buffer() {
		if(m_pbuff) {
			delete [] m_pbuff;
			m_pbuff=0;
		}
	}
	inline void malloc(size_t size) {
#ifdef _DEBUG
		m_pbuff=new uint8_t[size+sizeof(uint32_t)];
		*(uint32_t*)(m_pbuff+size)=0xABABABAB;
#else
		m_pbuff=new uint8_t[size];
#endif
		m_size=size;
	}
	inline void free() {
		delete [] m_pbuff;
		m_pbuff=0;
		m_size=0;
	}
	inline void* get_buffer() {
		return m_pbuff;
	}
	inline size_t size() const {
		return m_size;
	}
	inline void validate() const {
		assert(0xABABABAB==*(uint32_t*)(m_pbuff+m_size));
	}
};

} // namespace wyc

#endif // __HEADER_WYC_SHARE_BUFFER
