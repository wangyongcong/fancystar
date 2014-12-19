#include "wyc/math/mathex.h"

namespace wyc
{

// sin/cos查找表
static float g_xmath_sintab[361];
static float g_xmath_costab[361];

// 构造正/余弦函数值查找表
void build_sincos_table()
{
	// 从0度到360度
	float tmp;
	for(int ang=0;ang<=360;++ang)
	{
		// 转化为弧度
		tmp=DEG_TO_RAD(ang);
		// 使用C数学函数填充查找表
		g_xmath_sintab[ang]=sin(tmp);
		g_xmath_costab[ang]=cos(tmp);
	}
}

// 使用线性插值快速计算sin/cos，传入的参数为角度
float fast_sin(float ang)
{
	ang=fmodf(ang,360);
	if(ang<0)
		ang+=360;
	int alpha=(int)ang;
	float beta=ang-alpha;
	return (g_xmath_sintab[alpha]+beta*(g_xmath_sintab[alpha+1]-g_xmath_sintab[alpha]));
}

float fast_cos(float ang)
{
	ang=fmodf(ang,360);
	if(ang<0)
		ang+=360;
	int alpha=(int)ang;
	float beta=ang-alpha;
	return (g_xmath_costab[alpha]+beta*(g_xmath_costab[alpha+1]-g_xmath_costab[alpha]));
}

// 定位到[0,1]边界
void clamp(float32_t &f)
{
	FLOATBITS tmp;
	tmp.fval=f;
	tmp.ival&=~(tmp.ival>>31);
	tmp.fval-=1.0f;
	tmp.ival&=tmp.ival>>31;
	f=tmp.fval+1.0f;
}

// mod到正整数
float32_t mod(float32_t a, float32_t b)
{
	*(int32_t*)(&b)&=0x7FFFFFFF;
	int32_t n=int32_t(a/b);
	a-=n*b;
	if(a<0)
		a+=b;
	return a;
}

// 快速计算x的平方根的倒数，相对误差约为0.177585%
float32_t fast_invsqrt(float32_t x)
{
    float32_t xhalf=0.5f*x;
    int32_t i=*(int*)&x;
    i=0x5f37642f-(i>>1);	// 估计初值
    x=*(float32_t*)&i;
    x=x*(1.5f-xhalf*x*x);	// NR过程
    return x;
}

// 快速计算x的平方根，相对误差约为0.177585%
float32_t fast_sqrt(float32_t x)
{
	FLOATBITS convertor, convertor2;
	convertor.fval = x;
	convertor2.fval = x;
	convertor.ival = 0x1FBCF800 + (convertor.ival >> 1);
	convertor2.ival = 0x5f37642f - (convertor2.ival >> 1);
	return 0.5f*(convertor.fval + (x * convertor2.fval));
}

/* Mother *****************************************************************
|	George Marsaglia's The mother of all random number generators producing
| uniformly distributed pseudo random 32 bit values with period about 2^250.
|
|	The text of Marsaglia's posting is appended at the end of the function.
|
|	The arrays m_mother1 and m_mother2 store carry values in their first element,
| and random 16 bit numbers in elements 1 to 8.
|
|	These random numbers are moved to elements 2 to 9 and a new carry and
| number are generated and placed in elements 0 and 1.
|
|	The arrays m_mother1 and m_mother2 are filled with random 16 bit values on
| first call of Mother by another generator.  mStart is the switch.
|
|	Returns:
|	A 32 bit random number is obtained by combining the output of the two
| generators and returned in *pSeed.  It is also scaled by 2^32-1 and re-
| turned as a double between 0 and 1
|
|	SEED:
|	The inital value of *pSeed may be any long value
|
|	Bob Wheeler 8/8/94
**************************************************************************/

/* Initialize motheri with 9 random values the first time */
void xmother_random::start(unsigned long seed)
{
	unsigned short sNumber;
	unsigned long  number;
	short n, *p;

	sNumber= unsigned short(seed&m16Mask);   /* The low 16 bits */
	number= seed&m31Mask;    /* Only want 31 bits */

	p=m_mother1;
	for (n=18;n--;)
	{
		number=30903*sNumber+(number>>16);   /* One line multiply-with-cary */
		*p++=sNumber=unsigned short(number&m16Mask);
		if (n==9)
			p=m_mother2;
	}
	/* make cary 15 bits */
	m_mother1[0]&=m15Mask;
	m_mother2[0]&=m15Mask;
}

/* Get a random number between 0 and 1 */
double xmother_random::random()
{
	unsigned long  number1, number2;

	/* Move elements 1 to 8 to 2 to 9 */
	memmove(m_mother1+2,m_mother1+1,8*sizeof(short));
	memmove(m_mother2+2,m_mother2+1,8*sizeof(short));

	/* Put the carry values in numberi */
	number1=m_mother1[0];
	number2=m_mother2[0];

	/* Form the linear combinations */
	number1+=1941*m_mother1[2]+1860*m_mother1[3]+1812*m_mother1[4]+1776*m_mother1[5]+
			1492*m_mother1[6]+1215*m_mother1[7]+1066*m_mother1[8]+12013*m_mother1[9];

	number2+=1111*m_mother2[2]+2222*m_mother2[3]+3333*m_mother2[4]+4444*m_mother2[5]+
			5555*m_mother2[6]+6666*m_mother2[7]+7777*m_mother2[8]+9272*m_mother2[9];

	/* Save the high bits of numberi as the new carry */
	m_mother1[0]=short(number1/m16Long);
	m_mother2[0]=short(number2/m16Long);

	/* Put the low bits of numberi into motheri[1] */
	m_mother1[1]=short(m16Mask&number1);
	m_mother2[1]=short(m16Mask&number2);

	/* Combine the two 16 bit random numbers into one 32 bit */
	unsigned long seed=(((long)m_mother1[1])<<16)+(long)m_mother2[1];

	/* Return a double value between 0 and 1 */
	return ((double)seed)/m32Double;
}

} // namespace wyc

