#pragma once

#ifndef __HEADER_COLOR__
#define __HEADER_COLOR__

#define RED_MASK_ARGB32		0x00ff0000
#define GREEN_MASK_ARGB32	0x0000ff00
#define BLUE_MASK_ARGB32	0x000000ff
#define ALPHA_MASK_ARGB32	0xff000000

#define RED_MASK_RGB24		0x00ff0000
#define GREEN_MASK_RGB24	0x0000ff00
#define BLUE_MASK_RGB24		0x000000ff

#define RED_MASK_RGB565		0x0000f800
#define GREEN_MASK_RGB565	0x000007e0
#define BLUE_MASK_RGB565	0x0000001f

#define RED_MASK_RGB555		0x00007c00
#define GREEN_MASK_RGB555	0x000003e0
#define BLUE_MASK_RGB555	0x0000001f

//==pixel data==========================================================================================

struct xrgb555
{
	uint16_t color;
	
	xrgb555() {}
	xrgb555(uint32_t c) {color=c;}
	xrgb555(uint8_t r, uint8_t g, uint8_t b) { set(r,g,b); }
	
	inline uint8_t get_red() const { 
		return uint8_t((color&RED_MASK_RGB555)>>7); 
	}
	inline uint8_t get_green() const { 
		return uint8_t((color&GREEN_MASK_RGB555)>>2); 
	}
	inline uint8_t get_blue() const { 
		return uint8_t((color&BLUE_MASK_RGB555)<<3); 
	}
	inline uint8_t get_alpha() const {return 255;}

	inline void set(uint8_t r, uint8_t g, uint8_t b) { 
		color=uint16_t(((uint16_t(r)>>3)<<10)|((uint16_t(g)>>3)<<5)|(uint16_t(b)>>3)); 
	}
	inline void set_red(uint8_t r) {
		color=(color&(~RED_MASK_RGB555))|((uint16_t(r)>>3)<<10);
	}
	inline void set_green(uint8_t g) {
		color=(color&(~GREEN_MASK_RGB555))|((uint16_t(g)>>3)<<5);
	}
	inline void set_blue(uint8_t b) {
		color=(color&(~BLUE_MASK_RGB555))|(uint16_t(b)>>3);
	}
	inline void set_alpha(uint8_t a) {}

	inline bool operator == (const xrgb555 &c) const { 
		return color==c.color;
	}
	inline bool operator != (const xrgb555 &c) const {
		return color!=c.color;
	}

	inline uint32_t get() { return color; }
	inline xrgb555& operator = (uint16_t c) { 
		color=c; 
		return *this; 
	}
};

struct xrgb565
{
	uint16_t color;

	xrgb565() {}
	xrgb565(uint16_t c) {color=c;}
	xrgb565(uint8_t r, uint8_t g, uint8_t b) { set(r,g,b); }

	inline uint8_t get_red() const { 
		return uint8_t((color&RED_MASK_RGB565)>>8); 
	}
	inline uint8_t get_green() const { 
		return uint8_t((color&GREEN_MASK_RGB565)>>3); 
	}
	inline uint8_t get_blue() const { 
		return uint8_t((color&BLUE_MASK_RGB565)<<3); 
	}
	inline uint8_t get_alpha() const {return 255;}

	inline void set(uint8_t r, uint8_t g, uint8_t b) { 
		color=uint16_t(((uint16_t(r)>>3)<<11)|((uint16_t(g)>>2)<<5)|(uint16_t(b)>>3)); 
	}
	inline void set_red(uint8_t r) {
		color=(color&(~RED_MASK_RGB565))|((uint16_t(r)>>3)<<11);
	}
	inline void set_green(uint8_t g) {
		color=(color&(~GREEN_MASK_RGB565))|((uint16_t(g)>>2)<<5);
	}
	inline void set_blue(uint8_t b) {
		color=(color&(~BLUE_MASK_RGB565))|(uint16_t(b)>>3);
	}
	inline void set_alpha(uint8_t a) {}

	inline bool operator == (const xrgb565 &c) const { 
		return color==c.color; 
	}
	inline bool operator != (const xrgb565 &c) const { 
		return color!=c.color; 
	}

	inline uint32_t get() { return color; }
	inline xrgb565& operator = (uint16_t c) { 
		color=c; 
		return *this; 
	}
};

struct xrgb888
{
	uint8_t red, green, blue;

