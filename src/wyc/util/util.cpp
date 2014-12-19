#include <cstdarg>
#include <cassert>
#include <cstring>
#include "wyc/util/util.h"

namespace wyc
{

const char EMPTY_CSTRING[1]={0};
const wchar_t EMPTY_WSTRING[1]={0};

void print_libinfo()
{
	// check OS
#ifdef _WIN32
	const char* os="Win32";
#elif _WIN64
	const char* os="Win64";
#elif __linux
	const char* os="Linux";
#else
	const char* os="Unknown";
#endif

	// check compiler and architecture
#ifdef __GNUC__
	#if defined(_x86_) || defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__)
		const char* arch="x86";
	#elif __ia64__
		const char* arch="IA64"
	#elif __amd64__
		const char* arch="AMD64";
	#else
		const char* arch="Unknown";
	#endif
	printf("GNUC %d.%d.%d\n",__GNUC__,__GNUC_MINOR__,__GNUC_PATCHLEVEL__);
	#ifdef __MINGW32__
	printf("MinGW %d.%d\n",__MINGW32_MAJOR_VERSION,__MINGW32_MINOR_VERSION);
	#endif
#elif _MSC_VER
	#ifdef _M_IX86
		const char* arch="x86";
	#elif _M_IA64
		const char* arch="IA64"
	#elif _M_X64
		const char* arch="AMD64";
	#else
		const char* arch="Unknown";
	#endif
	printf("MSVC %d\n",_MSC_VER);
#else
	printf("Unknown compiler\n")
	const char* arch="Unknown";
#endif
	printf("Platform: %s %s\n",os,arch);

	// check debug mode
#ifdef _DEBUG
	printf("Debug: Yes\n");
#endif
}

unsigned power2(unsigned val)
{
	// val可能是2的幂
	--val;
	// 将MSB右边的所有位全部置为1
	val |= (val >>  1);
	val |= (val >>  2);
	val |= (val >>  4);
	val |= (val >>  8);		/* Ok, since int >= 16 bits */
#if (UINT_MAX != 0xffff)
	val |= (val >> 16);		/* For 32 bit int systems */
#if (UINT_MAX > 0xffffffffUL)
	val |= (val >> 32);		/* For 64 bit int systems */
#endif // UINT_MAX != 0xffff
#endif // UINT_MAX > 0xffffffffUL
	++val;
	return val;
}

unsigned log2(unsigned val)
{
	unsigned n=0;
	while(val>>=1) 
		n+=1;
	return n;
}

uint32_t log2p2(uint32_t val)
{
	// 32位DeBruijn数列
	static const int ls_DeBruijn32[32] = 
	{
		0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8, 
		31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
	};
	return ls_DeBruijn32[(uint32_t)(val * 0x077CB531U) >> 27];
}

void init_splitter(char **splitter, size_t sz)
{
	char *s=new char[sz];
	memset(s+1,'-',sz-2);
	s[0]='+';
	s[sz-1]=0;
	*splitter=s;
}

} // namespace wyc


