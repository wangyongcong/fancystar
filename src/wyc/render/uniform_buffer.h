#ifndef __HEADER_WYC_UNIFORM_BUFFER
#define __HEADER_WYC_UNIFORM_BUFFER

#include "wyc/util/util.h"
#include "wyc/util/hash.h"
#include "wyc/math/vecmath.h"

namespace wyc
{

struct xuniform_entry_info
{
	GLuint m_index;
	GLuint m_offset;
	GLenum m_type;
	GLuint m_array_size;
	GLuint m_array_stride;
	GLuint m_matrix_stride;
	GLuint m_row_major;
};

struct xuniform_block_info
{
	const char *m_name;
	unsigned m_binding;
	unsigned m_offset;
	unsigned m_size;
	const char **m_members;
	unsigned m_member_count;
	xuniform_entry_info m_member_info[1];
};

class xuniform_buffer
{
	xdict m_blocks;
	xdict m_uniforms;
	GLuint m_ubo;
	void *m_mapped_buffer;
	size_t m_buffer_size;
public:
	xuniform_buffer();
	~xuniform_buffer();
	void clear();
	void reserve(unsigned block_count, unsigned entry_count);
	bool append_block(const char *block_name, unsigned member_count, const char **member_names, unsigned binding_point);
	bool bind_program(GLuint program, const char *block_name);
	void create_buffer();
	void commit();

	bool set_uniform(const char *name, unsigned ival);
	bool set_uniform(const char *name, float ival);
	bool set_uniform(const char *name, const xvec3f_t vec3);
	bool set_uniform(const char *name, const xvec4f_t vec4);
	bool set_uniform(const char *name, const xmat3f_t mat3);
	bool set_uniform(const char *name, const xmat4f_t mat4);
private:
	bool querry_block_info(GLuint program, xuniform_block_info *ubi);
};

} // namespace wyc

#endif // __HEADER_WYC_UNIFORM_BUFFER

