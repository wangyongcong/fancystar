#include "py_move.h"

extern "C" {

void Move_Del (pyplt_Move *self)
{
	Py_CLEAR(self->_agent);
	Py_CLEAR(self->_terrain);
	delete self->_move;
	self->_move=0;
	// free memory
	self->ob_type->tp_free((PyObject*)self);
}

PyObject* Move_GetAgent (pyplt_Move *self, PyObject *)
{
	if(self->_agent) {
		Py_INCREF(self->_agent);
		return (PyObject*)self->_agent;
	}
	Py_INCREF(Py_None);
	return Py_None;
}

int Move_SetAgent (pyplt_Move *self, PyObject *args, void*)
{
	if(!PyObject_TypeCheck(args, &pyplt_AgentType))
	{
		PyErr_SetString(PyExc_TypeError, "The agent must be pyplt.agent");
		return -1;
	}
	if(self->_terrain) {
		self->_move->leave_terrain();
		Py_DECREF(self->_terrain);
	}
	if(self->_agent) {
		Py_DECREF(self->_agent);
	}
	self->_agent = (pyplt_Agent*)args;
	Py_INCREF(self->_agent);
	self->_move->set_agent(self->_agent->_aabb);
	return 0;
}

PyObject* Move_GetTerrain (pyplt_Move *self, PyObject *)
{
	if(self->_terrain)
	{
		Py_INCREF(self->_terrain);
		return (PyObject*)self->_terrain;
	}
	Py_INCREF(Py_None);
	return Py_None;
}

PyObject* Move_StickOnGround (pyplt_Move *self, PyObject *)
{
	PyObject *ret = Py_False;
	if(self->_agent && self->_agent->_parent && self->_move->stick_on_ground())
	{		
		wyc::xcollision_agent *terrain = self->_move->get_terrain();
		PyObject* pyobj = (PyObject*)terrain->get_data();
		if(pyobj && PyObject_TypeCheck(pyobj, &pyplt_AgentType)) {
			self->_terrain = (pyplt_Agent*)pyobj;
			Py_INCREF(pyobj);
			ret = Py_True;
		}
	}
	Py_INCREF(ret);
	return ret;
}

PyObject* Move_LeaveGround (pyplt_Move *self, PyObject *)
{
	self->_move->leave_terrain();
	Py_CLEAR(self->_terrain);
	Py_INCREF(Py_None);
	return Py_None;
}

#define SET_TERRAIN(self, terrain) {\
		Py_CLEAR((self)->_terrain);\
		if((terrain) && (terrain)->get_data()) {\
			(self)->_terrain = (pyplt_Agent*)((terrain)->get_data());\
			Py_INCREF((self)->_terrain);\
		}}

PyObject* Move_Walk (pyplt_Move *self, PyObject *args)
{
	if( !(self->_agent && self->_agent->_parent) )
		return Py_BuildValue("fO",0,Py_None);
	float t, spdx;
	unsigned extra_filter=0, max_iteration=3;
	if(!PyArg_ParseTuple(args,"ff|II",&t,&spdx,&extra_filter,&max_iteration)) {
		// bad parameters
		return Py_BuildValue("fO",0,Py_None);
	}
	wyc::xcollision_agent *terrain = self->_move->get_terrain();
	wyc::xcollision_agent *collidee = self->_move->walk(t, spdx, extra_filter, max_iteration);
	if(terrain!=self->_move->get_terrain())
	{
		terrain = self->_move->get_terrain();
		SET_TERRAIN(self,terrain);
	}
	PyObject *pyobj = collidee ? (PyObject*)collidee->get_data() : Py_None;
	return Py_BuildValue("fO",t,pyobj);
}

PyObject* Move_Fall (pyplt_Move *self, PyObject *args)
{
	if( !(self->_agent && self->_agent->_parent) )
		return Py_BuildValue("fOO",0,Py_None,Py_None);
	float t, accy;
	wyc::xvec2f_t speed;
	unsigned extra_filter=0, max_iteration=3;
	if(!PyArg_ParseTuple(args,"ffff|II",&t,&speed.x,&speed.y,&accy,&extra_filter,&max_iteration)) {
		// bad parameters
		return Py_BuildValue("fOO",0,Py_None,Py_None);
	}
	wyc::xcollision_agent *terrain = self->_move->get_terrain();
	wyc::xcollision_agent *collidee = self->_move->fall(t, speed, accy, extra_filter, max_iteration);
	if(terrain != self->_move->get_terrain())
	{
		terrain = self->_move->get_terrain();
		SET_TERRAIN(self,terrain);
	}
	PyObject *pyobj = collidee ? (PyObject*)collidee->get_data() : Py_None;
	return Py_BuildValue("fO(ff)",t,pyobj,speed.x,speed.y);
}

