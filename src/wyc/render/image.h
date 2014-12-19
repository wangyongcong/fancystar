#ifndef __HEADER_WYC_XIMAGE
#define __HEADER_WYC_XIMAGE

#include "wyc/basedef.h"

namespace wyc
{

class ximage
{
public:
	enum PIXEL_FORMAT 
	{
		RGBA_8888=0,
		RGB_888,
		LUMINANCE_8,
		SUPPORTED_PIXEL_FORMAT,
		UNKNOWN_FORMAT=SUPPORTED_PIXEL_FORMAT,
	};
protected:
	uint8_t *m_bitmap;
	unsigned m_width, m_height;
	unsigned m_pitch;
	PIXEL_FORMAT m_pxfmt;
public:
	ximage();
	~ximage();
	uint8_t* detach_bitmap();
	bool create(unsigned w, unsigned h, PIXEL_FORMAT fmt=RGBA_8888);
	bool load(const char *filename, PIXEL_FORMAT fmt=RGBA_8888);
	void clear();
	inline unsigned width() const {
		return m_width;
	}
	inline unsigned height() const {
		return m_height;
	}
	inline unsigned pitch() const {
		return m_pitch;
	}
	inline bool empty() const {
		return m_bitmap==0;
	}
	inline uint8_t* bitmap() {
		return m_bitmap;
	}
	inline PIXEL_FORMAT pixel_format() const {
		return m_pxfmt;
	}
	void sub_image(ximage &img, unsigned begx, unsigned begy, unsigned w, unsigned h) const;
	bool save_as(const char *filename, unsigned option=0);
	static inline void free_bitmap(uint8_t *bm) {
		delete [] bm;
	}

	void distance_transform();
protected:
	bool load_rgba8888(uintptr_t handle, unsigned w, unsigned h, unsigned offx=0, unsigned offy=0, bool reverse=false);
	bool load_luminance8(uintptr_t handle, unsigned w, unsigned h, unsigned offx=0, unsigned offy=0);
};

}; // namespace wyc

#endif // end of __HEADER_WYC_XIMAGE
