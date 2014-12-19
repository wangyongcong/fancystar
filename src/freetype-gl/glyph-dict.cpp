#include "fscorepch.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "texture-font.h"
#include "glyph-dict.h"

// minimum table size
#define GLYPH_DICT_MIN_SIZE 16
// maxmum table size
#define GLYPH_DICT_MAX_SIZE (4*1024*1024) 
// percentage of capacity to shrink table size and rehash
#define GLYPH_DICT_SHRINK_PERCENT 0.4f
// percentage of capacity to grow table size and rehash, should be 2*HT_SHRINK_PERCENT
#define GLYPH_DICT_GROW_PERCENT 0.8f
// the deleted glyph pointer
#define DELETED_GLYPH_PTR (texture_glyph_t*)(-1)

#define NO_MEMORY_AND_EXIT {\
	fprintf( stderr, "%s (line %d): No more memory for allocating data\n", __FILE__, __LINE__ );\
	exit( EXIT_FAILURE );\
	}

size_t glyph_dict_calculate_size(size_t required_size)
{
	size_t size=GLYPH_DICT_MIN_SIZE;
	while(required_size >= size*GLYPH_DICT_GROW_PERCENT) {
		size<<=1;
		assert(size<GLYPH_DICT_MAX_SIZE);
	}
	return size;
}

void glyph_dict_resize (glyph_dict_t *self, size_t capacity)
{
	assert( 0==( (capacity-1) & capacity) );
	assert( self->occupied-self->deleted < capacity*GLYPH_DICT_GROW_PERCENT );
	size_t size = sizeof(texture_glyph_t*) * capacity;
	texture_glyph_t ** new_data = (texture_glyph_t**)malloc(size);
	if(!new_data) 
		NO_MEMORY_AND_EXIT;
	memset(new_data, 0, size);
	size_t mask=capacity-1, prob, i, pos;
	texture_glyph_t *ptr;
	for(i=0; i<self->capacity; ++i) {
		ptr=self->data[i];
		if(!ptr || DELETED_GLYPH_PTR==ptr)
			continue;
		pos=ptr->charcode&mask;
		prob=0;
		while(0!=new_data[pos]) {
			prob+=1;
			assert(prob<capacity);
			pos=(pos+prob)&mask;
		}
		new_data[pos]=ptr;
	}
	free(self->data);
	self->data = new_data;
	self->capacity = capacity;
	self->occupied = self->occupied - self->deleted;
	self->deleted = 0;
}

glyph_dict_t* glyph_dict_new (size_t capacity)
{
	glyph_dict_t *self = (glyph_dict_t*)malloc( sizeof(glyph_dict_t) );
	if(!self)
		NO_MEMORY_AND_EXIT;
	capacity = glyph_dict_calculate_size(capacity);
	size_t size = sizeof(texture_glyph_t*)*capacity;
	self->data = (texture_glyph_t**)malloc(size);
	if(!self->data) 
		NO_MEMORY_AND_EXIT;
	memset(self->data,0,size);
	self->capacity = capacity;
	self->occupied = 0;
	self->deleted = 0;
	return self;
}

void glyph_dict_delete (glyph_dict_t *self)
{
	assert(self);
	free(self->data);
	free(self);
}

void glyph_dict_clear (glyph_dict_t *self)
{
	assert(self);
	memset(self->data, 0, sizeof(texture_glyph_t*)*self->capacity);
	self->occupied=0;
	self->deleted=0;
}

void glyph_dict_reserve (glyph_dict_t *self, size_t capacity)
{
	assert(self);
	capacity = glyph_dict_calculate_size(capacity);
	if (capacity > self->capacity)
		glyph_dict_resize(self, capacity);
}

texture_glyph_t* glyph_dict_get (const glyph_dict_t *self, wchar_t charcode)
{
	assert(self);
	size_t mask = self->capacity-1;
	assert( 0==(mask & self->capacity) );
	size_t pos=charcode&mask, prob=0;
	texture_glyph_t *ptr = self->data[pos];
	while( 0!=ptr ) {
		if( DELETED_GLYPH_PTR!=ptr && charcode==ptr->charcode )
			return ptr;
		prob+=1;
		assert(prob<self->capacity);
		pos=(pos+prob)&mask;
		ptr=self->data[pos];
	}
	return 0;
}

void glyph_dict_set (glyph_dict_t *self, wchar_t charcode, texture_glyph_t *glyph)
{
	assert(self);
	if( self->occupied+1 >= self->capacity*GLYPH_DICT_GROW_PERCENT )
	{
		glyph_dict_resize(self, glyph_dict_calculate_size(self->occupied-self->deleted+1) );
	}
	size_t mask=self->capacity-1;
	assert( 0==(mask & self->capacity) );
	size_t pos=charcode&mask, insert_pos=size_t(-1), prob=0;
	texture_glyph_t *ptr=self->data[pos];
	while( 0!=ptr ) {
		if( DELETED_GLYPH_PTR == ptr ) {
			if( size_t(-1) == insert_pos)
				insert_pos=pos;
		}
		else if( charcode == ptr->charcode ) {
			self->data[pos]=glyph;
			return;
		}
		prob+=1;
		assert(prob<self->capacity);
		pos=(pos+prob)&mask;
		ptr=self->data[pos];
	}
	if( size_t(-1) == insert_pos) {
		self->data[pos]=glyph;
		self->occupied+=1;
	}
	else {
		self->data[insert_pos]=glyph;
		self->deleted-=1;
	}
}

