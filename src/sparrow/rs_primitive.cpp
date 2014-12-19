#include <cassert>
#include <algorithm>
#include "xraster.h"

using namespace std;

#pragma warning(disable: 4244)

//===================================================================================================
// xraster 基本图元光栅化操作
//===================================================================================================

#if NATIVE_PIXEL_FORMAT==PIXEL_FMT_RGBA8888
	// 计算并设置ALPHA
	#define TRANSPARENT(c,a,t) ((uint8_t*)&(c))[CHANNEL_ALPHA]=uint8_t((a)*(t))
#else
	#define TRANSPARENT(c,a,t) c=transparency(t)
#endif

#ifdef DIRECT_MEMORY_ACCESS 

void xraster::line_bresenham(int x0, int y0, int x1, int y1)
{
	if(x0>x1) {
		swap(x0,x1);
		swap(y0,y1);
	}
	int deltax=x1-x0;
	int deltay=y1-y0;
	uint8_t *pw=m_colorBuffer.get_elem(x0,y0);
	int elem=m_colorBuffer.size_elem();
	int pitch=m_colorBuffer.pitch();
	if(deltay<0)
	{
		deltay=-deltay;
		pitch=-pitch;
	}
	m_plotter(this,pw,&m_color);
	if(deltax>=deltay)
	{
		int m=deltay<<1;
		int d=-deltax;
		for(int i=0;i<deltax;++i)
		{
			pw+=elem;
			d+=m;
			if(d>=0)
			{
				pw+=pitch;
				d-=deltax<<1;
			}
			m_plotter(this,pw,&m_color);
		}
	}
	else
	{
		int m=deltax<<1;
		int d=-deltay;
		for(int i=0;i<deltay;++i)
		{
			pw+=pitch;
			d+=m;
			if(d>=0)
			{
				pw+=elem;
				d-=deltay<<1;
			}
			m_plotter(this,pw,&m_color);
		}
	}
}

void xraster::line_bresenham_anti(int x0, int y0, int x1, int y1)
{
	if(x0>x1) {
		swap(x0,x1);
		swap(y0,y1);
	}
	int deltax=x1-x0;
	int deltay=y1-y0;
	uint8_t *pw=m_colorBuffer.get_elem(x0,y0);
	int elem=m_colorBuffer.size_elem();
	int pitch=m_colorBuffer.pitch();
	if(deltay<0)
	{
		deltay=-deltay;
		pitch=-pitch;
	}
	int d=0;
	pixel_t c=m_color;
	uint8_t a=ALPHA_CHANNEL(m_color);
	uint8_t at;
	m_plotter(this,pw,&m_color);
	if(deltay<deltax)
	{
		float invdx=1.0f/deltax;	
		for(int i=0; i<deltax; ++i)
		{
			pw+=elem;
			d+=deltay;
			if(d>deltax) {
				d-=deltax;
				pw+=pitch;	
			}
			at=uint8_t(a*(d*invdx));
			SET_ALPHA_CHANNEL(c,a-at);
			m_plotter(this,pw,&c);
			SET_ALPHA_CHANNEL(c,at);
			m_plotter(this,pw+pitch,&c);
		}
	}
	else
	{
		float invdy=1.0f/deltay;	
		for(int i=0; i<deltay; ++i)
		{
			pw+=pitch;
			d+=deltax;
			if(d>deltay) {
				d-=deltay;
				pw+=elem;	
			}
			at=uint8_t(a*(d*invdy));
			SET_ALPHA_CHANNEL(c,a-at);
			m_plotter(this,pw,&c);
			SET_ALPHA_CHANNEL(c,at);
			m_plotter(this,pw+elem,&c);
		}
	}
}

#else

void xraster::line_bresenham(int x0, int y0, int x1, int y1)
{
	if(x0>x1) {
		swap(x0,x1);
		swap(y0,y1);
	}
	int deltax=x1-x0;
	int deltay=y1-y0;
	int offy=1;
	if(deltay<0)
	{
		deltay=-deltay;
		offy=-1;
	}
	plot(x0,y0);
	if(deltax>=deltay)
	{
		int m=deltay<<1;
		int d=-deltax;
		for(int i=0;i<deltax;++i)
		{
			++x0;
			d+=m;
			if(d>=0)
			{
				y0+=offy;
				d-=deltax<<1;
			}
			plot(x0,y0);
		}
	}
	else
	{
		int m=deltax<<1;
		int d=-deltay;
		for(int i=0;i<deltay;++i)
		{
			y0+=offy;
			d+=m;
			if(d>=0)
			{
				++x0;
				d-=deltay<<1;
			}
			plot(x0,y0);
		}
	}
}

void xraster::line_bresenham_anti(int x0, int y0, int x1, int y1)
{
	if(x0>x1) {
		swap(x0,x1);
		swap(y0,y1);
	}
	int deltax=x1-x0;
	int deltay=y1-y0;
	int offy=1;
	if(deltay<0)
	{
		deltay=-deltay;
		offy=-1;
	}
	float d=0;
	pixel_t c=m_color;
	uint8_t a=ALPHA_CHANNEL(m_color);
	plot(x0,y0);
	if(deltay<deltax)
	{
		float k=float(deltay)/deltax;
		for(x0+=1; x0<=x1; ++x0)
		{
			d+=k;
			if(d>1) {
				d-=1;
				y0+=offy;	
			}
			TRANSPARENT(c,a,1-d);
		//	c=transparency(1-d);
			plot(x0,y0,c);
			TRANSPARENT(c,a,d);
		//	c=transparency(d);
			plot_s(x0,y0+offy,c);
		}
	}
	else
	{
		float k=float(deltax)/deltay;
		for(y0+=offy, y1+=offy; y0!=y1; y0+=offy)
		{
			d+=k;
			if(d>1) {
				d-=1;
				x0+=1;	
			}
			TRANSPARENT(c,a,1-d);
		//	c=transparency(1-d);
			plot(x0,y0,c);
			TRANSPARENT(c,a,d);
		//	c=transparency(d);
			plot_s(x0+1,y0,c);
		}
	}
}
#endif

