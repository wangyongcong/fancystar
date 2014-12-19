#ifndef __HEADER_WYC_XCAMERA
#define __HEADER_WYC_XCAMERA

#include "wyc/math/vecmath.h"
#include "wyc/world/gameobj.h"

namespace wyc
{

class xcamera : public xgameobj
{
	USE_RTTI;
	xmat4f_t m_mat_projection;
	xmat4f_t m_mat_mvp;
public:
	xcamera();
	/// 设置正交投影参数
	void set_orthograph(float xmin, float ymin, float zmin, float xmax, float ymax, float zmax);
	/**	@brief 设置透视参数
		@param yfov 垂直视锥角
		@param aspect 宽高比
		@param fnear 近裁剪面离原点的距离，必须大于0
		@param ffar 远裁剪面离原点的距离，必须大于fnear
	*/
	void set_perspective(float yfov, float aspect, float fnear, float ffar);
	/// 设置相机朝向
	void look_at(const xvec3f_t &target, const xvec3f_t &up);
	/// 取相机矩阵
	const xmat4f_t& get_projection() const;
	const xmat4f_t& get_mvp_matrix() const;
	/// 设置UI变换
	void set_ui_view(float screen_width, float screen_height, float z_range=256);
};

inline void xcamera::look_at(const xvec3f_t &target, const xvec3f_t &up)
{
	xvec3f_t forward=m_transform->position()-target;
	m_transform->set_forward(forward,up);
}

inline const xmat4f_t& xcamera::get_projection() const
{
	return m_mat_projection;
}

inline const xmat4f_t& xcamera::get_mvp_matrix() const
{
	return m_mat_mvp;
}


}; // namespace wyc

#endif // end of __HEADER_WYC_XCAMERA

