#include "fscorepch.h"
#include "wyc/render/vertexbatch.h"
#include "wyc/util/strutil.h"
#include "wyc/util/fjson.h"

EXPORT_MODULE(vertexbatch)

namespace wyc
{

REG_RTTI(xvertex_batch, xresbase)

xvertex_batch::xvertex_batch()
{
	m_vao=0;
	m_vertex_count=0;
	m_mode=GL_TRIANGLES;
	m_prim_count=0;
	m_offset=0;
	m_index_buff=0;
	m_index_type=GL_UNSIGNED_INT;
}

bool xvertex_batch::load(const char *res_name)
{
	if(!async_load(res_name))
		return false;
	on_async_complete();
	return true;
}

bool xvertex_batch::async_load(const char *res_name)
{
	assert(!m_vao);
	wyc_print("xvertex_batch::load: %X [%s]",id(),res_name);
	xjson json;
	if(!json.load_file(res_name))
		return false;
	std::string val;
	val = json.get("type","");
	if(val!="mesh")
	{
		wyc_error("xvertex_batch::create_from_file: not a mesh file");
		return false;
	}
	val = json.get("shape","");
	if(val.empty())
	{
		wyc_error("xvertex_batch::create_from_file: no shape info");
		return false;
	}
	if(val=="cube")
	{
		float size = json.get("size",1.0f);
		generate_mesh_cube(size);
	}
	else if(val=="cylinder")
	{
		float radius = json.get("radius",0.5f);
		unsigned fraction = json.get("fraction",32);
		float height = json.get("height",1.0f);
		generate_mesh_cylinder(radius,fraction,height);
	}
	else if(val=="sphere")
	{
		float radius = json.get("radius",0.5f);
		unsigned fraction = json.get("fraction",32);
		generate_mesh_sphere(radius,fraction);
	}
	else if(val=="torus")
	{
		float radius = json.get("radius",0.5f);
		unsigned fraction = json.get("fraction",32);
		float pipe_radius = json.get("pipe_radius",0.1f);
		unsigned pipe_fraction = json.get("pipe_fraction",8);
		generate_mesh_torus(radius,fraction,pipe_radius,pipe_fraction);
	}
	else 
	{
		wyc_error("xvertex_batch::create_from_file: unknown shape");
		return false;
	}
	return true;
}

void xvertex_batch::on_async_complete()
{
	GLenum glerr=glGetError();
	if(GLEW_ARB_vertex_array_object)
		r_build_vertex_array();
	else 
		r_build_vertex_buffers();
	glerr=glGetError();
	if(glerr!=GL_NO_ERROR) 
		wyc_error("[GL] xvertex_batch::on_async_complete: error code [%d]",glerr);
}

void xvertex_batch::unload()
{
	wyc_print("xvertex_batch::unload: %X",id());
	if(m_vao) {
		glDeleteVertexArrays(1,&m_vao);
		m_vao=0;
	}
	m_index_buff=0;
	xbuffer_attribute *pattr, *pdel;
	unsigned cnt=m_buffers.size();
	for(unsigned i=0; i<cnt; ++i) {
		m_buffers[i].first->decref();
		pattr=m_buffers[i].second;
		while(pattr) {
			pdel=pattr;
			pattr=pattr->m_next;
			delete pdel;
		}
	}
	m_buffers.clear();
}

void xvertex_batch::add_buffer(xvertex_buffer *pbuff, SHADER_USAGE usage, unsigned component, unsigned data_type, \
	bool normalize, unsigned stride, unsigned offset)
{
	assert(pbuff);
	xbuffer_attribute *pattr=new xbuffer_attribute;
	pattr->m_usage=usage;
	pattr->m_component=uint8_t(component);
	pattr->m_type=data_type;
	pattr->m_normalize=normalize;
	pattr->m_stride=stride;
	pattr->m_offset=offset;
	unsigned cnt=m_buffers.size();
	for(unsigned i=0; i<cnt; ++i) {
		if(pbuff==m_buffers[i].first) {
			pattr->m_next=m_buffers[i].second;
			m_buffers[i].second=pattr;
			return;
		}
	}
	pattr->m_next=0;
	pbuff->incref();
	m_buffers.push_back(xbuffer_entry(pbuff,pattr));
}

void xvertex_batch::render() 
{
	if(GLEW_ARB_vertex_array_object) {
		if(0==m_vao)
			return;
		glBindVertexArray(m_vao);
			if(m_index_buff) 
				glDrawElements(m_mode,m_prim_count,m_index_type,(char*)NULL+m_offset);
			else
				glDrawArrays(m_mode,m_offset,m_prim_count);
		glBindVertexArray(0);
	}
	else r_render_buffers();
	GLenum glerr=glGetError();
	if(glerr!=GL_NO_ERROR) 
		wyc_error("[GL] xvertex_batch::render: error code [%d]",glerr);
}

bool xvertex_batch::activate_buffer_as(SHADER_USAGE name, SHADER_USAGE bind_to)
{
	xbuffer_attribute *attr;
	for(size_t i=0, cnt = m_buffers.size(); i<cnt; ++i)
	{
		attr = m_buffers[i].second;
		while(attr)
		{
			if(attr->m_usage==name)
			{
				glBindBuffer(GL_ARRAY_BUFFER,m_buffers[i].first->handle());
				glVertexAttribPointer(bind_to,attr->m_component,attr->m_type,\
					attr->m_normalize?GL_TRUE:GL_FALSE,attr->m_stride,(char*)NULL+attr->m_offset);
				return true;
			}
			attr=attr->m_next;
		}
	}
	return false;
}

void xvertex_batch::r_render_buffers()
{
	xbuffer_attribute *buff_attr;
	unsigned cnt=m_buffers.size();
	for(unsigned i=0; i<cnt; ++i) {
		glBindBuffer(GL_ARRAY_BUFFER,m_buffers[i].first->handle());
		buff_attr=m_buffers[i].second;
		while(buff_attr) {
			glVertexAttribPointer(buff_attr->m_usage,buff_attr->m_component,buff_attr->m_type,\
				buff_attr->m_normalize?GL_TRUE:GL_FALSE,buff_attr->m_stride,(char*)NULL+buff_attr->m_offset);
			glEnableVertexAttribArray(buff_attr->m_usage);
			buff_attr=buff_attr->m_next;
		}
	}
	if(m_index_buff) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m_index_buff->handle());
			glDrawElements(m_mode,m_prim_count,m_index_type,(char*)NULL+m_offset);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
	}
	else {
		glDrawArrays(m_mode,m_offset,m_prim_count);
	}
	for(unsigned i=0; i<cnt; ++i) {
		buff_attr=m_buffers[i].second;
		while(buff_attr) {
			glDisableVertexAttribArray(buff_attr->m_usage);
			buff_attr=buff_attr->m_next;
		}
	}
	glBindBuffer(GL_ARRAY_BUFFER,0);
}