void xraster::line_bresenham_pattern(int x0, int y0, int x1, int y1)
{
	if(x0>x1) {
		swap(x0,x1);
		swap(y0,y1);
	}
	int deltax=x1-x0;
	int deltay=y1-y0;
	int offy=1;
	if(deltay<0)
	{
		deltay=-deltay;
		offy=-1;
	}
	float d=0;
	make_pattern();
	plot_pattern(x0,y0);
	if(deltay<deltax)
	{
		float k=float(deltay)/deltax;
		for(x0+=1; x0<=x1; ++x0)
		{
			d+=k;
			if(d>1) {
				d-=1;
				y0+=offy;	
			}
			make_pattern(1-d);
			plot_pattern(x0,y0);
			make_pattern(d);
			plot_pattern(x0,y0+offy);
		}
	}
	else
	{
		float k=float(deltax)/deltay;
		for(y0+=offy, y1+=offy; y0!=y1; y0+=offy)
		{
			d+=k;
			if(d>1) {
				d-=1;
				x0+=1;	
			}
			make_pattern(1-d);
			plot_pattern(x0,y0);
			make_pattern(d);
			plot_pattern(x0+1,y0);
		}
	}
}

void xraster::line_bresenham_fat(int x0, int y0, int x1, int y1, int w)
{
	if(x0>x1) {
		swap(x0,x1);
		swap(y0,y1);
	}
	int deltax=x1-x0;
	int deltay=y1-y0;
	int offy=1;
	if(deltay<0)
	{
		deltay=-deltay;
		offy=-1;
	}
	int up=w>>1;
	int low=w-up-1;
	if(deltax>=deltay)
	{
		int begy, endy;
		if(x0>=m_xmin && x0<m_xmax) {
			begy=max(y0-up,m_ymin);
			endy=min(y0+low,m_ymax-1);
			for(int i=begy; i<=endy; ++i) {
				plot(x0,i);
			}
		}
		int m=deltay<<1;
		int d=-deltax;
		for(int i=0;i<deltax;++i)
		{
			++x0;
			d+=m;
			if(d>=0)
			{
				y0+=offy;
				d-=deltax<<1;
			}
			if(x0>=m_xmin && x0<m_xmax) {
				begy=max(y0-up,m_ymin);
				endy=min(y0+low,m_ymax-1);
				for(int i=begy; i<=endy; ++i) {
					plot(x0,i);
				}
			}
		}
	}
	else
	{
		int begx, endx;
		if(y0>=m_ymin && y0<m_ymax) {
			begx=max(x0-up,m_xmin);
			endx=min(x0+low,m_xmax-1);
			for(int i=begx; i<=endx; ++i) {
				plot(i,y0);
			}
		}
		int m=deltax<<1;
		int d=-deltay;
		for(int i=0;i<deltay;++i)
		{
			y0+=offy;
			d+=m;
			if(d>=0)
			{
				++x0;
				d-=deltay<<1;
			}
			if(y0>=m_ymin && y0<m_ymax) {
				begx=max(x0-up,m_xmin);
				endx=min(x0+low,m_xmax-1);
				for(int i=begx; i<=endx; ++i) {
					plot(i,y0);
				}
			}
		}
	}
}

// XialonWu直线光栅化算法
void xraster::line_xw(int x0, int y0, int x1, int y1)
{
	if(x0>x1) {
		swap(x0,x1);
		swap(y0,y1);
	}
	int deltax=x1-x0;
	int deltay=y1-y0;
	int offy=1;
	if(deltay<0)
	{
		deltay=-deltay;
		offy=-1;
	}
	plot(x0,y0);
	if(deltax>deltay)
	{
		int times=(deltax-1)>>1;
		bool bLeft=!(deltax&0x01);
		int d=(deltay<<2)-deltax;
		int inc0=(deltay<<2)-(deltax<<2);
		int inc1=(deltay<<2)-(deltax<<1);
		int inc2=(deltay<<2);
		deltax<<=1;
		deltay<<=1;
		for(int i=0; i<times; ++i)
		{
			if(d>deltax)
			{
				y0+=offy; plot(++x0,y0);
				y0+=offy; plot(++x0,y0);
				d+=inc0;
			}
			else if(d>deltay)
			{
				y0+=offy; 
				plot(++x0,y0);
				plot(++x0,y0);
				d+=inc1;
			}
			else if(d>0)
			{
				plot(++x0,y0);
				y0+=offy; 
				plot(++x0,y0);
				d+=inc1;
			}
			else
			{
				plot(++x0,y0);
				plot(++x0,y0);
				d+=inc2;
			}
		}
		if(bLeft)
		{
			if(d>deltay) plot(++x0,y0+offy);
			else plot(++x0,y0);
		}
	}
	else
	{
		int times=(deltay-1)>>1;
		bool bLeft=!(deltay&0x01);
		int d=(deltax<<2)-deltay;
		int inc0=(deltax<<2)-(deltay<<2);
		int inc1=(deltax<<2)-(deltay<<1);
		int inc2=(deltax<<2);
		deltax<<=1;
		deltay<<=1;
		for(int i=0; i<times; ++i)
		{
			if(d>deltay)
			{
				y0+=offy; plot(++x0,y0);
				y0+=offy; plot(++x0,y0);
				d+=inc0;
			}
			else if(d>deltax)
			{
				y0+=offy; plot(++x0,y0);
				y0+=offy; plot(x0,y0);
				d+=inc1;
			}
			else if(d>0)
			{
				y0+=offy; plot(x0,y0);
				y0+=offy; plot(++x0,y0);
				d+=inc1;
			}
			else
			{
				y0+=offy; plot(x0,y0);
				y0+=offy; plot(x0,y0);
				d+=inc2;
			}
		}
		if(bLeft)
		{
			if(d>deltax) plot(++x0,y0+offy);
			else plot(x0,y0+offy);
		}
	}
	plot(x1,y1);
}

