#include "fscorepch.h"
#include "xobject2d_render.h"

EXPORT_STATIC_MODULE(xobject2d_render);

namespace wyc
{

BEGIN_EVENT_MAP(xobject2d_render,xrenderobj)
	REG_EVENT(ev_set_parent)
	REG_EVENT(ev_transform)
	REG_EVENT(ev_set_color)
	REG_EVENT(ev_set_image)
	REG_EVENT(ev_set_keypoint)
	REG_EVENT(ev_set_pick)
	REG_EVENT(ev_prepare_draw)
	REG_EVENT(ev_show_frame)
	REG_EVENT(ev_hide_frame)
	REG_EVENT(ev_enable_frame)
	REG_EVENT(ev_disable_frame)
	REG_EVENT(ev_set_frame_color)
END_EVENT_MAP

xobject2d_render::xobject2d_render()
{
	m_parent=0;
	m_pos.set_zero();
	m_size.set_zero();
	m_rotate=0;
	memset(m_color,0xFF,sizeof(uint32_t)*4);
	m_kp.set_zero();
	m_pickCode=0;
	m_alphaFilter=1;
	m_pTilebuffer=0;
	m_flag=0;
	m_pFrame=0;
}

void xobject2d_render::on_destroy()
{
	xrenderobj::on_destroy();
	m_spMesh=0;
	m_spImage=0;
	destroy_frame();
	child_list_t::iterator iter, end=m_children.end();
	for(iter=m_children.begin(); iter!=end; ++iter) 
		(*iter)->decref();
}

void xobject2d_render::get_world_pos(xvec2f_t &pos) const
{
	pos.x=m_pos.x, pos.y=m_pos.y;
	xobject2d_render *parent=m_parent;
	while(parent) {
		pos+=parent->get_pos();
		parent=parent->m_parent;
	}
}

//-------------------------------------------------------------------------

void xobject2d_render::ev_set_parent(xobjevent *pev) 
{
	unsigned parent_id;
	float z;
	if(!((xpackev*)pev)->unpack("df",&parent_id,&z)) {
		return;
	}
	xrenderer *pRenderer=xrenderer::get_renderer();
	xobject2d_render *parent=wyc_safecast(xobject2d_render,pRenderer->get_object(parent_id));
	if(parent==m_parent) 
		return;
	if(parent) // 先增加引用避免delthis
		incref();
	if(m_parent) {
		child_list_t::iterator iter=m_parent->m_children.find(this);
		if(iter!=m_parent->m_children.end()) {
			// release mesh
			m_spMesh=0;
			m_parent->m_children.erase(iter);
			decref();
		}
	}
	m_parent=parent;
	m_pos.z=z;
	if(m_parent)
		m_parent->m_children.insert(this);
	// rebuild mesh (position, vertex color, texture coord)
	m_flag|=OBJ2D_MESH_CHANGED|OBJ2D_COLOR_CHANGED|OBJ2D_IMAGE_CHANGED;
}

void xobject2d_render::ev_transform(xobjevent *pev)
{
	float *pos, *size;
	if(!((xpackev*)pev)->unpack("3ff2f",&pos,&m_rotate,&size)) {
		return;
	}
	if(pos[2]!=m_pos.z && m_parent) {
		child_list_t::iterator iter=m_parent->m_children.find(this);
		assert(iter!=m_parent->m_children.end());
		m_parent->m_children.erase(iter);
		m_pos.set(pos[0],pos[1],pos[2]);
		m_parent->m_children.insert(this);
	}
	else m_pos.set(pos[0],pos[1],pos[2]);
	m_size.set(size[0],size[1]);
	add_state(m_flag,OBJ2D_MESH_CHANGED);
}

void xobject2d_render::ev_set_color(xobjevent *pev)
{
	uint32_t *color;
	if(!((xpackev*)pev)->unpack("4d",&color)) {
		return;
	}
	memcpy(m_color,color,sizeof(uint32_t)*4);
	add_state(m_flag,OBJ2D_COLOR_CHANGED);
}

void xobject2d_render::ev_set_image(xobjevent *pev)
{
	const char *imgName;
	int blitType;
	float *kp;
	if(!((xpackev*)pev)->unpack("sd2f",&imgName,&blitType,&kp)) {
		return;
	}
	if(kp[0]!=m_kp.x && kp[1]!=m_kp.y) {
		m_kp.set(kp[0],kp[1]);
		add_state(m_flag,OBJ2D_MESH_CHANGED);
	}
	xrenderer *pRenderer=xrenderer::get_renderer();
	m_spImage=(xtexture*)pRenderer->get_texture(imgName);
	add_state(m_flag,OBJ2D_BLIT_MASK,BLIT_TYPE(blitType));
	add_state(m_flag,OBJ2D_IMAGE_CHANGED);
}

void xobject2d_render::ev_set_keypoint(xobjevent *pev)
{
	float *kp;
	if(!((xpackev*)pev)->unpack("2f",&kp))
		return;
	m_kp.set(kp[0],kp[1]);
	add_state(m_flag,OBJ2D_MESH_CHANGED);
}

void xobject2d_render::ev_set_pick(xobjevent *pev)
{
	int type;
	uint32_t code;
	float filter;
	if(!((xpackev*)pev)->unpack("ddf",&type,&code,&filter))
		return;
	m_pickCode=code;
	m_alphaFilter=type==PICK_ALPHA?filter:1;
	m_flag&=~OBJ2D_PICK_MASK;
	m_flag|=PICK_TYPE(type)<<OBJ2D_PICK_SHIFT;
}

void xobject2d_render::ev_set_visible(xobjevent *pev)
{
	int visible;
	if(!((xpackev*)pev)->unpack("d",&visible))
		return;
	if(visible) show();
	else hide();
}

xtile_buffer* xobject2d_render::get_tile_buffer()
{
	if(m_pTilebuffer)
		return m_pTilebuffer;
	if(m_parent)
		return m_parent->get_tile_buffer();
	return 0;
}

void xobject2d_render::ev_prepare_draw(xobjevent*)
{
	static const unsigned short BLIT_TYPE_2_VERTEX_COUNT[MAX_BLIT_TYPE]={
		4,4,4,4,6,6,9,
	};
	typedef void (*fn_build_mesh) (xobject2d_render*,int);
	static fn_build_mesh BLIT_TYPE_2_BUILDER[MAX_BLIT_TYPE]={
		&mesh_no_blit,
		&mesh_blit_h,
		&mesh_blit_v,
		&mesh_blit_hv,
		&mesh_blit_h3,
		&mesh_blit_v3,
		&mesh_blit_33,
	};
	BLIT_TYPE blit=BLIT_TYPE(m_flag&OBJ2D_BLIT_MASK);
	unsigned short count=BLIT_TYPE_2_VERTEX_COUNT[blit];
	int flag=0;
	if(!m_spMesh || m_spMesh->vertex_count()<count) {
		// rebuild all !
		if(!m_pTilebuffer) {
			m_pTilebuffer=get_tile_buffer();
			if(!m_pTilebuffer)
				return;
		}
		m_spMesh=m_pTilebuffer->alloc_mesh(count);
		flag=BUILD_ALL;
	}
	else {
		if(have_state(m_flag,OBJ2D_MESH_CHANGED)) {
			// update vertex position
			flag|=BUILD_POSITION|BUILD_TEXTURE;
		}
		if(have_state(m_flag,OBJ2D_COLOR_CHANGED)) {
			// update vertex color
			flag|=BUILD_COLOR;
		}
		if(have_state(m_flag,OBJ2D_IMAGE_CHANGED)) {
			// update texcoord
			flag|=BUILD_TEXTURE;
		}
	}
	if(flag) {
		fn_build_mesh pfnBuildMesh=BLIT_TYPE_2_BUILDER[blit];
		pfnBuildMesh(this,flag);
		m_spMesh->commit_vertex();
	}
	if(have_state(m_flag,OBJ2D_SHOW_FRAME)) {
		if(!m_pFrame)
			create_frame();
		else if(flag&BUILD_POSITION) {
			update_frame_position();
			m_pFrame->m_spFrameMesh->commit_vertex();
		}
	}
	remove_state(m_flag,OBJ2D_CHANGED_MASK);
}

void xobject2d_render::r_prepare_draw_mesh(xrender_context &rc, xmesh2d *pMesh)
{
	if(pMesh->get_buffer()!=rc.m_vbo) {
		rc.m_vbo=pMesh->get_buffer();
		glBindBuffer(GL_ARRAY_BUFFER,rc.m_vbo);
	}
	if(pMesh->get_texture()!=rc.m_tex) {
		rc.m_tex=pMesh->get_texture();
		glBindTexture(GL_TEXTURE_2D,rc.m_tex);
	}
	/*
	if(pMesh->wrap_s()!=rc.m_wrap_s) {
		rc.m_wrap_s=pMesh->wrap_s();
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,rc.m_wrap_s);
	}
	if(pMesh->wrap_t()!=rc.m_wrap_t) {
		rc.m_wrap_t=pMesh->wrap_t();
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,rc.m_wrap_t);
	}*/
}

void xobject2d_render::r_draw2d(xrender_context &rc)
{
	// draw self
	if(have_state(m_flag,OBJ2D_SHOW_FRAME)) {
	//	assert(m_spFrameMesh);
		if(m_pFrame) {
			r_prepare_draw_mesh(rc,m_pFrame->m_spFrameMesh);
			for(int i=0; i<32; i+=4) 
				m_pFrame->m_spFrameMesh->draw(i,4);
		}
		else
		{
			wyc_warn("m_spFrameMesh==0");
		}
	}
	if(m_spMesh && m_spImage) {
		r_prepare_draw_mesh(rc,m_spMesh);
		m_spMesh->draw();
	}
	// draw children
	if(!m_children.empty()) {
		xvec2f_t translate;
		if(m_parent) 
			translate=m_parent->get_pos();
		else
			translate.set_zero();
		glUniform2f(rc.m_uniTrans,translate.x+m_pos.x,translate.y+m_pos.y);

		GLint scissor[4];
		glGetIntegerv(GL_SCISSOR_BOX,scissor);
		glScissor(int(m_pos.x-m_kp.x),int(m_pos.y-m_kp.y),int(m_size.x),int(m_size.y));

		xobject2d_render *pobj;
		child_list_t::iterator iter, end=m_children.end();
		for(iter=m_children.begin(); iter!=end; ++iter) {
			pobj=*iter;
			if(pobj->visible())
				pobj->r_draw2d(rc);
		}

		glScissor(scissor[0],scissor[1],scissor[2],scissor[3]);
		glUniform2f(rc.m_uniTrans,translate.x,translate.y);
	}
}

void xobject2d_render::r_picker(xrender_context &rc)
{
	// draw self
	float factor=1.0f/255;
	if(have_state(m_flag,OBJ2D_PICK_FRAME)) {
		if(m_pFrame) {
			r_prepare_draw_mesh(rc,m_pFrame->m_spFrameMesh);
			uint32_t code;
			for(int i=0, j=0; i<32; i+=4, j+=1) {
				code=m_pFrame->m_frameCode[j];
				glUniform4f(rc.m_uniPick,(code&0xFF)*factor,((code&0xFF00)>>8)*factor,\
					((code&0xFF0000)>>16)*factor,1);
				m_pFrame->m_spFrameMesh->draw(i,4);
			}
		}
	}
	if(m_spMesh) {
		r_prepare_draw_mesh(rc,m_spMesh);
		glUniform4f(rc.m_uniPick,(m_pickCode&0xFF)*factor,((m_pickCode&0xFF00)>>8)*factor,\
			((m_pickCode&0xFF0000)>>16)*factor,m_alphaFilter);
		m_spMesh->draw();
	}
	// draw children
	if(!m_children.empty()) {
		xvec2f_t translate;
		if(m_parent) 
			translate=m_parent->get_pos();
		else
			translate.set_zero();
		glUniform2f(rc.m_uniTrans,translate.x+m_pos.x,translate.y+m_pos.y);
		xobject2d_render *pobj;
		child_list_t::iterator iter, end=m_children.end();
		for(iter=m_children.begin(); iter!=end; ++iter) {
			pobj=*iter;
			if(pobj->visible() && pobj->pickable())
				pobj->r_picker(rc);
		}
		glUniform2f(rc.m_uniTrans,translate.x,translate.y);
	}
}

void xobject2d_render::ev_show_frame(xobjevent*)
{
	if(have_state(m_flag,OBJ2D_SHOW_FRAME))
		return;
	m_flag|=OBJ2D_SHOW_FRAME;
}

void xobject2d_render::ev_hide_frame(xobjevent*)
{
	remove_state(m_flag,OBJ2D_SHOW_FRAME);
}

void xobject2d_render::ev_enable_frame(xobjevent *pev)
{
	if(have_state(m_flag,OBJ2D_PICK_FRAME))
		return;
	uint32_t *pcode;
	if(!((xpackev*)pev)->unpack("8d",&pcode)) 
		return;
	if(m_pFrame) {
		memcpy(m_pFrame->m_frameCode,pcode,sizeof(uint32_t)*8);
		add_state(m_flag,OBJ2D_PICK_FRAME);
	}
}

void xobject2d_render::ev_disable_frame(xobjevent*)
{
	if(!have_state(m_flag,OBJ2D_PICK_FRAME))
		return;
	remove_state(m_flag,OBJ2D_PICK_FRAME);
	if(m_pFrame) {
		memset(m_pFrame->m_frameCode,0,sizeof(uint32_t)*8);
	}
}

void xobject2d_render::ev_set_frame_color(xobjevent *pev)
{
	if(!m_pFrame)
		return;
	uint32_t *pcolor;
	if(((xpackev*)pev)->unpack("8d",&pcolor)) {
		update_frame_color(8,pcolor);
		m_pFrame->m_spFrameMesh->commit_vertex();
	}
	else {
		uint32_t idx, c;
		if(((xpackev*)pev)->unpack("dd",&idx,&c)) {
			if(idx<8) {
				update_frame_color(idx,c);
				m_pFrame->m_spFrameMesh->commit_vertex();
			}
		}
	}
}

void xobject2d_render::create_frame()
{
	/***********************
		-----------------
		| 5 |   6   | 7 |
		-----------------
		|   |       |   | 
		| 3 |       | 4 |	
		|   |       |   |	
		-----------------
		| 0 |   1   | 2 |
		-----------------
	************************/
	if(m_pFrame) 
		return;
	m_pFrame = new xobjframe;
	xmesh2d *pmesh=m_pTilebuffer->alloc_mesh(32);
	m_pFrame->m_spFrameMesh=pmesh;
	GLushort base=0;
	GLushort index[4]={0,1,2,3};
	for(int i=0; i<8; ++i) {
		pmesh->append_index(index,4,base);
		base+=4;
	}
	pmesh->set_mode(GL_TRIANGLE_STRIP);
	m_pFrame->m_spFrameImage=(xtexture*)xrenderer::get_renderer()->get_texture("fs_blank");
	pmesh->set_texture(m_pFrame->m_spFrameImage->handle(),GL_REPEAT,GL_REPEAT);
	m_pFrame->m_frameWidth=8;
	update_frame_position();
	update_frame_texture();
	uint32_t frameColor=0xFF00FF00;
	update_frame_color(1,&frameColor);
	for(int i=0; i<8; ++i)
		m_pFrame->m_frameCode[i]=0;
	pmesh->commit_vertex();
}

void xobject2d_render::destroy_frame()
{
	if(m_pFrame) {
		m_pFrame->m_spFrameMesh=0;
		m_pFrame->m_spFrameImage=0;
		delete m_pFrame;
		m_pFrame=0;
	}
}

void xobject2d_render::update_frame_position()
{
	assert(m_pFrame);
	assert(m_pFrame->m_spFrameMesh);
	xmesh2d &mesh=*m_pFrame->m_spFrameMesh;
	float x=-m_kp.x, y=-m_kp.y, x1, y1;
	float halfw=m_pFrame->m_frameWidth*0.5f;
	float orgx[4]={
		x-halfw, 
		x+halfw, 
		x+m_size.x-halfw, 
		x+m_size.x+halfw,
	};
	float orgy[4]={
		y-halfw, 
		y+halfw, 
		y+m_size.y-halfw, 
		y+m_size.y+halfw,
	};
	unsigned bid=0;
	xvertex2d *vert;
	for(int i=0; i<3; ++i) {
		y=orgy[i], y1=orgy[i+1];
		for(int j=0; j<3; ++j) {
			if(i==1 && j==1) 
				continue;
			x=orgx[j], x1=orgx[j+1];
			vert=&mesh[bid];
			vert->m_pos.set(x,y);
			++vert;
			vert->m_pos.set(x1,y);
			++vert;
			vert->m_pos.set(x,y1);
			++vert;
			vert->m_pos.set(x1,y1);
			bid+=4;
		}
	}
	if(m_rotate!=0) {
		xmat2f_t mat;
		matrix_rotate2d(mat,DEG_TO_RAD(m_rotate));
		vert=&mesh[0];
		for(int i=0; i<32; ++i) {
			vert->m_pos=vert->m_pos*mat;
			vert->m_pos+=m_pos;
			vert+=1;
		}
	}
	else {
		vert=&mesh[0];
		for(int i=0; i<32; ++i) {
			vert->m_pos+=m_pos;
			vert+=1;
		}
	}
}

void xobject2d_render::update_frame_texture()
{
	assert(m_pFrame);
	assert(m_pFrame->m_spFrameImage);
	xmesh2d &mesh=*m_pFrame->m_spFrameMesh;
	xtexture &tex=*m_pFrame->m_spFrameImage;
	float x=1.0f/tex.width(), 
		y=1.0f/tex.height(),
		x1, y1;
	float s[4]={
		0, m_pFrame->m_frameWidth*x, 
		m_size.x*x, (m_size.x+m_pFrame->m_frameWidth)*x,
	};
	float t[4]={
		0, m_pFrame->m_frameWidth*y,
		m_size.y*y, (m_size.y+m_pFrame->m_frameWidth)*y,
	};
	unsigned bid=0;
	xvertex2d *vert;
	for(int i=0; i<3; ++i) {
		y=t[i], y1=t[i+1];
		for(int j=0; j<3; ++j) {
			if(i==1 && j==1) 
				continue;
			x=s[j], x1=s[j+1];
			vert=&mesh[bid];
			vert->m_texcoord.set(x,y);
			++vert;
			vert->m_texcoord.set(x1,y);
			++vert;
			vert->m_texcoord.set(x,y1);
			++vert;
			vert->m_texcoord.set(x1,y1);
			bid+=4;
		}
	}
}

void xobject2d_render::update_frame_color(unsigned count, uint32_t *pc)
{
	assert(m_pFrame);
	assert(m_pFrame->m_spFrameMesh);
	assert(count>0);
	xmesh2d &mesh=*m_pFrame->m_spFrameMesh;
	uint32_t *last=pc+count-1;
	unsigned bid=0;
	xvertex2d *vert;
	for(int i=0; i<3; ++i) {
		for(int j=0; j<3; ++j) {
			if(i==1 && j==1) 
				continue;
			vert=&mesh[bid];
			vert->m_color=*pc;
			++vert;
			vert->m_color=*pc;
			++vert;
			vert->m_color=*pc;
			++vert;
			vert->m_color=*pc;
			bid+=4;
			if(pc<last) ++pc;
		}
	}
}

void xobject2d_render::update_frame_color(unsigned idx, uint32_t c)
{
	assert(m_pFrame);
	assert(m_pFrame->m_spFrameMesh);
	assert(idx<8);
	xmesh2d &mesh=*m_pFrame->m_spFrameMesh;
	xvertex2d *vert=&mesh[idx<<2];
	vert->m_color=c;
	++vert;
	vert->m_color=c;
	++vert;
	vert->m_color=c;
	++vert;
	vert->m_color=c;
}

//---------------------------------------------------------------------

void xobject2d_render::mesh_no_blit(xobject2d_render *pobj, int flag)
{
	xmesh2d &mesh=*pobj->m_spMesh;
	assert(mesh.vertex_count()>=4);
	xvertex2d *vert=0;
	if(flag&BUILD_POSITION) {
		build_mesh_quad(pobj);
	}
	if(flag&BUILD_TEXTURE && pobj->m_spImage) {
		const xvec4f_t &coord=pobj->m_spImage->get_coord();
		vert=&mesh[0];
		vert->m_texcoord.set(coord[0],coord[1]);
		vert+=1;
		vert->m_texcoord.set(coord[2],coord[1]);
		vert+=1;
		vert->m_texcoord.set(coord[0],coord[3]);
		vert+=1;
		vert->m_texcoord.set(coord[2],coord[3]);
		mesh.set_texture(pobj->m_spImage->handle(),GL_CLAMP,GL_CLAMP);
	}
	if(flag&BUILD_COLOR) {
		vert=&mesh[0];
		for(int i=0; i<4; ++i) {
			vert->m_color=pobj->m_color[i];
			vert+=1;
		}
	}
	if(flag&BUILD_INDEX) {
		GLushort index[4]={0, 1, 2, 3};
		mesh.set_index(GL_TRIANGLE_STRIP,index,4);
	}
}

void xobject2d_render::mesh_blit_h (xobject2d_render *pobj, int flag)
{
	pobj,flag;
}

void xobject2d_render::mesh_blit_v (xobject2d_render *pobj, int flag)
{
	pobj,flag;
}

void xobject2d_render::mesh_blit_hv(xobject2d_render *pobj, int flag)
{
	xmesh2d &mesh=*pobj->m_spMesh;
	assert(mesh.vertex_count()>=4);
	xvertex2d *vert=0;
	if(flag&BUILD_POSITION) {
		build_mesh_quad(pobj);
	}
	if(flag&BUILD_TEXTURE && pobj->m_spImage) {
		xvec4f_t coord;
		float s=pobj->m_size.x/pobj->m_spImage->width(), 
			t=pobj->m_size.y/pobj->m_spImage->height();
		vert=&mesh[0];
		vert->m_texcoord.set(0,0);
		vert+=1;
		vert->m_texcoord.set(s,0);
		vert+=1;
		vert->m_texcoord.set(0,t);
		vert+=1;
		vert->m_texcoord.set(s,t);
		mesh.set_texture(pobj->m_spImage->handle(),GL_REPEAT,GL_REPEAT);
	}
	if(flag&BUILD_COLOR) {
		vert=&mesh[0];
		for(int i=0; i<4; ++i) {
			vert->m_color=pobj->m_color[i];
			vert+=1;
		}
	}
	if(flag&BUILD_INDEX) {
		GLushort index[4]={0, 1, 2, 3};
		mesh.set_index(GL_TRIANGLE_STRIP,index,4);
	}
}

void xobject2d_render::mesh_blit_h3(xobject2d_render *pobj, int flag)
{
	pobj,flag;
}

void xobject2d_render::mesh_blit_v3(xobject2d_render *pobj, int flag)
{
	pobj,flag;
}

void xobject2d_render::mesh_blit_33(xobject2d_render *pobj, int flag)
{
	pobj,flag;
}

void xobject2d_render::build_mesh_quad(xobject2d_render *pobj)
{
	/*************
		2----3
		|    |
		0----1
	*************/
	xmesh2d &mesh=*pobj->m_spMesh;
	assert(mesh.vertex_count()>=4);
	float x=-pobj->m_kp.x, y=-pobj->m_kp.y;	
	float w=pobj->m_size.x, h=pobj->m_size.y;
	xvertex2d *vert=&mesh[0];
	vert->m_pos.set(x,y);
	vert+=1;
	vert->m_pos.set(x+w,y);
	vert+=1;
	vert->m_pos.set(x,y+h);
	vert+=1;
	vert->m_pos.set(x+w,y+h);
	if(pobj->m_rotate!=0) {
		xmat2f_t mat;
		matrix_rotate2d(mat,DEG_TO_RAD(pobj->m_rotate));
		vert=&mesh[0];
		for(int i=0; i<4; ++i) {
			vert->m_pos=vert->m_pos*mat;
			vert->m_pos.x+=pobj->m_pos.x;
			vert->m_pos.y+=pobj->m_pos.y;
			vert+=1;
		}
	}
	else {
		vert=&mesh[0];
		for(int i=0; i<4; ++i) {
			vert->m_pos.x+=pobj->m_pos.x;
			vert->m_pos.y+=pobj->m_pos.y;
			vert+=1;
		}
	}
}

}; // namespace wyc

