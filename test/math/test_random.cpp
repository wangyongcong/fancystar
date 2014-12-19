#ifndef _LIB

#include "test_math.h"

struct xrand_std
{
	xrand_std(unsigned seed) {
		wyc::random_seed(seed);
	}
	inline float operator() (void) {
		return wyc::random();
	}
	const char* name() const {
		static const char *ls_name="std rand";
		return ls_name;
	}
};

struct xrand_mother
{
	wyc::xmother_random mr;
	xrand_mother(unsigned seed) {
		mr.start(seed);
	}
	inline double operator() (void) {
		return mr.random();
	}
	const char* name() const {
		static const char *ls_name="mother";
		return ls_name;
	}
};

#define GRAN 10
#define STEP 0.1
template<typename random_t>
void _test_random(unsigned seed)
{
	random_t randgen(seed);
	unsigned round=1000000, i, j;
	unsigned stat[GRAN];
	memset(stat,0,sizeof(unsigned)*GRAN);
	double step[GRAN];
	step[0]=STEP;
	for(i=1; i<GRAN; ++i) 
		step[i]=step[i-1]+STEP;
	unsigned cnt_zero=0, cnt_one=0;
	double fs;
	for(i=0; i<round; ++i) {
		fs=randgen();
		assert(fs>=0 && fs<=1);
		if(fs==0) 
			++cnt_zero;
		else if(fs==1) 
			++cnt_one;
		else {
			for(j=0; j<GRAN; ++j) {
				if(fs<step[j])
				{
					++stat[j];
					break;
				}
			}
		}
	}
	wyc_print("[%s]",randgen.name());
	wyc_print("=0:%f%%",cnt_zero*100.0/round);
	for(i=0; i<GRAN; ++i) 
		wyc_print("<%.2f:%f%%",step[i],stat[i]*100.0/round);
	wyc_print("=1:%f%%",cnt_one*100.0/round);
}

void test_random()
{
	unsigned seed=rand_seed();
	_test_random<xrand_std>(seed);
	_test_random<xrand_mother>(seed);
}

void benchmark_random() 
{
	int round=1000000;
	wyc::xcode_timer ct;
	double dval, timer;
	float fval;
	int ival;
	wyc::xdate dt;
	dt.get_date();
	unsigned seed=dt.hour()*3600+dt.minute()*60+dt.second();
	printf("Generating %d random numbers...\n",round);
	timer=0;
	srand(seed);
	for(int i=0; i<round; ++i) {
		ct.start();
		ival=rand();
		ct.stop();
		timer+=ct.get_time();
	}
	printf("std rand (int): %f\n",timer);
	timer=0;
	wyc::random_seed(seed);
	for(int i=0; i<round; ++i) {
		ct.start();
		fval=wyc::random();
		ct.stop();
		timer+=ct.get_time();
	}
	printf("std rand (float): %f\n",timer);
	wyc::xmother_random mrand;
	timer=0;
	mrand.start(seed);
	for(int i=0; i<round; ++i) {
		ct.start();
		dval=mrand.random();
		ct.stop();
		timer+=ct.get_time();
	}
	printf("mother (double): %f\n",timer);
}

#endif //_LIB
