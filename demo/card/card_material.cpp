#include "fscorepch.h"
#include "card_material.h"
#include "wyc/util/fjson.h"
#include "wyc/game/glb_game.h"

#include "shaderlib.h"

namespace wyc
{

REG_RTTI(xcard_renderer, xobject);

void xcard_renderer::_on_load_layout (xcard_renderer *rc, xresbase *res)
{
	if(rc->m_card_layout.def_font==res)
	{
		rc->m_completion|=CARD_FONT_DEFAULT;
	}
	else if(rc->m_card_layout.back_face==res)
	{
		rc->m_completion|=CARD_BACKFACE_TEXTURE;
		rc->_on_backface_changed();
	}
	else if(rc->m_card_layout.front_face==res)
	{
		rc->m_completion|=CARD_FRONTFACE_TEXTURE;
	}
}

xcard_renderer::xcard_renderer()
{
	m_vbo_card=m_ibo_card=m_vbo_card_texcoord=0;
	m_texbuff_cap=0;
	m_texbuff_size=0;
	m_backface_texcoord=-1;
	m_fbo_layout=0;
	m_vbo_layout=0;
	m_vbo_layout_size=0;
	m_completion=0;
	m_atlas=0;
	m_flush_atlas=false;
//------------------------------------------------------------------
	m_debug_info.atlas_mesh=0;
	m_debug_info.atlas_size.set(0,0,0,0);
	m_debug_info.atlas_texture=0;
}

void xcard_renderer::on_destroy()
{
	if(m_vbo_card)
	{
		GLuint vbo[3] = {m_vbo_card,m_ibo_card,m_vbo_card_texcoord};
		glDeleteBuffers(3,vbo);
		m_vbo_card=0;
		m_ibo_card=0;
		m_vbo_card_texcoord=0;
	}
	if(m_fbo_layout) {
		glDeleteFramebuffers(1,&m_fbo_layout);
		m_fbo_layout=0;
	}
	if(m_vbo_layout)
	{
		glDeleteBuffers(1,&m_vbo_layout);
		m_vbo_layout=0;
		m_vbo_layout_size=0;
	}
	slot_t *s;
	xdict::iterator iter, end=m_cards.end();
	for(iter=m_cards.begin(); iter!=end; ++iter) {
		s=(slot_t*)(iter->second);
		s->card=0;
		s->avatar=0;
	}
	_clear_atlas();
	m_card_layout.def_font=0;
	m_card_layout.back_face=0;
	m_card_layout.front_face=0;
}

bool xcard_renderer::load_card_layout(const char *file_name)
{
	xjson json;
	if(!json.load_file(file_name))
		return false;
	const vjson::json_value *root = json.get_root();
	if(!root || root->type!=vjson::JSON_OBJECT)
		return false;
	const vjson::json_value *areas = 0;
	
	std::string path;
	xressvr *svr = get_resource_server();
	for(const vjson::json_value *iter = root->first_child; iter; iter=iter->next_sibling)
	{
		if(0==strcmp(iter->name,"area"))
			areas = iter;
		else if(0==strcmp(iter->name,"width"))
			m_card_layout.width=iter->int_value;
		else if(0==strcmp(iter->name,"height"))
			m_card_layout.height=iter->int_value;
		else if(0==strcmp(iter->name,"font")) {
			path = iter->string_value;
			get_resource_path(path);
			m_card_layout.def_font = (xfont*)svr->async_request(xfont::get_class()->id,path.c_str(),\
			this,(xressvr::resource_callback_t)_on_load_layout);
			if(m_card_layout.def_font->is_complete())
				add_state(m_completion,CARD_FONT_DEFAULT);
			else
				remove_state(m_completion,CARD_FONT_DEFAULT);
		}
		else if(0==strcmp(iter->name,"back_face")) {
			path = iter->string_value;
			get_resource_path(path);
			m_card_layout.back_face = (xtexture*)svr->async_request(xtexture::get_class()->id,path.c_str(),\
			this,(xressvr::resource_callback_t)_on_load_layout);
			if(m_card_layout.back_face->is_complete())
				add_state(m_completion,CARD_BACKFACE_TEXTURE);
			else
				remove_state(m_completion,CARD_BACKFACE_TEXTURE);			
		}
		else if(0==strcmp(iter->name,"front_face")) {
			path = iter->string_value;
			get_resource_path(path);
			m_card_layout.front_face= (xtexture*)svr->async_request(xtexture::get_class()->id,path.c_str(),\
			this,(xressvr::resource_callback_t)_on_load_layout);
			if(m_card_layout.front_face->is_complete())
				add_state(m_completion,CARD_FRONTFACE_TEXTURE);
			else
				remove_state(m_completion,CARD_FRONTFACE_TEXTURE);
		}
		else if(0==strcmp(iter->name,"scale"))
			m_card_layout.scale = iter->float_value;
	}
	if(!areas || areas->type!=vjson::JSON_ARRAY)
		return false;
	xrectf_t r;
	std::string name;
	const char *font;
	for(const vjson::json_value *iter = areas->first_child; iter; iter=iter->next_sibling)
	{
		if(iter->type!=vjson::JSON_OBJECT)
			continue;
		font=0;
		for(const vjson::json_value *attr = iter->first_child; attr; attr=attr->next_sibling)
		{
			if(0==strcmp(attr->name,"name"))
				name=attr->string_value;
			else if(0==strcmp(attr->name,"x"))
				r.xmin=float(attr->int_value);
			else if(0==strcmp(attr->name,"y"))
				r.ymin=float(attr->int_value);
			else if(0==strcmp(attr->name,"width"))
				r.xmax=float(attr->int_value);
			else if(0==strcmp(attr->name,"height"))
				r.ymax=float(attr->int_value);
			else if(0==strcmp(attr->name,"font"))
				font=attr->string_value;
		}
		r.xmax+=r.xmin;
		r.ymax+=r.ymin;
		if(name=="avatar") {
			m_card_layout.avatar=r;
		}
		if(name=="name") {
			m_card_layout.name=r;
		}
		else if(name=="cost") {
			m_card_layout.cost=r;
		}
		else if(name=="info") {
			m_card_layout.info=r;
		}
		else if(name=="race") {
			m_card_layout.type=r;
		}
		else if(name=="rarity") {
			m_card_layout.rarity=r;
		}
		else if(name=="hp") {
			m_card_layout.hp=r;
		}
		else if(name=="strength") {
			m_card_layout.strength=r;
		}
	}
	if(!_init_mesh(m_card_layout.width*m_card_layout.scale,m_card_layout.height*m_card_layout.scale))
		return false;
	if(!_init_atlas())
		return false;
	return true;
}

void xcard_renderer::draw(xrenderer *rc, const xmat4f_t *camera_transform)
{
	if(!is_complete())
		return;
	
	if(!rc->use_shader(SHADER_CARD))
		return;
	const wyc::xshader *sh=rc->get_shader();
	assert(sh);
	GLint uf=sh->get_uniform("camera_mvp");
	assert(uf!=-1);
	glUniformMatrix4fv(uf,1,GL_TRUE,camera_transform->data());
	uf=sh->get_uniform("texture");
	assert(-1!=uf);
	glUniform1i(uf,0);
//	uf = sh->get_uniform("transform");
//	assert(-1!=uf);

	glEnableVertexAttribArray(USAGE_POSITION);
	glEnableVertexAttribArray(USAGE_NORMAL);
	glEnableVertexAttribArray(USAGE_COLOR);
	glEnableVertexAttribArray(USAGE_TEXTURE0);
	
	// vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER,m_vbo_card);
	glVertexAttribPointer(USAGE_POSITION,3,GL_FLOAT,GL_FALSE,0,0);
	glVertexAttribPointer(USAGE_NORMAL,3,GL_FLOAT,GL_FALSE,0,(GLvoid*)(sizeof(GLfloat)*12));
	glVertexAttribPointer(USAGE_COLOR,3,GL_FLOAT,GL_FALSE,0,(GLvoid*)(sizeof(GLfloat)*24));
	// texture coordinate buffer
	glBindBuffer(GL_ARRAY_BUFFER,m_vbo_card_texcoord);
	// index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m_ibo_card);

	xdict::iterator iter, end=m_cards.end();
	// pass1: draw back face
	glFrontFace(GL_CW);
	glBindTexture(GL_TEXTURE_2D,m_card_layout.back_face->handle());
	glVertexAttribPointer(USAGE_TEXTURE0,2,GL_FLOAT,GL_FALSE,0,(GLvoid*)(sizeof(GLfloat*)*m_backface_texcoord));
	for(iter=m_cards.begin(); iter!=end; ++iter) {
		glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_SHORT,0);
	}