	xrgb888() {}
	xrgb888(uint32_t c) {*this=c;}
	xrgb888(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b) {}

	inline uint8_t get_red() const {return red;}
	inline uint8_t get_green() const {return green;}
	inline uint8_t get_blue() const {return blue;}
	inline uint8_t get_alpha() const {return 255;}

	inline void set(uint8_t r, uint8_t g, uint8_t b) { 
		red=r; green=g; blue=b; 
	}
	inline void set_red(uint8_t r) {red=r;}
	inline void set_green(uint8_t g) {green=g;}
	inline void set_blue(uint8_t b) {blue=b;}
	inline void set_alpha(uint8_t a) {}
	
	inline bool operator == (const xrgb888 &c) const { 
		return (red==c.red && green==c.green && blue==c.blue);
	}
	inline bool operator != (const xrgb888 &c) const { 
		return !(*this==c);
	}

	inline uint32_t get() { 
		return (uint32_t(blue)<<16)|(uint32_t(green)<<8)|(red); 
	}
	inline xrgb888& operator = (uint32_t c) { 
		red=uint8_t(c); 
		green=uint8_t(c>>8); 
		blue=uint8_t(c>>16); 
		return *this; 
	}
};

struct xrgba8888
{
	union
	{
		uint32_t color;
		struct { uint8_t red; uint8_t green; uint8_t blue; uint8_t alpha; };
	};

	xrgba8888() {}
	xrgba8888(uint32_t c) {color=c;}
	xrgba8888(uint8_t r, uint8_t g, uint8_t b, uint8_t a=0) : red(r), green(g), blue(b), alpha(a) {}
	
	inline uint8_t get_red() const {return red;}
	inline uint8_t get_green() const {return green;}
	inline uint8_t get_blue() const {return blue;}
	inline uint8_t get_alpha() const {return alpha;}

	inline void set(uint8_t r, uint8_t g, uint8_t b, uint8_t a=255) { 
		red=r; green=g; blue=b; alpha=a; 
	}
	inline void set_red(uint8_t r) {red=r;}
	inline void set_green(uint8_t g) {green=g;}
	inline void set_blue(uint8_t b) {blue=b;}
	inline void set_alpha(uint8_t a) {alpha=a;}

	inline bool operator == (const xrgba8888 &c) const {
		return color==c.color; 
	}
	inline bool operator != (const xrgba8888 &c) const {
		return color!=c.color; 
	}

	inline uint32_t get() { 
		return color; 
	}
	inline xrgba8888& operator= (uint32_t c) { 
		color=c;
		return *this; 
	}
};

struct xrgba32f
{
	union {
		int32_t ival;
		float32_t fval;
	}red, green, blue, alpha;

	xrgba32f() {}
	xrgba32f(float r, float g, float b, float a) {
		set(r,g,b,a);
	}

	inline float get_red() const {return red.fval;}
	inline float get_green() const {return green.fval;}
	inline float get_blue() const {return blue.fval;}
	inline float get_alpha() const {return alpha.fval;}

	inline void set(float r, float g, float b, float a=1.0f) { 
		red.fval=r; green.fval=g; blue.fval=b; alpha.fval=a; 
	}
	inline void set_red(float r) {red.fval=r;}
	inline void set_green(float g) {green.fval=g;}
	inline void set_blue(float b) {blue.fval=b;}
	inline void set_alpha(float a) {alpha.fval=a;}

	inline bool operator == (const xrgba32f &c) const {
		return red.ival==c.red.ival && green.ival==c.green.ival && blue.ival==c.green.ival && alpha.ival==c.alpha.ival;
	}
	inline bool operator != (const xrgba32f &c) const {
		return !(*this==c);
	}

	void clamp() 
	{
		red.ival&=~(red.ival>>31);
		red.fval-=1.0f;
		red.ival&=red.ival>>31;
		red.fval+=1.0f;

		green.ival&=~(green.ival>>31);
		green.fval-=1.0f;
		green.ival&=green.ival>>31;
		green.fval+=1.0f;

		blue.ival&=~(blue.ival>>31);
		blue.fval-=1.0f;
		blue.ival&=blue.ival>>31;
		blue.fval+=1.0f;

		alpha.ival&=~(alpha.ival>>31);
		alpha.fval-=1.0f;
		alpha.ival&=alpha.ival>>31;
		alpha.fval+=1.0f;
	}

};

#endif // end of __HEADER_COLOR__
