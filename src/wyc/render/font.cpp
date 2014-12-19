#include "fscorepch.h"
#include "wyc/util/strutil.h"
#include "wyc/render/font.h"
#include "wyc/obj/scheduler.h"
#include "wyc/render/edtaa3func.h"

EXPORT_MODULE(font)

#define DEFAULT_GLYPH_CACHE \
	(L" !\"#$%&'()*+,-./0123456789:;<=>?"\
		L"@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"\
		L"`abcdefghijklmnopqrstuvwxyz{|}~")

#define DEFAULT_DICT_SIZE 256
#define DEFAULT_ATLAS_SIZE 512
#define DEFAULT_GLYPH_BUCKET_SIZE 128

#define FLOAT_TO_POINTER(val) (*(void**)(&val))
#define POINTER_TO_FLOAT(ptr) (*(float*)(&ptr))

namespace wyc
{

REG_RTTI(xfont,xresbase)

unsigned generate_font_id (const char *font_name, float size)
{
	char compose_name[64];
	sprintf_s(compose_name, 63, "%s%f", font_name, size);
	compose_name[63]=0;
	return strhash(compose_name);
}

xfont::xfont()
{
	m_internal_font=0;
	m_task_pool=0;
	m_dict=0;
	m_bucket_used=DEFAULT_GLYPH_BUCKET_SIZE;
}

bool xfont::load(const char *font_name)
{
	return async_load(font_name);
}

bool xfont::async_load(const char *font_name)
{
	assert(font_name);
	wyc::xjson json;
	if(!json.load_file(font_name)) {
		wyc_error("xfont::load: invalid font settings");
		return false;
	}
	std::string file = json.get("file","");
	float point_size = json.get("size",0.0f);
	if(file.empty()) 
	{
		wyc_error("xfont::load: unknown font file");
		return false;
	}
	if(point_size<=0)
	{
		// no font size
		wyc_error("xfont::load: unknown font size");
		return false;
	}
	else if(point_size>128)
	{
		// too big
		wyc_error("xfont::load: font size is too large");
		return false;
	}
	int atlas_size = json.get("texture_size",0);
	if(atlas_size<=0)
		atlas_size=DEFAULT_ATLAS_SIZE;
	// TODO: calculate pixel size and texture size according point size and screen resolution
	texture_atlas_t *atlas=texture_atlas_new(atlas_size,atlas_size,1);
	texture_font_t *tf=texture_font_new(atlas,file.c_str(),point_size);
	if(!tf)
	{
		texture_atlas_delete(atlas);
		wyc_error("xfont::load: failed to load font");
		return false;
	}
	int technique = TEXTURE_FONT_RENDER_NORMAL;
//	if(json.get<bool>("edt"))
//		tf->technique|=TEXTURE_FONT_DISTANCE_TRANSFORM;
//	if(json.get<bool>("hinting"))
//		tf->technique|=TEXTURE_FONT_HINTING;
	float thickness = json.get("outline_thickness",0.0f);
	tf->outline_thickness = thickness;
	if(json.get("outline",false)) 
		technique|=TEXTURE_FONT_RENDER_OUTLINE;	
	if(json.get("italic",false))
		technique|=TEXTURE_FONT_RENDER_ITALIC;
	// generate cache
	size_t dict_size = json.get("dict_size",DEFAULT_DICT_SIZE);
	reserve_dict(dict_size);
	m_bucket_used=DEFAULT_GLYPH_BUCKET_SIZE;
	std::wstring pre_load=json.get("cache",L"");
	unsigned miss=0;
	if(!pre_load.empty())
	{
		size_t count = pre_load.size();
		texture_glyph_t *buffer = (texture_glyph_t*)malloc(sizeof(texture_glyph_t)*count), *glyph;
		memset(buffer,0,sizeof(texture_glyph_t)*count);
		m_glyph_buffers.push_back(buffer);
		for(size_t i=0; i<count; ++i)
		{
			glyph = buffer+i;
			if(!texture_font_load_glyph(tf,pre_load[i],glyph,technique))
				miss+=1;
			glyph_dict_set (m_dict, glyph->charcode, glyph);
		}
		if(miss) wyc_warn("xfont::load: %d glyphs are not found",miss);
	}
	// generate kernings for alphabet
	float k;
	for(wchar_t left = L'a'; left<='z'; left+=1)
	{
		for(wchar_t right = L'a'; right<='z'; right+=1)
		{
			k = texture_font_get_kerning(tf,left,right);
			if(k!=0) 
				m_kerning.add((left<<16)|right, FLOAT_TO_POINTER(k));
		}
	}
	for(wchar_t left = L'A'; left<='Z'; left+=1)
	{
		for(wchar_t right = L'A'; right<='Z'; right+=1)
		{
			k = texture_font_get_kerning(tf,left,right);
			if(k!=0) 
				m_kerning.add((left<<16)|right, FLOAT_TO_POINTER(k));
		}
	}
	m_internal_font=tf;
	return true;
}

class xtexture_font_holder : public xobject
{
	texture_font_t *m_tf;
public:
	xtexture_font_holder()
	{
		m_tf=0;
	}
	virtual ~xtexture_font_holder()
	{
		if(m_tf) 
		{
			texture_font_delete(m_tf);
		}
	}
	void attach(texture_font_t *tf)
	{
		m_tf=tf;
	}
	texture_font_t* detach() 
	{
		texture_font_t *tf = m_tf;
		m_tf=0;
		return tf;
	}
};

void xfont::unload()
{
	// free texturen font and atlas
	if(m_internal_font) 
	{
		texture_atlas_t *atlas;
		while(m_internal_font->atlas) {
			atlas=m_internal_font->atlas;
			m_internal_font->atlas = m_internal_font->atlas->next;
			texture_atlas_delete(atlas);
		}
	
		// delay texture font destruction (why?)
		/*
		xtexture_font_holder *holder = wycnew xtexture_font_holder;
		holder->attach(m_internal_font);
		holder->delthis();
		*/
		texture_font_delete(m_internal_font);
		m_internal_font=0;
	}

	// free all glyphs
	if(m_dict) {
		glyph_dict_delete( m_dict );
		m_dict=0;
	}
	for(size_t i=0, count=m_glyph_buffers.size(); i<count; ++i)
		free(m_glyph_buffers[i]);
	m_glyph_buffers.clear();

	// free all tasks
	xfont_task *ta;
	while(m_task_pool) 
	{
		ta=m_task_pool;
		m_task_pool=m_task_pool->m_next;
		ta->decref();
	}

	// reset status
	m_bucket_used = DEFAULT_GLYPH_BUCKET_SIZE;
}

void xfont::reserve_dict(size_t capacity)
{
	if(m_dict)
	{
		glyph_dict_reserve(m_dict,capacity);
	}
	else
	{
		m_dict = glyph_dict_new(capacity);
	}
}

void xfont::upload_texture()
{
	// TODO: upload the modified region only
	texture_atlas_t *atlas=m_internal_font->atlas;
	while(atlas && atlas->dirty) {
		texture_atlas_upload(atlas);
		atlas=atlas->next;
	}
}

float xfont::kerning(wchar_t left_char, wchar_t right_char)
{
	unsigned key = (left_char<<16)|right_char;
	xdict::value_t val = m_kerning.get(key);
	return val ? POINTER_TO_FLOAT(val) : 0;
}

xfont::xfont_task* xfont::new_task()
{
	xfont_task *ta;
	if(m_task_pool) {
		ta=m_task_pool;
		m_task_pool=m_task_pool->m_next;
	}
	else {
		ta=wycnew xfont_task;
		ta->incref();
		ta->m_buffer=0;
		ta->m_size=0;
	}
	ta->m_next=0;
	ta->m_font=this;
	return ta;
}

void xfont::del_task(xfont_task *ta)
{
	ta->m_buffer=0;
	ta->m_size=0;
	ta->m_font=0;
	ta->m_next=m_task_pool;
	m_task_pool=ta;
}

texture_glyph_t* xfont::new_glyph(wchar_t ch)
{
	texture_glyph_t *glyph;
	if(DEFAULT_GLYPH_BUCKET_SIZE==m_bucket_used)
	{
		glyph = (texture_glyph_t*)malloc(sizeof(texture_glyph_t)*DEFAULT_GLYPH_BUCKET_SIZE);
		memset(glyph,0,sizeof(texture_glyph_t)*DEFAULT_GLYPH_BUCKET_SIZE);
		m_glyph_buffers.push_back(glyph);
		m_bucket_used=0;
	}
	else 
	{
		assert(m_glyph_buffers.size());
		glyph=(texture_glyph_t*)m_glyph_buffers.back();
		glyph+=m_bucket_used;
	}
	assert(glyph);
	assert(m_bucket_used<DEFAULT_GLYPH_BUCKET_SIZE);
	++m_bucket_used;
	glyph->charcode = ch;
	glyph_dict_set(m_dict,ch,glyph);
	return glyph;
}

texture_glyph_t* xfont::new_glyph_buffer(const wchar_t *chars, size_t sz, size_t &pos, size_t &count)
{
	texture_glyph_t *glyph, *buffer;
	if(DEFAULT_GLYPH_BUCKET_SIZE==m_bucket_used)
	{
		buffer = (texture_glyph_t*)malloc(sizeof(texture_glyph_t)*DEFAULT_GLYPH_BUCKET_SIZE);
		memset(buffer,0,sizeof(texture_glyph_t)*DEFAULT_GLYPH_BUCKET_SIZE);
		m_glyph_buffers.push_back(buffer);
		m_bucket_used=0;
	}
	else 
	{
		assert(m_glyph_buffers.size());
		buffer=(texture_glyph_t*)m_glyph_buffers.back();
		buffer+=m_bucket_used;
	}
	assert(buffer);
	assert(m_bucket_used<DEFAULT_GLYPH_BUCKET_SIZE);
	size_t i=pos;
	glyph=buffer;
	for(; i<sz && m_bucket_used<DEFAULT_GLYPH_BUCKET_SIZE; ++i, ++m_bucket_used, ++glyph)
	{
		glyph->charcode = chars[i];
		glyph_dict_set (m_dict,glyph->charcode,glyph);
	}
	count=i-pos;
	pos=i;
	return buffer;
}

unsigned xfont::async_load_glyphs(const wchar_t *chars, size_t sz, int style)
{
	assert(chars);
	assert(style<GLYPH_STYLE_COUNT);
	if(!is_complete())
	{
		wyc_warn("[xfont::async_load_glyphs] The font is not complete.");
		return 0;
	}
	if(0==sz) {
		sz = wcslen(chars);
		if(!sz) return 0;
	}
	size_t loading=0, pos=0, count=0;
	texture_glyph_t* glyph, **buffer=0;
	xfont_task *ta;
	for(const wchar_t *ch = chars; *ch; ++ch)
	{
		glyph=glyph_dict_get(m_dict,*ch);
		if(!glyph) {
			glyph = new_glyph(*ch);
			if(!texture_font_load_glyph(m_internal_font,glyph->charcode,glyph,0)) {
				wyc_warn("[xfont::async_load_glyphs] glyph [0x%x] is not contained",glyph->charcode);
				continue;
			}
		}
		if(glyph->bitmap[style]) 
			continue;
		if(pos==count) {
			if(pos) {
				// buffer is full, send it
				assert(buffer);
				ta = new_task();
				ta->m_buffer = buffer;
				ta->m_size = pos;
				ta->m_style = style;
				send_async_task((worker_function_t)&_async_render_glyph, ta, (worker_callback_t)&_async_render_glyph_callback);
				loading += pos;
			}
			count = chars+sz-ch;
			if(count>256)
				count=256;
			buffer = (texture_glyph_t**)malloc(sizeof(void*)*count);
			pos = 0;
		}
		buffer[pos++]=glyph;
	}
	if(pos) {
		assert(buffer);
		ta = new_task();
		ta->m_buffer = buffer;
		ta->m_size = pos;
		ta->m_style = style;
		send_async_task((worker_function_t)&_async_render_glyph, ta, (worker_callback_t)&_async_render_glyph_callback);
		loading += pos;
	}
	return loading;
}

void xfont::_async_render_glyph(xfont_task *task)
{
	assert(task);
	xfont_task *ft=(xfont_task*)task;
	xfont *self=ft->m_font;
	texture_glyph_t *glyph;
	int technique;
	switch(ft->m_style)
	{
	case GLYPH_NORMAL:
		technique=TEXTURE_FONT_RENDER_NORMAL;
		break;
	case GLYPH_OUTLINE:
		technique=TEXTURE_FONT_RENDER_NORMAL|TEXTURE_FONT_RENDER_OUTLINE;
		break;
	case GLYPH_ITALIC:
		technique=TEXTURE_FONT_RENDER_ITALIC;
		break;
	case GLYPH_ITALIC_OUTLINE:
		technique=TEXTURE_FONT_RENDER_ITALIC|TEXTURE_FONT_RENDER_OUTLINE;
		break;
	case GLYPH_STYLE_ALL:
		technique=TEXTURE_FONT_RENDER_NORMAL|TEXTURE_FONT_RENDER_ITALIC|TEXTURE_FONT_RENDER_OUTLINE;
		break;
	default:
		// never arrive here!
		assert(0);
	}
	for(size_t i=0; i<ft->m_size; ++i)
	{
		glyph=ft->m_buffer[i];
		texture_font_render_glyph(self->m_internal_font,glyph,technique);
	}
}

void xfont::_async_render_glyph_callback(xfont_task *task)
{
	assert(task);
	xfont_task *ft=(xfont_task*)task;
	xfont *self=ft->m_font;
	free(ft->m_buffer);
	self->del_task(ft);
}

}; // namespace wyc

