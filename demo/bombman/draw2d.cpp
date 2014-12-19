#include "fscorepch.h"
#include "xobject2d.h"
#include "xcamera.h"
#include "renderer.h"
#include "vertexbatch.h"

using wyc::xpointer;
using wyc::xobjevent;
using wyc::xpackev;
using wyc::xrenderer;

class xdraw2d : public wyc::xrenderobj
{
	USE_EVENT_MAP;
	xpointer<wyc::xcamera> m_spCamera;
	unsigned m_shader;
	bool m_bWireMode;
	bool m_bDisplayModeChg;
	xpointer<wyc::xvertex_batch> m_spPrimBatch;
public:
	xdraw2d() {
		m_shader=0;
		m_bWireMode=false;
		m_bDisplayModeChg=true;
	}
	~xdraw2d() {
		m_spCamera=0;
		m_spPrimBatch=0;
	}
	virtual void update(float interval) {
		m_spCamera->update(interval);
		xrenderer::get_renderer()->draw(this);
	}
	void draw() {
		if(-1==m_shader)
			return;
		if(m_bDisplayModeChg) {
			if(m_bWireMode) 
				glPolygonMode(GL_FRONT,GL_LINE);
			else
				glPolygonMode(GL_FRONT,GL_FILL);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		}
		xrenderer *pRenderer=xrenderer::get_renderer();
		if(0==m_shader) {
			m_shader=pRenderer->get_shader("basic");
			if(m_shader==0) {
				m_shader=-1;
				wyc_error("[OpenGL] shader not found: basic");
				return;
			}
		}
		if(pRenderer->use_shader(m_shader)) {
			GLint uf;
			if(m_spCamera)  {
				uf=glGetUniformLocation(m_shader,"mvp");
				if(uf!=-1) {
					glUniformMatrix4fv(uf,1,GL_FALSE,m_spCamera->get_mvpmatrix().data());
				}
			}
			uf=glGetUniformLocation(m_shader,"color");
			if(uf!=-1) {
				glUniform4f(uf,0,1.0f,0,1.0f);
			}
		}
		if(!m_spPrimBatch) {
			r_build_triangle();
		//	r_build_cube();
			assert(m_spPrimBatch);
		}
		m_spPrimBatch->render();
	}
	enum EVENT_ID 
	{
		EV_SET_CAMERA = 0x9F59DB94,
		EV_SWITCH_WIRE_MODE = 0x562F61F9,
	};
private:
	void set_camera(xobjevent *pev)
	{
		wyc::xcamera *pcam;
		if(!((xpackev*)pev)->unpack("o",&pcam)) 
			return;
		m_spCamera=pcam;
		wyc_sys("set camera ok");
	}
	void switch_wire_mode(xobjevent *pev)
	{
		m_bWireMode=!m_bWireMode;
		m_bDisplayModeChg=true;
	}
	void r_build_cube()
	{
		float size[3]={1.0f,1.0f,1.0f};
		size[0]*=0.5f;
		size[1]*=0.5f;
		size[2]*=0.5f;
		GLfloat cubeVertex[]={
			-0.5f*size[0],  0.5f*size[1],  0.5f*size[2], 
			-0.5f*size[0], -0.5f*size[1],  0.5f*size[2],
			 0.5f*size[0], -0.5f*size[1],  0.5f*size[2], 
			 0.5f*size[0],  0.5f*size[1],  0.5f*size[2],
			-0.5f*size[0],  0.5f*size[1], -0.5f*size[2], 
			-0.5f*size[0], -0.5f*size[1], -0.5f*size[2],
			 0.5f*size[0], -0.5f*size[1], -0.5f*size[2], 
			 0.5f*size[0],  0.5f*size[1], -0.5f*size[2],
		};
		GLfloat cubeColor[]= {
			1.0f, 0.0f, 0.0f, 1.0f,
			0.0f, 1.0f, 0.0f, 1.0f,
			0.0f, 0.0f, 1.0f, 1.0f,
			0.0f, 1.0f, 0.0f, 1.0f,
			1.0f, 0.0f, 0.0f, 1.0f,
			0.0f, 1.0f, 0.0f, 1.0f,
			1.0f, 0.0f, 1.0f, 1.0f,
			0.0f, 1.0f, 0.0f, 1.0f,
		};
		GLuint cubeIndex[]={
			0, 1, 2, 0, 2, 3, // front face
			3, 2, 6, 3, 6, 7, // right
			7, 6, 5, 7, 5, 4, // back
			4, 5, 1, 4, 1, 0, // left
			4, 0, 3, 4, 3, 7, // top
			1, 5, 6, 1, 6, 2, // bottom
		};

		GLuint vbo[3];
		glGenBuffers(3,vbo);

		// vertex position
		glBindBuffer(GL_ARRAY_BUFFER,vbo[0]);
		glBufferData(GL_ARRAY_BUFFER,sizeof(cubeVertex),cubeVertex,GL_STATIC_DRAW);
	//	glVertexAttribPointer(USAGE_POSITION,3,GL_FLOAT,GL_FALSE,0,(void*)0);
	//	glEnableVertexAttribArray(USAGE_POSITION);
		
		// vertex index
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vbo[1]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(cubeIndex),cubeIndex,GL_STATIC_DRAW);

