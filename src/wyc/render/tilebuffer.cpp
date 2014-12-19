#include "fscorepch.h"
#include "wyc/render/renderer.h"
#include "wyc/render/tilebuffer.h"

namespace wyc
{

#define XTILE_BUFFER_MAX_VERTEX 65535

const unsigned xtile_buffer::ms_cache_size[CACHE_LEVEL]={4, 8, 16, 32, 64, 128, 256, 512, 1024};

xtile_buffer::xtile_buffer()
{
	m_vbo=m_ibo=0;
	m_vao=0;
	m_cap_vertex=m_cap_index=0;
	m_read_only=false;
	memset(m_free_mesh,0,sizeof(m_free_mesh));
}

void xtile_buffer::delthis()
{
	if(m_vbo) 
	{
		glDeleteBuffers(2,m_vbuffs);
		m_vbo=m_ibo=0;
	}
	if(m_vao) 
	{
		glDeleteVertexArrays(1,&m_vao);
		m_vao=0;
	}
	xmesh2d *pmesh;
	unsigned cnt=m_mesh_record.size();
	for(unsigned i=0; i<cnt; ++i) {
		pmesh=m_mesh_record[i];
		if(pmesh->refcount())
			pmesh->m_parent=0;
		else
			wycdel pmesh;
	}
	m_vertex_buff.clear();
	m_idx_buff.clear();
	m_cap_vertex=m_cap_index=0;
	xrefobj::delthis();
}

xmesh2d* xtile_buffer::alloc_mesh(unsigned size)
{
	int idx;
	binary_search(ms_cache_size,CACHE_LEVEL,size,idx);
	if(idx>=CACHE_LEVEL) {
		wyc_error("xtile_buffer::alloc_mesh: vertex count must less than [%d]",max_vertex_count());
		return 0;
	}
	assert(ms_cache_size[idx]>=size);
	size=ms_cache_size[idx];
	xmesh2d *pmesh;
	if(m_free_mesh[idx]) {
		pmesh=m_free_mesh[idx];
		m_free_mesh[idx]=pmesh->m_free_link;
		pmesh->m_parent=this;
	}
	else {
		unsigned base=m_vertex_buff.size();
		if(base+size>=XTILE_BUFFER_MAX_VERTEX)
			return 0;
		m_vertex_buff.resize(base+size);
		pmesh=wycnew xmesh2d(this,base,size);
		m_mesh_record.push_back(pmesh);
	}
	return pmesh;
}

bool xtile_buffer::alloc_mesh(xmesh2d** mesh_array, unsigned mesh_count, unsigned size)
{
	int idx;
	binary_search(ms_cache_size,CACHE_LEVEL,size,idx);
	if(idx>=CACHE_LEVEL) {
		wyc_error("xtile_buffer::alloc_mesh: vertex count must less than [%d]",max_vertex_count());
		return false;
	}
	assert(ms_cache_size[idx]>=size);
	size=ms_cache_size[idx];
	xmesh2d **iter=mesh_array, **end=mesh_array+mesh_count;
	for(; iter!=end && m_free_mesh[idx]; ++iter)
	{
		*iter=m_free_mesh[idx];
		m_free_mesh[idx]=(*iter)->m_free_link;
		(*iter)->m_parent=this;
	}
	if(iter!=end) {
		unsigned base=m_vertex_buff.size();
		m_vertex_buff.resize(base+(end-iter)*size);
		for(; iter!=end; ++iter, base+=size)
		{
			*iter=wycnew xmesh2d(this,base,size);
			m_mesh_record.push_back(*iter);
		}
	}
	return true;
}

void xtile_buffer::free_mesh(xmesh2d *pmesh)
{
	assert(this==pmesh->m_parent);
	int idx;
	binary_search(ms_cache_size,CACHE_LEVEL,pmesh->vertex_count(),idx);
	assert(idx<CACHE_LEVEL);
	pmesh->m_free_link=m_free_mesh[idx];
	m_free_mesh[idx]=pmesh;
}

void xtile_buffer::free_mesh(xmesh2d** mesh_array, unsigned mesh_count)
{
	int idx=CACHE_LEVEL;
	unsigned last_size=0;
	xmesh2d *pmesh;
	for(unsigned i=0; i<mesh_count; ++i) {
		pmesh=mesh_array[i];
		assert(this==pmesh->m_parent);
		if(pmesh->vertex_count()!=last_size) 
		{
			last_size=pmesh->vertex_count();
			binary_search(ms_cache_size,CACHE_LEVEL,pmesh->vertex_count(),idx);
		}
		assert(idx<CACHE_LEVEL);
		pmesh->m_free_link=m_free_mesh[idx];
		m_free_mesh[idx]=pmesh;
	}
}

void xtile_buffer::mark_section(unsigned base, unsigned size)
{
	if(m_read_only) 
		return;
	unsigned upper_bound;
	section_t sec(base,size);
	section_list_t::iterator upper=m_commit_section.upper_bound(sec), lower;
	if(upper!=m_commit_section.begin()) {
		lower=upper;
		--lower;
		upper_bound=lower->first+lower->second;
		if(upper_bound>=sec.first) {
		//	wyc_warn("combine: (%d,%d)+(%d,%d)=(%d,%d)",lower->first,lower->second,sec.first,sec.second,lower->first,sec.second+sec.first-lower->first);
			sec.second+=sec.first-lower->first;
			sec.first=lower->first;
			m_commit_section.erase(lower);
		}
	}
	if(upper!=m_commit_section.end()) {
		upper_bound=sec.first+sec.second;
		if(upper_bound>=upper->first) {
			sec.second+=upper->first+upper->second-upper_bound;
			m_commit_section.erase(upper);
		}
	}
	std::pair<section_list_t::iterator,bool> ret=m_commit_section.insert(sec);
	assert(ret.second);
}

void xtile_buffer::render(xrenderer *rc)
{
	if(!m_idx_buff.size())
		return;
	r_update_buffers();
	if(m_vao) {
		glBindVertexArray(m_vao);
	}
	else {
		glBindBuffer(GL_ARRAY_BUFFER,m_vbo);
		glVertexAttribPointer(wyc::USAGE_POSITION,2,GL_FLOAT,GL_FALSE,sizeof(xvertex2d),(GLvoid*)0);
		glVertexAttribPointer(wyc::USAGE_TEXTURE0,2,GL_FLOAT,GL_FALSE,sizeof(xvertex2d),(GLvoid*)sizeof(xvec2f_t));
		glVertexAttribPointer(wyc::USAGE_COLOR,4,GL_UNSIGNED_BYTE,GL_TRUE,sizeof(xvertex2d),(GLvoid*)(sizeof(xvec2f_t)+sizeof(xvec2f_t)));
		glEnableVertexAttribArray(USAGE_POSITION);
		glEnableVertexAttribArray(USAGE_TEXTURE0);
		glEnableVertexAttribArray(USAGE_COLOR);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m_ibo);
	}
	unsigned basic_shader=rc->get_shader_id(), prev_shader=0;
	for(draw_buffer_t::iterator iter=m_draw_buff.begin(), end=m_draw_buff.end(); iter!=end; ++iter) {
		if(iter->m_shader!=prev_shader) {
			if(iter->m_shader) 
				rc->use_shader(iter->m_shader);
			else rc->use_shader(basic_shader);
			prev_shader=iter->m_shader;
		}
		glBindTexture(GL_TEXTURE_2D,iter->m_tex);
		glDrawElements(GL_TRIANGLES,iter->m_count,GL_UNSIGNED_SHORT,(void*)(iter->m_begin*sizeof(GLushort)));
	}
	if(m_vao) {
		glBindVertexArray(0);
	}
	else {
		glDisableVertexAttribArray(USAGE_POSITION);
		glDisableVertexAttribArray(USAGE_TEXTURE0);
		glDisableVertexAttribArray(USAGE_COLOR);
		glBindBuffer(GL_ARRAY_BUFFER,0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
	}
	assert(glGetError()==GL_NO_ERROR);
}

inline size_t vbuff_size(size_t min_size)
{
	if(min_size<1024)
		return 1024;
	return (min_size+1023)&(~1023);
}

void xtile_buffer::r_init_buffers()
{
	glGenBuffers(2,m_vbuffs);
	// VAO begin
	if(GLEW_ARB_vertex_array_object)
	{
		glGenVertexArrays(1,&m_vao);
		glBindVertexArray(m_vao);
	}
	GLenum buff_hint=m_read_only?GL_STATIC_DRAW:GL_STREAM_DRAW;
	// init vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER,m_vbo);
	m_cap_vertex=vbuff_size(m_vertex_buff.size()*sizeof(xvertex2d));
	glBufferData(GL_ARRAY_BUFFER,m_cap_vertex,0,buff_hint);
	if(m_vertex_buff.size()) {
		glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(xvertex2d)*m_vertex_buff.size(),&m_vertex_buff[0]);
	}
	glVertexAttribPointer(USAGE_POSITION,2,GL_FLOAT,GL_FALSE,sizeof(xvertex2d),(GLvoid*)0);
	glVertexAttribPointer(USAGE_TEXTURE0,2,GL_FLOAT,GL_FALSE,sizeof(xvertex2d),(GLvoid*)sizeof(xvec2f_t));
	glVertexAttribPointer(USAGE_COLOR,4,GL_UNSIGNED_BYTE,GL_TRUE,sizeof(xvertex2d),(GLvoid*)(sizeof(xvec2f_t)+sizeof(xvec2f_t)));
	glEnableVertexAttribArray(USAGE_POSITION);
	glEnableVertexAttribArray(USAGE_TEXTURE0);
	glEnableVertexAttribArray(USAGE_COLOR);
	// init index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m_ibo);
	m_cap_index=vbuff_size(m_idx_buff.size()*sizeof(GLushort));
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,m_cap_index,0,buff_hint);
	if(m_idx_buff.size()) {
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,0,sizeof(GLushort)*m_idx_buff.size(),&m_idx_buff[0]);
	}
	// VAO end
	if(m_vao)
	{
		glBindVertexArray(0);
	}
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
	assert(glGetError()==GL_NO_ERROR);
}

