#include <fstream>
#include <sstream>
#include "wyc/util/util.h"
#include "wyc/util/strutil.h"
#include "wyc/util/fconfig.h"

#ifdef _MSC_VER
#pragma warning (disable: 4244)
#endif // _MSC_VER

#define XCONFIG_SCRIPT_BUFFER_SIZE 1024

namespace wyc
{

xconfig::xconfig() 
{
	m_pcurmap=0;
	m_bChanged=false;
}

xconfig::~xconfig() 
{
	clear();
}

bool xconfig::load(const char *filename) 
{
	m_cfgname=filename;
	std::fstream fs(filename,std::ios_base::in);
	if(!fs.is_open()) {
		wyc_warn("file not found: %s",filename);
		return false;
	}
	std::string secname, keyname, value;
	unsigned token;
	unsigned lineidx=1;
	SECTION *pmap=0;
	while(fs) {
		token=get_token(fs);
		switch(token) {
		case TOKEN_ERROR:
			wyc_error("io error");
			goto EXIT;
		case TOKEN_EOF:
		//	wyc_print("end");
			goto EXIT;
		case TOKEN_NEW_LINE:
			++lineidx;
			continue;
		case '#':
			ignore_line(fs);
			++lineidx;
			continue;
		case '[': 
			fs.get();
			get_word(fs,secname,']');
			if(get_token(fs)!=']' || secname.size()==0) {
				wyc_error("[%d] secname error",lineidx);
			}
			else {
			//	wyc_print("[%d] [%s]",lineidx,secname.c_str());
				pmap=new SECTION;
				if(!m_sections.insert(SECTION_NODE(secname,intptr_t(pmap))).second) {
					delete pmap;
					pmap=0;
					wyc_error("duplicated section: %s",secname.c_str());
				}
			}
			ignore_line(fs);
			++lineidx;
			continue;
		default:
			get_word(fs,keyname,'=');
			if(get_token(fs)!='=' || keyname.size()==0) {
				wyc_error("[%d] key-value error",lineidx);
			}
			else {
				fs.get();
				get_word_space(fs,value);
			//	wyc_print("[%d] %s=%s",lineidx,keyname.c_str(),value.c_str());
				if(pmap) {
					if(!pmap->insert(KEYNODE(keyname,value)).second) 
						wyc_error("duplicated key: %s",keyname.c_str());
				}
			}
			ignore_line(fs);
			++lineidx;
			continue;
		}
	}
EXIT:
	fs.close();
	return true;
}

bool xconfig::save(const char *filename)
{
	if(!filename || filename[0]==0) {
		if(!need_save())
			return true;
		filename=m_cfgname.c_str();
	}
	std::fstream fs(filename,std::ios_base::out|std::ios_base::trunc);
	if(!fs.is_open()) {
		wyc_error("file not exists: %s",filename);
		return false;
	}
	SECTION_GROUP::const_iterator iter=m_sections.begin(), end=m_sections.end();
	SECTION *pmap=0;
	std::string str;
	while(iter!=end) {
		fs<<"["<<iter->first.c_str()<<"]"<<std::endl;
		pmap=(SECTION*)iter->second;
		SECTION::const_iterator iter2=pmap->begin(), end2=pmap->end();
		while(iter2!=end2) {
			fs<<iter2->first.c_str()<<"=";
			fs<<iter2->second.c_str()<<std::endl;
			++iter2;
		}
		fs<<std::endl;
		++iter;
	}
	fs.close();
	m_bChanged=false;
	if(filename!=m_cfgname.c_str())
		m_cfgname=filename;
	return true;
}

void xconfig::clear() 
{
	m_pcurmap=0;
	m_mapname.clear();
	SECTION_GROUP::iterator iter=m_sections.begin(), end=m_sections.end();
	SECTION *pmap=0;
	while(iter!=end) {
		pmap=(SECTION*)iter->second;
		delete pmap;
		++iter;
	}
	m_sections.clear();
	m_bChanged=true;
}

bool xconfig::first_section()
{
	SECTION_GROUP::const_iterator iter=m_sections.begin();
	if(iter==m_sections.end()) {
		return false;
	}
	m_mapname=iter->first;
	m_pcurmap=(SECTION*)iter->second;
	return true;
}

bool xconfig::next_section()
{
	SECTION_GROUP::const_iterator iter=m_sections.find(m_mapname);
	assert(iter!=m_sections.end());
	++iter;
	if(iter==m_sections.end())
		return false;
	m_mapname=iter->first;
	m_pcurmap=(SECTION*)iter->second;
	return true;
}

bool xconfig::set_cur_section(const std::string &secname, bool bAdd)
{
	SECTION_GROUP::const_iterator iter=m_sections.find(secname);
	if(iter==m_sections.end()) {
		if(!bAdd)
			return false;
		SECTION *pmap=new SECTION;
		if(!m_sections.insert(SECTION_NODE(secname,intptr_t(pmap))).second) {
			delete pmap;
			return false;
		}
		m_mapname=secname;
		m_pcurmap=pmap;
	}
	else {
		m_mapname=iter->first;
		m_pcurmap=(SECTION*)iter->second;
	}
	return true;
}

void xconfig::clear_cur_section()
{
	if(m_pcurmap) {
		m_pcurmap->clear();
		m_bChanged=true;
	}
}

void xconfig::remove_section(const std::string &secname)
{
	SECTION_GROUP::iterator iter=m_sections.find(secname);
	if(iter==m_sections.end()) 
		return;
	SECTION *pmap=(SECTION*)(iter->second);
	if(pmap==m_pcurmap) {
		m_mapname.clear();
		m_pcurmap=0;
	}
	delete pmap;
	iter->second=0;
	m_sections.erase(iter);
	m_bChanged=true;
}

bool xconfig::get_key(const std::string &key, std::string &value) const
{
	if(m_pcurmap==0)
		return false;
	SECTION::const_iterator iter=m_pcurmap->find(key);
	if(iter==m_pcurmap->end())
		return false;
	value=iter->second;
	return true;
}

void xconfig::set_key(const std::string &key, const std::string &value)
{
	if(m_pcurmap) {
		(*m_pcurmap)[key]=value;
		m_bChanged=true;
	}
}

void xconfig::del_key(const std::string &key)
{
	if(m_pcurmap==0)
		return;
	SECTION::iterator iter=m_pcurmap->find(key);
	if(iter==m_pcurmap->end())
		return;
	m_pcurmap->erase(iter);
	m_bChanged=true;
}

int xconfig::get_token(std::istream &is) const 
{
	std::istream::int_type c;
	while(is) {
		c=is.peek();
		if(c==std::istream::int_type(EOF))
			return TOKEN_EOF;
		else if(c=='\n') {
			is.get();
			return TOKEN_NEW_LINE;
		}
		else if(isspace(c)) {
			is.get();
			continue;
		}
		return c;
	}
	return TOKEN_ERROR;
}

bool xconfig::get_word(std::istream &is, std::string &word, char delim) const 
{
	word="";
	std::istream::int_type c;
	wchar_t quote=0;
	while(is) {
		c=is.peek();
		if(c==std::istream::int_type(EOF))
			return false;
		if(c=='\'' || c=='"') {
			// 引用内的所有字符都会被包含进来
			if(quote==0) 
				quote=c;
			else if(c==quote) 
				quote=0;
		}
		else if(quote==0) {
			// 在非引用的情况下,遇到分隔符或注释立即返回
			if(c=='\n' || c=='#' || c==delim)
				return true;
			// 去掉前导空白,然后再遇到的空白字符则视为分隔符
			if(isspace(c)) {
				if(word.size()<1) {
					is.get();
					continue;
				}
				return true;
			}
		}
		is.get();
		word+=c;
	}
	return false;
}

bool xconfig::get_word_space(std::istream &is, std::string &word, char delim) const 
{
	word="";
	std::istream::int_type c;
	wchar_t quote=0;
	std::string space_str;
	while(is) {
		c=is.peek();
		if(c==std::istream::int_type(EOF))
			return false;
		if(c=='\'' || c=='"') {
			// 引用
			if(quote==0) 
				quote=c;
			else if(c==quote) 
				quote=0;
		}
		else if(quote==0) {
			// 在非引用的情况下,遇到分隔符或注释立即返回
			if(c=='\n' || c=='#' || c==delim)
				return true;
			// 只去掉首尾的空白字符,位于中间的空白字符会包含进来
			if(isspace(c)) {
				is.get();
				if(word.size())
					space_str+=c;
				continue;
			}
		}
		is.get();
		if(space_str.size()) {
			word+=space_str;
			space_str.clear();
		}
		word+=c;
	}
	return false;
}

bool xconfig::parse_script(const std::string &sc, uint32_t &pos, std::string &func, xargs &param) const
{
/*	std::string sbuff;
	sbuff.reserve(XCONFIG_SCRIPT_BUFFER_SIZE);
	std::istringstream iss(sc);
	while(iss) {

	}
*/
	char buffer[XCONFIG_SCRIPT_BUFFER_SIZE];
	size_t sz=sc.size(), cnt=0;
	param.clear();
	while(pos<sz) {
		if(sc[pos]!=';') {
			buffer[cnt++]=sc[pos++];
			if(cnt>=XCONFIG_SCRIPT_BUFFER_SIZE) {
				wyc_error("脚本缓冲溢出");
				return false;
			}
			continue;
		}
		buffer[cnt]=0;
		parse_function(buffer,func,param);
		++pos;
		return true;
	}
	if(cnt>0) {
		buffer[cnt]=0;
		parse_function(buffer,func,param);
		return true;
	}
	return false;
}

#ifdef _DEBUG
void xconfig::print_script(const std::string &sc) const
{
	char buffer[XCONFIG_SCRIPT_BUFFER_SIZE];
	size_t sz=sc.size(), i=0, cnt=0;
	std::string func;
	xargs param;
	while(i<sz) {
		if(sc[i]!=';') {
			buffer[cnt]=sc[i];
			++i;
			++cnt;
			if(cnt>=XCONFIG_SCRIPT_BUFFER_SIZE) {
				wyc_error("脚本缓冲溢出");
				return;
			}
			continue;
		}
		if(cnt>0) {
			buffer[cnt]=0;
			if(parse_function(buffer,func,param))
				dump_function(func,param);
			param.clear();
			cnt=0;
		}
		++i;
	}
}
#endif

bool xconfig::parse_function(const char *exp, std::string &func, xargs &param) const
{
	unsigned pos=0;
	TOKEN token;
	SCRIPT_STATE st=SC_EXPECT_IDENTIFIER;
	int ival;
	float fval;
	std::string str;
	std::wstring wstr;
	token=parse_token(exp,pos,ival,fval,str);
	while(token!=TOKEN_EOF) {
		switch(token) {
		case TOKEN_IDENTIFIER:
			if(st!=SC_EXPECT_IDENTIFIER) {
				wyc_error("表达式错误,无函数名: %s",exp);
				return false;
			}
			func=str;
			st=SC_EXPECT_BRACKET;
			break;
		case TOKEN_LEFT_BRACKET:
			if(st!=SC_EXPECT_BRACKET) {
				wyc_error("表达式错误,无左括号: %s",exp);
				return false;
			}
			st=SC_EXPECT_PARAMETER;
			break;
		case TOKEN_RIGHT_BRACKET:
			if(st!=SC_EXPECT_COMMA && st!=SC_EXPECT_PARAMETER) {
				wyc_error("表达式错误,无左括号: %s",exp);
				return false;
			}
			st=SC_EXPECT_END;
			break;
		case TOKEN_INTEGER:
			if(st!=SC_EXPECT_PARAMETER) {
				wyc_error("表达式错误,无左括号: %s",exp);
				return false;
			}
			param<<ival;
			st=SC_EXPECT_COMMA;
			break;
		case TOKEN_FLOAT:
			if(st!=SC_EXPECT_PARAMETER) {
				wyc_error("表达式错误,无左括号: %s",exp);
				return false;
			}
			param<<fval;
			st=SC_EXPECT_COMMA;
			break;
		case TOKEN_STRING:
			if(st!=SC_EXPECT_PARAMETER) {
				wyc_error("表达式错误,无左括号: %s",exp);
				return false;
			}
			param<<str;
			st=SC_EXPECT_COMMA;
			break;
		case TOKEN_USTRING:
			if(st!=SC_EXPECT_PARAMETER) {
				wyc_error("表达式错误,无左括号: %s",exp);
				return false;
			}
			str2wstr_utf8(wstr,str);
			param<<wstr;
			st=SC_EXPECT_COMMA;
			break;
		case TOKEN_COMMA:
			if(st!=SC_EXPECT_COMMA) {
				wyc_error("表达式错误,错误的参数列表: %s",exp);
				return false;
			}
			st=SC_EXPECT_PARAMETER;
			break;
		case TOKEN_ERROR:
			return false;
		}
		token=parse_token(exp,pos,ival,fval,str);
	}
	if(st!=SC_EXPECT_END) {
		wyc_error("表达式错误,无右括号: %s",exp);
		return false;
	}
	return true;
}

xconfig::TOKEN xconfig::parse_token(const char *exp, unsigned &pos, int &ival, float &fval, std::string &str) const
{
	char c=exp[pos], pair=0;
	std::string val;
	READ_TOKEN_STATE st=TOKEN_READ_NULL;
	while(c!=0) {
		switch(st) {
		case TOKEN_READ_NULL:
			if(c=='u') { // 可能是unicode前缀
				c=exp[pos+1];
				if(c=='\"' || c=='\'') {
					st=TOKEN_READ_USTRING;
					pair=c;
					++pos;
				}
				else {
					st=TOKEN_READ_IDENTIFIER;
					val+=c;
				}
			}
			else if(isalpha(c) || c=='_') {
				st=TOKEN_READ_IDENTIFIER;
				val+=c;
			}
			else if(isdigit(c) || c=='+' || c=='-') {
				st=TOKEN_READ_NUMBER;
				val+=c;
			}
			else if(c=='\"' || c=='\'') {
				st=TOKEN_READ_STRING;
				pair=c;
			}
			else if(c=='(') {
				++pos;
				return TOKEN_LEFT_BRACKET;
			}
			else if(c==')') {
				++pos;
				return TOKEN_RIGHT_BRACKET;
			}
			else if(c==',') {
				++pos;
				return TOKEN_COMMA;
			}
			else if(!isspace(c)) {
				wyc_error("非法字符: %d(\'%c\')",c,c);
				return TOKEN_ERR;
			}
			break;
		case TOKEN_READ_IDENTIFIER:
			if(isalnum(c) || c=='_') {
				val+=c;
				break;
			}
			str=val;
			return TOKEN_IDENTIFIER;

		case TOKEN_READ_NUMBER:
			if(isdigit(c)) {
				val+=c;
				break;
			}
			else if(c=='.') {
				st=TOKEN_READ_FLOAT;
				val+=c;
				break;
			}
			else if(c=='x' || c=='X') {
				if(val.size()==1 && val[0]=='0') {
					// 16进制数值
					val="";
					st=TOKEN_READ_HEX_NUMBER;
					break;
				}
			}
			ival=str2int(val);
			return TOKEN_INTEGER;

		case TOKEN_READ_HEX_NUMBER:
			if(isdigit(c) || (c>='A' && c<='F') || (c>='a' && c<='f')) {
				val+=c;
				break;
			}
			ival=str2hex(val);
			return TOKEN_INTEGER;

		case TOKEN_READ_FLOAT:
			if(isdigit(c)) {
				val+=c;
				break;
			}
			fval=str2float(val);
			return TOKEN_FLOAT;
			
		case TOKEN_READ_STRING:
		case TOKEN_READ_USTRING:
			if(c=='\\') {
				++pos;
				c=exp[pos];
				if(c==0) {
					wyc_error("期待转义字符");
					return TOKEN_ERROR;
				}
				val+=translate_char(c);
			}
			else if(c==pair) {
				++pos;
				str=val;
				return (st==TOKEN_READ_USTRING)?TOKEN_USTRING:TOKEN_STRING;
			}
			else val+=c;
			break;
		}
		++pos;
		c=exp[pos];
	}
	return TOKEN_EOF;
}

char xconfig::translate_char(char c) const
{
	switch(c) {
	case 'n':
		return '\n';
	case 't':
		return '\t';
	case 'r':
		return '\r';
	}
	return c;
}

////////////////////////////////////////////////////////////////////////////////////
// DEBUG 接口
////////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

void xconfig::dump_config() const 
{
	wyc_print("==Dump==============================");
	SECTION_GROUP::const_iterator iter=m_sections.begin(), end=m_sections.end();
	SECTION *pmap=0;
	while(iter!=end) {
		wyc_print("[%s]",iter->first.c_str());
		pmap=(SECTION*)iter->second;
		SECTION::const_iterator iter2=pmap->begin(), end2=pmap->end();
		while(iter2!=end2) {
			wyc_print("  %s = %s",iter2->first.c_str(),iter2->second.c_str());
			++iter2;
		}
		++iter;
	}
	wyc_print("==End===============================");
}

void xconfig::dump_script() const
{
	wyc_print("==Parse==============================");
	SECTION_GROUP::const_iterator iter=m_sections.begin(), end=m_sections.end();
	SECTION *pmap=0;
	while(iter!=end) {
		wyc_print("[%s]",iter->first.c_str());
		pmap=(SECTION*)iter->second;
		SECTION::const_iterator iter2=pmap->begin(), end2=pmap->end();
		while(iter2!=end2) {
			wyc_print("%s = %s",iter2->first.c_str(),iter2->second.c_str());
			print_script(iter2->second);
			++iter2;
		}
		++iter;
	}
	wyc_print("==End===============================");
}

void xconfig::dump_function(const std::string &func, const xargs &param) const {
	unsigned n=param.size();
	std::stringstream ss;
	int ival;
	float fval;
	std::string str;
	std::wstring wstr;
	ss<<func<<"(";
	for(unsigned i=0; i<n; ++i) {
		switch(param[i].type()) {
		case xvar::XVAR_TYPE_INT:
			if(param[i].get(ival))
				ss<<i<<",";
			else ss<<"Unknown,";
			break;
		case xvar::XVAR_TYPE_FLOAT:
			if(param[i].get(fval))
				ss<<fval<<",";
			else ss<<"Unknown,";
			break;
		case xvar::XVAR_TYPE_STR:
			if(param[i].get(str))
				ss<<'\"'<<str<<"\",";
			else ss<<"Unknown,";
			break;
		case xvar::XVAR_TYPE_WSTR:
			if(param[i].get(wstr)) {
				wstr2str_utf8(str,wstr);
				ss<<'u\"'<<str<<"\",";
			}
			else ss<<"Unknown,";
			break;
		default:
			ss<<"Unknown,";
		}
	}
	ss<<")";
	wyc_print("call: %s",ss.str().c_str());
}

#endif // _DEBUG

} // namespace wyc

