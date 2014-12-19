#include "wyc/util/util.h"
#include "wyc/util/wsbucket.h"
#include "wyc/util/time.h"

struct test_bucket_element
{
	static size_t ms_counter;
	int first, second;
	test_bucket_element() {
		++ms_counter;
	}
	test_bucket_element(int f, int s) {
		++ms_counter;
		first=f;
		second=s;
	}
	test_bucket_element(const test_bucket_element &elem) {
		++ms_counter;
		first=elem.first;
		second=elem.second;
	}
	~test_bucket_element() {
		--ms_counter;
	}
	test_bucket_element& operator= (const test_bucket_element &elem) {
		first=elem.first;
		second=elem.second;
	}
};

size_t test_bucket_element::ms_counter=0;

void test_wsbucket()
{
	typedef test_bucket_element element_t;
	wyc::xwsbucket<element_t> bucket;
	element_t *pelem;
	wyc::xwsbucket<element_t>::iterator iter, end;
	wyc::xwsbucket<element_t>::const_iterator c_iter, c_end;
	unsigned total;
	// 测试只有1个BUCKET时的数据遍历
	size_t count=bucket.page_size();
	while(count>1) {
		bucket.push_back(element_t(count,0));
		--count;
	}
	assert(bucket.size()<bucket.page_size());
	for(iter=bucket.begin(), end=bucket.end(); iter!=end; ++iter) {
		total=iter->first+iter->second;
	}
	// 测试刚好放满1个BUCKET时的数据遍历
	count=bucket.page_size();
	bucket.clear();
	while(count>0) {
		bucket.push_back(element_t(count,0));
		--count;
	}
	assert(bucket.size()==bucket.page_size());
	for(iter=bucket.begin(), end=bucket.end(); iter!=end; ++iter) {
		total=iter->first+iter->second;
	}
	bucket.clear();
	// 测试多个BUCKET时的数据遍历
	wyc::xdate dt;
	dt.get_date();
	count=unsigned(bucket.page_size()*(1.0f+dt.second()/60.0f));
	total=count;
	for(unsigned i=0; i<count; ++i) {
		bucket.push_back(element_t(i,0));
	}
	wyc_print("section[1]=%d",count);
	// 测试数据写入
	count=bucket.page_size()/2;
	total+=count;
	for(unsigned i=0; i<count; ++i) {
		pelem=bucket.push_back();
		pelem->first=0;
		pelem->second=i;
	}
	wyc_print("section[2]=%d",count);
	assert(bucket.size()==total && total>bucket.page_size());
	// 测试non-const和const遍历
	for(iter=bucket.begin(), end=bucket.end(); iter!=end; ++iter) {
		iter->first+=1;
		iter->second+=1;
	}
	for(c_iter=bucket.begin(), c_end=bucket.end(); c_iter!=c_end; ++c_iter) {
		wyc_print("(%d,%d)",c_iter->first,c_iter->second);
	}
	// 测试清除操作
	bucket.clear();
	assert(element_t::ms_counter==0);
	assert(bucket.size()==0);
	iter=bucket.begin();
	end=bucket.end();
	assert(iter==end);
}

