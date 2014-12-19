#include "wyc/platform.h"
#include "wyc/log/log.h"
#include <string>
#include "wyc/util/util.h"
#include "wyc/util/strutil.h"
#include "wyc/util/time.h"

namespace wyc 
{

const char* s_prefix[wyc::LOG_LEVEL_COUNT] = {
	"INFO",		// LOG_NORMAL
	"SYS",		// LOG_SYS
	"WARN",		// LOG_WARN
	"ERROR",	// LOG_ERROR
	"FATAL",	// LOG_FATAL
};

class xlog
{
	CRITICAL_SECTION m_outputLock;
	FILE *m_hfile;
	SYSTEMTIME m_time;
	std::string m_path;
	bool m_utf8;
public:
	xlog() 
	{
		m_hfile=0;
		m_utf8=true;
	}
	bool create(const char *log_file, const char *log_dir, wyc::LOG_LEVEL log_level, bool utf8) 
	{
		if(!log_dir || 0==log_dir[0])
			m_path="./";
		else {
			m_path=log_dir;
			char splitter=m_path[m_path.size()-1];
			if(splitter!='/' && splitter!='\\')
				m_path+="/";
		}
		m_path+=log_file;
		m_hfile = _fsopen(m_path.c_str(),"w",_SH_DENYWR);
		if(!m_hfile) {
			std::string cmd = "mkdir ";
			cmd+=log_dir;
			if(system(cmd.c_str()))
				return false;
			m_hfile = _fsopen(m_path.c_str(),"w",_SH_DENYWR);
			if(!m_hfile) 
				return false;
		}
		if(!InitializeCriticalSectionAndSpinCount(&m_outputLock,4000))
			return false;
		m_utf8 = utf8;
		return true;
	}
	void destroy()
	{
		if(m_hfile) {
			fclose(m_hfile);
			m_hfile=0;
		}
		DeleteCriticalSection(&m_outputLock);
	}
	inline void lock()
	{
		EnterCriticalSection(&m_outputLock);
	}
	inline void unlock()
	{
		LeaveCriticalSection(&m_outputLock);
	}
	inline void write(LOG_LEVEL level, const char *text)
	{
		if(m_hfile) {
			GetLocalTime(&m_time);
			fprintf(m_hfile,"[%02d:%02d:%02d %03d] [%s] %s\n",m_time.wHour,m_time.wMinute,
				m_time.wSecond,m_time.wMilliseconds,s_prefix[level],text);
			fflush(m_hfile);
		}
	}
	inline bool is_utf8() const {
		return m_utf8;
	}
};

xlog *s_logger=0;

}; // namespace wyc

using namespace wyc;

#define DEFAULT_LOG_BUFFER 512

#ifdef 	__cplusplus
extern "C"
{
#endif 

void wyc_log_init(const char *name, const char *log_dir, wyc::LOG_LEVEL log_level, bool utf8)
{
	if(s_logger)
		return;
	s_logger = new xlog();
	std::string file_name=name;
	if(file_name.empty())
		file_name="fs-client.log";
	else
		file_name+=".log";
	s_logger->create(file_name.c_str(), log_dir, log_level, utf8);
}

void wyc_log_close()
{
	if(s_logger) {
		s_logger->destroy();
		s_logger=0;
	}
}

void wyc_log(wyc::LOG_LEVEL level, const char *format, ...)
{
	if(!s_logger)
		return;

	char buff[DEFAULT_LOG_BUFFER], *palloc=0;
	const char *pinfo;
	va_list arglist;
	va_start(arglist,format);
	int cnt=::vsprintf_s(buff,DEFAULT_LOG_BUFFER,format,arglist);
	if(cnt+1<=DEFAULT_LOG_BUFFER) {		
		buff[cnt]=0;
		pinfo=buff;
	}
	else {
		cnt=::_vscprintf(format,arglist);
		palloc=new char[cnt+1];
		::vsprintf_s(palloc,cnt,format,arglist);
		palloc[cnt]=0;
		pinfo=palloc;
	}
	va_end(arglist);

	s_logger->lock();
	s_logger->write(level,pinfo);
	s_logger->unlock();

	if(palloc) {
		delete [] palloc;
		palloc=0;
	}
}

void wyc_logw(wyc::LOG_LEVEL level, const wchar_t *format, ...)
{
	if(!s_logger)
		return;
	wchar_t buff[DEFAULT_LOG_BUFFER], *palloc=0;
	const wchar_t *pinfo;
	va_list arglist;
	va_start(arglist,format);
	int cnt=::vswprintf_s(buff,DEFAULT_LOG_BUFFER,format,arglist);
	if(cnt+1<=DEFAULT_LOG_BUFFER) {
		buff[cnt]=0;
		pinfo=buff;
	}
	else {
		cnt=::_vscwprintf(format,arglist);
		palloc=new wchar_t[cnt+1];
		::vswprintf_s(palloc,cnt,format,arglist);
		palloc[cnt]=0;
		pinfo=palloc;
	}
	va_end(arglist);
	
	std::string s;
	if(s_logger->is_utf8())
		wstr2str_utf8(s,pinfo,cnt);
	else
		wstr2str(s,pinfo,cnt);
	if(palloc) {
		delete [] palloc;
		palloc=0;
	}

	s_logger->lock();
	s_logger->write(level,s.c_str());
	s_logger->unlock();
}

#ifdef __cplusplus
};
#endif
