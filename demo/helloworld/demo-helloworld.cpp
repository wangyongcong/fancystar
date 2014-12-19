#include "fscorepch.h"

#include "wyc/math/transform.h"

#include "wyc/game/game.h"
#include "wyc/gui/gui_image.h"
#include "wyc/gui/gui_text.h"
#include "wyc/gui/gui_rtinfo.h"
#include "wyc/util/strutil.h"

#include "shaderlib.h"
#include "simple_camera.h"
#include "simple_mesh.h"
#include "simple_light.h"
#include "render_tech.h"

using namespace wyc;

enum SHAPE_ID
{
	SHAPE_CUBE = 0,
	SHAPE_CYLINDER,
	SHAPE_SPHERE,
	SHAPE_TORUS,
	SHAPE_COUNT,
};

const char *s_model_files[SHAPE_COUNT]={
	"cube",
	"cylinder",
	"sphere",
	"torus",
};

enum RENDER_TECH
{
	TECH_FLAT_COLOR=0,
	TECH_BASE_MAP,
	TECH_LIGHTING,
	TECH_BUMP_MAP,
	TECH_PARALLAX,
	TECH_RELIEF,
	TECH_COUNT,
};

const char *s_render_techs[TECH_COUNT]={
	"xtech_flat_color",
	"xtech_base_map",
	"xtech_lighting",
	"xtech_bump_map",
	"xtech_parallax",
	"xtech_relief",
};

const wchar_t *s_tech_name[TECH_COUNT]={
	L"phone shading",
	L"apply texture",
	L"diffuse mapping",
	L"normal mapping",
	L"parallax mapping",
	L"relief mapping",
};

enum TEXTURE_INDEX
{
	IDX_DIFFUSE = 0,
	IDX_DISP,
	IDX_NORMAL,
};

const char *s_textures[][3] = {
	{"texture/bricks01-diffuse.jpg","texture/bricks01-disp.jpg","texture/bricks01-normal.jpg"},
	{"texture/paver08-diffuse.jpg","texture/paver08-disp.jpg","texture/paver08-normal.jpg"},
};

