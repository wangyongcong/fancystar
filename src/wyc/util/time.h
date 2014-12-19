#ifndef __HEADER_WYC_XTIME
#define __HEADER_WYC_XTIME

///////////////////////////////////////////////////////////////////////////////
//  时钟和定时器。
///////////////////////////////////////////////////////////////////////////////

#include <cstring>
#include <ctime>
#include <map>
#include "wyc/platform.h"

namespace wyc 
{

//
// 高精度时间
//
// 默认实现用标准C函数,精度为毫秒
// Windows下用Performance Counter,精度为微秒
class xtime_source
{
	double m_secsPerTick;
public:
	static inline xtime_source& singleton() {
		static xtime_source ls_timesrc;
		return ls_timesrc;
	}
	xtime_source() {
#if defined(_WIN32) || defined(_WIN64)
		LARGE_INTEGER freq;
		::QueryPerformanceFrequency(&freq);
		m_secsPerTick = 1.0/freq.QuadPart;
#else
		m_secsPerTick = 1.0/CLOCKS_PER_SEC;
#endif
	}
	inline double get_time() const {
#if defined(_WIN32) || defined(_WIN64)
		LARGE_INTEGER time;
		::QueryPerformanceCounter(&time);
		return time.QuadPart * m_secsPerTick;
#else
		return clock()*m_secsPerTick;
#endif
	}
};

//
// 代码计时器
//
class xcode_timer
{
	double m_usedTime;
public:
	inline void start() {
		m_usedTime=xtime_source::singleton().get_time();
	}
	inline void stop() {
		m_usedTime=xtime_source::singleton().get_time()-m_usedTime;
	}
	inline double get_time() const {
		return m_usedTime;
	}
};

//
// 函数调用统计
//
class xcode_statist
{
public:
	struct CALLINFO
	{
		unsigned called;
		double time_used;
		double max_used;
	};
private:
	typedef std::map<std::string,CALLINFO*> xfuncmap_t;
	xfuncmap_t m_funcmap;
	CALLINFO *m_pci;
	double m_tbeg;
	xcode_statist();
public:
	static inline xcode_statist& singleton() {
		static xcode_statist *ls_pstatist=new xcode_statist;
		return *ls_pstatist;
	}
	void func_begin(const std::string &funcName);
	void func_end();
	const CALLINFO* get_called_info(const std::string &funcName) const;
	void report() const;
};

//
// 系统日期
//
class xdate
{
private:
	tm	m_tm;
public:
	xdate() {}
	~xdate() {}
	inline void get_date();
	int year() const;	// 1900-????
	int month() const;	// 1-12
	int yday() const;	// 1-356
	int mday() const;	// 1-31
	int wday() const;	// 0-6, 0 is Sunday
	int hour() const;	// 0-23
	int minute() const;	// 0-59
	int second() const;	// 0-59
	// daylight saving time
	bool is_dst() const;
	// am or pm
	bool is_am() const;	
	bool is_pm() const;
};

inline void xdate::get_date()
{
	time_t t; time(&t);
#ifdef _MSC_VER
	localtime_s(&m_tm,&t);	// for MSVC 2005
#else
	memcpy(&m_tm,localtime(&t),sizeof(tm));
#endif
}
inline int xdate::year() const
{
	return m_tm.tm_year+1900;
}
inline int xdate::month() const
{
	return m_tm.tm_mon+1;
}
inline int xdate::yday() const
{
	return m_tm.tm_yday+1;
}
inline int xdate::mday() const
{
	return m_tm.tm_mday;
}
inline int xdate::wday() const
{
	return m_tm.tm_wday;
}
inline int xdate::hour() const
{
	return m_tm.tm_hour;
}
inline int xdate::minute() const
{
	return m_tm.tm_min;
}
inline int xdate::second() const
{
	return m_tm.tm_sec;
}
inline bool xdate::is_dst() const
{
	return m_tm.tm_isdst>0;
}
inline bool xdate::is_am() const
{
	return m_tm.tm_hour<=12;
}
inline bool xdate::is_pm() const
{
	return !is_am();
}

} // namespace wyc

#endif // end of __HEADER_WYC_XTIME