// XialonWu对称直线光栅化算法
// todo: 尝试改为并行模式可能以获得更优秀的性能
void xraster::line_xw_sym(int x0, int y0, int x1, int y1)
{
	if(x0>x1) {
		swap(x0,x1);
		swap(y0,y1);
	}
	int deltax=x1-x0;
	int deltay=y1-y0;
	int offy=1;
	if(deltay<0)
	{
		deltay=-deltay;
		offy=-1;
	}
	plot(x0,y0);
	plot(x1,y1);
	if(deltax>deltay)
	{
		int times=(deltax-1)>>2;
		int left=(deltax-1)&0x03;
		int d=(deltay<<2)-deltax;
		int inc0=(deltay<<2)-(deltax<<2);
		int inc1=(deltay<<2)-(deltax<<1);
		int inc2=(deltay<<2);
		deltax<<=1;
		deltay<<=1;
		for(int i=0; i<times; ++i)
		{
			if(d>deltax)
			{
				y0+=offy; plot(++x0,y0);
				y0+=offy; plot(++x0,y0);
				y1-=offy; plot(--x1,y1);
				y1-=offy; plot(--x1,y1);
				d+=inc0;
			}
			else if(d>deltay)
			{
				y0+=offy; 
				plot(++x0,y0);
				plot(++x0,y0);
				y1-=offy; 
				plot(--x1,y1);
				plot(--x1,y1);
				d+=inc1;
			}
			else if(d>0)
			{
				plot(++x0,y0);
				y0+=offy; 
				plot(++x0,y0);
				plot(--x1,y1);
				y1-=offy; 
				plot(--x1,y1);
				d+=inc1;
			}
			else
			{
				plot(++x0,y0);
				plot(++x0,y0);
				plot(--x1,y1);
				plot(--x1,y1);
				d+=inc2;
			}
		}
		// for the left points
		if(left)
		{
			if(d>deltax)
			{
				plot(++x0,y0+=offy);
				if(left>1) plot(++x0,y0+=offy);
				if(left>2) plot(--x1,y1-=offy);
			}
			else if(d>deltay)
			{
				plot(++x0,y0+=offy);
				if(left>1) plot(++x0,y0);
				if(left>2) plot(--x1,y1-=offy);
			}
			else if(d>0)
			{
				plot(++x0,y0);
				if(left>1) plot(++x0,y0+=offy);
				if(left>2) plot(--x1,y1);
			}
			else
			{
				plot(++x0,y0);
				if(left>1) plot(++x0,y0);
				if(left>2) plot(--x1,y1);
			}
		}
	}
	else
	{
		int times=(deltay-1)>>2;
		int left=(deltay-1)&0x03;
		int d=(deltax<<2)-deltay;
		int inc0=(deltax<<2)-(deltay<<2);
		int inc1=(deltax<<2)-(deltay<<1);
		int inc2=(deltax<<2);
		deltax<<=1;
		deltay<<=1;
		for(int i=0; i<times; ++i)
		{
			if(d>deltay)
			{
				y0+=offy; plot(++x0,y0);
				y0+=offy; plot(++x0,y0);
				y1-=offy; plot(--x1,y1);
				y1-=offy; plot(--x1,y1);
				d+=inc0;
			}
			else if(d>deltax)
			{
				y0+=offy; plot(++x0,y0);
				y0+=offy; plot(x0,y0);
				y1-=offy; plot(--x1,y1);
				y1-=offy; plot(x1,y1);
				d+=inc1;
			}
			else if(d>0)
			{
				y0+=offy; plot(x0,y0);
				y0+=offy; plot(++x0,y0);
				y1-=offy; plot(x1,y1);
				y1-=offy; plot(--x1,y1);
				d+=inc1;
			}
			else
			{
				y0+=offy; plot(x0,y0);
				y0+=offy; plot(x0,y0);
				y1-=offy; plot(x1,y1);
				y1-=offy; plot(x1,y1);
				d+=inc2;
			}
		}
		// for the left points
		if(left)
		{
			if(d>deltay)
			{
				plot(++x0,y0+=offy);
				if(left>1) plot(++x0,y0+=offy);
				if(left>2) plot(--x1,y1-=offy);
			}
			else if(d>deltax)
			{
				plot(++x0,y0+=offy);
				if(left>1) plot(x0,y0+=offy);
				if(left>2) plot(--x1,y1-=offy);
			}
			else if(d>0)
			{
				plot(x0,y0+=offy);
				if(left>1) plot(++x0,y0+=offy);
				if(left>2) plot(x1,y1-=offy);
			}
			else
			{
				plot(x0,y0+=offy);
				if(left>1) plot(x0,y0+=offy);
				if(left>2) plot(x1,y1-=offy);
			}
		}
	}
}

void xraster::circle_outline(int cx, int cy, int radius)
{
	int x=0, y=radius;
	int d=1-radius;
	plot_s(cx,cy+y);
	plot_s(cx,cy-y);
	plot_s(cx+y,cy);
	plot_s(cx-y,cy);
	while(x<y)
	{
		++x;
		if(d<0)
		{
			d+=(x<<1)+3;
		}
		else
		{
			y-=1;
			d+=((x-y)<<1)+5;
		}
		plot_octant(cx,cy,x,y);
	}
}

void xraster::circle_outline_anti(int cx, int cy, int radius)
{
	int x=0, y=radius, r2=radius*radius;
	int ix=fast_floor(0.7071f*radius);
	float d=0;
	pixel_t c=m_color;
	uint8_t a=ALPHA_CHANNEL(m_color);
	plot_s(cx,cy+y);
	plot_s(cx,cy-y);
	plot_s(cx+y,cy);
	plot_s(cx-y,cy);
	while(x<ix)
	{
		++x;
		d=y-sqrt(float(r2-x*x));
		if(d>1)
		{
			--y;
			--d;
		}
		TRANSPARENT(c,a,1-d);
		plot_octant(cx,cy,x,y,c);
		TRANSPARENT(c,a,d);
		plot_octant(cx,cy,x,y-1,c);
	}
	--y;
	d=sqrt(float(r2-y*y))-x;
	TRANSPARENT(c,a,1-d);
	plot_quadrant(cx,cy,x,y,c);
	TRANSPARENT(c,a,d);
	plot_quadrant(cx,cy,x+1,y,c);
}

