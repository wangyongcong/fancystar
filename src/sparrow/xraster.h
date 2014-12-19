#pragma once

#ifndef __HEADER_XRASTER__
#define __HEADER_XRASTER__

#include "basedef.h"
#include "color.h"
#include "xrect.h"
#include "xmath.h"
#include "driver.h"
#include "xbuffer.h"

struct xpattern
{
	unsigned width;
	unsigned height;
	unsigned cx;
	unsigned cy;
	uint8_t* bitmap;
};

class xcolor_buffer : public xbuffer
{
	puint8_t *m_spans;
	uint8_t m_elemPower;
	uint8_t m_elemMask;
public:
	xcolor_buffer();
	~xcolor_buffer();
	bool create(unsigned w, unsigned h, XG_PIXEL_FORMAT fmt);
	void clear();
	uint8_t* release(unsigned *pitch=0, unsigned *width=0, unsigned *height=0);
	void deliver(xcolor_buffer &accept);
	bool share(const xcolor_buffer &buffer, int x, int y, unsigned w, unsigned h, bool bsafe=true);
	inline uint8_t* get_elem(unsigned x, unsigned y) {
		assert(y<m_height);
		assert(m_spans!=0);
		return m_spans[y]+(x<<m_elemPower);
	//	return m_spans[y]+(x<<2);
	}
	inline uint8_t* get_line(unsigned y) {
		assert(y<m_height);
		assert(m_spans!=0);
		return m_spans[y];
	}
	inline uint8_t get_elem_power() const {
		return m_elemPower;
	}
	inline uint8_t get_elem_mask() const {
		return m_elemMask;
	}
private:
	void reset_span_map(bool resize=true);
};

