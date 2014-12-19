#include "fscorepch.h"

#include "Gwen/Gwen.h"
#include "Gwen/Skins/Simple.h"
#include "Gwen/Skins/TexturedBase.h"
#include "Gwen/Input/Windows.h"
#include "Gwen/Controls/WindowControl.h"

#include "wyc/game/game.h"
#include "wyc/game/glb_game.h"
#include "wyc/gui/gui_layout.h"
#include "wyc/gui/gui_rtinfo.h"
#include "wyc/util/strutil.h"
#include "wyc/util/md5.h"
#include "wyc/math/transform.h"

#include "shaderlib.h"
#include "card_material.h"
#include "gwen_renderer.h"
#include "gui_editor.h"
#include "network.h"

#ifdef _DEBUG
	#pragma comment(lib,"photon-cpp_d.lib")
	#pragma comment(lib,"photon-common-cpp_d.lib")
#else
	#pragma comment(lib,"photon-cpp.lib")
	#pragma comment(lib,"photon-common-cpp.lib")
#endif 

#pragma warning(disable: 4244)

using namespace wyc; 

class xgame_card : public xgame
{
	std::string m_server_ip;
	// generic resource
	xpointer<xfont> m_font_sys;
	xpointer<xfont> m_font_chn;
	// camera control
	xmat4f_t m_proj_gui;
	// ui components
	xpointer<xlayout> m_layout;
	xpointer<xcard_renderer> m_card_rc;
	// FXAA data
	GLuint m_frame_buffer;
	GLuint m_depth_buffer;
	GLuint m_screen_texture;
	GLuint m_screen_quad;
	unsigned m_screen_w, m_screen_h;

	struct USER_INFO
	{
		std::string account;
		std::string pwd;
		bool is_verified;
		bool is_linking;
		USER_INFO() {
			is_verified=false;
			is_linking=false;
		}
	} m_user;
	xpointer<xcard_editor> m_editor;
	xcard_network *m_network;
	bool m_wireframe, m_fxaa, m_edit_mode;
	bool m_debug_atlas, m_debug_flush;
public:
	xgame_card() 
	{
	//	m_server_ip = "ec2-175-41-221-91.ap-northeast-1.compute.amazonaws.com:5055";
		m_server_ip = "localhost:5055";

		m_wireframe=false;
		m_fxaa=true;
		m_edit_mode=true;

		m_frame_buffer=0;
		m_depth_buffer=0;
		m_screen_texture=0;
		m_screen_quad=0;
		set_code_name("fs-card");
		set_app_name(L"Card Designer v1.0");
		set_resource_path("data");
		
		m_debug_atlas=false;
		m_debug_flush=false;

	}

	virtual bool on_game_init() 
	{
		m_network=new xcard_network();

		std::string path;
		xressvr *svr = get_resource_server();

		// initialize resource
		path="font/wqy10.font";
		get_resource_path(path);
		m_font_chn = (xfont*)svr->request(xfont::get_class()->id,path.c_str());
		assert(m_font_chn->is_complete());
		path="font/vera10.font";
		get_resource_path(path);
		m_font_sys = (xfont*)svr->request(xfont::get_class()->id,path.c_str());
		assert(m_font_sys->is_complete());

		// initialize card renderer
		m_card_rc=wycnew xcard_renderer;
		path="card_layout.json";
		get_resource_path(path);
		m_card_rc->load_card_layout(path.c_str());
		
		// initialize GUI
		set_ui_projection(m_proj_gui,float(client_width()),float(client_height()),1000);
		m_layout = wycnew xlayout;
		wyc::xguiobj *gui_obj=m_layout->new_element("xgui_rtinfo","rt",4,32,0);
		wyc::xgui_rtinfo *gui_rt=wyc_safecast(wyc::xgui_rtinfo,gui_obj);
		gui_rt->set_font(m_font_sys);
		gui_rt->set_color(0,255,0,255);
		gui_rt->set_column(1);
		gui_rt->set_entry(L"FPS",L"0");
		gui_rt->set_entry(L"frame",L"0");
		gui_rt->set_entry(L"logic",L"0");
		gui_rt->set_entry(L"render",L"0");
		gui_rt->set_entry(L"skip",L"0");
		
		// initialize editor
		m_editor = wycnew xcard_editor();
		m_editor->initialize(client_width(),client_height());

		// initialize OpenGL
		glClearColor(0,0,0,1);
		glCullFace(GL_BACK);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		create_off_screen_buffer(client_width(),client_height());
	
		m_card_rc->add_card(m_editor->get_preview());

		return true;
	}

