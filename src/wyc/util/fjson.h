#ifndef __HEADER_WYC_FJSON
#define __HEADER_WYC_FJSON

#include <string>
#include <iomanip>
#include <ostream>
#include <vector>
#include "vjson/json.h"
#include "wyc/util/strutil.h"

namespace wyc
{

class xjson 
{
	char *m_source;
	size_t m_cap, m_size;
	vjson::block_allocator m_allocator;
	vjson::json_value *m_root;
	const vjson::json_value *m_node;
	mutable std::wstring m_tmp_wstr;
public:
	xjson();
	~xjson();
	void clear();
	bool load_file(const char *file_name);
	bool load_source(const char *source, size_t size=0);
	const vjson::json_value* get_root() const {
		return m_root;
	}
	bool seek(const char *arg_name=0);
	const vjson::json_value* get_value(const char *arg_name) const;
	template<typename T>
	T get(const char *arg_name, T default_value) const;
	template<>
	int get(const char *arg_name, int default_value) const {
		const vjson::json_value* v=get_value(arg_name);
		if(v && vjson::JSON_INT==v->type)
			return v->int_value;
		return default_value;
	}
	template<>
	float get(const char *arg_name, float default_value) const {
		const vjson::json_value* v=get_value(arg_name);
		if(v ){
			if(vjson::JSON_FLOAT==v->type)
				return v->float_value;
			else if(vjson::JSON_INT==v->type)
				return float(v->int_value);
		}
		return default_value;
	}
	template<>
	bool get(const char *arg_name, bool default_value) const {
		const vjson::json_value* v=get_value(arg_name);
		if(v && vjson::JSON_BOOL==v->type)
			return v->int_value!=0;
		return default_value;
	}
	template<>
	const char* get(const char *arg_name, const char* default_value) const {
		const vjson::json_value* v=get_value(arg_name);
		if(v && vjson::JSON_STRING==v->type)
			return v->string_value;
		return default_value;
	}
	template<>
	const wchar_t* get(const char *arg_name, const wchar_t* default_value) const {
		const vjson::json_value* v=get_value(arg_name);
		if(v && vjson::JSON_STRING==v->type)
		{
			if(wyc::str2wstr_utf8(m_tmp_wstr,v->string_value))
				return m_tmp_wstr.c_str();
		}
		return default_value;
	}
protected:
	bool _parse();
};

//---------------------------------------------------------------------

class xjson_writer
{
	std::string m_indent;
	std::ostream &m_stream;
	std::vector<char> m_states;
	enum {
		IN_OBJECT = 0,
		IN_ARRAY,
	};
public:
	xjson_writer(std::ostream &stream);
	~xjson_writer();
	void begin_object();
	void begin_object(const std::string &name);
	void end_object();
	void begin_array();
	void begin_array(const std::string &name);
	void end_array();
	void add_entry (int value);
	void add_entry (unsigned value);
	void add_entry (float value);
	void add_entry (double value);
	void add_entry (const char *value);
	void add_entry (const wchar_t *value);
	void add_entry (const std::string &value);
	void add_entry (const std::wstring &value);
	void add_entry (const std::string &key, int value);
	void add_entry (const std::string &key, unsigned value);
	void add_entry (const std::string &key, float value);
	void add_entry (const std::string &key, double value);
	void add_entry (const std::string &key, const char *value);
	void add_entry (const std::string &key, const wchar_t *value);
	void add_entry (const std::string &key, const std::string &value);
	void add_entry (const std::string &key, const std::wstring &value);
};

#define XJSON_WRITER_OUTPUT(value) m_stream<<m_indent<<value<<','<<std::endl
#define XJSON_WRITER_OUTPUT_PAIR(key,value) m_stream<<m_indent<<'\"'<<key<<"\" : "<<value<<','<<std::endl


inline void xjson_writer::add_entry (int value)
{
	XJSON_WRITER_OUTPUT(value);
}

inline void xjson_writer::add_entry (unsigned value)
{
	XJSON_WRITER_OUTPUT(value);
}

inline void xjson_writer::add_entry (float value)
{
	m_stream<<m_indent<<std::fixed<<value<<','<<std::endl;
}

inline void xjson_writer::add_entry (double value)
{
	m_stream<<m_indent<<value<<','<<std::endl;
}

inline void xjson_writer::add_entry (const std::string &value)
{
	m_stream<<m_indent<<'\"'<<value<<"\","<<std::endl;
}

inline void xjson_writer::add_entry (const std::wstring &value)
{
	std::string s;
	if(wstr2str_utf8(s,value))
		m_stream<<m_indent<<'\"'<<s<<"\","<<std::endl;
	else
		m_stream<<m_indent<<"\" : \"bad Unicode string\","<<std::endl;
}

inline void xjson_writer::add_entry (const char *value)
{
	add_entry(std::string(value));
}

inline void xjson_writer::add_entry (const wchar_t *value)
{
	add_entry(std::wstring(value));
}

inline void xjson_writer::add_entry (const std::string &key, int value) {
	XJSON_WRITER_OUTPUT_PAIR(key,value);
}

inline void xjson_writer::add_entry (const std::string &key, unsigned value) {
	XJSON_WRITER_OUTPUT_PAIR(key,value);
}

inline void xjson_writer::add_entry (const std::string &key, float value) {
	m_stream<<m_indent<<'\"'<<key<<"\" : "<<std::fixed<<value<<','<<std::endl;
}

inline void xjson_writer::add_entry (const std::string &key, double value) {
	m_stream<<m_indent<<'\"'<<key<<"\" : "<<value<<','<<std::endl;
}

inline void xjson_writer::add_entry (const std::string &key, const std::string &value) 
{
	m_stream<<m_indent<<'\"'<<key<<"\" : \""<<value<<"\","<<std::endl;
}

inline void xjson_writer::add_entry (const std::string &key, const std::wstring &value) 
{
	std::string s;
	if(wstr2str_utf8(s,value))
		m_stream<<m_indent<<'\"'<<key<<"\" : \""<<s<<"\","<<std::endl;
	else
		m_stream<<m_indent<<'\"'<<key<<"\" : \"bad Unicode string\","<<std::endl;
}

inline void xjson_writer::add_entry (const std::string &key, const char *value) {
	add_entry(key, std::string(value));
}

inline void xjson_writer::add_entry (const std::string &key, const wchar_t *value) {
	add_entry(key, std::wstring(value));
}

}; // namespace wyc

#endif // __HEADER_WYC_FJSON
