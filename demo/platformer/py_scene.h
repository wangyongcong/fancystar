#ifndef __HEADER_PY_SCENE
#define __HEADER_PY_SCENE

#include "py_plt.h"

extern "C" {

void Scene_Del (pyplt_Scene *self);

PyObject* Scene_AddAgent (pyplt_Scene *self, PyObject *args);
PyObject* Scene_DelAgent (pyplt_Scene *self, PyObject *args);

PyObject* Scene_StaticTest (pyplt_Scene *self, PyObject *args);
PyObject* Scene_SweepTest (pyplt_Scene *self, PyObject *args);
PyObject* Scene_SweepTestEx (pyplt_Scene *self, PyObject *args);
PyObject* Scene_SweepIntersect (pyplt_Scene *self, PyObject *args);

PyObject* Scene_GetSize (pyplt_Scene *self, PyObject *args);
PyObject* Scene_GetGridSize (pyplt_Scene *self, PyObject *args);
PyObject* Scene_GetLOD (pyplt_Scene *self, PyObject *args);
PyObject* Scene_GetAgentCount (pyplt_Scene *self, PyObject *args);

} // extern "C"

#endif // __HEADER_PY_SCENE
