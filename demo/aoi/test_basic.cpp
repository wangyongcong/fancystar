#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <vector>

#include "wyc/util/util.h"
#include "aoi3d.h"

using namespace wyc;

namespace TEST_BASIC
{

enum {
	GROUP_OBJECT = 1,
	GROUP_AREA = 2,
};

class xtest_object : public xaoi_entity
{
public:
	xtest_object(const xvec3f_t &r) : xaoi_entity(r)
	{
		set_mask(GROUP_OBJECT);
	}

	virtual void on_enter (xaoi_entity *en)
	{
		assert(0 && "should not arrive here");
	}

	virtual void on_leave (xaoi_entity *en)
	{
		assert(0 && "should not arrive here");
	}
};

class xtest_area : public xaoi_entity
{
	std::vector<xaoi_entity*> m_my_objs;
public:
	xtest_area(const xvec3f_t &r) : xaoi_entity(r)
	{
		set_mask(GROUP_AREA);
		set_filter(GROUP_OBJECT);
	}

	virtual void on_enter (xaoi_entity *en)
	{
		size_t i = find_object(en);
		if(i<m_my_objs.size())
		{
			assert(0 && "obj already in my range");
		}
		m_my_objs.push_back(en);
	}

	virtual void on_leave (xaoi_entity *en)
	{
		size_t i=find_object(en);
		if(i>=m_my_objs.size())
		{
			assert(0 && "obj not found");
		}
		m_my_objs.erase(m_my_objs.begin()+i);
	}

	size_t find_object (xaoi_entity *en)
	{
		size_t count = m_my_objs.size(), i=0;
		for(; i<count; ++i)
		{
			if(m_my_objs[i]==en)
				break;
		}
		return i;
	}

	bool is_obj_in(xaoi_entity *en)
	{
		return find_object(en)<m_my_objs.size();
	}

};

}

using namespace TEST_BASIC;

