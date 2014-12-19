#include "fscorepch.h"
#include "xobject3d.h"
#include "renderer.h"
#include "vertexbatch.h"
#include "xcamera.h"

#pragma warning (disable: 4245) // ÓÐ·ûºÅ/ÎÞ·ûºÅ²»Æ¥Åä

using wyc::xpointer;
using wyc::xobjevent;
using wyc::xpackev;
using wyc::xobject3d;
using wyc::xrenderer;
using wyc::xvertex_batch;

class xprimitive_draw : public xobject3d
{
	USE_EVENT_MAP;
	wyc::xpointer<wyc::xcamera> m_spCamera;
	unsigned m_shader;
public:
	xprimitive_draw() {
		m_shader=0;
	}
	~xprimitive_draw() {
		m_spCamera=0;
	}
	virtual void update(float) {
		xrenderer::get_renderer()->draw(this);
	}
	virtual void draw() {
		xrenderer *pRenderer=xrenderer::get_renderer();
		if(m_shader==0) {
		//	m_shader=pRenderer->get_shader("basic");
			m_shader=pRenderer->get_shader("simple");
			if(m_shader==0)
				return;
		}
		glCullFace(GL_BACK);
		glEnable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
		pRenderer->use_shader(m_shader);
		GLint uf;
		if(m_spCamera) {
			uf=glGetUniformLocation(m_shader,"mvp");
			if(uf!=-1)
				glUniformMatrix4fv(uf,1,GL_FALSE,m_spCamera->get_mvpmatrix().data());
		}
		uf=glGetUniformLocation(m_shader,"color");
		if(uf!=-1)
			glUniform4f(uf,0,1.0f,0,1.0f);
	}
private:
	void set_camera(xobjevent *pev)
	{
		wyc::xcamera *pcam;
		if(!((xpackev*)pev)->unpack("o",&pcam)) 
			return;
		m_spCamera=pcam;
	}
};
BEGIN_EVENT_MAP(xprimitive_draw,xobject3d)
	REG_EVENT(set_camera)
END_EVENT_MAP

class xbase_triangle : public xobject3d
{
	USE_EVENT_MAP;
	xpointer<xvertex_batch> m_spBatch;
public:
	xbase_triangle() {
		xobjevent *pev=xpackev::pack("od",this,ON_MODEL_OK);
		xrenderer::get_renderer()->send_event(xrenderer::EV_TRIANGLE,pev);
	}
	~xbase_triangle() {
		m_spBatch=0;
	}
	virtual void update(float) {
		if(m_spBatch)
			xrenderer::get_renderer()->draw(this);
	}
	virtual void draw() {
		m_spBatch->render();
	}
	enum EVENT_ID {
		ON_MODEL_OK=0x10DCBBEF,
	};
private:
	void on_model_ok(xobjevent *pev) {
		wyc::xvertex_batch *pbatch;
		if(!((xpackev*)pev)->unpack("o",&pbatch)) 
			return;
		m_spBatch=pbatch;
		if(m_spBatch)
			show();
	}
};
BEGIN_EVENT_MAP(xbase_triangle,xobject3d)
	REG_EVENT(on_model_ok)
END_EVENT_MAP

class xcube : public xobject3d
{
	USE_EVENT_MAP;
	xpointer<xvertex_batch> m_spBatch;
public:
	xcube() {
		m_spBatch=0;
		float size[3]={1,1,1};
		xobjevent *pev=xpackev::pack("od3f",this,ON_MODEL_OK,size);
		xrenderer::get_renderer()->send_event(xrenderer::EV_CUBE,pev);
	}
	~xcube() {
		m_spBatch=0;
	}
	virtual void update(float) {
		if(m_spBatch)
			xrenderer::get_renderer()->draw(this);
	}
	virtual void draw() {
		m_spBatch->render();
	}
	enum EVENT_ID {
		ON_MODEL_OK=0x10DCBBEF,
	};
private:
	void on_model_ok(xobjevent *pev) {
		wyc::xvertex_batch *pbatch;
		if(!((xpackev*)pev)->unpack("o",&pbatch)) 
			return;
		m_spBatch=pbatch;
		if(m_spBatch)
			show();
	}
};
BEGIN_EVENT_MAP(xcube,xobject3d)
	REG_EVENT(on_model_ok)
END_EVENT_MAP

class xsphere : public xobject3d
{
	USE_EVENT_MAP;
	xpointer<xvertex_batch> m_spBatch;
public:
	xsphere() {
		m_spBatch=0;
		xobjevent *pev=xpackev::pack("odf",this,ON_MODEL_OK,0.2f);
		xrenderer::get_renderer()->send_event(xrenderer::EV_SPHERE,pev);
	}
	~xsphere() {
		m_spBatch=0;
	}
	virtual void update(float) {
		if(m_spBatch)
			xrenderer::get_renderer()->draw(this);
	}
	virtual void draw() {
		m_spBatch->render();
	}
	enum EVENT_ID {
		ON_MODEL_OK=0x10DCBBEF,
	};
private:
	void on_model_ok(xobjevent *pev) {
		wyc::xvertex_batch *pbatch;
		if(!((xpackev*)pev)->unpack("o",&pbatch)) 
			return;
		m_spBatch=pbatch;
		if(m_spBatch)
			show();
	}
};
BEGIN_EVENT_MAP(xsphere,xobject3d)
	REG_EVENT(on_model_ok)
END_EVENT_MAP




