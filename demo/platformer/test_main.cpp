#pragma comment (lib, "fsutil_d.lib")
#pragma comment (lib, "fslog_d.lib")
#pragma comment (lib, "fsmath_d.lib")

#pragma warning (disable: 4201)

#include "wyc/util/util.h"
#include "wyc/util/time.h"
#include "recorder.h"

using namespace wyc;

extern void random_test_sweep_intersect();
extern void test_recorder();

int main(int, char **)
{
	setlocale(LC_ALL,"chs");
	xdate dt;
	dt.get_date();
	srand(dt.hour()*10000+dt.minute()*100+dt.second());

	// run all tests
	// random_test_sweep_intersect();
	test_recorder();

	// all tests are done
	wyc_print("Press [Enter] to continue");
	getchar();
	return 0;
}

void test_recorder()
{
	xrecorder rc;
	rc.start("on_game_init","fiI",3.14f,-10,99);
	rc.record("data[0]","ff",1.0f,2.0f);
	rc.start("test","");
	rc.record("data[1]","ii",1,1);
	rc.end();
	rc.record("data[2]","II",2,2);
	rc.end();
	rc.detail();
	rc.clear();
}