#pragma once

#ifndef __INLINE_RGBA8888__
#define __INLINE_RGBA8888__

//////////////////////////////////////////////////////////////////////////////////////
// 像素格式转换
//////////////////////////////////////////////////////////////////////////////////////

#ifdef BYTE_ORDER_LITTLE_ENDIAN
	#define MAKE_RGBA8888(r,g,b,a) ((uint32_t(a)<<24)|(uint32_t(b)<<16)|(uint32_t(g)<<8)|(r))
	#define RGBA8888_GET_RED(c) ((c)&0xFF)
	#define RGBA8888_GET_GREEN(c) (((c)>>8)&0xFF)
	#define RGBA8888_GET_BLUE(c) (((c)>>16)&0xFF)
	#define RGBA8888_GET_ALPHA(c) (((c)>>24)&0xFF)
	#define CHANNEL_RED 0
	#define CHANNEL_GREEN 1
	#define CHANNEL_BLUE 2
	#define CHANNEL_ALPHA 3
#else
	#define MAKE_RGBA8888(r,g,b,a) ((uint32_t(r)<<24)|(uint32_t(g)<<16)|(uint32_t(b)<<8)|(a))
	#define RGBA8888_GET_RED(c) (((c)>>24)&0xFF)
	#define RGBA8888_GET_GREEN(c) (((c)>>16)&0xFF)
	#define RGBA8888_GET_BLUE(c) (((c)>>8)&0xFF)
	#define RGBA8888_GET_ALPHA(c) ((c)&0xFF)
	#define CHANNEL_RED 3
	#define CHANNEL_GREEN 2
	#define CHANNEL_BLUE 1
	#define CHANNEL_ALPHA 0
#endif

#define MAKE_RGB555(r,g,b) (((uint16_t(r)>>3)<<10)|((uint16_t(g)>>3)<<5)|(uint16_t(b)>>3))
#define RGB555_GET_RED(c) (((c)&0x00007c00)>>7)
#define RGB555_GET_GREEN(c) (((c)&0x000003e0)>>2)
#define RGB555_GET_BLUE(c) (((c)&0x0000001f)<<3)

#define MAKE_RGB565(r,g,b) (((uint16_t(r)>>3)<<11)|((uint16_t(g)>>2)<<5)|(uint16_t(b)>>3))
#define RGB565_GET_RED(c) (((c)&0x0000f800)>>8)
#define RGB565_GET_GREEN(c) (((c)&0x000007e0)>>3)
#define RGB565_GET_BLUE(c) (((c)&0x0000001f)<<3)

#define PFLOAT32_TO_U32(psrc) (uint32_t((*(float32_t*)(psrc))*255))
#define FLOAT32_TO_U32(c) (uint32_t((c)*255))
#define INVERSE_255 1.0f/255

#define FAST_DIV_255(n) (((n)+((n)>>8))>>8)

#define DST_COLOR *((uint32_t*)pdst)

void gray8_to_rgba8888(void *pctx, void *pdst, void *psrc)
{
	uint8_t gray=*(uint8_t*)psrc;
	DST_COLOR=MAKE_RGBA8888(gray,gray,gray,255);
}

void rgba8888_to_gray8(void *pctx, void *pdst, void *psrc)
{
	uint32_t c=*(uint32_t*)psrc;
	uint32_t gray=RGBA8888_GET_RED(c);
	gray+=RGBA8888_GET_GREEN(c);
	gray+=RGBA8888_GET_BLUE(c);
	gray+=RGBA8888_GET_ALPHA(c);
	*(uint8_t*)pdst=gray>>2;
}

void gray16_to_rgba8888(void *pctx, void *pdst, void *psrc)
{
	uint16_t gray=*(uint16_t*)psrc;
	gray>>=8;
	DST_COLOR=MAKE_RGBA8888(gray,gray,gray,255);
}

void rgba8888_to_gray16(void *pctx, void *pdst, void *psrc)
{
	uint32_t c=*(uint32_t*)psrc;
	uint32_t gray=RGBA8888_GET_RED(c);
	gray+=RGBA8888_GET_GREEN(c);
	gray+=RGBA8888_GET_BLUE(c);
	gray+=RGBA8888_GET_ALPHA(c);
	*(uint8_t*)pdst=gray>>2;
}

void index8_to_rgba8888(void *pctx, void *pdst, void *psrc)
{
	DST_COLOR=((uint32_t*)pctx)[*(uint8_t*)psrc];
}

void rgba8888_to_index8(void *pctx, void *pdst, void *psrc)
{
	// do nothing
}

