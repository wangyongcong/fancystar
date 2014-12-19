#include "fscorepch.h"
#include "wyc/render/glrenderer.h"
#include "wyc/render/vertexbatch.h"
#include "wyc/render/texture.h"
#include "wyc/render/renderobj.h"

/**	@brief 创建测试三角形
	@param xpackev:od
		- object* 返回的对象
		- int 返回的EventID
	@return xpackev:o 
		- xvertex_batch* 顶点数据
*/
void ev_triangle(xobjevent *pev);
/**	@brief 创建立方体
	@param xpackev:od3f
		- object* 返回的对象
		- int 返回的EventID
		- xvec3f_t 立方体的尺寸
	@return xpackev:o 
		- xvertex_batch* 顶点数据
*/
void ev_cube(xobjevent *pev);
/**	@brief 创建球体
	@param xpackev:odf
		- object* 返回的对象
		- int 返回的EventID
		- float 精细等级(0.0~1.0)
	@return xpackev:o 
		- xvertex_batch* 顶点数据
*/
void ev_sphere(xobjevent *pev);
/**	@brief 加载默认纹理
	@param xpackev:od
		- object* 返回的对象
		- int 返回的EventID
	@return xpackev:o
		- xtexture* 纹理对象
*/
void ev_default_texture(xobjevent *pev);
/**	@brief 加载纹理
	@param xpackev:ods
		- object* 返回的对象
		- int 返回的EventID
		- const char* 图像文件名
	@return xpackev:o
		- xtexture* 纹理对象
*/
void ev_load_texture(xobjevent *pev);
/**	@brief 创建代理对象
	@param xpackev:sdde
		- const char* 对象类型名称
		- int 对象pid
		- int 绘制次序
		- xobjevent* 创建参数
*/