	// pass2: draw front face
	glFrontFace(GL_CCW);
	for(iter=m_cards.begin(); iter!=end; ++iter) {
		slot_t *slot=(slot_t*)(iter->second);
		glVertexAttribPointer(USAGE_TEXTURE0,2,GL_FLOAT,GL_FALSE,0,(GLvoid*)(sizeof(GLfloat)*slot->offset));
		glBindTexture(GL_TEXTURE_2D,slot->parent->texture_id);
		glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_SHORT,0);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(USAGE_POSITION);
	glDisableVertexAttribArray(USAGE_NORMAL);
	glDisableVertexAttribArray(USAGE_COLOR);
	glDisableVertexAttribArray(USAGE_TEXTURE0);
}

void xcard_renderer::add_card(xcard *card)
{
	assert(card);
	if(m_cards.contain(card->id()))
		return;
	slot_t *s = _new_slot();
	if(!s) 
		return;
	if(size_t(-1)==s->offset) {
		GLfloat texcoord[8] = {
			s->u0, s->v0,
			s->u1, s->v0,
			s->u1, s->v1,
			s->u0, s->v1,
		};
		s->offset=_append_texcoord(texcoord,8);
		if(size_t(-1)==s->offset)
		{
			_del_slot(s);
			return;
		}
	}
	s->card=card;
	card->set_render_context(s);
	card->redraw();
	m_cards.add(card->id(),s);
	_prepare_card_data(s);
}

void xcard_renderer::del_card(xcard *card)
{
	assert(card);
	slot_t *s=(slot_t*)m_cards.get(card->id());
	if(!s) return;
	card->set_render_context(0);
	_del_slot(s);
}

//-----------------------------------------------------------------------