		// vertex color
		glBindBuffer(GL_ARRAY_BUFFER,vbo[2]);
		glBufferData(GL_ARRAY_BUFFER,sizeof(cubeColor),cubeColor,GL_STATIC_DRAW);
		
		wyc::xvertex_batch *pbatch=(wyc::xvertex_batch*)xrenderer::get_renderer()->create_batch("cube batch",0);
		wyc::xvertex_buffer *pvb;
		pvb=wycnew wyc::xvertex_buffer;
		pvb->create(vbo[0]);
		pbatch->add_buffer(pvb,wyc::USAGE_POSITION,3,GL_FLOAT,false,0,0);

		pvb=wycnew wyc::xvertex_buffer;
		pvb->create(vbo[1]);
		pbatch->set_index(pvb,GL_UNSIGNED_INT);
		
		pvb=wycnew wyc::xvertex_buffer;
		pvb->create(vbo[2]);
		pbatch->add_buffer(pvb,wyc::USAGE_COLOR,4,GL_FLOAT,false,0,0);

		pbatch->set_mode(GL_TRIANGLES,36,0);
		m_spPrimBatch=pbatch;
		pbatch->decref();
	}
	void r_build_triangle()
	{
		GLfloat triangleVertex[]= {
			   0.0f,   0.5f,  0.0f, // top
			-0.433f, -0.25f,  0.0f, // left
			 0.433f, -0.25f,  0.0f, // right
		};
		GLfloat triangleColor[]= {
			1.0f, 0.0f, 0.0f, 1.0f,
			0.0f, 1.0f, 0.0f, 1.0f,
			0.0f, 0.0f, 1.0f, 1.0f,
		};
		GLuint vbo[2];
		glGenBuffers(2,vbo);

		// vertex position
		glBindBuffer(GL_ARRAY_BUFFER,vbo[0]);
		glBufferData(GL_ARRAY_BUFFER,sizeof(triangleVertex),triangleVertex,GL_STATIC_DRAW);
	//	glVertexAttribPointer(USAGE_POSITION,3,GL_FLOAT,GL_FALSE,0,(void*)0);
	//	glEnableVertexAttribArray(USAGE_POSITION);

		// vertex color
		glBindBuffer(GL_ARRAY_BUFFER,vbo[1]);
		glBufferData(GL_ARRAY_BUFFER,sizeof(triangleColor),triangleColor,GL_STATIC_DRAW);
	//	glVertexAttribPointer(USAGE_COLOR,4,GL_FLOAT,GL_FALSE,0,(void*)0);
	//	glEnableVertexAttribArray(USAGE_COLOR);

		wyc::xvertex_batch *pbatch=(wyc::xvertex_batch*)xrenderer::get_renderer()->create_batch("triangle batch",0);
		wyc::xvertex_buffer *pvb;

		pvb=wycnew wyc::xvertex_buffer;
		pvb->create(vbo[0]);
		pbatch->add_buffer(pvb,wyc::USAGE_POSITION,3,GL_FLOAT,false,0,0);
		
		pvb=wycnew wyc::xvertex_buffer;
		pvb->create(vbo[1]);
		pbatch->add_buffer(pvb,wyc::USAGE_COLOR,4,GL_FLOAT,false,0,0);

		pbatch->set_mode(GL_TRIANGLES,3,0);
		m_spPrimBatch=pbatch;
		pbatch->decref();
	}
};
BEGIN_EVENT_MAP(xdraw2d,xrenderobj)
	REG_EVENT(set_camera)
	REG_EVENT(switch_wire_mode)
END_EVENT_MAP
