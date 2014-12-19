#ifndef __HEADER_WYC_XUTIL
#define __HEADER_WYC_XUTIL

#include <cstdio>
#include <cstdlib>
#include <string>
#include "wyc/basedef.h"
#include "wyc/config.h"

#ifdef WYC_LOG2CONSOLE
	#define wyc_print(fmt,...)	{printf(fmt,__VA_ARGS__);printf("\n");}
	#define wyc_sys(fmt,...)	{printf("[SYS] ");printf(fmt,__VA_ARGS__);printf("\n");}
	#define wyc_warn(fmt,...)	{printf("[WARNING] ");printf(fmt,__VA_ARGS__);printf("\n");}
	#define wyc_error(fmt,...)	{printf("[ERROR] ");printf(fmt,__VA_ARGS__);printf("\n");}
	#define wyc_fatal(fmt,...)	{printf("[FATAL] ");printf(fmt,__VA_ARGS__);printf("\n");}
#elif defined WYC_LOG2FILE
	#include "wyc/log/log.h"
	#define wyc_print(fmt,...)	{wyc_log(wyc::LOG_NORMAL,fmt,__VA_ARGS__);}
	#define wyc_sys(fmt,...)	{wyc_log(wyc::LOG_SYS,fmt,__VA_ARGS__);}
	#define wyc_warn(fmt,...)	{wyc_log(wyc::LOG_WARN,fmt,__VA_ARGS__);}
	#define wyc_error(fmt,...)	{wyc_log(wyc::LOG_ERROR,fmt,__VA_ARGS__);}
	#define wyc_fatal(fmt,...)	{wyc_log(wyc::LOG_FATAL,fmt,__VA_ARGS__);}
#else 
	#define wyc_print(fmt,...)
	#define wyc_sys(fmt,...)
	#define wyc_warn(fmt,...)
	#define wyc_error(fmt,...)
	#define wyc_net(fmt,...)
#endif

#ifndef MIN 
	#define MIN(a,b)	((a)<(b)?(a):(b))
#endif

#ifndef MAX
	#define MAX(a,b)	((a)>(b)?(a):(b))
#endif

#include "wyc/util/rect.h"

