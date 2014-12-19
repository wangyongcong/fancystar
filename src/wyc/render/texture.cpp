#include "fscorepch.h"
#include "wyc/util/strutil.h"
#include "wyc/render/texture.h"
#include "wyc/util/fjson.h"

EXPORT_MODULE(texture)

namespace wyc
{

REG_RTTI(xtexture, xresbase)

inline void get_texture_format(ximage::PIXEL_FORMAT imgfmt, GLenum &dstfmt, GLenum &srcfmt) 
{
	static const GLenum ls_internal_format[ximage::SUPPORTED_PIXEL_FORMAT]={
		GL_RGBA,		// RGBA_8888
		GL_RGB,			// RGB_888
		GL_LUMINANCE,	// LUMINANCE_8
	};
	static const GLenum ls_source_format[ximage::SUPPORTED_PIXEL_FORMAT]={
		GL_RGBA,		// RGBA_8888
		GL_RGB,			// RGB_888
		GL_LUMINANCE,	// LUMINANCE_8
	};
	dstfmt=ls_internal_format[imgfmt];
	srcfmt=ls_source_format[imgfmt];
}

xtexture::xtexture() : xresbase()
{
	m_texid=0;
	m_imgw=m_imgh=0;
	m_texw=m_texh=0;
	m_wrap_s=m_wrap_t=GL_CLAMP_TO_EDGE;
	m_bitmap=0;
	m_pxfmt=ximage::UNKNOWN_FORMAT;
	m_imgset=0;
}

bool xtexture::load(const char *res_name)
{
	if(!async_load(res_name))
		return false;
	on_async_complete();
	return true;
}

bool xtexture::async_load(const char *res_name)
{
	if(m_texid) 
		unload();
	// is it a default texture ?
	if(starts_with(res_name,"fs_") && create_default_pattern(res_name+3)) {
		// try to create default texture first
		m_texname=res_name;
		return true;
	}
	// load it from file
	const char *ext=strrchr(res_name,'.');
	if(ext && 0==strcmp(ext,".json"))
	{
		// it's a imageset decscribed in JSON
		ximageset *imgset=create_imageset(res_name);
		if(!imgset)
			return false;
		if(!create_from_file(imgset->file_name())) {
			delete imgset;
			return false;
		}
		imgset->reset_texcoord(m_texw, m_texh);
		m_imgset=imgset;
	}
	// it's an image file
	// TODO: only support a minimal subset of formats (.tga, .png, etc.)
	else if(!create_from_file(res_name)) 
	{
		return false;
	}
	m_texname=res_name;
	return true;
}

void xtexture::unload()
{
	if(m_texid) {
		glDeleteTextures(1,&m_texid);
		m_texid=0;
	}
	m_texname.clear();
	m_imgw=m_imgh=0;
	m_texw=m_texh=0;
	if(m_bitmap) {
		if(BITMAP_IMAGE==m_bitmap_type) 
			ximage::free_bitmap(m_bitmap);
		else if(BITMAP_MEMORY==m_bitmap_type)
			delete [] m_bitmap;
		m_bitmap=0;
	}
	m_pxfmt=ximage::UNKNOWN_FORMAT;
	if(m_imgset) {
		delete m_imgset;
	}
}

void xtexture::on_async_complete()
{
	assert(!m_texid);
	assert(m_bitmap);
	glGenTextures(1,&m_texid);
	if(!m_texid) 
	{
		wyc_error("xtexture::on_async_complete: OpenGL failed to alloc texture");
		return;
	}
	glBindTexture(GL_TEXTURE_2D,m_texid);
	GLenum dstfmt, srcfmt;
	get_texture_format(ximage::PIXEL_FORMAT(m_pxfmt),dstfmt,srcfmt);
	if(BITMAP_SMALL==m_bitmap_type) 
	{
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,m_texw,m_texh,0,GL_RGB,GL_UNSIGNED_BYTE,m_small_bitmap);
	}
	else {
		if(m_imgw==m_texw && m_imgh==m_texh) {
			glTexImage2D(GL_TEXTURE_2D,0,dstfmt,m_texw,m_texh,0,srcfmt,GL_UNSIGNED_BYTE,m_bitmap);
		}
		else {
			glTexImage2D(GL_TEXTURE_2D,0,dstfmt,m_texw,m_texh,0,srcfmt,GL_UNSIGNED_BYTE,0);
			glTexSubImage2D(GL_TEXTURE_2D,0,0,0,m_imgw,m_imgh,srcfmt,GL_UNSIGNED_BYTE,m_bitmap);
		}
		if(BITMAP_IMAGE==m_bitmap_type) 
			ximage::free_bitmap(m_bitmap);
		else if(BITMAP_MEMORY==m_bitmap_type)
			delete [] m_bitmap;
	}
	m_bitmap=0;
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,m_wrap_s);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,m_wrap_t);
}

