#ifndef __HEADER_PY_PLT
#define __HEADER_PY_PLT

#include <cassert>
#include <Python.h>
#include <structmember.h>

#include "collision_detector.h"
#include "move.h"

extern "C" {

typedef struct
{
	PyObject_HEAD
	//----------------------
	wyc::xquad_tree *_map;
	unsigned _count;
} pyplt_Scene;

typedef struct
{
	PyObject_HEAD
	//----------------------
	pyplt_Scene *_parent;
	wyc::xcollision_agent *_aabb;
} pyplt_Agent;

typedef struct
{
	PyObject_HEAD
	//----------------------
	pyplt_Agent *_agent;
	pyplt_Agent *_terrain;
	wyc::xmove *_move;
} pyplt_Move;

extern PyTypeObject pyplt_AgentType;
extern PyTypeObject pyplt_MoveType;

} // extern "C"

#endif // __HEADER_PY_PLT
