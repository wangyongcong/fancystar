#include "fscorepch.h"
#include "data_map.h"

xmapdata::xmapdata()
{
	m_pmap=0;
	m_mapw=m_maph=0;
}

xmapdata::~xmapdata()
{
	clear();
}

void xmapdata::create(unsigned w, unsigned h)
{
	clear();
	size_t sz=w*h;
	m_pmap=new wyc::uint8_t[sz];
	memset(m_pmap,0,sz);
	m_mapw=w;
	m_maph=h;
}

void xmapdata::clear()
{
	if(m_pmap) {
		delete [] m_pmap;
		m_pmap=0;
		m_mapw=m_maph=0;
	}
}


