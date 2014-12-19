#include "fscorepch.h"
#include "wyc/world/camera.h"

namespace wyc
{

REG_RTTI(xcamera,xgameobj)

xcamera::xcamera()
{
	add_component<xcom_transform>();
	m_transform->set_orientation(xvec3f_t(0,0,-1),xvec3f_t(0,1,0),xvec3f_t(1,0,0));
	m_mat_projection.identity();
	m_mat_mvp.identity();
}

void xcamera::set_orthograph(float xmin, float ymin, float zmin, float xmax, float ymax, float zmax)
{
	m_mat_projection.identity();
	m_mat_projection.m00=2.0f/(xmax-xmin);
	m_mat_projection.m11=2.0f/(ymax-ymin);
	m_mat_projection.m22=-2.0f/(zmax-zmin);
	m_mat_projection.m03=-(xmax+xmin)/(xmax-xmin);
	m_mat_projection.m13=-(ymax+ymin)/(ymax-ymin);
	m_mat_projection.m23=-(zmax+zmin)/(zmax-zmin);
}

void xcamera::set_perspective(float fov, float aspect, float fnear, float ffar)
{
	if(fnear<0)
		fnear=-fnear;
	if(ffar<0)
		ffar=-ffar;
	if(fnear>ffar) 
		std::swap(fnear,ffar);
	float xmin, xmax, ymin, ymax;
	ymax=fnear*tan(DEG_TO_RAD(fov));
	ymin=-ymax;
	xmax=ymax*aspect;
	xmin=-xmax;
	m_mat_projection.m00=2*fnear/(xmax-xmin);
	m_mat_projection.m01=0;
	m_mat_projection.m02=(xmax+xmin)/(xmax-xmin);
	m_mat_projection.m03=0;
	m_mat_projection.m10=0;
	m_mat_projection.m11=2*fnear/(ymax-ymin);
	m_mat_projection.m12=(ymax+ymin)/(ymax-ymin);
	m_mat_projection.m13=0;
	m_mat_projection.m20=0;
	m_mat_projection.m21=0;
	m_mat_projection.m22=(ffar+fnear)/(fnear-ffar);
	m_mat_projection.m23=2*ffar*fnear/(fnear-ffar);
	m_mat_projection.m30=0;
	m_mat_projection.m31=0;
	m_mat_projection.m32=-1;
	m_mat_projection.m33=0;
}

void xcamera::set_ui_view(float screen_width, float screen_height, float z_range)
{
	set_orthograph(0,0,0,screen_width,screen_height,z_range*2);
	m_transform->set_position(0,0,z_range);
	m_transform->set_forward(xvec3f_t(0,0,-z_range),xvec3f_t(0,1,0));
	update_transform();
	wyc::xmat4f_t mat_ui;
	mat_ui.identity();
	mat_ui.set_row(1,wyc::xvec4f_t(0,-1,0,screen_height));
	mat_ui.set_row(2,wyc::xvec4f_t(0,0,1,-z_range));
	m_mat_mvp.mul(m_mat_projection,mat_ui);
}


}; // namespace wyc