namespace wyc
{

BEGIN_EVENT_MAP(xglrenderer,xobject)
	REG_EVENT(ev_triangle)
	REG_EVENT(ev_cube)
	REG_EVENT(ev_sphere)
	REG_EVENT(ev_default_texture)
	REG_EVENT(ev_load_texture)
END_EVENT_MAP

void xglrenderer::ev_triangle(xobjevent *pev)
{
	xobject *pNotifier;
	int evid;
	if(!((xpackev*)pev)->unpack("od",&pNotifier,&evid)) {
		wyc_error("xglrenderer::triangle: bad args");
		return;
	}

	unsigned batchid=strhash("base triangle");
	xvertex_batch *pbatch=(xvertex_batch*)m_groups[GROUP_BATCH]->request(batchid);

	if(pbatch==0) {
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

		// vertex color
		glBindBuffer(GL_ARRAY_BUFFER,vbo[1]);
		glBufferData(GL_ARRAY_BUFFER,sizeof(triangleColor),triangleColor,GL_STATIC_DRAW);

		xvertex_buffer *pvb;

		if(GLEW_ARB_vertex_array_object) {
			GLuint vao;
			glGenVertexArrays(1,&vao);
			glBindVertexArray(vao);

			glBindBuffer(GL_ARRAY_BUFFER,vbo[0]);
			glVertexAttribPointer(USAGE_POSITION,3,GL_FLOAT,GL_FALSE,0,(void*)0);
			glEnableVertexAttribArray(USAGE_POSITION);

			glBindBuffer(GL_ARRAY_BUFFER,vbo[1]);
			glVertexAttribPointer(USAGE_COLOR,4,GL_FLOAT,GL_FALSE,0,(void*)0);
			glEnableVertexAttribArray(USAGE_COLOR);
			
			glBindVertexArray(0);

			pbatch=(xvertex_batch*)m_groups[GROUP_BATCH]->create(batchid,(void*)vao);
			pvb=wycnew xvertex_buffer;
			pvb->create(vbo[0]);
			pbatch->add_buffer(pvb);
			pvb=wycnew xvertex_buffer;
			pvb->create(vbo[1]);
			pbatch->add_buffer(pvb);
		}
		else {
			pbatch=(xvertex_batch*)m_groups[GROUP_BATCH]->create(batchid,0);
			pvb=wycnew xvertex_buffer;
			pvb->create(vbo[0]);
			pbatch->add_buffer(pvb,USAGE_POSITION,3,GL_FLOAT,false,0,0);
			pvb=wycnew xvertex_buffer;
			pvb->create(vbo[1]);
			pbatch->add_buffer(pvb,USAGE_COLOR,4,GL_FLOAT,false,0,0);
		}
		pbatch->set_mode(GL_TRIANGLES,3,0);
	}

	if(pNotifier) {
		pev=xpackev::pack("o",pbatch);
		pNotifier->on_event(evid,pev);
	}
}

void xglrenderer::ev_cube(xobjevent *pev)
{
	xobject *pNotifier;
	int evid;
	float *size;
	if(!((xpackev*)pev)->unpack("od3f",&pNotifier,&evid,&size)) {
		wyc_error("xglrenderer::cube: bad args");
		return;
	}
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
	GLuint vbo[2];
	glGenBuffers(2,vbo);

	// vertex position
	glBindBuffer(GL_ARRAY_BUFFER,vbo[0]);
	glBufferData(GL_ARRAY_BUFFER,sizeof(cubeVertex),cubeVertex,GL_STATIC_DRAW);

	// vertex color
	glBindBuffer(GL_ARRAY_BUFFER,vbo[1]);
	glBufferData(GL_ARRAY_BUFFER,sizeof(cubeColor),cubeColor,GL_STATIC_DRAW);

	// vertex index
	unsigned id=strhash("cube index");
	xvertex_buffer *pvbIndex=(xvertex_buffer*)m_groups[GROUP_VERTEXBUFFER]->request(id);
	if(pvbIndex==0) {
		// create index buffer
		GLushort cubeIndex[]={
			0, 1, 2, 0, 2, 3, // front face
			3, 2, 6, 3, 6, 7, // right
			7, 6, 5, 7, 5, 4, // back
			4, 5, 1, 4, 1, 0, // left
			4, 0, 3, 4, 3, 7, // top
			1, 5, 6, 1, 6, 2, // bottom
		};
		GLuint ibo;
		glGenBuffers(1,&ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(cubeIndex),cubeIndex,GL_STATIC_DRAW);
		pvbIndex=(xvertex_buffer*)m_groups[GROUP_VERTEXBUFFER]->create(id,(void*)ibo);
	}
	xvertex_batch *pbatch;
	xvertex_buffer *pvb;
	if(GLEW_ARB_vertex_array_object) {
		GLuint vao;
		glGenVertexArrays(1,&vao);
		glBindVertexArray(vao);

		glBindBuffer(GL_ARRAY_BUFFER,vbo[0]);
		glVertexAttribPointer(USAGE_POSITION,3,GL_FLOAT,GL_FALSE,0,(void*)0);
		glEnableVertexAttribArray(USAGE_POSITION);

		glBindBuffer(GL_ARRAY_BUFFER,vbo[1]);
		glVertexAttribPointer(USAGE_COLOR,4,GL_FLOAT,GL_FALSE,0,(void*)0);
		glEnableVertexAttribArray(USAGE_COLOR);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,pvbIndex->handle());

		glBindVertexArray(0);

		pbatch=(xvertex_batch*)m_groups[GROUP_BATCH]->create(xresbase::NULL_ID,(void*)vao);
		pvb=wycnew xvertex_buffer;
		pvb->create(vbo[0]);
		pbatch->add_buffer(pvb);
		pvb=wycnew xvertex_buffer;
		pvb->create(vbo[1]);
		pbatch->add_buffer(pvb);
	}
	else {
		pbatch=(xvertex_batch*)m_groups[GROUP_BATCH]->create(xresbase::NULL_ID,0);
		pvb=wycnew xvertex_buffer;
		pvb->create(vbo[0]);
		pbatch->add_buffer(pvb,USAGE_POSITION,3,GL_FLOAT,false,0,0);
		pvb=wycnew xvertex_buffer;
		pvb->create(vbo[1]);
		pbatch->add_buffer(pvb,USAGE_COLOR,4,GL_FLOAT,false,0,0);
	}
	pbatch->set_index(pvbIndex,GL_UNSIGNED_SHORT);
	pbatch->set_mode(GL_TRIANGLES,36,0);
	