class xgame_helloworld : public xgame
{
	// generic resource
	xpointer<xfont> m_font_sys;
	// camera control
	xmat4f_t m_proj_gui;
	xpointer<xsimple_camera> m_camera;
	// world primitives
	int m_shape;
	xpointer<xsimple_mesh> m_meshes[SHAPE_COUNT];
	xpointer<xtexture> m_base_map;
	xpointer<xtexture> m_normal_map;
	xpointer<xtexture> m_height_map;
	// render techniques
	std::vector<xrender_tech*> m_techs;
	unsigned m_tech_idx;
	xpointer<xrender_tech> m_tech_billboard,
		m_tech_wireframe, m_tech_normals;
	// lights
	std::vector<xsimple_light*> m_lights;
	xpointer<xtexture> m_light_icon;
	// ui components
	xpointer<xlayout> m_layout;
	unsigned m_texture_idx;
	// game status
	bool m_move_cam;
	bool m_wireframe;
	bool m_normals;
public:
	xgame_helloworld() 
	{
		m_shape=SHAPE_CUBE;
		m_move_cam=false;
		m_wireframe=false;
		m_normals=false;
		m_tech_idx = TECH_LIGHTING;
		m_texture_idx = 0;

		set_code_name("fs-hello");
		set_app_name(L"Hello world");

		int tech_count = sizeof(s_render_techs)/sizeof(char*);
		m_techs.reserve(tech_count);
		xobject *tech;
		for(int i=0; i<tech_count; ++i)
		{
			tech = xobject::create_object(s_render_techs[i]);
			if(tech) {
				tech->incref();
				m_techs.push_back((xrender_tech*)tech);
			}
		}
		tech = xobject::create_object("xtech_billboard");
		assert(tech);
		m_tech_billboard = wyc_safecast(xrender_tech,tech);
		assert(m_tech_billboard);
		tech = xobject::create_object("xtech_wireframe");
		assert(tech);
		m_tech_wireframe = wyc_safecast(xrender_tech,tech);
		assert(m_tech_wireframe);
		tech = xobject::create_object("xtech_normals");
		assert(tech);
		m_tech_normals = wyc_safecast(xrender_tech,tech);
		assert(m_tech_normals);
	}
	virtual bool on_game_init() 
	{
		std::string path;
		xressvr *svr = get_resource_server();

		path="font/vera10.font";
		get_resource_path(path);
		m_font_sys = (xfont*)svr->request(xfont::get_class()->id,path.c_str());
		if(!m_font_sys->is_complete())
		{
			wyc_error("fail to load font: %s",path.c_str());
		}

		GLenum wrap_mode = GL_CLAMP_TO_EDGE; // GL_REPEAT or GL_CLAMP_TO_EDGE
		path=s_textures[m_texture_idx][IDX_DIFFUSE];
		get_resource_path(path);
		m_base_map = (xtexture*)svr->request(xtexture::get_class()->id,path.c_str());
		if(!m_base_map->is_complete())
		{
			wyc_error("faile to load texture: %s",path.c_str());
		}
		m_base_map->set_wrap_mode(wrap_mode,wrap_mode);

		path=s_textures[m_texture_idx][IDX_NORMAL];
		get_resource_path(path);
		m_normal_map = (xtexture*)svr->request(xtexture::get_class()->id,path.c_str());
		if(!m_normal_map->is_complete())
		{
			wyc_error("faile to load texture: %s",path.c_str());
		}
		m_normal_map->set_wrap_mode(wrap_mode,wrap_mode);

		path=s_textures[m_texture_idx][IDX_DISP];
		get_resource_path(path);
		m_height_map = (xtexture*)svr->request(xtexture::get_class()->id,path.c_str());
		if(!m_height_map->is_complete())
		{
			wyc_error("faile to load texture: %s",path.c_str());
		}
		m_height_map->set_wrap_mode(wrap_mode,wrap_mode);

		path="texture/icon-light.png";
		get_resource_path(path);
		m_light_icon = (xtexture*)svr->request(xtexture::get_class()->id,path.c_str());
		if(!m_light_icon->is_complete())
		{
			wyc_error("faile to load texture: %s",path.c_str());
		}

		// initialize GUI
		set_ui_projection(m_proj_gui,float(client_width()),float(client_height()),1000);
		m_layout = wycnew xlayout;
		wyc::xguiobj *gui_obj=m_layout->new_element("xgui_rtinfo","rt",4,0);
		wyc::xgui_rtinfo *gui_rt=wyc_safecast(wyc::xgui_rtinfo,gui_obj);
		gui_rt->set_font(m_font_sys);
		gui_rt->set_color(0,255,0,255);
		gui_rt->set_column(1);
		gui_rt->set_entry(L"shader",s_tech_name[m_tech_idx]);
		gui_rt->set_entry(L"FPS",L"0");
		gui_rt->set_entry(L"frame",L"0");
		gui_rt->set_entry(L"logic",L"0");
		gui_rt->set_entry(L"render",L"0");
		gui_rt->set_entry(L"skip",L"0");

		m_camera=wycnew xsimple_camera();
		m_camera->set_projection(45,float(client_width())/client_height(),1,1000);
		m_camera->set_min_max_distance(2.0f,4.0f);
		m_camera->set_position(DEG_TO_RAD(90),DEG_TO_RAD(-90),2.5f);
		// mesh
		_do_load_mesh();
		// lights
		_create_lights(3);
		xsimple_light *light;
		light = m_lights[0];
		light->set_rotation(xvec3f_t(1,0,0),1.2f,DEG_TO_RAD(10));
		light->set_intensity(1.0f);
		light = m_lights[1];
		light->set_rotation(xvec3f_t(0,1,0),1.8f,DEG_TO_RAD(10));
		light->set_intensity(0.8f);
		light = m_lights[2];
		light->set_rotation(xvec3f_t(0,0,1),2.4f,DEG_TO_RAD(10));
		light->set_intensity(0.6f);
		
		glClearColor(0,0,0,1.0f);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glEnable(GL_POINT_SPRITE);
		glEnable(GL_PROGRAM_POINT_SIZE);
		glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN,GL_LOWER_LEFT);
		glEnable(GL_LINE_SMOOTH);
		glLineWidth(1.2f);

