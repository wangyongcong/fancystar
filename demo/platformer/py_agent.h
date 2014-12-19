#ifndef __HEADER_PY_AGENT
#define __HEADER_PY_AGENT

#include "py_plt.h"

extern "C" {

// agent base
void Agent_Del (pyplt_Agent *self);

PyObject* Agent_GetParent (pyplt_Agent *self, PyObject *args);
PyObject* Agent_GetPos (pyplt_Agent *self, PyObject *args);
int Agent_SetPos (pyplt_Agent *self, PyObject *args, void*);
PyObject* Agent_GetRadius (pyplt_Agent *self, PyObject *args);
int Agent_SetRadius (pyplt_Agent *self, PyObject *args, void*);
PyObject* Agent_GetType (pyplt_Agent *self, PyObject *args);
PyObject* Agent_GetGroup (pyplt_Agent *self, PyObject *args);
int Agent_SetGroup (pyplt_Agent *self, PyObject *args, void*);
PyObject* Agent_GetBoundingBox (pyplt_Agent *self, PyObject *args);
PyObject* Agent_GetSurfaceNormal (pyplt_Agent *self, PyObject *args);

} // extern "C"

#endif // __HEADER_PY_AGENT