void index16_to_rgba8888(void *pctx, void *pdst, void *psrc)
{
	DST_COLOR=((uint32_t*)pctx)[*(uint16_t*)psrc];
}

void rgba8888_to_index16(void *pctx, void *pdst, void *psrc)
{
	// do nothing
}

void rgb555_to_rgba8888(void *pctx, void *pdst, void *psrc)
{
	uint16_t c=*(uint16_t*)psrc;
	DST_COLOR=MAKE_RGBA8888(RGB555_GET_RED(c),RGB555_GET_GREEN(c),RGB555_GET_BLUE(c),255);
}

void rgba8888_to_rgb555(void *pctx, void *pdst, void *psrc)
{
	uint8_t *ptr=(uint8_t*)psrc;
	*(uint16_t*)pdst=MAKE_RGB555(ptr[CHANNEL_RED],ptr[CHANNEL_GREEN],ptr[CHANNEL_BLUE]);
}

void rgb565_to_rgba8888(void *pctx, void *pdst, void *psrc)
{
	uint16_t c=*(uint16_t*)psrc;
	DST_COLOR=MAKE_RGBA8888(RGB565_GET_RED(c),RGB565_GET_GREEN(c),RGB565_GET_BLUE(c),255);
}

void rgba8888_to_rgb565(void *pctx, void *pdst, void *psrc)
{
	uint8_t *ptr=(uint8_t*)psrc;
	*(uint16_t*)pdst=MAKE_RGB565(ptr[CHANNEL_RED],ptr[CHANNEL_GREEN],ptr[CHANNEL_BLUE]);
}

void rgb888_to_rgba8888(void *pctx, void *pdst, void *psrc)
{
	uint8_t *ptr=(uint8_t*)psrc;
	DST_COLOR=MAKE_RGBA8888(ptr[CHANNEL_RED],ptr[CHANNEL_GREEN],ptr[CHANNEL_BLUE],255);
}

void rgba8888_to_rgb888(void *pctx, void *pdst, void *psrc)
{
	((uint8_t*)pdst)[CHANNEL_RED]=((uint8_t*)psrc)[CHANNEL_RED];
	((uint8_t*)pdst)[CHANNEL_GREEN]=((uint8_t*)psrc)[CHANNEL_GREEN];
	((uint8_t*)pdst)[CHANNEL_BLUE]=((uint8_t*)psrc)[CHANNEL_BLUE];
}

void rgba8888_to_rgba8888(void *pctx, void *pdst, void *psrc)
{
	DST_COLOR=*(uint32_t*)psrc;
}

void rgba16_to_rgba8888(void *pctx, void *pdst, void *psrc)
{
	*(uint32_t*)pdst=MAKE_RGBA8888(((uint16_t*)psrc)[0]>>8,
		((uint16_t*)psrc)[1]>>8,
		((uint16_t*)psrc)[2]>>8,
		((uint16_t*)psrc)[3]>>8);
}

void rgba8888_to_rgba16(void *pctx, void *pdst, void *psrc)
{
	((uint16_t*)pdst)[0]=((uint8_t*)psrc)[CHANNEL_RED]<<8;
	((uint16_t*)pdst)[1]=((uint8_t*)psrc)[CHANNEL_GREEN]<<8;
	((uint16_t*)pdst)[2]=((uint8_t*)psrc)[CHANNEL_BLUE]<<8;
	((uint16_t*)pdst)[3]=((uint8_t*)psrc)[CHANNEL_ALPHA]<<8;
}

void rgba32_to_rgba8888(void *pctx, void *pdst, void *psrc)
{
	*(uint32_t*)pdst=MAKE_RGBA8888(((uint32_t*)psrc)[0]>>24,
		((uint32_t*)psrc)[1]>>24,
		((uint32_t*)psrc)[2]>>24,
		((uint32_t*)psrc)[3]>>24);
}

void rgba8888_to_rgba32(void *pctx, void *pdst, void *psrc)
{
	((uint16_t*)pdst)[0]=((uint8_t*)psrc)[CHANNEL_RED]<<24;
	((uint16_t*)pdst)[1]=((uint8_t*)psrc)[CHANNEL_GREEN]<<24;
	((uint16_t*)pdst)[2]=((uint8_t*)psrc)[CHANNEL_BLUE]<<24;
	((uint16_t*)pdst)[3]=((uint8_t*)psrc)[CHANNEL_ALPHA]<<24;
}

