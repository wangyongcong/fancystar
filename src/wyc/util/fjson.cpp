#include <cassert>
#include "wyc/util/fjson.h"
#include "wyc/util/util.h"

namespace wyc
{

xjson::xjson() : m_allocator(1024)
{
	m_source=0;
	m_cap=0;
	m_size=0;
	m_root=0;
	m_node=0;
}

xjson::~xjson()
{
	if(m_source) 
		delete [] m_source;
}

void xjson::clear()
{
	m_source[0]=0;
	m_size=0;
	m_root=0;
	m_node=0;
	m_allocator.clear();
}

bool xjson::load_file(const char *pfile)
{
	FILE *fp;
	if(0!=fopen_s(&fp,pfile,"rb")) 
	{
		wyc_error("xjson::load: could not open file [%s]",pfile);
		return false;
	}
	fseek(fp,0,SEEK_END);
	size_t size=ftell(fp);
	if(!m_source || m_cap<=size) {
		if(m_source) 
			delete [] m_source;
		m_cap=size+1;
		m_source=new char[m_cap];
	}
	fseek(fp,0,SEEK_SET);
	fread(m_source,1,size,fp);
	m_source[size]=0;
	m_size=size;
	fclose(fp);
	return _parse();
}

bool xjson::load_source(const char *source, size_t size)
{
	if(!size)
		size=strlen(source);
	if(0==size)
	{
		clear();
		return true;
	}
	if(!m_source || m_cap<=size) {
		if(m_source) 
			delete [] m_source;
		m_cap=size+1;
		m_source=new char[m_cap];
	}
	strcpy_s(m_source,m_cap,source);
	m_size=size;
	return _parse();
}

bool xjson::_parse()
{
	char *error_pos=0;
	char *error_desc=0;
	int error_line;
	m_allocator.clear();
	m_root=vjson::json_parse(m_source,&error_pos,&error_desc,&error_line,&m_allocator);
	m_node=m_root;
	if(0==m_root)
	{
		wyc_error("xjson::load: faield to parse JSON file");
		if(error_pos) {
			wyc_print("\tposition: %s",error_pos);
		}
		if(error_line) {
			wyc_print("\tline: %d",error_line);
		}
		if(error_desc) {
			wyc_print("\tdesc: %s",error_desc);
		}
		return false;
	}
	return true;
}

bool xjson::seek(const char *arg_name)
{
	if(!m_root)
		return false;
	if(!arg_name) {
		m_node = m_root;
		return true;
	}
	for(const vjson::json_value *iter=m_node->first_child; iter; iter=iter->next_sibling)
	{
		if(iter->name && 0==strcmp(iter->name,arg_name))
		{
			m_node = iter;
			return true;
		}
	}
	return false;
}

const vjson::json_value* xjson::get_value(const char *arg_name) const
{
	if(!m_root)
		return 0;
	assert(m_node);
	if(m_node->type!=vjson::JSON_OBJECT)
		return 0;
	for(const vjson::json_value *iter=m_node->first_child; iter; iter=iter->next_sibling)
	{
		if(0==strcmp(iter->name,arg_name))
			return iter;
	}
	return 0;
}

//---------------------------------------------------------------------

xjson_writer::xjson_writer(std::ostream &stream) : m_stream(stream)
{
}

xjson_writer::~xjson_writer()
{
}

void xjson_writer::begin_object() {
	if(m_states.empty() || m_states.back()==IN_ARRAY)
		m_stream<<m_indent<<"{"<<std::endl;
	else 
		m_stream<<m_indent<<"\"anonymous_object\" : {"<<std::endl;
	m_indent.push_back('\t');
	m_states.push_back(IN_OBJECT);
}

void xjson_writer::begin_object(const std::string &name) {
	if(m_states.empty() || m_states.back()==IN_ARRAY)
		m_stream<<m_indent<<"{"<<std::endl;
	else
		m_stream<<m_indent<<"\""<<name<<"\" : {"<<std::endl;
	m_indent.push_back('\t');
	m_states.push_back(IN_OBJECT);
}

void xjson_writer::end_object() {
	m_indent.erase(m_indent.size()-1);
//	m_indent.pop_back();
	m_states.pop_back();
	m_stream<<m_indent;
	if(m_states.empty())
		m_stream<<"}";
	else m_stream<<"},";
	m_stream<<std::endl;
}

void xjson_writer::begin_array() {
	if(m_states.empty() || m_states.back()==IN_ARRAY)
		m_stream<<m_indent<<"["<<std::endl;
	else 
		m_stream<<m_indent<<"\"anonymous_array\" : ["<<std::endl;
	m_indent.push_back('\t');
	m_states.push_back(IN_ARRAY);
}

void xjson_writer::begin_array(const std::string &name) {
	if(m_states.empty() || m_states.back()==IN_ARRAY)
		m_stream<<m_indent<<"["<<std::endl;
	else 
		m_stream<<m_indent<<"\""<<name<<"\" : ["<<std::endl;
	m_indent.push_back('\t');
	m_states.push_back(IN_ARRAY);
}

void xjson_writer::end_array() {
	m_indent.erase(m_indent.size()-1);
//	m_indent.pop_back();
	m_states.pop_back();
	m_stream<<m_indent;
	if(m_states.empty())
		m_stream<<"]";
	else m_stream<<"],";
	m_stream<<std::endl;
}

}; // namespace wyc

