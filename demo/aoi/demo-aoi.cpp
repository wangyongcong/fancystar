#include "fscorepch.h"

#include "wyc/game/game.h"
#include "wyc/util/strutil.h"
#include "wyc/gui/gui_layout.h"
#include "wyc/gui/gui_rtinfo.h"

#include "shaderlib.h"
#include "obj_camera.h"
#include "obj_floor.h"
#include "obj_player.h"
#include "obj_watcher.h"

using namespace wyc;

enum AOI_GROUP
{
	GROUP_PLAYER = 1,
	GROUP_WATCHER = 2,
};

class xgame_aoi : public xgame
{
	xpointer<xfont> m_font_sys;
	xpointer<xlayout> m_gui;
	xmat4f_t m_gui_proj;

	// camera control
	xpointer<x3rd_camera> m_camera;
	xvec3f_t m_move_fwd;
	xvec3f_t m_move_dir;

	float m_cam_speed;
	float m_move_speed;
	float m_follow_distance;
	
	xpointer<xfloor> m_floor;
	unsigned m_floor_size;
	float m_grid_size;
	xpointer<xplayer> m_player;
	typedef std::vector<xpointer<xplayer> > player_group_t;
	player_group_t m_players;
	unsigned m_current_player;
	typedef std::vector<xpointer<xwatcher> > watcher_group_t;
	watcher_group_t m_watchers;

	// lighting
	xvec3f_t m_ambient;
	xvec3f_t m_light_intensity;
	xvec3f_t m_light_direction;

	// aoi
	xaoi_manager *m_aoi_mgr;

	bool m_translate_mode, m_rotate_mode;
public:
	xgame_aoi() 
	{
		set_app_name(L"AOI Demo");

		m_move_dir.zero();
		m_move_speed = 2;
		m_cam_speed = 4;
		m_follow_distance = 10;
		m_floor_size = 16;
		m_grid_size = 2;
		m_current_player=-1;
		m_aoi_mgr=aoi_manager_create(3);

		m_translate_mode=false;
		m_rotate_mode=false;
	}
	virtual bool on_game_init() 
	{
		std::string path;
		xressvr *svr = get_resource_server();

		path="font/vera10.font";
		get_resource_path(path);
		m_font_sys = (xfont*)svr->request(xfont::get_class()->id,path.c_str());
		assert(m_font_sys->is_complete());

		m_floor = wycnew xfloor();
		m_floor->create(m_floor_size,m_grid_size);
		m_floor->update_transform();

		distribute_players(100);
		distribute_watchers(100);

		m_camera = wycnew x3rd_camera();
		m_camera->init_camera(45,client_width(),client_height());
		m_camera->look_at(xvec3f_t(0,14,6),xvec3f_t(0,0,0));
		m_move_fwd.cross(m_camera->get_transform()->right(),xvec3f_t(0,1,0));
		m_move_fwd.normalize();

		// lightings
		m_ambient.set(0.2f,0.2f,0.2f);
		m_light_intensity.set(1.0f,1.0f,1.0f);
		m_light_direction.set(1.0f,1.0f,1.0f);
		m_light_direction.normalize();

		// gui
		wyc::set_ui_projection(m_gui_proj,float(client_width()),float(client_height()),1000);
		m_gui=wycnew xlayout;
		wyc::xguiobj *gui_obj;
		
		gui_obj=m_gui->new_element("xgui_rtinfo","rt",0,0,0);
		wyc::xgui_rtinfo *gui_rt=wyc_safecast(wyc::xgui_rtinfo,gui_obj);
		gui_rt->set_technique(SHADER_TEXT);
		gui_rt->set_font(m_font_sys);
		gui_rt->set_color(0,255,0,255);
		gui_rt->set_column(1);
		gui_rt->set_entry(L"FPS",L"0");
		std::wstring str;
		uint2str(str,m_watchers.size());
		gui_rt->set_entry(L"watcher",str.c_str());
		uint2str(str,m_players.size());
		gui_rt->set_entry(L"player",str.c_str());
		gui_rt->set_entry(L"aoi_t",L"0");

		glClearColor(0,0,0,1.0f);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

		return true;
	}