bool xvertex_batch::r_build_vertex_array()
{
	xvertex_buffer *buff;
	xbuffer_attribute *buff_attr;
	unsigned vbuff_count=m_buffers.size();
	if(0==vbuff_count)
		return false;
	if(m_index_buff) 
		vbuff_count+=1;

	GLuint *vbo=new GLuint[vbuff_count];
	glGenBuffers(vbuff_count,vbo);
	glGenVertexArrays(1,&m_vao);
	glBindVertexArray(m_vao);

	unsigned vbo_idx=0;
	std::vector<xbuffer_entry>::iterator iter, end=m_buffers.end();
	for(iter=m_buffers.begin(); iter!=end; ++iter, ++vbo_idx) {
		assert(vbo_idx<vbuff_count);
		assert(vbo[vbo_idx]);
		buff=iter->first;
		buff_attr=iter->second;
		glBindBuffer(GL_ARRAY_BUFFER,vbo[vbo_idx]);
		glBufferData(GL_ARRAY_BUFFER,buff->size(),buff->get_buffer(),buff->usage_hint());
		while(buff_attr) {
			glVertexAttribPointer(buff_attr->m_usage,buff_attr->m_component,buff_attr->m_type,\
				buff_attr->m_normalize,buff_attr->m_stride,(void*)buff_attr->m_offset);
			glEnableVertexAttribArray(buff_attr->m_usage);
			buff_attr=buff_attr->m_next;
		}
		buff->set_handle(vbo[vbo_idx]);
		buff->release_buffer();
	}
	if(m_index_buff) {
		assert(vbo_idx<vbuff_count);
		assert(vbo[vbo_idx]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vbo[vbo_idx]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,m_index_buff->size(),m_index_buff->get_buffer(),m_index_buff->usage_hint());
		m_index_buff->set_handle(vbo[vbo_idx]);
		m_index_buff->release_buffer();
	}

	glBindVertexArray(0);
	delete [] vbo;
	return true;
}

bool xvertex_batch::r_build_vertex_buffers()
{
	xvertex_buffer *buff;
	unsigned vbuff_count=m_buffers.size();
	if(0==vbuff_count)
		return false;
	if(m_index_buff) 
		vbuff_count+=1;

	GLuint *vbo=new GLuint[vbuff_count];
	glGenBuffers(vbuff_count,vbo);
	unsigned vbo_idx=0;
	std::vector<xbuffer_entry>::iterator iter, end=m_buffers.end();
	for(iter=m_buffers.begin(); iter!=end; ++iter, ++vbo_idx) {
		assert(vbo_idx<vbuff_count);
		assert(vbo[vbo_idx]);
		buff=iter->first;
		glBindBuffer(GL_ARRAY_BUFFER,vbo[vbo_idx]);
		glBufferData(GL_ARRAY_BUFFER,buff->size(),buff->get_buffer(),buff->usage_hint());
		buff->set_handle(vbo[vbo_idx]);
		buff->release_buffer();
	}
	glBindBuffer(GL_ARRAY_BUFFER,0);
	if(m_index_buff) {
		assert(vbo_idx<vbuff_count);
		assert(vbo[vbo_idx]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vbo[vbo_idx]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,m_index_buff->size(),m_index_buff->get_buffer(),m_index_buff->usage_hint());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
		m_index_buff->set_handle(vbo[vbo_idx]);
		m_index_buff->release_buffer();
	}
	delete [] vbo;
	return true;
}