void xraster::circle_outline_pattern(int cx, int cy, int radius)
{
	int x=0, y=radius, r2=radius*radius;
	int ix=fast_floor(0.7071f*radius);
	float d=0;
	make_pattern();
	plot_pattern(cx,cy+y);
	plot_pattern(cx,cy-y);
	plot_pattern(cx+y,cy);
	plot_pattern(cx-y,cy);
	while(x<ix)
	{
		++x;
		d=y-sqrt(float(r2-x*x));
		if(d>1)
		{
			--y;
			--d;
		}
		make_pattern(1-d);
		plot_octant_pattern(cx,cy,x,y);
		make_pattern(d);
		plot_octant_pattern(cx,cy,x,y-1);
	}
	--y;
	d=sqrt(float(r2-y*y))-x;
	make_pattern(1-d);
	plot_quadrant_pattern(cx,cy,x,y);
	make_pattern(d);
	plot_quadrant_pattern(cx,cy,x+1,y);
}

void xraster::fill_circle_flat(int cx, int cy, int radius)
{
	int x=0, y=radius;
	int begx, endx;
	int d=1-radius;
	plot_s(cx,cy+y);
	plot_s(cx,cy-y);
	scan_line_s(cy,cx-y,cx+y);
	while(x<y)
	{
		++x;
		if(d<0)
		{
			d+=(x<<1)+3;
			plot_quadrant(cx,cy,x,y);
		}
		else
		{
			y-=1;
			d+=((x-y)<<1)+5;
			begx=cx-x;
			endx=cx+x;
			scan_line_s(cy+y,begx,endx);
			scan_line_s(cy-y,begx,endx);
		}
		begx=cx-y;
		endx=cx+y;
		scan_line_s(cy+x,begx,endx);
		scan_line_s(cy-x,begx,endx);
	}
}

void xraster::fill_circle_flat_anti(int cx, int cy, int radius)
{
	int x=0, y=radius, r2=radius*radius;
	int ix=fast_floor(0.7071f*radius);
	int begx, endx;
	float d=0;
	pixel_t c=m_color;
	uint8_t a=ALPHA_CHANNEL(m_color);
	plot_s(cx,cy+y);
	plot_s(cx,cy-y);
	scan_line_s(cy,cx-y,cx+y);
	while(x<ix)
	{
		++x;
		d=y-sqrt(float(r2-x*x));
		if(d>1)
		{
			--y;
			--d;
			// fill inner
			begx=cx-x+1;
			endx=cx+x-1;
			scan_line_s(cy+y,begx,endx);
			scan_line_s(cy-y,begx,endx);
		}
		// fill inner
		begx=cx-y+1;
		endx=cx+y-1;
		scan_line_s(cy+x,begx,endx);
		scan_line_s(cy-x,begx,endx);
		// outline
		TRANSPARENT(c,a,1-d);
	//	c=transparency(1-d);
		plot_octant(cx,cy,x,y,c);
		TRANSPARENT(c,a,d);
	//	c=transparency(d);
		plot_octant(cx,cy,x,y-1,c);
	}
	--y;
	d=sqrt(float(r2-y*y))-x;
	// fill inner
	begx=cx-x;
	endx=cx+x;
	scan_line_s(cy+y,begx,endx);
	scan_line_s(cy-y,begx,endx);
	TRANSPARENT(c,a,1-d);
//	c=transparency(1-d);
	plot_quadrant(cx,cy,x,y,c);
	TRANSPARENT(c,a,d);
//	c=transparency(d);
	plot_quadrant(cx,cy,x+1,y,c);
}

// 中点椭圆算法,在大约rx+ry>850左右第二部分会出现收敛错误,可能是由于误差累积造成?
void xraster::ellipse_outline(int cx, int cy, int rx, int ry)
{
	int x=0, y=ry;
	int rx2 = rx*rx;
	int ry2 = ry*ry;
	int tx = rx2<<1;
	int ty = ry2<<1;
	int kx = 0;
	int ky = tx*y;
	int d = int(ry2-rx2*ry+0.25*rx2);
	plot_quadrant(cx,cy,x,y);
	while(kx<ky)
	{
		++x;
		kx += ty;
		if(d<0)
		{
			d += kx+ry2;
		}
		else
		{
			--y;
			ky -= tx;
			d += kx-ky+ry2;
		}
		plot_quadrant(cx,cy,x,y);
	}
	// 下面部分有BUG
	d = int(ry2*(x*x+x+0.25)+rx2*(y*y-(y<<1)+1)-rx2*ry2);
	while(y>0)
	{
		--y;
		ky -= tx;
		if(d>0)
		{
			d += -ky+rx2;
		}
		else
		{
			++x;
			kx += ty;
			d += -ky+kx+rx2;
		}
		plot_quadrant(cx,cy,x,y);
	}
}

void xraster::ellipse_outline_anti(int cx, int cy, int rx, int ry)
{
	int x=0, y=ry;
	int rx2=rx*rx;
	int ry2=ry*ry;
	float ryrx=float(ry2)/rx2;
	float rxry=float(rx2)/ry2;
	int ix=fast_floor(rx2/sqrt(float(rx2+ry2)));
	float d=0;
	pixel_t c=m_color;
	uint8_t a=ALPHA_CHANNEL(m_color);
	plot_quadrant(cx,cy,x,y);
	while(x<ix)
	{
		++x;
		// ry2-ryrx*x*x有可能溢出而成为负数,改为double可解决
		d=y-sqrt(max(ry2-ryrx*x*x,0.0f));
		if(d>=1) {
			--y;
			--d;
		}
		c=transparency(1-d);
		plot_quadrant(cx,cy,x,y,c);
		c=transparency(d);
		plot_quadrant(cx,cy,x,y-1,c);
	}
	while(y>0) {
		--y;
		d=sqrt(rx2-rxry*y*y)-x;
		if(d>=1) {
			++x;
			--d;
		}
		TRANSPARENT(c,a,1-d);
	//	c=transparency(1-d);
		plot_quadrant(cx,cy,x,y,c);
		TRANSPARENT(c,a,d);
	//	c=transparency(d);
		plot_quadrant(cx,cy,x+1,y,c);
	}
}

