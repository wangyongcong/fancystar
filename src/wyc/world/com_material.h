#ifndef __HEADER_WYC_COM_MATERIAL
#define __HEADER_WYC_COM_MATERIAL

#include "wyc/render/renderer.h"
#include "wyc/world/component.h"
#include "wyc/render/texture.h"

namespace wyc
{

class xcom_material : public xcomponent
{
	USE_RTTI;
protected:
	unsigned m_shaderID;
public:
	xcom_material();
	virtual void on_destroy();
	virtual bool apply(xrenderer *rd);
};

class xmtl_diffuse : public xcom_material
{
	USE_RTTI;
	xvec4f_t m_diffuse, m_specular;
	float m_smoothness;
	xpointer<xtexture> m_diffuse_map;
public:
	xmtl_diffuse() 
	{
		m_shaderID=strhash("diffuse");
	}
	virtual bool apply(xrenderer *rd) {
		if(!xcom_material::apply(rd))
			return false;
		const xshader *sh = rd->get_shader();
		GLint uf = sh->get_uniform("mtl_diffuse");
		glUniform3fv(uf,1,m_diffuse.elem);
		uf = sh->get_uniform("mtl_specular");
		glUniform3fv(uf,1,m_specular.elem);
		uf = sh->get_uniform("mtl_smoothness");
		glUniform1f(uf,m_smoothness);
		return true;
	}
	void set_diffuse_color(float r, float g, float b, float a) {
		m_diffuse.set(r,g,b,a);
	}
	void set_diffuse_color(const xvec4f_t &c) {
		m_diffuse = c;
	}
	const xvec4f_t& get_diffuse_color() const {
		return m_diffuse;
	}
	void set_specular_color(float r, float g, float b, float a) {
		m_specular.set(r,g,b,a);
	}
	void set_specular_color(const xvec4f_t &c) {
		m_specular = c;
	}
	const xvec4f_t& get_specular_color() const {
		return m_specular;
	}
	void set_diffuse_map(xtexture *tex) {
		m_diffuse_map = tex;
	}
	xtexture* get_diffuse_map() {
		return m_diffuse_map;
	}
	void set_smoothness(float sm) {
		m_smoothness = sm;
	}
	float get_smoothness() const {
		return m_smoothness;
	}
};

}; // namespace wyc

#endif // __HEADER_WYC_COM_MATERIAL
