#pragma once

#ifndef __HEADER_XBUFFER__
#define __HEADER_XBUFFER__

#include "basedef.h"

//==xbuffer===============================================================================================

class xbuffer
{
	enum BUFFER_INFO
	{
		BI_ELEMENT_SIZE=0xFFFF,
		BI_MEM_ALIGNMENT=0xF0000,
		BI_BIG_ENDIAN=0x100000,
		BI_SHARE=0x200000
	};
	unsigned m_info;
	uint8_t *m_pBuffer;
protected:
	unsigned m_pitch;
	unsigned m_width, m_height;
public:
	xbuffer();
	~xbuffer();
	bool create(unsigned w, unsigned h, unsigned size_elem, unsigned char alignment=4);
	void clear();
	uint8_t* release(unsigned *pitch=0, unsigned *width=0, unsigned *height=0);
	void deliver(xbuffer &accept);
	bool share(const xbuffer &buffer, int x, int y, unsigned w, unsigned h, bool bsafe=true);
	static inline void free(uint8_t *pmem) {
		_free_mem(pmem);
	}
	inline bool empty() const {
		return m_pBuffer==0;
	}
	inline bool little_endian() const {
		return !have_state(m_info,BI_BIG_ENDIAN);
	}
	inline bool is_share() const {
		return have_state(m_info,BI_SHARE);
	}
	inline unsigned char alignment() const {
		return (m_info&BI_MEM_ALIGNMENT)>>16;
	}
	inline unsigned width() const {
		return m_width;
	}
	inline unsigned height() const {
		return m_height;
	}
	inline unsigned pitch() const {
		return m_pitch;
	}
	inline unsigned size() const {
		return m_pitch*m_height;
	}
	inline unsigned size_elem() const {
		return m_info&BI_ELEMENT_SIZE;
	}
	inline bool check_pos(unsigned x, unsigned y) const {
		return (x<m_width && y<m_height);
	}
	inline uint8_t* get_buffer() {
		return m_pBuffer;
	}
	inline uint8_t* get_line(unsigned idx) {
		return m_pBuffer+idx*m_pitch;
	}
	inline uint8_t* get_elem(unsigned x, unsigned y) {
		return m_pBuffer+y*m_pitch+x*size_elem();
	}
	inline const uint8_t* get_elem(unsigned x, unsigned y) const {
		return m_pBuffer+y*m_pitch+x*size_elem();
	}
	template<class T>
	inline T& get(unsigned x, unsigned y) {
		return ((T*)(m_pBuffer+y*m_pitch))[x];
	}
	template<class T>
	inline void set(unsigned x, unsigned y, const T& val) {
		((T*)(m_pBuffer+y*m_pitch))[x]=val;
	}
	template<class T>
	void set_line(unsigned ln, const T& val) {
		T* iter=(T*)(m_pBuffer+ln*m_pitch);
		for(unsigned i=0; i<m_width; ++i) {
			*iter=val;
			iter+=1;
		}
	}
	template<class T>
	void set_line(unsigned ln, const T& val, unsigned begx, unsigned endx) {
		T* iter=(T*)(m_pBuffer+ln*m_pitch);
		for(unsigned i=begx; i<=endx; ++i) {
			*iter=val;
			iter+=1;
		}
	}
	void init(const uint8_t *pdata, unsigned size);
	template<class T>
	void init(const T& val) 
	{
		uint8_t *pline=m_pBuffer;
		for(unsigned y=0; y<m_height; ++y) {
			T* iter=(T*)pline;
			pline+=m_pitch;
			for(unsigned i=0; i<m_width; ++i) {
				*iter=val;
				iter+=1;
			}
		}
	}
	template<class T>
	void move_elem(unsigned dstx, unsigned dsty, unsigned srcx, unsigned srcy, unsigned w, unsigned h) 
	{
		if(dstx==srcx && dsty==srcy)
			return;
		int xoff, yoff;
		uint8_t *psrc, *pdst;
		T *src_iter, *dst_iter;
		if(dsty<=srcy) {
			yoff=int(pitch());
			psrc=get_line(srcy);
			pdst=get_line(dsty);
		}
		else {
			yoff=-int(pitch());
			psrc=get_line(srcy+h-1);
			pdst=get_line(dsty+h-1);
		}
		if(dstx<=srcx) {
			xoff=1;
			psrc+=srcx*sizeof(T);
			pdst+=dstx*sizeof(T);
		}
		else {
			xoff=-1;
			psrc+=(srcx+w-1)*sizeof(T);
			pdst+=(dstx+w-1)*sizeof(T);
		}
		while(h>0) {
			src_iter=(T*)psrc, dst_iter=(T*)pdst;
			for(unsigned i=0; i<w; ++i) {
				*dst_iter=*src_iter;
				src_iter+=xoff;
				dst_iter+=xoff;
			}
			h-=1;
			psrc+=yoff;
			pdst+=yoff;
		}
	}
private:
	static uint8_t* _alloc_mem(unsigned w, unsigned h, unsigned size_elem, 
		unsigned char alignment, unsigned &pitch);
	static void _free_mem(uint8_t* pBuffer);
	xbuffer(const xbuffer &buffer);
	xbuffer& operator = (const xbuffer &buffer);
};

#endif // end of __HEADER_XBUFFER__
