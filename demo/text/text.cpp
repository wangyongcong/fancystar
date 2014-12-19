#include "fscorepch.h"

#include "wyc/game/game.h"
#include "wyc/world/camera.h"
#include "wyc/gui/gui_image.h"
#include "wyc/gui/gui_text.h"

using wyc::xgame;
using wyc::xrenderer;
using wyc::xvec3f_t;
using wyc::xpointer;
using wyc::xobject;
using wyc::xgameobj;
using wyc::xcamera;
using wyc::xcom_transform;
using wyc::xcom_renderer;
using wyc::xguiobj;
using wyc::xcom_layout;
using wyc::xgui_text;
using wyc::xgui_image;

const unsigned SHADER_SIMPLE_2D = wyc::strhash("simple2d");
const unsigned SHADER_GLYPH = wyc::strhash("glyph");

class xgame_helloworld : public xgame
{
	xpointer<xgameobj> m_root;
	xpointer<xcamera> m_cam_gui;
	xpointer<xgameobj> m_gui;
	wchar_t *m_doc;
	size_t m_doc_size, m_doc_show;
	unsigned m_char_per_page, m_char_advance;
public:
	xgame_helloworld() 
	{
		m_doc=0;
		m_doc_size=0;
		m_doc_show=0;
		m_char_per_page=(wyc::xtile_buffer::max_vertex_count()>>2)+1;
		m_char_advance=m_char_per_page-4;
	}
	virtual bool on_game_init() 
	{
		xguiobj *gui_obj;
		xcom_layout *gui_layer;
		xgui_image *gui_img;

		m_gui=wycnew xgameobj;
		m_gui->set_name("gui");
		m_gui->add_component<xcom_transform>();
		gui_layer=m_gui->add_component<wyc::xcom_layout>();

		gui_obj=gui_layer->new_element("xgui_image","bk",100,100,-1);
		gui_img=wyc_safecast(wyc::xgui_image,gui_obj);
		gui_img->set_image("res/common/bg_blue.png",wyc::BLIT_33);
		gui_img->set_size(400,300);

		xgui_text *gui_text[2];
		char text_name[32];
		for(int i=0; i<2; ++i) {
			sprintf_s(text_name,32,"text[%d]",i);
			gui_obj=gui_layer->new_element("xgui_text",text_name,110,110,float(i));
			gui_text[i]=wyc_safecast(wyc::xgui_text,gui_obj);
			gui_text[i]->set_size(380,280);
			gui_text[i]->set_word_wrap(true);
		}
		gui_text[0]->set_font("res/wqy16_outline.font");
		gui_text[0]->set_color(0,0,0,255);
		gui_text[1]->set_font("res/wqy16.font");
		gui_text[1]->set_color(255,255,255,255);

		FILE* f=fopen("res/demo-text/doc_utf16.txt","r");
		fseek(f,0,SEEK_END);
		m_doc_size=ftell(f);
		fseek(f,0,SEEK_SET);
		m_doc=new wchar_t[m_doc_size+1];
		fread(m_doc,sizeof(char),m_doc_size,f);
		fclose(f);
		gui_text[0]->set_text(m_doc,m_doc_show,m_char_per_page);
		gui_text[1]->set_text(m_doc,m_doc_show,m_char_per_page);

		m_cam_gui=wycnew xcamera;
		m_cam_gui->set_name("gui camera");
		m_cam_gui->set_ui_view(800,600);

		glClearColor(0,0,0,1.0f);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	//	glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);

		return true;
	}
	virtual void on_game_exit() {
		wyc_print("quit game");
		m_gui=0;
		m_cam_gui=0;
		if(m_doc) {
			delete [] m_doc;
			m_doc=0;
		}
	}
	virtual void process_input()
	{
		input().update_input();
		const wyc::xinput::xinputbuffer &ibuff=input().get_buffer();
		wyc::xinput::xkeyque::iterator iter, end;
		for(iter=ibuff.keyque.begin(), end=ibuff.keyque.end(); iter!=end; ++iter)
		{
			if(iter->msg==wyc::EV_KEY_UP)
			{
				switch(iter->key)
				{
				case VK_SPACE:
					switch_text_color();
					break;
				case VK_DOWN:
					page_down();
					break;
				case VK_UP:
					page_up();
					break;
				}
			}
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

		wyc::xuniform_buffer *ub=m_renderer->uniform_buffer();
		ub->set_uniform("gui_mvp",m_cam_gui->get_mvp_matrix());
		ub->commit();

		m_renderer->use_shader(SHADER_GLYPH);
		const wyc::xshader *shader=m_renderer->get_shader();
		GLint uf=shader->get_uniform("gui_texture");
		glUniform1i(uf,0);
		m_gui->render(m_renderer);

		assert(glGetError()==GL_NO_ERROR);
	}
	void switch_text_color()
	{
		xcom_layout *gui_layer;
		gui_layer=m_gui->add_component<wyc::xcom_layout>();
		if(!gui_layer) return;
		xgui_text *gui_text[2];
		gui_text[0]=wyc_safecast(xgui_text,gui_layer->get_element("text[0]"));
		gui_text[1]=wyc_safecast(xgui_text,gui_layer->get_element("text[1]"));
		wyc::uint32_t c=gui_text[0]->get_color();
		gui_text[0]->set_color(gui_text[1]->get_color());
		gui_text[1]->set_color(c);
	}
	void page_down()
	{
		xcom_layout *gui_layer;
		gui_layer=m_gui->add_component<wyc::xcom_layout>();
		if(!gui_layer) return;
		xgui_text *gui_text[2];
		gui_text[0]=wyc_safecast(xgui_text,gui_layer->get_element("text[0]"));
		gui_text[1]=wyc_safecast(xgui_text,gui_layer->get_element("text[1]"));
		if(m_doc_show+m_char_advance<m_doc_size) {
			m_doc_show+=m_char_advance;
			gui_text[0]->set_text(m_doc,m_doc_show,m_char_per_page);
			gui_text[1]->set_text(m_doc,m_doc_show,m_char_per_page);
		}
	}
	void page_up()
	{
		xcom_layout *gui_layer;
		gui_layer=m_gui->add_component<wyc::xcom_layout>();
		if(!gui_layer) return;
		xgui_text *gui_text[2];
		gui_text[0]=wyc_safecast(xgui_text,gui_layer->get_element("text[0]"));
		gui_text[1]=wyc_safecast(xgui_text,gui_layer->get_element("text[1]"));
		if(m_doc_show>0) {
			if(m_doc_show>m_char_advance) 
				m_doc_show-=m_char_advance;
			else m_doc_show=0;
			gui_text[0]->set_text(m_doc,m_doc_show,m_char_per_page);
			gui_text[1]->set_text(m_doc,m_doc_show,m_char_per_page);
		}
	}
};

xgame* xgame::create_game() 
{
	return new xgame_helloworld;
}