void xtile_buffer::r_update_buffers()
{
	if(!m_vbo) {
		r_init_buffers();
		m_commit_section.clear();
		return;
	}
	if(m_read_only) 
		return;
	// update vertex buffer
	if(m_commit_section.size()) 
	{
		glBindBuffer(GL_ARRAY_BUFFER,m_vbo);
		size_t sz=m_vertex_buff.size()*sizeof(xvertex2d);
		if(sz>m_cap_vertex) {
			// resize vertex buffer
			m_cap_vertex=vbuff_size(sz);
			glBufferData(GL_ARRAY_BUFFER,m_cap_vertex,0,m_read_only?GL_STATIC_DRAW:GL_STREAM_DRAW);
			glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(xvertex2d)*m_vertex_buff.size(),&m_vertex_buff[0]);
		}
		else {
			// update vertex buffer
			section_list_t::iterator iter, end;
			for(iter=m_commit_section.begin(), end=m_commit_section.end(); iter!=end; ++iter) {
				GLintptr offset=(uint8_t*)&m_vertex_buff[iter->first]-(uint8_t*)&m_vertex_buff[0];
				glBufferSubData(GL_ARRAY_BUFFER,offset,sizeof(xvertex2d)*iter->second,&m_vertex_buff[iter->first]);
			//	wyc_print("tilebuffer: update [%d,%d]",iter->first,iter->second);
			}
		}
		glBindBuffer(GL_ARRAY_BUFFER,0);
		m_commit_section.clear();
	}
	// update index buffer
	if(m_commit_index) 
	{
		m_commit_index=false;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m_ibo);
		size_t sz=m_idx_buff.size()*sizeof(GLushort);
		if(sz>m_cap_index) {
			m_cap_index=vbuff_size(sz);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER,m_cap_index,0,m_read_only?GL_STATIC_DRAW:GL_STREAM_DRAW);
		}
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,0,sizeof(GLushort)*m_idx_buff.size(),&m_idx_buff[0]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
	}
}