	virtual void on_game_exit() {
		m_font_sys=0;
		m_gui=0;
		m_camera=0;
		m_floor=0;
		m_players.clear();
		m_watchers.clear();
		aoi_manager_destroy(m_aoi_mgr);
		m_aoi_mgr=0;
	}

	virtual void on_input(const wyc::xinput_buffer &input)
	{
		// handle mouse input
		for(wyc::xmouseque::const_iterator iter=input.mouseque.begin(), 
			end=input.mouseque.end(); iter!=end; ++iter) {
			switch(iter->msg)
			{
			case EV_LB_DOWN:
				m_translate_mode=true;
				m_rotate_mode=false;
				break;
			case EV_LB_UP:
				m_translate_mode=false;
				break;
			case EV_RB_DOWN:
				m_rotate_mode=true;
				m_translate_mode=false;
				break;
			case EV_RB_UP:
				m_rotate_mode=false;
				break;
			}
		}
		xtransform *trans;
		if(m_current_player<m_players.size())
		{
			trans = m_players[m_current_player]->get_transform();
			control_player(trans,input.offx,input.offy,input.offz);
		}
		else 
		{
			trans=m_camera->get_transform();
			control_camera(trans,input.offx,input.offy,input.offz);
		}
		// handle key input
		for(wyc::xkeyque::const_iterator iter = input.keyque.begin(),
			end = input.keyque.end(); iter != end; ++iter)
		{
			if(EV_KEY_DOWN==iter->msg)
			{
				switch(iter->key)
				{
				case 'W':
					m_move_dir.z=-1.0f;
					break;
				case 'S':
					m_move_dir.z=1.0f;
					break;
				case 'A':
					m_move_dir.x=-1.0f;
					break;
				case 'D':
					m_move_dir.x=1.0f;
					break;
				case 'T':
					toggle_control_mode();
					break;
				}
			}
			if(EV_KEY_UP==iter->msg)
			{
				switch(iter->key)
				{
				case 'W':
					m_move_dir.z=0;
					break;
				case 'S':
					m_move_dir.z=0;
					break;
				case 'A':
					m_move_dir.x=0;
					break;
				case 'D':
					m_move_dir.x=0;
					break;
				}
			}

		}
	}

	void control_camera(xtransform *trans, int offx, int offy, int offz)
	{
		if(m_translate_mode)
		{
			if( offx!=0 || offy!=0 ) {
				xvec3f_t pos = trans->position();
				pos+=trans->right()*(offx*0.01f);
				pos+=m_move_fwd*(offy*0.01f);
				trans->set_position(pos);
			}
		}
		else if(m_rotate_mode)
		{
			if(offx!=0)
			{
				trans->rotate(xvec3f_t(0,1,0),offx*0.1f);
				m_move_fwd.cross(trans->right(),xvec3f_t(0,1,0));
				m_move_fwd.normalize();
			}
			if(offy!=0)
			{
				trans->rotate(trans->right(),offy*0.02f);
			}
		}
		if(offz!=0)
		{
			trans->translate_forward(-0.002f*offz);
		}
	}

	void control_player(xtransform *trans, int offx, int offy, int offz)
	{
		if(m_rotate_mode)
		{
			if(offx!=0)
			{
				trans->rotate_up(offx*-0.1f);
				m_move_fwd = trans->forward();
			}
		}
		if(offz!=0)
		{
			m_follow_distance += -0.002f*offz;
		}
	}

	virtual void update_metrics(const wyc::xgame_metric &metrics) 
	{
		assert(m_gui);
		wyc::xgui_rtinfo *rt=wyc_safecast(wyc::xgui_rtinfo,m_gui->get_element("rt"));
		assert(rt);
		std::wstring str;
		wyc::format(str,L"%.2f",metrics.m_fps);
		rt->set_entry(L"FPS",str.c_str());
		wyc::format(str,L"%f",g_aoi_stat.ms_per_frame());
		rt->set_entry(L"aoi_t",str.c_str());
		g_aoi_stat.reset();
	}

