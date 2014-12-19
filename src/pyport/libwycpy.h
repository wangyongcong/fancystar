#ifndef __HEADER_LIB_WYCPY
#define __HEADER_LIB_WYCPY

#ifdef WYCLIB_PYTHON
	#undef WYC_EXPORT_PYTHON
	#undef WYC_IMPORT_PYTHON
#endif
#ifdef WYC_EXPORT_PYTHON
	#undef WYC_IMPORT_PYTHON
	#define WYCAPI_PYTHON __declspec(dllexport)
#elif WYC_IMPORT_PYTHON
	#define WYCAPI_PYTHON __declspec(dllimport)
#else
	#define WYCAPI_PYTHON
#endif


#endif // end of __HEADER_LIB_WYCPY

