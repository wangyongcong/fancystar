#include "fscorepch.h"
#include "wyc/render/uniform_buffer.h"

namespace wyc
{

xuniform_buffer::xuniform_buffer()
{
	m_ubo=0;
	m_mapped_buffer=0;
	m_buffer_size=0;
}

xuniform_buffer::~xuniform_buffer()
{
	clear();
}

void xuniform_buffer::clear()
{
	if(m_ubo) 
	{
		glDeleteBuffers(1,&m_ubo);
		m_ubo=0;
		m_mapped_buffer=0;
		m_buffer_size=0;
	}
	xdict::iterator iter, end;
	for(iter=m_blocks.begin(), end=m_blocks.end();
		iter!=end; ++iter) 
	{
		delete [] (uint8_t*)iter->second;
	}
	m_blocks.clear();
	m_uniforms.clear();
}

void xuniform_buffer::reserve(unsigned block_count, unsigned entry_count)
{
	m_blocks.reserve(block_count);
	m_blocks.reserve(entry_count);
}

bool xuniform_buffer::append_block(const char *block_name, unsigned member_count, const char **member_names, unsigned binding_point)
{
	assert(block_name);
	assert(member_names);
	assert(member_count);
	xuniform_block_info* ub=(xuniform_block_info*)new uint8_t[sizeof(xuniform_block_info)+
		sizeof(xuniform_entry_info)*(member_count-1)];
	ub->m_name=block_name;
	ub->m_binding=binding_point;
	ub->m_offset=0;
	ub->m_size=0;
	ub->m_members=member_names;
	ub->m_member_count=member_count;
	unsigned gid=strhash(block_name);
	if (!m_blocks.add(gid,(xdict::value_t)ub)) {
		delete ub;
		wyc_error("uniform block [%s] already exists",block_name);
		return false;
	}
	for(unsigned i=0; i<ub->m_member_count; ++i) {
		gid=strhash(ub->m_members[i]);
		if(!m_uniforms.add(gid,(xdict::value_t)&ub->m_member_info[i]))
			wyc_error("uniform block entry [%s] alredy exists",ub->m_members[i]);
	}
	return true;
}

bool xuniform_buffer::querry_block_info(GLuint program, xuniform_block_info *ub)
{
	GLuint ub_idx=glGetUniformBlockIndex(program,ub->m_name);
	if(ub_idx==GL_INVALID_INDEX)
		return false;
	GLuint *indices=new GLuint[ub->m_member_count];
	glGetUniformIndices(program,ub->m_member_count,ub->m_members,indices);
	GLenum err=glGetError();
	if(GL_NO_ERROR!=glGetError()) {
		delete [] indices;
		return false;
	}
	GLint *params=new GLint[ub->m_member_count];
	glGetActiveUniformBlockiv(program,ub_idx,GL_UNIFORM_BLOCK_DATA_SIZE,params);
	ub->m_size=params[0];
	unsigned i;
	for(i=0; i<ub->m_member_count; ++i) 
		ub->m_member_info[i].m_index=indices[i];
	glGetActiveUniformsiv(program,ub->m_member_count,indices,GL_UNIFORM_TYPE,params);
	for(i=0; i<ub->m_member_count; ++i) 
		ub->m_member_info[i].m_type=params[i];
	glGetActiveUniformsiv(program,ub->m_member_count,indices,GL_UNIFORM_OFFSET,params);
	for(i=0; i<ub->m_member_count; ++i) 
		ub->m_member_info[i].m_offset=params[i];
	glGetActiveUniformsiv(program,ub->m_member_count,indices,GL_UNIFORM_SIZE,params);
	for(i=0; i<ub->m_member_count; ++i) 
		ub->m_member_info[i].m_array_size=params[i];
	glGetActiveUniformsiv(program,ub->m_member_count,indices,GL_UNIFORM_ARRAY_STRIDE,params);
	for(i=0; i<ub->m_member_count; ++i) 
		ub->m_member_info[i].m_array_stride=params[i];
	glGetActiveUniformsiv(program,ub->m_member_count,indices,GL_UNIFORM_MATRIX_STRIDE,params);
	for(i=0; i<ub->m_member_count; ++i) 
		ub->m_member_info[i].m_matrix_stride=params[i];
	glGetActiveUniformsiv(program,ub->m_member_count,indices,GL_UNIFORM_IS_ROW_MAJOR,params);
	for(i=0; i<ub->m_member_count; ++i) 
		ub->m_member_info[i].m_row_major=params[i];
	delete [] params;
	delete [] indices;
	return GL_NO_ERROR==err;
}

bool xuniform_buffer::bind_program(GLuint program, const char *block_name)
{
	assert(program && block_name);
	xuniform_block_info* ubi;
	unsigned gid=strhash(block_name);
	ubi = (xuniform_block_info*)m_blocks.get(gid);
	if(!ubi)
		return false;
	if(!ubi->m_size) {
		if(!querry_block_info(program,ubi)) 
			return false;
	}
	gid=glGetUniformBlockIndex(program,block_name);
	glUniformBlockBinding(program,gid,ubi->m_binding);
	return true;
}

void xuniform_buffer::create_buffer()
{
	if(!m_blocks.size()) 
		return;
	if(!m_ubo)
		glGenBuffers(1,&m_ubo);
	m_buffer_size=0;
	xuniform_block_info *ub;
	xdict::iterator iter, end;
	for(iter=m_blocks.begin(), end=m_blocks.end();
		iter!=end; ++iter) 
	{
		ub=(xuniform_block_info*)iter->second;
		ub->m_offset=m_buffer_size;
		for(unsigned i=0; i<ub->m_member_count; ++i) 
			ub->m_member_info[i].m_offset+=ub->m_offset;
		m_buffer_size+=ub->m_size;
	}
	glBindBuffer(GL_UNIFORM_BUFFER,m_ubo);
	glBufferData(GL_UNIFORM_BUFFER,m_buffer_size,0,GL_STREAM_DRAW);
	for(iter=m_blocks.begin(), end=m_blocks.end();
		iter!=end; ++iter) 
	{
		ub=(xuniform_block_info*)iter->second;
		glBindBufferRange(GL_UNIFORM_BUFFER,ub->m_binding,m_ubo,ub->m_offset,ub->m_size);
	}
	glBindBuffer(GL_UNIFORM_BUFFER,0);
	if(glGetError()!=GL_NO_ERROR)
		wyc_error("[GL] create uniform buffer object");
}

void xuniform_buffer::commit()
{
	if(!m_ubo || !m_mapped_buffer)
		return;
	m_mapped_buffer=0;
	if(GL_TRUE!=glUnmapBuffer(GL_UNIFORM_BUFFER))
		wyc_error("[GL] failed to commit uniform buffer");
}

bool xuniform_buffer::set_uniform(const char *name, const xmat3f_t mat3)
{
	assert(name);
	unsigned gid=strhash(name);
	xuniform_entry_info *entry = (xuniform_entry_info*)m_uniforms.get(gid);
	if(!entry)
		return false;
	if(GL_FLOAT_MAT3!=entry->m_type)
		return false;
	if(!m_mapped_buffer) 
	{
		if(!m_ubo) 
			return false;
		glBindBuffer(GL_UNIFORM_BUFFER,m_ubo);
		m_mapped_buffer=glMapBuffer(GL_UNIFORM_BUFFER,m_ubo);
	}
	uint8_t *buff=((uint8_t*)m_mapped_buffer)+entry->m_offset;
	for(int i=0; i<3; ++i) {
		memcpy(buff,mat3.elem[i],sizeof(xvec3f_t));
		buff+=entry->m_matrix_stride;
	}
	return true;
}

bool xuniform_buffer::set_uniform(const char *name, const xmat4f_t mat4)
{
	assert(name);
	unsigned gid=strhash(name);
	xuniform_entry_info *entry = (xuniform_entry_info*)m_uniforms.get(gid);
	if(!entry)
		return false;
	if(GL_FLOAT_MAT4!=entry->m_type)
		return false;
	if(!m_mapped_buffer) 
	{
		if(!m_ubo) 
			return false;
		glBindBuffer(GL_UNIFORM_BUFFER,m_ubo);
		m_mapped_buffer=glMapBuffer(GL_UNIFORM_BUFFER,GL_WRITE_ONLY);
	}
	uint8_t *buff=((uint8_t*)m_mapped_buffer)+entry->m_offset;
	for(int i=0; i<4; ++i) {
		memcpy(buff,mat4.elem[i],sizeof(xvec4f_t));
		buff+=entry->m_matrix_stride;
	}
	return true;
}


}; // namespace wyc