void xvertex_batch::generate_mesh_cube(float size)
{
	assert(sizeof(xvec3f_t)==sizeof(GLfloat)*3);
	size*=0.5f;
	GLfloat cubeVertex[]={
		// front face
		-size,  size,  size, 
		-size, -size,  size,
		 size, -size,  size, 
		 size,  size,  size,
		// right face
		 size,  size,  size,
		 size, -size,  size, 
		 size, -size, -size, 
		 size,  size, -size,
		// back face
		 size,  size, -size,
		 size, -size, -size, 
		-size, -size, -size,
		-size,  size, -size, 
		// left face
		-size,  size, -size, 
		-size, -size, -size,
		-size, -size,  size,
		-size,  size,  size, 
		// top face
		-size,  size, -size, 
		-size,  size,  size,
		 size,  size,  size,
		 size,  size, -size, 
		// bottom face
		-size, -size,  size,
		-size, -size, -size, 
		 size, -size, -size, 
		 size, -size,  size,
	};
	const size_t vertex_count = sizeof(cubeVertex)/(sizeof(GLfloat)*3);
	// pos:3 + normal:3 + uv:2 + tangent:4
	const size_t size_buffer = vertex_count*sizeof(GLfloat)*12;
	xvertex_buffer *vb = wycnew xvertex_buffer();
	vb->alloc_buffer(size_buffer,GL_STATIC_DRAW);
	size_t offset=0;
	xvec3f_t *vertex;
	xvec3f_t *normal;
	xvec2f_t *uv;
	xvec4f_t *tangent;

	// vertex position
	vertex = (xvec3f_t*)vb->get_buffer();
	memcpy(vertex,cubeVertex,sizeof(cubeVertex));
	add_buffer(vb,USAGE_POSITION,3,GL_FLOAT,false,0,offset);
	offset+=sizeof(xvec3f_t)*vertex_count;

	// vertex normal
	GLfloat cubeNormal[]={
		// front face
		0, 0, 1,
		// right face
		1, 0, 0,
		// back face
		0, 0, -1,
		// left face
		-1, 0, 0,
		// top face
		0, 1, 0,
		// bottom face
		0, -1, 0,
	};
	normal = (xvec3f_t*)(vb->get_buffer()+offset);
	xvec3f_t *vec3 = (xvec3f_t*)cubeNormal;
	for(size_t i=0; i<vertex_count; ++vec3)
	{
		normal[i++] = *vec3;
		normal[i++] = *vec3;
		normal[i++] = *vec3;
		normal[i++] = *vec3;
	}
	add_buffer(vb,USAGE_NORMAL,3,GL_FLOAT,false,0,offset);
	offset+=sizeof(xvec3f_t)*vertex_count;

	// texture coordinates
	uv = (xvec2f_t*)(vb->get_buffer()+offset);
	xvec2f_t *vec2 = uv;
	for(size_t i=0; i<6; ++i)
	{
		vec2++->set(0,1.0f);
		vec2++->set(0,0);
		vec2++->set(1.0f,0);
		vec2++->set(1.0f,1.0f);
	}
	assert(vec2==uv+vertex_count);
	add_buffer(vb,USAGE_TEXTURE0,2,GL_FLOAT,false,0,offset);
	offset+=sizeof(xvec2f_t)*vertex_count;

	// calculate tangent vectors
	tangent = (xvec4f_t*)(vb->get_buffer()+offset);
	add_buffer(vb,USAGE_TANGENT,4,GL_FLOAT,false,0,offset);
	offset+=sizeof(xvec4f_t)*vertex_count;
	assert(offset==size_buffer);
	
	// index buffer
	vb=wycnew xvertex_buffer;
	vb->alloc_buffer(sizeof(GLushort)*36,GL_STATIC_DRAW);
	GLushort *index = (GLushort*)vb->get_buffer();
	GLushort base = 0;
	for(size_t i=0; i<36; base+=4)
	{
		index[i++]=base;
		index[i++]=base+1;
		index[i++]=base+2;
		index[i++]=base;
		index[i++]=base+2;
		index[i++]=base+3;
	}
	set_index(vb,GL_UNSIGNED_SHORT);
	set_mode(GL_TRIANGLES,36,0);
	m_vertex_count = vertex_count;

	calculate_tangents(vertex_count,vertex,normal,uv,36,index,tangent);
}

