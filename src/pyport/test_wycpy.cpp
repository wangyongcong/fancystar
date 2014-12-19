#ifndef WYC_EXPORT_PYTHON

#include <cstring>
#include <cassert>
#include <python.h>
#include "wycpy.h"

#ifdef _DEBUG
	#pragma comment (lib, "fsutil_d.lib")
#else
	#pragma comment (lib, "fsutil.lib")
#endif

int main(int argc, char **argv)
{
	printf("test wycpy\n");
	unsigned key=wyc::strhash("ÄãºÃ");
	printf("ÄãºÃ: %d (0x%X)\n",key,key);
	getchar();
	return 0;
}

#endif // !WYC_EXPORT_PYTHON
