#include <cassert>
#include <cstring>
#include "wyc/platform.h"
#include "wyc/util/fpath.h"
#include "wyc/util/util.h"

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4996)
#endif // _MSC_VER

namespace wyc
{

#if defined(_WIN32) || defined(_WIN64)
	const char xfilepath::ms_splitter='\\';
#else
	const char xfilepath::ms_splitter='/';
#endif

void get_current_directory(char *pstr, unsigned max_size)
{
#if defined(_WIN32) || defined(_WIN64)
	::GetCurrentDirectoryA(max_size,pstr);
#else
	wyc_error("can't get current directory");
#endif
}

inline bool is_splitter(char c) {
	return c=='/' || c=='\\';
}

inline bool is_nstd_splitter(char c) {
#if defined(_WIN32) || defined(_WIN64)
	return c=='/';
#else 
	return c=='\\';
#endif
}

char* combine_path(char *iter, const char *beg, const char *end, const char *path, char** psplitter)
{
	assert(beg<=end);
	assert(iter>=beg);
	assert(iter<=end);
	char *splitter=iter;
	const char *read=path;
	unsigned len;
	bool volume=false;
	char c;
	while(iter!=end) {
		c=*read++;
		if(c==0)
			break;
		if(!is_splitter(c)) {
			*iter++=c;
			continue;
		}
		len=iter-splitter;
		if(len<1) 
			continue;
		if(*splitter=='.') {
			if(len==1) {
				iter=splitter;
				continue;
			}
			else if(len==2 && splitter[1]=='.') {
				if(splitter==beg) {
					if(volume) {
						iter=const_cast<char*>(beg);
						continue;
					}
					beg=iter+1;
				}
				else {
					char *last=splitter-2;
					while(last>beg) {
						if(*last==xfilepath::ms_splitter) {
							++last;
							break;
						}
						--last;
					}
					assert(last>=beg);
					len=splitter-last-1;
					if(len==2 && *last=='.' && last[1]=='.') {
						beg=iter+1;
					}
					else if(last[len-1]==':') {
						iter=splitter;
						beg=splitter;
						volume=true;
						continue;
					}
					else {
						iter=last;
						splitter=last;
						continue;
					}
				}
			}
		}
		else if(splitter[len-1]==':') {
			beg=iter+1;
			volume=true;
		}
		*iter++=xfilepath::ms_splitter;
		splitter=iter;
	}
	if(psplitter)
		*psplitter=splitter;
	return iter;
}

xfilepath::xfilepath(const char *path) 
{
	set_fpath(path);
}

void xfilepath::set_workdir(const char *curdir)
{
	if(!curdir || curdir[0]==0) {
		goto RESET_PATH;
	}
	char *iter, *end=m_path+FILENAME_MAX;
	iter=combine_path(m_path,m_path,end,curdir,&m_wpos);
	if(iter==end) {
		wyc_warn("path is too long: %s",curdir);
		goto RESET_PATH;
	}
	if(iter!=m_wpos) {
		*iter++=ms_splitter;
		if(iter==end) {
			wyc_warn("path is too long: %s",curdir);
			goto RESET_PATH;
		}
		m_wpos=iter;
	}
	*iter=0;
	m_size=iter-m_path;
	return;
RESET_PATH:
	m_wpos=m_path;
	*m_wpos=0;
	m_size=0;
}

void xfilepath::chg_workdir(const char *reldir)
{
	if(!reldir || reldir[0]==0) 
		return;
	char *iter, *end=m_path+FILENAME_MAX;
	iter=combine_path(m_wpos,m_path,end,reldir,&m_wpos);
	if(iter==end) {
		wyc_warn("path is too long: %s",reldir);
		goto RESET_PATH;
	}
	if(iter!=m_wpos) {
		*iter++=ms_splitter;
		if(iter==end) {
			wyc_warn("path is too long: %s",reldir);
			goto RESET_PATH;
		}
		m_wpos=iter;
	}
	*iter=0;
	m_size=iter-m_path;
	return;
RESET_PATH:
	m_wpos=m_path;
	*m_wpos=0;
	m_size=0;
}

void xfilepath::set_fpath(const char *path)
{
	if(!path || path[0]==0) {
		goto RESET_PATH;
	}
	char *iter, *end=m_path+FILENAME_MAX;
	iter=combine_path(m_path,m_path,end,path,&m_wpos);
	if(iter==end) {
		wyc_warn("path is too long: %s",path);
		goto RESET_PATH;
	}
	*iter=0;
	m_size=iter-m_path;
	return;
RESET_PATH:
	m_wpos=m_path;
	*m_wpos=0;
	m_size=0;
}

void xfilepath::chg_fpath(const char *relpath)
{
	if(!relpath || relpath[0]==0) 
		return;
	char *iter, *end=m_path+FILENAME_MAX;
	iter=combine_path(m_wpos,m_path,end,relpath,&m_wpos);
	if(iter==end) {
		wyc_warn("path is too long: %s",relpath);
		goto RESET_PATH;
	}
	*iter=0;
	m_size=iter-m_path;
	return;
RESET_PATH:
	m_wpos=m_path;
	*m_wpos=0;
	m_size=0;
}

void xfilepath::set_fname(const char *filename) {
	if(!filename || filename[0]==0) {
		*m_wpos=0;
		m_size=m_wpos-m_path;
		return;
	}
	char *iter=m_wpos, *end=m_path+FILENAME_MAX;
	while(iter<end) {
		char c=*filename;
		if(is_nstd_splitter(c))
			*iter=ms_splitter;
		else *iter=c;
		if(c==0)
			break;
		++iter;
		++filename;
	}
	if(iter==end) {
		*(--iter)=0;
		wyc_warn("path is too long: %s",filename);
	}
	m_size=iter-m_path;
}

void xfilepath::append_subpath(const char *pathname)
{
	if(!pathname || pathname[0]==0) 
		return;
	char *iter=m_path+m_size, *end=m_path+FILENAME_MAX;
	while(iter<end) {
		char c=*pathname;
		if(is_nstd_splitter(c))
			*iter=ms_splitter;
		else *iter=c;
		if(c==0)
			break;
		++iter;
		++pathname;
	}
	if(iter==end) {
		*(--iter)=0;
		wyc_warn("path is too long: %s",pathname);
	}
	m_size=iter-m_path;
}

const char* xfilepath::basename() const {
	const char *riter=m_path+m_size;
	while(riter>m_wpos) {
		--riter;
		if(*riter==ms_splitter) {
			riter+=1;
			break;
		}
	}
	return riter;
}

const char* xfilepath::extname() const {
	const char *riter=m_path+m_size;
	while(riter>m_wpos) {
		--riter;
		if(*riter=='.')
			return riter+1;
		if(*riter==ms_splitter) 
			break;
	}
	return wyc::EMPTY_CSTRING;
}

unsigned xfilepath::get_workdir(char *strbuff, unsigned max_size) const
{
	unsigned len=m_wpos-m_path;
	if(strbuff!=0) {
		if(len>=max_size)
			len=max_size-1;
		strncpy(strbuff,m_path,len);
		strbuff[len]=0;
	}
	return len;
}

void xfilepath::absdir()
{
#if defined(_WIN32) || defined(_WIN64)
	m_size=::GetCurrentDirectoryA(FILENAME_MAX,m_path);
	assert(m_size<FILENAME_MAX);
	if(m_path[m_size-1]!=ms_splitter && m_size+1<FILENAME_MAX) {
		m_path[m_size++]=ms_splitter;
		m_path[m_size]=0;
	}
	m_wpos=m_path+m_size;
#endif // WIN32
}

#ifdef _MSC_VER
#pragma warning (pop)
#endif // _MSC_VER

} // namespace wyc