void test_point_object ()
{
	printf("test point object");

	xaoi_manager mgr;

	xtest_object *obj = new xtest_object(xvec3f_t(0,0,0));
	xtest_area *point = new xtest_area(xvec3f_t(0,0,0));
	xtest_area *line = new xtest_area(xvec3f_t(1,0,0));
	xtest_area *plane = new xtest_area(xvec3f_t(1,1,0));
	xtest_area *cube = new xtest_area(xvec3f_t(1,1,1));

	// add/remove object
	mgr.add(obj,xvec3f_t(0,0,0));
	mgr.add(point,xvec3f_t(0,0,0));
	assert(point->is_obj_in(obj));
	mgr.add(line,xvec3f_t(0,0,0));
	assert(line->is_obj_in(obj));
	mgr.add(plane,xvec3f_t(0,0,0));
	assert(plane->is_obj_in(obj));
	mgr.add(cube,xvec3f_t(0,0,0));
	assert(cube->is_obj_in(obj));	
	mgr.remove(obj);
	assert(!point->is_obj_in(obj));
	assert(!line->is_obj_in(obj));
	assert(!plane->is_obj_in(obj));
	assert(!cube->is_obj_in(obj));
	mgr.add(obj,xvec3f_t(0,0,0));
	assert(point->is_obj_in(obj));
	assert(line->is_obj_in(obj));
	assert(plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));
	mgr._verify();
	printf(".");

	// move along X
	obj->move_to(xvec3f_t(-1.5f,0,0));
	assert(!point->is_obj_in(obj));
	assert(!line->is_obj_in(obj));
	assert(!plane->is_obj_in(obj));
	assert(!cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(-1,0,0));
	assert(!point->is_obj_in(obj));
	assert(line->is_obj_in(obj));
	assert(plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(1,0,0));
	assert(!point->is_obj_in(obj));
	assert(line->is_obj_in(obj));
	assert(plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(1.5f,0,0));
	assert(!point->is_obj_in(obj));
	assert(!line->is_obj_in(obj));
	assert(!plane->is_obj_in(obj));
	assert(!cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(-1.5f,0,0));
	assert(!point->is_obj_in(obj));
	assert(!line->is_obj_in(obj));
	assert(!plane->is_obj_in(obj));
	assert(!cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(0,0,0));
	assert(point->is_obj_in(obj));
	assert(line->is_obj_in(obj));
	assert(plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));
	mgr._verify();
	printf(".");

	// move along Y
	obj->move_to(xvec3f_t(0,-1.5f,0));
	assert(!point->is_obj_in(obj));
	assert(!line->is_obj_in(obj));
	assert(!plane->is_obj_in(obj));
	assert(!cube->is_obj_in(obj));	

	obj->move_to(xvec3f_t(0,-1,0));
	assert(!point->is_obj_in(obj));
	assert(!line->is_obj_in(obj));
	assert(plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));	

	obj->move_to(xvec3f_t(0,1,0));
	assert(!point->is_obj_in(obj));
	assert(!line->is_obj_in(obj));
	assert(plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));	

	obj->move_to(xvec3f_t(0,1.5f,0));
	assert(!point->is_obj_in(obj));
	assert(!line->is_obj_in(obj));
	assert(!plane->is_obj_in(obj));
	assert(!cube->is_obj_in(obj));	

	obj->move_to(xvec3f_t(0,-1.5f,0));
	assert(!point->is_obj_in(obj));
	assert(!line->is_obj_in(obj));
	assert(!plane->is_obj_in(obj));
	assert(!cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(0,0,0));
	assert(point->is_obj_in(obj));
	assert(line->is_obj_in(obj));
	assert(plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));
	mgr._verify();
	printf(".");

	// move along Z
	obj->move_to(xvec3f_t(0,0,-1.5f));
	assert(!point->is_obj_in(obj));
	assert(!line->is_obj_in(obj));
	assert(!plane->is_obj_in(obj));
	assert(!cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(0,0,-1));
	assert(!point->is_obj_in(obj));
	assert(!line->is_obj_in(obj));
	assert(!plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(0,0,1));
	assert(!point->is_obj_in(obj));
	assert(!line->is_obj_in(obj));
	assert(!plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(0,0,1.5f));
	assert(!point->is_obj_in(obj));
	assert(!line->is_obj_in(obj));
	assert(!plane->is_obj_in(obj));
	assert(!cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(0,0,-1.5f));
	assert(!point->is_obj_in(obj));
	assert(!line->is_obj_in(obj));
	assert(!plane->is_obj_in(obj));
	assert(!cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(0,0,0));
	assert(point->is_obj_in(obj));
	assert(line->is_obj_in(obj));
	assert(plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));
	mgr._verify();
	printf(".");

	// move to corner
	obj->move_to(xvec3f_t(-1,1,0));
	assert(plane->is_obj_in(obj));
	obj->move_to(xvec3f_t(-1,-1,0));
	assert(plane->is_obj_in(obj));
	obj->move_to(xvec3f_t(1,-1,0));
	assert(plane->is_obj_in(obj));
	obj->move_to(xvec3f_t(1,1,0));
	assert(plane->is_obj_in(obj));

	obj->move_to(xvec3f_t(-1,-1,-1));
	assert(cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(-1,-1,1));
	assert(cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(1,-1,1));
	assert(cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(1,-1,-1));
	assert(cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(-1,1,-1));
	assert(cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(-1,1,1));
	assert(cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(1,1,1));
	assert(cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(1,1,-1));
	assert(cube->is_obj_in(obj));

	mgr._verify();
	mgr.clear();
	printf(".\n");

	delete obj;
	delete point;
	delete line;
	delete plane;
	delete cube;
}