texture_glyph_t* glyph_dict_pop (glyph_dict_t *self, wchar_t charcode)
{
	assert(self);
	size_t mask=self->capacity-1;
	assert( 0==(mask & self->capacity) );
	size_t pos=charcode&mask, prob=0;
	texture_glyph_t *ptr = self->data[pos];
	while( 0!=ptr ) {
		if( DELETED_GLYPH_PTR!=ptr && charcode==ptr->charcode ) {
			self->data[pos]=DELETED_GLYPH_PTR;
			self->deleted+=1;
			return ptr;
		}
		prob+=1;
		assert(prob<self->capacity);
		pos=(pos+prob)&mask;
		ptr=self->data[pos];
	}
	return 0;
}

size_t glyph_dict_len (const glyph_dict_t *self)
{
	assert(self);
	return self->occupied-self->deleted;
}


texture_glyph_t* glyph_dict_get_first( glyph_dict_t *self, dict_iterator_t *iter)
{
	assert(self);
	assert(iter);
	size_t i;
	for (i=0; i<self->capacity; ++i) {
		if( !self->data[i] || DELETED_GLYPH_PTR==self->data[i])
			continue;
		iter->_idx=i+1;
		return self->data[i];
	}
	return 0;
}

texture_glyph_t* glyph_dict_get_next( glyph_dict_t *self, dict_iterator_t *iter)
{
	assert(self);
	assert(iter);
	size_t i;
	for (i=iter->_idx; i<self->capacity; ++i) {
		if( !self->data[i] || DELETED_GLYPH_PTR==self->data[i])
			continue;
		iter->_idx=i+1;
		return self->data[i];
	}
	return 0;
}

void glyph_dict_dump_buckets( glyph_dict_t *self, FILE *file )
{
	assert(self);
	fprintf(file,"+------------------------------------------------\n");
	fprintf(file,"| dump glyph dict (0-empty; X-deleted; E-data)\n");
	fprintf(file,"+------------------------------------------------\n| ");
	size_t i, col;
	for (i=0, col=0; i<self->capacity; ++i, ++col) {
		if(40==col) {
			fprintf(file,"\n| ");
			col=0;
		}
		if( !self->data[i] )
			fprintf(file,"0");
		else if( DELETED_GLYPH_PTR==self->data[i] )
			fprintf(file,"X");
		else
			fprintf(file,"E");
	}
	fprintf(file,"\n+------------------------------------------------\n");
}

/**
 * unit test for glyph dict
 */
extern texture_glyph_t *texture_glyph_new( void );
extern void texture_glyph_delete( texture_glyph_t * );

void test_glyph_dict ()
{
	printf("test glyph dict...");
	glyph_dict_t *di = glyph_dict_new(8);
	texture_glyph_t *glyph;
	size_t cap;
	wchar_t c;
	printf("insert [a-z]\n");
	for(c='a'; c<='z'; ++c) {
		glyph=texture_glyph_new();
		glyph->charcode=c;
		cap=di->capacity;
		glyph_dict_set(di,c,glyph);
		if(cap!=di->capacity) 
		printf("[%d]: capacity glow\n",c-'a');
	}
	assert(glyph_dict_len(di)==26);
	for(c='a'; c<='z'; ++c) {
		glyph=glyph_dict_get(di,c);
		assert(glyph);
		assert(glyph->charcode==c);
	}
	dict_iterator_t iter;
	printf("[iteritems] capacity: %d (%d/%d)\n",di->capacity,di->deleted,di->occupied);
	glyph=glyph_dict_get_first(di,&iter);
	while(glyph) {
		printf("%c",char(glyph->charcode));
		glyph=glyph_dict_get_next(di,&iter);
	}
	printf(" [END]\n");
	glyph_dict_reserve(di,52);
	printf("reserve space for 52 items: %d (%d/%d)\n",di->capacity,di->deleted,di->occupied);
	printf("insert [A-Z]\n");
	for(c='A'; c<='Z'; ++c) {
		glyph=glyph_dict_get(di,c);
		assert(!glyph);
		glyph=texture_glyph_new();
		glyph->charcode=c;
		cap=di->capacity;
		glyph_dict_set(di,c,glyph);
		assert(cap==di->capacity);
	}
	assert(glyph_dict_len(di)==52);
	printf("[iteritems] capacity: %d (%d/%d)\n",di->capacity,di->deleted,di->occupied);
	glyph=glyph_dict_get_first(di,&iter);
	while(glyph) {
		printf("%c",char(glyph->charcode));
		glyph=glyph_dict_get_next(di,&iter);
	}
	printf(" [END]\n");
	printf("delete [A-Z]\n");
	for(c='A'; c<='Z'; ++c) {
		glyph=glyph_dict_pop(di,c);
		assert(glyph);
		assert(glyph->charcode==c);
		texture_glyph_delete(glyph);
		glyph=glyph_dict_get(di,c);
		assert(!glyph);
	}
	assert(glyph_dict_len(di)==26);
	printf("[iteritems] capacity: %d (%d/%d)\n",di->capacity,di->deleted,di->occupied);
	glyph=glyph_dict_get_first(di,&iter);
	while(glyph) {
		printf("%c",char(glyph->charcode));
		texture_glyph_delete(glyph);
		glyph=glyph_dict_get_next(di,&iter);
	}
	printf(" [END]\n");
	glyph_dict_dump_buckets(di,stdout);
	glyph_dict_clear(di);
	assert(glyph_dict_len(di)==0);
	glyph=glyph_dict_get_first(di,&iter);
	assert(!glyph);
	printf("all test OK");
}