	virtual void on_logic(double accum_time, double frame_time) 
	{
		xtransform *trans;
		float speed;
		if(m_current_player<m_players.size())
		{
			// move player
			trans = m_players[m_current_player]->get_transform();
			speed = m_move_speed;
		}
		else {
			// move camera
			trans = m_camera->get_transform();
			speed = m_cam_speed;
		}
		if(m_move_dir != xvec3f_t(0,0,0))
		{
			xvec3f_t step = m_move_dir.x * trans->right() + m_move_dir.z * m_move_fwd;
			step.normalize();
			step = step * (speed * float(frame_time));
		//	printf("move[%d]: (%f,%f,%f), %f\n",m_metrics.m_frame_count,m_move_dir.x,m_move_dir.y,m_move_dir.z,step.length());
			step += trans->position();
			trans->set_position(step);
		}

		if(m_current_player<m_players.size())
		{
			m_players[m_current_player]->update_transform();
			camera_follow(m_players[m_current_player]);
		}
		else 
		{
			random_move_players(float(frame_time));
		}
		m_camera->update_transform();

		g_aoi_stat.update();
	}

	virtual void on_render(double, double delta)
	{
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		const xmat4f_t &camera_w2l = m_camera->get_transform()->world2local();
		const xmat4f_t &camera_proj = m_camera->get_projection();
		xmat4f_t camera_matrix;
		camera_matrix = camera_proj * camera_w2l;

		const xshader *sh;
		GLint uf;

		// render floor
		if(!m_renderer->use_shader(SHADER_FLOOR))
			return;
		sh = m_renderer->get_shader();
		uf=sh->get_uniform("mat_camera");
		if(-1==uf)
			return;
		glUniformMatrix4fv(uf,1,GL_TRUE,camera_matrix.data());
		glDisable(GL_DEPTH_TEST);
		m_floor->draw(sh);
		
		// render objects
		if(!m_renderer->use_shader(SHADER_PLAYER))
			return;
		sh = m_renderer->get_shader();
		uf = sh->get_uniform("mat_camera");
		if(-1==uf)
			return;
		glUniformMatrix4fv(uf,1,GL_TRUE,camera_matrix.data());
		uf = sh->get_uniform("eye_position");
		if(-1==uf)
			return;
		const xvec3f_t &cam_pos = m_camera->get_transform()->position();
		glUniform3f(uf,cam_pos.x,cam_pos.y,cam_pos.z);
		uf = sh->get_uniform("light_ambient");
		if(-1==uf)
			return;
		glUniform3f(uf,m_ambient.x,m_ambient.y,m_ambient.z);
		uf = sh->get_uniform("light_intensity");
		if(-1==uf)
			return;
		glUniform3f(uf,m_light_intensity.x,m_light_intensity.y,m_light_intensity.z);
		uf = sh->get_uniform("light_direction");
		if(-1==uf)
			return;
		glUniform3f(uf,m_light_direction.x,m_light_direction.y,m_light_direction.z);

		uf = sh->get_uniform("mtl_specular");
		if(-1==uf)
			return;
		glUniform3f(uf,1.0f,1.0f,1.0f);
		uf = sh->get_uniform("smoothness");
		if(-1==uf)
			return;
		glUniform1f(uf,2.0f);

		glEnable(GL_DEPTH_TEST);
		// render player
		for(player_group_t::iterator iter=m_players.begin(), end=m_players.end();
			iter!=end; ++iter)
		{
			(*iter)->draw(sh);
		}
		// render watchers
		for(watcher_group_t::iterator iter=m_watchers.begin(), end=m_watchers.end();
			iter!=end; ++iter) 
		{
			(*iter)->draw(sh);
		}

		glDisable(GL_DEPTH_TEST);
		// render scopes
		if(!m_renderer->use_shader(SHADER_CURVE))
			return;
		sh = m_renderer->get_shader();
		uf = sh->get_uniform("mat_camera");
		if(-1==uf)
			return;
		glUniformMatrix4fv(uf,1,GL_TRUE,camera_matrix.data());
		for(player_group_t::iterator iter=m_players.begin(), end=m_players.end();
			iter!=end; ++iter)
		{
			(*iter)->draw_scope(sh);
		}
		for(watcher_group_t::iterator iter=m_watchers.begin(), end=m_watchers.end();
			iter!=end; ++iter) 
		{
			(*iter)->draw_scope(sh);
		}

		render_gui();
	}

