#include <cassert>
#include "wyc/util/util.h"
#include "wyc/util/time.h"
#include "wyc/util/circuque.h"
#include "wyc/util/priorque.h"

using namespace wyc;

void test_circular_queue()
{
	unsigned i, ret, mid=10, cnt=mid<<1;
	xcircuque<unsigned> que(cnt);
	assert(que.capacity()==cnt);
	xcircuque<unsigned>::const_iterator iter, end;
	xcircuque<unsigned>::iterator iter2, end2;
	for(i=0; i<cnt; ++i) {
		que.push(i);
	}
	assert(que.size()==cnt);
	for(i=0, iter=que.begin(), end=que.end(); iter!=end; ++iter, ++i)
	{
		assert(*iter==i);
	}
	for(i=0; i<mid; ++i) {
		que.push(cnt+i);
	}
	assert(que.size()==cnt);
	for(iter=que.begin(), end=que.end(); iter!=end; ++iter, ++i)
	{
		assert(*iter==i);
	}
	for(i=0; i<mid; ++i) {
		que.pop(ret);
		assert(ret==mid+i);
	}
	assert(que.size()==mid);
	for(i=99, iter2=que.begin(), end2=que.end(); iter2!=end2; ++iter2, --i) {
		*iter2=i;
	}
	for(i=99, iter=que.begin(), end=que.end(); iter!=end; ++iter, --i) {
		assert(*iter==i);
	}
	que.reserve(mid);
	assert(que.capacity()==cnt);
	cnt<<=1;
	que.reserve(cnt);
	assert(que.capacity()==cnt);
	for(i=99, iter=que.begin(), end=que.end(); iter!=end; ++iter, --i) {
		assert(*iter==i);
	}
	que.clear();
	assert(que.size()==0);
	assert(que.capacity()==cnt);
	assert(que.begin()==que.end());
	wyc_print("test xcircuque...OK");
}

void test_priority_queue() 
{
	srand(clock());
	xpriorque<int,std::less<int> > que(512);
	unsigned round=1024;
	int val, prev, re=0;
	for(unsigned i=0; i<round; ++i) {
		val=que.capacity();
		que.push(rand());
		if(que.capacity()!=unsigned(val))
			re+=1;
	}
	assert(que.size()==round);
	bool bOK=que.pop(prev);
	assert(bOK);
	wyc_print("[0] %d",prev);
	for(unsigned i=1; i<round; ++i) {
		bOK=que.pop(val);
		assert(bOK);
		wyc_print("[%d] %d",i,val);
		assert(val>=prev);
		prev=val;
	}
	wyc_sys("Priority queue test OK: %d/%d, %d resize",que.size(),que.capacity(),re);
}

