#include "py_plt.h"
#include "py_agent.h"
#include "py_scene.h"
#include "py_move.h"

#if (PY_MAJOR_VERSION==2) && (PY_MINOR_VERSION==5)
	#pragma comment (lib, "python25.lib")
#elif (PY_MAJOR_VERSION==2) && (PY_MINOR_VERSION==7)
	#pragma comment (lib, "python27.lib")
#else
	#error Unsupported Python version!
#endif

#ifdef USE_LIBWYC
	#ifdef _DEBUG
		#pragma comment (lib, "fsmath_d.lib")
		#pragma comment (lib, "fsutil_d.lib")
	#else
		#pragma comment (lib, "fsmath.lib")
		#pragma comment (lib, "fsutil.lib")
	#endif // _DEBUG
#endif // LIBWYC

#define PYPLT_VERSION_MAJOR "1"
#define PYPLT_VERSION_MINOR "0"
#define PYPLT_VERSION_REVISION "7"

using namespace wyc;

extern "C" {

static PyGetSetDef Agent_getsets[] = 
{
	{(char*)"parent", (getter)Agent_GetParent, NULL, (char*)"Get the agent's parent which is a collision scene.", NULL},
	{(char*)"pos", (getter)Agent_GetPos, (setter)Agent_SetPos, (char*)"Get agent's position (x,y).", NULL},
	{(char*)"radius", (getter)Agent_GetRadius, (setter)Agent_SetRadius, (char*)"Get the agent's radius (half_w,half_h).", NULL},
	{(char*)"type", (getter)Agent_GetType, NULL, (char*)"Get the agent's collision primitive type.", NULL},
	{(char*)"group", (getter)Agent_GetGroup, (setter)Agent_SetGroup, (char*)"Get the agent's group.", NULL},
	{(char*)"bounding_box", (getter)Agent_GetBoundingBox, NULL, (char*)"Get the agent's world bounding as (lower_x,lower_y,upper_x,upper_y).", NULL},
	{(char*)"surface_normal", (getter)Agent_GetSurfaceNormal, NULL, (char*)"Get agent's upper surface normal as a normalized vector (nx,ny).", NULL},
	{NULL, NULL, NULL, NULL, NULL}  /* Sentinel */
};

PyTypeObject pyplt_AgentType = {
	PyObject_HEAD_INIT(NULL)
	0,							/*ob_size*/
	"pyplt.agent",				/*tp_name*/
	sizeof(pyplt_Agent),		/*tp_basicsize*/
	0,							/*tp_itemsize*/
	(destructor)Agent_Del,		/*tp_dealloc*/
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
	Py_TPFLAGS_DEFAULT 
	| Py_TPFLAGS_BASETYPE,		/*tp_flags*/
	"Platformer game object",	/* tp_doc */
	0,							/* tp_traverse */
	0,							/* tp_clear */
	0,							/* tp_richcompare */
	0,							/* tp_weaklistoffset */
	0,							/* tp_iter */
	0,							/* tp_iternext */
	0,							/* tp_methods */
	0,							/* tp_members */
	Agent_getsets,				/* tp_getset */
	0,							/* tp_base */
	0,							/* tp_dict */
	0,							/* tp_descr_get */
	0,							/* tp_descr_set */
	0,							/* tp_dictoffset */
	0,							/* tp_init */
	0,							/* tp_alloc */
	0,							/* tp_new */
	0,							/* tp_free */
	0,							/* tp_is_gc */
	0,							/* tp_bases */
	0,							/* tp_mro */
	0,							/* tp_cache */
	0,							/* tp_subclasses */
	0,							/* tp_weaklist */
	0,							/* tp_del */
};

//-------------------------------------------------------------------------------------------------------

static PyMethodDef Scene_methods[] = 
{
	{(char*)"add_agent", (PyCFunction)Scene_AddAgent, METH_VARARGS, (char*)"scn.add_agent(agent)"},
	{(char*)"del_agent", (PyCFunction)Scene_DelAgent, METH_VARARGS, (char*)"scn.del_agent(agent)"},
	{(char*)"static_test", (PyCFunction)Scene_StaticTest, METH_VARARGS, (char*)"scn.static_test(agent, filter)"},
	{(char*)"sweep_test", (PyCFunction)Scene_SweepTest, METH_VARARGS, (char*)"scn.sweep_test(agent, (x,y), filter)"},
	{(char*)"sweep_test_ex", (PyCFunction)Scene_SweepTestEx, METH_VARARGS, (char*)"scn.sweep_test(agent, (x,y), filter, normal_filter, (nx0,ny0), (nx1,ny1))"},
	{(char*)"sweep_intersect", (PyCFunction)Scene_SweepIntersect, METH_VARARGS, (char*)"scn.sweep_intersect(agent, (x,y), filter)"},
	{NULL,NULL,0,NULL} /* Sentinel */
};

static PyGetSetDef Scene_getsets[] = 
{
	{(char*)"size", (getter)Scene_GetSize, NULL, (char*)"Get the scene's size (width, height). Read only.", NULL},
	{(char*)"grid_size", (getter)Scene_GetGridSize, NULL, (char*)"Get the grid's level-0 size (width, height). Read only.", NULL},
	{(char*)"lod", (getter)Scene_GetLOD, NULL, (char*)"Get the scene's LOD count. Read only.", NULL},
	{(char*)"agent_count", (getter)Scene_GetAgentCount, NULL, (char*)"Get the count of agents in the scene. Read only.", NULL},
	{NULL, NULL, NULL, NULL, NULL}  /* Sentinel */
};

static PyTypeObject pyplt_SceneType = {
	PyObject_HEAD_INIT(NULL)
	0,							/*ob_size*/
	"pyplt.scene",				/*tp_name*/
	sizeof(pyplt_Scene),		/*tp_basicsize*/
	0,							/*tp_itemsize*/
	(destructor)Scene_Del,		/*tp_dealloc*/
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
	"Platformer game scene",	/* tp_doc */
	0,							/* tp_traverse */
	0,							/* tp_clear */
	0,							/* tp_richcompare */
	0,							/* tp_weaklistoffset */
	0,							/* tp_iter */
	0,							/* tp_iternext */
	Scene_methods,				/* tp_methods */
	0,							/* tp_members */
	Scene_getsets,				/* tp_getset */
	0,							/* tp_base */
	0,							/* tp_dict */
	0,							/* tp_descr_get */
	0,							/* tp_descr_set */
	0,							/* tp_dictoffset */
	0,							/* tp_init */
	0,							/* tp_alloc */
	0,							/* tp_new */
	0,							/* tp_free */
	0,							/* tp_is_gc */
	0,							/* tp_bases */
	0,							/* tp_mro */
	0,							/* tp_cache */
	0,							/* tp_subclasses */
	0,							/* tp_weaklist */
	0,							/* tp_del */
};

//-------------------------------------------------------------------------------------------------------

static PyMethodDef Move_methods[] = 
{
	{(char*)"stick_on_ground", (PyCFunction)Move_StickOnGround, METH_NOARGS, (char*)"move.stick_on_ground(), make the agent stand on ground"},
	{(char*)"leave_ground", (PyCFunction)Move_LeaveGround, METH_NOARGS, (char*)"move.leave_ground(), make the agent leave ground"},
	{(char*)"walk", (PyCFunction)Move_Walk, METH_VARARGS, (char*)"move.walk(t, spdx, extra_filter=0, max_iteration=3), make the agent walk on ground"},
	{(char*)"fall", (PyCFunction)Move_Fall, METH_VARARGS, (char*)"move.fall(t, spdx, spdy, accy, extra_filter=0, max_iteration=3), make the agent fall in the air until reaching the ground"},
	{(char*)"jump", (PyCFunction)Move_Jump, METH_VARARGS, (char*)"move.jump(t, spdx, spdy, accy, extra_filter=0, max_iteration=3), make the agent jump"},
	{(char*)"fly",  (PyCFunction)Move_Fly,  METH_VARARGS, (char*)"move.fly(t, spdx, spdy, extra_filter=0), make the agent fly in the air"},
	{(char*)"flush_kpos", (PyCFunction)Move_FlushKeyPos, METH_NOARGS, (char*)"Get the key position of previous movement."},
	{(char*)"detail", (PyCFunction)Move_Detail, METH_NOARGS, (char*)"Get the detail of call stack for debug purpose."},
	{NULL,NULL,0,NULL} /* Sentinel */
};

static PyGetSetDef Move_getsets[] = 
{
	{(char*)"agent", (getter)Move_GetAgent, (setter)Move_SetAgent, (char*)"Get/set the associated agent.", NULL},
	{(char*)"terrain", (getter)Move_GetTerrain, NULL, (char*)"Get current standing terrain, or None if the agent is in the air.", NULL},
	{(char*)"kpos", (getter)Move_GetKeyPosAsList, NULL, (char*)"Get the key position of previous movement.", NULL},
	{NULL, NULL, NULL, NULL, NULL}  /* Sentinel */
};

PyTypeObject pyplt_MoveType = {
	PyObject_HEAD_INIT(NULL)
	0,							/*ob_size*/
	"pyplt.move",				/*tp_name*/
	sizeof(pyplt_Move),			/*tp_basicsize*/
	0,							/*tp_itemsize*/
	(destructor)Move_Del,		/*tp_dealloc*/
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
	"Platformer agent move",	/* tp_doc */
	0,							/* tp_traverse */
	0,							/* tp_clear */
	0,							/* tp_richcompare */
	0,							/* tp_weaklistoffset */
	0,							/* tp_iter */
	0,							/* tp_iternext */
	Move_methods,				/* tp_methods */
	0,							/* tp_members */
	Move_getsets,				/* tp_getset */
	0,							/* tp_base */
	0,							/* tp_dict */
	0,							/* tp_descr_get */
	0,							/* tp_descr_set */
	0,							/* tp_dictoffset */
	0,							/* tp_init */
	0,							/* tp_alloc */
	0,							/* tp_new */
	0,							/* tp_free */
	0,							/* tp_is_gc */
	0,							/* tp_bases */
	0,							/* tp_mro */
	0,							/* tp_cache */
	0,							/* tp_subclasses */
	0,							/* tp_weaklist */
	0,							/* tp_del */
};

//-------------------------------------------------------------------------------------------------------

PyObject* pyplt_GetVersion (PyObject*, PyObject*)
{
	return PyString_FromFormat("%s.%s.%s",PYPLT_VERSION_MAJOR,PYPLT_VERSION_MINOR,PYPLT_VERSION_REVISION);
}

PyObject* pyplt_CreateAgent (PyObject*, PyObject *args, PyObject *kwds)
{
	static const char *kwdlist[] = {"x", "y", "half_w", "half_h", "type", "group", NULL};
	float x, y, half_w, half_h;
	int type, group;
	if(!PyArg_ParseTupleAndKeywords(args,kwds,"ffffii",(char**)kwdlist,&x,&y,&half_w,&half_h,&type,&group))
		return NULL;
	pyplt_Agent *agent = (pyplt_Agent*)pyplt_AgentType.tp_alloc(&pyplt_AgentType,0);
	if(!agent)
		return NULL;
	agent->_parent = 0;
	if(type<0 || type>=AGENT_TYPE_COUNT)
	{
		// unknown type
		Py_DECREF(agent);
		return NULL;
	}
	agent->_aabb = new wyc::xcollision_agent(type);
	if(half_w<0)
		half_w=-half_w;
	if(half_h<0)
		half_h=-half_h;
	agent->_aabb->set_position(x,y);
	agent->_aabb->set_radius(half_w,half_h);
	agent->_aabb->set_mask(group);
	agent->_aabb->set_data(agent);
	agent->_aabb->update_aabb();
	return (PyObject*)agent;
}

PyObject* pyplt_CreateScene (PyObject*, PyObject *args, PyObject *kwds)
{
	static const char *kwdlist[] = {"left", "bottom", "right", "top", "min_obj_w", "min_obj_h", NULL};
	wyc::xvec2f_t lower, upper, min_obj_size;
	if(!PyArg_ParseTupleAndKeywords(args,kwds,"ffffff",(char**)kwdlist,&lower.x,&lower.y,&upper.x,&upper.y,&min_obj_size.x,&min_obj_size.y))
		return NULL;
	unsigned scn_w, scn_h, grid_w, grid_h;
	scn_w = unsigned(std::ceil(upper.x-lower.x));
	scn_h = unsigned(std::ceil(upper.y-lower.y));
	grid_w= unsigned(std::ceil(min_obj_size.x*2));
	grid_h= unsigned(std::ceil(min_obj_size.y*2));
	if( (grid_w&(grid_w-1))!=0 ) 
		grid_w = wyc::power2(grid_w);
	if( (grid_h&(grid_h-1))!=0 ) 
		grid_h = wyc::power2(grid_h);
	unsigned col = (scn_w+grid_w-1)/grid_w, row = (scn_h+grid_h-1)/grid_h;
	assert(col>0 && row>0);
	pyplt_Scene *scene = (pyplt_Scene*) pyplt_SceneType.tp_alloc(&pyplt_SceneType,0);
	if(!scene)
		return NULL;
	scene->_map = new wyc::xquad_tree();
	if(!scene->_map->initialize(grid_w,grid_h,row,col)) {
		Py_DECREF(scene);
		return NULL;
	}
	scene->_map->set_translate(lower);
	scene->_count = 0;
	return (PyObject*)scene;
}

PyObject* pyplt_CreateMove (PyObject*, PyObject *args, PyObject *kwds)
{
	static const char *kwdlist[] = {"agent", "terrain", NULL};
	pyplt_Agent *agent, *terrain=0;
	if(!PyArg_ParseTupleAndKeywords(args, kwds, "O!|O", (char**)kwdlist, &pyplt_AgentType, &agent, &terrain))
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
	pyplt_Move *move = (pyplt_Move*) pyplt_MoveType.tp_alloc(&pyplt_MoveType,0);
	move->_agent = agent;
	Py_INCREF(move->_agent);
	move->_move = new wyc::xmove();
	move->_move->set_agent(agent->_aabb);
	if(terrain && PyObject_TypeCheck(terrain,&pyplt_AgentType)) {
		move->_move->set_terrain(terrain->_aabb);
		if(move->_move->get_terrain()==terrain->_aabb)
		{
			move->_terrain = terrain;
			Py_INCREF(terrain);
		}
	}
	return (PyObject*)move;
}

PyObject* pyplt_SetMoveEnv (PyObject*, PyObject *args, PyObject *kwds)
{
	static const char *kwdlist[] = {"terrain_filter", "platform_filter", NULL};
	unsigned terrain_filter, platform_filter;
	if(!PyArg_ParseTupleAndKeywords(args, kwds, "II", (char**)kwdlist, &terrain_filter, &platform_filter))
		return NULL;
	wyc::xmove::init_move_env(terrain_filter, platform_filter);
	Py_INCREF(Py_None);
	return Py_None;
}

// create_trigger(name, scene_obj, size_tuple, callback_enter, callback_leave£¬group, group_filter)
static PyMethodDef pyplt_methods[] = 
{
	{"version", (PyCFunction)pyplt_GetVersion, METH_NOARGS, "pyplt.version()\nReturn the version string of pyplt module"},
	{"create_scene", (PyCFunction)pyplt_CreateScene, METH_KEYWORDS, "scene = pyplt.create_scene(width, height, min_obj_w=32, min_obj_h=32)"},
	{"create_agent", (PyCFunction)pyplt_CreateAgent, METH_KEYWORDS, "agent = pyplt.create_agent(x, y, half_w, half_h, type, group)"},
	{"create_move",  (PyCFunction)pyplt_CreateMove, METH_KEYWORDS, "move = pyplt.create_move(agent, terrain=None), create a move object for the agent (can't be None), who stands on the specific terrain by default."},
	{"set_move_env", (PyCFunction)pyplt_SetMoveEnv, METH_KEYWORDS, "pyplt.set_move_env(terrain_filter, platform_filter), set the movement enviroment."},
	{NULL,NULL,0,NULL},
};

//-------------------------------------------------------------------------------------------------------

PyMODINIT_FUNC
initpyplt(void)
{
	PyObject *m = Py_InitModule("pyplt",pyplt_methods);
	// register class agent
	if(PyType_Ready(&pyplt_AgentType)<0)
		return;
	Py_INCREF(&pyplt_AgentType);
	PyModule_AddObject(m,"agent",(PyObject*)&pyplt_AgentType);
	// register class scene
	if(PyType_Ready(&pyplt_SceneType)<0)
		return;
	Py_INCREF(&pyplt_SceneType);
	PyModule_AddObject(m,"scene",(PyObject*)&pyplt_SceneType);
	// register class move
	if(PyType_Ready(&pyplt_MoveType)<0)
		return;
	Py_INCREF(&pyplt_MoveType);
	PyModule_AddObject(m,"move",(PyObject*)&pyplt_MoveType);
}

} // extern "C"
