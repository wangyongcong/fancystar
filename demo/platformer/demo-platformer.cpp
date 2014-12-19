#include "fscorepch.h"

#include "wyc/game/game.h"
#include "wyc/util/strutil.h"
#include "wyc/math/transform.h"
#include "wyc/gui/gui_layout.h"
#include "wyc/gui/gui_rtinfo.h"

#include "shaderlib.h"
#include "scene.h"
#include "entity.h"

using namespace wyc;
using namespace demo;

class xgame_platformer : public xgame
{
	xpointer<xfont> m_font_sys;
	xpointer<xlayout> m_gui_layout;
	xmat4f_t m_gui_proj;
	xpointer<xscene> m_scene;
	xpointer<xplayer> m_player;
	xpointer<xobstacle> m_new_obstacle;
	xpointer<xentity> m_picked;
	xvec2f_t m_camera_pos, m_mouse_pos, m_camera_dest;
	float m_camera_spd;
	xvec2f_t m_start_pos;
	bool m_begin_drag, m_ctrl_down, m_shift_down;
public:
	xgame_platformer() 
	{
		m_begin_drag=false;
		m_ctrl_down=false;
		m_shift_down=false;

		set_code_name("fs-platformer");
		set_app_name(L"Demo Platformer");
	}
	virtual bool on_game_init() 
	{
		xmove::init_move_env(CG_TERRAIN | CG_OBSTACLE, CG_PLATFORM);

		std::string path;
		xressvr *svr = get_resource_server();

		path="font/vera10.font";
		get_resource_path(path);
		m_font_sys = (xfont*)svr->request(xfont::get_class()->id,path.c_str());
		assert(m_font_sys->is_complete());
	
		m_scene = wycnew xscene();
		path="map/map_900.map";
		get_resource_path(path);
		m_scene->load_map(path.c_str(),1);

		m_player = wycnew xplayer();
		m_player->on_create(m_scene->get_next_pid(), AGENT_AABB, CG_PLAYER);
		m_player->set_position(32,64);
		m_player->update_aabb();
		m_scene->add_object(m_player);

		m_camera_pos.zero();
		m_mouse_pos.zero();
		m_camera_dest.zero();
		m_camera_spd=300;

		// GUI initialization
		wyc::set_ui_projection(m_gui_proj,float(client_width()),float(client_height()),1000);
		m_gui_layout=wycnew xlayout;
		wyc::xguiobj *gui_obj;
		gui_obj=m_gui_layout->new_element("xgui_rtinfo","rt",0,0,0);
		wyc::xgui_rtinfo *gui_rt=wyc_safecast(wyc::xgui_rtinfo,gui_obj);
		gui_rt->set_technique(SHADER_TEXT);
		gui_rt->set_font(m_font_sys);
		gui_rt->set_color(0,255,0,255);
		gui_rt->set_column(1);
		gui_rt->set_entry(L"FPS",L"0");

		// OpenGL initialization
		glClearColor(0,0,0,1.0f);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_LINE_SMOOTH);
		glLineWidth(1.2f);

