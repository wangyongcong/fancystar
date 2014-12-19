#ifndef __HEADER_WYC_XLOG
#define __HEADER_WYC_XLOG

#ifdef WYC_EXPORT_LOG
	#undef WYC_IMPORT_LOG
	#define WYCAPI_LOG __declspec(dllexport)
#elif WYC_IMPORT_LOG
	#define WYCAPI_LOG __declspec(dllimport)
#else
	#define WYCAPI_LOG
#endif

typedef int (*CommandHandler) (const char *argv[], unsigned argn);

#ifdef 	__cplusplus
extern "C"
{
#endif 

WYCAPI_LOG void wyc_log_init(const char *name, const char *log_dir, int log_level);

WYCAPI_LOG void wyc_log(int level, const char *format, ...);

WYCAPI_LOG void wyc_logw(int level, const wchar_t *format, ...);

WYCAPI_LOG bool wyc_regcmd(const char *cmd, CommandHandler handler);

#ifdef __cplusplus
};
#endif

#endif // __HEADER_WYC_XLOG
