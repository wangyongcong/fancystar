#include "fscorepch.h"
#include "xlayer_render.h"

EXPORT_STATIC_MODULE(xlayer_render);

namespace wyc
{

BEGIN_EVENT_MAP(xlayer_render,xobject2d_render)
END_EVENT_MAP

xlayer_render::xlayer_render() 
{
	m_pTilebuffer=new xtile_buffer;
	// TODO: 不要在构造函数做这种事情
	// 应该能够大致预估所需要的数量
	m_pTilebuffer->reserve(255);
}

void xlayer_render::on_create(xrenderer *pRenderer, xobjevent *pev)
{
	unsigned render_pass, picker_pass;
	if(!((xpackev*)pev)->unpack("dd",&render_pass,&picker_pass))
		return;
	xrenderobj *pobj=pRenderer->get_object(render_pass);
	if(pobj) {
		((xlayer_pass*)pobj)->append(this);
	}
	pobj=pRenderer->get_object(picker_pass);
	if(pobj) {
		((xlayer_pickpass*)pobj)->append(this);
	}
}

void xlayer_render::on_destroy()
{
	xobject2d_render::on_destroy();
	delete m_pTilebuffer;
	m_pTilebuffer=0;
}

void xlayer_render::draw(xrenderer *pRenderer) 
{
	m_pTilebuffer->r_update_buffer();
	xrender_context rc;
	rc.m_vbo=0;
	rc.m_tex=0;
	rc.m_wrap_s=0;
	rc.m_wrap_t=0;
	rc.m_pickType=PICK_QUAD;
	xshader *pShader=pRenderer->current_shader();
	rc.m_uniTrans=pShader->get_uniform("translate");
	rc.m_uniPick=-1;
	r_draw2d(rc);
}

void xlayer_render::draw_picker(xrenderer *pRenderer)
{
	m_pTilebuffer->r_update_buffer();
	xrender_context rc;
	rc.m_vbo=0;
	rc.m_tex=0;
	rc.m_wrap_s=0;
	rc.m_wrap_t=0;
	rc.m_pickType=PICK_QUAD;
	xshader *pShader=pRenderer->current_shader();
	rc.m_uniTrans=pShader->get_uniform("translate");
	rc.m_uniPick=pShader->get_uniform("pickid");
	r_picker(rc);
}

//---------------------------------------------------------------------

BEGIN_EVENT_MAP(xlayer_pass,xrenderobj)
	REG_EVENT(ev_mvp_matrix)
END_EVENT_MAP

xlayer_pass::xlayer_pass()
{
	m_shader2D=strhash("draw2d");
	m_mvp.identity();
}

void xlayer_pass::on_destroy()
{
	m_renderList.clear();
}

void xlayer_pass::on_create(xrenderer*, xobjevent *pev)
{
	int vieww, viewh;
	if(!((xpackev*)pev)->unpack("dd",&vieww,&viewh)) {
		wyc_error("xlayer_pass::on_create: bad args");
		return;
	}
	m_vieww=vieww;
	m_viewh=viewh;
}

void xlayer_pass::draw(xrenderer *pRenderer)
{
	if(!pRenderer->use_shader(m_shader2D))
		return;
	xshader *pShader=pRenderer->current_shader();
	GLint mvp, texmap, trans;
	mvp=pShader->get_uniform("mvp");
	texmap=pShader->get_uniform("texmap");
	trans=pShader->get_uniform("translate");
	glUniformMatrix4fv(mvp,1,GL_FALSE,m_mvp.data());
	glUniform1i(texmap,0);
	glUniform2f(trans,0,0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_SCISSOR_TEST);
	glScissor(0,0,m_vieww,m_viewh);

	glEnableVertexAttribArray(wyc::USAGE_POSITION);
	glEnableVertexAttribArray(wyc::USAGE_COLOR);
	glEnableVertexAttribArray(wyc::USAGE_TEXTURE0);
	
	m_renderList.draw(pRenderer);
	
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glDisableVertexAttribArray(wyc::USAGE_POSITION);
	glDisableVertexAttribArray(wyc::USAGE_COLOR);
	glDisableVertexAttribArray(wyc::USAGE_TEXTURE0);

	glDisable(GL_BLEND);
	glDisable(GL_SCISSOR_TEST);
}

void xlayer_pass::ev_mvp_matrix(xobjevent *pev)
{
	float *mat;
	if(!((xpackev*)pev)->unpack("16f",&mat)) {
		wyc_error("xlayer_render::ev_mvp_matrix: bad args");
		return;
	}
	memcpy(m_mvp.data(),mat,sizeof(float)*16);
}

//---------------------------------------------------------------------

BEGIN_EVENT_MAP(xlayer_pickpass,xlayer_pass)
END_EVENT_MAP

xlayer_pickpass::xlayer_pickpass()
{
	m_shader2D=strhash("pick2d");
	m_pickbuff=0;
	m_vieww=0;
	m_viewh=0;
	m_pickmaps[0]=m_pickmaps[1]=0;
	m_gpu2vram=0;
}

void xlayer_pickpass::on_create(xrenderer*, xobjevent *pev)
{
	int vieww, viewh;
	uint32_t *pbuff;
	if(!((xpackev*)pev)->unpack("ddp",&vieww,&viewh,&pbuff)) {
		wyc_error("xlayer_pickpass::on_create: bad args");
		return;
	}
	assert(pbuff);
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT,viewport);
	assert(viewport[2]==vieww && viewport[3]==viewh);
	m_vieww=vieww, m_viewh=viewh;
	m_pickbuff=pbuff;
	
