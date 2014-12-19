#include "py_agent.h"

extern "C" {

void Agent_Del (pyplt_Agent *self)
{
	if(self->_parent) {
		self->_parent->_count-=1;
		self->_parent->_map->del_entity(self->_aabb);
		Py_CLEAR(self->_parent);
	}
	delete self->_aabb;
	self->_aabb=0;
	// free memory
	self->ob_type->tp_free((PyObject*)self);
}

PyObject* Agent_GetParent (pyplt_Agent *self, PyObject *)
{
	if(self->_parent) {
		Py_INCREF(self->_parent);
		return (PyObject*)self->_parent;
	}
	Py_INCREF(Py_None);
	return Py_None;
}

PyObject* Agent_GetPos (pyplt_Agent *self, PyObject *)
{
	const wyc::xvec2f_t &pos = self->_aabb->get_position();
	return Py_BuildValue("ff",pos.x,pos.y);
}

int Agent_SetPos (pyplt_Agent *self, PyObject *args, void*)
{
	if(!PyTuple_Check(args))
	{
		PyErr_SetString(PyExc_TypeError, "The position must be a tuple such as (x,y)");
		return -1;
	}
	float x, y;
	if(!PyArg_ParseTuple(args,"ff",&x,&y)) {
		PyErr_SetString(PyExc_TypeError, "The position must be a tuple such as (x,y)");
		return -1;
	}
	self->_aabb->set_position(x,y);
	self->_aabb->update_aabb();
	return 0;
}

PyObject* Agent_GetRadius (pyplt_Agent *self, PyObject *)
{
	const wyc::xvec2f_t &r = self->_aabb->get_radius();
	return Py_BuildValue("ff",r.x,r.y);
}

int Agent_SetRadius (pyplt_Agent *self, PyObject *args, void*)
{
	if(!PyTuple_Check(args))
	{
		PyErr_SetString(PyExc_TypeError, "The radius must be a tuple such as (half_w, half_h)");
		return -1;
	}
	float x, y;
	if(!PyArg_ParseTuple(args,"ff",&x,&y)) {
		PyErr_SetString(PyExc_TypeError, "The radius must be a tuple such as (half_w, half_h)");
		return -1;
	}
	self->_aabb->set_radius(x,y);
	self->_aabb->update_aabb();
	return 0;
}

PyObject* Agent_GetType (pyplt_Agent *self, PyObject *)
{
	return Py_BuildValue("i",self->_aabb->agent_type());
}

PyObject* Agent_GetGroup (pyplt_Agent *self, PyObject *)
{
	return Py_BuildValue("i",self->_aabb->get_mask());
}

int Agent_SetGroup (pyplt_Agent *self, PyObject *args, void*)
{
	if(!PyInt_Check(args))
	{
		PyErr_SetString(PyExc_TypeError, "The group must be an integer value");
		return -1;
	}
	self->_aabb->set_mask(PyInt_AsLong(args));
	return 0;
}

PyObject* Agent_GetBoundingBox (pyplt_Agent *self, PyObject *)
{
	const wyc::xvec2f_t &lower = self->_aabb->get_lower();
	const wyc::xvec2f_t &upper = self->_aabb->get_upper();
	return Py_BuildValue("ffff",lower.x,lower.y,upper.x,upper.y);
}

PyObject* Agent_GetSurfaceNormal (pyplt_Agent *self, PyObject *)
{
	const wyc::xvec2f_t &n = self->_aabb->get_slope_normal();
	return Py_BuildValue("ff",n.x,n.y);
}

} // extern "C"