void red_to_rgba8888(void *pctx, void *pdst, void *psrc)
{
#ifdef BYTE_ORDER_LITTLE_ENDIAN
	DST_COLOR=*(uint8_t*)psrc|0xFF000000;
#else
	DST_COLOR=(*(uint8_t*)psrc)<<24|0xFF
#endif
}

void rgba8888_to_red(void *pctx, void *pdst, void *psrc)
{
	*(uint8_t*)pdst=((uint8_t*)psrc)[CHANNEL_RED];
}

void green_to_rgba8888(void *pctx, void *pdst, void *psrc)
{
#ifdef BYTE_ORDER_LITTLE_ENDIAN
	DST_COLOR=((*(uint8_t*)psrc)<<8)|0xFF000000;
#else
	DST_COLOR=((*(uint8_t*)psrc)<<16)|0xFF;
#endif
}

void rgba8888_to_green(void *pctx, void *pdst, void *psrc)
{
	*(uint8_t*)pdst=((uint8_t*)psrc)[CHANNEL_GREEN];
}

void blue_to_rgba8888(void *pctx, void *pdst, void *psrc)
{
#ifdef BYTE_ORDER_LITTLE_ENDIAN
	DST_COLOR=((*(uint8_t*)psrc)<<16)|0xFF000000;
#else
	DST_COLOR=((*(uint8_t*)psrc)<<8)|0xFF;
#endif
}

void rgba8888_to_blue(void *pctx, void *pdst, void *psrc)
{
	*(uint8_t*)pdst=((uint8_t*)psrc)[CHANNEL_BLUE];
}

void alpha_to_rgba8888(void *pctx, void *pdst, void *psrc)
{
#ifdef BYTE_ORDER_LITTLE_ENDIAN
	DST_COLOR=(*(uint8_t*)psrc)<<24;
#else
	DST_COLOR=*(uint8_t*)psrc;
#endif
}

void rgba8888_to_alpha(void *pctx, void *pdst, void *psrc)
{
	*(uint8_t*)pdst=((uint8_t*)psrc)[CHANNEL_ALPHA];
}

void rgba32f_to_rgba8888(void *pctx, void *pdst, void *psrc)
{
	float32_t *ptr=(float32_t*)psrc;
	DST_COLOR=MAKE_RGBA8888(FLOAT32_TO_U32(ptr[0]),FLOAT32_TO_U32(ptr[1]),
		FLOAT32_TO_U32(ptr[2]),FLOAT32_TO_U32(ptr[3]));
}

void rgba8888_to_rgba32f(void *pctx, void *pdst, void *psrc)
{
	((float32_t*)pdst)[0]=((uint8_t*)psrc)[CHANNEL_RED];
	((float32_t*)pdst)[1]=((uint8_t*)psrc)[CHANNEL_GREEN];
	((float32_t*)pdst)[2]=((uint8_t*)psrc)[CHANNEL_BLUE];
	((float32_t*)pdst)[3]=((uint8_t*)psrc)[CHANNEL_ALPHA];
}

void red32f_to_rgba8888(void *pctx, void *pdst, void *psrc)
{
#ifdef BYTE_ORDER_LITTLE_ENDIAN
	DST_COLOR=(PFLOAT32_TO_U32(psrc)&0xFF)|0xFF000000;
#else
	DST_COLOR=(PFLOAT32_TO_U32(psrc)<<24)|0xFF;
#endif
}

void rgba8888_to_red32f(void *pctx, void *pdst, void *psrc)
{
	*(float32_t*)pdst=((uint8_t*)psrc)[CHANNEL_RED]*INVERSE_255;
}

void green32f_to_rgba8888(void *pctx, void *pdst, void *psrc)
{
#ifdef BYTE_ORDER_LITTLE_ENDIAN
	DST_COLOR=(PFLOAT32_TO_U32(psrc)<<8)|0xFF000000;
#else
	DST_COLOR=(PFLOAT32_TO_U32(psrc)<<16)|0xFF;
#endif
}

void rgba8888_to_green32f(void *pctx, void *pdst, void *psrc)
{
	*(float32_t*)pdst=((uint8_t*)psrc)[CHANNEL_GREEN]*INVERSE_255;
}

void blue32f_to_rgba8888(void *pctx, void *pdst, void *psrc)
{
#ifdef BYTE_ORDER_LITTLE_ENDIAN
	DST_COLOR=(PFLOAT32_TO_U32(psrc)<<16)|0xFF000000;
#else
	DST_COLOR=(PFLOAT32_TO_U32(psrc)<<8)|0xFF;
#endif
}