void test_line_object ()
{
	printf("test line object");

	xaoi_manager mgr;

	xtest_object *obj = new xtest_object(xvec3f_t(1,0,0));
	xtest_area *line = new xtest_area(xvec3f_t(4,0,0));
	xtest_area *plane = new xtest_area(xvec3f_t(4,4,0));
	xtest_area *cube = new xtest_area(xvec3f_t(4,4,4));

	// add/remove object
	mgr.add(obj,xvec3f_t(0,0,0));
	mgr.add(line,xvec3f_t(0,0,0));
	assert(line->is_obj_in(obj));
	mgr.add(plane,xvec3f_t(0,0,0));
	assert(plane->is_obj_in(obj));
	mgr.add(cube,xvec3f_t(0,0,0));
	assert(cube->is_obj_in(obj));
	mgr.remove(obj);
	assert(!line->is_obj_in(obj));
	assert(!plane->is_obj_in(obj));
	assert(!cube->is_obj_in(obj));
	mgr.add(obj,xvec3f_t(0,0,0));
	assert(line->is_obj_in(obj));
	assert(plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));

	mgr._verify();
	printf(".");

	// move along X
	obj->move_to(xvec3f_t(-6,0,0));
	assert(!line->is_obj_in(obj));
	assert(!plane->is_obj_in(obj));
	assert(!cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(-5,0,0));
	assert(line->is_obj_in(obj));
	assert(plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(-3,0,0));
	assert(line->is_obj_in(obj));
	assert(plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(3,0,0));
	assert(line->is_obj_in(obj));
	assert(plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(5,0,0));
	assert(line->is_obj_in(obj));
	assert(plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(6,0,0));
	assert(!line->is_obj_in(obj));
	assert(!plane->is_obj_in(obj));
	assert(!cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(-6,0,0));
	assert(!line->is_obj_in(obj));
	assert(!plane->is_obj_in(obj));
	assert(!cube->is_obj_in(obj));

	mgr._verify();
	printf(".");

	// move along Y
	obj->move_to(xvec3f_t(0,4,0));
	assert(!line->is_obj_in(obj));
	assert(plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(0,-4,0));
	assert(!line->is_obj_in(obj));
	assert(plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(0,-5,0));
	assert(!line->is_obj_in(obj));
	assert(!plane->is_obj_in(obj));
	assert(!cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(0,5,0));
	assert(!line->is_obj_in(obj));
	assert(!plane->is_obj_in(obj));
	assert(!cube->is_obj_in(obj));

	mgr._verify();
	printf(".");

	// move along Z
	obj->move_to(xvec3f_t(0,0,-5));
	assert(!line->is_obj_in(obj));
	assert(!plane->is_obj_in(obj));
	assert(!cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(0,0,-4));
	assert(!line->is_obj_in(obj));
	assert(!plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(0,0,4));
	assert(!line->is_obj_in(obj));
	assert(!plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(0,0,5));
	assert(!line->is_obj_in(obj));
	assert(!plane->is_obj_in(obj));
	assert(!cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(0,0,-5));
	assert(!line->is_obj_in(obj));
	assert(!plane->is_obj_in(obj));
	assert(!cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(0,0,0));
	assert(line->is_obj_in(obj));
	assert(plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));

	mgr._verify();
	mgr.clear();
	printf(".\n");

	delete obj;
	delete line;
	delete plane;
	delete cube;
}

void test_plane_object ()
{
	printf("test plane object");

	xaoi_manager mgr;

	xtest_object *obj = new xtest_object(xvec3f_t(1,1,0));
	xtest_area *plane = new xtest_area(xvec3f_t(4,4,0));
	xtest_area *cube = new xtest_area(xvec3f_t(4,4,4));

	// add/remove object
	mgr.add(obj,xvec3f_t(0,0,0));
	mgr.add(plane,xvec3f_t(0,0,0));
	assert(plane->is_obj_in(obj));
	mgr.add(cube,xvec3f_t(0,0,0));
	assert(cube->is_obj_in(obj));
	mgr.remove(obj);
	assert(!plane->is_obj_in(obj));
	assert(!cube->is_obj_in(obj));
	mgr.add(obj,xvec3f_t(0,0,0));
	assert(plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));

	mgr._verify();
	printf(".");

	// move along X
	obj->move_to(xvec3f_t(-6,0,0));
	assert(!plane->is_obj_in(obj));
	assert(!cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(-5,0,0));
	assert(plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(-3,0,0));
	assert(plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(3,0,0));
	assert(plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(5,0,0));
	assert(plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(6,0,0));
	assert(!plane->is_obj_in(obj));
	assert(!cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(-6,0,0));
	assert(!plane->is_obj_in(obj));
	assert(!cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(0,0,0));
	assert(plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));
	mgr._verify();
	printf(".");

	// move along Y
	obj->move_to(xvec3f_t(0,-6,0));
	assert(!plane->is_obj_in(obj));
	assert(!cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(0,-5,0));
	assert(plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(0,-3,0));
	assert(plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(0,3,0));
	assert(plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(0,5,0));
	assert(plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(0,6,0));
	assert(!plane->is_obj_in(obj));
	assert(!cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(0,-6,0));
	assert(!plane->is_obj_in(obj));
	assert(!cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(0,0,0));
	assert(plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));
	mgr._verify();
	printf(".");

	// move along Z
	obj->move_to(xvec3f_t(0,0,-5));
	assert(!plane->is_obj_in(obj));
	assert(!cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(0,0,-4));
	assert(!plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(0,0,4));
	assert(!plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(0,0,5));
	assert(!plane->is_obj_in(obj));
	assert(!cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(0,0,-5));
	assert(!plane->is_obj_in(obj));
	assert(!cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(0,0,0));
	assert(plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));
	mgr._verify();
	printf(".");

	// move to corner
	obj->move_to(xvec3f_t(-5,-5,0));
	assert(plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(5,-5,0));
	assert(plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(5,5,0));
	assert(plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(-5,5,0));
	assert(plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(-5,-5,-4));
	assert(!plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(5,-5,-4));
	assert(!plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(5,-5,4));
	assert(!plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(-5,-5,4));
	assert(!plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(-5,5,-4));
	assert(!plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(5,5,-4));
	assert(!plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(5,5,4));
	assert(!plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(-5,5,4));
	assert(!plane->is_obj_in(obj));
	assert(cube->is_obj_in(obj));

	mgr._verify();
	mgr.clear();
	printf(".\n");

	delete obj;
	delete plane;
	delete cube;
}