bool xcard_renderer::_init_mesh(float w, float h)
{
	if(!m_vbo_card)
	{
		GLuint vbo[3];
		glGenBuffers(3,vbo);
		if(0==vbo[0] || 0==vbo[1] || 0==vbo[2]) {
			glDeleteBuffers(3,vbo);
			return false;
		}
		m_vbo_card=vbo[0];
		m_ibo_card=vbo[1];
		m_vbo_card_texcoord=vbo[2];
	}
	float halfw = w*0.5f, halfh=h*0.5f;
	GLfloat vertex[12] = {
		// front face
		-halfw, -halfh, 0,
		 halfw, -halfh, 0,
		 halfw,  halfh, 0,
		-halfw,  halfh, 0,
	};
	GLfloat normal[12] = {
		// front face
		0, 0, 1,
		0, 0, 1,
		0, 0, 1,
		0, 0, 1,
	};
	GLfloat color[12] = {
		// front face
		1, 1, 1,
		1, 1, 1, 
		1, 1, 1,
		1, 1, 1,
	};
	GLushort vindex[6] = {
		// front face
		0, 1, 3, 1, 2, 3,
	};
	// vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER,m_vbo_card);
	glBufferData(GL_ARRAY_BUFFER,sizeof(vertex)+sizeof(normal)+sizeof(color),0,GL_STATIC_DRAW);
	uint8_t *pbuff = (uint8_t*)glMapBuffer(GL_ARRAY_BUFFER,GL_WRITE_ONLY);
	assert(pbuff);
		memcpy(pbuff,vertex,sizeof(vertex));
		pbuff+=sizeof(vertex);
		memcpy(pbuff,normal,sizeof(normal));
		pbuff+=sizeof(normal);
		memcpy(pbuff,color,sizeof(color));
		pbuff+=sizeof(color);
	glUnmapBuffer(GL_ARRAY_BUFFER);
	// index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m_ibo_card);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(vindex),vindex,GL_STATIC_DRAW);
	// texture coordinate buffer
	m_texbuff_cap=256;
	m_texbuff_size=0;
	glBindBuffer(GL_ARRAY_BUFFER,m_vbo_card_texcoord);
	glBufferData(GL_ARRAY_BUFFER,sizeof(GLfloat)*m_texbuff_cap,0,GL_STREAM_DRAW);

	m_completion|=CARD_MESH_DATA;
	return true;
}

bool xcard_renderer::_init_atlas()
{
	if(!m_atlas) {
		m_atlas = _new_atlas();
		if(!m_atlas)
			return false;
	}
	if(0==m_fbo_layout) {
		GLuint fbo;
		glGenFramebuffers(1,&fbo);
		if(0==fbo)
			return false;
		m_fbo_layout = fbo;
		m_completion|=LAYOUT_FBO;
	}
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER,m_fbo_layout);
	glClearColor(0,1,0,1);
	for(atlas_t *iter=m_atlas; iter; iter=iter->next)
	{
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,iter->texture_id,0);
		GLenum fbo_status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
		if(fbo_status != GL_FRAMEBUFFER_COMPLETE) {
			wyc_error("xcard_renderer::_init_atlas: frame buffer is not complete");
			continue;
		}
		glClear(GL_COLOR_BUFFER_BIT);
	}
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER,0);
	return true;
}

#define LAYOUT_VERTEX_OFFSET_BACKGROUND 0
#define LAYOUT_VERTEX_OFFSET_AVATAR (sizeof(GLfloat)*16)
#define LAYOUT_VERTEX_OFFSET_TEXT (sizeof(GLfloat)*32)

void xcard_renderer::_resize_layout_buffer(slot_t *slot, size_t &text_size)
{	
	assert(slot);
	xcard *card=slot->card;
	assert(card);
#ifdef _DEBUG
	GLint current_vbo;
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING,&current_vbo);
	assert(current_vbo==m_vbo_layout);
#endif // _DEBUG
	const xcard_data *cd = card->get_card_data();
	text_size = 0;
	if(cd) {
		// 所有文本的长度,有勾边的字体需要x2
		size_t char_count = cd->name.length()*2 + cd->type.length() + cd->desc.length();
		// 费用: 最多2位数
		// example: 费 10
		char_count += 4;
		// HP: 最多2位数, 带勾边
		// example: 10/10
		char_count += 5*2;
		// 力量: 最多2位数, 带勾边
		// example: 攻 10
		char_count += 4*2;
		// 稀有度: 首字母表示, 带勾边
		char_count += 2;
		text_size = xtext_renderer::calc_buffer_size(char_count);
	}
	size_t size = text_size + LAYOUT_VERTEX_OFFSET_TEXT;
	assert(size<0xFFFF);
	if(m_vbo_layout_size>=size)
		return;
	m_vbo_layout_size = (size+255)&0xFF00;
//	glBindBuffer(GL_ARRAY_BUFFER,m_vbo_layout);
	glBufferData(GL_ARRAY_BUFFER,m_vbo_layout_size,0,GL_DYNAMIC_DRAW);
	// background and card image
	GLfloat bk_xmax, bk_ymax, bk_s1, bk_t1;
	bk_xmax=(GLfloat)m_card_layout.width;
	bk_ymax=(GLfloat)m_card_layout.height;
	bk_s1=float(m_card_layout.front_face->image_width())/m_card_layout.front_face->width();
	bk_t1=float(m_card_layout.front_face->image_height())/m_card_layout.front_face->height();
	GLfloat avt_xmax = m_card_layout.avatar.xmin+m_card_layout.avatar.width(), 
		avt_ymax = m_card_layout.avatar.ymin+m_card_layout.avatar.height();
	GLfloat vert_bk[] = {
		// background position
		0, 0,
		0, bk_ymax,
		bk_xmax, 0,
		bk_xmax, bk_ymax,
		// background uv
		0, bk_t1,
		0, 0, 
		bk_s1, bk_t1,
		bk_s1, 0, 
		// avatar position
		m_card_layout.avatar.xmin, m_card_layout.avatar.ymin,
		m_card_layout.avatar.xmin, avt_ymax,
		avt_xmax, m_card_layout.avatar.ymin,
		avt_xmax, m_card_layout.avatar.ymax,
		// avatar uv
		0, 1,
		0, 0,
		1, 1,
		1, 0,
	};
	
	assert(LAYOUT_VERTEX_OFFSET_TEXT==sizeof(vert_bk));
	glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(vert_bk),vert_bk);
	assert(glGetError()==GL_NO_ERROR);
	return;
}

