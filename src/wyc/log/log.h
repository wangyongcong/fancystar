#ifndef __HEADER_WYC_LOG
#define __HEADER_WYC_LOG

#include "wyc/basedef.h"

#ifdef 	__cplusplus
extern "C"
{
#endif 

void wyc_log_init(const char *name, const char *log_dir, wyc::LOG_LEVEL log_level, bool utf8=true);

void wyc_log_close();

void wyc_log(wyc::LOG_LEVEL level, const char *format, ...);

void wyc_logw(wyc::LOG_LEVEL level, const wchar_t *format, ...);

#ifdef __cplusplus
};
#endif

#endif // __HEADER_WYC_LOG
