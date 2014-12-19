#ifndef __HEADER_WYC_XCONFIG
#define __HEADER_WYC_XCONFIG

#include <cassert>
#include <string>
#include <map>
#include "wyc/basedef.h"
#include "wyc/util/var.h"

namespace wyc
{

class xconfig
{
	typedef std::map<std::string,intptr_t> SECTION_GROUP;
	typedef std::pair<std::string,intptr_t> SECTION_NODE;
	typedef std::map<std::string,std::string> SECTION;
	typedef std::pair<std::string,std::string> KEYNODE;
	typedef SECTION::const_iterator section_iterator;
	enum TOKEN {
		TOKEN_ERROR=-1,
		TOKEN_ERR=-1,
		TOKEN_EOF=0,
		TOKEN_IDENTIFIER=1,
		TOKEN_INTEGER,
		TOKEN_FLOAT,
		TOKEN_STRING,
		TOKEN_USTRING,
		TOKEN_LEFT_BRACKET='(',
		TOKEN_RIGHT_BRACKET=')',
		TOKEN_NEW_LINE='\n',
		TOKEN_SPACE=' ',
		TOKEN_COMMA=',',
	};
	enum READ_TOKEN_STATE {
		TOKEN_READ_NULL=0,
		TOKEN_READ_IDENTIFIER,
		TOKEN_READ_NUMBER,
		TOKEN_READ_HEX_NUMBER,
		TOKEN_READ_FLOAT,
		TOKEN_READ_STRING,
		TOKEN_READ_USTRING,
	};
	enum SCRIPT_STATE {
		SC_EXPECT_IDENTIFIER,
		SC_EXPECT_BRACKET,
		SC_EXPECT_PARAMETER,
		SC_EXPECT_COMMA,
		SC_EXPECT_END,
	};
	std::string m_cfgname;
	SECTION_GROUP m_sections;
	mutable SECTION *m_pcurmap;
	mutable std::string m_mapname;
	bool m_bChanged;
public:
	xconfig();
	~xconfig();
	bool load(const char *filename);
	bool save(const char *filename=0);
	void clear();
	inline const std::string& get_name() const {
		return m_cfgname;
	}
	inline bool need_save() const {
		return m_bChanged;
	}
	const std::string& cur_section() const {
		return m_mapname;
	}
	// 设置当前section,如果bAdd为true,则会自动创建
	bool set_cur_section(const std::string &secname, bool bAdd=false);
	// 清空当前section
	void clear_cur_section();
	// 删除section
	void remove_section(const std::string &secname);
	// 读取key
	bool get_key(const std::string &key, std::string &value) const;
	// 设置key,如果没有找到key则自动添加
	void set_key(const std::string &key, const std::string &value);
	// 删除key
	void del_key(const std::string &key);
	// 解释脚本
	bool parse_script(const std::string &sc, uint32_t &pos, std::string &func, xargs &param) const;
	// 访问第一个section
	bool first_section();
	// 访问下一个section
	bool next_section();
	// 迭代器,遍历当前section下的所有key
	inline section_iterator section_begin() const {
		assert(m_pcurmap);
		return m_pcurmap->begin();
	}
	inline section_iterator section_end() const {
		assert(m_pcurmap);
		return m_pcurmap->end();
	}
	// debug接口,打印信息
#ifdef _DEBUG
	void print_script(const std::string &sc) const;
	void dump_config() const;
	void dump_script() const;
	void dump_function(const std::string &func, const xargs &param) const;
#endif
private:
	inline void clear_error(std::istream &is) const {
		is.clear(is.rdstate()&~(std::ios::failbit | std::ios::badbit));
	}
	inline void ignore_line(std::istream &is) const {
		is.ignore(100000,'\n');
	}
	// 取语法单元
	int get_token(std::istream &is) const;
	// 取词(忽略空白)
	bool get_word(std::istream &is, std::string &word, char delim=0) const;
	// 取词(包括空白)
	bool get_word_space(std::istream &is, std::string &word, char delim=0) const;
	// 解析函数调用
	bool parse_function(const char *exp, std::string &func, xargs &param) const;
	// 解析语法单元
	TOKEN parse_token(const char *exp, unsigned &pos, int &ival, float &fval, std::string &str) const;
	// 转义字符
	char translate_char(char c) const;
};

} // namespace wyc

#endif // end of __HEADER_WYC_XCONFIG
