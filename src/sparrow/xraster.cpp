#include <cassert>
#include <algorithm>
#include "xutil.h"
#include "xraster.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////////////////////////////
// 预定义模板图元
//////////////////////////////////////////////////////////////////////////////////////////////////

static uint8_t XPATTERN_BRUSH_W3[]={ // 3x3
	35, 94, 35,
	94, 255,94,
	35, 94, 35
};

static uint8_t XPATTERN_BRUSH_W5[]={ // 5x5
	16, 45,  64,  45,  16,
	45, 128, 182, 128, 45,
	65, 182, 255, 182, 65,
	45, 128, 182, 128, 45,
	16, 45,  64,  45,  16
};

static uint8_t XPATTERN_BRUSH_W7[]={ // 7x7
	0,  9,   35,  54,  35,  9,   0,
	9,  80,  201, 239, 201, 80,  9,
	35, 202, 255, 255, 255, 202, 35,
	54, 239, 255, 255, 255, 239, 54,
	35, 202, 255, 255, 255, 202, 35,
	9,  80,  201, 239, 201, 80,  9,
	0,  9,   35,  54,  35,  9,   0
};

xpattern xraster::s_PreDefinedPattern[xraster::NUM_PATTERN]={
	{3,3,1,1,XPATTERN_BRUSH_W3},
	{5,5,2,2,XPATTERN_BRUSH_W5},
	{7,7,3,3,XPATTERN_BRUSH_W7}
};

//////////////////////////////////////////////////////////////////////////////////////////////////
// 颜色缓存
//////////////////////////////////////////////////////////////////////////////////////////////////

xcolor_buffer::xcolor_buffer() {
	m_spans=0;
}

xcolor_buffer::~xcolor_buffer()
{
	clear();
}

bool xcolor_buffer::create(unsigned w, unsigned h, XG_PIXEL_FORMAT fmt)
{
	unsigned elem=pixel_size(fmt);
	assert(is_power2(elem));
	m_elemPower=log2(elem);
	m_elemMask=elem-1;
	print("sizeelem=%d,power=%d,mask=%d",elem,m_elemPower,m_elemMask);
	unsigned oldh=m_height;
	if(!xbuffer::create(w,h,pixel_size(fmt),BOUNDARY_ALIGNMENT))
		return false;
	reset_span_map(oldh!=h);
	return true;
}

void xcolor_buffer::reset_span_map(bool resize)
{
	if(resize || m_spans==0) {
		if(m_spans) 
			delete [] m_spans;
		m_spans=new puint8_t[m_height];
	}
	uint8_t *pline=get_buffer();
	for(unsigned i=0; i<m_height; ++i) {
		m_spans[i]=pline;
		pline+=m_pitch;
	}
}

void xcolor_buffer::clear()
{
	if(m_spans) {
		delete [] m_spans;
		m_spans=0;
	}
	xbuffer::clear();
}

uint8_t* xcolor_buffer::release(unsigned *pitch, unsigned *width, unsigned *height)
{
	if(m_spans) {
		delete [] m_spans;
		m_spans=0;
	}
	return xbuffer::release(pitch,width,height);
}

void xcolor_buffer::deliver(xcolor_buffer &accept)
{
	xbuffer::deliver(accept);
	accept.m_elemPower=m_elemPower;
	accept.m_elemMask=m_elemMask;
	accept.m_spans=m_spans;
	m_spans=0;
}