void xtile_buffer::draw_mesh(unsigned base, unsigned count, unsigned short *index, unsigned idx_count, GLuint tex, unsigned shader)
{
	if(tex==0) {
		// TODO: use default texture
		wyc_warn("xtile_buffer::draw_mesh: No texture");
		return;
	}
	assert(base+count<=m_vertex_buff.size());
	m_idx_buff.reserve(m_idx_buff.size()+idx_count);
	GLushort idx;
	for(unsigned i=0; i<idx_count; ++i) {
		idx=GLushort(base+index[i]);
		assert(idx<m_vertex_buff.size());
		m_idx_buff.push_back(idx);
	}
	m_commit_index=true;
	unsigned begin=0;
	if(m_draw_buff.size()) {
		xdraw_section &last=m_draw_buff.back();
		if(tex==last.m_tex && shader==last.m_shader) {
			last.m_count+=idx_count;
			return;
		}
		begin=last.m_begin+last.m_count;
	}
	xdraw_section ds;
	ds.m_tex=tex;
	ds.m_shader=shader;
	ds.m_begin=begin;
	ds.m_count=idx_count;
	m_draw_buff.push_back(ds);
}

void xtile_buffer::clear_draw_buffer()
{
	m_draw_buff.clear();
	m_idx_buff.clear();
}

} // namespace wyc


