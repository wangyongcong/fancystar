#include "wyc/mem/memtracer.h"
#include "wyc/mem/memobj.h"

using namespace wyc;

class xtestobj : public xmemobj
{
	int m_value;
public:
	xtestobj(int v=0) : m_value(v) {}
};

void test_memobj() {
	xtestobj *pobj=wycnew xtestobj;
	xtestobj *plist=wycnew xtestobj[16];
	wycdel pobj;
	wycdel[] plist;
	xmemtracer::singleton().report();
}

