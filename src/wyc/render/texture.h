#ifndef __HEADER_WYC_XTEXTURE
#define __HEADER_WYC_XTEXTURE

#include "wyc/obj/resbase.h"
#include "wyc/render/image.h"

namespace wyc
{

class ximageset;

class xtexture : public xresbase
{
	USE_RTTI;
	std::string m_texname;
	GLuint m_texid;
	GLenum m_wrap_s, m_wrap_t;
	unsigned m_texw, m_texh;
	unsigned m_imgw, m_imgh;
	// async context
	union {
		uint8_t *m_bitmap;
		uint8_t m_small_bitmap[sizeof(void*)];
	};
	uint16_t m_pxfmt;
	enum {
		BITMAP_IMAGE=0,
		BITMAP_MEMORY,
		BITMAP_SMALL,
	};
	uint16_t m_bitmap_type;
	// texture profile
	ximageset *m_imgset;
public:
	xtexture();
	virtual bool load(const char *res_name);
	virtual void unload();
	virtual bool async_load(const char *res_name);
	virtual void on_async_complete();
	inline GLuint handle() const {
		return m_texid;
	}
	inline unsigned width() const {
		return m_texw;
	}
	inline unsigned height() const {
		return m_texh;
	}
	inline unsigned image_width() const {
		return m_imgw;
	}
	inline unsigned image_height() const {
		return m_imgh;
	}
	inline void get_wrap_mode(unsigned &wrap_s, unsigned wrap_t) const {
		wrap_s=m_wrap_s, wrap_t=m_wrap_t;
	}
	void set_wrap_mode(unsigned wrap_s, unsigned wrap_t);
	inline const ximageset* get_imageset() const {
		return m_imgset;
	}
private:
	bool create_texture(ximage *img);
	bool create_from_file(const char *pfile);
	bool create_default_pattern(const char *name);
	ximageset* create_imageset(const char *pfile);
	static uint8_t* texel_chess_board(uint8_t *bitmap, unsigned size, uint8_t white=0xFF, uint8_t black=0);
};

class ximageset
{
	std::string m_texfile;
	xdict *m_subimg;
	friend class xtexture;
public:
	typedef struct
	{
		std::string m_name;
		unsigned m_id;
		unsigned m_xpos;
		unsigned m_ypos;
		unsigned m_width;
		unsigned m_height;
		float m_s0, m_t0, m_s1, m_t1;
	} subimage_t;
	class const_iterator : public xdict::const_iterator
	{
		friend class ximageset;
		const_iterator(const xdict::const_iterator &iter) : xdict::const_iterator(iter) {}
	public:
		const subimage_t& operator * () const {
			return *(subimage_t*)(m_pos->second);
		}
		const subimage_t* operator ->() const {
			return (subimage_t*)(m_pos->second);
		}
	};
	ximageset();
	~ximageset();
	inline const subimage_t* get_image(unsigned img_id) const {
		if(!m_subimg)
			return 0;
		subimage_t* v = (subimage_t*)m_subimg->get(img_id);
		return v;
	}
	inline const subimage_t* get_image(const char *name) const {
		return get_image(strhash(name));
	}
	inline const char* file_name() const {
		return m_texfile.c_str();
	}
	void reset_texcoord(unsigned texture_width, unsigned texture_height);

	const_iterator begin() const {
		return m_subimg->begin();
	}
	const_iterator end() const {
		return m_subimg->end();
	}
	size_t size() const {
		return m_subimg?m_subimg->size():0;
	}
};

}; // namespace wyc

#endif // __HEADER_WYC_XTEXTURE

