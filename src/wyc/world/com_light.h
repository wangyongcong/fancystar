#ifndef __HEADER_WYC_COM_LIGHT
#define __HEADER_WYC_COM_LIGHT

#include "wyc/world/component.h"
#include "wyc/math/vecmath.h"

namespace wyc
{

class xcom_light : public xcomponent
{
	USE_RTTI;
	float m_intensity;
	xvec3f_t m_color;
public:
	xcom_light();
	void set_intensity(float i) {
		m_intensity = i;
	}
	const float intensity() const {
		return m_intensity;
	}
	void set_color(const xvec3f_t &c) {
		m_color = c;
	}
	void set_color(float r, float g, float b) {
		m_color.set(r,g,b);
	}
	const xvec3f_t& color() const {
		return m_color;
	}
	void apply_lighting(int idx, xvec3f_t *pos, xvec3f_t *intensity);
};

}; // namespace wyc


#endif // __HEADER_WYC_COM_LIGHT

