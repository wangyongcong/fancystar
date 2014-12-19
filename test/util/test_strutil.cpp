#include <cassert>
#include "wyc/util/util.h"
#include "wyc/util/time.h"
#include "wyc/util/strutil.h"

using namespace wyc;

void wyc_strcpy1(char *dst, const char *src)
{
	do {
		*dst++=*src++;
	} while(*src!=0);
}

void wyc_strcpy2(char *dst, const char *src)
{
	register char c;
	do {
		c=*src++;
		*dst++=c;
	} while(c!=0);
}

void wyc_strcpy3(char *dst, const char *src)
{
	register const int off=dst-src-1;
	register char c;
	do {
		c=*src++;
		((char*)src)[off]=c;
	} while(c!=0);
}

void strcpy_profile()
{
	wyc::xcode_timer ct;
	char splitter[80];
	memset(splitter,'-',80);
	splitter[0]='+';
	splitter[79]=0;
	const size_t sz=4096;
	char *dst=new char[sz];
	char *src=new char[sz];
	for(int i=0; i<sz; ++i) {
		src[i]=rand()%26+'a';
	}
	src[sz-1]=0;
	memset(dst,0,sz);
	wyc_print(splitter);
	//
	ct.start();
	wyc_strcpy1(dst,src);
	ct.stop();
	wyc_print("| pointer iteration: %f",ct.get_time());
	assert(0==strcmp(src,dst));
	memset(dst,0,sz);
	//
	ct.start();
	wyc_strcpy2(dst,src);
	ct.stop();
	wyc_print("| register char copy: %f",ct.get_time());
	assert(0==strcmp(src,dst));
	memset(dst,0,sz);
	//
	ct.start();
	wyc_strcpy3(dst,src);
	ct.stop();
	wyc_print("| register char copy with offset: %f",ct.get_time());
	wyc_print(splitter);
	assert(0==strcmp(src,dst));
	memset(dst,0,sz);
	delete [] src;
	delete [] dst;
}

void test_strutil ()
{
	wyc_print("Testing string util function...");
	const char *postfix="postfix", *prefix="prefix";
	std::string s="prefix_xxxxxxxx_postfix";
	bool b;
	b=starts_with(s.c_str(),prefix);
	assert(b);
	b=starts_with(s.c_str(),"");
	assert(b);
	b=ends_with(s.c_str(),postfix);
	assert(b);
	b=ends_with(s.c_str(),"");
	s="";
	b=starts_with(s.c_str(),prefix);
	assert(!b);
	b=ends_with(s.c_str(),postfix);
	assert(!b);
	s="prefix_xxx";
	b=starts_with(s.c_str(),prefix);
	assert(b);
	b=ends_with(s.c_str(),postfix);
	assert(!b);
	s="xxx";
	b=starts_with(s.c_str(),prefix);
	assert(!b);
	b=ends_with(s.c_str(),postfix);
	assert(!b);

	wyc_print("All tests are passed");
}
