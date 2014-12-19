/* ===========================================================================
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
#include "fscorepch.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
// #include FT_ADVANCES_H
#include FT_LCD_FILTER_H
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <wchar.h>
#include "texture-font.h"
#include "wyc/render/edtaa3func.h"
#include "wyc/util/util.h"

#undef __FTERRORS_H__
#define FT_ERRORDEF( e, v, s )  { e, s },
#define FT_ERROR_START_LIST     {
#define FT_ERROR_END_LIST       { 0, 0 } };
#ifdef _DEBUG
const struct {
	int code;
	const char* message;
} FT_Errors[] =
#include FT_ERRORS_H
#define HANDLE_FT_ERROR(error) \
	fprintf(stderr, "FT_Error (code 0x%02x) : %s\n", FT_Errors[error].code, FT_Errors[error].message);
#else
#define HANDLE_FT_ERROR(error) wyc_warn("FT_Error: %d",error);
#endif //_DEBUG

#if defined(_WIN32) || defined(_WIN64)
#define round(v) (floor(v+0.5f))
#pragma warning (disable: 4245)
#endif // _WIN32 || _WIN64

#define SCALE_FIX_26_6  1.0f/64
#define SCALE_FIX_16_16 1.0f/65536

typedef struct
{
	FT_Library library;
	FT_Face face;
} internal_font_t;

static FT_Matrix  s_matrix_shearing = { 
	(int)((1.0)	* 0x10000L),
	(int)((tan(0.2618f))	* 0x10000L),
	(int)((0.0)	* 0x10000L),
	(int)((1.0)	* 0x10000L) 
};

// ------------------------------------------------- texture_font_load_face ---

int texture_font_load_face( FT_Library *library, FT_Face *face, const char *filename, const float size )
{
	assert( library );
	assert( filename );
	assert( size );

	size_t hres = 64;
	FT_Error error;

	FT_Matrix matrix = { 
		(int)((1.1)	* 0x10000L),
		(int)((0.0)	* 0x10000L),
		(int)((0.0)	* 0x10000L),
		(int)((1.0)	* 0x10000L) 
	};

	// Initialize library
	error = FT_Init_FreeType( library );
	if( error )
	{
		HANDLE_FT_ERROR(error);
		return 0;
	}

	// Load face
	error = FT_New_Face( *library, filename, 0, face );
	if( error )
	{
		FT_Done_FreeType( *library );
		HANDLE_FT_ERROR(error);
		return 0;
	}

	// Select charmap
	error = FT_Select_Charmap( *face, FT_ENCODING_UNICODE );
	if( error )
	{
		FT_Done_Face( *face );
		FT_Done_FreeType( *library );
		HANDLE_FT_ERROR(error);
		return 0;
	}

	// Set char size
	error = FT_Set_Char_Size( *face, (int)(size*64), 0, 96, 96 );
	if( error )
	{
		FT_Done_Face( *face );
		FT_Done_FreeType( *library );
		HANDLE_FT_ERROR(error);
		return 0;
	}

	return 1;
}

// ------------------------------------------------- texture_font_load_font ---

void* texture_font_load_font ( const char *font_file, float font_size)
{
	assert(font_file && font_file[0]);
	internal_font_t *font=(internal_font_t*)malloc(sizeof(internal_font_t));
	if( 0 == texture_font_load_face(&font->library,&font->face,font_file,font_size) ) {
		free(font);
		return 0;
	}
	return font;
}

// ------------------------------------------------- texture_font_unload_font ---

void texture_font_unload_font ( void *internal_handle )
{
	assert(internal_handle);
	internal_font_t *font=(internal_font_t*)internal_handle;
	FT_Done_Face(font->face);
	FT_Done_FreeType(font->library);
	free(font);
}

// ------------------------------------------------------ texture_glyph_new ---

texture_glyph_t * texture_glyph_new( void )
{
	texture_glyph_t *self = (texture_glyph_t *) malloc( sizeof(texture_glyph_t) );
	if( self == NULL)
	{
		fprintf( stderr, "%s, line %d: No more memory for allocating data\n", __FILE__, __LINE__ );
		return 0;
	}
	self->charcode = 0;
	self->advance_x = 0.0;
	self->advance_y = 0.0;
	memset(self->bitmap,0,sizeof(self->bitmap));
	return self;
}

// --------------------------------------------------- texture_glyph_delete ---

void texture_glyph_delete( texture_glyph_t *self )
{
	assert( self );
	for(int i=0; i<GLYPH_STYLE_COUNT; ++i) {
		if(self->bitmap[i])
			free(self->bitmap[i]);
	}
	free( self );
}

// ---------------------------------------------------- texture_font_generate_background_glyph ---

void texture_font_generate_background_glyph( texture_font_t *self)
{
	size_t width  = self->atlas->width;
	size_t height = self->atlas->height;
	ivec4 region = texture_atlas_get_region( self->atlas, 5, 5 );
	if ( region.x < 0 )
	{
		fprintf( stderr, "%s, line %d: Texture atlas is full\n",  __FILE__, __LINE__ );
		return;
	}
	texture_glyph_t * glyph = texture_glyph_new( );
	if( !glyph ) return;

	unsigned char data[4*4*3] = {
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	};
	texture_atlas_set_region( self->atlas, region.x, region.y, 4, 4, data, 0 );
	glyph->charcode = (wchar_t)(-1);
	glyph_bitmap_t *bm = (glyph_bitmap_t*)malloc(sizeof(glyph_bitmap_t));
	bm->atlas_id = (uintptr_t)self->atlas;
	bm->width = 1;
	bm->height= 1;
	bm->s0 = (region.x+2)/(float)width;
	bm->t0 = (region.y+2)/(float)height;
	bm->s1 = (region.x+3)/(float)width;
	bm->t1 = (region.y+3)/(float)height;
	glyph->bitmap[GLYPH_NORMAL]=bm;
	self->background_glyph = glyph;
}

// ------------------------------------------------------- texture_font_new ---

texture_font_t* texture_font_new( texture_atlas_t * atlas, const char * filename, const float size)
{
	assert( filename && filename[0] );
	assert( size>0 );

	texture_font_t *self = (texture_font_t *) malloc( sizeof(texture_font_t) );
	if( self == NULL)
	{
		fprintf( stderr, "line %d: No more memory for allocating data\n", __FILE__, __LINE__ );
		return 0;
	}
	atlas->ref += 1;
	self->atlas = atlas;
	self->height = 0;
	self->ascender = 0;
	self->descender = 0;
	self->filename = _strdup( filename );
	self->size = size;
	self->outline_thickness = 0.0;

	self->internal_handle = texture_font_load_font( filename, size );
	self->renderer_handle = 0;
	if(!self->internal_handle)
	{
		free(self->filename);
		free(self);
		return 0;
	}
	internal_font_t *internal_font=(internal_font_t*)self->internal_handle;

	// Get font metrics at high resolution
	FT_Size_Metrics *metrics = &internal_font->face->size->metrics; 
	float y_ppu  = float(metrics->y_ppem) / internal_font->face->units_per_EM, 
		x_ppu = float(metrics->x_ppem) / internal_font->face->units_per_EM;

	self->underline_position = (internal_font->face->underline_position * y_ppu) * SCALE_FIX_26_6;
	self->underline_position = round( self->underline_position );
	if( self->underline_position > -2 )
	{
		self->underline_position = -2.0f;
	}

	self->underline_thickness = (internal_font->face->underline_thickness * y_ppu) * SCALE_FIX_26_6;
	self->underline_thickness = round( self->underline_thickness );
	if( self->underline_thickness < 1 )
	{
		self->underline_thickness = 1.0f;
	}
	
	self->ascender = metrics->ascender * SCALE_FIX_26_6;
	self->descender = metrics->descender * SCALE_FIX_26_6;
	self->height = metrics->height * SCALE_FIX_26_6;

	texture_font_generate_background_glyph(self);

	return self;
}

// ---------------------------------------------------- texture_font_delete ---

void texture_font_delete( texture_font_t *self )
{
	assert( self );

	if( self->internal_handle )
	{
		texture_font_unload_font( self->internal_handle );
	}
	if( self->renderer_handle )
	{
		texture_font_unload_font( self->renderer_handle );
	}
	if( self->filename )
	{
		free( self->filename );
	}
	if( self->background_glyph )
	{
		texture_glyph_delete(self->background_glyph);
	}
	free( self );
}

// ---------------------------------------------------- texture_font_get_kerning ---

float texture_font_get_kerning( texture_font_t *self, wchar_t left_char, wchar_t right_char  )
{
	internal_font_t *internal_font = (internal_font_t*)self->internal_handle;
	FT_UInt left_index = FT_Get_Char_Index( internal_font->face, left_char );
	FT_UInt right_index = FT_Get_Char_Index( internal_font->face, right_char );
	FT_Vector kerning;
	FT_Get_Kerning( internal_font->face, left_index, right_index, FT_KERNING_UNFITTED, &kerning );
	return kerning.x / (float)(64.0f*64.0f);
}

void texture_font_new_glyph_bitmap( texture_font_t *self, texture_glyph_t *glyph, int bitmap_index, FT_BitmapGlyph ft_glyph )
{
	assert(self->atlas);
	// We want each glyph to be separated by at least one black pixel
	// (for example for shader used in demo-subpixel.c)
	size_t x, y, w, h;
	ivec4 region;
	w = ft_glyph->bitmap.width/self->atlas->depth + 1;
	h = ft_glyph->bitmap.rows + 1;
	region = texture_atlas_get_region( self->atlas, w, h );
	if(region.x<0)
	{
		// the atlas is full, create a new one
		texture_atlas_t *new_atlas=texture_atlas_new(self->atlas->width,self->atlas->height,self->atlas->depth);
		new_atlas->next=self->atlas;
		self->atlas=new_atlas;
		region = texture_atlas_get_region(new_atlas, w, h);
		assert(region.x>=0);
	}
	w = w - 1;
	h = h - 1;
	x = region.x;
	y = region.y;
	texture_atlas_set_region( self->atlas, region.x, region.y, w, h, 
		ft_glyph->bitmap.buffer, ft_glyph->bitmap.pitch );
	// update texture info
	glyph_bitmap_t *bm = glyph->bitmap[bitmap_index];
	if(!bm) {
		bm = (glyph_bitmap_t*)malloc(sizeof(glyph_bitmap_t));
		glyph->bitmap[bitmap_index] = bm;
	}
	bm->atlas_id= (uintptr_t)self->atlas;
	bm->width	= ft_glyph->bitmap.width;
	bm->height	= ft_glyph->bitmap.rows;
	bm->offset_x= ft_glyph->left;
	bm->offset_y= ft_glyph->top;
	bm->s0		= region.x/(float)self->atlas->width;
	bm->t0		= region.y/(float)self->atlas->height;
	bm->s1		= (region.x + w)/(float)self->atlas->width;
	bm->t1		= (region.y + h)/(float)self->atlas->height;
}

bool texture_font_render_outline(texture_font_t *self, texture_glyph_t *glyph, int bitmap_index, FT_Glyph ft_glyph, FT_Stroker ft_stroker)
{
	FT_Error error;
	FT_Glyph ft_image = ft_glyph;
	error = FT_Glyph_Stroke(&ft_image, ft_stroker, 0);
//	error = FT_Glyph_StrokeBorder(&ft_image, ft_stroker, 0, 0);
	if(error) 
		goto FT_ERROR_HANDLER;
	error = FT_Glyph_To_Bitmap(&ft_image, FT_RENDER_MODE_NORMAL, 0, 1);
	if( error ) 
		goto FT_ERROR_HANDLER;
	texture_font_new_glyph_bitmap(self,glyph,bitmap_index,(FT_BitmapGlyph)ft_image);
	FT_Done_Glyph(ft_image);
	return true;

FT_ERROR_HANDLER:
	HANDLE_FT_ERROR(error);
	return false;
}


void texture_font_glyph_bitmap (texture_font_t *self, texture_glyph_t *glyph, int technique, FT_Library ft_lib, FT_Face ft_face, FT_Glyph ft_glyph)
{
	FT_Error error;
	FT_Glyph ft_image;
	FT_Stroker ft_stroker = NULL;
	if( (TEXTURE_FONT_RENDER_OUTLINE & technique) && self->outline_thickness>0 )
	{
		error = FT_Stroker_New(ft_lib, &ft_stroker);
		if(error) {
			ft_stroker=NULL;
			HANDLE_FT_ERROR(error);
		}
		else FT_Stroker_Set(ft_stroker, (int)(self->outline_thickness*64), \
			FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
	}
	// render glyph
	if(TEXTURE_FONT_RENDER_NORMAL & technique) {
		ft_image = ft_glyph;
		error = FT_Glyph_To_Bitmap(&ft_image,FT_RENDER_MODE_NORMAL,0,0);
		if(error) {
			HANDLE_FT_ERROR(error);
		}
		else {
			texture_font_new_glyph_bitmap(self, glyph, GLYPH_NORMAL, (FT_BitmapGlyph)ft_image);
			FT_Done_Glyph(ft_image);
		}
		// render glyph outline
		if(ft_stroker)
			texture_font_render_outline(self,glyph,GLYPH_OUTLINE,ft_glyph,ft_stroker);
	}
	// render italic
	if(TEXTURE_FONT_RENDER_ITALIC & technique) {
		FT_Glyph_Transform(ft_glyph,&s_matrix_shearing,0);
		ft_image = ft_glyph;
		error = FT_Glyph_To_Bitmap(&ft_image, FT_RENDER_MODE_NORMAL, 0, 0);
		if( error ) {
			HANDLE_FT_ERROR(error);
		}
		else {
			texture_font_new_glyph_bitmap(self,glyph,GLYPH_ITALIC,(FT_BitmapGlyph)ft_image);
			FT_Done_Glyph(ft_image);
		}
		if(ft_stroker) 
			texture_font_render_outline(self,glyph,GLYPH_ITALIC_OUTLINE,ft_glyph,ft_stroker);
	}
	if(ft_stroker)
		FT_Stroker_Done(ft_stroker);
}

bool texture_font_load_glyph ( texture_font_t *self, wchar_t charcode, texture_glyph_t *glyph, int render_technique)
{
	assert(self);
	assert(glyph);

	internal_font_t *internal_font = (internal_font_t*) self->internal_handle;
	assert(internal_font);
	
	FT_Error error;
	FT_GlyphSlot ft_slot;
	FT_Glyph ft_glyph;
	error = FT_Load_Char( internal_font->face, charcode, FT_LOAD_DEFAULT );
	if( error ) {
		memcpy(glyph, self->background_glyph, sizeof(texture_glyph_t));
		glyph->charcode = charcode;
		goto FT_ERROR_HANDLER;
	}
	
	ft_slot = internal_font->face->glyph;
	FT_Glyph_Metrics *mt = &ft_slot->metrics;
	// basic data
	glyph->charcode	 = charcode;	
	glyph->width	 = mt->width  * SCALE_FIX_26_6;
	glyph->height	 = mt->height * SCALE_FIX_26_6;
	glyph->advance_x = ft_slot->linearHoriAdvance * SCALE_FIX_16_16;
	glyph->advance_y = ft_slot->linearVertAdvance * SCALE_FIX_16_16;
	glyph->offset_x  = mt->horiBearingX * SCALE_FIX_26_6;
	glyph->offset_y  = mt->horiBearingY * SCALE_FIX_26_6;

	// I found that with some font face and small point size, 
	// glyph->advance_x may less than (glyph->width + glyph->offset_x),
	// which resulting glyph quads overlap each other when rendering.
	// I'm not sure what cause this problem, 
	// and just make some hinting to work around this issue.
	if(glyph->offset_x+glyph->width>glyph->advance_x) 
		glyph->advance_x = glyph->offset_x+glyph->width;

	if(render_technique) {
		error = FT_Get_Glyph(internal_font->face->glyph, &ft_glyph);
		if(error)
			goto FT_ERROR_HANDLER;
		texture_font_glyph_bitmap(self, glyph, render_technique, \
			internal_font->library, internal_font->face, ft_glyph);
		FT_Done_Glyph(ft_glyph);
	}
	return true;
FT_ERROR_HANDLER:
	HANDLE_FT_ERROR(error);
	return false;
}

bool texture_font_render_glyph ( texture_font_t *self, texture_glyph_t *glyph, int technique )
{
	assert(self);
	assert(glyph);
	internal_font_t *internal_font = (internal_font_t*) self->renderer_handle;
	if(!internal_font) {
		internal_font = (internal_font_t*)texture_font_load_font(self->filename,self->size);
		if(!internal_font)
			return false;
		self->renderer_handle = internal_font;
	}
	assert(self->atlas);
	FT_Error error;
	FT_Glyph ft_glyph;
	error = FT_Load_Char( internal_font->face, glyph->charcode, FT_LOAD_DEFAULT);
	if(error) 
		goto FT_ERROR_HANDLER;
	error = FT_Get_Glyph( internal_font->face->glyph, &ft_glyph);
	if(error)
		goto FT_ERROR_HANDLER;
	texture_font_glyph_bitmap(self, glyph, technique, internal_font->library, internal_font->face, ft_glyph);
	FT_Done_Glyph(ft_glyph);
	return true;
FT_ERROR_HANDLER:
	HANDLE_FT_ERROR(error);
	return false;
}

// ------------------------------------------------------ make_distance_map ---

unsigned char* make_distance_map( unsigned char *img, unsigned int width, unsigned int height )
{
    short * xdist	= (short *)  malloc( width * height * sizeof(short) );
    short * ydist	= (short *)  malloc( width * height * sizeof(short) );
    double * gx		= (double *) calloc( width * height, sizeof(double) );
    double * gy		= (double *) calloc( width * height, sizeof(double) );
    double * data	= (double *) calloc( width * height, sizeof(double) );
    double * outside = (double *) calloc( width * height, sizeof(double) );
    double * inside  = (double *) calloc( width * height, sizeof(double) );
    unsigned int i;

    // Convert img into double (data)
    double img_min = 255, img_max = -255;
    for( i=0; i<width*height; ++i)
    {
        double v = img[i];
        data[i] = v;
        if (v > img_max) img_max = v;
        if (v < img_min) img_min = v;
    }
    // Rescale image levels between 0 and 1
    for( i=0; i<width*height; ++i)
    {
        data[i] = (img[i]-img_min)/img_max;
    }

    // Compute outside = edtaa3(bitmap); % Transform background (0's)
    computegradient( data, height, width, gx, gy);
    edtaa3(data, gx, gy, height, width, xdist, ydist, outside);
    for( i=0; i<width*height; ++i)
        if( outside[i] < 0 )
            outside[i] = 0.0;

    // Compute inside = edtaa3(1-bitmap); % Transform foreground (1's)
    memset(gx, 0, sizeof(double)*width*height );
    memset(gy, 0, sizeof(double)*width*height );
    for( i=0; i<width*height; ++i)
        data[i] = 1 - data[i];
    computegradient( data, height, width, gx, gy);
    edtaa3(data, gx, gy, height, width, xdist, ydist, inside);
    for( i=0; i<width*height; ++i)
        if( inside[i] < 0 )
            inside[i] = 0.0;

    // distmap = outside - inside; % Bipolar distance field
    unsigned char *out = (unsigned char *) malloc( width * height * sizeof(unsigned char) );
    for( i=0; i<width*height; ++i)
    {
        outside[i] -= inside[i];
        outside[i] = 128+outside[i]*16;
        if( outside[i] < 0 ) outside[i] = 0;
        if( outside[i] > 255 ) outside[i] = 255;
        out[i] = 255 - (unsigned char) outside[i];
        //out[i] = (unsigned char) outside[i];
    }

    free( xdist );
    free( ydist );
    free( gx );
    free( gy );
    free( data );
    free( outside );
    free( inside );
    return out;
}

// ------------------------------------------------------ clamp_image ---

bool clamp_image (unsigned char *img, unsigned &x, unsigned &y, unsigned &w, unsigned &h)
{
	unsigned min_x=w, min_y=h, max_x=0, max_y=0;
	unsigned char *row;
	bool valid_row;
	for(row=img, y=0; y<h; ++y, row+=w)
	{
		valid_row=false;
		for(x=0; x<w; ++x)
		{
			if(row[x])
			{
				if(x<min_x)
					min_x=x;
				if(x>max_x)
					max_x=x;
				valid_row=true;
			}
		}
		if(valid_row)
		{
			if(y<min_y)
				min_y=y;
			if(y>max_y)
				max_y=y;
		}
	}
	x=min_x, y=min_y;
	w=max_x-min_x+1, h=max_y-min_y+1;
	return min_x<=max_x && min_y<=max_y;
}

// ----------------------------------------------- texture_font_render_edt_glyph ---

bool texture_font_render_edt_glyph ( texture_font_t *self, texture_glyph_t *glyph )
{
	assert(self);
	assert(self->atlas);
	assert(1==self->atlas->depth);
	assert(glyph);

	internal_font_t *internal_font = (internal_font_t*) self->renderer_handle;
	if(!internal_font)
		return false;

	FT_Error error;
	FT_GlyphSlot ft_slot;
	FT_Bitmap ft_bitmap;

	error = FT_Load_Char(internal_font->face, glyph->charcode, FT_LOAD_RENDER);
	if(error)
		goto FT_ERROR_HANDLER;

	ft_slot	  = internal_font->face->glyph;
	ft_bitmap = ft_slot->bitmap;

	unsigned x, y, w, h, stride;
	int x_offset, y_offset;
	unsigned char *row, *out=0;
	ivec4 region;
	if(ft_bitmap.width>ft_bitmap.rows)
		w=ft_bitmap.width;
	else
		w=ft_bitmap.rows;
	if(0==w) 
		return false;

	w=size_t(1.5f*w);
	if(w&1) w+=1;
	h=w;
	unsigned char *tmp_bitmap=(unsigned char*)calloc(w*h,sizeof(unsigned char));
	assert(w>=unsigned(ft_bitmap.width));
	assert(h>=unsigned(ft_bitmap.rows));
	x=(w-ft_bitmap.width)>>1;
	y=(h-ft_bitmap.rows)>>1;
	x_offset=x;
	y_offset=y;
	stride=w;
	row=ft_bitmap.buffer;
	out=tmp_bitmap+y*w+x;
	for(y=0; y<unsigned(ft_bitmap.rows); ++y, row+=ft_bitmap.pitch, out+=w) 
	{
		memcpy(out,row,sizeof(unsigned char)*ft_bitmap.width);
	}
	out=make_distance_map(tmp_bitmap,w,h);
	free(tmp_bitmap);
	clamp_image(out,x,y,w,h);
	assert(w>0 && h>0);

	region = texture_atlas_get_region( self->atlas, w+1, h+1 );
	if(region.x<0) 
	{
		// the atlas is full, create a new one
		texture_atlas_t *new_atlas=texture_atlas_new(self->atlas->width,self->atlas->height,self->atlas->depth);
		new_atlas->next=self->atlas;
		self->atlas=new_atlas;
		region = texture_atlas_get_region(new_atlas, w+1, h+1);
		assert(region.x>=0);
	}
	texture_atlas_set_region_ex(self->atlas, region.x, region.y, w, h, out, x, y, stride);
	free(out);
	assert(x_offset>=int(x) && y_offset>=int(y));
	x_offset=ft_slot->bitmap_left-x_offset+x;
	y_offset=ft_slot->bitmap_top+y_offset-y;

	glyph_bitmap_t *bm = glyph->bitmap[GLYPH_EDT];
	if(!bm) {
		bm = (glyph_bitmap_t*)malloc(sizeof(glyph_bitmap_t));
		glyph->bitmap[GLYPH_EDT]=bm;
	}
	assert(w<65536 && h<65536);
	assert(abs(x_offset)<32768 && abs(y_offset)<32768);
	bm->atlas_id = (uintptr_t)self->atlas; 
	bm->width	 = unsigned short(w);
	bm->height	 = unsigned short(h);
	bm->offset_x = short(x_offset);
	bm->offset_y = short(y_offset);
	bm->s0		 = region.x/(float)self->atlas->width;
	bm->t0		 = region.y/(float)self->atlas->height;
	bm->s1		 = (region.x + w)/(float)self->atlas->width;
	bm->t1		 = (region.y + h)/(float)self->atlas->height;

	return true;

FT_ERROR_HANDLER:
	HANDLE_FT_ERROR(error);
	return false;
}
