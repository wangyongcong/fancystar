#ifndef __HASH_H__
#define __HASH_H__

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    unsigned int _idx;

} dict_iterator_t;

typedef struct 
{
    texture_glyph_t **data;

    unsigned int capacity;

    unsigned int occupied;

    unsigned int deleted;

} glyph_dict_t;

glyph_dict_t* glyph_dict_new (size_t capacity);

void glyph_dict_delete (glyph_dict_t *self);

void glyph_dict_clear (glyph_dict_t *self);

void glyph_dict_reserve (glyph_dict_t *self, size_t capacity);

texture_glyph_t* glyph_dict_get (const glyph_dict_t *self, wchar_t charcode);

void glyph_dict_set (glyph_dict_t *self, wchar_t charcode, texture_glyph_t *glyph);

texture_glyph_t* glyph_dict_pop (glyph_dict_t *self, wchar_t charcode);

size_t glyph_dict_len (const glyph_dict_t *self);

texture_glyph_t* glyph_dict_get_first( glyph_dict_t *self, dict_iterator_t *iter);

texture_glyph_t* glyph_dict_get_next( glyph_dict_t *self, dict_iterator_t *iter);

void glyph_dict_dump_buckets( glyph_dict_t *self, FILE *file );

#ifdef __cplusplus
}
#endif

#endif // __HASH_H__