class xraster
{
public:
	// 光栅化状态
	enum RASTERSTATE
	{
		PEN_ST_WIDTH=0xF,
		PEN_ST_ANTI=0x10,
		PEN_ST_PATTERN=0x20,
		BITBLT_ST_COLORKEY=0x40
	};
	// 笔画样式
	enum PATTERN_STYLE
	{
		BRUSH_W3=0,
		BRUSH_W5,
		BRUSH_W7,
		NUM_PATTERN
	};

private:
	// 缓冲区
	xcolor_buffer m_colorBuffer;
	XG_PIXEL_FORMAT m_pixelfmt;
	// 绘制模式
	xplotter m_plotter;
	XG_PLOT_MODE m_plotmode;
	// 当前状态
	pixel_t m_color;
	pixel_t m_bkcolor;
	pixel_t m_colorkey;
	// 光栅化状态
	uint32_t m_state;
	// 裁剪区域
	int m_xmin, m_ymin, 
		m_xmax, m_ymax;
	// 调色板
	pixel_t *m_palette;
	unsigned m_colorNum;
	// 画笔样式
	static xpattern s_PreDefinedPattern[NUM_PATTERN];
	xpattern m_pattern;
	unsigned m_patternPitch;
	uint8_t *m_patternBuffer;
	// 填充器
	void *m_pfiller;
	// 任意像素格式绘制器
	static void general_format_plotter(void *pctx, void *pdst, void *psrc);
public:
	xraster();
	~xraster();
	void destroy();
	bool create(unsigned w, unsigned h, XG_PIXEL_FORMAT fmt=PIXEL_FMT_RGBA8888);
	// 缓冲区数据访问
	inline uint8_t* color_buffer() {
		return m_colorBuffer.get_buffer();
	}
	inline uint8_t* get_pixel(int x, int y) {
		return m_colorBuffer.get_elem(x,y);
	}
	inline unsigned pitch() const {
		return m_colorBuffer.pitch();
	}
	inline unsigned width() const {
		return m_colorBuffer.width();
	}
	inline unsigned height() const {
		return m_colorBuffer.height();
	}
	inline unsigned stride() const {
		return m_colorBuffer.size_elem();
	}
	inline XG_PIXEL_FORMAT pixel_format() const {
		return m_pixelfmt;
	}
	// 缓冲区绑定
	inline void share_color_buffer(xcolor_buffer &sub_buffer, unsigned x, unsigned y, unsigned w, unsigned h) {
		m_colorBuffer.share(sub_buffer,x,y,w,h);
	}
	void attach_color_buffer(xcolor_buffer &new_buffer, XG_PIXEL_FORMAT fmt);
	XG_PIXEL_FORMAT detach_color_buffer(xcolor_buffer &pre_buffer);
	// 设置颜色
	inline void set_index(unsigned index) {
		if(m_palette && index<m_colorNum) 
			m_color=m_palette[index];
	}
	inline const pixel_t& get_color() const {
		return m_color;
	}
	inline void set_color(pixel_t &c) {
		m_color=c;
	}
	inline void set_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a=255) {
		m_color=MAKE_COLOR(r,g,b,a);
	}
	inline pixel_t get_bkcolor() const {
		return m_bkcolor;
	}
	inline void set_bkcolor(uint8_t r, uint8_t g, uint8_t b, uint8_t a=255) {
		m_bkcolor=MAKE_COLOR(r,g,b,a);
	}
	void enable_colorkey(bool b) {
		b?add_state(m_state,BITBLT_ST_COLORKEY):remove_state(m_state,BITBLT_ST_COLORKEY);
	}
	bool is_colorkey() const {
		return have_state(m_state,BITBLT_ST_COLORKEY);
	}
	inline pixel_t colorkey() const {
		return m_colorkey;
	}
	inline void set_colorkey(uint8_t r, uint8_t g, uint8_t b, uint8_t a=255) {
		m_colorkey=MAKE_COLOR(r,g,b,a);
	}
	// 设置调色板
	bool create_palette(unsigned numColor);
	inline void set_palette(int index, const pixel_t &color) {
		m_palette[index]=color;
	}
	inline unsigned get_palette_color_num() const {
		return m_colorNum;
	}
	inline const pixel_t& get_palette_color(unsigned index) const {
		return m_palette[index];
	}
	//  设置绘制模式
	void set_plot_mode(XG_PLOT_MODE mode);
	inline XG_PLOT_MODE plot_mode() const {
		return m_plotmode;
	}
	// 设置画笔宽度
	inline void set_pen_width(unsigned wpen) {
		add_state(m_state,PEN_ST_WIDTH,wpen);
	}
	inline unsigned pen_width() const {
		return m_state&PEN_ST_WIDTH;
	}
	// 开启抗锯齿
	inline void enable_anti(bool b) {
		b?add_state(m_state,PEN_ST_ANTI):remove_state(m_state,PEN_ST_ANTI);
	}
	inline bool is_anti() const {
		return have_state(m_state,PEN_ST_ANTI);
	}
	// 开启笔画样式
	inline void enable_pattern(bool b) {
		b?add_state(m_state,PEN_ST_PATTERN):remove_state(m_state,PEN_ST_PATTERN);
	}
	inline bool is_pattern() const {
		return have_state(m_state,PEN_ST_PATTERN);
	}
	// 设置笔画样式
	void set_pattern(const xpattern &pat);
	inline void set_pattern(PATTERN_STYLE pat) {
		set_pattern(s_PreDefinedPattern[pat]);
	}
	inline const xpattern& cur_pattern() const {
		return m_pattern;
	}
	// 设置裁剪区域
	inline void set_clip_rect(int xmin, int ymin, int xmax, int ymax) {
		m_xmin=MAX(0,xmin), m_ymin=MAX(0,ymin), m_xmax=MIN(int(width()),xmax), m_ymax=MIN(int(height()),ymax);
	}
	inline void set_clip_rect(const xrecti_t &rect) {
		m_xmin=MAX(0,rect.left), m_ymin=MAX(0,rect.top), m_xmax=MIN(int(width()),rect.right), m_ymax=MIN(int(height()),rect.bottom);
	}
	inline void get_clip_rect(xrecti_t &rect) const {
		rect.xmin=m_xmin, rect.ymin=m_ymin, rect.xmax=m_xmax, rect.ymax=m_ymax;
	}
	inline void reset_clip_rect() {
		m_xmin=0, m_ymin=0, m_xmax=width(), m_ymax=height();
	}
	// 用背景色清屏
	inline void clear_screen() {
		m_colorBuffer.init(m_bkcolor);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// 像素绘制
	////////////////////////////////////////////////////////////////////////////////////////////////
	
	// 读取指定位置处的像素颜色
	pixel_t read_pixel(unsigned x, unsigned y);
	// 读取矩形像素区域
	void read_pixel(unsigned x, unsigned y, unsigned w, unsigned h, uint8_t *pret);

	////////////////////////////////////////////////////////////////////////////////////////////////
	// 像素绘制
	////////////////////////////////////////////////////////////////////////////////////////////////
	
	// 绘制位图
	void bitmap(int x, int y, uint8_t *pbits, int pitch, int srcw, int srch, bool trans=false);
	// 复制缓冲区像素
	void bitblt(int x, int y, int srcx, int srcy, int srcw, int srch);
	// 绘制任意格式像素
	void bitblt(int x, int y, uint8_t *pbits, XG_PIXEL_FORMAT srcfmt, int pitch, int srcx, int srcy, int srcw, int srch);
	// 绘制像素,可进行缩放
	void bitblt(int x, int y, int w, int h, uint8_t *pbits, XG_PIXEL_FORMAT srcfmt, int pitch, int srcx, int srcy, int srcw, int srch, int hint=1);
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// 绘制图元
	////////////////////////////////////////////////////////////////////////////////////////////////
	
	// 绘制点
	void point(int x, int y);
	// 绘制直线
	void line(int x0, int y0, int x1, int y1);
	// 绘制矩形
	void rect(int x, int y, int w, int h);
	inline void rect(const xrecti_t area) 
	{
		rect(area.left,area.top,area.width(),area.height());
	}
	// 绘制圆角矩形
	void round_rect(int x, int y, int w, int h, int radius);
	inline void round_rect(const xrecti_t area, int radius) 
	{
		round_rect(area.left,area.top,area.width(),area.height(),radius);
	}
	// 绘制圆
	void circle(int cx, int cy, int radius);
	// 绘制椭圆
	void ellipse(int cx, int cy, int rx, int ry);
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// 填充图元
	////////////////////////////////////////////////////////////////////////////////////////////////
	
	// 填充矩形
	void fill_rect(int x, int y, int w, int h);
	inline void fill_rect(const xrecti_t area) {
		fill_rect(area.left,area.top,area.width(),area.height());
	}
	// 填充圆角矩形
	void fill_round_rect(int x, int y, int w, int h, int radius);
	inline void fill_round_rect(const xrecti_t area, int radius) {
		fill_round_rect(area.left,area.top,area.width(),area.height(),radius);
	}
	// 填充圆
	void fill_circle(int cx, int cy, int radius);
	// 填充椭圆
	void fill_ellipse(int cx, int cy, int rx, int ry);
	// 填充任意多边形
	void poly_fill_beg();
	void poly_fill_add(int x, int y);
	void poly_fill_end();
	// 填充任意封闭区域
	void flood_fill(int x, int y);

//protected:
	////////////////////////////////////////////////////////////////////////////////////////////////
	// 基本绘制接口: 所有的光栅化函数使用(且仅使用)以下这些接口读写颜色缓存
	////////////////////////////////////////////////////////////////////////////////////////////////

	inline pixel_t transparency(float t) 
	{
		// 注意uint8_t*float比uint32_t*float快很多
		pixel_t c=m_color;
		SET_ALPHA_CHANNEL(c,uint8_t(ALPHA_CHANNEL(m_color)*t));
		return c;
	}
	inline void plot(int x, int y) 
	{
		assert(x>=m_xmin && x<m_xmax && y>=m_ymin && y<m_ymax);
		m_plotter(this,m_colorBuffer.get_elem(x,y),&m_color);
	}
	inline void plot(int x, int y, pixel_t c) 
	{
		assert(x>=m_xmin && x<m_xmax && y>=m_ymin && y<m_ymax);
		m_plotter(this,m_colorBuffer.get_elem(x,y),&c);
	}
	inline void plot_s(int x, int y) // safe plot
	{
		if(x>=m_xmin && x<m_xmax && y>=m_ymin && y<m_ymax)
			m_plotter(this,m_colorBuffer.get_elem(x,y),&m_color);
	}
	inline void plot_s(int x, int y, pixel_t c) // safe plot
	{
		if(x>=m_xmin && x<m_xmax && y>=m_ymin && y<m_ymax)
			m_plotter(this,m_colorBuffer.get_elem(x,y),&c);
	}
	void scan_line(int y, int x0, int x1);
	void vert_line(int x, int y0, int y1);

	inline void plot_quadrant(int cx, int cy, int x, int y)
	{
		plot_s(cx+x,cy+y);
		plot_s(cx-x,cy+y);
		plot_s(cx+x,cy-y);
		plot_s(cx-x,cy-y);
	}
	inline void plot_quadrant(int cx, int cy, int x, int y, pixel_t c)
	{
		plot_s(cx+x,cy+y,c);
		plot_s(cx-x,cy+y,c);
		plot_s(cx+x,cy-y,c);
		plot_s(cx-x,cy-y,c);
	}
	inline void plot_octant(int cx, int cy, int x, int y)
	{
		plot_quadrant(cx,cy,x,y);
		plot_quadrant(cx,cy,y,x);
	}
	inline void plot_octant(int cx, int cy, int x, int y, pixel_t c)
	{
		plot_quadrant(cx,cy,x,y,c);
		plot_quadrant(cx,cy,y,x,c);
	}

	void scan_line_s(int y, int x0, int x1);
	void vert_line_s(int x, int y0, int y1);

	void scan_line_w(int y, int x0, int x1, int w);
	void vert_line_w(int x, int y0, int y1, int w);

	void make_pattern(float trans=1.0f);
	void plot_pattern(int x, int y);
	inline void plot_quadrant_pattern(int cx, int cy, int x, int y)
	{
		plot_pattern(cx+x,cy+y);
		plot_pattern(cx-x,cy+y);
		plot_pattern(cx+x,cy-y);
		plot_pattern(cx-x,cy-y);
	}
	inline void plot_octant_pattern(int cx, int cy, int x, int y)
	{
		plot_quadrant_pattern(cx,cy,x,y);
		plot_quadrant_pattern(cx,cy,y,x);
	}
	void scan_line_pattern(int y, int x0, int x1);
	void vert_line_pattern(int x, int y0, int y1);

	////////////////////////////////////////////////////////////////////////////////////////////////
	// 基本图元光栅化操作
	////////////////////////////////////////////////////////////////////////////////////////////////

	// 基本bresenham直线
	void line_bresenham(int x0, int y0, int x1, int y1);
	// 抗锯齿bresenham直线
	void line_bresenham_anti(int x0, int y0, int x1, int y1);
	// bresenham宽直线
	void line_bresenham_fat(int x0, int y0, int x1, int y1, int w);
	// 抗锯齿bresenham宽直线(未实现)
//	void line_bresenham_fat_anti(int x0, int y0, int x1, int y1, int w);
	// pattern直线
	void line_bresenham_pattern(int x0, int y0, int x1, int y1);
	// 点画线(未实现)
//	void line_bresenham_stipple(int x0, int y0, int x1, int y1);
	// XialonWu直线
	void line_xw(int x0, int y0, int x1, int y1);
	// XialonWu对称直线
	void line_xw_sym(int x0, int y0, int x1, int y1);

	// 绘制bresenham圆
	void circle_outline(int cx, int cy, int radius);
	// 抗锯齿bresenham圆
	void circle_outline_anti(int cx, int cy, int radius);
	// pattern模式圆
	void circle_outline_pattern(int cx, int cy, int radius);
	// 填充圆(flat模式)
	void fill_circle_flat(int cx, int cy, int radius);
	// 填充圆,平滑轮廓
	void fill_circle_flat_anti(int cx, int cy, int radius);

	// 绘制bresenham椭圆
	void ellipse_outline(int cx, int cy, int rx, int ry);
	// 绘制抗锯齿bresenham椭圆
	void ellipse_outline_anti(int cx, int cy, int rx, int ry);
	// pattern模式椭圆
	void ellipse_outline_pattern(int cx, int cy, int rx, int ry);
	// 填充椭圆(flat模式)
	void fill_ellipse_flat(int cx, int cy, int rx, int ry);
	// 填充椭圆,平滑轮廓
	void fill_ellipse_flat_anti(int cx, int cy, int rx, int ry);

	// 贝塞尔曲线
	void curve_bezier(int nPoints, const xpt2i_t &beg, const xpt2i_t &ctrl1, const xpt2i_t &ctrl2, const xpt2i_t &end);
	// 抛物线
	void curve_parabola(int nPoints, float hs, float vs, float dt);

	// 绘制圆角
	void circle_part(int cx0, int cy0, int cx1, int cy1, int radius);
	// 绘制圆角(抗锯齿)
	void circle_part_anti(int cx0, int cy0, int cx1, int cy1, int radius);
	// 绘制圆角(pattern模式)
	void circle_part_pattern(int cx0, int cy0, int cx1, int cy1, int radius);
	// 填充圆角
	void fill_circle_part(int cx0, int cy0, int cx1, int cy1, int radius);
	// 绘制圆角(边缘平滑)
	void fill_circle_part_anti(int cx0, int cy0, int cx1, int cy1, int radius);

	////////////////////////////////////////////////////////////////////////////////////////////////
	// 直线裁剪
	////////////////////////////////////////////////////////////////////////////////////////////////

	// 基于标记检测的裁剪算法(Cohen-Sutherland)
	bool line_clip_2d_cs(int &x0, int &y0, int &x1, int &y1, const xrecti_t &rect);
	// 基于参数直线的直线裁剪算法(梁友栋-Barsky)
	bool line_clip_2d_lb(int &x0, int &y0, int &x1, int &y1, const xrecti_t &rect);

	////////////////////////////////////////////////////////////////////////////////////////////////
	// 绘制像素区域
	////////////////////////////////////////////////////////////////////////////////////////////////

	// 绘制位图,0用背景色绘制,1用前景色绘制
	void _bitmap(unsigned dstx, unsigned dsty, uint8_t *pbits, unsigned pitch, unsigned srcx, unsigned srcy, unsigned srcw, unsigned srch);
	// 绘制位图,0不绘制,1用前景色绘制
	void _bitmap_transparent(unsigned dstx, unsigned dsty, uint8_t *pbits, unsigned pitch, unsigned srcx, unsigned srcy, unsigned srcw, unsigned srch);

	// 绘制任意格式像素
	void _bitblt(unsigned dstx, unsigned dsty, uint8_t *pbits, XG_PIXEL_FORMAT fmt, unsigned pitch, unsigned srcx, unsigned srcy, unsigned srcw, unsigned srch);
	// 绘制像素,带颜色键检测
	void _bitblt_colorkey(unsigned dstx, unsigned dsty, uint8_t *pbits, XG_PIXEL_FORMAT fmt, unsigned pitch, unsigned srcx, unsigned srcy, unsigned srcw, unsigned srch);
	// 绘制像素,带缩放,使用箱式滤波
	void _bitblt_stretch_neareast(unsigned dstx, unsigned dsty, unsigned dstw, unsigned dsth, uint8_t *pbits, XG_PIXEL_FORMAT fmt, unsigned pitch, unsigned srcx, unsigned srcy, unsigned srcw, unsigned srch, float u0, float v0, float u1, float v1);
	// 绘制像素,带缩放,使用双线性插值滤波(NATIVE_PIXEL_FORMAT必须为RGBA8888)
	void _bitblt_stretch_bilinear(unsigned dstx, unsigned dsty, unsigned dstw, unsigned dsth, uint8_t *pbits, XG_PIXEL_FORMAT fmt, unsigned pitch, unsigned srcx, unsigned srcy, unsigned srcw, unsigned srch, float u0, float v0, float u1, float v1);

	// 以浮点坐标精度绘制像素
	void _bitblt_blur(float fdstx, float fdsty, uint8_t *pbits, XG_PIXEL_FORMAT fmt, unsigned pitch, unsigned srcx, unsigned srcy, unsigned srcw, unsigned srch);

	// 移植自fblend: 快速透明混合(NATIVE_PIXEL_FORMAT必须为RGBA8888)
	void _fast_blend(unsigned dstx, unsigned dsty, uint8_t *pbits, unsigned pitch, unsigned srcx, unsigned srcy, unsigned srcw, unsigned srch, uint8_t fact);

	////////////////////////////////////////////////////////////////////////////////////////////////
	// debug 接口
	////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG

	bool poly_fill_start();
	bool poly_fill_step();
	void poly_fill_clear();
	void poly_dump();

#endif // _DEBUG

};

#endif // end of __HEADER_XRASTER__

