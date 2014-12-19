#ifndef __HEADER_WYC_MEMDEF
#define __HEADER_WYC_MEMDEF

#include "wyc/config.h"
#include "wyc/mem/membase.h"

#ifdef WYC_MEMDEBUG
	#define wycnew new(__FILE__,__LINE__,__FUNCTION__)
	#define wycdel delete
	#define wyc_malloc(size) debug_malloc(size,__FILE__,__LINE__,__FUNCTION__)
	#define wyc_free(ptr) debug_free(ptr)
#else
	#define wycnew new
	#define wycdel delete
	#define wyc_malloc(size) malloc(size)
	#define wyc_free(ptr) free(ptr)
#endif // WYC_MEMDEBUG


#endif // end of __HEADER_WYC_MEMDEF

