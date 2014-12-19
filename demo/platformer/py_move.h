#ifndef __HEADER_PY_MOVE
#define __HEADER_PY_MOVE

#include "py_plt.h"

extern "C" {

void Move_Del (pyplt_Move *self);

PyObject* Move_GetAgent (pyplt_Move *self, PyObject *args);
int Move_SetAgent (pyplt_Move *self, PyObject *args, void*);
PyObject* Move_GetTerrain (pyplt_Move *self, PyObject *args);
PyObject* Move_StickOnGround (pyplt_Move *self, PyObject *args);
PyObject* Move_LeaveGround (pyplt_Move *self, PyObject *args);
PyObject* Move_Walk (pyplt_Move *self, PyObject *args);
PyObject* Move_Fall (pyplt_Move *self, PyObject *args);
PyObject* Move_Jump (pyplt_Move *self, PyObject *args);
PyObject* Move_Fly  (pyplt_Move *self, PyObject *args);
PyObject* Move_FlushKeyPos (pyplt_Move *self, PyObject *args);
PyObject* Move_GetKeyPosAsList (pyplt_Move *self, PyObject *args);
PyObject* Move_Detail (pyplt_Move *self, PyObject *args);

} // extern "C"

#endif // __HEADER_PY_MOVE