void xraster::ellipse_outline_pattern(int cx, int cy, int rx, int ry)
{
	int x=0, y=ry;
	int rx2=rx*rx;
	int ry2=ry*ry;
	float ryrx=float(ry2)/rx2;
	float rxry=float(rx2)/ry2;
	int ix=fast_floor(rx2/sqrt(float(rx2+ry2)));
	float d=0;
	make_pattern();
	plot_quadrant_pattern(cx,cy,x,y);
	while(x<ix)
	{
		++x;
		// ry2-ryrx*x*x有可能溢出而成为负数,改为double可解决
		d=y-sqrt(max(ry2-ryrx*x*x,0.0f));
		if(d>=1) {
			--y;
			--d;
		}
		make_pattern(1-d);
		plot_quadrant_pattern(cx,cy,x,y);
		make_pattern(d);
		plot_quadrant_pattern(cx,cy,x,y-1);
	}
	while(y>0) {
		--y;
		d=sqrt(rx2-rxry*y*y)-x;
		if(d>=1) {
			++x;
			--d;
		}
		make_pattern(1-d);
		plot_quadrant_pattern(cx,cy,x,y);
		make_pattern(d);
		plot_quadrant_pattern(cx,cy,x+1,y);
	}
}

void xraster::fill_ellipse_flat(int cx, int cy, int rx, int ry)
{
	int x=0, y=ry;
	int rx2 = rx*rx;
	int ry2 = ry*ry;
	int tx = rx2<<1;
	int ty = ry2<<1;
	int kx = 0;
	int ky = tx*y;
	int d = int(ry2-rx2*ry+0.25*rx2);
	int begx, endx;
	plot_s(cx,cy+y);
	plot_s(cx,cy-y);
	while(kx<ky)
	{
		++x;
		kx += ty;
		if(d<0)
		{
			d += kx+ry2;
			plot_quadrant(cx,cy,x,y);
		}
		else
		{
			--y;
			ky -= tx;
			d += kx-ky+ry2;
			begx=cx-x;
			endx=cx+x;
			scan_line_s(cy+y,begx,endx);
			scan_line_s(cy-y,begx,endx);
		}
	}
	d = int(ry2*(x*x+x+0.25)+rx2*(y*y-(y<<1)+1)-rx2*ry2);
	while(y>0)
	{
		--y;
		ky -= tx;
		if(d>0)
		{
			d += -ky+rx2;
		}
		else
		{
			++x;
			kx += ty;
			d += -ky+kx+rx2;
		}
		begx=cx-x;
		endx=cx+x;
		scan_line_s(cy+y,begx,endx);
		scan_line_s(cy-y,begx,endx);
	}
}

void xraster::fill_ellipse_flat_anti(int cx, int cy, int rx, int ry)
{
	int x=0, y=ry;
	int rx2=rx*rx;
	int ry2=ry*ry;
	float ryrx=float(ry2)/rx2;
	float rxry=float(rx2)/ry2;
	int ix=fast_floor(rx2/sqrt(float(rx2+ry2)));
	float d=0;
	int begx, endx;
	pixel_t c=m_color;
	uint8_t a=ALPHA_CHANNEL(m_color);
	plot_s(cx,cy+y);
	plot_s(cx,cy-y);
	while(x<ix)
	{
		++x;
		d=y-sqrt(max(ry2-ryrx*x*x,0.0f));
		if(d>=1) {
			--y;
			--d;
			begx=cx-x+1;
			endx=cx+x-1;
			scan_line_s(cy+y,begx,endx);
			scan_line_s(cy-y,begx,endx);
		}
		TRANSPARENT(c,a,1-d);
	//	c=transparency(1-d);
		plot_quadrant(cx,cy,x,y,c);
		TRANSPARENT(c,a,d);
	//	c=transparency(d);
		plot_quadrant(cx,cy,x,y-1,c);
	}
	while(y>0) {
		--y;
		d=sqrt(rx2-rxry*y*y)-x;
		if(d>=1) {
			++x;
			--d;
		}
		begx=cx-x;
		endx=cx+x;
		scan_line_s(cy+y,begx,endx);
		scan_line_s(cy-y,begx,endx);
		TRANSPARENT(c,a,1-d);
	//	c=transparency(1-d);
		plot_quadrant(cx,cy,x,y,c);
		TRANSPARENT(c,a,d);
	//	c=transparency(d);
		plot_quadrant(cx,cy,x+1,y,c);
	}
}