void xcard_renderer::_set_avatar_vertex(void *buff, slot_t *slot)
{
	GLfloat *tex_coords = (GLfloat*)buff;
	GLfloat avt_s0=0, avt_t0=0, avt_s1=1, avt_t1=1;
	if(slot->avatar && slot->avatar->is_complete())
	{
		const ximageset *imgset = slot->avatar->get_imageset();
		if(imgset)
		{
			const ximageset::subimage_t *avt_img = imgset->get_image(slot->avatar_id);
			if(avt_img) {
				avt_s0 = avt_img->m_s0;
				avt_t0 = avt_img->m_t0;
				avt_s1 = avt_img->m_s1;
				avt_t1 = avt_img->m_t1;
			}
		}
	}
	tex_coords[0]=avt_s0, tex_coords[1]=avt_t1,
	tex_coords[2]=avt_s0, tex_coords[3]=avt_t0,
	tex_coords[4]=avt_s1, tex_coords[5]=avt_t1,
	tex_coords[6]=avt_s1, tex_coords[7]=avt_t0;
}

void xcard_renderer::_render_card_text(xtext_renderer *tr, xcard *card)
{
	assert(tr);
	assert(card);
	const xcard_data *cd = card->get_card_data();
	assert(cd);
	float penx, peny;
	wchar_t ws[6];
	int len;
	float text_w;
	texture_glyph_t *glyph;

	penx = m_card_layout.name.left, peny = m_card_layout.name.top + m_card_layout.def_font->ascender();
	tr->draw_text(penx, peny, cd->name.c_str(), m_card_layout.def_font, 0xFF000000, 1, &m_card_layout.name, GLYPH_OUTLINE);
	penx = m_card_layout.name.left, peny = m_card_layout.name.top + m_card_layout.def_font->ascender();
	tr->draw_text(penx, peny, cd->name.c_str(), m_card_layout.def_font, 0xFFFFFFFF, 1, &m_card_layout.name);

	penx = m_card_layout.type.left, peny = m_card_layout.type.top + m_card_layout.def_font->ascender();
	tr->draw_text(penx, peny, cd->type.c_str(), m_card_layout.def_font, 0xFF000000, 1, &m_card_layout.type);

	ws[0]=get_rarity_desc((CARD_RARITY)cd->rarity)[0];
	ws[1]=0;
	penx = m_card_layout.rarity.left, peny = m_card_layout.rarity.top + m_card_layout.def_font->ascender();
	tr->draw_text(penx, peny, ws, m_card_layout.def_font, 0xFF000000, 1, &m_card_layout.rarity, GLYPH_OUTLINE);
	penx = m_card_layout.rarity.left, peny = m_card_layout.rarity.top + m_card_layout.def_font->ascender();
	tr->draw_text(penx, peny, ws, m_card_layout.def_font, 0xFF00FFFF, 1, &m_card_layout.rarity);

	penx = m_card_layout.info.left, peny = m_card_layout.info.top + m_card_layout.def_font->ascender();
	tr->draw_text(penx, peny, cd->desc.c_str(), m_card_layout.def_font, 0xFF000000, 1, &m_card_layout.info, GLYPH_ITALIC);

	len=swprintf_s(ws,6,L"费 %d",cd->cost);
	ws[len]=0;
	text_w=0;
	for(wchar_t *ch = ws; *ch; ++ch)
	{
		glyph = m_card_layout.def_font->get_glyph(*ch);
		if(glyph) 
			text_w += glyph->advance_x;
	}
	penx = m_card_layout.cost.left+m_card_layout.cost.width()-text_w, peny = m_card_layout.cost.top + m_card_layout.def_font->ascender();
	tr->draw_text(penx, peny, ws, m_card_layout.def_font, 0xFF000000, 1, &m_card_layout.cost);

	len=swprintf_s(ws,6,L"攻 %d",cd->strength);
	ws[len]=0;
	penx = m_card_layout.strength.left, peny = m_card_layout.strength.top + m_card_layout.def_font->ascender();
	tr->draw_text(penx, peny, ws, m_card_layout.def_font, 0xFF000000, 1, &m_card_layout.strength, GLYPH_OUTLINE);
	penx = m_card_layout.strength.left, peny = m_card_layout.strength.top + m_card_layout.def_font->ascender();
	tr->draw_text(penx, peny, ws, m_card_layout.def_font, 0xFF0000FF, 1, &m_card_layout.strength);

	len=swprintf_s(ws,6,L"%d/%d",cd->max_hp,cd->max_hp);
	ws[len]=0;
	text_w=0;
	for(wchar_t *ch = ws; *ch; ++ch)
	{
		glyph = m_card_layout.def_font->get_glyph(*ch);
		if(glyph) 
			text_w += glyph->advance_x;
	}
	float offset_x = (m_card_layout.hp.width()-text_w)*0.5f;
	penx = m_card_layout.hp.left+offset_x, peny = m_card_layout.hp.top + m_card_layout.def_font->ascender();
	tr->draw_text(penx, peny, ws, m_card_layout.def_font, 0xFF000000, 1, &m_card_layout.hp, GLYPH_OUTLINE);
	penx = m_card_layout.hp.left+offset_x, peny = m_card_layout.hp.top + m_card_layout.def_font->ascender();
	tr->draw_text(penx, peny, ws, m_card_layout.def_font, 0xFFFF8000, 1, &m_card_layout.hp);
}