		return true;
	}
	virtual void on_game_exit() {
		wyc_print("quit game");
		m_font_sys=0;
		m_layout=0;
		m_camera=0;
		m_base_map=0;
		m_normal_map=0;
		m_height_map=0;
		m_light_icon=0;
		for(int i=0; i<SHAPE_COUNT; ++i)
			m_meshes[i]=0;
		for(int i=0, count=m_techs.size(); i<count; ++i)
			m_techs[i]->decref();
		m_techs.clear();
		m_tech_billboard=0;
		m_tech_wireframe=0;
		m_tech_normals=0;
		for(int i=0, count=m_lights.size(); i<count; ++i)
			m_lights[i]->decref();
		m_lights.clear();
	}
	virtual void on_input(const wyc::xinput_buffer &ibuff)
	{
		// handle mouse input
		bool repos=false;
		if(ibuff.offz) {
		//	wyc_print("wheel:%d",ibuff.offz);
			m_camera->zoom(ibuff.offz*0.002f);
			repos=true;
		}
		if(m_move_cam) {
		//	wyc_print("cursor:(%d,%d) (%d,%d)",ibuff.offx,ibuff.offy,ibuff.x,ibuff.y);
			if(ibuff.offx!=0 || ibuff.offy!=0) 
				m_camera->move(ibuff.offx*0.003f,ibuff.offy*0.003f);
		}

		for(xmouseque::const_iterator iter=ibuff.mouseque.begin(), 
			end=ibuff.mouseque.end(); iter!=end; ++iter) {
			if(iter->msg==wyc::EV_LB_DOWN) {
				m_move_cam=true;
			}
			else if(iter->msg==wyc::EV_LB_UP) {
				m_move_cam=false;
			}
		}
		// handle key input
		for(xkeyque::const_iterator iter = ibuff.keyque.begin(),
			end = ibuff.keyque.end(); iter != end; ++iter)
		{
			if(iter->msg==wyc::EV_KEY_UP)
			{
				if(iter->key == 'W')
					m_wireframe = !m_wireframe;
				else if(iter->key == 'N')
					m_normals = !m_normals;
				else if(iter->key == VK_SPACE)
				{
					m_shape = (m_shape+1)%SHAPE_COUNT;
					_do_load_mesh();
				}
				else if(iter->key>='1' && iter->key<='9')
				{
					m_tech_idx = (iter->key-'1')%TECH_COUNT;
				}
			}
		}
	}
	virtual void update_metrics(const wyc::xgame_metric &metrics) 
	{
		if(!m_layout) return;
		wyc::xgui_rtinfo *gui_rt=wyc_safecast(wyc::xgui_rtinfo,m_layout->get_element("rt"));
		if(!gui_rt) return;
		std::wstring str;
		gui_rt->set_entry(L"shader",s_tech_name[m_tech_idx]);
		wyc::format(str,L"%.2f",metrics.m_fps);
		gui_rt->set_entry(L"FPS",str.c_str());
		wyc::format(str,L"%f",metrics.m_cosumed_time_last/metrics.m_frame_count_last);
		gui_rt->set_entry(L"frame",str.c_str());
		wyc::format(str,L"%f",metrics.m_logic_time_last/metrics.m_frame_count_last);
		gui_rt->set_entry(L"logic",str.c_str());
		wyc::format(str,L"%f",metrics.m_render_time_last/metrics.m_frame_count_last);
		gui_rt->set_entry(L"render",str.c_str());
		wyc::uint2str(str,metrics.m_tick_skipped_last);
		gui_rt->set_entry(L"skip",str.c_str());
	}
	virtual void on_logic(double accum_time, double frame_time) 
	{
		m_camera->update();
		float t = (float)frame_time;
		if(m_meshes[m_shape])
			m_meshes[m_shape]->update_transform(t);
		for(size_t i=0, count=m_lights.size(); i<count; ++i)
			m_lights[i]->update_transform(t);
		m_layout->update();
	}
	virtual void on_render(double, double)
	{
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		render_world();
		assert(glGetError()==GL_NO_ERROR);
		render_gui();
		assert(glGetError()==GL_NO_ERROR);
	}
	void render_world()
	{
		if(m_tech_idx>=m_techs.size())
			return;
		xrender_tech *tech = m_techs[m_tech_idx];
		if(!tech)
			return;
		xsimple_mesh *mesh = m_meshes[m_shape];
		if(!mesh)
			return;
		tech->begin(m_renderer,m_camera);
		tech->draw(mesh);
		tech->end();

		// draw wire frame
		if(m_wireframe || m_normals) {
			glDepthMask(GL_FALSE);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

			if(m_wireframe) {
				m_tech_wireframe->begin(m_renderer,m_camera);
				m_tech_wireframe->draw(mesh);
				m_tech_wireframe->end();
			}
			if(m_normals) {
				m_tech_normals->begin(m_renderer,m_camera);
				m_tech_normals->draw(mesh);
				m_tech_normals->end();
			}
			glDepthMask(GL_TRUE);
			glDisable(GL_BLEND);
		}
		
		// draw lights
		m_tech_billboard->begin(m_renderer,m_camera);
		for(size_t i=0, count= m_lights.size(); i<count; ++i)
			m_tech_billboard->draw(m_lights[i]);
		m_tech_billboard->end();
	}

	void render_gui()
	{
		const wyc::xshader *shader;
		GLint uf;
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		// real time info
		if(m_renderer->use_shader(SHADER_TEXT)) {
			shader=m_renderer->get_shader();
			uf=shader->get_uniform("gui_proj");
			assert(-1!=uf);
			glUniformMatrix4fv(uf,1,GL_TRUE,m_proj_gui.data());
			uf=shader->get_uniform("gui_texture");
			assert(-1!=uf);
			glUniform1i(uf,0);
			m_layout->render(m_renderer);
		}
	}

private:
	void _do_load_mesh()
	{
		std::string path="mesh/";
		path+=s_model_files[m_shape];
		path+=".json";
		get_resource_path(path);
		xsimple_mesh *mesh=m_meshes[m_shape];
		if(!mesh) {
			mesh=wycnew xsimple_mesh();
			m_meshes[m_shape]=mesh;
			if(!mesh->load(path.c_str()))
			{
				wyc_warn("fail to load mesh: %s",path.c_str());
			}
		}
		mesh->set_base_map(m_base_map);
		mesh->set_normal_map(m_normal_map);
		mesh->set_height_map(m_height_map);
	}

	void _create_lights(int count)
	{
		uint32_t evid = strhash("add_light");
		xsimple_light *light;
		for(int i=0; i<count; ++i) {
			light = wycnew xsimple_light();
			light->incref();
			light->set_base_map(m_light_icon);
			m_lights.push_back(light);
			for(int j=0; j<TECH_COUNT; ++j)
				m_techs[j]->add_light(light);
		}
	}
};

xgame* xgame::create_game() 
{
	return new xgame_helloworld;
}




