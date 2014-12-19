#include <cassert>
#include "xraster.h"

#pragma warning(disable: 4244)

//===================================================================================================
// xraster 像素绘制
//===================================================================================================

void xraster::_bitmap(unsigned dstx, unsigned dsty, uint8_t *pbits, unsigned src_pitch, 
						   unsigned srcx, unsigned srcy, unsigned srcw, unsigned srch)
{
	assert(dstx<width() && dsty<height());
	assert(dstx+srcw<=width() && dsty+srch<=height());
	/*
	unsigned _width=width();
	unsigned _height=height();
	if(dstx>=_width || dsty>=_height)
		return;
	if(dstx+srcw>_width) 
		srcw=_width-dstx;
	if(dsty+srch>_height)
		srch=_height-dsty;*/
	unsigned dst_pitch=pitch();
	unsigned dst_stride=stride();
	uint8_t *pline=get_pixel(dstx,dsty);
	pbits+=srcy*src_pitch;
	if(srcx>0) {
		srcw+=srcx;
		pbits+=srcx>>3;
	}
	uint8_t color;
	for(unsigned i=0; i<srch; ++i) {
		uint8_t *pdst=pline;
		color=(*pbits)<<(srcx&0x7);
		m_plotter(this,pdst,(color&0x80)?&m_color:&m_bkcolor);
		for(unsigned j=srcx+1; j<srcw; ++j, pdst+=dst_stride) {
			if(j&0x7)
				color<<=1;
			else color=*(pbits+(j>>3));
			m_plotter(this,pdst,(color&0x80)?&m_color:&m_bkcolor);
		}
		pline+=dst_pitch;
		pbits+=src_pitch;
	}
}

void xraster::_bitmap_transparent(unsigned dstx, unsigned dsty, uint8_t *pbits, unsigned src_pitch, 
	unsigned srcx, unsigned srcy, unsigned srcw, unsigned srch)
{
	assert(dstx<width() && dsty<height());
	assert(dstx+srcw<=width() && dsty+srch<=height());
	unsigned dst_pitch=pitch();
	unsigned dst_stride=stride();
	uint8_t *pline=get_pixel(dstx,dsty);
	pbits+=srcy*src_pitch;
	if(srcx>0) {
		srcw+=srcx;
		pbits+=srcx>>3;
	}
	uint8_t color;
	for(unsigned i=0; i<srch; ++i) {
		uint8_t *pdst=pline;
		color=(*pbits)<<(srcx&0x7);
		if(color&0x80)
			m_plotter(this,pdst,&m_color);
		for(unsigned j=srcx+1; j<srcw; ++j, pdst+=dst_stride) {
			if(j&0x7)
				color<<=1;
			else color=*(pbits+(j>>3));
			if(color&0x80)
				m_plotter(this,pdst,&m_color);
		}
		pline+=dst_pitch;
		pbits+=src_pitch;
	}
}

void xraster::_bitblt(unsigned dstx, unsigned dsty, uint8_t *pbits, XG_PIXEL_FORMAT fmt, 
			unsigned src_pitch, unsigned srcx, unsigned srcy, unsigned srcw, unsigned srch)
{
	assert(dstx<width() && dsty<height());
	assert(dstx+srcw<=width() && dsty+srch<=height());
	/*
	unsigned _width=width();
	unsigned _height=height();
	if(dstx>=_width || dsty>=_height)
		return;
	if(dstx+srcw>_width) 
		srcw=_width-dstx;
	if(dsty+srch>_height)
		srch=_height-dsty;*/
	xplotter translater=get_translater(fmt);
	assert(translater!=0);
	unsigned dst_pitch=pitch();
	unsigned dst_stride=stride();
	unsigned src_stride=pixel_size(fmt);
	uint8_t *pline=get_pixel(dstx,dsty);
	pbits+=srcy*src_pitch+srcx*src_stride;
	pixel_t c;
	for(unsigned i=0; i<srch; ++i) {
		uint8_t *pdst=pline;
		uint8_t *psrc=pbits;
		for(unsigned j=0; j<srcw; ++j, psrc+=src_stride, pdst+=dst_stride) {
			translater(this,&c,psrc);
			m_plotter(this,pdst,&c);
		}
		pline+=dst_pitch;
		pbits+=src_pitch;
	}
}

