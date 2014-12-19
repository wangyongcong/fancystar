#include "fscorepch.h"
#include "wyc/game/game.h"
#include "wyc/world/camera.h"
#include "wyc/world/com_mesh.h"
#include "wyc/world/com_mesh_render.h"
#include "wyc/gui/gui_image.h"
#include "wyc/render/edtaa3func.h"

using wyc::xgame;
using wyc::xrenderer;
using wyc::xvec3f_t;
using wyc::xpointer;
using wyc::xobject;
using wyc::xgameobj;
using wyc::xcamera;
using wyc::xcom_transform;
using wyc::xcom_renderer;
using wyc::xcom_mesh;
using wyc::xcom_mesh_render;
using wyc::xcom_layout;
using wyc::xguiobj;
using wyc::xgui_image;

unsigned char *make_distance_map( unsigned char *img, unsigned int width, unsigned int height )
{
	short * xdist = (short *)  malloc( width * height * sizeof(short) );
	short * ydist = (short *)  malloc( width * height * sizeof(short) );
	double * gx = (double *) calloc( width * height, sizeof(double) );
	double * gy = (double *) calloc( width * height, sizeof(double) );
	double * data = (double *) calloc( width * height, sizeof(double) );
	double * outside = (double *) calloc( width * height, sizeof(double) );
	double * inside  = (double *) calloc( width * height, sizeof(double) );
	unsigned i;

	// Convert img into double (data)
	double img_min = 255, img_max = -255;
	for( i=0; i<width*height; ++i)
	{
		double v = img[i];
		data[i] = v;
		if (v > img_max) img_max = v;
		if (v < img_min) img_min = v;
	}
	// Rescale image levels between 0 and 1
	for( i=0; i<width*height; ++i)
	{
		data[i] = (img[i]-img_min)/img_max;
	}

	// Compute outside = edtaa3(bitmap); % Transform background (0's)
	computegradient( data, height, width, gx, gy);
	edtaa3(data, gx, gy, height, width, xdist, ydist, outside);
	for( i=0; i<width*height; ++i)
		if( outside[i] < 0 )
			outside[i] = 0.0;

	// Compute inside = edtaa3(1-bitmap); % Transform foreground (1's)
	memset(gx, 0, sizeof(double)*width*height );
	memset(gy, 0, sizeof(double)*width*height );
	for( i=0; i<width*height; ++i)
		data[i] = 1 - data[i];
	computegradient( data, height, width, gx, gy);
	edtaa3(data, gx, gy, height, width, xdist, ydist, inside);
	for( i=0; i<width*height; ++i)
		if( inside[i] < 0 )
			inside[i] = 0.0;

	// distmap = outside - inside; % Bipolar distance field
	unsigned char *out = (unsigned char *) malloc( width * height * sizeof(unsigned char) );
	for( i=0; i<width*height; ++i)
	{
		outside[i] -= inside[i];
		outside[i] = 128+outside[i]*16;
		if( outside[i] < 0 ) outside[i] = 0;
		if( outside[i] > 255 ) outside[i] = 255;
		out[i] = 255 - (unsigned char) outside[i];
		//out[i] = (unsigned char) outside[i];
	}

	free( xdist );
	free( ydist );
	free( gx );
	free( gy );
	free( data );
	free( outside );
	free( inside );
	return out;
}

