#include <cstring>
#include "xbuffer.h"

uint8_t* xbuffer::_alloc_mem(unsigned w, unsigned h, unsigned size_elem, unsigned char alignment, unsigned &pitch) 
{
	if(alignment<2) pitch=w*size_elem;
	pitch=(w*size_elem+alignment-1)/alignment*alignment;
	unsigned size=pitch*h;
	if(size<1) return 0;
	return new uint8_t[size];
}

void xbuffer::_free_mem(uint8_t* pBuffer) 
{
	delete [] pBuffer;
}

xbuffer::xbuffer() 
{
	m_info=0;
	m_pBuffer=0;
	m_pitch=0;
	m_width=0;
	m_height=0;
}

xbuffer::~xbuffer() 
{
	if(!empty() && !is_share()) 
		_free_mem(m_pBuffer);
}

bool xbuffer::create(unsigned w, unsigned h, unsigned size_elem, unsigned char alignment) {
	if(!empty() && !is_share()) 
		_free_mem(m_pBuffer);
	m_pBuffer=_alloc_mem(w,h,size_elem,alignment,m_pitch);
	if(m_pBuffer==0)
		return false;
	m_info=((alignment&0xF)<<16)|(size_elem&0xFFFF);
	m_width=w;
	m_height=h;
	return true;
}

void xbuffer::clear() {
	if(!empty()) {
		if(!is_share())
			_free_mem(m_pBuffer);
		m_pBuffer=0;
		m_info=0;
		m_pitch=0;
		m_width=0;
		m_height=0;
	}
}

uint8_t* xbuffer::release(unsigned *pitch, unsigned *width, unsigned *height) {
	uint8_t *pBuffer=m_pBuffer;
	if(pitch) 
		*pitch=m_pitch;
	if(width) 
		*width=m_width;
	if(height) 
		*height=m_height;
	m_info=0;
	m_pBuffer=0;
	m_pitch=0;
	m_width=0;
	m_height=0;
	return pBuffer;
}

void xbuffer::deliver(xbuffer &accept)
{
	accept.m_info=m_info;
	accept.m_pBuffer=m_pBuffer;
	accept.m_pitch=m_pitch;
	accept.m_width=m_width;
	accept.m_height=m_height;
	m_pBuffer=0;
	m_info=0;
	m_pitch=0;
	m_width=0;
	m_height=0;
}

bool xbuffer::share(const xbuffer &buffer, int x, int y, unsigned w, unsigned h, bool bsafe)
{
	if(x>=int(buffer.m_width) || y>=int(buffer.m_height))
		return false;
	if(bsafe) {
		if(x<0) {
			w+=x;
			x=0;
		}
		if(y<0) {
			h+=y;
			y=0;
		}
	}
	if(!empty() && !is_share()) 
		_free_mem(m_pBuffer);
	m_pBuffer=buffer.m_pBuffer+y*buffer.m_pitch+x*buffer.size_elem();		
	m_info=buffer.m_info|BI_SHARE;
	m_pitch=buffer.m_pitch;
	m_width=x+w>buffer.m_width?buffer.m_width-x:w;
	m_height=y+h>buffer.m_height?buffer.m_height-y:h;
	return true;
}

void xbuffer::init(const uint8_t *pdata, unsigned size) {
	unsigned cnt, packed;
	if(size>m_pitch)
		size=m_pitch;
	else {
		cnt=m_pitch/size;
		packed=m_pitch%size;
	}
	uint8_t *pbuff=m_pBuffer;
	for(unsigned y=0; y<m_height; ++y) {
		uint8_t *pline=pbuff;
		pbuff+=m_pitch;
		for(unsigned i=0; i<cnt; ++i) {
			memcpy(pline,pdata,size);
			pline+=size;
		}
	//	if(packed>0) 
	//		memset(pline,0,packed);
	}
}