void rgba8888_to_blue32f(void *pctx, void *pdst, void *psrc)
{
	*(float32_t*)pdst=((uint8_t*)psrc)[CHANNEL_BLUE]*INVERSE_255;
}

void alpha32f_to_rgba8888(void *pctx, void *pdst, void *psrc)
{
#ifdef BYTE_ORDER_LITTLE_ENDIAN
	DST_COLOR=(PFLOAT32_TO_U32(psrc)<<24)|0xFF000000;
#else
	DST_COLOR=PFLOAT32_TO_U32(psrc)&0xFF;
#endif
}

void rgba8888_to_alpha32f(void *pctx, void *pdst, void *psrc)
{
	*(float32_t*)pdst=((uint8_t*)psrc)[CHANNEL_ALPHA]*INVERSE_255;
}

//////////////////////////////////////////////////////////////////////////////////////
// 读写像素
//////////////////////////////////////////////////////////////////////////////////////

void plot_replace_rgba8888(void *pctx, void *pdst, void *psrc) 
{
	DST_COLOR=*(uint32_t*)psrc;
}

void plot_red_rgba8888(void *pctx, void *pdst, void *psrc) 
{
	((uint8_t*)pdst)[CHANNEL_RED]=*(uint8_t*)psrc;
}

void plot_green_rgba8888(void *pctx, void *pdst, void *psrc) 
{
	((uint8_t*)pdst)[CHANNEL_GREEN]=*(uint8_t*)psrc;
}

void plot_blue_rgba8888(void *pctx, void *pdst, void *psrc) 
{
	((uint8_t*)pdst)[CHANNEL_BLUE]=*(uint8_t*)psrc;
}

void plot_alpha_rgba8888(void *pctx, void *pdst, void *psrc) 
{
	((uint8_t*)pdst)[CHANNEL_ALPHA]=*(uint8_t*)psrc;
}

void plot_add_rgba8888(void *pctx, void *pdst, void *psrc) 
{
	((uint8_t*)pdst)[0]+=((uint8_t*)psrc)[0];
	((uint8_t*)pdst)[1]+=((uint8_t*)psrc)[1];
	((uint8_t*)pdst)[2]+=((uint8_t*)psrc)[2];
	((uint8_t*)pdst)[3]+=((uint8_t*)psrc)[3];
}

void plot_and_rgba8888(void *pctx, void *pdst, void *psrc)
{
	*(uint32_t*)pdst&=*(uint32_t*)psrc;
}

void plot_inverse_rgba8888(void *pctx, void *pdst, void *psrc)
{
	*(uint32_t*)pdst=~(*(uint32_t*)psrc);
}

void plot_not_rgba8888(void *pctx, void *pdst, void *psrc)
{
	*(uint32_t*)pdst=~(*(uint32_t*)pdst);
}

void plot_or_rgba8888(void *pctx, void *pdst, void *psrc) 
{
	*(uint32_t*)pdst|=*(uint32_t*)pdst;
}

void plot_sub_rgba8888(void *pctx, void *pdst, void *psrc) 
{
	((uint8_t*)pdst)[0]-=((uint8_t*)psrc)[0];
	((uint8_t*)pdst)[1]-=((uint8_t*)psrc)[1];
	((uint8_t*)pdst)[2]-=((uint8_t*)psrc)[2];
	((uint8_t*)pdst)[3]-=((uint8_t*)psrc)[3];
}

void plot_xor_rgba8888(void *pctx, void *pdst, void *psrc) 
{
	*(uint32_t*)pdst^=*(uint32_t*)pdst;
}

void plot_blend_rgba8888(void *pctx, void *pdst, void *psrc) 
{
	uint8_t a=((uint8_t*)psrc)[CHANNEL_ALPHA];

/*	// 在混合无法避免的情况下,去掉下面的alpha测试可以提高10%左右的性能
	// 但如果大部分的alpha都是0或255呢?
	if(a==0) return;
	else if(a==255) {
		*(uint32_t*)pdst=*(uint32_t*)psrc;
		return;
	}*/
	uint32_t tmp;

#define _BLEND_CHANNEL(channel) \
	tmp=(((uint8_t*)psrc)[channel]-((uint8_t*)pdst)[channel])*a;\
	((uint8_t*)pdst)[channel]+=FAST_DIV_255(tmp);

	_BLEND_CHANNEL(CHANNEL_RED);
	_BLEND_CHANNEL(CHANNEL_GREEN);
	_BLEND_CHANNEL(CHANNEL_BLUE);
	_BLEND_CHANNEL(CHANNEL_ALPHA);
}

#endif // end of __INLINE_RGBA8888__

