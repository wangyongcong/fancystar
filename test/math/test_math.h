#include <cassert>
#include <iostream>
#include <iomanip>
#include "wyc/util/util.h"
#include "wyc/util/time.h"
#include "wyc/math/mathex.h"

struct STATISTICS 
{
	unsigned cmpcnt, errcnt, relcnt;
	double relerr, maxrel;
	double abserr, maxabs;
	STATISTICS() {
		cmpcnt=0;
		errcnt=0;
		relcnt=0;
		relerr=0;
		maxrel=0;
		abserr=0;
		maxabs=0;
	}
	void reset() {
		cmpcnt=0;
		errcnt=0;
		relcnt=0;
		relerr=0;
		maxrel=0;
		abserr=0;
		maxabs=0;
	}
	void record_error(double absolute) {
		if(absolute>maxabs)
			maxabs=absolute;
		abserr+=absolute;
		++errcnt;
	}
	void record_error(double absolute, double relative) {
		if(absolute>maxabs)
			maxabs=absolute;
		abserr+=absolute;
		++errcnt;
		if(relative>maxrel)
			maxrel=relative;
		relerr+=relative;
		++relcnt;
	}
	inline void report()
	{
		printf("absolute error: %f (max %f)\n",errcnt>0?maxabs/errcnt:0,maxabs);
		printf("relative error: %f%% (max %f%%)\n",relcnt>0?relerr*100/relcnt:0,maxrel*100);
	}
};
extern STATISTICS s_statistics;

struct BENCHMARK_MATRIX {
	unsigned round;
	double tmatadd;
	double tmatmul;
	double tmatinv;
	BENCHMARK_MATRIX() {
		round=0;
		tmatadd=0;
		tmatmul=0;
		tmatinv=0;
	}
	void reset() 
	{
		tmatadd=0;
		tmatmul=0;
		tmatinv=0;
	}
	void report(const char *title) {
		if(title)
			printf("[%s]\n",title);
		printf("  round: %d\n",round);
		printf("  add: %f\n",tmatadd);
		printf("  mul: %f\n",tmatmul);
		printf("  inv: %f\n",tmatinv);
	}
};

// 随机数生成器
extern wyc::xmother_random s_mrand;

unsigned rand_seed();

inline void mrand_init(unsigned seed) 
{
	s_mrand.start(seed);
}

inline float mrand() 
{
	return float(2.0*s_mrand.random()-1.0);
}

// 误差容忍值
#define ABSOLUTE_ERROR 1E-6
#define RELATIVE_ERROR 0.005f

// 浮点数比较
bool FCMP(float v1, float v2);

// 打印浮点数
void fprint(float v1, float v2);