	size_t size=vieww*viewh*4;
	glGenBuffers(2,m_pickmaps);
	for(int i=0; i<2; ++i) {
		glBindBuffer(GL_PIXEL_PACK_BUFFER,m_pickmaps[i]);
		glBufferData(GL_PIXEL_PACK_BUFFER,size,0,GL_STATIC_READ);
	}
	glBindBuffer(GL_PIXEL_PACK_BUFFER,0);
}

void xlayer_pickpass::draw(xrenderer *pRenderer)
{
	if(!m_pickbuff)
		return;
	if(!pRenderer->use_shader(m_shader2D))
		return;
	xshader *pShader=pRenderer->current_shader();
	GLint mvp, texmap, trans;
	mvp=pShader->get_uniform("mvp");
	texmap=pShader->get_uniform("texmap");
	trans=pShader->get_uniform("translate");
	glUniformMatrix4fv(mvp,1,GL_FALSE,m_mvp.data());
	glUniform1i(texmap,0);
	glUniform2f(trans,0,0);

	glEnableVertexAttribArray(wyc::USAGE_POSITION);
	glEnableVertexAttribArray(wyc::USAGE_TEXTURE0);
	
	m_renderList.draw(pRenderer,draw_picker());
	
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glDisableVertexAttribArray(wyc::USAGE_POSITION);
	glDisableVertexAttribArray(wyc::USAGE_TEXTURE0);

	update_pick_map();

	glClear(GL_COLOR_BUFFER_BIT);
}

void xlayer_pickpass::update_pick_map()
{
	glBindBuffer(GL_PIXEL_PACK_BUFFER,m_pickmaps[m_gpu2vram]);
	glReadBuffer(GL_BACK);
	glReadPixels(0,0,m_vieww,m_viewh,GL_RGBA,GL_UNSIGNED_BYTE,0);
	m_gpu2vram=1-m_gpu2vram;
	glBindBuffer(GL_PIXEL_PACK_BUFFER,m_pickmaps[m_gpu2vram]);
	void *pdata=glMapBuffer(GL_PIXEL_PACK_BUFFER,GL_READ_ONLY);
	memcpy(m_pickbuff,pdata,m_vieww*m_viewh*4);
	glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
	glBindBuffer(GL_PIXEL_PACK_BUFFER,0);
	assert(0==m_pickbuff[m_vieww*m_viewh]);

}

}; // namespace wyc

