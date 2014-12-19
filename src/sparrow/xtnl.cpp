#include "xtnl.h"

xvertex_buffer::xvertex_buffer()
{
	m_pVertexBuffer=0;
	m_size=0;
	m_pushCursor=0;
}

xvertex_buffer::~xvertex_buffer()
{
}

bool xvertex_buffer::create(unsigned num_of_vertex)
{
	if(!empty()) 
		clear();
	m_pVertexBuffer=new xvec4f_t[num_of_vertex];
	m_size=num_of_vertex;
	m_pushCursor=0;
	return true;
}

void xvertex_buffer::clear()
{
	if(m_pVertexBuffer) {
		delete [] m_pVertexBuffer;
		m_pVertexBuffer=0;
		m_size=0;
		m_pushCursor=0;
	}
}

void xvertex_buffer::set_buffer(uint8_t *pdata, unsigned size) 
{
	if(!empty()) 
		clear();
	m_pVertexBuffer=(xvec4f_t*)pdata;
	m_size=size;
	m_pushCursor=0;
}

//==xvertex_transformer===========================================================================

xvertex_transformer::xvertex_transformer()
{
}

xvertex_transformer::~xvertex_transformer()
{
}