	virtual void on_game_exit() {
		wyc_print("quit game");

		if(m_network) {
			m_network->disconnect();
			delete m_network;
			m_network=0;
		}

		m_layout=0;
		m_card_rc=0;
		if(m_frame_buffer)
		{
			glDeleteRenderbuffers(1,&m_depth_buffer);
			glDeleteFramebuffers(1,&m_frame_buffer);
			glDeleteTextures(1,&m_screen_texture);
			glDeleteBuffers(1,&m_screen_quad);
			m_frame_buffer=0;
			m_depth_buffer=0;
			m_screen_texture=0;
			m_screen_quad=0;
		}
		m_editor=0;
		m_font_chn=0;
		m_font_sys=0;
	}
	virtual void on_input(const xinput_buffer &input)
	{
		if(m_edit_mode && m_editor->handle_input(input))
			return;
		// handle key input
		for(xkeyque::const_iterator iter = input.keyque.begin(),
			end = input.keyque.end(); iter != end; ++iter)
		{
			if(m_debug_atlas)
			{
				break;
			}
			if(iter->msg==wyc::EV_KEY_DOWN)
			{
				if(iter->key == 'W')
				{
					m_wireframe = !m_wireframe;
				}
				else if(iter->key == 'F')
				{
					m_fxaa = !m_fxaa;
				}
				else if(iter->key == VK_F2)
				{
					m_edit_mode = !m_edit_mode;
					if(m_edit_mode)
						m_editor->open();
					else
						m_editor->close();
				}
				else if(iter->key == 'A')
				{
					m_debug_atlas=!m_debug_atlas;
					if(m_debug_atlas) 
					{
						m_card_rc->debug_atlas(client_width(),client_height());
					}
				}
				else if(iter->key == VK_SPACE)
				{
					if(m_debug_atlas)
					{
						m_card_rc->debug_next_atlas();
					}
					else m_network->rpc_echo("hello world");
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
		m_network->update();
		m_layout->update();
		if(m_edit_mode) 
		{
			m_editor->update();
		}
		assert(glGetError()==GL_NO_ERROR);
	}

	virtual void on_render(double, double)
	{
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		render_world();
		render_gui();
		assert(glGetError()==GL_NO_ERROR);
	}

	void render_world()
	{
		if(!m_card_rc->is_complete())
			return;
		m_card_rc->flush_atlas(m_renderer);
		if(m_debug_atlas)
			return;

		xmat4f_t camera_matrix;
		xrecti_t viewport;

		if(m_edit_mode) {
			m_editor->get_preview_matrix(camera_matrix);
			m_editor->get_preview_region(viewport);
			glViewport(viewport.xmin,viewport.ymin,viewport.width(),viewport.height());
		}

		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		if(!m_debug_atlas)
			glEnable(GL_CULL_FACE);
		if(m_fxaa && m_frame_buffer) {
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER,m_frame_buffer);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}
		if(m_wireframe) 
			glPolygonMode(GL_FRONT,GL_LINE);
		m_card_rc->draw(m_renderer,&camera_matrix);
		if(m_wireframe) 
			glPolygonMode(GL_FRONT,GL_FILL);
		if(m_fxaa && m_frame_buffer) {
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER,0);
			glViewport(0,0,m_screen_w,m_screen_h);
			draw_screen_quad();
		}
		if(!m_debug_atlas) 
			glDisable(GL_CULL_FACE);
	}

	void render_gui()
	{
		const wyc::xshader *shader;
		GLint uf;
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);

		// card texture atlas
		if(m_debug_atlas)
		{
			if(m_debug_flush) {
				m_card_rc->debug_flush(m_renderer);
			}
			if(m_renderer->use_shader(SHADER_CARD_LAYOUT)) {
				shader=m_renderer->get_shader();
				uf=shader->get_uniform("proj");
				glUniformMatrix4fv(uf,1,GL_TRUE,m_proj_gui.data());
				uf=shader->get_uniform("texture");
				glUniform1i(uf,0);
				m_card_rc->debug_draw_atlas();
			}
			return;
		}

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

		if(m_edit_mode)
			m_editor->render(m_renderer);
	}

	bool create_off_screen_buffer(unsigned screen_width, unsigned screen_height)
	{
		if(m_frame_buffer)
			return true;
		glGenFramebuffers(1,&m_frame_buffer);
		if(0==m_frame_buffer)
			return false;
		glGenRenderbuffers(1,&m_depth_buffer);
		if(0==m_depth_buffer)
		{
			glDeleteRenderbuffers(1,&m_depth_buffer);
			glDeleteFramebuffers(1,&m_frame_buffer);
			m_frame_buffer=0;
			m_depth_buffer=0;
			return false;
		}
		glGenTextures(1,&m_screen_texture);
		assert(0!=m_screen_texture);
		glBindTexture(GL_TEXTURE_2D,m_screen_texture);
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,screen_width,screen_height,0,GL_RGBA,GL_UNSIGNED_BYTE,0);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