	void render_gui()
	{
		if(!m_renderer->use_shader(SHADER_TEXT))
			return;
		const wyc::xshader *sh=m_renderer->get_shader();
		GLint uf=sh->get_uniform("gui_proj");
		if(-1==uf)
			return;
		glUniformMatrix4fv(uf,1,GL_TRUE,m_gui_proj.data());
		uf=sh->get_uniform("gui_texture");
		if(-1==uf)
			return;
		glUniform1i(uf,0);
		glEnable(GL_BLEND);
		m_gui->render(m_renderer);
		glDisable(GL_BLEND);
	}

	void distribute_players (unsigned count)
	{
		float size = m_floor_size * m_grid_size, half_size, x=0, z=0;
		half_size = size * 0.5f;
		xplayer *pl;
		for(unsigned i=0; i<count; ++i)
		{
			pl = wycnew xplayer();
			pl->get_transform()->set_position(x,0,z);
			pl->join_aoi(m_aoi_mgr,0.5,GROUP_PLAYER,GROUP_PLAYER);
			pl->update_transform();
			m_players.push_back(pl);
			x=wyc::random()*size-half_size;
			z=wyc::random()*size-half_size;
		}
	}

	void distribute_watchers (unsigned count)
	{

		float size = m_floor_size * m_grid_size, half_size, x, z;
		half_size = size * 0.5f;

		xwatcher *wtc;
		for(unsigned i=0; i<count; ++i)
		{
			wtc = wycnew xwatcher();
			x=wyc::random()*size-half_size;
			z=wyc::random()*size-half_size;
			wtc->get_transform()->set_position(x,0,z);
			wtc->join_aoi(m_aoi_mgr,1,GROUP_WATCHER,GROUP_PLAYER);
			wtc->update_transform();
			m_watchers.push_back(wtc);
		}
	}

	void toggle_control_mode()
	{
		if(m_current_player>=m_players.size())
		{
			if(!m_players.size())
				return;
			m_current_player = 0;
			m_move_fwd = m_players[m_current_player]->get_transform()->forward();
		//	camera_follow(m_players[m_current_player]);
		}
		else 
		{
			m_current_player=-1;
			m_move_fwd.cross(m_camera->get_transform()->right(),xvec3f_t(0,1,0));
			m_move_fwd.normalize();
		}
	}

	void camera_follow ( xplayer *pl )
	{
		xtransform *trans = pl->get_transform();
		float dist = m_follow_distance * 0.707f;
		xvec3f_t cam_pos=trans->position()+dist*trans->forward();
		cam_pos.y += dist;
		m_camera->look_at(cam_pos,trans->position());
	}

	void random_move_players(float frame_time)
	{
		xplayer *pl;
		xtransform *trans;
		float boundry = m_floor_size * m_grid_size * 0.5f;
		xvec3f_t fwd;
		for(player_group_t::iterator iter = m_players.begin(), end = m_players.end();
			iter!=end;  ++iter) {
			pl = *iter;
			trans = pl->get_transform();
			trans->translate_forward(m_move_speed*frame_time);
			const xvec3f_t &pos = trans->position();
			if(pos.x<-boundry || pos.x>boundry || pos.z<-boundry || pos.z>boundry)
			{
				fwd = trans->forward();
				if(pos.x<-boundry || pos.x>boundry)
					fwd.x = -fwd.x;
				if(pos.z<-boundry || pos.z>boundry)
					fwd.z = -fwd.z;
				trans->set_forward(fwd,xvec3f_t(0,1,0));
			}
			pl->update_transform();
		}
	}

};

xgame* xgame::create_game() 
{
	return new xgame_aoi;
}




