#include "fscorepch.h"
#include "wyc/util/util.h"
#include "wyc/util/strutil.h"
#include "wyc/render/image.h"

#define XIMAGE_PITCH_ALIGNMENT 4

namespace wyc
{

inline unsigned int image_pitch(unsigned w, uint8_t bpp, uint8_t align=XIMAGE_PITCH_ALIGNMENT) { 
	uint8_t mask=(align<<3)-1;
	assert((mask&(mask+1))==0);
	return ((w*bpp+mask)&(~mask))>>3;
}

inline uint8_t image_bpp(ximage::PIXEL_FORMAT fmt)
{
	static const uint8_t ls_bpp[ximage::SUPPORTED_PIXEL_FORMAT] = {
		32,	// RGBA_8888
		24,	// RGBA_888
		8,	// LUMINANCE_8
	};
	return ls_bpp[fmt];
}

ximage::ximage()
{
	m_bitmap=0;
	m_width=0;
	m_height=0;
	m_pitch=0;
	m_pxfmt=UNKNOWN_FORMAT;
}

ximage::~ximage() 
{
	clear();
}

void ximage::clear() 
{
	if(m_bitmap) {
		delete [] m_bitmap;
		m_bitmap=0;
		m_width=m_height=m_pitch=0;
		m_pxfmt=UNKNOWN_FORMAT;
	}
}

uint8_t* ximage::detach_bitmap()
{
	if(!m_bitmap)
		return 0;
	uint8_t *ret=m_bitmap;
	m_bitmap=0;
	m_width=m_height=m_pitch=0;
	m_pxfmt=UNKNOWN_FORMAT;
	return ret;
}

bool ximage::load(const char *filename, PIXEL_FORMAT fmt) 
{
	if(m_bitmap)
		clear();
	FREE_IMAGE_FORMAT file_type;
	file_type=FreeImage_GetFileType(filename);
	if(file_type==FIF_UNKNOWN) {
		file_type=FreeImage_GetFIFFromFilename(filename);
		if(file_type==FIF_UNKNOWN) {
			wyc_error("未知的图像文件格式: %s",filename);
			return false;
		}
	}
	FIBITMAP *pbm=FreeImage_Load(file_type,filename);
	if(!pbm)  {
		wyc_error("加载图像失败: %s",filename);
		return false;
	}
	if(FreeImage_GetImageType(pbm)!=FIT_BITMAP) {
		wyc_error("加载图像失败,不支持非标准位图: %s",filename);
		FreeImage_Unload(pbm);
		return false;
	}
	bool ret;
	if(fmt==LUMINANCE_8)
		ret=load_luminance8((uintptr_t)pbm,FreeImage_GetWidth(pbm),FreeImage_GetHeight(pbm));
	else
		ret=load_rgba8888((uintptr_t)pbm,FreeImage_GetWidth(pbm),FreeImage_GetHeight(pbm));
	FreeImage_Unload(pbm);
	return ret;
}

bool ximage::load_rgba8888(uintptr_t handle, unsigned w, unsigned h, unsigned begx, unsigned begy, bool reverse)
{
	FIBITMAP *pbm=(FIBITMAP*)handle;
	unsigned pitch=image_pitch(w,32);
	unsigned sz=pitch*h;
	uint8_t *pbitmap=new uint8_t[sz];
	unsigned endx=std::min(begx+w,w), endy=std::min(begy+h,h);
	uint8_t *pdst=pbitmap;
	uint8_t idx_red, idx_blue;
	if(reverse) {
		idx_red=2;
		idx_blue=0;
	}
	else {
		idx_red=0;
		idx_blue=2;
	}
	switch(FreeImage_GetBPP(pbm)) {
	case 1: {
		assert(2>=FreeImage_GetColorsUsed(pbm));
		RGBQUAD *palette=(RGBQUAD*)FreeImage_GetPalette(pbm);
		uint8_t shift=7;
		for(unsigned int y=begy; y<endy; ++y) {
			uint8_t *psrc=(uint8_t*)FreeImage_GetScanLine(pbm,y);
			for(unsigned int x=begx; x<endx; ++x) {
				const RGBQUAD &color=palette[(psrc[x>>3]>>shift)&1];
				pdst[idx_red]=color.rgbRed;
				pdst[1]=color.rgbGreen;
				pdst[idx_blue]=color.rgbBlue;
				pdst[3]=255;
				pdst+=4;
				shift=(shift-1)&0x7;
			}
		}
		break;
	}
	case 4: {
		assert(16>=FreeImage_GetColorsUsed(pbm));
		RGBQUAD *palette=(RGBQUAD*)FreeImage_GetPalette(pbm);
		uint8_t shift=4;
		for(unsigned int y=begy; y<endy; ++y) {
			uint8_t *psrc=(uint8_t*)FreeImage_GetScanLine(pbm,y);
			for(unsigned int x=begx; x<endx; ++x) {
				const RGBQUAD &color=palette[(psrc[x>>1]>>shift)&3];
				pdst[idx_red]=color.rgbRed;
				pdst[1]=color.rgbGreen;
				pdst[idx_blue]=color.rgbBlue;
				pdst[3]=255;
				pdst+=4;
				shift^=4;
			}
		}
		break;
	}
	case 8: {
		unsigned int color_num=FreeImage_GetColorsUsed(pbm);
		RGBQUAD *palette=(RGBQUAD*)FreeImage_GetPalette(pbm);
		if(color_num<=0 || palette==NULL)
			return false;
		unsigned transnum=0;
		uint8_t *pTransTable=0;
		if(FreeImage_IsTransparent(pbm)) {
			transnum=FreeImage_GetTransparencyCount(pbm);
			pTransTable=FreeImage_GetTransparencyTable(pbm);
		}
		uint8_t index;
		for(unsigned int y=begy; y<endy; ++y) {
			uint8_t *psrc=(uint8_t*)FreeImage_GetScanLine(pbm,y);
			for(unsigned int x=begx; x<endx; ++x) {
				index=psrc[x];
				assert(index<color_num);
				const RGBQUAD &color=palette[index];
				pdst[idx_red]=color.rgbRed;
				pdst[1]=color.rgbGreen;
				pdst[idx_blue]=color.rgbBlue;
				if(pTransTable)
					pdst[3]=index<transnum?pTransTable[index]:255;
				else pdst[3]=255;
				pdst+=4;
			}
		}
		break;
	}
	case 16: {
		unsigned int red_mask, green_mask, blue_mask;
		red_mask=FreeImage_GetRedMask(pbm);
		green_mask=FreeImage_GetGreenMask(pbm);
		blue_mask=FreeImage_GetBlueMask(pbm);
		unsigned char red_shift, green_shift;
		if(green_mask==FI16_555_GREEN_MASK) { // RGB555
			red_shift=7;
			green_shift=2;
		}
		else { // RGB565
			red_shift=8;
			green_shift=3;
		}
		for(unsigned int y=begy; y<endy; ++y) {
			WORD *psrc=(WORD*)FreeImage_GetScanLine(pbm,y);
			for(unsigned int x=begx; x<endx; ++x) {
				pdst[idx_red]=uint8_t((psrc[x]&red_mask)>>red_shift);
				pdst[1]=uint8_t((psrc[x]&green_mask)>>green_shift);
				pdst[idx_blue]=uint8_t((psrc[x]&blue_mask)<<3);
				pdst[3]=255;
				pdst+=4;
			}
		}
		break;
	}
	case 24: {
		for(unsigned int y=begy; y<endy; ++y) {
			uint8_t *psrc=FreeImage_GetScanLine(pbm,y);
			for(unsigned int x=begx; x<endx; ++x) {
				pdst[idx_red]=psrc[FI_RGBA_RED];
				pdst[1]=psrc[FI_RGBA_GREEN];
				pdst[idx_blue]=psrc[FI_RGBA_BLUE];
				pdst[3]=255;
				pdst+=4;
				psrc+=3;
			}
		}
		break;
	}
	case 32: {
		for(unsigned int y=begy; y<endy; ++y) {
			uint8_t *psrc=FreeImage_GetScanLine(pbm,y);
			for(unsigned int x=begx; x<endx; ++x) {
				pdst[idx_red]=psrc[FI_RGBA_RED];
				pdst[1]=psrc[FI_RGBA_GREEN];
				pdst[idx_blue]=psrc[FI_RGBA_BLUE];
				pdst[3]=psrc[FI_RGBA_ALPHA];
				pdst+=4;
				psrc+=4;
			}
		}
		break;
	}
	default: 
		wyc_warn("ximage::load_rgba8888: can not load [%d] bit image as RGBA_8888",FreeImage_GetBPP(pbm));
		return false;
	}
	m_pitch=pitch;
	m_bitmap=pbitmap;
	m_width=w;
	m_height=h;
	m_pxfmt=RGBA_8888;
	return true;
}

#define LUMINANCE_R_FACTOR 0.299f
#define LUMINANCE_G_FACTOR 0.587f
#define LUMINANCE_B_FACTOR 0.114f

bool ximage::load_luminance8(uintptr_t handle, unsigned w, unsigned h, unsigned begx, unsigned begy)
{
	FIBITMAP *pbm=(FIBITMAP*)handle;
	unsigned pitch=image_pitch(w,8);
	unsigned sz=pitch*h;
	uint8_t *pbitmap=new uint8_t[sz];
	unsigned endx=std::min(begx+w,w), endy=std::min(begy+h,h);
	uint8_t *pdst=pbitmap;
	switch(FreeImage_GetBPP(pbm)) {
	case 1: {
		RGBQUAD *palette=(RGBQUAD*)FreeImage_GetPalette(pbm);
		uint8_t shift=7;
		for(unsigned int y=begy; y<endy; ++y) {
			uint8_t *psrc=(uint8_t*)FreeImage_GetScanLine(pbm,y);
			for(unsigned int x=begx; x<endx; ++x) {
				const RGBQUAD &color=palette[(psrc[x>>3]>>shift)&1];
				*pdst++ = uint8_t(LUMINANCE_R_FACTOR*color.rgbRed + LUMINANCE_G_FACTOR*color.rgbGreen + LUMINANCE_B_FACTOR*color.rgbBlue);
				shift=(shift-1)&0x7;
			}
		}
		break;
	}
	case 4: {
		RGBQUAD *palette=(RGBQUAD*)FreeImage_GetPalette(pbm);
		uint8_t shift=4;
		for(unsigned int y=begy; y<endy; ++y) {
			uint8_t *psrc=(uint8_t*)FreeImage_GetScanLine(pbm,y);
			for(unsigned int x=begx; x<endx; ++x) {
				const RGBQUAD &color=palette[(psrc[x>>1]>>shift)&1];
				*pdst++ = uint8_t(LUMINANCE_R_FACTOR*color.rgbRed + LUMINANCE_G_FACTOR*color.rgbGreen + LUMINANCE_B_FACTOR*color.rgbBlue);
				shift^=4;
			}
		}
		break;
	}
	case 8: {
		unsigned int color_num=FreeImage_GetColorsUsed(pbm);
		RGBQUAD *palette=(RGBQUAD*)FreeImage_GetPalette(pbm);
		if(color_num<=0 || palette==NULL) 
			return false;
		uint8_t index;
		for(unsigned int y=begy; y<endy; ++y) {
			uint8_t *psrc=(uint8_t*)FreeImage_GetScanLine(pbm,y);
			for(unsigned int x=begx; x<endx; ++x) {
				index=psrc[x];
				RGBQUAD &color=palette[index];
				*pdst++ = uint8_t(LUMINANCE_R_FACTOR*color.rgbRed + LUMINANCE_G_FACTOR*color.rgbGreen + LUMINANCE_B_FACTOR*color.rgbBlue);
			}
		}
		break;
	}
	case 16: {
		unsigned int red_mask, green_mask, blue_mask;
		red_mask=FreeImage_GetRedMask(pbm);
		green_mask=FreeImage_GetGreenMask(pbm);
		blue_mask=FreeImage_GetBlueMask(pbm);
		unsigned char red_shift, green_shift;
		if(green_mask==FI16_555_GREEN_MASK) { // RGB555
			red_shift=7;
			green_shift=2;
		}
		else { // RGB565
			red_shift=8;
			green_shift=3;
		}
		for(unsigned int y=begy; y<endy; ++y) {
			WORD *psrc=(WORD*)FreeImage_GetScanLine(pbm,y);
			for(unsigned int x=begx; x<endx; ++x) 
				*pdst++ = uint8_t(LUMINANCE_R_FACTOR*((psrc[x]&red_mask)>>red_shift) \
					+ LUMINANCE_G_FACTOR*((psrc[x]&green_mask)>>green_shift) \
					+ LUMINANCE_B_FACTOR*((psrc[x]&blue_mask)<<3));
		}
		break;
	}
	case 24: {
		for(unsigned int y=begy; y<endy; ++y) {
			uint8_t *psrc=FreeImage_GetScanLine(pbm,y);
			for(unsigned int x=begx; x<endx; ++x, psrc+=3) 
				*pdst++ = uint8_t(LUMINANCE_R_FACTOR*psrc[FI_RGBA_RED] \
					+ LUMINANCE_G_FACTOR*psrc[FI_RGBA_GREEN] \
					+ LUMINANCE_B_FACTOR*psrc[FI_RGBA_BLUE]);
		}
		break;
	}
	case 32: {
		for(unsigned int y=begy; y<endy; ++y) {
			uint8_t *psrc=FreeImage_GetScanLine(pbm,y);
			for(unsigned int x=begx; x<endx; ++x, psrc+=4) 
				*pdst++ = uint8_t(LUMINANCE_R_FACTOR*psrc[FI_RGBA_RED] \
					+ LUMINANCE_G_FACTOR*psrc[FI_RGBA_GREEN] \
					+ LUMINANCE_B_FACTOR*psrc[FI_RGBA_BLUE]);
		}
		break;
	}
	default: 
		wyc_warn("ximage::load_luminance8: can not load [%d] bit image as 8 bit luminance",FreeImage_GetBPP(pbm));
		return false;
	}
	m_pitch=pitch;
	m_bitmap=pbitmap;
	m_width=w;
	m_height=h;
	m_pxfmt=LUMINANCE_8;

	return true;
}

bool ximage::create(unsigned w, unsigned h, PIXEL_FORMAT fmt)
{
	if(fmt>=SUPPORTED_PIXEL_FORMAT || 0==w || 0==h)
		return false;
	if(m_bitmap)
		clear();
	m_width=w;
	m_height=h;
	m_pitch=image_pitch(w,image_bpp(fmt));
	m_bitmap=new uint8_t[m_pitch*h];
	return true;
}

void ximage::sub_image(ximage &img, unsigned begx, unsigned begy, unsigned w, unsigned h) const
{
	if(begx>=m_width || begy>=m_height) 
		return;
	if(w==0) w=m_width;
	if(h==0) h=m_height;
	if(begx+w>m_width) 
		w=m_width-begx;
	if(begy+h>m_height) 
		h=m_height-begy;
	if(!img.create(w,h,m_pxfmt)) 
		return;
	uint8_t *dst=img.m_bitmap;
	const uint8_t *src=m_bitmap+m_pitch*begy+4*begx;
	unsigned xsize=w*4;
	unsigned xfill=img.m_pitch-xsize;
	unsigned endy=begy+h;
	for(unsigned i=begy; i<endy; i+=1) {
		memcpy(dst,src,xsize);
		memset(dst+xsize,0,xfill);
		src+=m_pitch;
		dst+=img.m_pitch;
	}
}

bool ximage::save_as(const char *filename, unsigned option)
{
	if(!m_bitmap)
		return false;
	std::string ext=strrchr(filename,'.');
	wyc::to_lower(ext);
	FREE_IMAGE_FORMAT fif;
	if(ext==".png")
		fif=FIF_PNG;
	else if(ext==".jpg" || ext==".jpeg")
		fif=FIF_JPEG;
	else {
		wyc_warn("ximage::save_as: unknown image file format [%s]",ext.c_str());
		return false;
	}
	FIBITMAP *fi_bitmap;
	switch(m_pxfmt) {
	case RGBA_8888:
	case RGB_888:
		fi_bitmap=FreeImage_ConvertFromRawBits(m_bitmap,m_width,m_height,m_pitch,image_bpp(m_pxfmt),FI_RGBA_RED_MASK,FI_RGBA_GREEN_MASK,FI_RGBA_BLUE_MASK,TRUE);
		break;
	case LUMINANCE_8:
		fi_bitmap=FreeImage_ConvertFromRawBits(m_bitmap,m_width,m_height,m_pitch,8,0,0,0,FALSE);
		break;
	default:
		wyc_warn("ximage::save_as: pixel format un-support [%d]",m_pxfmt);
		return false;
	}
	bool ok=FreeImage_Save(fif,fi_bitmap,filename,option)?true:false;
	FreeImage_Unload(fi_bitmap);
	return ok;
}

}; // namespace wyc

