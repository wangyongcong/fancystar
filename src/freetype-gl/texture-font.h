/* ============================================================================
 * Freetype GL - A C OpenGL Freetype engine
 * Platform:    Any
 * WWW:         http://code.google.com/p/freetype-gl/
 * ----------------------------------------------------------------------------
 * Copyright 2011,2012 Nicolas P. Rougier. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY NICOLAS P. ROUGIER ''AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL NICOLAS P. ROUGIER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of Nicolas P. Rougier.
 * ============================================================================
 */
#ifndef __TEXTURE_FONT_H__
#define __TEXTURE_FONT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "vector.h"
#include "texture-atlas.h"

/**
 * A structure that hold a kerning value relatively to a charcode.
 *
 * This structure cannot be used alone since the (necessary) right charcode is
 * implicitely held by the owner of this structure.
 */
typedef struct
{
    /**
     * Left character code in the kern pair.
     */
    wchar_t charcode;
    
    /**
     * Kerning value (in fractional pixels).
     */
    float kerning;

} kerning_t;

/**
 * A structure that hold texture info (such as texture id and texture coordinates) of the glyph bitmap.
 *
 */
typedef struct 
{
    /**
     * Atlas id (for whitch atlas it belongs)
     */
	uintptr_t atlas_id;

    /**
     * Bitmap's width in pixels.
     */
	unsigned short width;

    /**
     * Bitmap's height in pixels.
     */
	unsigned short height;

    /**
     * Bitmap's left bearing expressed in pixels.
     */
	short offset_x;

    /**
     * Bitmap's top bearing expressed in pixels.
     *
     * Remember that this is the distance from the baseline to the top-most
     * glyph scanline, upwards y coordinates being positive.
     */
	short offset_y;


    /**
     * First normalized texture coordinate (x) of top-left corner
     */
    float s0;

    /**
     * Second normalized texture coordinate (y) of top-left corner
     */
	float t0;

    /**
     * First normalized texture coordinate (x) of bottom-right corner
     */
	float s1;

    /**
     * Second normalized texture coordinate (y) of bottom-right corner
     */
	float t1;

} glyph_bitmap_t;

enum GLYPH_STYLE 
{
	GLYPH_NORMAL=0,
	GLYPH_OUTLINE,
	GLYPH_ITALIC,
	GLYPH_ITALIC_OUTLINE,
	GLYPH_EDT,
	GLYPH_STYLE_COUNT,
	GLYPH_STYLE_ALL = GLYPH_STYLE_COUNT,
};

/**
 *
 * A structure that describe a glyph.
 *
 *
 * Glyph metrics:
 * --------------
 *
 *                       xmin                     xmax
 *                        |                         |
 *                        |<-------- width -------->|
 *                        |                         |    
 *              |         +-------------------------+----------------- ymax
 *              |         |    ggggggggg   ggggg    |     ^        ^
 *              |         |   g:::::::::ggg::::g    |     |        | 
 *              |         |  g:::::::::::::::::g    |     |        | 
 *              |         | g::::::ggggg::::::gg    |     |        | 
 *              |         | g:::::g     g:::::g     |     |        | 
 *    offset_x -|-------->| g:::::g     g:::::g     |  offset_y    | 
 *              |         | g:::::g     g:::::g     |     |        | 
 *              |         | g::::::g    g:::::g     |     |        | 
 *              |         | g:::::::ggggg:::::g     |     |        |  
 *              |         |  g::::::::::::::::g     |     |      height
 *              |         |   gg::::::::::::::g     |     |        | 
 *  baseline ---*---------|---- gggggggg::::::g-----*--------      |
 *            / |         |             g:::::g     |              | 
 *     origin   |         | gggggg      g:::::g     |              | 
 *              |         | g:::::gg   gg:::::g     |              | 
 *              |         |  g::::::ggg:::::::g     |              | 
 *              |         |   gg:::::::::::::g      |              | 
 *              |         |     ggg::::::ggg        |              | 
 *              |         |         gggggg          |              v
 *              |         +-------------------------+----------------- ymin
 *              |                                   |
 *              |------------- advance_x ---------->|
 */
typedef struct
{
    /**
     * Wide character this glyph represents
     */
	wchar_t charcode;

	/**
	 * Glyph's width.
	 */
	float width;

	/**
	 * Glyph's height.
	 */
	float height;

	/**
	 * Glyph's bearing X.
	 */
	float offset_x;

	/**
	 * Glyph's bearing Y.
	 */
	float offset_y;
	
	/**
     * For horizontal text layouts, this is the horizontal distance (in
     * fractional pixels) used to increment the pen position when the glyph is
     * drawn as part of a string of text.
     */
    float advance_x;

    /**
     * For vertical text layouts, this is the vertical distance (in fractional
     * pixels) used to increment the pen position when the glyph is drawn as
     * part of a string of text.
     */
    float advance_y;

	glyph_bitmap_t *bitmap[GLYPH_STYLE_COUNT];

} texture_glyph_t;

