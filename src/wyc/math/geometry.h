#ifndef __HEADER_WYC_XGEOMETRY
#define __HEADER_WYC_XGEOMETRY

/////////////////////////////////////////////////////////////////////////////
//
// 名称：XGeometry Lib
// 作者：Blackbird
// 版本：1.01
// 描述：提供3D几何检测相关支持
//
/////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include "wyc/math/vecmath.h"

namespace wyc
{

// 计算点pt到直线的投影,直线由参数式给出: line(t)=line_org+t*line_d, line_d为单位向量, 返回投影点的参数t
template<class T, int D>
inline T project_point_on_line(const xvector<T,D> &pt, const xvector<T,D> &line_org, const xvector<T,D> &line_d)
{
	return (pt-line_org)*line_d;
}

// 计算点pt到直线的投影,直线由参数式给出: line(t)=line_org+t*line_d, line_d为单位向量, 返回投影点
template<class T, int D>
inline void project_point_on_line(const xvector<T,D> &pt, const xvector<T,D> &line_org, const xvector<T,D> &line_d, xvector<T,D> &ret)
{
	T t=(pt-line_org)*line_d;
	ret=line_org;
	ret.scale(t);
}

// 计算点到直线的距离
template<class T, int D>
inline T distance_point_line(const xvector<T,D> &pt, const xvector<T,D> &line_org, const xvector<T,D> &line_d)
{
	xvector<T,D> v;
	project_point_on_line<T,D>(pt,line_org,line_d,v);
	v-=pt;
	return v.length();
}

// 计算点pt到平面的投影,平面由点法式给出,plane_org是平面上的点,plane_normal是平面法线(单位向量),返回投影点
template<class T, int D>
inline void project_point_on_plane(const xvector<T,D> &pt, const xvector<T,D> &plane_org, const xvector<T,D> &plane_normal, xvector<T,D> &ret)
{
	T t=distance_point_plane<T,D>(pt,plane_org,plane_normal);
	ret=plane_normal;
	ret.scale(-t);
	ret+=pt;
}

// 计算点到平面的有向距离(沿平面法线方向为正)
template<class T, int D>
inline T distance_point_plane(const xvector<T,D> &pt, const xvector<T,D> &plane_org, const xvector<T,D> &plane_normal)
{
	T d=plane_org*plane_normal;
	return pt*plane_normal-d;
}

// 射线与AABB的相交检测
// 返回是否相交,以及接触点参数t (接触点p=org+t*dir)
bool intersect_ray_aabb (const xvec2f_t &org, const xvec2f_t &dir, const xvec2f_t &lower, const xvec2f_t &upper, float &t)
{
	float t1, t2, t3, t4, inv_x, inv_y;
	
	if(fabs(dir.y)<EPSILON_E4) // horizontal line
	{
		inv_x = dir.x>0?1.0f:-1.0f;
		t1 = (lower.x-org.x)*inv_x;
		t2 = (upper.x-org.x)*inv_x;
		t = std::min(t1,t2);
		return org.y>=lower.y && org.y<upper.y && (t1>0 || t2>0);
	}

	if(fabs(dir.x)<EPSILON_E4) // vertical line
	{
		inv_y = dir.y>0?1.0f:-1.0f;
		t3 = (lower.y-org.y)*inv_y;
		t4 = (upper.y-org.y)*inv_y;
		t = std::min(t3,t4);
		return org.x>=lower.x && org.x<upper.x && (t3>0 || t4>0);
	}
	
	inv_x = 1.0f/dir.x, inv_y = 1.0f/dir.y;
	t1 = (lower.x-org.x)*inv_x;
	t2 = (upper.x-org.x)*inv_x;
	t3 = (lower.y-org.y)*inv_y;
	t4 = (upper.y-org.y)*inv_y;

	float tmin = std::max(std::min(t1,t2),std::min(t3,t4)); // entry point
	float tmax = std::min(std::max(t1,t2),std::max(t3,t4)); // leave point
	
	if(tmin<tmax)
	{
		t=tmin;
		return true;
	}
	t=tmax;
	return false;
}

} // namespace wyc

#endif // end of __HEADER_WYC_XGEOMETRY