class xgame_distance : public xgame
{
	wyc::xspherical<float> m_campos;
	xpointer<xcamera> m_cam_gui;
	xpointer<xgameobj> m_gui;
	std::vector<unsigned> m_shader_list;
	std::vector<xgui_image*> m_images;
	xgui_image *m_cur_img;
	wyc::xvec2f_t m_trans;
	wyc::xvec2f_t m_size;
	bool m_drag_mode;
public:
	xgame_distance() 
	{
		m_shader_list.push_back(wyc::strhash("simple2d"));
		m_shader_list.push_back(wyc::strhash("glyph_smooth"));
		m_shader_list.push_back(wyc::strhash("glyph_outline"));
		m_shader_list.push_back(wyc::strhash("glyph_glow"));
		m_cur_img=0;
		m_trans.zero();
		m_size.set(512,512);
		m_drag_mode=false;
	}
	virtual bool on_game_init() 
	{
	//	xgameobj *gobj;
		xcom_transform *trans;

		m_gui=wycnew xgameobj;
		m_gui->set_name("gui");
		m_gui->add_component<xcom_transform>();
		xcom_layout *layer=m_gui->add_component<xcom_layout>();
		
		const char *img_name[]={
			"res/edt2d/flipper128.png",
			"res/edt2d/flipper128_edt.png",
		//	"res/edt2d/circle256.png",
		//	"res/edt2d/circle256_edt.png",
		};
		
	/*	wyc::ximage edt_img;
		if(edt_img.load("res/edt2d/circle256.png",wyc::ximage::LUMINANCE_8)) {
			wyc::xcode_timer ct;
			ct.start();
			unsigned char *dt=make_distance_map(edt_img.bitmap(),edt_img.width(),edt_img.height());
			ct.stop();
			wyc_print("make_distance_map OK (%.4f)",ct.get_time());
			memcpy(edt_img.bitmap(),dt,edt_img.width()*edt_img.height());
			free(dt);
			edt_img.save_as("res/edt2d/circle256_edt.png");
		}
	*/	

		const unsigned img_count=sizeof(img_name)/sizeof(char*);
		xgui_image *gui_img;
		char name[32];
		for(unsigned i=0; i<img_count; ++i) {
			sprintf_s(name, 32, "image%d",i);
			gui_img=wyc_safecast(xgui_image,layer->new_element("xgui_image",name,0,0,1));
			gui_img->set_size(m_size.x,m_size.y);
			gui_img->set_position(0.5f*(client_width()-gui_img->get_size().x)+m_trans.x,
				0.5f*(client_height()-gui_img->get_size().y)+m_trans.y);
		//	gui_img->set_color(0xFF00FF00);
			gui_img->set_image(img_name[i]);
			gui_img->hide();
			m_images.push_back(gui_img);
		}
		m_cur_img=m_images.front();
		m_cur_img->show();

		m_cam_gui=wycnew xcamera;
		m_cam_gui->set_name("gui camera");
		m_cam_gui->set_orthograph(0,0,1,800,600,100);
		trans=m_cam_gui->get_component<xcom_transform>();
		trans->set_position(xvec3f_t(0,0,10));
		m_cam_gui->look_at(xvec3f_t(0,0,0),xvec3f_t(0,1,0));
		m_cam_gui->update(0,0);

	//	glClearColor(1.0f,1.0f,1.0f,1.0f);
		glClearColor(0.0f,0.0f,0.0f,1.0f);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	//	glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

		m_renderer->use_shader(m_shader_list[0]);
		const wyc::xshader *shader=m_renderer->get_shader();
		wyc_print("use shader: %s",shader->name().c_str());

		return true;
	}
	virtual void on_game_exit() {
		wyc_print("quit game");
		m_gui=0;
		m_cam_gui=0;
	}
	virtual void process_input()
	{
		input().update_input();
		const wyc::xinput::xinputbuffer &ibuff=input().get_buffer();
		for(wyc::xinput::xkeyque::const_iterator key_iter=ibuff.keyque.begin(), 
			key_end=ibuff.keyque.end(); key_iter!=key_end; ++key_iter)
		{
			if(wyc::EV_KEY_DOWN==key_iter->msg) 
				on_key_down(key_iter->key);
		}
		bool drag_mode=m_drag_mode;
		for(wyc::xinput::xmouseque::const_iterator mo_iter=ibuff.mouseque.begin(), 
			mo_end=ibuff.mouseque.end(); mo_iter!=mo_end; ++mo_iter) {
				if(wyc::EV_LB_DOWN==mo_iter->msg)
					m_drag_mode=true;
				else if(wyc::EV_LB_UP==mo_iter->msg)
					m_drag_mode=false;
		}
		if(!m_cur_img)
			return;
		bool changed=false;
		if(m_drag_mode && drag_mode)
		{
			if(ibuff.offx || ibuff.offy) {
				m_trans.x+=ibuff.offx;
				m_trans.y-=ibuff.offy;
				changed=true;
			}
		}
		if(ibuff.offz) {
			float s=1.0f+ibuff.offz*0.0005f;
			m_size*=s;
			m_trans*=s;
			changed=true;
			wyc_print("scale to: %.2fx%.2f",m_size.x,m_size.y);
		}
		if(changed) {
			m_cur_img->set_size(m_size.x,m_size.y);
			m_cur_img->set_position(0.5f*(client_width()-m_size.x)+m_trans.x, 0.5f*(client_height()-m_size.y)+m_trans.y);
		}
	}
	virtual void on_logic(double accum_time, double frame_time) 
	{
		m_gui->update(accum_time, frame_time);
		assert(glGetError()==GL_NO_ERROR);
	}
	virtual void on_render(double, double)
	{
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		const wyc::xshader *shader;
		GLint uf;
		shader=m_renderer->get_shader();
		uf=shader->get_uniform("mat_camera");
		xcom_transform *trans=m_cam_gui->get_component<xcom_transform>();
		const wyc::xmat4f_t &w2l=trans->world2local();
		wyc::xmat4f_t mat_cam=m_cam_gui->get_projection()*w2l;
		glUniformMatrix4fv(uf,1,GL_FALSE,mat_cam.data());
		uf=shader->get_uniform("texture");
		glUniform1i(uf,0);
		m_gui->render(m_renderer);
		assert(glGetError()==GL_NO_ERROR);
	}
	void on_key_down (unsigned key_code)
	{
		if(key_code>'0' && key_code<='9') {
			unsigned idx=(key_code-49)%m_images.size();
			if(m_images[idx]!=m_cur_img) {
				m_cur_img->hide();
				m_cur_img=m_images[idx];
				m_cur_img->set_size(m_size.x,m_size.y);
				m_cur_img->set_position(0.5f*(client_width()-m_size.x)+m_trans.x, 0.5f*(client_height()-m_size.y)+m_trans.y);
				m_cur_img->show();
				wyc_print("show image[%d]: %s",idx,m_cur_img->name().c_str());
			}
		}
		else if(VK_SPACE==key_code) {
			if(m_renderer->use_shader(m_shader_list[0])) {
				const wyc::xshader* shader=m_renderer->get_shader();
				wyc_print("use shader: %s",shader->name().c_str());
			}
		}
		else if('G'==key_code) {
			if(m_renderer->use_shader(m_shader_list[3])) {
				const wyc::xshader* shader=m_renderer->get_shader();
				wyc_print("use shader: %s",shader->name().c_str());
			}
		}
		else if('O'==key_code) {
			if(m_renderer->use_shader(m_shader_list[2])) {
				const wyc::xshader* shader=m_renderer->get_shader();
				wyc_print("use shader: %s",shader->name().c_str());
			}
		}
		else if('S'==key_code) {
			if(m_renderer->use_shader(m_shader_list[1])) {
				const wyc::xshader* shader=m_renderer->get_shader();
				wyc_print("use shader: %s",shader->name().c_str());
			}
		}
	}
};

xgame* xgame::create_game() 
{
	return new xgame_distance;
}




