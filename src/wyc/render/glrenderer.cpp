#include "fscorepch.h"
#include <fstream>
#include "wyc/util/time.h"
#include "wyc/thread/asyncqueue.h"
#include "wyc/render/renderobj.h"
#include "wyc/render/glrenderer.h"

namespace wyc
{

BEGIN_EVENT_MAP(xglrenderer,xobject)

END_EVENT_MAP

int xglrenderer::ms_glMajorVersion=1; 
int xglrenderer::ms_glMinorVersion=0;
int xglrenderer::ms_glslMajorVersion=1; 
int xglrenderer::ms_glslMinorVersion=10;
int xglrenderer::ms_pixelfmt=-1;

static PFNWGLCREATECONTEXTATTRIBSARBPROC s_pfnWGLCreateContextAttribsARB=0;

bool extension_available(const char *extname)
{
	GLint nNum;
	glGetIntegerv(GL_NUM_EXTENSIONS, &nNum);
	for(GLint i = 0; i< nNum; i++) {
		if(strcmp(extname,(const char*)glGetStringi(GL_EXTENSIONS,i))==0)
		{
			return true;
		}
	}
	return false;
}

bool enable_extensions()
{
	GLenum err;
	err=glewInit();
	if(GLEW_OK!=err) {
		wyc_error("[GL] failed to init GL extensions: %s",(const char*)glewGetErrorString(err));
		return false;
	}
	err=wglewInit();
	if(GLEW_OK!=err) {
		wyc_error("[GL] failed to WGL extensions: %s",(const char*)glewGetErrorString(err));
		return false;
	}
	wyc_sys("[GL] GLEW version %s",glewGetString(GLEW_VERSION));
	return true;
}

xglrenderer::xglrenderer()
{
	m_hWnd=NULL;
	m_hDC=NULL;
	m_hRC=NULL;

	m_shader_path="shader";
	m_cur_shader=0;

	m_pRenderList=new xrenderlist;

	m_block_binding=0;
	m_fence = 0;
}

#pragma warning(push)
#pragma warning(disable:4996)
void get_version(const char *ver, int &major, int &minor)
{
	size_t sz=strlen(ver);
	char *pstr=new char[sz+1];
	strcpy(pstr,ver);
	const char *splitter=".";
	char *tok=strtok(pstr,splitter);
	if(tok) {
		major=atoi(tok);
		tok=strtok(0,splitter);
		if(tok) minor=atoi(tok);
	}
	delete [] pstr;
}
#pragma warning(pop)

bool xglrenderer::initialize_driver(HWND hTmpWnd, const char *config)
{
	if(ms_pixelfmt>=0)
		return true;
	if(NULL==hTmpWnd)
		return false;
	HDC hDC=::GetDC(hTmpWnd);
	if(NULL==hDC)
		return false;
	// load graphic config
	if(config) {
	}
	// 最简单的PFD, 避免出错
	::PIXELFORMATDESCRIPTOR pfd={
		sizeof(PIXELFORMATDESCRIPTOR),		// Size Of This Pixel Format Descriptor
		1,									// Version Number
		PFD_DRAW_TO_WINDOW |				// Format Must Support Window
		PFD_SUPPORT_COMPOSITION |			// Wordk with DWM 
		PFD_SUPPORT_OPENGL |				// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,					// Must Support Double Buffering
		PFD_TYPE_RGBA,						// Request An RGBA Format
		32,									// Select Our Color Depth
		0, 0, 0, 0, 0, 0,					// Color Bits Ignored
		8,									// 8-bit alpha
		0,									// Shift Bit Ignored
		0,									// No Accumulation Buffer
		0, 0, 0, 0,							// Accumulation Bits Ignored
		24,									// 24Bit Z-Buffer (Depth Buffer)  
		8,									// 8-bit Stencil Buffer
		0,									// No Auxiliary Buffer
		PFD_MAIN_PLANE,						// Main Drawing Layer
		0,									// Reserved
		0, 0, 0								// layer masks ignored 
	};
	if(!SetPixelFormat(hDC,1,&pfd))
		return false;
	HGLRC hRC=wglCreateContext(hDC);
	if(NULL==hRC)
		return false;
	wglMakeCurrent(hDC,hRC);

	const char *ver_gl=(const char*)glGetString(GL_VERSION);
	get_version(ver_gl,ms_glMajorVersion,ms_glMinorVersion);
	if(ms_glMajorVersion<2) {
		wyc_error("[GL] only support 2.0+");
		return false;
	}
	const char *ver_glsl=(const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
	get_version(ver_glsl,ms_glslMajorVersion,ms_glslMinorVersion);

	s_pfnWGLCreateContextAttribsARB=(PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
	PFNWGLCHOOSEPIXELFORMATARBPROC pfnWGLChoosePixelFormatARB=\
		(PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
	if(pfnWGLChoosePixelFormatARB==0) {
		wyc_error("[GL] no WGL_ARB_pixel_format");
		return false;
	}
	int pixel_attr[]={
		WGL_SUPPORT_OPENGL_ARB,	1,	// Must support OGL rendering
		WGL_DRAW_TO_WINDOW_ARB,	1,	// pf that can run a window
		WGL_RED_BITS_ARB,	8,	// bits of red precision in window
		WGL_GREEN_BITS_ARB,	8,	// bits of green precision in window
		WGL_BLUE_BITS_ARB,	8,	// bits of blue precision in window
		WGL_ALPHA_BITS_ARB,	8,	// bits of alpha precision in window
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB, // pf should be RGBA type
		WGL_DEPTH_BITS_ARB,		16,	// bits of depth precision for window
		WGL_STENCIL_BITS_ARB,	8,	// bits of precision for stencil buffer
		WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB, // must be HW accelerated
		WGL_DOUBLE_BUFFER_ARB, 1, // double buffer enabled
		WGL_SAMPLES_ARB,	0, // number of multisample samples per pixel
		0 // NULL termination
	};
	unsigned numfmt;
	if(!pfnWGLChoosePixelFormatARB(hDC,pixel_attr,0,1,&ms_pixelfmt,&numfmt) || ms_pixelfmt==-1) {
		wyc_error("[GL] can't find a proper pixel format");
		return false;
	}
	const char *vendor=(const char*)glGetString(GL_VENDOR);
	const char *renderer=(const char*)glGetString(GL_RENDERER);
	wyc_sys("[GL] %s",vendor);
	wyc_sys("[GL] %s",renderer);
	wyc_sys("[GL] OpenGL %s (GLSL %s)",ver_gl,ver_glsl);
	wglMakeCurrent(hDC,NULL);
	wglDeleteContext(hRC);
	ReleaseDC(hTmpWnd,hDC);
	return true;
}

xglrenderer* xglrenderer::create_renderer(HWND hTargetWindow)
{
	xglrenderer *pRenderer=wycnew xglrenderer;
	pRenderer->m_hWnd=hTargetWindow;
	return pRenderer;	
}

bool xglrenderer::initialize()
{
	if(NULL==m_hWnd) 
		return false;
	assert(m_hRC==NULL);
	HDC hDC=GetDC(m_hWnd);
	assert(hDC!=NULL);
	if(!SetPixelFormat(hDC,ms_pixelfmt,&m_pfd)) {
		ReleaseDC(m_hWnd,hDC);
		return false;
	}
	if(s_pfnWGLCreateContextAttribsARB) {
		// for OpenGL 3.0 or above
		int attributes[]={
			WGL_CONTEXT_MAJOR_VERSION_ARB, ms_glMajorVersion,
			WGL_CONTEXT_MINOR_VERSION_ARB, ms_glMinorVersion,
	#ifndef GL_COMPAT
			// 只兼容OpenGL Core, 去除所有deprecated, 适用于OpenGL 3.2 (或更高)
			// TODO：GLEW在该模式下不可用,因为glewGetExtension使用旧的
			// glGetString(GL_EXTENSIONS) 来获取扩展的名字
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB, 
	#else
			// 兼容3.0之前的版本
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB, 
	#endif // GL_COMPAT
	#ifdef _DEBUG
			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB, // 开启debug模式
	#endif // _DEBUG
			0,
		};
		m_hRC=s_pfnWGLCreateContextAttribsARB(hDC,NULL,attributes);
	}
	else {
		m_hRC=::wglCreateContext(hDC);
	}
	if(m_hRC==NULL) {
		ReleaseDC(m_hWnd,hDC);
		wyc_error("[GL] failed to create context");
		return false;
	}
	m_hDC=hDC;
	wglMakeCurrent(m_hDC,m_hRC);
	// initialize OpenGL extensions 
	if(!enable_extensions())
		return false;
	// 开启/关闭垂直同步
	// TODO: 从文件读取配置
	if(WGLEW_EXT_swap_control) {
		wglSwapIntervalEXT(0);
	}
	RECT rect;
	GetClientRect(m_hWnd,&rect);
	wyc_print("viewport: (%d,%d,%d,%d)",0,0,rect.right-rect.left,rect.bottom-rect.top);
	glViewport(0,0,rect.right-rect.left,rect.bottom-rect.top);

	// initialize shaders
	// TODO: shader program may be shared across threads?
	initialize_shaders();
	clear_source();
	m_uniform_buffer.create_buffer();
	return true;
}

void xglrenderer::terminate()
{
	m_pRenderList->clear();
	delete m_pRenderList;
	m_pRenderList=0;
	xdict::iterator iter=m_shader_lib.begin(), end=m_shader_lib.end();
	for(; iter!=end; ++iter) {
		glDeleteProgram((uintptr_t)(iter->second));
	}
	m_shader_lib.clear();
	m_uniform_buffer.clear();
	if(m_hRC) {
		wglMakeCurrent(m_hDC,NULL);
		wglDeleteContext(m_hRC);
		ReleaseDC(m_hWnd,m_hDC);
		m_hWnd=NULL;
		m_hDC=NULL;
		m_hRC=NULL;
	}
}

const char* xglrenderer::load_source(const char *fname)
{
	std::fstream fs;
	fs.open(fname,std::ios_base::in);
	if(!fs.is_open()) {
		wyc_warn("[GL] can't open file: %s",fname);
		return 0;
	}
	fs.seekg(0,std::ios_base::end);
	std::streamoff size=fs.tellg();
	char *src=new char[size+1];
	fs.seekg(0);
	fs.read(src,size);
	std::streamoff  cnt=fs.gcount();
	if(size<cnt) {
		fs.close();
		delete [] src;
		wyc_error("[GL] read file error");
		return 0;
	}
	src[cnt]=0;
	fs.close();
	return src;
}

void xglrenderer::free_source(const char *src)
{
	delete [] src;
}

bool xglrenderer::compile_shader(GLuint shader)
{
	glCompileShader(shader);
	GLint ret;
	::glGetShaderiv(shader,GL_COMPILE_STATUS,&ret);
	if(ret==GL_FALSE) {
		glGetShaderiv(shader,GL_INFO_LOG_LENGTH,&ret);
		if(ret<1) {
			wyc_error("[GL] shader compile error: Unknown");
		}
		else {
			ret+=1;
			char *plog=new char[ret];
			glGetShaderInfoLog(shader,ret,&ret,plog);
			wyc_error("[GL] shader compile error:");
			wyc_error(plog);
			delete [] plog;
		}
		return false;
	}
	return true;
}

bool xglrenderer::link_shader(GLuint program)
{
	glLinkProgram(program);
	GLint ret;
	::glGetProgramiv(program,GL_LINK_STATUS,&ret);
	if(ret==GL_FALSE) {
		glGetShaderiv(program,GL_INFO_LOG_LENGTH,&ret);
		if(ret<1) {
			wyc_error("[GL] shader link error: Unknown");
		}
		else {
			ret+=1;
			char *plog=new char[ret];
			glGetShaderInfoLog(program,ret,&ret,plog);
			wyc_error("[GL] shader link error:");
			wyc_error(plog);
			delete [] plog;
		}
		return false;
	}
	return true;
}


struct predicate_is_shader_attribute
{
	bool operator() (const xshader_attribute &attr) const
	{
		return attr.m_attr<USAGE_UNIFORM;
	}
};

struct predicate_is_shader_uniform
{
	bool operator() (const xshader_attribute &attr) const
	{
		return attr.m_attr==USAGE_UNIFORM;
	}
};

GLuint xglrenderer::create_shader(GLenum shader_type, const char *sorce_name)
{
	std::string fname=m_shader_path;
	fname+="/";
	fname+=sorce_name;
	unsigned gid=strhash(fname.c_str());
	void *hf = m_shader_source.get(gid);
	if(hf) 
		return (GLuint)hf;
	wyc_sys("[GL] loading [%s]...",fname.c_str());
	const GLchar *src[1];
	src[0]=load_source(fname.c_str());
	if(!src[0])
		return 0;
	GLuint hShader=glCreateShader(shader_type);
	if(hShader==0) {
		free_source(src[0]);
		return 0;
	}
	glShaderSource(hShader,1,src,0);
	free_source(src[0]);
	if(!compile_shader(hShader)) {
		glDeleteShader(hShader);
		return 0;
	}
	m_shader_source.add(gid,xdict::value_t(hShader));
	return hShader;
}

bool xglrenderer::load_shader(const char *shader_name, unsigned args, xshader_attribute *argv, \
							  const char *vs_name, const char *fs_name, const char *gs_name)
{
	GLenum glerr;
	GLuint hVertexShader=0, hFragmentShader=0, hGeometryShader=0;
	wyc_sys("[GL] init shader [%s]...",shader_name);
	wyc::xcode_timer ct;
	ct.start();
	// vertex shader
	if(vs_name && vs_name[0]) {
		hVertexShader = create_shader(GL_VERTEX_SHADER,vs_name);
		if(!hVertexShader)
			return false;
	}
	// fragment shader
	if(fs_name && fs_name[0]) {
		hFragmentShader = create_shader(GL_FRAGMENT_SHADER,fs_name);
		if(!hFragmentShader)
		{
			if(hVertexShader)
				glDeleteShader(hVertexShader);
			return false;
		}
	}
	// geometry shader
	if(gs_name && gs_name[0])
	{
		hGeometryShader = create_shader(GL_GEOMETRY_SHADER,gs_name);
		if(!hGeometryShader)
		{
			if(hVertexShader)
				glDeleteShader(hVertexShader);
			if(hFragmentShader)
				glDeleteShader(hFragmentShader);
			return false;
		}
	}
	// link shader program
	GLuint program=glCreateProgram();
	if(hVertexShader) 
		glAttachShader(program,hVertexShader);
	if(hFragmentShader) 
		glAttachShader(program,hFragmentShader);
	if(hGeometryShader)
		glAttachShader(program,hGeometryShader);
	// bind attribute location
	unsigned uniform_index, uniform_block;
	uniform_index=arrange(argv,0,args,predicate_is_shader_attribute());
	uniform_block=arrange(argv,uniform_index,args,predicate_is_shader_uniform());
	for(unsigned i=0; i<uniform_index; ++i) {
		glBindAttribLocation(program,argv[i].m_attr,argv[i].m_name);
		glerr=glGetError();
		if(GL_NO_ERROR!=glerr) 
			wyc_error("[GL] bind shader attribute failed: %d, %s",argv[i].m_attr,argv[i].m_name);
	}
	if(!link_shader(program)) {
		glDeleteProgram(program);
		program=0;
		return false;
	}
	// bind uniform location
	for(unsigned i=uniform_index; i<uniform_block; ++i) {
		assert(argv[i].m_attr==USAGE_UNIFORM);
		argv[i].m_attr=glGetUniformLocation(program,argv[i].m_name);
		if(argv[i].m_attr==-1) 
			wyc_error("[GL] shader uniform not found: %s",argv[i].m_name);
	}
	// initialize uniform block
	for(unsigned i=uniform_block; i<args; ++i) {
		assert(argv[i].m_attr==USAGE_UNIFORM_BLOCK);
		if(!m_uniform_buffer.bind_program(program,argv[i].m_name)) {
			wyc_warn("[GL] uniform block [%s] is not registered",argv[i].m_name);
			continue;
		}
	}
	ct.stop();
	glerr=glGetError();
	if(GL_NO_ERROR!=glerr) {
		wyc_error("[GL] some error may occurred [%d]",glerr);
		glDeleteProgram(program);
		return false;
	}
	wyc_sys("[GL] [%s] OK: %.4f",shader_name,ct.get_time());
	xshader *pShader=new xshader(shader_name, program, args, argv, uniform_index);
	m_shader_lib.add(pShader->shader_id(),pShader);
	return true;
}

void xglrenderer::clear_source()
{
	xdict::iterator iter, end;
	for(iter=m_shader_source.begin(), end=m_shader_source.end();
		iter!=end; ++iter) {
			glDeleteShader(GLuint(iter->second));
	}
	m_shader_source.clear();
}

bool xglrenderer::use_shader(unsigned shaderID)
{
	if(!m_cur_shader || m_cur_shader->shader_id()!=shaderID) {
		if(shaderID==0)
		{
			m_cur_shader=0;
			glUseProgram(0);
			return true;
		}
		xshader *pShader=(xshader*)m_shader_lib.get(shaderID);
		if(!pShader)
			return false;
		m_cur_shader=pShader;
		glUseProgram(m_cur_shader->handle());
	}
	return true;
}

void xglrenderer::update(double, double)
{
	glClearColor(0,0,0,1);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	m_uniform_buffer.commit();
	m_pRenderList->draw(this);
}

void xglrenderer::flush()
{
	glFlush();
}

void xglrenderer::swap_buffer()
{
	SwapBuffers(m_hDC);
//	if(m_fence)
//		glDeleteSync(m_fence);
//	m_fence=glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE,0);
}

void xglrenderer::wait_for_finish()
{
/*	if(!m_fence)
		return 0;
	GLint status;
	glGetSynciv(m_fence,GL_SYNC_STATUS,1,0,&status);
	double max_wait=0;
	while(GL_SIGNALED!=status)
	{
		Sleep(1);
		glGetSynciv(m_fence,GL_SYNC_STATUS,1,0,&status);
	}
*/
	glFinish();
}


}; // namespace wyc