void xraster::_bitblt_colorkey(unsigned dstx, unsigned dsty, uint8_t *pbits, XG_PIXEL_FORMAT fmt, 
			unsigned src_pitch, unsigned srcx, unsigned srcy, unsigned srcw, unsigned srch)
{
	assert(dstx<width() && dsty<height());
	assert(dstx+srcw<=width() && dsty+srch<=height());
	xplotter translater=get_translater(fmt);
	assert(translater!=0);
	unsigned dst_pitch=pitch();
	unsigned dst_stride=stride();
	unsigned src_stride=pixel_size(fmt);
	uint8_t *pline=get_pixel(dstx,dsty);
	pbits+=srcy*src_pitch+srcx*src_stride;
	pixel_t c;
	for(unsigned i=0; i<srch; ++i) {
		uint8_t *pdst=pline;
		uint8_t *psrc=pbits;
		for(unsigned j=0; j<srcw; ++j, psrc+=src_stride, pdst+=dst_stride) {
			translater(this,&c,psrc);
			if(c!=m_colorkey)
				m_plotter(this,pdst,&c);
		}
		pline+=dst_pitch;
		pbits+=src_pitch;
	}
}

void xraster::_bitblt_stretch_neareast(unsigned dstx, unsigned dsty, unsigned dstw, unsigned dsth, 
			uint8_t *pbits, XG_PIXEL_FORMAT fmt, unsigned src_pitch, unsigned srcx, unsigned srcy, 
			unsigned srcw, unsigned srch, float u0, float v0, float u1, float v1) 
{
	unsigned _width=width();
	unsigned _height=height();
	if(dstx>=_width || dsty>=_height)
		return;
	if(dstx+dstw>_width) 
		dstw=_width-dstx;
	if(dsty+dsth>_height)
		dsth=_height-dsty; 
	xplotter translater=get_translater(fmt);
	assert(translater!=0);
	unsigned dst_pitch=pitch();
	unsigned dst_stride=stride();
	unsigned src_stride=pixel_size(fmt);
	float stepx=(u1-u0)/dstw, stepy=(v1-v0)/dsth;
	unsigned *sx=new unsigned[dstw];
	for(unsigned j=0; j<dstw; ++j) {
		sx[j]=(srcx+fast_floor(u0*srcw))*src_stride;
		u0+=stepx;
	}
	uint8_t *pline=get_pixel(dstx,dsty);
	pixel_t c;
	for(unsigned i=0; i<dsth; ++i) {
		uint8_t *pdst=pline;
		uint8_t *psrc=pbits+(srcy+fast_floor(v0*srch))*src_pitch;
		for(unsigned j=0; j<dstw; ++j, pdst+=dst_stride) {
			translater(this,&c,psrc+sx[j]);
			m_plotter(this,pdst,&c);
		}
		v0+=stepy;
		pline+=dst_pitch;
	}
	delete [] sx;
}