void xcard_renderer::flush_atlas(xrenderer *rc, bool force)
{
	assert(is_complete());

	if(!m_flush_atlas && !force) return;
	m_flush_atlas = false;

	// upload all glyphs texture
	m_card_layout.def_font->upload_texture();

	// save current status
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT,viewport);

	// generate mesh
	if(!m_vbo_layout) {
		GLuint vbo;
		glGenBuffers(1,&vbo);
		if(0==vbo)
		{
			assert(0 && "xcard_renderer::flush_atlas: can't generate vertex buffer");
			return;
		}
		m_vbo_layout=vbo;
	}
	glBindBuffer(GL_ARRAY_BUFFER,m_vbo_layout);
	glEnableVertexAttribArray(USAGE_POSITION);
	glEnableVertexAttribArray(USAGE_TEXTURE0);

	slot_t *s;
	size_t text_buffer_size;
	void *vbuffer;
	xtext_renderer tr;
	xmat4f_t proj;
	set_ui_projection(proj,float(m_card_layout.width),float(m_card_layout.height),100);
	const xshader *sh;
	GLint uf;

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER,m_fbo_layout);
	for(atlas_t *atlas = m_atlas; atlas; atlas=atlas->next)
	{
		if(!atlas->mod_count)
			continue;
		atlas->mod_count=0;
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,atlas->texture_id,0);
		assert(GL_FRAMEBUFFER_COMPLETE==glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));
		for(size_t i=0, cnt=atlas->slots.size(); i!=cnt; ++i)
		{
			s=&atlas->slots[i];
			if(!s->card || !s->is_dirty())
				continue;
			s->reset();
			_resize_layout_buffer(s,text_buffer_size);
			vbuffer=glMapBufferRange(GL_ARRAY_BUFFER,sizeof(GLfloat)*24,sizeof(GLfloat)*8+text_buffer_size,GL_MAP_WRITE_BIT|GL_MAP_INVALIDATE_RANGE_BIT);
			assert(vbuffer);
			// update avatar UV
			_set_avatar_vertex(vbuffer,s);
			// update text vertex
			if(text_buffer_size) 
			{
				vbuffer = ((char*)vbuffer)+sizeof(GLfloat)*8;
				tr.attach_buffer(vbuffer,text_buffer_size);
				_render_card_text(&tr,s->card);
				tr.detach_buffer();
			}
			glUnmapBuffer(GL_ARRAY_BUFFER);
			assert(glGetError()==GL_NO_ERROR);

			glViewport(s->x,s->y,m_card_layout.width,m_card_layout.height);
			//-----------------------------------------------------
			// draw background
			glDisable(GL_BLEND);
			if(rc->use_shader(SHADER_CARD_LAYOUT))
			{
				sh = rc->get_shader();
				uf = sh->get_uniform("proj");
				assert(-1!=uf);
				glUniformMatrix4fv(uf,1,GL_TRUE,proj.data());
				uf = sh->get_uniform("texture");
				assert(-1!=uf);
				glUniform1i(uf,0);
				glBindTexture(GL_TEXTURE_2D,m_card_layout.front_face->handle());

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m_ibo_card);
				glVertexAttribPointer(USAGE_POSITION,2,GL_FLOAT,GL_FALSE,0,(GLvoid*)(LAYOUT_VERTEX_OFFSET_BACKGROUND));
				glVertexAttribPointer(USAGE_TEXTURE0,2,GL_FLOAT,GL_FALSE,0,(GLvoid*)(LAYOUT_VERTEX_OFFSET_BACKGROUND+sizeof(GLfloat)*8));
				glDrawArrays(GL_TRIANGLE_STRIP,0,4);

				//-----------------------------------------------------
				// draw avatar
				if(s->avatar && s->avatar->is_complete())
				{
					glBindTexture(GL_TEXTURE_2D,s->avatar->handle());
					glVertexAttribPointer(USAGE_POSITION,2,GL_FLOAT,GL_FALSE,0,(GLvoid*)(LAYOUT_VERTEX_OFFSET_AVATAR));
					glVertexAttribPointer(USAGE_TEXTURE0,2,GL_FLOAT,GL_FALSE,0,(GLvoid*)(LAYOUT_VERTEX_OFFSET_AVATAR+sizeof(GLfloat)*8));
					glDrawArrays(GL_TRIANGLE_STRIP,0,4);
				}
			}


			//-----------------------------------------------------
			// draw text
			if(text_buffer_size && rc->use_shader(SHADER_TEXT))
			{
				sh = rc->get_shader();
				uf = sh->get_uniform("gui_proj");
				assert(-1!=uf);
				glUniformMatrix4fv(uf,1,GL_TRUE,proj.data());
				uf = sh->get_uniform("gui_texture");
				assert(-1!=uf);
				glUniform1i(uf,0);

				glEnable(GL_BLEND);
				glEnableVertexAttribArray(USAGE_COLOR);
				glVertexAttribPointer(USAGE_POSITION,2,GL_FLOAT,GL_FALSE,sizeof(xvertex2d),(GLvoid*)LAYOUT_VERTEX_OFFSET_TEXT);
				glVertexAttribPointer(USAGE_TEXTURE0,2,GL_FLOAT,GL_FALSE,sizeof(xvertex2d),(GLvoid*)(LAYOUT_VERTEX_OFFSET_TEXT+sizeof(GLfloat)*2));
				glVertexAttribPointer(USAGE_COLOR,4,GL_UNSIGNED_BYTE,GL_TRUE,sizeof(xvertex2d),(GLvoid*)(LAYOUT_VERTEX_OFFSET_TEXT+sizeof(GLfloat)*4));
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
				tr.render();
				glDisableVertexAttribArray(USAGE_COLOR);
				glDisable(GL_BLEND);
			}

			assert(GL_NO_ERROR==glGetError());
		}
	}
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER,0);

	glDisableVertexAttribArray(USAGE_POSITION);
	glDisableVertexAttribArray(USAGE_TEXTURE0);
	glBindBuffer(GL_ARRAY_BUFFER,0);

	glViewport(viewport[0],viewport[1],viewport[2],viewport[3]);
}