PyObject* Move_Jump (pyplt_Move *self, PyObject *args)
{
	if( !(self->_agent && self->_agent->_parent) )
		return Py_BuildValue("fOO",0,Py_None,Py_None);
	float t, accy;
	wyc::xvec2f_t speed;
	unsigned extra_filter=0, max_iteration=3;
	if(!PyArg_ParseTuple(args,"ffff|II",&t,&speed.x,&speed.y,&accy,&extra_filter,&max_iteration)) {
		// bad parameters
		return Py_BuildValue("fOO",0,Py_None,Py_None);
	}
	wyc::xcollision_agent *terrain = self->_move->get_terrain();
	wyc::xcollision_agent *collidee = self->_move->jump(t, speed, accy, extra_filter, max_iteration);
	if(terrain != self->_move->get_terrain())
	{
		terrain = self->_move->get_terrain();
		SET_TERRAIN(self,terrain);
	}
	PyObject *pyobj = collidee ? (PyObject*)collidee->get_data() : Py_None;
	return Py_BuildValue("fO(ff)",t,pyobj,speed.x,speed.y);
}

PyObject* Move_Fly  (pyplt_Move *self, PyObject *args)
{
	if( !(self->_agent && self->_agent->_parent) )
		return Py_BuildValue("fO",0,Py_None);
	float t;
	wyc::xvec2f_t speed;
	unsigned extra_filter=0;
	if(!PyArg_ParseTuple(args,"fff|I",&t,&speed.x,&speed.y,&extra_filter)) {
		// bad parameters
		return Py_BuildValue("fO",0,Py_None);
	}
	if(self->_terrain) 
	{
		Py_DECREF(self->_terrain);
		self->_terrain = 0;
	}
	wyc::xcollision_agent *collidee = self->_move->fly(t, speed, extra_filter);
	PyObject *pyobj = collidee ? (PyObject*)collidee->get_data() : Py_None;
	return Py_BuildValue("fO",t,pyobj);
}

PyObject* Move_FlushKeyPos (pyplt_Move *self, PyObject *)
{
	const std::vector<float> &kpos = self->_move->get_kpos();
	if(kpos.empty())
		return PyString_FromString("");
	const char *v = (const char*)(&kpos[0]);
	PyObject *ret = PyString_FromStringAndSize(v, kpos.size()*sizeof(float));
	self->_move->clear_kpos();
	return ret;
}

PyObject* Move_GetKeyPosAsList (pyplt_Move *self, PyObject *)
{
	const std::vector<float> &kpos = self->_move->get_kpos();
	size_t count = kpos.size();
	PyObject* list = PyList_Type.tp_alloc(&PyList_Type,count);
	for(size_t i=0; i<count; ++i)
		PyList_Append(list,PyFloat_FromDouble(kpos[i]));
	return list;
}

void* wyc::xrecorder::py_detail() const
{
	std::string indent;
	unsigned idx=0;
	PyObject* list = PyList_Type.tp_alloc(&PyList_Type,m_headers.size()), *s, *t;
	std::vector<header_t>::const_iterator iter = m_headers.begin(), end = m_headers.end();
	for(; iter!=end; ++iter)
	{
		if(iter->op == OP_CALL) 
			s = PyString_FromFormat("%s%s",indent.c_str(),iter->name.c_str());
		else if(iter->op == OP_RETURN) {
			indent.erase(indent.size()-1);
			s = PyString_FromFormat("%s%s END",indent.c_str(),iter->name.c_str());
		}
		else {
			s = PyString_FromFormat("%s%s",indent.c_str(),iter->name.c_str());
		}
		if(iter->format.size())
		{
			t = PyTuple_New(iter->format.size());
			for(size_t i=0, cnt=iter->format.size(); i<cnt; ++i)
			{
				switch(iter->format[i])
				{
				case 'f':
					PyTuple_SetItem(t,i,PyFloat_FromDouble(m_data[idx++].fval));
					break;
				case 'i':
				case 'I':
					PyTuple_SetItem(t,i,PyInt_FromLong(m_data[idx++].ival));
					break;
				}
			}
		}
		else t = Py_None;
		PyList_Append(list,Py_BuildValue("OO",s,t));
		if(iter->op == OP_CALL) 
			indent += "\t";
	}
	return list;
}

PyObject* Move_Detail (pyplt_Move *self, PyObject *)
{
	return (PyObject*)self->_move->get_record().py_detail();
}


} // extern "C"