bool xtexture::create_texture(ximage *img)
{
	assert(img);
	assert(!m_texid);
	glGenTextures(1,&m_texid);
	if(m_texid==0)
		return false;
	m_wrap_s=m_wrap_t=GL_CLAMP_TO_EDGE;
	m_imgw=img->width(), m_imgh=img->height();
	bool size_adjusted=false;
	if((m_imgw&(m_imgw-1))!=0) {
		m_texw=power2(m_imgw);
		size_adjusted=true;
	} 
	else m_texw=m_imgw;
	if((m_imgh&(m_imgh-1))!=0) {
		m_texh=power2(m_imgh);
		size_adjusted=true;
	}
	else m_texh=m_imgh;
	glBindTexture(GL_TEXTURE_2D,m_texid);
	GLenum dstfmt, srcfmt;
	get_texture_format(img->pixel_format(),dstfmt,srcfmt);
	if(size_adjusted) {
		glTexImage2D(GL_TEXTURE_2D,0,dstfmt,m_texw,m_texh,0,srcfmt,GL_UNSIGNED_BYTE,0);
		glTexSubImage2D(GL_TEXTURE_2D,0,0,0,m_imgw,m_imgh,GL_RGBA,GL_UNSIGNED_BYTE,img->bitmap());
	}
	else {
		glTexImage2D(GL_TEXTURE_2D,0,dstfmt,m_texw,m_texh,0,srcfmt,GL_UNSIGNED_BYTE,img->bitmap());
	}
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,m_wrap_s);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,m_wrap_t);
	return true;
}

void xtexture::set_wrap_mode(unsigned wrap_s, unsigned wrap_t)
{
	if(m_wrap_s==wrap_s && m_wrap_t==wrap_t)
		return;
	m_wrap_s=wrap_s;
	m_wrap_t=wrap_t;
	if(!m_texid) 
		return;
	GLuint texid;
	glGetIntegerv(GL_TEXTURE_BINDING_2D,(GLint*)&texid);
	if(texid!=m_texid) 
		glBindTexture(GL_TEXTURE_2D,m_texid);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,wrap_s);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,wrap_t);
	if(texid!=m_texid)
		glBindTexture(GL_TEXTURE_2D,texid);
}

bool xtexture::create_from_file(const char *pfile)
{
	ximage img;
	if(!img.load(pfile))
		return false;
	m_imgw=img.width(), m_imgh=img.height();
	bool size_adjusted=false;
	if((m_imgw&(m_imgw-1))!=0) {
		m_texw=power2(m_imgw);
		size_adjusted=true;
	} 
	else m_texw=m_imgw;
	if((m_imgh&(m_imgh-1))!=0) {
		m_texh=power2(m_imgh);
		size_adjusted=true;
	}
	else m_texh=m_imgh;
	if(size_adjusted) {
		wyc_warn("image size isn't power of 2 (memory may waste): %s (%dx%d)",pfile,m_imgw,m_imgh);
	}
	m_wrap_s=m_wrap_t=GL_CLAMP_TO_EDGE;
	assert(ximage::SUPPORTED_PIXEL_FORMAT<0xFFFF);
	m_pxfmt=(uint16_t)img.pixel_format();
	m_bitmap=img.detach_bitmap();
	m_bitmap_type=BITMAP_IMAGE;
	return true;
}