xcard_renderer::atlas_t* xcard_renderer::_new_atlas()
{
	GLuint tex;
	glGenTextures(1,&tex);
	if(0==tex) {
		return 0;
	}
	unsigned atlas_w = 1024, atlas_h = 1024;
	glBindTexture(GL_TEXTURE_2D,tex);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,atlas_w,atlas_h,0,GL_RGBA,GL_UNSIGNED_BYTE,0);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

	atlas_t *atlas = new atlas_t;
	atlas->parent = this;
	atlas->texture_id = tex;
	atlas->width=atlas_w;
	atlas->height=atlas_h;
	atlas->next=0;
	atlas->ref_count=0;
	atlas->mod_count=0;

	unsigned x = atlas_w/(m_card_layout.width+1), y = atlas_h/(m_card_layout.height+1);
	unsigned count = x * y;
	atlas->slots.resize(count);
	float u0, v0, u1, v1;
	x=0, y=0;
	u0=0.5f, v0=0.5f;
	u1=(m_card_layout.width-0.5f)/atlas_w;
	v1=(m_card_layout.height-0.5f)/atlas_h;
	slot_t *iter;
	for(unsigned i=0; i<count; ++i) 
	{
		iter=&atlas->slots[i];
		iter->parent=atlas;
		iter->card=0;
		iter->x=x;
		iter->y=y;
		iter->u0=u0;
		iter->v0=v0;
		iter->u1=u1;
		iter->v1=v1;
		iter->offset=size_t(-1);
		m_slot_cache.push_back(iter);

		x+=m_card_layout.width+1;
		if(x+m_card_layout.width>atlas_w)
		{
			x=0;
			y+=m_card_layout.height+1;
			v0=float(y+0.5f)/atlas_h;
			v1=float(y+m_card_layout.height-0.5f)/atlas_h;
		}
		u0=float(x+0.5f)/atlas_w;
		u1=float(x+m_card_layout.width-0.5f)/atlas_w;
	}
	return atlas;
}

void xcard_renderer::_clear_atlas()
{
	atlas_t *iter = m_atlas;
	while(m_atlas)
	{
		iter=m_atlas;
		m_atlas=m_atlas->next;
		assert(iter->texture_id);
		glDeleteTextures(1,&iter->texture_id);
		delete iter;
	}
	m_slot_cache.clear();
}

xcard_renderer::slot_t* xcard_renderer::_new_slot()
{
	if(m_slot_cache.empty())
	{
		atlas_t *atlas = _new_atlas();
		if(!atlas) 
			return 0;
		atlas->next = m_atlas;
		m_atlas = atlas;
		assert(!m_slot_cache.empty());
	}
	slot_t *slot = m_slot_cache.back();
	slot->parent->ref_count+=1;
	m_slot_cache.pop_back();
	return slot;
}

void xcard_renderer::_del_slot(slot_t *s)
{
	m_slot_cache.push_back(s);
	s->parent->ref_count-=1;
	s->card=0;
	s->avatar=0;
}

void xcard_renderer::slot_t::redraw(int flag)
{
	parent->mod_count+=1;
	parent->parent->_set_atlas_dirty();
	parent->parent->_prepare_card_data(this);
	xcard_render_context::redraw(flag);
}