void test_cube_object()
{
	printf("test cube object");

	xaoi_manager mgr;

	xtest_object *obj = new xtest_object(xvec3f_t(1,1,1));
	xtest_area *cube = new xtest_area(xvec3f_t(4,4,4));

	// add/remove object
	mgr.add(obj,xvec3f_t(0,0,0));
	mgr.add(cube,xvec3f_t(0,0,0));
	assert(cube->is_obj_in(obj));
	mgr.remove(obj);
	assert(!cube->is_obj_in(obj));
	mgr.add(obj,xvec3f_t(0,0,0));
	assert(cube->is_obj_in(obj));
	mgr._verify();
	printf(".");

	// move along X
	obj->move_to(xvec3f_t(-6,0,0));
	assert(!cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(-5,0,0));
	assert(cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(-3,0,0));
	assert(cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(3,0,0));
	assert(cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(5,0,0));
	assert(cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(6,0,0));
	assert(!cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(-6,0,0));
	assert(!cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(0,0,0));
	assert(cube->is_obj_in(obj));
	mgr._verify();
	printf(".");

	// move along Y
	obj->move_to(xvec3f_t(0,-6,0));
	assert(!cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(0,-5,0));
	assert(cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(0,-3,0));
	assert(cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(0,3,0));
	assert(cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(0,5,0));
	assert(cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(0,6,0));
	assert(!cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(0,-6,0));
	assert(!cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(0,0,0));
	assert(cube->is_obj_in(obj));
	mgr._verify();
	printf(".");

	// move along Z
	obj->move_to(xvec3f_t(0,0,-6));
	assert(!cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(0,0,-5));
	assert(cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(0,0,-3));
	assert(cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(0,0,3));
	assert(cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(0,0,5));
	assert(cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(0,0,6));
	assert(!cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(0,0,-6));
	assert(!cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(0,0,0));
	assert(cube->is_obj_in(obj));
	mgr._verify();
	printf(".");

	// move to corner
	obj->move_to(xvec3f_t(-5,-5,-5));
	assert(cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(5,-5,-5));
	assert(cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(5,-5,5));
	assert(cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(-5,-5,5));
	assert(cube->is_obj_in(obj));

	obj->move_to(xvec3f_t(-5,5,-5));
	assert(cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(5,5,-5));
	assert(cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(5,5,5));
	assert(cube->is_obj_in(obj));
	obj->move_to(xvec3f_t(-5,5,5));
	assert(cube->is_obj_in(obj));

	mgr._verify();
	mgr.clear();
	printf(".\n");

	delete obj;
	delete cube;
}

void test_new_and_clean()
{
	printf("test new and clean");
	xaoi_manager mgr;
	xtest_object *obj;
	std::vector<xtest_object*> all_objs;
	unsigned OBJ_COUNT=10, DEL_COUNT=OBJ_COUNT/3;
	xvec3f_t pos, r;
	for(int i=0; i<OBJ_COUNT; ++i) {
		r.x=wyc::random();
		r.y=wyc::random();
		r.z=wyc::random();
		pos.x=wyc::random();
		pos.y=wyc::random();
		pos.z=wyc::random();
		obj=new xtest_object(r);
		all_objs.push_back(obj);
		mgr.add(obj,pos);
	}	
	for(int i=0; i<DEL_COUNT; ++i) {
		int idx=int(random()*(OBJ_COUNT-1));
		obj=all_objs[idx];
		all_objs[idx]=0;
		delete obj;
	}
	mgr._verify();
	printf(".");
	mgr.clear();
	for(int i=0; i<4; ++i)
	{
		obj=all_objs[i];
		if(obj) {
			assert(obj->get_manager()==0);
			delete obj;
		}
	}
	all_objs.clear();
	printf(".\n");
}

void test_basic ()
{
	test_point_object();
	test_line_object();
	test_plane_object();
	test_cube_object();
	test_new_and_clean();
}

