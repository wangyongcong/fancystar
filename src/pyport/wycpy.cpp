#include <cassert>
#include <cstring>
#include <python.h>
#include "wycpy.h"

namespace wyc
{

PyObject* py_says(PyObject* self, PyObject *args)
{
	printf("[Yongcong Wang] says: Hello, friend! \n");
    return Py_BuildValue("");
}

PyObject* py_strhash(PyObject* self, PyObject *args)
{
    const char* msg = NULL;
	Py_ssize_t len;
	if(!PyArg_ParseTuple(args,"s#",&msg,&len))
        return NULL;
	int ret=hash_crc32(msg,len);
    return Py_BuildValue("I",ret);
}

} // namespace wyc