	if(pNotifier) {
		pev=xpackev::pack("o",pbatch);
		pNotifier->on_event(evid,pev);
	}
}

void xglrenderer::ev_sphere(xobjevent *pev) 
{
	xobject *pNotifier;
	int evid;
	float detail;
	if(!((xpackev*)pev)->unpack("odf",&pNotifier,&evid,&detail)) {
		wyc_error("xglrenderer::sphere: bad args");
		return;
	}

	unsigned batchid=strhash("sphere");
	xvertex_batch *pbatch=(xvertex_batch*)m_groups[GROUP_BATCH]->request(batchid);
	if(pbatch==0) {
		// a quarter of circle have 4~16 points
		unsigned quarter=4+unsigned(detail*12); 
		unsigned circle=quarter*4;
		float step=float(XMATH_HPI/quarter);
		float rad=step;
		// pre-compute the sin & cos
		float *sina=new float[quarter], *cosa=new float[quarter];
		sina[0]=0, cosa[0]=1;
		for(unsigned i=1; i<quarter; ++i) {
			sina[i]=sin(rad);
			cosa[i]=cos(rad);
			rad+=step;
		}
		// fill vertex positions
		unsigned vertexCount=(2*quarter-1)*quarter*4+2;
		xvec3f_t *buffer=new xvec3f_t[vertexCount];
		xvec3f_t *part1=buffer, *part2, *part3, *part4;
		// top half of sphere
		part1->set(0,1,0);
		part1+=1;
		for(int i=quarter-1; i>=0; --i) {
			float y=sina[i], len=cosa[i];
			part2=part1+quarter;
			part3=part2+quarter;
			part4=part3+quarter;
			part1->set(len,y,0);
			part2->set(0,y,len);
			part3->set(-len,y,0);
			part4->set(0,y,-len);
			part2+=quarter;
			part4+=quarter;
			for(unsigned j=1; j<quarter; ++j) {
				float x=len*cosa[j], z=len*sina[j];
				++part1;
				part1->set(x,y,z);
				--part2;
				part2->set(-x,y,z);
				++part3;
				part3->set(-x,y,-z);
				--part4;
				part4->set(x,y,-z);
			}
			part1=part4+quarter-1;
		}
		// bottom half of sphere
		part1=buffer+1;
		part2=buffer+vertexCount-1;
		part2->set(0,-1,0);
		for(unsigned i=quarter-1; i>0; --i) {
			part2-=circle;
			for(unsigned j=0; j<circle; ++j, ++part1) 
				part2[j].set(part1->x,-part1->y,part1->z);
		}
		assert(unsigned(part2-part1)==circle);
		// vertex buffer
		GLuint vertbuffs[2];
		glGenBuffers(2,vertbuffs);
		glBindBuffer(GL_ARRAY_BUFFER,vertbuffs[0]);
		glBufferData(GL_ARRAY_BUFFER,vertexCount*sizeof(xvec3f_t),buffer,GL_STATIC_DRAW);
		delete [] buffer;
		delete [] sina;
		delete [] cosa;
		// index buffer
		unsigned indexCount=48*quarter*quarter-24*quarter;
		assert(indexCount<65536);
		GLushort *index=new GLushort[indexCount];
		GLushort *iter=index;
		GLushort vidx=1;
		while(vidx<circle) {
			*iter++=0;
			*iter++=vidx+1;
			*iter++=vidx;
			++vidx;
		}
		*iter++=0;
		*iter++=1;
		*iter++=vidx;
		vidx=1;
		for(unsigned i=2*quarter-2; i>0; --i) {
			unsigned end=vidx+circle-1;
			while(vidx<end) {
				iter[0]=vidx;
				iter[1]=vidx+1;
				iter[2]=GLushort(vidx+circle);
				iter[3]=iter[2];
				iter[4]=iter[1];
				iter[5]=GLushort(iter[1]+circle);
				iter+=6;
				++vidx;
			}
			iter[0]=vidx;
			iter[1]=GLushort(vidx+1-circle);
			iter[2]=GLushort(vidx+circle);
			iter[3]=iter[2];
			iter[4]=iter[1];
			iter[5]=vidx+1;
			iter+=6;
			++vidx;
		}
		GLushort last=GLushort(vertexCount-1);
		for(unsigned i=1; i<circle; ++i) {
			*iter++=last;
			*iter++=vidx;
			*iter++=vidx+1;
			++vidx;
		}
		*iter++=last;
		*iter++=vidx;
		*iter++=GLushort(vidx+1-circle);
		assert(unsigned(iter-index)==indexCount);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vertbuffs[1]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,indexCount*sizeof(GLushort),index,GL_STATIC_DRAW);
		delete [] index;
		
		xvertex_buffer *pvb;
		if(GLEW_ARB_vertex_array_object) 
		{
			GLuint vao;
			glGenVertexArrays(1,&vao);
			glBindVertexArray(vao);

			glBindBuffer(GL_ARRAY_BUFFER,vertbuffs[0]);
			glVertexAttribPointer(USAGE_POSITION,3,GL_FLOAT,GL_FALSE,0,(void*)0);
			glEnableVertexAttribArray(USAGE_POSITION);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vertbuffs[1]);

			glBindVertexArray(0);

			pbatch=(xvertex_batch*)m_groups[GROUP_BATCH]->create(batchid,(void*)vao);
			pvb=wycnew xvertex_buffer;
			pvb->create(vertbuffs[0]);
			pbatch->add_buffer(pvb);
		}
		else {
			pbatch=(xvertex_batch*)m_groups[GROUP_BATCH]->create(batchid,0);
			pvb=wycnew xvertex_buffer;
			pvb->create(vertbuffs[0]);
			pbatch->add_buffer(pvb,USAGE_POSITION,3,GL_FLOAT,false,0,0);
		}
		pvb=wycnew xvertex_buffer;
		pvb->create(vertbuffs[1]);
		pbatch->set_index(pvb,GL_UNSIGNED_SHORT);
		pbatch->set_mode(GL_TRIANGLES,indexCount,0);
	} // if (pbatch==0)

	if(pNotifier) {
		pev=xpackev::pack("o",pbatch);
		pNotifier->on_event(evid,pev);
	}
}

