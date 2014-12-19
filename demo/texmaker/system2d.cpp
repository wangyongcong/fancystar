#include "fscorepch.h"
#include "xtexture.h"
#include "xcamera.h"
#include "xobject2d.h"
#include "xlayer.h"

#pragma warning (disable: 4245) // ÓÐ·ûºÅ/ÎÞ·ûºÅ²»Æ¥Åä

namespace wyc
{

class xeditor2d : public xobject
{
	USE_EVENT_MAP;
	wyc::xpointer<wyc::xcamera> m_spCamera;
	xpointer<xlayer> m_spLayerBk;
public:
	xeditor2d() {
		set_id(wyc::strhash("object2d editor"));
		xrenderer *pRenderer=xrenderer::get_renderer();
		xpackev *pevCreate=xpackev::pack("dd",xobject2d::ms_renderPassID,xobject2d::ms_pickerPassID);
		pRenderer->send_event(xrenderer::EV_CREATE_PROXY,xpackev::pack("sddp",\
			"xeditor2d_render",id(),RENDER_PRIORITY(1),pevCreate));
	}
	virtual void on_destry() {
		m_spCamera=0;
		m_spLayerBk=0;
	}
	virtual void update(double accum_time, double frame_time) {
		unsigned nid=wyc::strhash("ev_mvp_matrix");
		xpackev *notify=xpackev::pack("16f",m_spCamera->get_mvpmatrix().data());
		xpackev *pev=xpackev::pack("ddp",xobject2d::ms_pickerPassID,nid,notify);
		xrenderer::get_renderer()->send_event(xrenderer::EV_NOTIFY_PROXY,pev);
		notify=xpackev::pack("16f",m_spCamera->get_mvpmatrix().data());
		pev=xpackev::pack("ddp",xobject2d::ms_renderPassID,nid,notify);
		xrenderer::get_renderer()->send_event(xrenderer::EV_NOTIFY_PROXY,pev);
		m_spLayerBk->update(accum_time,frame_time);
	}
	enum EVENT_ID 
	{
		EV_INITIALIZE = 0x71EA4CD,
		EV_CREATE_TILE = 0x11927B26,
	};
private:
	//-------------------------------------------------------
	// event handlers
	//-------------------------------------------------------
	void ev_initialize(xobjevent *pev)
	{
		wyc::xcamera *pcam;
		if(!((xpackev*)pev)->unpack("o",&pcam)) 
			return;
		m_spCamera=pcam;
		
		

		m_spLayerBk=wycnew xlayer;
		m_spLayerBk->assign_context(get_context());
		m_spLayerBk->create_proxy();
		m_spLayerBk->set_camera(pcam);
		m_spLayerBk->set_size(512,512);
		m_spLayerBk->set_keypoint(256,256);
		m_spLayerBk->set_image("",wyc::BLIT_HV);
		m_spLayerBk->set_pick(true,wyc::PICK_QUAD);
	}
	void ev_create_tile(xobjevent *pev)
	{
		wyc::xtexture* ptex;
		if(!((xpackev*)pev)->unpack("o",&ptex))
			return;
		/*
		xobject2d *pobj=wycnew xobject2d;
		m_spLayerBk->add_child(pobj);
		unsigned w=ptex->width(), h=ptex->height();
		pobj->set_size(float(w),float(h));
		pobj->set_keypoint(w*0.5f,h*0.5f);
		pobj->set_image(ptex);
		pobj->set_pick(true,xobject2d::PICK_ALPHA);
		*/
	}
	/*
	void r_create_fbo()
	{
		glGenFramebuffers(1,&m_fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER,m_fbo);
		GLuint rbo;
		glGenRenderbuffers(1,&rbo);
		glBindRenderbuffer(GL_RENDERBUFFER,rbo);
		glRenderbufferStorage(GL_RENDERBUFFER,GL_RGBA8,800,600);
		glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,
			GL_RENDERBUFFER,rbo);
		GLenum status=glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
		if(status != GL_FRAMEBUFFER_COMPLETE)
		{
			switch (status)
			{
				case GL_FRAMEBUFFER_UNDEFINED:
					wyc_error("Oops, no window exists?");
					break;
				case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
					wyc_error("Check the status of each attachment");
					break;
				case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
					wyc_error("Attach at least one buffer to the FBO");
					break;
				case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
					wyc_error("Check that all attachments enabled via glDrawBuffers exist in FBO");
					break;
				case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
					wyc_error("Check that the buffer specified via glReadBuffer exists in FBO");
					break;
				case GL_FRAMEBUFFER_UNSUPPORTED:
					wyc_error("Reconsider formats used for attached buffers");
					break;
				case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
					wyc_error("Make sure the number of samples for each attachment is the same");
					break;
				case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
					wyc_error("Make sure the number of layers for each attachment is the same");
					break;
			}
		}
		assert(GL_FRAMEBUFFER_COMPLETE==status);
	}
	void r_ouput_fbo()
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER,0);
		glBindFramebuffer(GL_READ_FRAMEBUFFER,m_fbo);
		glBlitFramebuffer(0,0,800,600,0,0,400,300,GL_COLOR_BUFFER_BIT,GL_LINEAR);
	}*/

}; // class xeditor2d

BEGIN_EVENT_MAP(xeditor2d,xobject)
	REG_EVENT(ev_initialize)
	REG_EVENT(ev_create_tile)
END_EVENT_MAP

}; // namespace wyc
