
// Python 3.0

#ifdef WYC_EXPORT_PYTHON

#include <cstring>
#include <cassert>
#include <python.h>
#include "libwycpy.h"
#include "wycpy.h"

#ifdef _DEBUG
	#pragma comment (lib, "fsutil_d.lib")
	#pragma comment (lib, "python30_d.lib")
#else
	#pragma comment (lib, "fsutil.lib")
	#pragma comment (lib, "python30.lib")
#endif

extern "C" {

struct module_state 
{
	PyObject *error;
};

#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))

static int wycpy_traverse(PyObject *m, visitproc visit, void *arg) 
{
	Py_VISIT(GETSTATE(m)->error);
	return 0;
}

static int wycpy_clear(PyObject *m) 
{
	Py_CLEAR(GETSTATE(m)->error);
	return 0;
}

static PyMethodDef wycpy_methods[] = 
{
	{"says", wyc::py_says, METH_VARARGS},
	{"strhash", wyc::py_strhash, METH_VARARGS},
    {NULL, NULL, NULL} // sentinel
};


static struct PyModuleDef wycpy_moduledef = 
{
	PyModuleDef_HEAD_INIT,
	"wycpy",
	NULL,
	sizeof(struct module_state),
	wycpy_methods,
	NULL,
	wycpy_traverse,
	wycpy_clear,
	NULL
};

WYCAPI_PYTHON PyObject* PyInit_wycpy(void)
{
    PyObject *module = PyModule_Create(&wycpy_moduledef);

    if (module == NULL)
        return NULL;
    struct module_state *st = GETSTATE(module);

    st->error = PyErr_NewException("wycpy.Error", NULL, NULL);
    if (st->error == NULL) {
        Py_DECREF(module);
        return NULL;
    }
    return module;
}

} // extern "C"

#endif // WYC_EXPORT_PYTHON