void xvertex_batch::generate_mesh_cylinder(float radius, unsigned fraction, float height)
{
	const unsigned quarter = fraction>>2;
	const unsigned cap = (quarter<<2)+1;
	const unsigned half_circle = (quarter<<1)+1;
	const unsigned circle = half_circle<<1;

	const size_t vertex_count=(circle+cap)<<1; 
	// pos:3 + normal:3 + uv:2 + tangent:4
	const size_t size_buffer = vertex_count*sizeof(GLfloat)*12;
	xvertex_buffer *vb = wycnew xvertex_buffer;
	vb->alloc_buffer(size_buffer,GL_STATIC_DRAW);
	xvec3f_t *vertex;
	xvec3f_t *normal;
	xvec2f_t *uv;
	xvec4f_t *tangent;
	size_t offset=0;
	
	// calculate vertex position
	float step=float(XMATH_HPI/quarter);
	float rad=step;
	float hh=height*0.5f;
	vertex = (xvec3f_t*)vb->get_buffer();
	xvec3f_t *part1, *part2, *part3, *part4;
	part1 = vertex;
	part1++->set(0,hh,0);
	part2 = part1+quarter;
	part3 = part2+quarter;
	part4 = part3+quarter;
	part1->set(radius,hh,0);
	part2->set(0,hh,radius);
	part3->set(-radius,hh,0);
	part4->set(0,hh,-radius);
	part2+=quarter;
	part4+=quarter;
	for(unsigned i=1; i<quarter; ++i, rad+=step)
	{
		++part1;
		part1->set(cos(rad)*radius,hh,sin(rad)*radius);
		--part2;
		part2->set(-part1->x,hh,part1->z);
		++part3;
		part3->set(-part1->x,hh,-part1->z);
		--part4;
		part4->set(part1->x,hh,-part1->z);
	}
	part1=vertex;
	part2=part1+cap;
	for(unsigned i=0; i<cap; ++i, ++part1)
	{
		part2++->set(part1->x,-hh,part1->z);
	}
	part1=vertex+1;
	memcpy(part2,part1,sizeof(xvec3f_t)*half_circle);
	memcpy(part2+half_circle,part1+(quarter<<1),sizeof(xvec3f_t)*(half_circle-1));
	part2[circle-1]=*part1;
	part1+=cap;
	part2+=circle;
	memcpy(part2,part1,sizeof(xvec3f_t)*half_circle);
	memcpy(part2+half_circle,part1+(quarter<<1),sizeof(xvec3f_t)*(half_circle-1));
	part2[circle-1]=*part1;
	assert(part2+circle==(xvec3f_t*)vb->get_buffer()+vertex_count);
	add_buffer(vb,USAGE_POSITION,3,GL_FLOAT,false,0,0);
	offset += sizeof(xvec3f_t)*vertex_count;

	// vertex normals
	normal = (xvec3f_t*)(vb->get_buffer()+offset);
	part1 = normal;
	part2 = part1+cap;
	for(unsigned i=0; i<cap; ++i)
	{
		part1++->set(0,1,0);
		part2++->set(0,-1,0);
	}
	part1 = part2;
	part2 = part1+circle;
	part3 = vertex+(cap<<1);
	step = 1.0f/radius;
	for(unsigned i=0; i<circle; ++i)
	{
		part1->set(part3->x*step,0,part3->z*step);
		*part2=*part1;
		++part1;
		++part2;
		++part3;
	}
	assert(part2==(xvec3f_t*)(vb->get_buffer()+offset)+vertex_count);
	add_buffer(vb,USAGE_NORMAL,3,GL_FLOAT,false,0,offset);
	offset += sizeof(xvec3f_t)*vertex_count;

	// texture coordinates
	uv = (xvec2f_t*)(vb->get_buffer()+offset);
	xvec2f_t *uv_iter1 = uv, *uv_iter2, *uv_iter3, *uv_iter4;
	uv_iter1++->set(0.5f,0.5f);
	uv_iter2=uv_iter1+quarter;
	uv_iter3=uv_iter2+quarter;
	uv_iter4=uv_iter3+quarter;
	uv_iter1->set(1,0.5f);
	uv_iter2->set(0.5f,0);
	uv_iter3->set(0,0.5f);
	uv_iter4->set(0.5f,1);
	uv_iter2+=quarter;
	uv_iter4+=quarter;
	part1=normal+(cap<<1)+1;
	for(unsigned i=1; i<quarter; ++i, ++part1)
	{
		rad=part1->x*0.5f;
		step=part1->z*0.5f;
		++uv_iter1;
		uv_iter1->set(0.5f+rad,0.5f-step);
		--uv_iter2;
		uv_iter2->set(0.5f-rad,0.5f-step);
		++uv_iter3;
		uv_iter3->set(0.5f-rad,0.5f+step);
		--uv_iter4;
		uv_iter4->set(0.5f+rad,0.5f+step);
	}
	uv_iter1=uv+cap;
	memcpy(uv_iter1,uv,sizeof(xvec2f_t)*cap);
	step = 0.5f/quarter;
	rad = 1;
	uv_iter1+=cap;
	uv_iter2=uv_iter1+circle;
	uv_iter3=uv_iter1;
	uv_iter4=uv_iter2;
	uv_iter1++->set(1,1);
	uv_iter2++->set(1,0);
	for(unsigned i=2; i<half_circle; ++i)
	{
		rad-=step;
		uv_iter1++->set(rad,1);
		uv_iter2++->set(rad,0);
	}
	uv_iter1++->set(0,1);
	uv_iter2++->set(0,0);
	memcpy(uv_iter1,uv_iter3,sizeof(xvec2f_t)*half_circle);
	memcpy(uv_iter2,uv_iter4,sizeof(xvec2f_t)*half_circle);
	assert(uv_iter2+half_circle==uv+vertex_count);
	add_buffer(vb,USAGE_TEXTURE0,2,GL_FLOAT,false,0,offset);
	offset += sizeof(xvec2f_t)*vertex_count;

	// tangent vectors
	tangent = (xvec4f_t*)(vb->get_buffer()+offset);
	add_buffer(vb,USAGE_TANGENT,4,GL_FLOAT,false,0,offset);
	offset += sizeof(xvec4f_t)*vertex_count;
	assert(offset==size_buffer);

	// index buffer
	const unsigned triangle_count = quarter<<4;
	const unsigned index_count = triangle_count*3;
	assert(index_count<65536);
	xvertex_buffer *ibuff=wycnew xvertex_buffer;
	ibuff->alloc_buffer(sizeof(GLushort)*index_count,GL_STATIC_DRAW);
	GLushort *index=(GLushort*)ibuff->get_buffer();
	GLushort *idx_iter=index;
	GLushort vidx=1, next_vidx;
	for(unsigned i=2; i<cap; ++i, ++vidx)
	{
		*idx_iter++=0;
		*idx_iter++=vidx+1;
		*idx_iter++=vidx;
	}
	*idx_iter++=0;
	*idx_iter++=1;
	*idx_iter++=vidx;
	vidx=cap+1;
	for(unsigned i=2; i<cap; ++i, ++vidx)
	{
		*idx_iter++=cap;
		*idx_iter++=vidx;
		*idx_iter++=vidx+1;
	}
	*idx_iter++=cap;
	*idx_iter++=vidx;
	*idx_iter++=cap+1;
	vidx=cap<<1;
	next_vidx=vidx+circle;
	for(unsigned i=1; i<half_circle; ++i)
	{
		*idx_iter++=vidx;
		*idx_iter++=vidx+1;
		*idx_iter++=next_vidx+1;
		*idx_iter++=vidx;
		*idx_iter++=next_vidx+1;
		*idx_iter++=next_vidx;
		++vidx;
		++next_vidx;
	}
	for(GLushort *beg=idx_iter-((half_circle-1)*6), *end=idx_iter; beg!=end; ++beg)
	{
		*idx_iter++=*beg+half_circle;
	}
	assert(idx_iter==index+index_count);
	set_index(ibuff,GL_UNSIGNED_SHORT);
	set_mode(GL_TRIANGLES,index_count,0);

	// finally, calculate tangents
	calculate_tangents(vertex_count,vertex,normal,uv,index_count,index,tangent);

	m_vertex_count = vertex_count;
}