namespace wyc
{

// 打印库平台信息
void print_libinfo();

extern const char EMPTY_CSTRING[1];
extern const wchar_t EMPTY_WSTRING[1];

// 状态控制
inline void add_state(uint32_t &st, uint32_t stat) { st|=stat; }
inline void add_state(uint32_t &st, uint32_t mask, uint32_t stat) { st&=(~mask);st|=mask&stat; }
inline void remove_state(uint32_t &st, uint32_t stat) { st&=(~stat); }
inline bool is_state(uint32_t st, uint32_t stat) { return (st&stat)==stat; }
inline bool have_state(uint32_t st, uint32_t stat) { return (st&stat)!=0; }
inline void switch_state(uint32_t &st, uint32_t stat) { st=(st&(~stat))|((~st)&stat); }

// 返回大于等于val的最小的2的幂
unsigned power2(unsigned val);

// 返回2为底的对数
unsigned log2(unsigned val);

// 返回2为底的对数,val必须为2的幂
uint32_t log2p2(uint32_t val);

// 整数val是否2的幂
inline bool is_power2(unsigned val)
{
	return (val&(val-1))==0;
}

// 计算字符串hash值
unsigned int strhash(const char *str);
unsigned int strhash(const wchar_t *str);

/**********************************************************
	生成伪随机数, 使用标准库的rand()实现
	随机数范围: [0,1]
	数值分布(基于VC9的C标准库实现):
		=0:0.002300%
		<0.100000:9.929100%
		<0.200000:10.004400%
		<0.300000:10.006900%
		<0.400000:10.023500%
		<0.500000:10.001100%
		<0.600000:9.973400%
		<0.700000:9.989700%
		<0.800000:10.037800%
		<0.900000:10.021600%
		<1.000000:10.007500%
		=1:0.002700%
**********************************************************/

// 初始化随机数种子
inline void random_seed(unsigned s) {
	srand(s);
}

// 返回随机数：[0,1]
inline float random() {
	return rand()*(1.0f/RAND_MAX);
}

// 快速排序
template<typename T>
void quick_sort(T *x, int l, int u)
{
	if (l >= u) 
		return;
	int i, m;
	m=(l+u)>>1;
	T tmp=x[l];
	x[l]=x[m];
	x[m]=tmp;
	m = l;
	for (i = l+1; i < u; i++) {
		if (x[i] < x[l]) {
			tmp=x[++m];
			x[m]=x[i];
			x[i]=tmp;
		}
	}
	tmp=x[l];
	x[l]=x[m];
	x[m]=tmp;
	quick_sort(x, l, m);
	quick_sort(x, m+1, u);
}

// 冒泡排序
template<typename T>
void bubble_sort(T *x, int n)
{
	T tmp;
	bool sw;
	int i, j, h=0;
	while(h!=n) {
		sw=false;
		i=n-1;
		j=i-1;
		while(j>=h) {
			if(x[j]>x[i]) {
				tmp=x[j];
				x[j]=x[i];
				x[i]=tmp;
				sw=true;
			}
			i=j--;
		}
		if(!sw) break;
		++h;
	}
}

// 对已序数组执行二分搜索,
// 如果成功,返回true,idx为对应的索引
// 否则返回false,idx为第一个大于val的索引或者n(大于最后一个元素)
template<typename T>
bool binary_search(const T *x, int n, const T &val, int &idx)
{
	int m, l=0;
	while(l<n) {
		m=(l+n)>>1;
		if(val==x[m]) {
			idx=m;
			return true;
		}
		if(val<x[m]) 
			n=m;
		else 
			l=m+1;
	}
	idx=l;
	return false;
}

template<typename T, typename Compare>
bool binary_search(const T *x, int n, const T &val, int &idx)
{
	int m, l=0, ret;
	Compare cmp;
	while(l<n) {
		m=(l+n)>>1;
		ret=cmp(x[m],val);
		if(0==ret) {
			idx=m;
			return true;
		}
		if(0<ret) 
			n=m;
		else 
			l=m+1;
	}
	idx=l;
	return false;
}

template<typename Left, typename Right, typename Compare>
bool binary_search(const Left *x, int n, const Right &val, int &idx)
{
	int m, l=0, ret;
	Compare cmp;
	while(l<n) {
		m=(l+n)>>1;
		ret=cmp(x[m],val);
		if(0==ret) {
			idx=m;
			return true;
		}
		if(0<ret) 
			n=m;
		else 
			l=m+1;
	}
	idx=l;
	return false;
}

template<typename T, typename Predicate>
int arrange (T *list, int beg, int end, const Predicate &pred)
{
	int i, j;
	for(i=beg; i<end; ++i) {
		if(!pred(list[i])) 
			break;
	}
	for(j=i+1; j<end; ++j) {
		if(pred(list[j])) {
			std::swap(list[i],list[j]);
			i+=1;
		}
	}
	return i;
}

#define MEM_UNIT_KB 1024
#define MEM_UNIT_MB 1048576
#define MEM_UNIT_GB 1073741824

inline int format_memory_size(float &sz)
{
	if(sz<MEM_UNIT_KB)
		return 0; // Byte
	else if(sz<MEM_UNIT_MB) {
		sz/=MEM_UNIT_KB;
		return 1; // KB
	}
	else if(sz<MEM_UNIT_GB) { 
		sz/=MEM_UNIT_MB;
		return 2; // MB
	}
	else {
		sz/=MEM_UNIT_GB;
		return 3; // GB
	}
}

inline const char* memory_unit(int idx)
{
	static const char* ls_memUnit[4]={"Byte","KB","MB","GB"};
	return ls_memUnit[idx];
}

extern void init_splitter(char **splitter, size_t sz);

inline const char* log_splitter() 
{
	static char *ls_splitter=0;
	if(0==ls_splitter)
		init_splitter(&ls_splitter,80);
	return ls_splitter;
}

}; // namespace wyc

#endif // end of __HEADER_WYC_XUTIL