void xcard_renderer::_prepare_card_data(slot_t *slot)
{
	static bool ls_load_common_glyphs=false;
	xfont *font = m_card_layout.def_font;
	if(!ls_load_common_glyphs) {
		ls_load_common_glyphs=true;
		font->async_load_glyphs(L"攻费",0,GLYPH_OUTLINE);
	}
	xcard *card = slot->card;
	assert(card);
	const xcard_data *cd = card->get_card_data();
	if(!cd) 
		return;
	// prepare card avatar
	if(!cd->avatar.empty())
	{
		xressvr *svr = get_resource_server();
		std::string avatar;
		wstr2str(avatar,cd->avatar);
		std::string::size_type pos = avatar.find('@');
		std::string img_name, img_file;
		if(std::string::npos!=pos)
		{
			img_name = avatar.substr(0,pos);
			img_file = avatar.substr(pos+1);
		}
		else img_file = avatar;
		get_resource_path(img_file);
		slot->avatar = (xtexture*)svr->async_request(xtexture::get_class()->id,img_file.c_str());
		if(img_name.empty())
			slot->avatar_id = 0;
		else
			slot->avatar_id = strhash(img_name.c_str());
	}
	// prepare card text
	font->async_load_glyphs(cd->name.c_str(),cd->name.length(),GLYPH_OUTLINE);
	font->async_load_glyphs(cd->type.c_str(),cd->type.length());
	font->async_load_glyphs(cd->desc.c_str(),cd->desc.length(),GLYPH_ITALIC);

}

void xcard_renderer::_on_backface_changed()
{
	xtexture *tex = m_card_layout.back_face;
	if(!tex->is_complete())
		return;
	float u=float(tex->image_width())/tex->width(), 
		v=float(tex->image_height())/tex->height();
	GLfloat coords[8] = {
		u, 0,
		0, 0,
		0, v,
		u, v,
	};
	if(size_t(-1)==m_backface_texcoord)
	{
		m_backface_texcoord = _append_texcoord(coords,8);
	}
	else {
		_append_texcoord(coords,8,m_backface_texcoord);
	}
}

size_t xcard_renderer::_append_texcoord(GLfloat *coords, size_t size, size_t offset)
{
	if(!m_vbo_card_texcoord) 
	{
		return -1;
	}
	if(offset==-1) {
		if(size+m_texbuff_size>m_texbuff_cap)
		{
			wyc_warn("xcard_renderer::_append_texcoord: space not enough");
			// TODO: auto increase space
			return -1;
		}
		offset=m_texbuff_size;
		m_texbuff_size+=size;
	}
	else if(offset+size>m_texbuff_size)
	{
		wyc_error("xcard_renderer::_append_texcoord: invalid offset");
		return -1;
	}
	glBindBuffer(GL_ARRAY_BUFFER,m_vbo_card_texcoord);
	GLfloat *pbuff = (GLfloat*)glMapBufferRange(GL_ARRAY_BUFFER,sizeof(GLfloat)*offset,sizeof(GLfloat)*size,\
		GL_MAP_WRITE_BIT|GL_MAP_INVALIDATE_RANGE_BIT);
		assert(pbuff);
		memcpy(pbuff,coords,sizeof(GLfloat)*size);
	glUnmapBuffer(GL_ARRAY_BUFFER);
	return offset;
}

//-----------------------------------------------------------------------
// debug interface
//-----------------------------------------------------------------------

void xcard_renderer::debug_atlas(unsigned screen_w, unsigned screen_h)
{
	float x0, y0, view_w, view_h;
	view_w=(float)std::min(screen_w,screen_h), view_h=view_w;
	x0 = 0.5f*(screen_w-view_w), y0 = 0.5f*(screen_h-view_h);
	if(!m_debug_info.atlas_mesh)
	{
		GLuint vbo;
		glGenBuffers(1,&vbo);
		if(0==vbo)
			return;
		m_debug_info.atlas_mesh=vbo;
	}
	xrectf_t size;
	if(m_atlas)
	{
		m_debug_info.atlas_texture=m_atlas->texture_id;
		float scale = std::min(view_w/float(m_atlas->width),view_h/float(m_atlas->height));
		float w = m_atlas->width*scale, h = m_atlas->height*scale;
		x0+=0.5f*(view_w-w);
		y0+=0.5f*(view_h-h);
		size.set(x0,y0,x0+w,y0+h);
	}
	else {
		m_debug_info.atlas_texture=0;
		size.set(x0,y0,x0+view_w,y0+view_h);
	}
	if(m_debug_info.atlas_size!=size)
	{
		m_debug_info.atlas_size=size;
		GLfloat x1 = x0+size.width(), y1 = y0+size.height();
		GLfloat vertex[]={
			// position
			x0, y1,
			x1, y1,
			x0, y0,
			x1, y0,
			// uv
			0,  0,
			1,  0,
			0,  1, 
			1,  1,
		};
		glBindBuffer(GL_ARRAY_BUFFER,m_debug_info.atlas_mesh);
			glBufferData(GL_ARRAY_BUFFER,sizeof(vertex),vertex,GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER,0);
	}
}

void xcard_renderer::debug_draw_atlas()
{
	if(0==m_debug_info.atlas_mesh || 0==m_debug_info.atlas_texture)
		return;

	glEnableVertexAttribArray(USAGE_POSITION);
	glEnableVertexAttribArray(USAGE_TEXTURE0);

	glBindBuffer(GL_ARRAY_BUFFER,m_debug_info.atlas_mesh);
		glVertexAttribPointer(USAGE_POSITION,2,GL_FLOAT,GL_FALSE,0,0);
		glVertexAttribPointer(USAGE_TEXTURE0,2,GL_FLOAT,GL_FALSE,0,(GLvoid*)(sizeof(GLfloat)*8));
		glBindTexture(GL_TEXTURE_2D,m_debug_info.atlas_texture);
		glDrawArrays(GL_TRIANGLE_STRIP,0,4);
	glBindBuffer(GL_ARRAY_BUFFER,0);

	glDisableVertexAttribArray(USAGE_POSITION);
	glDisableVertexAttribArray(USAGE_TEXTURE0);
	
}

