#ifndef __HEADER_SYSTEM_CONFIGURATION
#define __HEADER_SYSTEM_CONFIGURATION

//------------------------------------------------------------------------------
// 版本信息
//------------------------------------------------------------------------------

#define FANCYSTAR_MAJOR_VERSION	1
#define FANCYSTAR_MINOR_VERSION	1

//------------------------------------------------------------------------------
// memory
//------------------------------------------------------------------------------

#ifdef _WIN32
	#define WYC_SYS32
#elif _WIN64
	#define WYC_SYS64
#endif

// size of CPU cache line
#define WYC_CACHE_LINE 8

// memory alignment declaration
#ifdef _MSC_VER
	#define WYC_CACHE_ALIGN __declspec(align(8))
	#define WYC_ALIGN(s) __declspec(align(s))
#else
	NOT_IMPLEMENT
#endif // _MSC_VER

#define IS_CACHE_ALIGN(ptr) (((uintptr_t)(ptr)&(WYC_CACHE_LINE-1))==0)

// memory alignment
#define WYC_MEMORY_ALIGNMENT 8
#define WYC_MEMORY_ALIGNMENT_MASK 7

// 补齐memory size使其为WYC_MEMORY_ALIGNMENT的倍数
#define MEMORY_PADDING(size) ((size+WYC_MEMORY_ALIGNMENT_MASK)&~WYC_MEMORY_ALIGNMENT_MASK)

#endif // __HEADER_SYSTEM_CONFIGURATION