// Cohen-Sutherland直线裁剪算法
// 用矩形区域裁剪直线,并将裁剪后的新端点保存在(x0,y0)和(x1,y1)中
// 如果线段被完全裁剪,返回false,否则返回true
bool xraster::line_clip_2d_cs(int &x0, int &y0, int &x1, int &y1, const xrecti_t &rect)
{
#define CLIPLINE_TOP_LEFT	0x0A
#define CLIPLINE_TOP_CENTER	0x08
#define CLIPLINE_TOP_RIGHT	0x09
#define CLIPLINE_MID_LEFT	0x02
#define CLIPLINE_MID_CENTER	0x00
#define CLIPLINE_MID_RIGHT	0x01
#define CLIPLINE_BOT_LEFT	0x06
#define CLIPLINE_BOT_CENTER	0x04
#define CLIPLINE_BOT_RIGHT	0x05
	
	int nFlag0=0, nFlag1=0;
	if(x0<rect.left)
		nFlag0 |= CLIPLINE_MID_LEFT;
	else if(x0>rect.right)
		nFlag0 |= CLIPLINE_MID_RIGHT;
	if(y0<rect.top)
		nFlag0 |= CLIPLINE_TOP_CENTER;
	else if(y0>rect.bottom)
		nFlag0 |= CLIPLINE_BOT_CENTER;
	if(x1<rect.left)
		nFlag1 |= CLIPLINE_MID_LEFT;
	else if(x1>rect.right)
		nFlag1 |= CLIPLINE_MID_RIGHT;
	if(y1<rect.top)
		nFlag1 |= CLIPLINE_TOP_CENTER;
	else if(y1>rect.bottom)
		nFlag1 |= CLIPLINE_BOT_CENTER;

	if(nFlag0==0 && nFlag1==0)	// 两端点均在裁剪区域内
		return true;
	else if(nFlag0 & nFlag1)	// 两端点均在裁剪区域的同一侧
		return false;
	
	// 计算与边界的交点
	float x=float(x0), y=float(y0);
	switch(nFlag0)
	{
	case CLIPLINE_MID_CENTER:
		break;
	case CLIPLINE_TOP_CENTER:
		y=float(rect.top);
		x=x0+(x1-x0)*(y-y0)/(y1-y0);
		break;
	case CLIPLINE_BOT_CENTER:
		y=float(rect.bottom);
		x=x0+(x1-x0)*(y-y0)/(y1-y0);
		break;
	case CLIPLINE_MID_LEFT:
		x=float(rect.left);
		y=y0+(y1-y0)*(x-x0)/(x1-x0);
		break;
	case CLIPLINE_MID_RIGHT:
		x=float(rect.right);
		y=y0+(y1-y0)*(x-x0)/(x1-x0);
		break;
	case CLIPLINE_TOP_LEFT:
		x=float(rect.left);
		y=y0+(y1-y0)*(x-x0)/(x1-x0);
		if(y<rect.top || y>rect.bottom)
		{
			y=float(rect.top);
			x=x0+(x1-x0)*(y-y0)/(y1-y0);
		}
		break;
	case CLIPLINE_TOP_RIGHT:
		x=float(rect.right);
		y=y0+(y1-y0)*(x-x0)/(x1-x0);
		if(y<rect.top || y>rect.bottom)
		{
			y=float(rect.top);
			x=x0+(x1-x0)*(y-y0)/(y1-y0);
		}
		break;
	case CLIPLINE_BOT_LEFT:
		x=float(rect.left);
		y=y0+(y1-y0)*(x-x0)/(x1-x0);
		if(y<rect.top || y>rect.bottom)
		{
			y=float(rect.bottom);
			x=x0+(x1-x0)*(y-y0)/(y1-y0);
		}
		break;
	case CLIPLINE_BOT_RIGHT:
		x=float(rect.right);
		y=y0+(y1-y0)*(x-x0)/(x1-x0);
		if(y<rect.top || y>rect.bottom)
		{
			y=float(rect.bottom);
			x=x0+(x1-x0)*(y-y0)/(y1-y0);
		}
		break;
	}
	if(x<rect.left || x>rect.right || y<rect.top || y>rect.bottom)
		nFlag0=0;
	else
	{
		x0=fast_round(x);
		y0=fast_round(y);
	}
	x=float(x1); y=float(y1);
	switch(nFlag1)
	{
	case CLIPLINE_MID_CENTER:
		break;
	case CLIPLINE_TOP_CENTER:
		y=float(rect.top);
		x=x1+(x1-x0)*(y-y1)/(y1-y0);
		break;
	case CLIPLINE_BOT_CENTER:
		y=float(rect.bottom);
		x=x1+(x1-x0)*(y-y1)/(y1-y0);
		break;
	case CLIPLINE_MID_LEFT:
		x=float(rect.left);
		y=y1+(y1-y0)*(x-x1)/(x1-x0);
		break;
	case CLIPLINE_MID_RIGHT:
		x=float(rect.right);
		y=y1+(y1-y0)*(x-x1)/(x1-x0);
		break;
	case CLIPLINE_TOP_LEFT:
		x=float(rect.left);
		y=y1+(y1-y0)*(x-x1)/(x1-x0);
		if(y<rect.top || y>rect.bottom)
		{
			y=float(rect.top);
			x=x1+(x1-x0)*(y-y1)/(y1-y0);
		}
		break;
	case CLIPLINE_TOP_RIGHT:
		x=float(rect.right);
		y=y1+(y1-y0)*(x-x1)/(x1-x0);
		if(y<rect.top || y>rect.bottom)
		{
			y=float(rect.top);
			x=x1+(x1-x0)*(y-y1)/(y1-y0);
		}
		break;
	case CLIPLINE_BOT_LEFT:
		x=float(rect.left);
		y=y1+(y1-y0)*(x-x1)/(x1-x0);
		if(y<rect.top || y>rect.bottom)
		{
			y=float(rect.bottom);
			x=x1+(x1-x0)*(y-y1)/(y1-y0);
		}
		break;
	case CLIPLINE_BOT_RIGHT:
		x=float(rect.right);
		y=y1+(y1-y0)*(x-x1)/(x1-x0);
		if(y<rect.top || y>rect.bottom)
		{
			y=float(rect.bottom);
			x=x1+(x1-x0)*(y-y1)/(y1-y0);
		}
		break;
	}
	if(x<rect.left || x>rect.right || y<rect.top || y>rect.bottom)
		nFlag1=0;
	else
	{
		x1=fast_round(x);
		y1=fast_round(y);
	}
	if(!nFlag0 && !nFlag1) return false;
	return true;
}

// 梁友栋-Barsky直线裁剪算法
// 对于过大的坐标值(>10^9),会因为浮点溢出而出错,可用double代替float
bool xraster::line_clip_2d_lb(int &x0, int &y0, int &x1, int &y1, const xrecti_t &rect)
{
	float r0, r1;
	float t0, t1;
	bool bx=false, by=false; // 是否平行于X或Y坐标轴
	int dx=x1-x0, dy=y1-y0;
	if(dx==0)	// 平行于左右边界
	{
		if(x0<rect.left || x0>=rect.right) return false;
		bx=true;
	}
	else
	{
		r0=float(rect.left-x0)/dx;
		r1=float(rect.right-x0)/dx;
		if(dx<0)
		{
			t1=r0;
			t0=r1;
		}
		else // dx>0 
		{
			t0=r0;
			t1=r1;
		}
	}
	if(dy==0)	// 平行于上下边界
	{
		if(y0<rect.top || y0>=rect.bottom) return false;
		by=true;
	}
	else
	{
		r0=float(rect.bottom-y0)/dy;
		r1=float(rect.top-y0)/dy;
		if(dy<0)
		{
			if(bx)
			{
				t0=r0;
				t1=r1;
			}
			else
			{
				t0=MAX(t0,r0);
				t1=MIN(t1,r1);
			}
		}
		else // dy>0
		{
			if(bx)
			{
				t0=r1;
				t1=r0;
			}
			else
			{
				t0=MAX(t0,r1);
				t1=MIN(t1,r0);
			}
		}
	}
	// 单个点
	if(bx && by) return false;
	// 计算进入点和离开点的参数
	t0=MAX(0,t0);
	t1=MIN(1,t1);
	// 位于裁剪区域之外
	if(t0>t1) return false;
	// 计算新的端点
	if(!bx)
	{
		int begx=x0;
		x0=fast_round(begx+dx*t0);
		x1=fast_round(begx+dx*t1);
	}
	if(!by)
	{
		int begy=y0;
		y0=fast_round(begy+dy*t0);
		y1=fast_round(begy+dy*t1);
	}
	return true;
}

