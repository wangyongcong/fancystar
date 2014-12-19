#ifndef __HEADER_WYC_RECORDER
#define __HEADER_WYC_RECORDER

#include <cstdarg>
#include <string>
#include <vector>

namespace wyc
{

class xrecorder
{
	enum OP_CODE 
	{
		OP_RECORD,
		OP_CALL,
		OP_RETURN,
	};
	struct header_t
	{
		std::string name;
		std::string format;
		OP_CODE op;
	};
	union slot_t
	{
		float fval;
		unsigned uval;
		int ival;
	};
	std::vector<header_t> m_headers;
	std::vector<slot_t> m_data;
	std::vector<std::string> m_callstack;
public:
	void start(const char *name, const char *fmt=0, ...);
	void end();
	void record(const char *desc, const char *fmt=0, ...);
	void clear();
	void detail() const;
	void* py_detail() const;
private:
	void _var_parse(const char *fmt, va_list args);
};

} // namespace wyc

#endif // __HEADER_WYC_RECORDER