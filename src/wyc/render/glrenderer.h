#ifndef __HEADER_WYC_GLRENDERER
#define __HEADER_WYC_GLRENDERER

#include "wyc/thread/thread.h"
#include "wyc/thread/asyncqueue.h"
#include "wyc/thread/tls.h"
#include "wyc/util/hash.h"
#include "wyc/util/util.h"
#include "wyc/math/vecmath.h"
#include "wyc/obj/object.h"
#include "wyc/obj/ressvr.h"
#include "wyc/render/uniform_buffer.h"

namespace wyc 
{

enum SHADER_USAGE {
	USAGE_POSITION = 0,
	USAGE_COLOR, 
	USAGE_NORMAL,
	USAGE_TANGENT,
	USAGE_TEXTURE0, 
	USAGE_TEXTURE1,
	USAGE_TEXTURE2, 
	USAGE_TEXTURE3,
	USAGE_CUSTOM0,
	USAGE_CUSTOM1,
	USAGE_MAX=16,
	USAGE_UNIFORM,
	USAGE_UNIFORM_BLOCK,
};

struct xshader_attribute
{
	int m_attr;
	const char *m_name;
};

class xshader 
{
	std::string m_name;
	unsigned m_sid;
	GLuint m_program;
	xshader_attribute *m_argv;
	unsigned m_args;
	unsigned m_uniform_idx;
public:
	xshader(const char *name, GLuint program, unsigned args, \
		xshader_attribute *argv, unsigned uniform_idx) : m_name(name) {
		m_sid=strhash(name);
		assert(m_sid);
		assert(program);
		m_program=program;
		assert(argv);
		assert(args);
		m_argv=argv;
		m_args=args;
		assert(uniform_idx<=m_args);
		m_uniform_idx=uniform_idx;
	}
	inline GLuint handle() const {
		return m_program;
	}
	inline const std::string& name() const {
		return m_name;
	}
	inline unsigned shader_id() const {
		return m_sid;
	}
	GLint get_uniform(const char *uniform_name) const {
		for(unsigned i=m_uniform_idx; i<m_args; ++i) {
			if(strcmp(m_argv[i].m_name,uniform_name)==0)
				return m_argv[i].m_attr;
		}
		return -1;
	}
};

class xrenderobj;
class xrenderlist;

#define RENDER_PRIORITY(priority) (0x80000000|(priority&0xFFFF))

class xglrenderer : public xobject
{
	USE_EVENT_MAP;
	static int ms_glMajorVersion, ms_glMinorVersion;
	static int ms_glslMajorVersion, ms_glslMinorVersion;
	static int ms_pixelfmt;
	// windows system
	HWND m_hWnd;
	HDC m_hDC;
	HGLRC m_hRC;
	PIXELFORMATDESCRIPTOR m_pfd;
	// GLEW
	GLEWContext m_glewctx;
	WGLEWContext m_wglewctx;
	// resource path
	std::string m_shader_path;
	// shader program management
	xdict m_shader_lib;
	xdict m_shader_source;
	xshader *m_cur_shader;
	// shader uniform management
	xuniform_buffer m_uniform_buffer;
	GLuint m_block_binding;
	enum GROUP_ID {
		GROUP_VERTEXBUFFER=0,
		GROUP_BATCH,
		GROUP_TEXTURE,
		GROUP_COUNT,
	};
	GLsync m_fence;
	xrenderlist *m_pRenderList;
public:
	xglrenderer();
	bool initialize();
	void terminate();
	virtual void update(double accumulateTime, double frameTime);
	void flush();
	void swap_buffer();
	void wait_for_finish();

	//-----------------------------------------------------
	// shader program interface
	//-----------------------------------------------------
	void set_shader_path(const std::string &path);
	const std::string& shader_path() const;
	bool use_shader(unsigned shaderID);
	unsigned get_shader_id() const;
	const xshader* get_shader() const;
	xuniform_buffer* uniform_buffer();
	
	//-----------------------------------------------------
	// internal functions
	//-----------------------------------------------------
	GLEWContext* glew_context();
	WGLEWContext* wglew_context();
	static bool check_driver();
	static bool initialize_driver(HWND hTmpWnd, const char *profile);
	static xglrenderer* create_renderer(HWND hTargetWindow);

private:
	void initialize_shaders();
	bool load_shader(const char *shader_name, unsigned args, xshader_attribute *argv,\
		const char *vs_name=0, const char *fs_name=0, const char *gs_name=0);
	GLuint create_shader(GLenum shader_type, const char *sorce_name);
	void clear_source();
	bool new_uniform_block(const char *name, unsigned member_count, const char **members);
	static const char* load_source(const char *filename);
	static void  free_source(const char *psrc);
	static bool compile_shader(GLuint shader);
	static bool link_shader(GLuint program);
};

inline bool xglrenderer::check_driver() 
{
	return ms_pixelfmt>=0;
}

inline unsigned xglrenderer::get_shader_id() const
{
	return m_cur_shader?m_cur_shader->shader_id():0;
}

inline const xshader* xglrenderer::get_shader() const
{
	return m_cur_shader;
}

inline GLEWContext* xglrenderer::glew_context()
{
	return &m_glewctx;
}

inline WGLEWContext* xglrenderer::wglew_context()
{
	return &m_wglewctx;
}

inline void xglrenderer::set_shader_path(const std::string &path) {
	m_shader_path = path;
}

inline const std::string& xglrenderer::shader_path() const {
	return m_shader_path;
}

inline xuniform_buffer* xglrenderer::uniform_buffer() {
	return &m_uniform_buffer;
}

#define BEGIN_SHADER(shader_name) \
	wyc::xshader_attribute __argv_##shader_name[] = {

#define END_SHADER() };

#define USE_SHADER(shader_name, vs_name, fs_name, gs_name) \
	load_shader(#shader_name,sizeof(__argv_##shader_name)/sizeof(wyc::xshader_attribute),__argv_##shader_name,vs_name,fs_name,gs_name);
	
#define BEGIN_UNIFORM_BLOCK(uniform_name) \
	const char *__ub_##uniform_name[] = {

#define END_UNIFORM_BLOCK() };

#define REGISTER_UNIFORM_BLOCK(uniform_name) \
	m_uniform_buffer.append_block(#uniform_name,sizeof(__ub_##uniform_name)/sizeof(const char*),__ub_##uniform_name,m_block_binding++)

}; // namespace wyc


#endif // __HEADER_WYC_GLRENDERER