// 2D贝赛尔曲线
void xraster::curve_bezier(int nPoints, const xpt2i_t &beg, const xpt2i_t &ctrl1, const xpt2i_t &ctrl2, const xpt2i_t &end)
{
	if(nPoints<=0) return;
	float t=1.0f/nPoints;
	float t2=t*t;
	float t3=t2*t;
	float fx=float(beg.x);
	float fy=float(beg.y);
	float dfx=3*(ctrl1.x-beg.x)*t;
	float dfy=3*(ctrl1.y-beg.y)*t;
	float ddfx=6*(beg.x-ctrl1.x*2+ctrl2.x)*t2;
	float ddfy=6*(beg.y-ctrl1.y*2+ctrl2.y)*t2;
	float dddfx=6*(-beg.x+3*(ctrl1.x-ctrl2.x)+end.x)*t3;
	float dddfy=6*(-beg.y+3*(ctrl1.y-ctrl2.y)+end.y)*t3;
	plot_s(beg.x,beg.y);
	for(int i=0; i<nPoints; ++i)
	{
		fx+=dfx;
		fy+=dfy;
		dfx+=ddfx;
		dfy+=ddfy;
		ddfx+=dddfx;
		ddfy+=dddfy;
		plot_s(fast_round(fx),fast_round(fy));
	}
}

// 抛物线
void xraster::curve_parabola(int nPoints, float hs, float vs, float dt)
{
	if(nPoints<=0) return;
	float x=0, y=0;
	float dx=hs*dt;
	float dy=vs*dt, ddy=-9.8f*dt*dt;
	plot_s(x,y);
	for(int i=0; i<nPoints; ++i) 
	{
		x+=dx;
		y+=dy;
		dy+=ddy;
		plot_s(x,y);
	}
}

void xraster::fill_rect(int x, int y, int w, int h)
{
	if(w<1 || h<1) return;
	int endx=min(m_xmax-1,x+w-1), endy=min(m_ymax,y+h);
	if(x<m_xmin) x=m_xmin;
	if(y<m_ymin) y=m_ymin;
	while(y<endy) {
		scan_line(y,x,endx);
		++y;
	}
}

void xraster::fill_round_rect(int x, int y, int w, int h, int r)
{
	int rmax=min(w,h)>>1;
	if(rmax<1 || r<1) {
		fill_rect(x,y,w,h);
		return;	
	}
	if(r>rmax) r=rmax;
	if(w<1 || h<1) return;
	int x2=x+w-1, y2=y+h-1;
	if(x>=m_xmax || x2<m_xmin) return;
	if(y>=m_ymax || y2<m_ymin) return;
	int begx=max(m_xmin,x+r), begy=max(m_ymin,y+r);
	int endx=min(m_xmax-1,x2-r), endy=min(m_ymax-1,y2-r);
	// 边
	if(y>=m_ymin) 
		scan_line(y,begx,endx);
	if(y2<m_ymax)
		scan_line(y2,begx,endx);
	if(x<m_xmin) x=m_xmin;
	if(x2>=m_xmax) x2=m_xmax-1;
	for(int i=begy; i<=endy; ++i)
		scan_line(i,x,x2);
	// 圆弧
	if(is_anti())
		fill_circle_part_anti(begx,begy,endx,endy,r);
	else fill_circle_part(begx,begy,endx,endy,r);
}

void xraster::circle_part(int cx0, int cy0, int cx1, int cy1, int radius)
{
	int x=0, y=radius;
	int d=1-radius;
		plot_s(cx1+x,cy0-y);
		plot_s(cx1+y,cy0-x);
		plot_s(cx0-x,cy0-y);
		plot_s(cx0-y,cy0-x);
		plot_s(cx0-x,cy1+y);
		plot_s(cx0-y,cy1+x);
		plot_s(cx1+x,cy1+y);
		plot_s(cx1+y,cy1+x);
	while(x<y)
	{
		++x;
		if(d<0)
		{
			d+=(x<<1)+3;
		}
		else
		{
			y-=1;
			d+=((x-y)<<1)+5;
		}
			plot_s(cx1+x,cy0-y);
			plot_s(cx1+y,cy0-x);
			plot_s(cx0-x,cy0-y);
			plot_s(cx0-y,cy0-x);
			plot_s(cx0-x,cy1+y);
			plot_s(cx0-y,cy1+x);
			plot_s(cx1+x,cy1+y);
			plot_s(cx1+y,cy1+x);
	}
}

