#include <cassert>
#include <vector>
#include <utility>
#include "wyc/util/util.h"
#include "wyc/util/time.h"

using namespace wyc;

void test_bubble_sort(int n)
{
	int *x=new int[n];
	wyc_print("array:");
	for(int i=0; i<n; ++i) { 
		x[i]=rand();
		printf("%d ",x[i]);
	}
	printf("\n");
	wyc::xcode_timer ct;
	ct.start();
	bubble_sort(x,n);
	ct.stop();
	wyc_print("after sort:");
	printf("%d ",x[0]);
	int err=0;
	for(int i=1; i<n; ++i) {
		printf("%d ",x[i]);
		if(x[i]<x[i-1])
			++err;
	}
	printf("\n");
	delete [] x;
	if(err>0) {
		wyc_error("Error!");
		assert(0);
	}
	wyc_print("bubble sort time: %.4f",ct.get_time());
}

int _test_quick_sort(int n, int *x)
{
	wyc::xcode_timer ct;
	printf("array: [ ");
	for(int i=0; i<n; ++i) { 
		printf("%d ",x[i]);
	}
	printf(" ]\n");
	ct.start();
	quick_sort(x,0,n);
	ct.stop();
	wyc_print("after sort [%f]:",ct.get_time());
	printf("%d ",x[0]);
	int err=0;
	for(int i=1; i<n; ++i) {
		printf("%d ",x[i]);
		if(x[i]<x[i-1])
			++err;
	}
	printf("\n");
	return err;
}

void test_quick_sort(int n)
{
	int *x=new int[n];
	int err=0;
	// random array
	for(int i=0; i<n; ++i) 
		x[i]=rand();
	err+=_test_quick_sort(n,x);
	// sorted array: asc
	for(int i=0; i<n; ++i) 
		x[i]=i;
	err+=_test_quick_sort(n,x);
	// sorted array: desc
	for(int i=0, j=n-1; i<n; ++i, --j) 
		x[i]=j;
	err+=_test_quick_sort(n,x);
	
	delete [] x;
	if(err>0) {
		wyc_error("Error!");
		assert(0);
	}
}

struct int_compare
{
	int operator() (int left, int right)
	{
		return left-right;
	}
};

void order_array()
{
	std::vector<int> data;
	size_t cnt=100;
	data.reserve(cnt+1);
	data.push_back(rand()%1000);
	int idx, val;
	for(size_t i=0; i<cnt; ++i) {
		val=rand()%1000;
		binary_search<int,int_compare>(&data[0],data.size(),val,idx);
		data.insert(data.begin()+idx,val);
	}
	for(size_t i=0, j=1; j<data.size(); ++i, ++j) {
		assert(data[i]<=data[j]);
	}
}

void test_binary_search()
{
	wyc_print("test binary searching...");
	int idx;
	bool find;
	// smoke test
	int data[8]={1, 4, 42, 55, 67, 87, 100, 245};
	find=binary_search(data,8,1,idx);
	assert( binary_search(data,8,  1,idx) && 0==idx);
	assert( binary_search(data,8,  4,idx) && 1==idx);
	assert( binary_search(data,8, 87,idx) && 5==idx);
	assert( binary_search(data,8,245,idx) && 7==idx);
	assert(!binary_search(data,8, 41,idx) && 2==idx);
	// boundery test
	int data2[1]={7};
	assert( !binary_search(data2, 1,9,idx) && 1==idx);
	assert(  binary_search(data2, 1,7,idx) && 0==idx);
	assert( !binary_search(data2, 0,7,idx) && 0==idx);
	assert( !binary_search(data2,-1,7,idx) && 0==idx);
	// random test
	int n=256, val;
	int *data3=new int[n];
	for(int i=0; i<n; ++i)
		data3[i]=rand();
	quick_sort(data3,0,n);
	for(int i=0; i<n; ++i) {
		find=binary_search(data3,n,data3[i],idx);
		assert(find && data3[idx]==data3[i]);
	}
	assert( binary_search(data3,n,data3[0],idx) && 0==idx);
	assert( binary_search(data3,n,data3[n-1],idx) && n-1==idx);
	assert(!binary_search(data3,n,data3[0]-1,idx) && 0==idx);
	assert(!binary_search(data3,n,data3[n-1]+1,idx) && n==idx);
	// ordering test
	wyc_print("range: [%d ~ %d]",data3[0],data3[n-1]);
	for(int i=0; i<n; ++i) {
		val=rand();
		if(!binary_search(data3,n,val,idx)) {
			if(n==idx) {
				assert(val>data3[n-1]);
				continue;
			}
			wyc_print("[%d] %d: %d, %d, %d",i,val,idx>0?data3[idx-1]:-1,data3[idx],idx<n-1?data3[idx+1]:-1);
			assert(data3[idx]>val);
			if(idx>0) {
				assert(data3[idx-1]<val);
			}
		}
	}
	delete [] data3;
	order_array();
}

struct int_compare_unary
{
	int m_cmp;
	int_compare_unary(int cmp) : m_cmp(cmp) {
	}
	bool operator() (int val) const {
		return val<m_cmp;
	}
};

void test_arrange()
{
	const int count = 64;
	const int range = 16;
	int *list=new int[count];
	for (int i=0; i<count; ++i)
		list[i]=int(wyc::random()*range);
	int beg=0, end=count;
	for (int i=0; i<=range; ++i) {
		beg=arrange(list,beg,end,int_compare_unary(i));
	}
	for (int i=1; i<count; ++i) {
		assert(list[i]>=list[i-1]);
	}
	for (int i=0; i<count; ++i) 
		printf("%2d, ",list[i]);
	printf("END\n");
	beg=arrange(list,0,end,int_compare_unary(range+1));
	assert(beg==end);
	beg=arrange(list,0,end,int_compare_unary(-1));
	assert(beg==0);
	delete [] list;
}