void xvertex_batch::generate_mesh_sphere(float radius, unsigned fraction)
{
	assert(sizeof(float)==sizeof(GLfloat));
	assert(fraction>4);
	// X-Z平面上,1/4圆切分多少段
	const unsigned quarter=fraction>>2;
	// X-Z平面上,半圆的顶点数(两个半圆之间有接缝)
	const unsigned half_circle=quarter*2+1; 
	// X-Z平面上,大圆的顶点数
	const unsigned circle=half_circle*2;
	// 顶(底)部半圆盖的顶点数
	// 通过复制最顶(底)部顶点数并赋予合适的UV值,来降低顶(底)部的变形
	const unsigned half_cap = quarter*2;
	float step=float(XMATH_HPI/quarter);
	float rad=step;
	// pre-compute the sin & cos
	float *sina=new float[quarter], *cosa=new float[quarter];
	sina[0]=0, cosa[0]=1;
	for(unsigned i=1; i<quarter; ++i) {
		sina[i]=sin(rad);
		cosa[i]=cos(rad);
		rad+=step;
	}
	const size_t vertex_count=circle*(2*quarter-1)+4*half_cap; 
	// pos:3 + normal:3 + uv:2 + tangent:4
	const size_t size_buffer = vertex_count*sizeof(GLfloat)*12;
	xvertex_buffer *vb = wycnew xvertex_buffer;
	vb->alloc_buffer(size_buffer,GL_STATIC_DRAW);
	size_t offset=0;
	xvec3f_t *vertex;
	xvec3f_t *normal;
	xvec2f_t *uv;
	xvec4f_t *tangent;
	//------------------------------------
	// 1, 计算+Y/+Z上的1/4球
	// 2, 根据对称,复制1/4球的数据到-Y/+Z
	// 3, 根据对称,复制数据到-Z半球
	//------------------------------------
	// vertex positions
	vertex = (xvec3f_t*)vb->get_buffer();
	xvec3f_t *part1=vertex, *part2;
	// top half of sphere
	for(unsigned i=0; i<half_cap; ++i)
		part1++->set(0,radius,0);
	for(int i=quarter-1; i>=0; --i) {
		float y=radius*sina[i], len=radius*cosa[i];
		part2=part1+quarter;
		part1->set(len,y,0);
		part2->set(0,y,len);
		part2+=quarter;
		part2->set(-len,y,0);
		for(unsigned j=1; j<quarter; ++j) {
			float x=len*cosa[j], z=len*sina[j];
			++part1;
			part1->set(x,y,z);
			--part2;
			part2->set(-x,y,z);
		}
		part1=part1+quarter+2;
	}
	// bottom half of sphere
	part2=part1;
	part1-=circle;
	for(int i=quarter-1; i>0; --i) {
		for(unsigned j=0; j<half_circle; ++j, ++part1) 
			part2++->set(part1->x,-part1->y,part1->z);
		part1-=circle;
	}
	for(unsigned i=0; i<half_cap; ++i)
		part2++->set(0,-radius,0);
	// mirror to -z
	part1=vertex;
	for(int i=vertex_count>>1; i>0; --i, ++part1) 
		part2++->set(-part1->x,part1->y,-part1->z);
	assert(part2==vertex+vertex_count);
	add_buffer(vb,USAGE_POSITION,3,GL_FLOAT,false,0,offset);
	offset += sizeof(xvec3f_t)*vertex_count;
	// clean up
	delete [] sina;
	delete [] cosa;
	sina = 0;
	cosa = 0;

	// normal buffer
	normal = (xvec3f_t*)(vb->get_buffer()+offset);
	memcpy(normal,vertex,sizeof(xvec3f_t)*vertex_count);
	for(unsigned i=0; i<vertex_count; ++i)
		normal[i].normalize();
	add_buffer(vb,USAGE_NORMAL,3,GL_FLOAT,false,0,offset);
	offset += sizeof(xvec3f_t)*vertex_count;

	// texture coordinates
	uv = (xvec2f_t*)(vb->get_buffer()+offset);
	xvec2f_t *uv1=uv, *uv2;
	step = 0.5f/quarter;
	float u = step*0.5f, v;
	for(unsigned i=0; i<half_cap; ++i) {
		uv1++->set(u,1.0f);
		u+=step;
	}
	for(int i=quarter-1; i>=0; --i)
	{
		u=0, v=i*step+0.5f;
		uv2=uv1+quarter;
		uv1->set(0.0f,v);
		uv2->set(0.5f,v);
		uv2+=quarter;
		uv2->set(1.0f,v);
		for(unsigned j=1; j<quarter; ++j)
		{
			u+=step;
			++uv1;
			uv1->set(u,v);
			--uv2;
			uv2->set(1.0f-u,v);
		}
		uv1=uv1+quarter+2;
	}
	uv2=uv1;
	uv1-=circle;
	for(int i=quarter-1; i>0; --i) {
		for(unsigned j=0; j<half_circle; ++j, ++uv1) 
			uv2++->set(uv1->x,1.0f-uv1->y);
		uv1-=circle;
	}
	u = step*0.5f;
	for(unsigned i=0; i<half_cap; ++i) {
		uv2++->set(u,0.0f);
		u+=step;
	}
	assert(uv2==uv+(vertex_count>>1));
	// mirror to -z
	memcpy(uv2,uv,sizeof(xvec2f_t)*(vertex_count>>1));
	add_buffer(vb,USAGE_TEXTURE0,2,GL_FLOAT,false,0,offset);
	offset += sizeof(xvec2f_t)*vertex_count;

	// tangent vectors
	tangent=(xvec4f_t*)(vb->get_buffer()+offset);
	add_buffer(vb,USAGE_TANGENT,4,GL_FLOAT,false,0,offset);
	offset += sizeof(xvec4f_t)*vertex_count;
	assert(offset==size_buffer);

	// index buffer
	unsigned index_count=(half_circle-1)*(12*quarter-6)*2;
	assert(index_count<65536);
	xvertex_buffer *ibuff=wycnew xvertex_buffer;
	ibuff->alloc_buffer(sizeof(GLushort)*index_count,GL_STATIC_DRAW);
	GLushort *index=(GLushort*)ibuff->get_buffer();
	GLushort *idx_iter=index;
	GLushort vidx=0;
	while(vidx<half_cap) {
		*idx_iter++=vidx;
		*idx_iter++=vidx+half_cap+1;
		*idx_iter++=vidx+half_cap;
		++vidx;
	}
	for(int i=2*(quarter-1); i>0; --i, ++vidx) {
		for(unsigned j=1; j<half_circle; ++j)
		{
			idx_iter[0]=vidx;
			idx_iter[1]=vidx+1;
			idx_iter[2]=GLushort(vidx+half_circle);
			idx_iter[3]=idx_iter[2];
			idx_iter[4]=idx_iter[1];
			idx_iter[5]=GLushort(idx_iter[1]+half_circle);
			idx_iter+=6;
			++vidx;
		}
	}
	for(unsigned i=1; i<half_circle; ++i, ++vidx)
	{
		*idx_iter++=vidx+half_circle;
		*idx_iter++=vidx;
		*idx_iter++=vidx+1;
	}
	// the -z half part
	vidx += half_cap+1;
	for(GLushort *beg=index, *end=idx_iter; beg!=end; ++beg)
	{
		*idx_iter++ = vidx+*beg;
	}
	assert((idx_iter-index)==index_count);
	set_index(ibuff,GL_UNSIGNED_SHORT);
	set_mode(GL_TRIANGLES,index_count,0);

	calculate_tangents(vertex_count,vertex,normal,uv,index_count,index,tangent);

	m_vertex_count = vertex_count;
}

