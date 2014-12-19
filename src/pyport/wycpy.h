#ifndef __HEADER_WYC_PYTHON
#define __HEADER_WYC_PYTHON

#include "xutil.h"
#include "xhash.h"

namespace wyc 
{

PyObject* py_says(PyObject* self, PyObject *args);
PyObject* py_strhash(PyObject* self, PyObject *args);


}; // namespace wyc

#endif


