#include "fscorepch.h"
#include "wyc/world/gameobj.h"
#include "wyc/world/com_transform.h"

namespace wyc
{

REG_RTTI(xcom_transform,xcomponent)

xcom_transform::xcom_transform()
{
	m_position.zero();
	m_forward.set(0,0,1);
	m_up.set(0,1,0);
	m_right.set(1,0,0);
	m_scale.set(1,1,1);
	m_local2world.identity();
	m_world2local.identity();
	m_flag=0;

}

bool xcom_transform::update(xcom_transform *parent_trans, bool rebuild)
{
	if(rebuild || have_state(m_flag,LOCAL_2_WORLD)) {
		rebuild_local2world();
		add_state(m_flag,WORLD_2_LOCAL);
		if(parent_trans) 
			m_local2world=parent_trans->local2world()*m_local2world;
		return true;
	}
	return false;
}

void xcom_transform::rebuild_local2world()
{
//	wyc_print("[%s] rebuild local2world",gameobj()->get_name().c_str());
	m_local2world.set_col(0,m_right*m_scale.x);
	m_local2world.set_col(1,m_up*m_scale.y);
	m_local2world.set_col(2,m_forward*m_scale.z);
	m_local2world.set_col(3,m_position);
	remove_state(m_flag,LOCAL_2_WORLD);
}

void xcom_transform::rebuild_world2local()
{
//	wyc_print("[%s] rebuild world2local",gameobj()->get_name().c_str());
	m_world2local.set_row(0,m_right*(1.0f/m_scale.x));
	m_world2local.set_row(1,m_up*(1.0f/m_scale.y));
	m_world2local.set_row(2,m_forward*(1.0f/m_scale.z));
	xvec3f_t tpos=m_world2local*-m_position;
	m_world2local.set_col(3,tpos);
	remove_state(m_flag,WORLD_2_LOCAL);
}

const xmat4f_t& xcom_transform::world2local() 
{
	if(m_flag&WORLD_2_LOCAL) {
		rebuild_world2local();
		xgameobj *gobj=gameobj()->parent();
		if(gobj) {
			xcom_transform *trans=gobj->get_component<xcom_transform>();
			if(trans) m_world2local*=trans->world2local();
		}
	}
	return m_world2local;
}

void xcom_transform::set_forward(const xvec3f_t &forward, const xvec3f_t &up)
{
	m_forward=forward;
	m_up=up;
	m_right.cross(m_up,m_forward);
	if(fequal(m_right.length2(),0.0f)) {
		unsigned mind=0;
		if(fabs(m_forward.x)>fabs(m_forward.y))
			mind=1;
		if(fabs(m_forward.y)>fabs(m_forward.z))
			mind=2;
		m_right.zero();
		m_right[mind]=1.0f;
		m_up.cross(m_forward,m_right);
	}
	else {
		m_up.cross(m_forward,m_right);
		m_right.normalize();
	}
	m_forward.normalize();
	m_up.normalize();
	m_flag|=ROTATE;
}

void xcom_transform::rotate(const xvec3f_t &axis, float angle)
{
	if(wyc::fabs(angle)>360)
		angle=wyc::mod(angle,360);
	xmat3f_t mat;
	matrix_rotate3d(mat,axis,DEG_TO_RAD(angle));
	m_forward=m_forward*mat;
	m_up=m_up*mat;
	m_right=m_right*mat;
	m_flag|=ROTATE;
}

void xcom_transform::rotate_forward(float angle)
{
	if(wyc::fabs(angle)>360)
		angle=wyc::mod(angle,360);
	xmat3f_t mat;
	matrix_rotate3d(mat,m_forward,DEG_TO_RAD(angle));
	m_up=m_up*mat;
	m_right=m_right*mat;
	m_flag|=ROTATE;
}

void xcom_transform::rotate_up(float angle)
{
	if(wyc::fabs(angle)>360)
		angle=wyc::mod(angle,360);
	xmat3f_t mat;
	matrix_rotate3d(mat,m_up,DEG_TO_RAD(angle));
	m_forward=m_forward*mat;
	m_right=m_right*mat;
	m_flag|=ROTATE;
}

void xcom_transform::rotate_right(float angle)
{
	if(wyc::fabs(angle)>360)
		angle=wyc::mod(angle,360);
	xmat3f_t mat;
	matrix_rotate3d(mat,m_right,DEG_TO_RAD(angle));
	m_forward=m_forward*mat;
	m_up=m_up*mat;
	m_flag|=ROTATE;
}

}; // namespace wyc

