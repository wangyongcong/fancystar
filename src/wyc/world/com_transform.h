#ifndef __HEADER_WYC_COM_TRANSFORM
#define __HEADER_WYC_COM_TRANSFORM

#include "wyc/math/vecmath.h"
#include "wyc/world/component.h"

namespace wyc
{

class xcom_transform : public xcomponent
{
	USE_RTTI;
	xvec3f_t m_position;
	xvec3f_t m_forward, m_up, m_right;
	xvec3f_t m_scale;
	xmat4f_t m_local2world;
	xmat4f_t m_world2local;
	enum {
		TRANSLATE=0x11,
		ROTATE=0x22,
		LOCAL_2_WORLD=0x0F,
		WORLD_2_LOCAL=0xF0,
	};
	unsigned m_flag;
public:
	xcom_transform();
	bool update(xcom_transform *parent_trans=0, bool rebuild=false);
	// 朝向
	inline void set_orientation(const xvec3f_t &forward, const xvec3f_t &up, const xvec3f_t &right) {
		m_forward=forward;
		m_up=up;
		m_right=right;
		m_flag|=ROTATE;
	}
	void set_forward(const xvec3f_t &forward, const xvec3f_t &up);
	inline const xvec3f_t& forward() const {
		return m_forward;
	}
	inline const xvec3f_t& up() const {
		return m_up;
	}
	inline const xvec3f_t& right() const {
		return m_right;
	}
	// 平移
	inline void set_position(const xvec3f_t &pos) {
		m_position=pos;
		m_flag|=TRANSLATE;
	}
	inline void set_position(float x, float y, float z) {
		m_position.set(x,y,z);
		m_flag|=TRANSLATE;
	}
	inline const xvec3f_t& position() const {
		return m_position;
	}
	inline void translate(float forward, float up, float right) {
		m_position+=m_forward*forward;
		m_position+=m_up*up;
		m_position+=m_right*right;
		m_flag|=TRANSLATE;
	}
	inline void translate_forward(float d) {
		m_position+=m_forward*d;
		m_flag|=TRANSLATE;
	}
	inline void translate_up(float d) {
		m_position+=m_up*d;
		m_flag|=TRANSLATE;
	}
	inline void translate_right(float d) {
		m_position+=m_right*d;
		m_flag|=TRANSLATE;
	}
	// 旋转
	void rotate(const xvec3f_t &axis, float angle);
	void rotate_forward(float angle);
	void rotate_up(float angle);
	void rotate_right(float angle);
	// 缩放
	inline void scale(float x, float y, float z) {
		m_scale.set(x,y,z);
		m_flag|=ROTATE;
	}
	inline void scale(float s) {
		m_scale.set(s,s,s);
		m_flag|=ROTATE;
	}
	inline void scale(const xvec3f_t &scale) {
		m_scale=scale;
		m_flag|=ROTATE;
	}
	inline const xvec3f_t& scaling () const {
		return m_scale;
	}
	// 变换
	inline const xmat4f_t& local2world() {
		return m_local2world;
	}
	const xmat4f_t& world2local();
private:
	void rebuild_local2world();
	void rebuild_world2local();
};

}; // namespace wyc

#endif // __HEADER_WYC_COM_TRANSFORM