void xraster::_bitblt_stretch_bilinear(unsigned dstx, unsigned dsty, unsigned dstw, unsigned dsth, 
			uint8_t *pbits, XG_PIXEL_FORMAT fmt, unsigned src_pitch, unsigned srcx, unsigned srcy, 
			unsigned srcw, unsigned srch, float u0, float v0, float u1, float v1) 
{
	assert(NATIVE_PIXEL_FORMAT==PIXEL_FMT_RGBA8888);
	unsigned _width=width();
	unsigned _height=height();
	if(dstx>=_width || dsty>=_height)
		return;
	if(dstx+dstw>_width) 
		dstw=_width-dstx;
	if(dsty+dsth>_height)
		dsth=_height-dsty;
	xplotter translater=get_translater(fmt);
	assert(translater!=0);
	// 采样公式为:
	// dstx=srcx+floor(tx*(srcw-1)), dsty=srcy+floor(ty*(srch-1)), 0<=tx,ty<1
	unsigned dst_pitch=pitch();
	unsigned dst_stride=stride(), src_stride=pixel_size(fmt);
	float stepx=(u1-u0)/(dstw-1), stepy=(v1-v0)/(dsth-1);
	srcw-=1;
	srch-=1;
	// 预计算每行的比例和采样地址
	float *tx=new float[dstw];
	unsigned *sx=new unsigned[dstw];
	tx[0]=u0;
	sx[0]=(srcx+fast_floor(u0*srcw))*src_stride;
	for(unsigned j=1; j<dstw; ++j) {
		u0+=stepx;
		u1=u0*srcw;
		unsigned x=fast_floor(u1);
		tx[j]=u1-x;
		sx[j]=(srcx+x)*src_stride;
	}
	// 绘制像素
	xrgba8888 c0, c1, c2, c3;
	uint8_t *pline=get_pixel(dstx,dsty);
	for(unsigned i=0; i<dsth; ++i) {
		v1=v0*srch;
		unsigned sy=fast_floor(v1);
		v1-=sy;
		uint8_t *pdst=pline;
		uint8_t *psrcbeg=pbits+(srcy+sy)*src_pitch;
		for(unsigned j=0; j<dstw; ++j, pdst+=dst_stride) {
			// 取出样本
			uint8_t *psrc=psrcbeg+sx[j];
			translater(this,&c0,psrc);
			translater(this,&c1,psrc+src_stride);
			psrc+=src_pitch;
			translater(this,&c2,psrc);
			translater(this,&c3,psrc+src_stride);
			// 插值计算最终颜色
			u1=tx[j];
			c0.set(c0.red*(1-u1)+c1.red*u1,c0.green*(1-u1)+c1.green*u1,c0.blue*(1-u1)+c1.blue*u1,c0.alpha*(1-u1)+c1.alpha*u1);
			c1.set(c2.red*(1-u1)+c3.red*u1,c2.green*(1-u1)+c3.green*u1,c2.blue*(1-u1)+c3.blue*u1,c2.alpha*(1-u1)+c3.alpha*u1);
			c0.set(c0.red*(1-v1)+c1.red*v1,c0.green*(1-v1)+c1.green*v1,c0.blue*(1-v1)+c1.blue*v1,c0.alpha*(1-v1)+c1.alpha*v1);
			m_plotter(this,pdst,&c0);
		}
		v0+=stepy;
		pline+=dst_pitch;
	}
	delete [] tx;
	delete [] sx;
}

