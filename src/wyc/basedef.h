#pragma once

#ifndef __HEADER_BASEDEF__
#define __HEADER_BASEDEF__

#include <cstdint>

namespace wyc 
{

typedef float  float32_t;
typedef double float64_t;
typedef float32_t* pfloat32_t;
typedef float64_t* pfloat64_t;

// 基本数据类型

#ifndef _X_BASE_TYPE_DEFINE
#define _X_BASE_TYPE_DEFINE

/*// interger type

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;

#if (defined _WIN32) || (defined _WIN64)
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
typedef long long int64_t;
typedef unsigned long long uint64_t;
#endif // _WIN32

typedef int8_t* pint8_t;
typedef uint8_t* puint8_t;
typedef int16_t* pint16_t;
typedef uint16_t* puint16_t;
typedef int32_t* pint32_t;
typedef uint32_t* puint32_t;
typedef int64_t* pint64_t;
typedef uint64_t* puint64_t;*/

// float type
typedef float float32_t;
typedef double float64_t;
typedef float32_t* pfloat32_t;
typedef float64_t* pfloat64_t;

#endif // end of _X_BASE_TYPE_DEFINE

#ifndef WYC_LOG_LEVEL
#define WYC_LOG_LEVEL
enum LOG_LEVEL {
	LOG_NORMAL=0,
	LOG_SYS,
	LOG_WARN,
	LOG_ERROR,
	LOG_FATAL,
	LOG_LEVEL_COUNT,
};
#endif // WYC_LOG_LEVEL

} // namespace wyc

#ifdef WYCLIB
	#undef WYC_EXPORT
	#undef WYC_IMPORT
#endif
#ifdef WYC_EXPORT
	#undef WYC_IMPORT
	#define WYCAPI __declspec(dllexport)
#elif WYC_IMPORT
	#define WYCAPI __declspec(dllimport)
#else
	#define WYCAPI
#endif

#define EXPORT_MODULE(module_name) void module_wyc_##module_name(void) {}
#define IMPORT_MODULE(module_name)\
	extern void module_wyc_##module_name(void);\
	module_wyc_##module_name();

#endif // end of __HEADER_BASEDEF__
