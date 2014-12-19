#ifndef _LIB

#ifdef _DEBUG
	#pragma comment (lib, "fsutil_d.lib")
#else
	#pragma comment (lib, "fsutil.lib")
#endif

#include "test_math.h"

STATISTICS s_statistics;

wyc::xmother_random s_mrand;

unsigned rand_seed() 
{
	wyc::xdate dt;
	dt.get_date();
	return dt.hour()*3600+dt.minute()*60+dt.second();
}

bool FCMP(float v1, float v2)
{
	if(v1==v2) return true;
	float diff=fabs(v1-v2);
	if(diff<ABSOLUTE_ERROR) {
		s_statistics.record_error(diff);
		return true;
	}
	float rela=fabs(v1)>fabs(v2) ? fabs(diff/v1) : fabs(diff/v2);
	s_statistics.record_error(diff,rela);
	return rela<RELATIVE_ERROR;
}

void fprint(float v1, float v2)
{
	printf("    (1): %f(%X)\n",v1,*(unsigned*)(&v1));
	printf("    (2): %f(%X)\n",v2,*(unsigned*)(&v2));
	printf("    absolute error: %f\n",fabs(v1-v2));
	printf("    relative error: %f\n",wyc::relative_error(v1,v2));
}

extern void test_floatint_point();
extern void benchmark_float2int();
extern void benchmark_square_root();

extern void test_vector();
extern void test_matrix();
extern void test_matrix_inverse();
extern void test_vecmat();
extern void benchmark_matrix();
extern void test_vecmat_assignment();

extern void test_random();
extern void benchmark_random();

int main(int argc, char **argv)
{
	unsigned seed=rand_seed();
	srand(seed);
	mrand_init(seed);
	wyc_sys("Test math lib");
#ifdef _DEBUG
//	test_floatint_point();
//	test_vector();
//	test_matrix();
//	test_matrix_inverse();
//	test_vecmat();
//	test_vecmat_assignment();
//	test_random();
#else 
//	benchmark_float2int();
//	benchmark_square_root();
	benchmark_random();
//	benchmark_matrix();
#endif
	printf("all tests are passed\n");
	printf("press Enter to exit\n");
	getchar();
}

#endif // !_LIB

