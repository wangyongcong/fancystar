#pragma once

#ifndef __HEADER_XTNL__
#define __HEADER_XTNL__

#include "xmath.h"
#include "xbuffer.h"

class xvertex_buffer
{
	xvec4f_t *m_pVertexBuffer;
	unsigned m_size;
	unsigned m_pushCursor;
public:
	xvertex_buffer();
	~xvertex_buffer();
	bool create(unsigned num_of_vertex);
	void clear();
	void set_buffer(uint8_t *pdata, unsigned size);
	inline xvec4f_t* get_buffer() {
		return m_pVertexBuffer;
	}
	inline unsigned size() const {
		return m_size;
	}
	inline bool empty() const {
		return m_pVertexBuffer==0;
	}
	inline void push_vertex(const xvec4f_t &vertex) {
		m_pVertexBuffer[m_pushCursor++]=vertex;
	}
	inline void push_vertex(const xvec3f_t &vertex) {
		m_pVertexBuffer[m_pushCursor++]=vertex;
	}
	inline void push_vertex(float x, float y, float z=0, float w=1) {
		m_pVertexBuffer[m_pushCursor++].set(x,y,z,w); 
	}
	inline const xvec4f_t& get_vertex(unsigned idx) {
		return m_pVertexBuffer[idx];
	}
	inline void remove_all() {
		m_pushCursor=0;
	}
};

class xvertex_transformer
{
	enum {
		MATRIX_STACK_SIZE=8,
	};
	struct MATRIX_STACK {
		xmat4f_t m_matrix[MATRIX_STACK_SIZE];
		unsigned m_cursor;
	};
	MATRIX_STACK m_stack_modelview, m_stack_projection;
	MATRIX_STACK *m_pcurstack;
public:
	xvertex_transformer();
	~xvertex_transformer();
	void set_matrix_mode();
	void push_matrix();
	void pop_matrix();
	void load_matrix(const xmat4f_t &mat);
	void load_matrix_t(const xmat4f_t &mat);
	void mul_matrix(const xmat4f_t &mat);
	void mul_matrix_t(const xmat4f_t &mat);
	void rotate(float x, float y, float z, float angle);
	void rotate_x(float angle);
	void rotate_y(float angle);
	void rotate_z(float angle);
	void translate(float x, float y, float z);
	void perspective(float left, float right, float bottom, float top, float znear, float zfar);
	void orthogonal(float left, float right, float bottom, float top, float znear, float zfar);
	void process(xvertex_buffer &vb);
};


#endif // end of __HEADER_XTNL__