void xcard_renderer::debug_next_atlas()
{
	atlas_t *atlas;
	for( atlas = m_atlas; atlas && atlas->texture_id!=m_debug_info.atlas_texture; atlas=atlas->next);
	if(atlas && atlas->next)
		m_debug_info.atlas_texture=atlas->next->texture_id;
	else if(m_atlas)
		m_debug_info.atlas_texture=m_atlas->texture_id;
	else
		m_debug_info.atlas_texture=0;
	wyc_print("show atlas: [%d]",m_debug_info.atlas_texture);
}

void xcard_renderer::debug_add_card()
{
	slot_t *s = _new_slot();
	assert(s);
	if(size_t(-1)==s->offset) {
		GLfloat texcoord[8] = {
			s->u0, s->v0,
			s->u1, s->v0,
			s->u1, s->v1,
			s->u0, s->v1,
		};
		s->offset=_append_texcoord(texcoord,8);
		if(size_t(-1)==s->offset)
		{
			_del_slot(s);
			return;
		}
	}
	xcard *card = wycnew xcard();
	s->card = card;
	card->set_render_context(s);
	card->redraw();
	m_debug_info.dummy_slots.push_back(s);
}

void xcard_renderer::debug_del_card()
{
	if(m_debug_info.dummy_slots.empty())
		return;
	unsigned idx = rand()%m_debug_info.dummy_slots.size();
	slot_t *s = (slot_t*)m_debug_info.dummy_slots[idx];
	xcard *card = s->card;
	card->set_render_context(0);
	s->parent->mod_count+=1;
	s->parent->parent->_set_atlas_dirty();
	_del_slot(s);
	m_debug_info.dummy_slots[idx]=m_debug_info.dummy_slots.back();
	m_debug_info.dummy_slots.pop_back();
}

void xcard_renderer::debug_flush(xrenderer *rc)
{
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT,viewport);
	GLfloat clear_color[4];
	glGetFloatv(GL_COLOR_CLEAR_VALUE,clear_color);

	// generate mesh
	GLuint vbo;
	glGenBuffers(1,&vbo);
	assert(0!=vbo);

	GLfloat x0, y0, x1, y1, u, v;
	x0=y0=0;
	x1=x0+m_card_layout.width;
	y1=y0+m_card_layout.height;
	u=float(m_card_layout.front_face->image_width())/m_card_layout.front_face->width();
	v=float(m_card_layout.front_face->image_height())/m_card_layout.front_face->height();
	GLfloat vertex[] = {
		// position
		x0, y0,
		x0, y1,
		x1, y0,
		x1, y1,
		// uv
		0, v,
		0, 0, 
		u, v,
		u, 0, 
	};
	glBindBuffer(GL_ARRAY_BUFFER,vbo);
	glBufferData(GL_ARRAY_BUFFER,sizeof(vertex),vertex,GL_STATIC_DRAW);
	glVertexAttribPointer(USAGE_POSITION,2,GL_FLOAT,GL_FALSE,0,0);
	glVertexAttribPointer(USAGE_TEXTURE0,2,GL_FLOAT,GL_FALSE,0,(GLvoid*)(sizeof(GLfloat)*8));

	if(!rc->use_shader(SHADER_CARD_LAYOUT)) {
		wyc_error("xcard_renderer::debug_flush: can't apply shader");
		assert(0);
	}
	xmat4f_t proj;
	set_ui_projection(proj,float(m_card_layout.width),float(m_card_layout.height),100);
	const xshader *sh = rc->get_shader();
	GLint uf = sh->get_uniform("proj");
	assert(-1!=uf);
	glUniformMatrix4fv(uf,1,GL_TRUE,proj.data());
	uf = sh->get_uniform("texture");
	assert(-1!=uf);
	glUniform1i(uf,0);
	glBindTexture(GL_TEXTURE_2D,m_card_layout.front_face->handle());

	GLenum fbo_status;
	slot_t *slot;
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER,m_fbo_layout);
	glEnableVertexAttribArray(USAGE_POSITION);
	glEnableVertexAttribArray(USAGE_TEXTURE0);
	glClearColor(0,1,0,1);
	for (atlas_t *atlas = m_atlas; atlas; atlas=atlas->next)
	{
		if(atlas->mod_count==0)
			continue;
		atlas->mod_count=0;
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,atlas->texture_id,0);
		fbo_status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
		if(fbo_status != GL_FRAMEBUFFER_COMPLETE) {
			wyc_error("xcard_renderer::debug_flush: frame buffer is not complete");
			assert(0);
		}
		glClear(GL_COLOR_BUFFER_BIT);
		for(unsigned i=0; i<atlas->slots.size(); ++i)
		{
			slot = &atlas->slots[i];
			if(!slot->card)
				continue;
			glViewport(slot->x,slot->y,m_card_layout.width,m_card_layout.height);
			glDrawArrays(GL_TRIANGLE_STRIP,0,4);
		}
	}
	glDisableVertexAttribArray(USAGE_POSITION);
	glDisableVertexAttribArray(USAGE_TEXTURE0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER,0);

	glBindBuffer(GL_ARRAY_BUFFER,0);
	glDeleteBuffers(1,&vbo);
	glViewport(viewport[0],viewport[1],viewport[2],viewport[3]);
	glClearColor(clear_color[0],clear_color[1],clear_color[2],clear_color[3]);
}

}; // namespace wyc
