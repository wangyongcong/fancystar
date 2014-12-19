#include <cassert>
#include <Python.h>
#include <structmember.h>
#include "aoi3d.h"
#ifdef NEOX
#include "python/ipython.h"
#include "python/iplugin.h"
#else
#pragma comment (lib, "python25.lib")
#endif

#define PYAOI_VERSION_MAJOR "1"
#define PYAOI_VERSION_MINOR "0"
#define PYAOI_VERSION_REVISION "2"

using namespace wyc;

typedef PyObject* (*callback_enter) (PyObject*, PyObject *);
typedef PyObject* (*callback_leave) (PyObject*, PyObject *);

extern "C" {

class pyaoi_Entity;

typedef struct {
	PyObject_HEAD
	//----------------------
	xaoi_manager *_mgr;
	int _dimension;
} pyaoi_Manager;

typedef struct {
	PyObject_HEAD
	//----------------------
	pyaoi_Entity *_entity;
	pyaoi_Manager *_parent;
	PyObject *_name;
	PyObject *_scene_obj;
	PyObject *_on_enter;
	PyObject *_on_leave;
} pyaoi_Trigger;

class pyaoi_Entity : public xaoi_entity
{
	pyaoi_Trigger *m_trigger;
public:
	pyaoi_Entity(pyaoi_Trigger *tr, const xvec3f_t &radius) : xaoi_entity(radius), m_trigger(tr)
	{
		assert(tr);
	}
	virtual void on_enter(xaoi_entity *entity) {
		pyaoi_Trigger *other = ((pyaoi_Entity*)entity)->m_trigger;
		if(PyCallable_Check(m_trigger->_on_enter))
		{
			::PyObject_CallFunctionObjArgs(m_trigger->_on_enter,m_trigger->_scene_obj,m_trigger->_name,other->_scene_obj,other->_name,NULL);
		}
	}
	virtual void on_leave(xaoi_entity *entity) {
		pyaoi_Trigger *other = ((pyaoi_Entity*)entity)->m_trigger;
		if(PyCallable_Check(m_trigger->_on_leave))
		{
			::PyObject_CallFunctionObjArgs(m_trigger->_on_leave,m_trigger->_scene_obj,m_trigger->_name,other->_scene_obj,other->_name,NULL);
		}
	}
};

//-------------------------------------------------------------------------------------------------------

void Trigger_Del (pyaoi_Trigger *self)
{
	if(self->_parent) {
		assert(self->_entity);
		// do not fire aoi events
		self->_parent->_mgr->remove(self->_entity,false);
		Py_CLEAR(self->_parent);
	}
	delete self->_entity;
	self->_entity=0;
	Py_CLEAR(self->_name);
	Py_CLEAR(self->_scene_obj);
	Py_CLEAR(self->_on_enter);
	Py_CLEAR(self->_on_leave);
	// free memory
	self->ob_type->tp_free((PyObject*)self);
}

PyObject * Trigger_GetX (pyaoi_Trigger *self, void *)
{
	assert(self->_entity);
	PyObject *obj = PyFloat_FromDouble(self->_entity->get_pos().x);
    Py_XINCREF(obj);
    return obj;
}

PyObject * Trigger_GetY (pyaoi_Trigger *self, void *)
{
	assert(self->_entity);
	PyObject *obj = PyFloat_FromDouble(self->_entity->get_pos().y);
    Py_XINCREF(obj);
    return obj;
}

PyObject * Trigger_GetZ (pyaoi_Trigger *self, void *)
{
	assert(self->_entity);
	PyObject *obj = PyFloat_FromDouble(self->_entity->get_pos().z);
    Py_XINCREF(obj);
    return obj;
}

PyObject* Trigger_GetPosition (pyaoi_Trigger *self, void *)
{
	assert(self->_entity);
	const xvec3f_t &pos = self->_entity->get_pos();
	return Py_BuildValue("fff",pos.x,pos.y,pos.z);
}

PyObject* Trigger_GetRadius (pyaoi_Trigger *self, void *)
{
	assert(self->_entity);
	const xvec3f_t &r = self->_entity->get_radius();
	return Py_BuildValue("fff",r.x,r.y,r.z);
}

PyObject* Trigger_GetGroup (pyaoi_Trigger *self, void *)
{
	assert(self->_entity);
	return Py_BuildValue("i",self->_entity->get_mask());
}

int Trigger_SetGroup (pyaoi_Trigger *self, PyObject *args, void*)
{
	if(!args) {
		PyErr_SetString(PyExc_TypeError, "Attribute is NULL");
		return -1;
	}
	if(!PyInt_Check(args))
	{
		PyErr_SetString(PyExc_TypeError, "The group must be an integer value");
		return -1;
	}
	self->_entity->set_mask(PyInt_AsLong(args));
	return 0;
}

PyObject* Trigger_GetGroupFilter (pyaoi_Trigger *self, void *)
{
	assert(self->_entity);
	return Py_BuildValue("i",self->_entity->get_filter());
}

int Trigger_SetGroupFilter (pyaoi_Trigger *self, PyObject *args, void*)
{
	if(!args) {
		PyErr_SetString(PyExc_TypeError, "Attribute is NULL");
		return -1;
	}
	if(!PyInt_Check(args))
	{
		PyErr_SetString(PyExc_TypeError, "The group filter must be an integer value");
		return -1;
	}
	self->_entity->set_filter(PyInt_AsLong(args));
	return 0;
}

PyObject* Trigger_MoveTo (pyaoi_Trigger *self, PyObject *args)
{
	if(self->_parent)
	{
		xvec3f_t pos;
		pos.zero();
		if(!PyArg_ParseTuple(args,"f|ff",&pos.x,&pos.y,&pos.z))
		{
			return NULL;
		}
		self->_entity->move_to(pos);
	}
	Py_RETURN_NONE;
}

PyObject* Trigger_Leave (pyaoi_Trigger *self, PyObject*)
{
	if(self->_parent)
	{
		assert(self->_entity);
		self->_parent->_mgr->remove(self->_entity);
		Py_CLEAR(self->_parent);
	}
	Py_RETURN_NONE;
}

static PyMemberDef Trigger_members[] = 
{
	{"name", T_OBJECT, offsetof(pyaoi_Trigger, _name), 0, "Name of trigger"},
	{"scene_obj", T_OBJECT, offsetof(pyaoi_Trigger, _scene_obj), 0, "The associated scene object."},
	{"callback_enter", T_OBJECT, offsetof(pyaoi_Trigger, _on_enter), 0, "The callback function invokes when other triggers enter its scope."},
	{"callback_leave", T_OBJECT, offsetof(pyaoi_Trigger, _on_leave), 0, "The callback function invokes when other triggers leave its scope."},
	{NULL}  /* Sentinel */
};

static PyGetSetDef Trigger_getseters[] = 
{
	{"radius", (getter)Trigger_GetRadius, NULL, "Radius of the trigger's scope. Read only.", NULL},
	{"pos", (getter)Trigger_GetPosition, NULL,"Position of the trigger. Read only", NULL},
	{"x", (getter)Trigger_GetX, NULL,"X coordinate of the trigger's position. Read only.", NULL},
	{"y", (getter)Trigger_GetY, NULL,"Y coordinate of the trigger's position. Read only.", NULL},
	{"z", (getter)Trigger_GetZ, NULL,"Z coordinate of the trigger's position. Read only.", NULL},
	{"group", (getter)Trigger_GetGroup, (setter)Trigger_SetGroup,"Group of the trigger. Read only.", NULL},
	{"group_filter", (getter)Trigger_GetGroupFilter, (setter)Trigger_SetGroupFilter,"Group filter of the trigger. Read only.", NULL},
	{NULL}  /* Sentinel */
};

static PyMethodDef Trigger_methods[] = 
{
	{"move_to", (PyCFunction)Trigger_MoveTo, METH_VARARGS, "tr.move_to(x, y=0, z=0)\nMove the trigger to (x,y,z). Corresponding AOI events could be fired if the object is already in manager. Otherwise just modify the position."},
	{"leave", (PyCFunction)Trigger_Leave, METH_VARARGS, "tr.leave()\nLeave manager. Corresponding AOI events could be fired."},
	{NULL,NULL,0,NULL},
};

static PyTypeObject pyaoi_TriggerType = {
	PyObject_HEAD_INIT(NULL)
	0,							/*ob_size*/
	"pyaoi.trigger",			/*tp_name*/
	sizeof(pyaoi_Trigger),		/*tp_basicsize*/
	0,							/*tp_itemsize*/
	(destructor)Trigger_Del,	/*tp_dealloc*/
	0,							/*tp_print*/
	0,							/*tp_getattr*/
	0,							/*tp_setattr*/
	0,							/*tp_compare*/
	0,							/*tp_repr*/
	0,							/*tp_as_number*/
	0,							/*tp_as_sequence*/
	0,							/*tp_as_mapping*/
	0,							/*tp_hash */
	0,							/*tp_call*/
	0,							/*tp_str*/
	0,							/*tp_getattro*/
	0,							/*tp_setattro*/
	0,							/*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,			/*tp_flags*/
	"AOI trigger objects",		/* tp_doc */
	0,							/* tp_traverse */
	0,							/* tp_clear */
	0,							/* tp_richcompare */
	0,							/* tp_weaklistoffset */
	0,							/* tp_iter */
	0,							/* tp_iternext */
	Trigger_methods,			/* tp_methods */
	Trigger_members,			/* tp_members */
	Trigger_getseters,			/* tp_getset */
};

//-------------------------------------------------------------------------------------------------------

void Manager_Del (pyaoi_Manager *self)
{
	delete self->_mgr;
	self->_mgr=0;
	// free memory
	self->ob_type->tp_free(self);
}

PyObject* Manager_AddTrigger (pyaoi_Manager *self, PyObject *args)
{
	xvec3f_t pos;
	pos.zero();
	pyaoi_Trigger *tr;
	if(!PyArg_ParseTuple(args,"O!|fff",&pyaoi_TriggerType,&tr,&pos.x,&pos.y,&pos.z))
	{
		return NULL;
	}
	if(tr->_parent != self)
	{
		if(tr->_parent) {
			tr->_parent->_mgr->remove(tr->_entity);
			Py_DECREF(tr->_parent);
		}
		self->_mgr->add(tr->_entity,pos);
		Py_INCREF(self);
		tr->_parent=self;
	}
	Py_RETURN_NONE;
}

PyObject* Manager_RemoveTrigger (pyaoi_Manager *self, PyObject *args)
{
	pyaoi_Trigger *tr;
	if(!PyArg_ParseTuple(args,"O!",&pyaoi_TriggerType,&tr))
	{
		return NULL;
	}
	if(tr->_parent==self)
	{
		self->_mgr->remove(tr->_entity);
		Py_DECREF(self);
		tr->_parent=NULL;
	}
	Py_RETURN_NONE;
}

PyObject* Manager_GetDimension (pyaoi_Manager *self, PyObject *)
{
	return Py_BuildValue("i",self->_dimension);
}

static PyGetSetDef Manager_getsets[] = 
{
	{"dimension", (getter)Manager_GetDimension, NULL,"Dimension of manager. Read only.", NULL},
    {NULL}  /* Sentinel */
};

static PyMethodDef Manager_methods[] = 
{
	{"add", (PyCFunction)Manager_AddTrigger, METH_VARARGS, "mgr.add(trigger, x, y=0, z=0)\nAdd trigger to the manager. Its initial position will be (x,y,z). If the trigger is already in other manager, it'll be remove first, and corresponding AOI events could be fired."},
	{"remove", (PyCFunction)Manager_RemoveTrigger, METH_VARARGS, "mgr.remove(trigger)\nRemove trigger from manager. Corresponding AOI events could be fired."},
	{NULL,NULL,0,NULL},
};

static PyTypeObject pyaoi_ManagerType = {
	PyObject_HEAD_INIT(NULL)
	0,							/*ob_size*/
	"pyaoi.manager",			/*tp_name*/
	sizeof(pyaoi_Manager),		/*tp_basicsize*/
	0,							/*tp_itemsize*/
	(destructor)Manager_Del,	/*tp_dealloc*/
	0,							/*tp_print*/
	0,							/*tp_getattr*/
	0,							/*tp_setattr*/
	0,							/*tp_compare*/
	0,							/*tp_repr*/
	0,							/*tp_as_number*/
	0,							/*tp_as_sequence*/
	0,							/*tp_as_mapping*/
	0,							/*tp_hash */
	0,							/*tp_call*/
	0,							/*tp_str*/
	0,							/*tp_getattro*/
	0,							/*tp_setattro*/
	0,							/*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,			/*tp_flags*/
	"AOI Manager",				/* tp_doc */
	0,							/* tp_traverse */
    0,							/* tp_clear */
    0,							/* tp_richcompare */
    0,							/* tp_weaklistoffset */
    0,							/* tp_iter */
    0,							/* tp_iternext */
    Manager_methods,			/* tp_methods */
    0,							/* tp_members */
    Manager_getsets,			/* tp_getset */
};

//----------------------------------------------------------------------------

PyObject* pyaoi_GetVersion (PyObject*, PyObject*)
{
	return PyString_FromFormat("%s.%s.%s",PYAOI_VERSION_MAJOR,PYAOI_VERSION_MINOR,PYAOI_VERSION_REVISION);
}

PyObject* pyaoi_CreateTrigger (PyObject*, PyObject *args, PyObject *kwds)
{
	static char *kwdlist[] = {"name", "size", "scene_obj", "callback_enter", "callback_leave", "group", "group_filter", NULL};
	PyObject *name=NULL, *scene_obj=Py_None, *size=NULL, *on_enter=Py_None, *on_leave=Py_None;
	int group, filter;
	if(!PyArg_ParseTupleAndKeywords(args,kwds,"|SOOOOii",kwdlist,&name,&size,&scene_obj,&on_enter,&on_leave,&group,&filter))
	{
		return NULL;
	}
	xvec3f_t radius;
	radius.zero();
	if(NULL!=size)
	{
		if(!PyArg_ParseTuple(size,"f|ff",&radius.x,&radius.y,&radius.z))
			return NULL;
	}

	if(on_enter!=Py_None && !PyCallable_Check(on_enter))
	{
		PyErr_SetString(PyExc_TypeError,"callback_enter must be callable: void on_enter (trigger_name, scene_obj)");
		return NULL;
	}
	if(on_enter!=Py_None && !PyCallable_Check(on_leave))
	{
		PyErr_SetString(PyExc_TypeError,"callback_leave must be callable: void on_leave (trigger_name, scene_obj)");
		return NULL;
	}
	pyaoi_Trigger *trigger = (pyaoi_Trigger*)pyaoi_TriggerType.tp_alloc(&pyaoi_TriggerType,0);
	if(trigger) {
		trigger->_entity = new pyaoi_Entity(trigger,radius);
		trigger->_entity->set_mask(group);
		trigger->_entity->set_filter(filter);
		trigger->_parent=NULL;
		if(name) {
			Py_INCREF(name);
			trigger->_name = name;
		}
		else trigger->_name = Py_BuildValue("s","");
		Py_INCREF(scene_obj);
		trigger->_scene_obj = scene_obj;
		Py_INCREF(on_enter);
		trigger->_on_enter = on_enter;
		Py_INCREF(on_leave);
		trigger->_on_leave = on_leave;
	}

	return (PyObject*)trigger;
}

PyObject* pyaoi_CreateManager (PyObject*, PyObject *args, PyObject *kwds)
{
	static char *kwdlist[] = {"dimension", NULL};
	int dimension = 3;
	if(!PyArg_ParseTupleAndKeywords(args,kwds,"|i",kwdlist,&dimension))
	{
		return NULL;
	}
	if(dimension<=0 || dimension>3)
	{
		PyErr_SetString(PyExc_ValueError,"dimension must in [1,3]!");
		return NULL;
	}
	pyaoi_Manager *mgr = (pyaoi_Manager*)pyaoi_ManagerType.tp_alloc(&pyaoi_ManagerType,0);
	if(mgr) {
		mgr->_mgr = new xaoi_manager();
		mgr->_dimension = dimension;
	}
	return (PyObject*)mgr;
}

// create_trigger(name, scene_obj, size_tuple, callback_enter, callback_leave£¬group, group_filter)
static PyMethodDef pyaoi_methods[] = 
{
	{"version", (PyCFunction)pyaoi_GetVersion, METH_NOARGS, "pyaoi.version()\nReturn the version string of pyaoi module"},
	{"create_manager", (PyCFunction)pyaoi_CreateManager, METH_KEYWORDS, "mgr=pyaoi.create_manager(dimension=3)\nCrate a trigger manager. The 'dimension' parameter indicates what type of world (1D, 2D or 3D) the client needs. It's a hint that implementation could make optimizations occording to. If a trigger object has been added to manager, corresponding AOI events could be fired when the trigger moves. Currently there're two AOI events could be fired. One is 'enter event', which is fired when one trigger enter another trigger's scope. The other is 'leave event', which is fired when one trigger leave another trigger's scope."},
	{"create_trigger", (PyCFunction)pyaoi_CreateTrigger, METH_KEYWORDS, "tr=pyaoi.create_trigger(name='', size=(x,y,z), scene_obj=None, callback_enter=None, callback_leave=None, group=0, group_filter=0)\nCrate a trigger object with the specified name and size. The size could be 1/2/3D tuple. It indicates the radius of trigger's scope. A scene object could be associated with the trigger which will be passed to the callback function. When callback_enter and callback_leave must be a callable object (or None) with the form 'void callable (scene_obj, trigger_name, other_scene_obj, other_trigger_name)'. callback_enter is invoked when other triggers enter its scope, and callback_leave is invoked when leave. One trigger's callbacks will be invoked only if (self.group_filter & other.group)!=0"},
	{NULL,NULL,0,NULL},
};

PyMODINIT_FUNC
initpyaoi(void)
{
	PyObject *m = Py_InitModule("pyaoi",pyaoi_methods);

	if(PyType_Ready(&pyaoi_TriggerType)<0)
		return;
	Py_INCREF(&pyaoi_TriggerType);
	PyModule_AddObject(m,"trigger",(PyObject*)&pyaoi_TriggerType);
	if(PyType_Ready(&pyaoi_ManagerType)<0)
		return;
	Py_INCREF(&pyaoi_ManagerType);
	PyModule_AddObject(m,"manager",(PyObject*)&pyaoi_ManagerType);
}

} // extern "C"
