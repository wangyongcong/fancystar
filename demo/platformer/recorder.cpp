#include "recorder.h"

#include <cstdio>

namespace wyc
{

void xrecorder::_var_parse(const char *fmt, va_list args)
{
	slot_t v;
	double d;
	for(; *fmt; ++fmt)
	{
		switch(*fmt)
		{
		case 'f':
			d = va_arg(args, double);
			v.fval = float(d);
			m_data.push_back(v);
			break;
		case 'i':
			v.ival = va_arg(args, int);
			m_data.push_back(v);
			break;
		case 'I':
			v.uval = va_arg(args, unsigned);
			m_data.push_back(v);
			break;
		default:
			break;
		}
	}
}

void xrecorder::start(const char *name, const char *fmt, ...)
{
	m_headers.resize(m_headers.size()+1);
	header_t &h = m_headers.back();
	h.name = name;
	h.op = OP_CALL;
	if(fmt && *fmt) {
		h.format = fmt;
		va_list args;
		va_start(args, fmt);
		_var_parse(fmt, args);
		va_end(args);
	}
	m_callstack.push_back(h.name);
}

void xrecorder::end()
{
	m_headers.resize(m_headers.size()+1);
	header_t &h = m_headers.back();
	h.name = m_callstack.back();
	h.op = OP_RETURN;
	m_callstack.pop_back();
}

void xrecorder::record(const char *desc, const char *fmt, ...)
{
	m_headers.resize(m_headers.size()+1);
	header_t &h = m_headers.back();
	h.name = desc;
	h.op = OP_RECORD;
	if(fmt && *fmt) {
		h.format = fmt;
		va_list args;
		va_start(args, fmt);
		_var_parse(fmt, args);
		va_end(args);
	}
}

void xrecorder::clear()
{
	m_headers.clear();
	m_data.clear();
}

void xrecorder::detail() const
{
	std::string indent;
	unsigned idx=0;
	const char *spliter[2]={"",", "};
	unsigned char sp;
	std::vector<header_t>::const_iterator iter = m_headers.begin(), end = m_headers.end();
	for(; iter!=end; ++iter)
	{
		if(iter->op == OP_CALL) 
			printf("%s%s(",indent.c_str(),iter->name.c_str());
		else if(iter->op == OP_RETURN) {
			indent.erase(indent.size()-1);
			printf("%s%s END",indent.c_str(),iter->name.c_str());
		}
		else printf("%s%s: ",indent.c_str(),iter->name.c_str());
		if(iter->format.size())
		{
			for(size_t i=0, cnt=iter->format.size(); i<cnt; ++i)
			{
				sp = i==0?0:1;
				switch(iter->format[i])
				{
				case 'f':
					printf("%s%f",spliter[sp],m_data[idx++].fval);
					break;
				case 'i':
					printf("%s%i",spliter[sp],m_data[idx++].ival);
					break;
				case 'I':
					printf("%s%u",spliter[sp],m_data[idx++].uval);
					break;
				}
			}
		}
		if(iter->op == OP_CALL) {
			printf(")\n");
			indent += "\t";
		}
		else printf("\n");
	}
}

} // namespace wyc

