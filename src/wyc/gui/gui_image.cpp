#include "fscorepch.h"
#include "wyc/gui/gui_image.h"
#include "wyc/game/glb_game.h"

namespace wyc
{

REG_RTTI(xgui_image,xguiobj)

static const uint8_t s_blit_vertcount[MAX_BLIT_TYPE]={
	4,	// NO_BLIT
	4,	// BLIT_HV
	8,	// BLIT_H3
	8,	// BLIT_V3
	16,	// BLIT_33
};

static unsigned short s_index_no_blit[6]={0,1,2,2,1,3}; 

static unsigned short s_index_blit_33[54]=
{ 
	0, 1, 4, 4, 1, 5,
	1, 2, 5, 5, 2, 6,
	2, 3, 6, 6, 3, 7,
	4, 5, 8, 8, 5, 9,
	5, 6, 9, 9, 6, 10,
	6, 7, 10, 10, 7, 11,
	8, 9, 12, 12, 9, 13,
	9, 10, 13, 13, 10, 14,
	10, 11, 14, 14, 11, 15
};

static unsigned short* s_blit_index[MAX_BLIT_TYPE]=
{
	s_index_no_blit, // NO_BLIT
	s_index_no_blit, // BLIT_HV
	s_index_blit_33, // BLIT_H3
	s_index_blit_33, // BLIT_V3
	s_index_blit_33, // BLIT_33
};

static unsigned s_blit_index_count[MAX_BLIT_TYPE]=
{
	6,	// NO_BLIT
	6,	// BLIT_HV
	18,	// BLIT_H3
	18,	// BLIT_V3
	54,	// BLIT_33
};

xgui_image::xgui_image()
{
	m_blit=NO_BLIT;
	m_color=0xFFFFFFFF;
}

void xgui_image::on_destroy()
{
	m_tex=0;
}

xgui_image::mesh_builder_t xgui_image::ms_mesh_builder[MAX_BLIT_TYPE]=
{
	&xgui_image::_mesh_no_blit,
	&xgui_image::_mesh_blit_hv,
	&xgui_image::_mesh_blit_h3,
	&xgui_image::_mesh_blit_v3,
	&xgui_image::_mesh_blit_33,
};

void xgui_image::build_mesh()
{
	if(!m_tex || !m_tex->is_complete())
	{
		// show something special
		return;
	}
	if(!m_mesh || s_blit_vertcount[m_blit]>m_mesh->vertex_count())
		alloc_mesh(s_blit_vertcount[m_blit]);
	assert(m_mesh);
	mesh_builder_t builder=ms_mesh_builder[m_blit];
	(this->*builder)();
	m_mesh->commit_vertex();
}

void xgui_image::draw()
{
	if(!m_tex || !m_tex->is_complete())
		return;
	if(m_mesh && s_blit_index[m_blit]) 
	{
		m_mesh->draw(s_blit_index[m_blit],s_blit_index_count[m_blit],m_tex->handle(),get_technique());
	}
}

void xgui_image::set_image (const char *image_name, BLIT_TYPE blit)
{
	m_tex_name=image_name;
	m_img_name="";
	m_blit=blit;
	xressvr *svr=get_resource_server();
	m_tex=(xtexture*)svr->async_request(xtexture::get_class()->id,m_tex_name.c_str(),this,xgui_image::on_image_ok);
	if(!m_tex || m_tex->is_complete()) {
		rebuild();
		redraw();
	}
}

void xgui_image::set_image (const char *imageset, const char *image_name, BLIT_TYPE blit)
{
	m_tex_name=imageset;
	m_tex_name+=".json";
	m_img_name=image_name;
	m_blit=blit;
	xressvr *svr=get_resource_server();
	m_tex=(xtexture*)svr->async_request(xtexture::get_class()->id,m_tex_name.c_str(),this,xgui_image::on_image_ok);
	if(!m_tex || m_tex->is_complete()) {
		rebuild();
		redraw();
	}
}

void xgui_image::on_image_ok(xrefobj *obj, xresbase *res)
{
	assert(obj);
	assert(res);
	xgui_image *gui_obj=(xgui_image*)obj;
	if(res != gui_obj->m_tex) 
		return;
	if(!res->is_complete())
	{
		// loading failed
		gui_obj->m_tex=0;
	}
	gui_obj->rebuild();
	gui_obj->redraw();
}

void xgui_image::_mesh_no_blit()
{
	//	2 --- 3
	//	|     |
	//	0 --- 1

	assert(m_mesh);
	assert(m_mesh->vertex_count()>=4);
	assert(m_tex);

	float s0, t0, s1, t1;
	const ximageset::subimage_t *img=0;
	if(!m_img_name.empty())
	{
		const ximageset *imgset=m_tex->get_imageset();
		if(imgset) 
			img=imgset->get_image(m_img_name.c_str());
	}
	if(img) {
		s0=img->m_s0, t0=img->m_t0;
		s1=img->m_s1, t1=img->m_t1;
	}
	else {
		s0=t0=0.0f;
		s1=float(m_tex->image_width())/m_tex->width(),
		t1=float(m_tex->image_height())/m_tex->height();
	}
	xvertex2d *vert=&m_mesh->get_vertex(0);
	vert->m_pos.set(m_pos.x,m_pos.y+m_size.y);
	vert->m_color=m_color;
	vert->m_texcoord.set(s0,t0);

	++vert;
	vert->m_pos.set(m_pos.x+m_size.x,m_pos.y+m_size.y);
	vert->m_color=m_color;
	vert->m_texcoord.set(s1,t0);

	++vert;
	vert->m_pos.set(m_pos.x,m_pos.y);
	vert->m_color=m_color;
	vert->m_texcoord.set(s0,t1);

	++vert;
	vert->m_pos.set(m_pos.x+m_size.x,m_pos.y);
	vert->m_color=m_color;
	vert->m_texcoord.set(s1,t1);

}

void xgui_image::_mesh_blit_hv()
{
	//	2 --- 3
	//	|     |
	//	0 --- 1

	assert(m_mesh);
	assert(m_mesh->vertex_count()>=4);
	assert(m_tex);

	float s0, t0, s1, t1;
	const ximageset::subimage_t *img=0;
	if(!m_img_name.empty())
	{
		const ximageset *imgset=m_tex->get_imageset();
		if(imgset) 
			img=imgset->get_image(m_img_name.c_str());
	}
	if(img) {
		s0=img->m_s0, t0=img->m_t0;
		s1=m_size.x/img->m_width, t1=m_size.y/img->m_height;
	}
	else {
		s0=t0=0.0f;
		s1=m_size.x/float(m_tex->image_width()),
		t1=m_size.y/float(m_tex->image_height());
	}

	xvertex2d *vert=&m_mesh->get_vertex(0);
	vert->m_pos.set(m_pos.x,m_pos.y+m_size.y);
	vert->m_color=m_color;
	vert->m_texcoord.set(s0,t0);

	++vert;
	vert->m_pos.set(m_pos.x+m_size.x,m_pos.y+m_size.y);
	vert->m_color=m_color;
	vert->m_texcoord.set(s1,t0);

	++vert;
	vert->m_pos.set(m_pos.x,m_pos.y);
	vert->m_color=m_color;
	vert->m_texcoord.set(s0,t1);

	++vert;
	vert->m_pos.set(m_pos.x+m_size.x,m_pos.y);
	vert->m_color=m_color;
	vert->m_texcoord.set(s1,t1);
}

void xgui_image::_mesh_blit_33()
{
	//	---------------
	//	| 5 |  6  | 7 |
	//	---------------
	//	| 3 |     | 4 |	
	//	---------------
	//	| 0 |  1  | 2 |
	//	---------------

	assert(m_mesh);
	assert(m_mesh->vertex_count()>=16);
	assert(m_tex);

	const ximageset::subimage_t *img=0;
	float corner_w, corner_h;
	float s[4], t[4];
	if(!m_img_name.empty())
	{
		const ximageset *imgset=m_tex->get_imageset();
		if(imgset) img=imgset->get_image(m_img_name.c_str());
	}
	if(img) {
		corner_w=img->m_width/3.0f;
		corner_h=img->m_height/3.0f;
		s[0]=img->m_s0; 
		s[1]=float(img->m_xpos+corner_w)/m_tex->width();
		s[2]=float(img->m_xpos+img->m_width-corner_w)/m_tex->width();
		s[3]=img->m_s1;
		t[0]=img->m_t0;
		t[1]=float(img->m_ypos+corner_h)/m_tex->height();
		t[2]=float(img->m_ypos+img->m_height-corner_h)/m_tex->height();
		t[3]=img->m_t1;
	}
	else {
		corner_w=m_tex->image_width()/3.0f;
		corner_h=m_tex->image_height()/3.0f;
		s[0]=0;
		s[1]=float(corner_w)/m_tex->width();
		s[2]=float(m_tex->image_width()-corner_w)/m_tex->width();
		s[3]=float(m_tex->image_width())/m_tex->width();
		t[0]=0;
		t[1]=float(corner_h)/m_tex->height();
		t[2]=float(m_tex->image_height()-corner_h)/m_tex->height();
		t[3]=float(m_tex->image_height())/m_tex->height();
	}
	float x[4]={m_pos.x, m_pos.x+corner_w, m_pos.x+m_size.x-corner_w, m_pos.x+m_size.x};
	float y[4]={m_pos.y+m_size.y, m_pos.y+m_size.y-corner_h, m_pos.y+corner_h, m_pos.y};

	xvertex2d *vert=&m_mesh->get_vertex(0);
	for(int i=0; i<4; ++i) {
		for(int j=0; j<4; ++j) {
			vert->m_pos.set(x[j],y[i]);
			vert->m_color=m_color;
			vert->m_texcoord.set(s[j],t[i]);
			++vert;
		}
	}
}

void xgui_image::_mesh_blit_h3()
{
	//	---------------
	//	| 0 |  1  | 2 |
	//	---------------

	assert(m_mesh);
	assert(m_mesh->vertex_count()>=8);
	assert(m_tex);

	const ximageset::subimage_t *img=0;
	float corner_w, corner_h;
	float s[4], t[2];
	if(!m_img_name.empty())
	{
		const ximageset *imgset=m_tex->get_imageset();
		if(imgset) img=imgset->get_image(m_img_name.c_str());
	}
	if(img) {
		corner_w=img->m_width/3.0f;
		corner_h=float(img->m_height);
		s[0]=img->m_s0; 
		s[1]=float(img->m_xpos+corner_w)/m_tex->width();
		s[2]=float(img->m_xpos+img->m_width-corner_w)/m_tex->width();
		s[3]=img->m_s1;
		t[0]=img->m_t0;
		t[1]=img->m_t1;
	}
	else {
		corner_w=m_tex->image_width()/3.0f;
		corner_h=float(m_tex->image_height());
		s[0]=0;
		s[1]=float(corner_w)/m_tex->width();
		s[2]=float(m_tex->image_width()-corner_w)/m_tex->width();
		s[3]=float(m_tex->image_width())/m_tex->width();
		t[0]=0;
		t[1]=float(m_tex->image_height())/m_tex->height();
	}
	float x[4]={m_pos.x, m_pos.x+corner_w, m_pos.x+m_size.x-corner_w, m_pos.x+m_size.x};
	float y[2]={m_pos.y+m_size.y, m_pos.y};

	xvertex2d *vert=&m_mesh->get_vertex(0);
	for(int i=0; i<2; ++i) {
		for(int j=0; j<4; ++j) {
			vert->m_pos.set(x[j],y[i]);
			vert->m_color=m_color;
			vert->m_texcoord.set(s[j],t[i]);
			++vert;
		}
	}
}

void xgui_image::_mesh_blit_v3()
{
	//	-----
	//	| 2 |
	//	-----
	//	| 1 |
	//	-----
	//	| 0 |
	//	-----

	assert(m_mesh);
	assert(m_mesh->vertex_count()>=8);
	assert(m_tex);

	const ximageset::subimage_t *img=0;
	float corner_w, corner_h;
	float s[2], t[4];
	if(!m_img_name.empty())
	{
		const ximageset *imgset=m_tex->get_imageset();
		if(imgset) img=imgset->get_image(m_img_name.c_str());
	}
	if(img) {
		corner_w=float(img->m_width);
		corner_h=img->m_height/3.0f;
		s[0]=img->m_s0; 
		s[1]=img->m_s1;
		t[0]=img->m_t0;
		t[1]=float(img->m_ypos+corner_h)/m_tex->height();
		t[2]=float(img->m_ypos+img->m_height-corner_h)/m_tex->height();
		t[3]=img->m_t1;
	}
	else {
		corner_w=float(m_tex->image_width());
		corner_h=m_tex->image_height()/3.0f;
		s[0]=0;
		s[1]=float(m_tex->image_width())/m_tex->width();
		t[0]=0;
		t[1]=float(corner_h)/m_tex->height();
		t[2]=float(m_tex->image_height()-corner_h)/m_tex->height();
		t[3]=float(m_tex->image_height())/m_tex->height();
	}
	float x[2]={m_pos.x, m_pos.x+m_size.x};
	float y[4]={m_pos.y+m_size.y, m_pos.y+m_size.y-corner_h, m_pos.y+corner_h, m_pos.y};

	xvertex2d *vert=&m_mesh->get_vertex(0);
	for(int i=1; i>=0; --i) {
		for(int j=0; j<4; ++j) {
			vert->m_pos.set(x[i],y[j]);
			vert->m_color=m_color;
			vert->m_texcoord.set(s[i],t[j]);
			++vert;
		}
	}
}

}; // namespace wyc