void xraster::circle_part_anti(int cx0, int cy0, int cx1, int cy1, int radius)
{
	int x=0, y=radius, y2, r2=radius*radius;
	float d=0;
	pixel_t c=m_color;
	uint8_t a=ALPHA_CHANNEL(m_color);
		plot_s(cx1+x,cy0-y);
		plot_s(cx1+y,cy0-x);
		plot_s(cx0-x,cy0-y);
		plot_s(cx0-y,cy0-x);
		plot_s(cx0-x,cy1+y);
		plot_s(cx0-y,cy1+x);
		plot_s(cx1+x,cy1+y);
		plot_s(cx1+y,cy1+x);
	while(x<y)
	{
		++x;
		d=y-sqrt(float(r2-x*x));
		if(d>1)
		{
			--y;
			--d;
			if(d>1) d=1;
		}
		TRANSPARENT(c,a,1-d);
			plot_s(cx1+x,cy0-y,c);
			plot_s(cx1+y,cy0-x,c);
			plot_s(cx0-x,cy0-y,c);
			plot_s(cx0-y,cy0-x,c);
			plot_s(cx0-x,cy1+y,c);
			plot_s(cx0-y,cy1+x,c);
			plot_s(cx1+x,cy1+y,c);
			plot_s(cx1+y,cy1+x,c);
		TRANSPARENT(c,a,d);
			y2=y-1;
			plot_s(cx1+x,cy0-y2,c);
			plot_s(cx1+y2,cy0-x,c);
			plot_s(cx0-x,cy0-y2,c);
			plot_s(cx0-y2,cy0-x,c);
			plot_s(cx0-x,cy1+y2,c);
			plot_s(cx0-y2,cy1+x,c);
			plot_s(cx1+x,cy1+y2,c);
			plot_s(cx1+y2,cy1+x,c);
	}
}

void xraster::circle_part_pattern(int cx0, int cy0, int cx1, int cy1, int radius)
{
	int x=0, y=radius, y2, r2=radius*radius;
	float d=0;
		plot_pattern(cx1+x,cy0-y);
		plot_pattern(cx1+y,cy0-x);
		plot_pattern(cx0-x,cy0-y);
		plot_pattern(cx0-y,cy0-x);
		plot_pattern(cx0-x,cy1+y);
		plot_pattern(cx0-y,cy1+x);
		plot_pattern(cx1+x,cy1+y);
		plot_pattern(cx1+y,cy1+x);
	while(x<y)
	{
		++x;
		d=y-sqrt(float(r2-x*x));
		if(d>1)
		{
			--y;
			--d;
			if(d>1) d=1;
		}
		make_pattern(1-d);
			plot_pattern(cx1+x,cy0-y);
			plot_pattern(cx1+y,cy0-x);
			plot_pattern(cx0-x,cy0-y);
			plot_pattern(cx0-y,cy0-x);
			plot_pattern(cx0-x,cy1+y);
			plot_pattern(cx0-y,cy1+x);
			plot_pattern(cx1+x,cy1+y);
			plot_pattern(cx1+y,cy1+x);
		make_pattern(d);
			y2=y-1;
			plot_pattern(cx1+x,cy0-y2);
			plot_pattern(cx1+y2,cy0-x);
			plot_pattern(cx0-x,cy0-y2);
			plot_pattern(cx0-y2,cy0-x);
			plot_pattern(cx0-x,cy1+y2);
			plot_pattern(cx0-y2,cy1+x);
			plot_pattern(cx1+x,cy1+y2);
			plot_pattern(cx1+y2,cy1+x);
	}
}

void xraster::fill_circle_part(int cx0, int cy0, int cx1, int cy1, int radius)
{
	int x=0, y=radius;
	int d=1-radius;
		plot_s(cx1+x,cy0-y);
		plot_s(cx0-x,cy0-y);
		plot_s(cx0-x,cy1+y);
		plot_s(cx1+x,cy1+y);
		scan_line_s(cy0-x,cx0-y,cx1+y);
		scan_line_s(cy1+x,cx0-y,cx1+y);
	while(x<y)
	{
		++x;
		if(d<0)
		{
			d+=(x<<1)+3;
		}
		else
		{
			y-=1;
			d+=((x-y)<<1)+5;
				scan_line_s(cy0-y,cx0-x,cx1+x);
				scan_line_s(cy1+y,cx0-x,cx1+x);
		}
			plot_s(cx1+x,cy0-y);
			plot_s(cx0-x,cy0-y);
			plot_s(cx0-x,cy1+y);
			plot_s(cx1+x,cy1+y);
			scan_line_s(cy0-x,cx0-y,cx1+y);
			scan_line_s(cy1+x,cx0-y,cx1+y);
	}
}

void xraster::fill_circle_part_anti(int cx0, int cy0, int cx1, int cy1, int radius)
{
	int x=0, y=radius, y2, r2=radius*radius;
	float d=0;
	pixel_t c=m_color;
	uint8_t a=ALPHA_CHANNEL(m_color);
		plot_s(cx1+x,cy0-y);
		plot_s(cx0-x,cy0-y);
		plot_s(cx0-x,cy1+y);
		plot_s(cx1+x,cy1+y);
		scan_line_s(cy0-x,cx0-y,cx1+y);
		scan_line_s(cy1+x,cx0-y,cx1+y);
	while(x<y)
	{
		++x;
		d=y-sqrt(float(r2-x*x));
		if(d>1)
		{
			--y;
			--d;
			if(d>1) d=1;
				scan_line_s(cy0-y,cx0-x+1,cx1+x-1);
				scan_line_s(cy1+y,cx0-x+1,cx1+x-1);
		}
			scan_line_s(cy0-x,cx0-y+1,cx1+y-1);
			scan_line_s(cy1+x,cx0-y+1,cx1+y-1);
		TRANSPARENT(c,a,1-d);
			plot_s(cx1+x,cy0-y,c);
			plot_s(cx1+y,cy0-x,c);
			plot_s(cx0-x,cy0-y,c);
			plot_s(cx0-y,cy0-x,c);
			plot_s(cx0-x,cy1+y,c);
			plot_s(cx0-y,cy1+x,c);
			plot_s(cx1+x,cy1+y,c);
			plot_s(cx1+y,cy1+x,c);
		TRANSPARENT(c,a,d);
			y2=y-1;
			plot_s(cx1+x,cy0-y2,c);
			plot_s(cx1+y2,cy0-x,c);
			plot_s(cx0-x,cy0-y2,c);
			plot_s(cx0-y2,cy0-x,c);
			plot_s(cx0-x,cy1+y2,c);
			plot_s(cx0-y2,cy1+x,c);
			plot_s(cx1+x,cy1+y2,c);
			plot_s(cx1+y2,cy1+x,c);
	}
}

