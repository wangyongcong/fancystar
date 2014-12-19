#include <fstream>
#include "py_scene.h"
#include "collision_detector.h"

extern "C" {

void Scene_Del (pyplt_Scene *self)
{
	delete self->_map;
	self->_map = 0;
	// free memory
	self->ob_type->tp_free(self);
}


PyObject* Scene_AddAgent (pyplt_Scene *self, PyObject *args)
{
	pyplt_Agent *agent;
	if(!PyArg_ParseTuple(args,"O!",&pyplt_AgentType,&agent))
		return NULL;
	if(agent->_parent!=self)
	{
		if(agent->_parent)
		{
			agent->_parent->_count -= 1;
			agent->_parent->_map->del_entity(agent->_aabb);
			Py_DECREF(agent->_parent);
		}
		Py_INCREF(self);
		agent->_parent = self;
		agent->_parent->_map->add_entity(agent->_aabb);
		self->_count+=1;
	}
	Py_INCREF(Py_None);
	return Py_None;
}

PyObject* Scene_DelAgent (pyplt_Scene *self, PyObject *args)
{
	pyplt_Agent *agent;
	if(!PyArg_ParseTuple(args,"O!",&pyplt_AgentType,&agent))
		return NULL;
	if(agent->_parent==self)
	{
		self->_count-=1;
		self->_map->del_entity(agent->_aabb);
		agent->_parent=0;
		Py_DECREF(self);
	}
	Py_INCREF(Py_None);
	return Py_None;
}

PyObject* Scene_StaticTest (pyplt_Scene *self, PyObject *args)
{
	int filter;
	pyplt_Agent *agent;
	if(!PyArg_ParseTuple(args,"O!i",&pyplt_AgentType,&agent,&filter))
		return NULL;
	wyc::xstatic_detector detector(agent->_aabb);
	self->_map->find_neighbors(agent->_aabb->get_lower(),agent->_aabb->get_upper(),filter,detector);
	unsigned count = detector.count();
	PyObject* list = PyList_Type.tp_alloc(&PyList_Type,count);
	for(unsigned i=0; i<count; ++i) 
		PyList_Append( list, (PyObject*)(detector[i]->get_data()) );
	return (PyObject*)list;
}

PyObject* Scene_SweepTest (pyplt_Scene *self, PyObject *args)
{
	int filter;
	pyplt_Agent *agent;
	wyc::xvec2f_t end_pos;
	if(!PyArg_ParseTuple(args,"O!(ff)i",&pyplt_AgentType,&agent,&end_pos.x,&end_pos.y,&filter))
		return NULL;
	wyc::xvec2f_t lower, upper;
	const wyc::xvec2f_t &pos = agent->_aabb->get_position();
	wyc::xsweep_detector detector(agent->_aabb,end_pos-pos);
	lower.set( std::min(pos.x,end_pos.x),
		std::min(pos.y, end_pos.y) );
	upper.set( std::max(pos.x,end_pos.x),
		std::max(pos.y, end_pos.y) );
	lower.sub(agent->_aabb->get_radius());
	upper.add(agent->_aabb->get_radius());
	self->_map->find_neighbors(lower,upper,filter,detector);
	PyObject *ret;
	wyc::xcollision_agent *collidee = detector.get_object();
	if(collidee)
	{
		ret = Py_BuildValue("Of(ff)",collidee->get_data(),detector.get_distance(),
			detector.get_normal().x,detector.get_normal().y);
	}
	else 
	{
		ret = Py_BuildValue("Of()",Py_None,1.0f);
	}
	return ret;
}

PyObject* Scene_SweepTestEx (pyplt_Scene *self, PyObject *args)
{
	int filter, normal_filter;
	pyplt_Agent *agent;
	wyc::xvec2f_t end_pos, min_dir, max_dir;
	if(!PyArg_ParseTuple(args,"O!(ff)ii(ff)(ff)",&pyplt_AgentType,&agent,&end_pos.x,&end_pos.y,&filter,\
		&normal_filter,&min_dir.x,&min_dir.y,&max_dir.x,&max_dir.y))
		return NULL;
	wyc::xvec2f_t lower, upper;
	const wyc::xvec2f_t &pos = agent->_aabb->get_position();
	wyc::xsweep_detector detector(agent->_aabb,end_pos-pos);
	detector.set_normal_filter(normal_filter,min_dir,max_dir);
	lower.set( std::min(pos.x,end_pos.x),
		std::min(pos.y, end_pos.y) );
	upper.set( std::max(pos.x,end_pos.x),
		std::max(pos.y, end_pos.y) );
	lower.sub(agent->_aabb->get_radius());
	upper.add(agent->_aabb->get_radius());
	self->_map->find_neighbors(lower,upper,filter,detector);
	PyObject *ret;
	wyc::xcollision_agent *collidee = detector.get_object();
	if(collidee)
	{
		ret = Py_BuildValue("Of(ff)",collidee->get_data(),detector.get_distance(),
			detector.get_normal().x,detector.get_normal().y);
	}
	else 
	{
		ret = Py_BuildValue("Of()",Py_None,1.0f);
	}
	return ret;
}

PyObject* Scene_SweepIntersect (pyplt_Scene *self, PyObject *args)
{
	int filter;
	pyplt_Agent *agent;
	wyc::xvec2f_t end_pos;
	if(!PyArg_ParseTuple(args,"O!(ff)i",&pyplt_AgentType,&agent,&end_pos.x,&end_pos.y,&filter))
		return NULL;
	wyc::xvec2f_t lower, upper;
	const wyc::xvec2f_t &pos = agent->_aabb->get_position();
	lower.set( std::min(pos.x,end_pos.x),
		std::min(pos.y, end_pos.y) );
	upper.set( std::max(pos.x,end_pos.x),
		std::max(pos.y, end_pos.y) );
	lower.sub(agent->_aabb->get_radius());
	upper.add(agent->_aabb->get_radius());
	PyObject* list = NULL;
	end_pos.sub(pos);
	if(fabs(end_pos.x)<EPSILON_E4 && fabs(end_pos.y)<EPSILON_E4) {
		// no displacement, do nothing
		list = PyList_Type.tp_alloc(&PyList_Type,0);
	}
	else {
		wyc::xsweep_intersect_detector detector(agent->_aabb,end_pos);
		self->_map->find_neighbors(lower,upper,filter,detector);
		unsigned count = detector.count();
		list = PyList_Type.tp_alloc(&PyList_Type,count);
		for(unsigned i=0; i<count; ++i) 
			PyList_Append( list, (PyObject*)(detector[i]->get_data()) );
	}
	return (PyObject*)list;
}

PyObject* Scene_GetSize (pyplt_Scene *self, PyObject *)
{
	return Py_BuildValue("II",self->_map->width(),self->_map->height());
}
PyObject* Scene_GetGridSize (pyplt_Scene *self, PyObject *)
{
	unsigned r=0, c=0, w=0, h=0;
	self->_map->get_lod_info(0,r,c,w,h);
	return Py_BuildValue("II",w,h);
}

PyObject* Scene_GetLOD (pyplt_Scene *self, PyObject *)
{
	return Py_BuildValue("I",self->_map->lod_count());
}

PyObject* Scene_GetAgentCount (pyplt_Scene *self, PyObject *)
{
	return Py_BuildValue("I",self->_count);
}

} // extern "C"