void xvertex_batch::generate_mesh_torus(float main_radius, unsigned main_fraction, float pipe_radius, unsigned pipe_fraction)
{
	const unsigned quarter = main_fraction>>2;
	const unsigned half_circle = quarter*2+1;
	const unsigned circle = half_circle*2;
	const unsigned pipe_quarter = pipe_fraction>>2;
	const unsigned pipe_circle = (pipe_quarter<<2)+1;
	float step = float(XMATH_HPI/quarter);
	float rad = step;
	wyc::xvec2f_t *axis=new xvec2f_t[half_circle], *part1, *part2, *part3, *part4;
	axis[0].set(1,0);
	axis[quarter].set(0,1);
	axis[half_circle-1].set(-1,0);
	part1 = &axis[half_circle-2];
	for(unsigned i=1; i<quarter; ++i, rad+=step, --part1)
	{
		part1->set(-cos(rad),sin(rad));
		axis[i].set(-part1->x,part1->y);
	}
	assert(part1==axis+quarter);
	// pre-compute sin and cos
	step = float(XMATH_HPI/pipe_quarter);
	rad = step;
	xvec2f_t *pipe_axis = new xvec2f_t[pipe_circle];
	part1 = pipe_axis;
	part2 = part1+pipe_quarter;
	part3 = part2+pipe_quarter;
	part4 = part3+pipe_quarter;
	part1->set(-pipe_radius, 0);
	part2->set( 0,-pipe_radius);
	part3->set( pipe_radius, 0);
	part4->set( 0, pipe_radius);
	part2+=pipe_quarter;
	part4+=pipe_quarter;
	assert(part4-pipe_axis==pipe_circle-1);
	part4->set(-pipe_radius, 0);
	for(unsigned i=1; i<pipe_quarter; ++i, rad+=step)
	{
		++part3;
		part3->set(cos(rad)*pipe_radius,sin(rad)*pipe_radius);
		--part4;
		part4->set(-part3->x,part3->y);
		++part1;
		part1->set(-part3->x,-part3->y);
		--part2;
		part2->set(part3->x,-part3->y);
	}

	const size_t vertex_count = circle*pipe_circle;
	// pos:3 + normal:3 + tangent:3 + uv:2
	const size_t size_buffer = vertex_count*sizeof(GLfloat)*12;
	xvertex_buffer *vb = wycnew xvertex_buffer;
	vb->alloc_buffer(size_buffer,GL_STATIC_DRAW);
	size_t offset=0;
	xvec3f_t *vertex;
	xvec3f_t *normal;
	xvec2f_t *uv;
	xvec4f_t *tangent;
	
	// vertex position and normals
	vertex = (xvec3f_t*)vb->get_buffer();
	normal = vertex + vertex_count;
	xvec3f_t *vec=vertex, *n=normal; 
	for(unsigned i=0; i<half_circle; ++i)
	{
		part1 = axis+i;
		part2 = pipe_axis;
		for(unsigned j=0; j<pipe_circle; ++j, ++part2)
		{
			n->set(part1->x*part2->x, part2->y, part1->y*part2->x);
			n->normalize();
			step = main_radius + part2->x;
			vec->set(part1->x*step, part2->y, part1->y*step);
			++vec;
			++n;
		}
	}
	assert(vec==vertex+(vertex_count>>1));
	// mirror to -z
	for(xvec3f_t *iter=vec-pipe_circle, *beg=(xvec3f_t*)vb->get_buffer(); iter>=beg; iter-=pipe_circle)
	{
		for(unsigned i=0; i<pipe_circle; ++i)
		{
			vec++->set(iter[i].x,iter[i].y,-iter[i].z);
		}
	}
	for(xvec3f_t *iter=n-pipe_circle, *beg=(xvec3f_t*)vb->get_buffer()+vertex_count; iter>=beg; iter-=pipe_circle)
	{
		for(unsigned i=0; i<pipe_circle; ++i)
		{
			n++->set(iter[i].x,iter[i].y,-iter[i].z);
		}
	}
	assert(vec==vertex+vertex_count);
	add_buffer(vb,USAGE_POSITION,3,GL_FLOAT,false,0,0);
	add_buffer(vb,USAGE_NORMAL,3,GL_FLOAT,false,0,sizeof(xvec3f_t)*vertex_count);
	offset += sizeof(xvec3f_t)*vertex_count*2;
	// clean up
	delete [] pipe_axis;
	delete [] axis;
	pipe_axis = 0;
	axis = 0;

	// texture uv
	step = 1.0f/(half_circle-1);
	rad = 1.0f/(pipe_circle-1);
	float u=0, v;
	uv = (xvec2f_t*)(vb->get_buffer()+offset);
	part1 = uv;
	for(unsigned i=1; i<half_circle; ++i, u+=step)
	{
		v=0;
		for(unsigned j=1; j<pipe_circle; ++j, v+=rad)
			part1++->set(u,v);
		part1++->set(u,1.0f);
	}
	v=0;
	for(unsigned j=1; j<pipe_circle; ++j, v+=rad)
		part1++->set(1.0f,v);
	part1++->set(1.0f,1.0f);
	assert(part1==uv+(vertex_count>>1));
	memcpy(part1,uv,sizeof(xvec2f_t)*(vertex_count>>1));
	add_buffer(vb,USAGE_TEXTURE0,2,GL_FLOAT,false,0,offset);
	offset += sizeof(xvec2f_t)*vertex_count;

	// tangents
	tangent = (xvec4f_t*)(vb->get_buffer()+offset);
	add_buffer(vb,USAGE_TANGENT,4,GL_FLOAT,false,0,offset);
	offset += sizeof(xvec4f_t)*vertex_count;

	assert(offset==size_buffer);
	
	// index buffer
	unsigned index_count = (pipe_circle-1)*quarter*24;
	assert(index_count<65536);
	xvertex_buffer *ibuff=wycnew xvertex_buffer;
	ibuff->alloc_buffer(sizeof(GLushort)*index_count,GL_STATIC_DRAW);
	GLushort *index=(GLushort*)ibuff->get_buffer();
	GLushort *idx_iter=index;
	GLushort vidx=0, next_vidx;;
	for(unsigned i=1; i<half_circle; ++i, ++vidx)
	{
		for(unsigned j=1; j<pipe_circle; ++j, ++vidx)
		{
			next_vidx=vidx+pipe_circle;
			*idx_iter++=vidx;
			*idx_iter++=vidx+1;
			*idx_iter++=next_vidx+1;
			*idx_iter++=vidx;
			*idx_iter++=next_vidx+1;
			*idx_iter++=next_vidx;
		}
	}
	assert(idx_iter-index == (index_count>>1));
	vidx += pipe_circle;
	for(GLushort *beg = index, *end = idx_iter; beg!=end; ++beg)
	{
		*idx_iter++ = vidx + *beg;
	}
	assert(idx_iter-index == index_count);
	set_index(ibuff,GL_UNSIGNED_SHORT);
	set_mode(GL_TRIANGLES,index_count,0);
	
	calculate_tangents(vertex_count, vertex, normal, uv, index_count, index, tangent);
	
	m_vertex_count = vertex_count;
}

