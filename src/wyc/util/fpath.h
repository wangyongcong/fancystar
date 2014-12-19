#ifndef __HEADER_WYC_FPATH
#define __HEADER_WYC_FPATH

#include <cstring>
#include <string>

namespace wyc
{

class xfilepath
{
	char m_path[FILENAME_MAX];
	char *m_wpos;
	unsigned m_size;
public:
	static const char ms_splitter;
	xfilepath(const char *curdir=0);
	xfilepath& operator = (const char *curdir);
	void set_workdir(const char *curdir);
	void chg_workdir(const char *reldir);
	void set_fpath(const char *path);
	void chg_fpath(const char *relpath);
	void set_fname(const char *filename);
	void append_subpath(const char *pathname);
	unsigned size() const;
	const char* get_path() const;
	const char* basename() const;
	const char* extname() const;
	void get_workdir(std::string &ret) const;
	unsigned get_workdir(char *strbuff, unsigned max_size) const;
	void absdir();
};

inline const char* xfilepath::get_path() const {
	return m_path;
}

inline unsigned xfilepath::size() const {
	return m_size;
}

inline void xfilepath::get_workdir(std::string &ret) const
{
	ret.assign(m_path,m_wpos);
}

inline xfilepath& xfilepath::operator = (const char *path)
{
	set_fpath(path);
	return *this;
}

char* combine_path(char *iter, const char *beg, const char *end, const char *path, char** psplitter=0);

} // namespace wyc

#endif // end of __HEADER_WYC_FPATH