ximageset* xtexture::create_imageset(const char *pfile)
{
	xjson json;
	if(!json.load_file(pfile))
		return 0;
	const vjson::json_value *root=json.get_root();
	if(!root || !root->type==vjson::JSON_OBJECT)
		return 0;
	unsigned complete=0;
	ximageset *imgset=new ximageset;
	const vjson::json_value *img_array=0;
	unsigned img_count=0;
	for(const vjson::json_value *iter=root->first_child; iter; iter=iter->next_sibling)
	{
		if(0==strcmp(iter->name,"image_file")) {
			assert(iter->type==vjson::JSON_STRING);
			imgset->m_texfile=iter->string_value;
			complete+=1;
		}
		else if(0==strcmp(iter->name,"image_count")) {
			assert(iter->type==vjson::JSON_INT);
			img_count=iter->int_value;
			complete+=1;
		}
		else if(0==strcmp(iter->name,"images")) {
			assert(iter->type==vjson::JSON_ARRAY);
			img_array=iter;
			complete+=1;
		}
	}
	if(complete<3) 
	{
		delete imgset;
		return 0;
	}
//	adjust_texture_size(img_width,img_height);
	if(img_array && img_count>0) {
		imgset->m_subimg=new xdict(img_count);
		ximageset::subimage_t *img;
		const char *img_name;
		for(const vjson::json_value *iter=img_array->first_child; iter; iter=iter->next_sibling)
		{
			assert(iter->type==vjson::JSON_OBJECT);
			img=new ximageset::subimage_t;
			img_name=0;
			complete=0;
			for(vjson::json_value *attr=iter->first_child; attr; attr=attr->next_sibling)
			{
				if(0==strcmp(attr->name,"name")) {
					assert(attr->type==vjson::JSON_STRING);
					img_name = attr->string_value;
					img->m_name=img_name;
					img->m_id=strhash(img_name);
					complete+=1;
				}
				else if(0==strcmp(attr->name,"xpos")) {
					assert(attr->type==vjson::JSON_INT);
					img->m_xpos=attr->int_value;
					complete+=1;
				}
				else if(0==strcmp(attr->name,"ypos")) {
					assert(attr->type==vjson::JSON_INT);
					img->m_ypos=attr->int_value;
					complete+=1;
				}
				else if(0==strcmp(attr->name,"width")) {
					assert(attr->type==vjson::JSON_INT);
					img->m_width=attr->int_value;
					complete+=1;
				}
				else if(0==strcmp(attr->name,"height")) {
					assert(attr->type==vjson::JSON_INT);
					img->m_height=attr->int_value;
					complete+=1;
				}
			}
			if(complete<5)
			{
				// non-complete infomation
				delete img;
				wyc_warn("xtexture::create_imageset: invalid sub image [%s]",img_name?img_name:"NO NAME");
				continue;
			}
			imgset->m_subimg->add(img->m_id,xdict::value_t(img));
		}
	}
	return imgset;
}

bool xtexture::create_default_pattern(const char *name)
{
	if(0==strcmp(name,"chess_board")) {
		uint8_t *bitmap=new uint8_t[16*16*3];
		uint8_t *end=texel_chess_board(bitmap,16,0xFF,0xCC);
		assert(bitmap+(16*16*3)==end);
		m_wrap_s=m_wrap_t=GL_REPEAT;
		m_imgw=m_imgh=16;
		m_texw=m_texh=16;
		m_pxfmt=ximage::RGB_888;
		m_bitmap=bitmap;
		m_bitmap_type=BITMAP_MEMORY;
	}
	else if(0==strcmp(name,"blank")) {
		*(uint32_t*)m_small_bitmap=0xFFFFFF;
		m_wrap_s=m_wrap_t=GL_REPEAT;
		m_imgw=m_imgh=1;
		m_texw=m_texh=1;
		m_pxfmt=ximage::RGB_888;
		m_bitmap_type=BITMAP_SMALL;
	}
	else {
		wyc_error("xtexture: unknown default texture name [%s]",name);
		return false;
	}
	return true;
}

uint8_t* xtexture::texel_chess_board(uint8_t *bitmap, unsigned size, uint8_t w, uint8_t b)
{
	assert((size&(size-1))==0);
	unsigned half_row=size>>1;
	unsigned half=half_row*3, pitch=size*3;
	uint8_t *dst=bitmap, *src=bitmap;
	memset(dst,w,half);
	memset(dst+half,b,half);
	dst+=pitch;
	for(unsigned i=1; i<half_row; ++i, dst+=pitch)
		memcpy(dst,src,pitch);
	src=dst;
	memset(dst,b,half);
	memset(dst+half,w,half);
	dst+=pitch;
	for(unsigned i=1; i<half_row; ++i, dst+=pitch)
		memcpy(dst,src,pitch);
	return dst;
}

//-------------------------------------------------------------------
// class ximageset
//-------------------------------------------------------------------

ximageset::ximageset() 
{
	m_subimg=0;
}

ximageset::~ximageset()
{
	if(!m_subimg) return;
	for(xdict::iterator iter=m_subimg->begin(), end=m_subimg->end();
		iter!=end; ++iter)
		delete (subimage_t*)(iter->second);
	delete m_subimg;
}

void ximageset::reset_texcoord(unsigned texture_width, unsigned texture_height)
{
	if(!m_subimg) return;
	subimage_t* img;
	for(xdict::iterator iter=m_subimg->begin(), end=m_subimg->end();
		iter!=end; ++iter) {
			img=(subimage_t*)iter->second;
			img->m_ypos=texture_height-img->m_ypos-img->m_height;
			img->m_s0=float(img->m_xpos)/texture_width;
			img->m_t0=float(img->m_ypos)/texture_height;
			img->m_s1=float(img->m_xpos+img->m_width)/texture_width;
			img->m_t1=float(img->m_ypos+img->m_height)/texture_height;
	}
}

}; // namespace wyc