void xvertex_batch::calculate_tangents(unsigned vertexCount, const xvec3f_t *vertex, const xvec3f_t *normal, 
									  const xvec2f_t *texcoord, unsigned indexCount, const unsigned short *index, xvec4f_t *tangent)
{
	xvec3f_t *tan1 = new xvec3f_t[vertexCount * 2];
	xvec3f_t *tan2 = tan1 + vertexCount;
	ZeroMemory(tan1, vertexCount * sizeof(xvec3f_t) * 2);

	for (unsigned a = 2; a < indexCount; a+=3, index+=3)
	{
		long i1 = index[0];
		long i2 = index[1];
		long i3 = index[2];

		const xvec3f_t& v1 = vertex[i1];
		const xvec3f_t& v2 = vertex[i2];
		const xvec3f_t& v3 = vertex[i3];

		const xvec2f_t& w1 = texcoord[i1];
		const xvec2f_t& w2 = texcoord[i2];
		const xvec2f_t& w3 = texcoord[i3];

		float x1 = v2.x - v1.x;
		float x2 = v3.x - v1.x;
		float y1 = v2.y - v1.y;
		float y2 = v3.y - v1.y;
		float z1 = v2.z - v1.z;
		float z2 = v3.z - v1.z;

		float s1 = w2.x - w1.x;
		float s2 = w3.x - w1.x;
		float t1 = w2.y - w1.y;
		float t2 = w3.y - w1.y;

		float r = 1.0F / (s1 * t2 - s2 * t1);
		xvec3f_t sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
		xvec3f_t tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);

		tan1[i1] += sdir;
		tan1[i2] += sdir;
		tan1[i3] += sdir;

		tan2[i1] += tdir;
		tan2[i2] += tdir;
		tan2[i3] += tdir;
	}

	for (unsigned a = 0; a < vertexCount; a++)
	{
		const xvec3f_t& n = normal[a];
		const xvec3f_t& t = tan1[a];
        
		// Gram-Schmidt orthogonalize
		xvec3f_t v = t - n * n.dot(t);
		v.normalize();
		tangent[a] = v;
        
		// Calculate handedness
		v.cross(n,t);
		tangent[a].w = v.dot(tan2[a]) < 0.0f ? -1.0f : 1.0f;
	}
    
    delete[] tan1;
}

}; // namespace wyc