void xraster::_fast_blend(unsigned dstx, unsigned dsty, uint8_t *pbits, unsigned pitch, 
			unsigned srcx, unsigned srcy, unsigned srcw, unsigned srch, uint8_t fact) 
{
	assert(NATIVE_PIXEL_FORMAT==PIXEL_FMT_RGBA8888);
	assert(dstx<width() && dsty<height());
	assert(dstx+srcw<=width() && dsty+srch<=height());

	unsigned _pitch=m_colorBuffer.pitch();
	uint8_t *psrc=pbits+srcy*pitch+(srcx<<2);
	uint8_t *pdst=m_colorBuffer.get_elem(dstx,dsty);

	if (fact == 128 || fact == 127) {
		for (unsigned j = 0; j < srch; j++) {
			uint32_t color1, color2;
			uint32_t *s=(uint32_t*)(psrc), *d=(uint32_t*)(pdst);
			for (unsigned i = srcw; i; i--) {
				// Read data, 1 pixel at a time 
				color2 = *s;
				color1 = *d;
				if (color2 == m_colorkey) {
					++s;
					++d;
					continue;
				}
				color1 =  ((color1 & 0xFEFEFE) >> 1)
						+ ((color2 & 0xFEFEFE) >> 1)
						+ (color1 & color2 & 0x010101);
	 			// Write the data 
				*d=color1 | 0xFF000000;
				++s;
				++d;
			}
			psrc+=pitch;
			pdst+=_pitch;
		}
	}
	else {
		for (unsigned j = 0; j < srch; j++) {
			uint32_t color1, color2;
			uint32_t *s=(uint32_t*)(psrc), *d=(uint32_t*)(pdst);
			for (unsigned i = srcw; i; i--) {
				uint32_t temp1, temp2;
				// Read data, 1 pixel at a time
				color2 = *s;
				color1 = *d;
				if (color2 == m_colorkey) {
					++s;
					++d;
					continue;
				}
				// Mutiply by the factor
				temp2 = color1 & 0xFF00FF;
				temp1 = (color2 & 0xFF00FF) - temp2;
				temp1 = (((temp1 * fact) >> 8) + temp2) & 0xFF00FF;
				color1 &= 0xFF00;
				color2 &= 0xFF00;
				temp2 = ((((color2 - color1) * fact) >> 8) + color1) & 0xFF00;
	 			// Write the data
				*d=temp1 | temp2 | 0xFF000000;
				++s;
				++d;
			}
			psrc+=pitch;
			pdst+=_pitch;
		}
	}
	return;
}