		return true;
	}

	virtual void on_game_exit() {
		m_font_sys=0;
		m_gui_layout=0;
		m_scene=0;
		m_player=0;
		m_new_obstacle=0;
		m_picked=0;
	}

	virtual void on_input(const wyc::xinput_buffer &input)
	{
		// handle mouse input
		if(input.offx!=0 || input.offy!=0)	
			on_mouse_move(input.x, input.y, input.offx, input.offy);

		for(wyc::xmouseque::const_iterator iter=input.mouseque.begin(), 
			end=input.mouseque.end(); iter!=end; ++iter) 
		{
			switch(iter->msg)
			{
			case EV_LB_DOWN:
				on_mouse_left_down(input.x, input.y);
				break;
			case EV_LB_UP:
				on_mouse_left_up(input.x, input.y);
				break;
			case EV_RB_DOWN:
				break;
			case EV_RB_UP:
				break;
			}
		} // mouse message loop

		// handle key input
		for(wyc::xkeyque::const_iterator iter = input.keyque.begin(),
			end = input.keyque.end(); iter != end; ++iter)
		{
			if(iter->msg==EV_KEY_DOWN)
				on_key_down(iter->key);
			else if(iter->msg==EV_KEY_UP)
				on_key_up(iter->key);
		} // key message loop
	}

	void on_mouse_move(int x, int y, int offx, int offy)
	{
		m_mouse_pos.set(m_camera_pos.x+x,m_camera_pos.y+client_height()-y);
		if(m_new_obstacle) {
			xvec2f_t center(m_camera_pos.x+0.5f*(m_start_pos.x+x),m_camera_pos.y+client_height()-0.5f*(m_start_pos.y+y));
			xvec2f_t radius(0.5f*abs(m_start_pos.x-x), 0.5f*abs(m_start_pos.y-y));
			m_new_obstacle->set_aabb(center-radius,center+radius);
			m_new_obstacle->update_aabb();
		}
		else if(m_picked && m_begin_drag && !m_shift_down) {
			m_picked->set_position(m_mouse_pos+m_start_pos);
			m_picked->update_aabb();
		}
	}

	void on_mouse_left_down(int x, int y)
	{
		m_begin_drag=true;
		if(m_ctrl_down)
		{
			m_start_pos.set( float(x), float(y) );
			m_new_obstacle = wycnew xobstacle();
			m_new_obstacle->on_create(m_scene->get_next_pid(), wyc::AGENT_AABB, CG_TERRAIN);
			m_new_obstacle->set_position(m_camera_pos.x+x,m_camera_pos.y+client_height()-y);
		}
		else {
			xentity *en = m_scene->pick(m_camera_pos.x+x,m_camera_pos.y+client_height()-y,0xFFFF);
			select_object(en);
			if(m_picked) 
				m_start_pos=en->get_position()-m_mouse_pos;
		}
	}

	void on_mouse_left_up (int x, int y)
	{
		if(m_new_obstacle)
		{
			const wyc::xvec2f_t &r = m_new_obstacle->get_radius();
			if(r.x>4 && r.y>4) {
				m_scene->add_object(m_new_obstacle);
				m_new_obstacle->update_aabb();
				select_object(m_new_obstacle);
			}
			m_new_obstacle=0;
		}
		else if(m_picked == m_player)
		{
			m_player->stand();
			update_camera_pos();
		}
		m_begin_drag=false;
	}

	void on_key_down(unsigned key)
	{
		switch(key)
		{
		case VK_UP:
			m_player->move_y( 1);
			break;
		case VK_DOWN:
			m_player->move_y(-1);
			break;
		case VK_LEFT:
			m_player->move_x(-1);
			break;
		case VK_RIGHT:
			m_player->move_x( 1);
			break;
		case VK_SPACE:
			switch_obstacle_type();
			break;
		case VK_CONTROL:
			m_ctrl_down=true;
			break;
		case VK_SHIFT:
			m_shift_down=true;
			break;
		}	
	}

	void on_key_up (unsigned key)
	{
		switch(key)
		{
		case VK_OEM_PLUS:
			m_scene->lod_increase();
			break;
		case VK_OEM_MINUS:
			m_scene->lod_decrease();
			break;
		case VK_UP:
			m_player->stop_y( 1);
			break;
		case VK_DOWN:
			m_player->stop_y(-1);
			break;
		case VK_LEFT:
			m_player->stop_x(-1);
			break;
		case VK_RIGHT:
			m_player->stop_x( 1);
			break;
		case VK_CONTROL:
			m_ctrl_down=false;
			break;
		case VK_SHIFT:
			m_shift_down=false;
			break;
		case VK_F2:
			m_scene->switch_tree_show();
			break;
		case VK_F3:
			m_scene->switch_filled_show();
			break;
		case 'F':
			m_player->switch_fly_mode();
			break;
		}
	}

	virtual void update_metrics(const wyc::xgame_metric &metrics) 
	{
		assert(m_gui_layout);
		wyc::xgui_rtinfo *rt=wyc_safecast(wyc::xgui_rtinfo,m_gui_layout->get_element("rt"));
		assert(rt);
		std::wstring str;
		wyc::format(str,L"%.2f",metrics.m_fps);
		rt->set_entry(L"FPS",str.c_str());
	}

	virtual void on_logic(double accum_time, double frame_time) 
	{
		if(!m_begin_drag) {
			m_player->update(frame_time);
			update_camera_pos();
		}
		// move camera
		xvec2f_t pos = m_camera_dest - m_camera_pos;
		if(std::fabs(pos.x)<0.1f)
			m_camera_pos.x = m_camera_dest.x;
		else
			m_camera_pos.x += pos.x * 0.16f;
		if(std::fabs(pos.y)<0.1f)
			m_camera_pos.y = m_camera_dest.y;
		else
			m_camera_pos.y += pos.y * 0.16f;
		m_gui_layout->update();
	}

	void update_camera_pos ()
	{
		// camera follow player
		wyc::xvec2f_t pos = m_player->get_position();
		const wyc::xvec2f_t &r = m_player->get_radius();
		pos.x-=client_width()*0.5f-r.x;
		pos.y-=client_height()*0.5f-r.y;
		float max_x, max_y;
		max_x = float(m_scene->width()-client_width());
		max_y = float(m_scene->height()-client_height());
		if(pos.x<0)
			pos.x=0;
		else if(pos.x>max_x)
			pos.x=max_x;
		if(pos.y<0)
			pos.y=0;
		else if(pos.y>max_y)
			pos.y=max_y;
		m_camera_dest = pos;
	}

	virtual void on_render(double, double delta)
	{
		glClear(GL_COLOR_BUFFER_BIT);
		render_world();
		render_gui();
	}

	void render_world()
	{
		m_renderer->use_shader(0);
		glViewport(0,0,client_width(),client_height());
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho( m_camera_pos.x, m_camera_pos.x+client_width(), m_camera_pos.y, m_camera_pos.y+client_height(), -1.0, 1.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glColor3f(1.0f,1.0f,1.0f);
		m_scene->draw(m_renderer);
		m_player->draw(m_renderer);
		if(m_new_obstacle)
			m_new_obstacle->draw(m_renderer);
	}

	void render_gui()
	{
		const wyc::xshader *shader;
		GLint uf;
		// real time info
		if(m_renderer->use_shader(SHADER_TEXT)) {
			shader=m_renderer->get_shader();
			uf=shader->get_uniform("gui_proj");
			assert(-1!=uf);
			glUniformMatrix4fv(uf,1,GL_TRUE,m_gui_proj.data());
			uf=shader->get_uniform("gui_texture");
			assert(-1!=uf);
			glUniform1i(uf,0);
			m_gui_layout->render(m_renderer);
		}
	}

	void select_object(xentity *en)
	{
		if(en!=m_picked)
		{
			if(m_picked) 
				m_picked->set_selected(false);
			if(en) 
				en->set_selected(true);
			m_picked=en;
		}
	}

	void switch_obstacle_type()
	{
		if(m_picked && !m_new_obstacle && m_picked->get_agent()->get_mask()==CG_TERRAIN)
		{
			int t = (m_picked->type()+1)%wyc::AGENT_TYPE_COUNT;
			if(t==m_picked->type()) 
				return;
			m_picked->set_type((AGENT_TYPE)t);
			m_picked->update_aabb();
		}
	}
};

xgame* xgame::create_game() 
{
	return new xgame_platformer;
}