bool xcolor_buffer::share(const xcolor_buffer &buffer, int x, int y, unsigned w, unsigned h, bool bsafe)
{
	unsigned oldh=m_height;
	if(!xbuffer::share(buffer,x,y,w,h,bsafe))
		return false;
	m_elemPower=buffer.m_elemPower;
	m_elemMask=buffer.m_elemMask;
	reset_span_map(oldh!=h);
	return true;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
// 软件光栅器
//////////////////////////////////////////////////////////////////////////////////////////////////

void xraster::general_format_plotter(void *pctx, void *pdst, void *psrc)
{
	pixel_t c;
	get_translater(((xraster*)pctx)->m_pixelfmt)(pctx,&c,psrc);
	get_plotter(((xraster*)pctx)->m_plotmode)(pctx,pdst,&c);
}

xraster::xraster() 
{
	m_pixelfmt=PIXEL_FMT_UNKNOWN;
	m_color=0xFFFFFFFF;
	m_bkcolor=0;
	m_colorkey=0;
	m_plotter=0;
	m_plotmode=PLOT_MODE_UNKNOWN;

	m_state=0;
	m_palette=0;
	m_colorNum=0;
	m_patternBuffer=0;
	m_pfiller=0;
}

xraster::~xraster() 
{
	destroy();
}

bool xraster::create(unsigned w, unsigned h, XG_PIXEL_FORMAT fmt) {
	if(!m_colorBuffer.create(w,h,fmt))
		return false;
	m_pixelfmt=fmt;
	set_plot_mode(PLOT_MODE_REPLACE);
	set_pattern(BRUSH_W3);
	set_bkcolor(0,0,0,255);
	set_color(255,255,255,255);
	m_xmin=m_ymin=0;
	m_xmax=w;
	m_ymax=h;
	return true;
}

void xraster::destroy()
{
	if(m_palette) {
		delete [] m_palette;
		m_palette=0;
	}
	if(m_patternBuffer) {
		xbuffer::free(m_patternBuffer);
		m_patternBuffer=0;
	}
	m_pixelfmt=PIXEL_FMT_UNKNOWN;
}

void xraster::attach_color_buffer(xcolor_buffer &new_buffer, XG_PIXEL_FORMAT fmt)
{
	if(!m_colorBuffer.empty())
		m_colorBuffer.clear();
	new_buffer.deliver(m_colorBuffer);
	if(m_pixelfmt!=fmt) {
		m_pixelfmt=fmt;
		set_plot_mode(m_plotmode);
	}
	m_xmin=m_ymin=0;
	m_xmax=m_colorBuffer.width();
	m_ymax=m_colorBuffer.height();
}

XG_PIXEL_FORMAT xraster::detach_color_buffer(xcolor_buffer &pre_buffer)
{
	m_colorBuffer.deliver(pre_buffer);
	XG_PIXEL_FORMAT fmt=m_pixelfmt;
	m_pixelfmt=PIXEL_FMT_UNKNOWN;
	return fmt;
}

void xraster::set_plot_mode(XG_PLOT_MODE mode) {
	if(m_pixelfmt==NATIVE_PIXEL_FORMAT) {
		m_plotter=get_plotter(mode);
	}
	else {
		m_plotter=&xraster::general_format_plotter;
	}
	m_plotmode=mode;
}

bool xraster::create_palette(unsigned numColor)
{
	if(m_palette) 
		delete [] m_palette;
	m_palette=new pixel_t[numColor];
	m_colorNum=numColor;
	return true;
}

pixel_t xraster::read_pixel(unsigned x, unsigned y) {
	if(m_pixelfmt==NATIVE_PIXEL_FORMAT)
		return *(pixel_t*)m_colorBuffer.get_elem(x,y);
	pixel_t c;
	get_translater(m_pixelfmt)(this,&c,m_colorBuffer.get_elem(x,y));
	return c;
}

void xraster::set_pattern(const xpattern &pat) {
	if(m_pattern.width!=pat.width || m_pattern.height!=pat.height) {
		xbuffer buffer;
		buffer.create(pat.width,pat.height,sizeof(pixel_t),1);
		m_patternPitch=buffer.pitch();
		if(m_patternBuffer) 
			xbuffer::free(m_patternBuffer);
		m_patternBuffer=buffer.release();
	}
	m_pattern=pat;
}

void xraster::make_pattern(float trans) {
	uint8_t *pbitmap=m_pattern.bitmap;
	uint8_t *pattern=m_patternBuffer;
	pixel_t c=m_color;
	uint8_t alpha=uint8_t(ALPHA_CHANNEL(c)*trans);
	for(unsigned i=0; i<m_pattern.height; ++i) {
		for(unsigned j=0; j<m_pattern.width; ++j) {
			SET_ALPHA_CHANNEL(c,FAST_DIV_255(alpha*pbitmap[j]));
			((pixel_t*)pattern)[j]=c;
		}
		pattern+=m_patternPitch;
		pbitmap+=m_pattern.width;
	}
}

void xraster::point(int x, int y) 
{
	if(is_pattern()) {
		make_pattern();
		plot_pattern(x,y);
		return;
	}
	int w=pen_width();
	if(w<2) {
		plot_s(x,y);
		return;
	}
	is_anti()?
		fill_circle_flat_anti(x,y,w>>1):
		fill_circle_flat(x,y,w>>1);
}

void xraster::line(int x0, int y0, int x1, int y1) 
{
	if(is_pattern()) {
		if(y0==y1) {
			scan_line_pattern(y0,x0,x1);
		}
		else if(x0==x1) {
			vert_line_pattern(x0,y0,y1);
		}
		else if(line_clip_2d_lb(x0,y0,x1,y1,xrecti_t(m_xmin-int(m_pattern.cx),m_ymin-int(m_pattern.cy),
			m_pattern.cx+m_xmax-1,m_pattern.cy+m_ymax-1))) {
			line_bresenham_pattern(x0,y0,x1,y1);
		}
		return;
	}
	int w=pen_width();
	if(w<2) {
		if(y0==y1) {
			scan_line_s(y0,x0,x1);
		}
		else if(x0==x1) {
			vert_line_s(x0,y0,y1);
		}
		else if(line_clip_2d_lb(x0,y0,x1,y1,xrecti_t(m_xmin,m_ymin,m_xmax-1,m_ymax-1))) {
			is_anti() ? 
				line_bresenham_anti(x0,y0,x1,y1) : 
				line_bresenham(x0,y0,x1,y1);
		}
		return;
	}
	if(y0==y1) {
		scan_line_w(y0,x0,x1,w);
	}
	else if(x0==x1) {
		vert_line_w(x0,y0,y1,w);
	}
	else if(line_clip_2d_lb(x0,y0,x1,y1,xrecti_t(m_xmin-w,m_ymin-w,w+m_xmax-1,w+m_ymax-1))) {
		line_bresenham_fat(x0,y0,x1,y1,w);
	}
}

void xraster::rect(int x, int y, int w, int h)
{
	if(w<1 || h<1) return;
	int x2=x+w-1, y2=y+h-1;
	if(x>=m_xmax || x2<m_xmin) return;
	if(y>=m_ymax || y2<m_ymin) return;
	int lef, top, rig, bot;
	unsigned wpen=pen_width();
	if(wpen<2) {
		lef=max(m_xmin,x); top=max(m_ymin,y);
		rig=min(m_xmax-1,x2); bot=min(m_ymax-1,y2);
		if(x>=m_xmin) 
			vert_line(x,top,bot);
		if(y>=m_ymin) 
			scan_line(y,lef,rig);
		if(x2<m_xmax)
			vert_line(x2,top,bot);
		if(y2<m_ymax)
			scan_line(y2,lef,rig);
	}
	else {
		int off=wpen>>1;
		lef=max(m_xmin,x-off); top=max(m_ymin,y-off);
		rig=min(m_xmax-1,x2+off); bot=min(m_ymax-1,y2+off);
		off=wpen&1?0:1;
		if(x>=m_xmin) 
			vert_line_w(x,top,bot-off,wpen);
		if(y>=m_ymin) 
			scan_line_w(y,lef,rig-off,wpen);
		if(x2<m_xmax)
			vert_line_w(x2,top,bot-off,wpen);
		if(y2<m_ymax)
			scan_line_w(y2,lef,rig-off,wpen);
	}
}

void xraster::round_rect(int x, int y, int w, int h, int r)
{
	int rmax=min(w,h)>>1;
	if(rmax<1 || r<1) {
		rect(x,y,w,h);
		return;	
	}
	if(r>rmax) r=rmax;
	if(w<1 || h<1) return;
	int x2=x+w-1, y2=y+h-1;
	if(x>=m_xmax || x2<m_xmin) return;
	if(y>=m_ymax || y2<m_ymin) return;
	int begx=max(m_xmin,x+r), begy=max(m_ymin,y+r);
	int endx=min(m_xmax-1,x2-r), endy=min(m_ymax-1,y2-r);
	if(!is_pattern()) {
		// 矩形边
		if(x>=m_xmin) 
			vert_line(x,begy,endy);
		if(y>=m_ymin) 
			scan_line(y,begx,endx);
		if(x2<m_xmax)
			vert_line(x2,begy,endy);
		if(y2<m_ymax)
			scan_line(y2,begx,endx);
		// 圆弧角
		if(is_anti())
			circle_part_anti(begx,begy,endx,endy,r);
		else circle_part(begx,begy,endx,endy,r);
	}
	else {
		if(x>=m_xmin) 
			vert_line_pattern(x,begy,endy);
		if(y>=m_ymin) 
			scan_line_pattern(y,begx,endx);
		if(x2<m_xmax)
			vert_line_pattern(x2,begy,endy);
		if(y2<m_ymax)
			scan_line_pattern(y2,begx,endx);
		circle_part_pattern(begx,begy,endx,endy,r);
	}
}

// 圆形,(cx,cy)为圆心,radius为半径
void xraster::circle(int cx, int cy, int radius)
{
	if(radius<1) return;
	if(is_pattern())
		circle_outline_pattern(cx,cy,radius);
	else if(is_anti()) 
		circle_outline_anti(cx,cy,radius);
	else circle_outline(cx,cy,radius);
}

// 椭圆形,(cx,cy)为中心,rx和ry分别为实轴半径和虚轴半径
void xraster::ellipse(int cx, int cy, int rx, int ry)
{
	if(rx<1 || ry<1) return;
	if(is_pattern())
		ellipse_outline_pattern(cx,cy,rx,ry);
	else if(is_anti()) 
		ellipse_outline_anti(cx,cy,rx,ry);
	else ellipse_outline(cx,cy,rx,ry);
}

void xraster::fill_circle(int cx, int cy, int radius)
{
	if(radius<1) return;
	is_anti() ? 
		fill_circle_flat_anti(cx,cy,radius) : 
		fill_circle_flat(cx,cy,radius);
}

void xraster::fill_ellipse(int cx, int cy, int rx, int ry)
{
	if(rx<1 || ry<1) return;
	is_anti() ? 
		fill_ellipse_flat_anti(cx,cy,rx,ry) : 
		fill_ellipse_flat(cx,cy,rx,ry);
}

void xraster::bitmap(int x, int y, uint8_t *pbits, int pitch, int srcw, int srch, bool trans)
{
	int srcx=0, srcy=0, offset;
	if(x<m_xmin) {
		offset=m_xmin-x;
		srcx+=offset;
		srcw-=offset;
		x=m_xmin;
	}
	if(y<m_ymin) {
		offset=m_ymin-y;
		srcy+=offset;
		srch-=offset;
		y=m_ymin;
	}
	if(x+srcw>m_xmax) {
		srcw=m_xmax-x;
	}
	if(y+srch>m_ymax) {
		srch=m_ymax-y;
	}
	if(srcw<1 || srch<1)
		return;
	if(trans) 
		_bitmap_transparent(x,y,pbits,pitch,srcx,srcy,srcw,srch);
	else _bitmap(x,y,pbits,pitch,srcx,srcy,srcw,srch);
}

void xraster::bitblt(int x, int y, int srcx, int srcy, int srcw, int srch)
{
	if(srcx>=m_xmax || srcy>=m_ymax)
		return;
	srcx=MAX(srcx,m_xmin);
	srcy=MAX(srcy,m_ymin);
	if(srcx+srcw>m_xmax) 
		srcw=m_xmax-srcx;
	if(srcy+srch>m_ymax) 
		srch=m_ymax-srcy;
	int offset;
	if(x<m_xmin) {
		offset=m_xmin-x;
		srcx+=offset;
		srcw-=offset;
		x=m_xmin;
	}
	if(y<m_ymin) {
		offset=m_ymin-y;
		srcy+=offset;
		srch-=offset;
		y=m_ymin;
	}
	if(x+srcw>m_xmax) {
		srcw=m_xmax-x;
	}
	if(y+srch>m_ymax) {
		srch=m_ymax-y;
	}
	if(srcw<1 || srch<1)
		return;
	size_t elemsize=stride();
	if(elemsize==sizeof(uint32_t)) {
		m_colorBuffer.move_elem<uint32_t>(x,y,srcx,srcy,srcw,srch);
	}
	else if(elemsize==sizeof(uint16_t)) {
		m_colorBuffer.move_elem<uint16_t>(x,y,srcx,srcy,srcw,srch);
	}
	else if(elemsize==sizeof(uint8_t)) {
		m_colorBuffer.move_elem<uint8_t>(x,y,srcx,srcy,srcw,srch);
	}
	else err("xraster::bitblt: not support %d bit pixel format",elemsize);
}

void xraster::bitblt(int x, int y, uint8_t *pbits, XG_PIXEL_FORMAT srcfmt, int pitch, 
			int srcx, int srcy, int srcw, int srch)
{
	int offset;
	if(x<m_xmin) {
		offset=m_xmin-x;
		srcx+=offset;
		srcw-=offset;
		x=m_xmin;
	}
	if(y<m_ymin) {
		offset=m_ymin-y;
		srcy+=offset;
		srch-=offset;
		y=m_ymin;
	}
	if(x+srcw>m_xmax) {
		srcw=m_xmax-x;
	}
	if(y+srch>m_ymax) {
		srch=m_ymax-y;
	}
	if(srcw<1 || srch<1)
		return;
	if(is_colorkey())
		_bitblt_colorkey(x,y,pbits,srcfmt,pitch,srcx,srcy,srcw,srch);
	else _bitblt(x,y,pbits,srcfmt,pitch,srcx,srcy,srcw,srch);
}

void xraster::bitblt(int x, int y, int w, int h, uint8_t *pbits, XG_PIXEL_FORMAT srcfmt, int pitch, 
			int srcx, int srcy, int srcw, int srch, int hint)
{
	if(x>=m_xmax || y>=m_ymax)
		return;
	float xbeg=0, ybeg=0, xend=1.0f, yend=1.0f;
	int dstw=w, dsth=h;
	if(x<m_xmin) {
		int offset=m_xmin-x;
		xbeg=float(offset)/w;
		w-=offset;
		x=m_xmin;
	}
	if(y<m_ymin) {
		int offset=m_ymin-y;
		ybeg=float(offset)/h;
		h-=offset;
		y=m_ymin;
	}
	if(x+w>m_xmax) {
		xend=1-float(x+w-m_xmax)/dstw;
		w=m_xmax-x;
	}
	if(y+h>m_ymax) {
		yend=1-float(y+h-m_ymax)/dsth;
		h=m_ymax-y;
	}
	if(w<1 || h<1)
		return;
	if(hint)
		_bitblt_stretch_bilinear(x,y,w,h,pbits,srcfmt,pitch,srcx,srcy,srcw,srch,xbeg,ybeg,xend,yend);
	else
		_bitblt_stretch_neareast(x,y,w,h,pbits,srcfmt,pitch,srcx,srcy,srcw,srch,xbeg,ybeg,xend,yend);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// base driver: 所有的光栅化函数使用(且仅使用)以下这些接口读写颜色缓存
//////////////////////////////////////////////////////////////////////////////////////////////////

void xraster::scan_line(int y, int begx, int endx) 
{
	assert(y>=m_ymin && y<m_ymax);
	assert(begx>=m_xmin && endx<m_xmax);
	unsigned _stride=stride();
	uint8_t *pwrite=get_pixel(begx,y);
	for(; begx<=endx; ++begx, pwrite+=_stride)
		m_plotter(this,pwrite,&m_color);
}

void xraster::vert_line(int x, int y0, int y1) 
{
	assert(x>=m_xmin && x<m_xmax);
	assert(y0>=m_ymin && y1<m_ymax);
	uint8_t *pwrite=get_pixel(x,y0);
	unsigned _pitch=pitch();
	for(; y0<=y1; ++y0) {
		m_plotter(this,pwrite,&m_color);
		pwrite+=_pitch;
	}
}

void xraster::scan_line_s(int y, int x0, int x1) {
	if(y<m_ymin || y>=m_ymax) return;
	if(x0>x1) swap(x0,x1);
	if(x0>=m_xmax || x1<m_xmin) return;
	if(x0<m_xmin) x0=m_xmin;
	if(x1>=m_xmax) x1=m_xmax-1;
	scan_line(y,x0,x1);
}

void xraster::vert_line_s(int x, int y0, int y1) {
	if(x<m_xmin || x>=m_xmax) return;
	if(y0>y1) swap(y0,y1);
	if(y0>=m_ymax || y1<m_ymin) return;
	if(y0<m_ymin) y0=m_ymin;
	if(y1>=m_ymax) y1=m_ymax-1;
	vert_line(x,y0,y1);
}

void xraster::scan_line_w(int y, int x0, int x1, int w)
{
	int up=w>>1;
	int low=w-up;
	up=max(y-up,m_ymin);
	low=min(y+low-1,m_ymax-1);
	if(up<m_ymax && low>=m_ymin) {
		if(x0>x1) swap(x0,x1);
		if(x0>=m_xmax || x1<m_xmin) return;
		x0=max(x0,m_xmin);
		x1=min(x1,m_xmax-1);
		for(int i=up; i<=low; ++i) {
			scan_line(i,x0,x1);
		}
	}
}

void xraster::vert_line_w(int x, int y0, int y1, int w)
{
	int up=w>>1;
	int low=w-up;
	up=max(x-up,m_xmin);
	low=min(x+low-1,m_xmax-1);
	if(up<m_xmax && low>=m_xmin) {
		if(y0>y1) swap(y0,y1);
		if(y0>=m_ymax || y1<m_ymin) return;
		y0=max(y0,m_ymin);
		y1=min(y1,m_ymax-1);
		unsigned _stride=stride();
		uint8_t *pwrite;
		while(y0<=y1) {
			pwrite=get_pixel(up,y0);
			for(int i=up; i<=low; ++i) {
				m_plotter(this,pwrite,&m_color);
				pwrite+=_stride;
			}
			y0+=1;
		}
	}
}

void xraster::plot_pattern(int x, int y)
{
	x-=m_pattern.cx;
	y-=m_pattern.cy;
	if(x>=m_xmax || y>=m_ymax)
		return;
	xrecti_t rect(m_xmin,m_ymin,m_xmax,m_ymax);
	rect.offset(-x,-y);
	rect.intersect(xrecti_t(0,0,m_pattern.width,m_pattern.height));
	if(rect.empty()) 
		return;
	_bitblt(max(m_xmin,x),max(m_ymin,y),m_patternBuffer,NATIVE_PIXEL_FORMAT,m_patternPitch,
		rect.left,rect.top,rect.width(),rect.height());
}

void xraster::scan_line_pattern(int y, int x0, int x1)
{
	if(y<m_ymin || y>=m_ymax) return;
	if(x0>x1) swap(x0,x1);
	if(x0>=m_xmax || x1<m_xmin) return;
	if(x0<m_xmin) x0=m_xmin;
	if(x1>=m_xmax) x1=m_xmax-1;
	make_pattern();
	for(int i=x0; i<=x1; ++i) {
		plot_pattern(i,y);
	}
}

void xraster::vert_line_pattern(int x, int y0, int y1)
{
	if(x<m_xmin || x>=m_xmax) return;
	if(y0>y1) swap(y0,y1);
	if(y0>=m_ymax || y1<m_ymin) return;
	if(y0<m_ymin) y0=m_ymin;
	if(y1>=m_ymax) y1=m_ymax-1;
	make_pattern();
	for(int i=y0; i<=y1; ++i) {
		plot_pattern(x,i);
	}
}




