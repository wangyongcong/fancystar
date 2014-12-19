#ifndef __HEADER_BOMBMAN_DATA_MAP
#define __HEADER_BOMBMAN_DATA_MAP

#include "basedef.h"
#include "xrefobj.h"

class xmapdata : public wyc::xrefobj
{
	wyc::uint8_t *m_pmap;
	unsigned m_mapw, m_maph;
public:
	xmapdata();
	virtual ~xmapdata();
	void create(unsigned w, unsigned h);
	void clear();
	inline unsigned width() const {
		return m_mapw;
	}
	inline unsigned height() const {
		return m_maph;
	}
	inline wyc::uint8_t get(unsigned x, unsigned y) const {
		return m_pmap[y*m_maph+x];
	}
	inline wyc::uint8_t set(unsigned x, unsigned y, wyc::uint8_t data) {
		m_pmap[y*m_maph+x]=data;
	}
	inline wyc::uint8_t get(unsigned idx) const {
		return m_pmap[idx];
	}
	inline wyc::uint8_t set(unsigned idx, wyc::uint8_t data) const {
		return m_pmap[idx]=data;
	}
	inline wyc::uint8_t operator [] (size_t idx) const {
		return m_pmap[idx];
	}
	inline wyc::uint8_t& operator [] (size_t idx) {
		return m_pmap[idx];
	}
	bool load(const char *file);
	bool save(const char *file) const;
};

#endif //__HEADER_BOMBMAN_DATA_MAP

