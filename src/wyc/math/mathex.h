#ifndef __HEADER_WYC_XMATHEX
#define __HEADER_WYC_XMATHEX

#include <cmath>
#include <cstring>
#include <limits>
#include "wyc/basedef.h"

namespace wyc
{

//
// 常量和宏定义
//

#define XMATH_PI	(3.1415926535897932384626433832795)
#define XMATH_2PI	(6.283185307179586476925286766559)		// 2*PI
#define XMATH_HPI	(1.5707963267948966192313216916398)		// PI/2
#define XMATH_QPI	(0.78539816339744830961566084581988)	// PI/4
#define XMATH_INVPI	(0.31830988618379067153776752674503)	// 1/PI

// Napier's constant, its symbol (e) honors Euler
#define XMATH_EULER	(2.71828182845904523536)

// 极小值
#define EPSILON_E4	(float)(1E-4) 
#define EPSILON_E6	(float)(1E-6)
#define EPSILON_E10	(double)(1E-10)

#ifndef MIN 
#define MIN(a,b)	((a)<(b)?(a):(b))
#endif

#ifndef MAX
#define MAX(a,b)	((a)>(b)?(a):(b))
#endif

#ifndef SWAP
#define SWAP	std::swap
#endif

#ifndef SWAP_BITS
#define SWAP_BITS(a,b)	{(a)^=(b);(b)^=(a);(a)^=(b);}
#endif

#define FAST_DIV_255(n) (((n)+((n)>>8))>>8)

// 角度和弧度之间的转换
#define XMATH_PID180 (0.01745329251994329576923690768489)	// PI/180
#define XMATH_180DPI (57.295779513082320876798154814105)	// 180/PI
#define DEG_TO_RAD(ang) (float((ang)*XMATH_PID180))
#define RAD_TO_DEG(rad) (float((rad)*XMATH_180DPI))

///////////////////////////////////////////////////////////
//
// 三角函数
//

// 构建sin/cos查找表
void build_sincos_table();

// 快速sin/cos函数
float fast_sin(float ang);
float fast_cos(float ang);

////////////////////////////////////////////////////////////
//
// 浮点数辅助函数
//
// 注意：部分函数依赖于sizeof(float)==4 && sizeof(double)==8
//

#define FLT_BIT_COUNT 32
#define FLT_SIGNBIT_MASK 0x80000000
#define FLT_MANTISSA_MASK 0x7FFFFF
#define FLT_EXPONENT_MASK 0x7F800000

#define DBL_BIT_COUNT 64
#define DBL_SIGNBIT_MASK 0x8000000000000000L
#define DBL_MANTISSA_MASK 0xFFFFFFFFFFFFFL
#define DBL_EXPONENT_MASK 0x7FF0000000000000L

union FLOATBITS
{
	uint32_t ival;
	float32_t fval;
};

union DOUBLEBITS
{
	uint64_t ival;
	float64_t fval;
};

inline bool is_nan(float f) 
{	
	FLOATBITS bits;
	bits.fval=f;
	return (bits.ival&FLT_EXPONENT_MASK)==FLT_EXPONENT_MASK 
		&& bits.ival&DBL_MANTISSA_MASK;
}

inline bool is_nan(double f) 
{	
	DOUBLEBITS bits;
	bits.fval=f;
	return (bits.ival&DBL_EXPONENT_MASK)==DBL_EXPONENT_MASK 
		&& bits.ival&DBL_MANTISSA_MASK;
}

inline bool is_infinity(float f)
{
	FLOATBITS bits;
	bits.fval=f;
	return (bits.ival&FLT_EXPONENT_MASK)==FLT_EXPONENT_MASK;
}

inline bool is_infinity(double f) 
{	
	DOUBLEBITS bits;
	bits.fval=f;
	return (bits.ival&DBL_EXPONENT_MASK)==DBL_EXPONENT_MASK;
}

// 计算相对误差
template<typename FLOAT_T>
inline FLOAT_T relative_error(FLOAT_T v1, FLOAT_T v2)
{
	return fabs(v1)>fabs(v2) ? fabs((v1-v2)/v1) : fabs((v1-v2)/v2);
}

// 判断两数是否相等
template<typename FLOAT_T>
inline bool almost_equal(FLOAT_T v1, FLOAT_T v2, FLOAT_T maxRelativeError)
{
	if(v1==v2) return true;
	return fabs(v1-v2) <= (fabs(v1)>fabs(v2) ? fabs(v1) : fabs(v2)) * maxRelativeError;
}

inline bool fequal (float v1, float v2) 
{
//	return fabs(v1-v2)<=std::numeric_limits<float>::epsilon();
	return fabs(v1-v2)<=EPSILON_E6;
}

inline bool fequal (double v1, double v2) 
{
//	return fabs(v1-v2)<=std::numeric_limits<double>::epsilon();
	return fabs(v1-v2)<=EPSILON_E10;
}

//
// 浮点数到整数的快速转换
//
#define FTI_MAGIC 6755399441055744.0
#define FTI_MAGIC_DELTA 0.499999999999

// 四舍五入到整数,比C强制类型转换快15%,而且更加精确
inline int32_t fast_round (float64_t dval) 
{
	dval += FTI_MAGIC;
	return *(int32_t*)&dval;
}

// 截取到整数
inline int32_t fast_floor (float64_t dval) 
{
	return fast_round(dval-FTI_MAGIC_DELTA);
}

// 进位到整数
inline int32_t fast_ceil (float64_t dval) 
{
	return fast_round(dval+FTI_MAGIC_DELTA);
}

// 绝对值
inline float32_t fabs(float32_t a)
{
	*(int*)(&a)&=0x7FFFFFFF;
	return a;
}

// 复制符号
inline float32_t copy_sign(float32_t a, float32_t b)
{
	*(uint32_t*)(&a) = ( (*(uint32_t*)(&a)) & 0x7FFFFFFF ) | ( (*(uint32_t*)(&b)) & 0x80000000 );
	return a;
}

// 定位到[0,1]边界
void clamp(float32_t &f);

// 正周期函数
float32_t mod(float32_t a, float32_t b);


//
// 快速计算平方根和平方根的倒数，基于NR
// Deprecated: 在MSVC下与std::sqrt()和1.0/std::sqrt()已无任何优势
//
float32_t fast_sqrt(float32_t x);
float32_t fast_invsqrt(float32_t x);

/**********************************************************
	伪随机数生成器, 使用 George Marsaglia 的算法
	随机数范围: (0,1)
	循环周期: 2^250
	数值分布: 
		=0:0.000000%
		<0.100000:9.968100%
		<0.200000:10.033800%
		<0.300000:9.960000%
		<0.400000:9.984700%
		<0.500000:10.001200%
		<0.600000:10.000700%
		<0.700000:9.972800%
		<0.800000:9.999600%
		<0.900000:10.091500%
		<1.000000:9.987600%
		=1:0.000000%
	效率: 耗时若为rand()的1.4倍,
**********************************************************/
class xmother_random
{
	short m_mother1[10];
	short m_mother2[10];

	#define m16Long		65536L				/* 2^16 */
	#define m16Mask		0xFFFF				/* mask for lower 16 bits */
	#define m15Mask		0x7FFF				/* mask for lower 15 bits */
	#define m31Mask		0x7FFFFFFF			/* mask for 31 bits */
	#define m32Double	4294967295.0		/* 2^32-1 */

public:
	/* Initialize motheri with 9 random values the first time */
	void start(unsigned long seed);
	/* Get a random number between 0 and 1 */
	double random();
};


} // namespace wyc

#endif // end of __HEADER_WYC_XMATHEX