		glBindRenderbuffer(GL_RENDERBUFFER,m_depth_buffer);
		glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT32,screen_width,screen_height);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER,m_frame_buffer);
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,m_screen_texture,0);
			glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,m_depth_buffer);
		GLenum fbo_status=glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
		if(fbo_status != GL_FRAMEBUFFER_COMPLETE)
		{
			assert(0);
		}
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER,0);
		assert(glGetError()==GL_NO_ERROR);

		glGenBuffers(1,&m_screen_quad);
		assert(0!=m_screen_quad);
		glBindBuffer(GL_ARRAY_BUFFER,m_screen_quad);
		GLfloat quad[] = {
			// vertex position
			0, 0, 0,
			screen_width, 0, 0,
			0, screen_height, 0,
			screen_width, screen_height, 0,
			// texcoord
			0, 0,
			1, 0,
			0, 1,
			1, 1,
		};
		
		xmat4f_t ortho;
		set_orthograph(ortho,0,0,-1,screen_width,screen_height,1);
		xvec3f_t *vertex = (xvec3f_t*)quad;
		xvec4f_t v4;
		v4.w=1;
		for(int i=0; i<4; ++i)
		{
			v4=vertex[i];
			vertex[i] = ortho * v4;
		}

		glBufferData(GL_ARRAY_BUFFER,sizeof(quad),quad,GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER,0);
		assert(glGetError()==GL_NO_ERROR);
		m_screen_w = screen_width;
		m_screen_h = screen_height;
		return true;
	}

	void draw_screen_quad()
	{
		if(!m_renderer->use_shader(SHADER_FXAA))
			return;

		glDisable(GL_DEPTH_TEST);
		const xshader *sh;
		GLint uf;

		glBindBuffer(GL_ARRAY_BUFFER,m_screen_quad);
		glVertexAttribPointer(wyc::USAGE_POSITION,3,GL_FLOAT,false,0,0);
		glVertexAttribPointer(wyc::USAGE_TEXTURE0,2,GL_FLOAT,false,0,(GLvoid*)(sizeof(GLfloat)*12));
		glEnableVertexAttribArray(USAGE_POSITION);
		glEnableVertexAttribArray(USAGE_TEXTURE0);
		
		sh = m_renderer->get_shader();
		uf = sh->get_uniform("screen_sample");
		glUniform1i(uf,0);
		uf=sh->get_uniform("screen_size");
		glUniform2f(uf,1.0f/m_screen_w,1.0f/m_screen_h);
		glBindTexture(GL_TEXTURE_2D,m_screen_texture);
		glDrawArrays(GL_TRIANGLE_STRIP,0,4);

		glDisableVertexAttribArray(USAGE_POSITION);
		glDisableVertexAttribArray(USAGE_TEXTURE0);
		glBindBuffer(GL_ARRAY_BUFFER,0);
	}

	int login_state() const
	{
		if(m_user.is_verified)
			return 1;
		if(m_user.is_linking)
			return 2;
		return 0;
	}

	void login(const std::string &usr, const std::string &pwd)
	{
		if(m_user.is_verified || m_user.is_linking)
		{
			return;
		}
		MD5_CTX ctx;
		md5_init(&ctx);
		md5_update(&ctx,(const unsigned char*)pwd.c_str(),pwd.length());
		md5_final(&ctx);
		char digest[32];
		md5_hexdigest(&ctx,digest);
		m_user.account=usr;
		m_user.pwd.assign(digest,digest+32);
		if(m_network->is_connected())
		{
			m_user.is_linking=true;
			m_network->rpc_login(m_user.account.c_str(),m_user.pwd.c_str(),_login_callback);
			m_user.pwd.clear();
			return;
		}
		if(!m_network->connect(m_server_ip.c_str(),&_connection_callback))
		{
			_on_login_fail(1,L"无法连接服务器");
		}
	}

	void logout ()
	{
		if(!m_user.is_verified)
			return;
		if(!m_network->is_connected())
			return;
		m_network->rpc_logout();
		m_user.is_verified=false;
		m_user.account.clear();
		m_user.pwd.clear();
	}

private:
	static void _connection_callback(int err)
	{
		xgame_card& app = (xgame_card&)xgame::singleton();
		if(err) {
			app._on_login_fail(err,L"无法连接服务器");
			return;
		}
		if(!app.m_user.account.empty())
		{
			app.m_network->rpc_login(app.m_user.account.c_str(),app.m_user.pwd.c_str(),_login_callback);
			app.m_user.pwd.clear();
		}
	}

	static void _login_callback(int err)
	{
		xgame_card& app = (xgame_card&)xgame::singleton();
		if(err) {
			app._on_login_fail(err,L"登录失败");
			return;
		}
		app._on_login_ok();
	}

	void _on_login_ok()
	{
		m_user.is_verified=true;
		m_user.is_linking=false;
		if(m_edit_mode) {
			m_editor->on_login_ok();
			return;
		}
	}

	void _on_login_fail(int err, const wchar_t *info)
	{
		m_user.is_linking=false;
		if(m_edit_mode) {
			m_editor->on_login_fail(err,info);
			return;
		}
	}
};

xgame* xgame::create_game() 
{
	return new xgame_card;
}

int get_login_state()
{
	xgame_card& app = (xgame_card&)xgame::singleton();
	return app.login_state();
}

void login (const std::string &usr, const std::string &pwd)
{
	xgame_card& app = (xgame_card&)xgame::singleton();
	app.login(usr,pwd);
}

void logout()
{
	xgame_card& app = (xgame_card&)xgame::singleton();
	app.logout();
}