void xglrenderer::ev_default_texture(xobjevent *pev)
{
	xobject *pNotifier;
	int evid;
	if(!((xpackev*)pev)->unpack("od",&pNotifier,&evid)) {
		wyc_error("xglrenderer::ev_default_texture: bad args");
		return;
	}
	// DEFAULT = 0xE35E00DF,
	xtexture *ptex=(xtexture*)m_groups[GROUP_TEXTURE]->request(0xE35E00DF);
	if(!ptex) {
		ptex=(xtexture*)m_groups[GROUP_TEXTURE]->create(0xE35E00DF,"default");
		if(!ptex)
			return;
	}
	if(pNotifier) {
		pev=xpackev::pack("o",ptex);
		pNotifier->on_event(evid,pev);
	}
}

void xglrenderer::ev_load_texture(xobjevent *pev)
{
	xobject *pNotifier;
	int evid;
	const char *pfile;
	if(!((xpackev*)pev)->unpack("ods",&pNotifier,&evid,&pfile)) {
		wyc_error("xglrenderer::load_texture: bad args");
		return;
	}
	xresbase *ptex=get_texture(pfile);
	if(pNotifier) {
		pev=xpackev::pack("o",ptex);
		pNotifier->on_event(evid,pev);
	}
}


}; // namespace wyc

