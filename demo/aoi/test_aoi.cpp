#include <cstdlib>
#include <cstdio>
#include <cassert>

#include "wyc/util/util.h"
#include "wyc/util/time.h"

#ifdef _DEBUG
	#pragma comment(lib,"fsutil_d.lib")
#else
	#pragma comment(lib,"fsutil.lib")
#endif

extern void test_basic(void);
extern void test_walk_through(void);

int main(int argv, char **args)
{
	wyc::xdate dt;
	dt.get_date();
	wyc::random_seed(dt.hour()*10000+dt.minute()*100+dt.second());

	printf("AOI System Test Suit\n");
	test_basic();
	test_walk_through();
	printf("All tests are passed.\nPress [Enter] to exit.\n");
	getchar();
	return 0;
}