/**
 * create an empty glyph
 */
  texture_glyph_t *
  texture_glyph_new( void );


/**
 * free a glyph
 */
  void
  texture_glyph_delete( texture_glyph_t *self );


/**
 *  Texture font structure.
 */
typedef struct
{
	/**
	 * Holding the internal Freetype data
	 */
	void * internal_handle;

	/**
	 * Atlas structure to store glyphs data.
	 */
	texture_atlas_t * atlas;

	/**
	 * Font filename
	 */
	char * filename;

	/**
	 * Font size (in points)
	 */
	float size;

	/**
	 * This field is simply used to compute a default line spacing (i.e., the
	 * baseline-to-baseline distance) when writing text with this font. Note
	 * that it usually is larger than the sum of the ascender and descender
	 * taken as absolute values. There is also no guarantee that no glyphs
	 * extend above or below subsequent baselines when using this distance.
	 */
	float height;

	/**
	 * The ascender is the vertical distance from the horizontal baseline to
	 * the highest 'character' coordinate in a font face. Unfortunately, font
	 * formats define the ascender differently. For some, it represents the
	 * ascent of all capital latin characters (without accents), for others it
	 * is the ascent of the highest accented character, and finally, other
	 * formats define it as being equal to bbox.yMax.
	 */
	float ascender;

	/**
	 * The descender is the vertical distance from the horizontal baseline to
	 * the lowest 'character' coordinate in a font face. Unfortunately, font
	 * formats define the descender differently. For some, it represents the
	 * descent of all capital latin characters (without accents), for others it
	 * is the ascent of the lowest accented character, and finally, other
	 * formats define it as being equal to bbox.yMin. This field is negative
	 * for values below the baseline.
	 */
	float descender;

	/**
	 * Outline thickness
	 */
	float outline_thickness;

	/**
	 * The position of the underline line for this face. It is the center of
	 * the underlining stem. Only relevant for scalable formats.
	 */
	float underline_position;

	/**
	 * The thickness of the underline for this face. Only relevant for scalable
	 * formats.
	 */
	float underline_thickness;

	/**
	 * default background glyph
	 */
	texture_glyph_t *background_glyph;

	/**
	 * internal handle to render glyph
	 */
	void *renderer_handle;

} texture_font_t;

enum TEXTURE_FONT_FLAGS
{
	// texture font error code
	TEXTURE_FONT_NO_ERROR = 0,
	TEXTURE_FONT_ERROR = 1,
	TEXTURE_FONT_ATLAS_NOT_ENOUGH,
	
	// texture font render technique
	TEXTURE_FONT_RENDER_NORMAL = 1,
	TEXTURE_FONT_RENDER_ITALIC = 2,
	TEXTURE_FONT_RENDER_OUTLINE= 4,
	TEXTURE_FONT_RENDER_ALL_STYLE = 0xF,
	TEXTURE_FONT_RENDER_EDT = 0x10,
};

typedef int (*glyph_generator_t) (texture_font_t*, wchar_t, texture_glyph_t*);

/**
 * This function creates a new texture font from given filename and size.  The
 * texture atlas is used to store glyph on demand. Note the depth of the atlas
 * will determine if the font is rendered as alpha channel only (depth = 1) or
 * RGB (depth = 3) that correspond to subpixel rendering (if available on your
 * freetype implementation).
 *
 * @param atlas     A texture atlas
 * @param filename  A font filename
 * @param size      Size of font to be created (in points)
 *
 * @return A new empty font (no glyph inside yet)
 *
 */
  texture_font_t *
  texture_font_new( texture_atlas_t * atlas,
                    const char * filename,
                    const float size );


/**
 * Delete a texture font. Note that this does not delete the glyph from the
 * texture atlas.
 *
 * @param self a valid texture font
 */
  void
  texture_font_delete( texture_font_t * self );

/**
 * Get the kerning between two horizontal glyphs.
 *
 * @param self      a valid texture font
 * @param charcode  codepoint of the peceding glyph
 * 
 * @return x kerning value
 */
float texture_font_get_kerning( texture_font_t *self, wchar_t left_char, wchar_t right_char );

/**
 * Generate a glyph
 */
bool texture_font_load_glyph ( texture_font_t *self, wchar_t charcode, texture_glyph_t *glyph, int render_technique );

/**
 * Render a glyph. The glyph should be loaded first with texture_font_load_glyph.
 */
bool texture_font_render_glyph ( texture_font_t *self, texture_glyph_t *glyph, int style );

#ifdef __cplusplus
}
#endif

#endif /* __TEXTURE_FONT_H__ */

