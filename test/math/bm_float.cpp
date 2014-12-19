#ifndef _LIB

#include "test_math.h"

void test_floatint_point()
{
	int round=100000;
	printf("testing float operations...\n");
	wyc::FLOATBITS f1, f2;
	for(int i=0; i<round; ++i) {
		f1.fval=mrand();
		if(f1.ival&FLT_SIGNBIT_MASK) 
			f2.ival=f1.ival&(~FLT_SIGNBIT_MASK);
		else
			f2.ival=f1.ival|FLT_SIGNBIT_MASK;
		assert(f1.fval+f2.fval==0);
	}
	printf("testing double operations...\n");
	wyc::DOUBLEBITS d1, d2;
	for(int i=0; i<round; ++i) {
		d1.fval=mrand();
		if(d1.ival&DBL_SIGNBIT_MASK)
			d2.ival=d1.ival&(~DBL_SIGNBIT_MASK);
		else 
			d2.ival=d1.ival|DBL_SIGNBIT_MASK;
		assert(d1.fval+d2.fval==0);
	}
}

void benchmark_float2int()
{
	int round=1000000;
	unsigned seed=rand_seed();
	wyc::xcode_timer ct;
	double timer;
	double dval;
	float fval;
	int ival;
	printf("convert floating point to integer...\n");
	mrand_init(seed);
	timer=0;
	for(int i=0; i<round; ++i) {
		dval=mrand();
		ct.start();
		ival=int(dval);
		ct.stop();
		timer+=ct.get_time();
	}
	printf("int(double): %f\n",timer);
	mrand_init(seed);
	timer=0;
	for(int i=0; i<round; ++i) {
		fval=float(mrand());
		ct.start();
		ival=int(fval);
		ct.stop();
		timer+=ct.get_time();
	}
	printf("int(float): %f\n",timer);
	mrand_init(seed);
	timer=0;
	for(int i=0; i<round; ++i) {
		dval=mrand();
		ct.start();
		ival=wyc::fast_round(dval);
		ct.stop();
		timer+=ct.get_time();
	}
	printf("wyc::fast_round(double): %f\n",timer);
}

void benchmark_square_root()
{
	int round=1000000;
	unsigned seed=rand_seed();
	wyc::xcode_timer ct;
	double timer;
	double dval, dret;
	float fval, fret;
	printf("calculate square root...\n");
	mrand_init(seed);
	timer=0;
	for(int i=0; i<round; ++i) {
		fval=mrand();
		ct.start();
		fret=sqrt(fval);
		ct.stop();
		timer+=ct.get_time();
	}
	printf("std::sqrt(float): %f\n",timer);
	mrand_init(seed);
	timer=0;
	for(int i=0; i<round; ++i) {
		dval=mrand();
		ct.start();
		dret=sqrt(dval);
		ct.stop();
		timer+=ct.get_time();
	}
	printf("std::sqrt(double): %f\n",timer);
	mrand_init(seed);
	timer=0;
	for(int i=0; i<round; ++i) {
		fval=mrand();
		ct.start();
		fret=wyc::fast_sqrt(fval);
		ct.stop();
		timer+=ct.get_time();
	}
	printf("wyc::fast_sqrt(float): %f\n",timer);
	mrand_init(seed);
	timer=0;
	for(int i=0; i<round; ++i) {
		fval=mrand();
		ct.start();
		fret=1.0f/sqrt(fval);
		ct.stop();
		timer+=ct.get_time();
	}
	printf("1/std::sqrt(float): %f\n",timer);
	mrand_init(seed);
	timer=0;
	for(int i=0; i<round; ++i) {
		fval=mrand();
		ct.start();
		fret=wyc::fast_invsqrt(fval);
		ct.stop();
		timer+=ct.get_time();
	}
	printf("wyc::fast_invsqrt(float): %f\n",timer);
}

#endif // _LIB
