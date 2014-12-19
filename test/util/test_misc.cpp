#include <cassert>
#include "wyc/util/util.h"
#include "wyc/util/time.h"

using namespace wyc;

void test_log2() 
{
	wyc_print("[Test][Function] wyc::log2");
	for(unsigned i=0; i<10; ++i) {
		unsigned n=1<<i;
		unsigned k=wyc::log2p2(n);
		wyc_print("log2(%d)=%d",n,k);
		assert(i==k);
		unsigned end=n<<1;
		while(++n<end) {
			k=wyc::log2(n);
			assert(i==k);
		}
	}
	wyc_print("[Test] successful");
}