void xraster::_bitblt_blur(float fdstx, float fdsty, uint8_t *pbits, XG_PIXEL_FORMAT fmt, 
			unsigned pitch, unsigned srcx, unsigned srcy, unsigned srcw, unsigned srch)
{
	unsigned _width=width();
	unsigned _height=height();
	unsigned dstx, dsty, dstendx, dstendy;
	float fx, fy;
	bool first_line, first_pixel;
	if(fdstx>=0) {
		dstx=fast_floor(fdstx);
		if(dstx>=_width)
			return;
		dstendx=dstx+srcw;
		if(dstendx>_width)
			dstendx=_width;
		fx=fdstx-dstx;
		first_pixel=true;
	}
	else {
		fx=fdstx+srcw;
		if(fx<0) return;
		int offx=fast_ceil(fdstx);
		fx=1-(offx-fdstx);
		srcx-=offx;
		dstendx=srcw+offx;
		dstx=0;
		first_pixel=false;
	}
	if(fdsty>=0) {
		dsty=fast_floor(fdsty);
		if(dsty>=_height)
			return;
		dstendy=dsty+srch;
		if(dstendy>_height)
			dstendy=_height;
		fy=fdsty-dsty;
		first_line=true;
	}
	else {
		fy=fdsty+srch;
		if(fy<0) return;
		int offy=fast_ceil(fdsty);
		fy=1-(offy-fdsty);
		srcy-=offy;
		dstendy=srch+offy;
		dsty=0;
		first_line=false;
	}
	float fac1, fac2, fac3, fac4;
	fac1=(1-fx)*(1-fy);
	fac2=fx*(1-fy);
	fac3=(1-fx)*fy;
	fac4=fx*fy;
	assert(NATIVE_PIXEL_FORMAT==PIXEL_FMT_RGBA8888);
	assert(m_pixelfmt==NATIVE_PIXEL_FORMAT);
	xplotter translater=get_translater(fmt);
	assert(translater!=0);
	unsigned _pitch=this->pitch();
	unsigned dstelem=stride();
	unsigned srcelem=pixel_size(fmt);
	uint8_t *pline=get_pixel(dstx,dsty);
	pbits+=srcy*pitch+srcx*srcelem;
	uint8_t *pnext=pbits+pitch;
	xrgba8888 s1, s2, s3, s4, c;
	uint8_t *pdst=pline, *psrc=pbits, *psrc_next=pnext;
	unsigned i, j;
	// the first line
	if(first_line) {
		translater(this,&s2,psrc);
		if(first_pixel) {
			c.set(s2.red*fac1,s2.green*fac1,s2.blue*fac1,s2.alpha*fac1);
			m_plotter(this,pdst,&c);
		}
		for(j=dstx+1; j<dstendx; ++j) 
		{
			pdst+=dstelem;
			psrc+=srcelem;
			s1=s2;
			translater(this,&s2,psrc);
			c.set(s1.red*fac2+s2.red*fac1,
				s1.green*fac2+s2.green*fac1,
				s1.blue*fac2+s2.blue*fac1,
				s1.alpha*fac2+s2.alpha*fac1);
			m_plotter(this,pdst,&c);
		}
		if(j<_width) {
			c.set(s2.red*fac2,s2.green*fac2,s2.blue*fac2,s2.alpha*fac2);
			m_plotter(this,pdst+dstelem,&c);
		}
		pline+=_pitch;
	}
	// the middle part
	for(i=dsty+1; i<dstendy; ++i) {
		pdst=pline;
		psrc=pbits;
		psrc_next=pnext;
		// the first pixel of line
		translater(this,&s2,psrc);
		translater(this,&s4,psrc_next);
		if(first_pixel) {
			c.set(s2.red*fac3+s4.red*fac1,
				s2.green*fac3+s4.green*fac1,
				s2.blue*fac3+s4.blue*fac1,
				s2.alpha*fac3+s4.alpha*fac1);
			m_plotter(this,pdst,&c);
		}
		// the middle part of line
		for(j=dstx+1; j<dstendx; ++j) {
			pdst+=dstelem;
			psrc+=srcelem;
			psrc_next+=srcelem;
			s1=s2;
			translater(this,&s2,psrc);
			s3=s4;
			translater(this,&s4,psrc_next);
			c.set(s1.red*fac4+s2.red*fac3+s3.red*fac2+s4.red*fac1,
				s1.green*fac4+s2.green*fac3+s3.green*fac2+s4.green*fac1,
				s1.blue*fac4+s2.blue*fac3+s3.blue*fac2+s4.blue*fac1,
				s1.alpha*fac4+s2.alpha*fac3+s3.alpha*fac2+s4.alpha*fac1);
			m_plotter(this,pdst,&c);
		}
		// the last pixel of line
		if(j<_width) {
			pdst+=dstelem;
			c.set(s2.red*fac2+s4.red*fac4,
				s2.green*fac2+s4.green*fac4,
				s2.blue*fac2+s4.blue*fac4,
				s2.alpha*fac2+s4.alpha*fac4);
			m_plotter(this,pdst,&c);
		}
		// next line
		pline+=_pitch;
		pbits=pnext;
		pnext+=pitch;
	}
	// the last line
	if(i<_height) {
		pdst=pline;
		psrc=pbits;
		translater(this,&s2,psrc);
		if(first_pixel) {
			c.set(s2.red*fac3,s2.green*fac3,s2.blue*fac3,s2.alpha*fac3);
			m_plotter(this,pdst,&c);
		}
		for(j=dstx+1; j<dstendx; ++j) 
		{
			pdst+=dstelem;
			psrc+=srcelem;
			s1=s2;
			translater(this,&s2,psrc);
			c.set(s1.red*fac4+s2.red*fac3,
				s1.green*fac4+s2.green*fac3,
				s1.blue*fac4+s2.blue*fac3,
				s1.alpha*fac4+s2.alpha*fac3);
			m_plotter(this,pdst,&c);
		}
		if(j<_width) {
			c.set(s2.red*fac4,s2.green*fac4,s2.blue*fac4,s2.alpha*fac4);
			m_plotter(this,pdst+dstelem,&c);
		}
	}
}



