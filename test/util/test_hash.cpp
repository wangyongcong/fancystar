#include <string>
#include <deque>
#include <ctime>
#include <iostream>
#include "wyc/util/util.h"
#include "wyc/util/hash.h"

using namespace wyc;

struct ITEM
{
	unsigned key;
	unsigned value;
	bool operator == (const ITEM &item) const
	{
		return key==item.key && value==item.value;
	}
	bool operator != (const ITEM &item) const
	{
		return !(*this==item);
	}
	struct hasher
	{
		static inline unsigned hash (unsigned key)
		{
			return key;
		}
		static inline unsigned get_key (const ITEM &item)
		{
			assert(item.key!=0); // not empty
			assert(item.key!=unsigned(-1)); // not deleted
			return item.key;
		}
		static inline bool is_empty (const ITEM &item)
		{
			return item.key==0;
		}
		static inline bool is_deleted (const ITEM &item)
		{
			return item.key==unsigned(-1);
		}
		static inline void set_empty (ITEM &item)
		{
			item.key=0;
		}
		static inline void set_deleted (ITEM &item)
		{
			assert(item.key!=0); // not empty
			assert(item.key!=unsigned(-1)); // not deleted
			item.key=unsigned(-1);
		}
	};
};

unsigned hashword(unsigned c)
{
   // Bob Jenkin's mix function, possibly overkill for only 32 bits?
   // but a simpler one was no faster, so what the heck
   unsigned a,b;
   a = b = 0x9e3779b9;  // the golden ratio; an arbitrary value 
   a -= b; a -= c; a ^= (c>>13);
   b -= c; b -= a; b ^= (a<<8);
   c -= a; c -= b; c ^= (b>>13);
   a -= b; a -= c; a ^= (c>>12);
   b -= c; b -= a; b ^= (a<<16);
   c -= a; c -= b; c ^= (b>>5);
   a -= b; a -= c; a ^= (c>>3);
   b -= c; b -= a; b ^= (a<<10);
   c -= a; c -= b; c ^= (b>>15);
   return c;
}

void handle_command(int bMode)
{
	typedef xhashtable<unsigned,ITEM,ITEM::hasher> TestHashTable;
	static TestHashTable ht;
	static std::deque<ITEM> stack;
	static unsigned cnt=1;
	std::deque<ITEM>::iterator iter;
	ITEM item;
	unsigned err_num=0, add=rand()%32, fail=0;
	srand(clock());
	bool bShowStack=true;
	switch(bMode) {
		case 0: { // out
			TestHashTable::iterator iter;
			unsigned idx=0;
			for(iter=ht.begin(); iter!=ht.end(); ++iter)
				wyc_print("(%d) %d",idx++,iter->value);
			bShowStack=false;
			break;
		}
		case 1: { // add
			for(unsigned i=0; i<add; ++i)
			{
				item.key=hashword(cnt);
				item.value=cnt++;
				if(ht.add(item.key,item))
					stack.push_back(item);
				else fail+=1;
			}
			wyc_print("add %d item, %d fail",add,fail);
			break;
		}
		case 2: { // del
			unsigned del;
			for(del=0; del<add && !stack.empty(); ++del)
			{
				if(!ht.del(stack.front().key))
					fail+=1;
				stack.pop_front();
			}
			wyc_print("del %d/%d item, %d fail",del,add,fail);
			break;
		}
		case 3: { // reset
			ht.clear();
			stack.clear();
			cnt=1;
			wyc_print("reset");
			break;
		}
		default:
			wyc_print("unknown request");
			return;
	}
	if(bShowStack) {
		for(iter=stack.begin(); iter!=stack.end(); ++iter)
		{
			ht.get(iter->key,item);
			if(item!=*iter) {
				wyc_error("error:item=(%d,%d),ht=(%d,%d)",iter->key,iter->value,item.key,item.value);
				err_num+=1;
			}
		}
	}
	if(err_num) {
		wyc_error("total %d err",err_num);
	}
	else {
		wyc_print("ht:%d/%d(%d del); stack=%d",ht.size(),ht.capacity(),ht.num_deleted(),stack.size());
	}
};

void test_hash()
{
	std::string cmd;
	bool exit=false;
	while(!exit) {
		std::cin>>cmd;
		if(cmd=="exit") {
			exit=true;
			break;
		}
		if(cmd=="out") 
			handle_command(0);
		else if(cmd=="add")
			handle_command(1);
		else if(cmd=="del")
			handle_command(2);
		else if(cmd=="reset")
			handle_command(3);
		else if(cmd=="hash") {
			std::cin>>cmd;
			wyc_print("Hash: 0x%X",hash_crc32(cmd.c_str(),cmd.size()));
		}
		else {
			wyc_sys("Invalid command");
		}
	}
}

#include <vector>
#include <algorithm>

#define INITIAL_SIZE 100
#define ELEMENT_COUNT 1000

#define VALUE(key) (key*key)

void test_dict()
{
	xdict di;
	wyc_print("Testing dictionary...");
	assert(di.capacity()==0);
	di.reserve(INITIAL_SIZE);
	wyc_print("reserved: %d",di.capacity());
	assert(di.capacity()>=INITIAL_SIZE);
	bool ret;
	uintptr_t k, v;
	// insert
	for(k=0; k<ELEMENT_COUNT; ++k) {
		ret=di.add(k,(void*)k);
		assert(ret);
	}
	// invalid data
	ret=di.add(0,HT_EMPTY_POINTER);
	assert(!ret);
	ret=di.add(0,HT_DUMMY_POINTER);
	assert(!ret);
	// const iteration
	unsigned elem_cnt=0;
	xdict::const_iterator con_iter, con_end;
	for(con_iter=di.begin(), con_end=di.end(); con_iter!=con_end; ++con_iter) {
		assert(con_iter->first==(xdict::key_t)con_iter->second);
		elem_cnt+=1;
	}
	assert(elem_cnt==di.size());
	// iterate and modified
	xdict::iterator iter, end;
	for(iter=di.begin(), end=di.end(); iter!=end; ++iter) {
		iter->second=(void*)VALUE(iter->first);
	}
	// get/set elements
	for(k=0; k<ELEMENT_COUNT; ++k) {
		// duplicated is not allow!
		ret=di.add(k,(void*)k);
		assert(!ret);
		// validate value
		v=(uintptr_t)di.get(k);
		assert(VALUE(k)==v);
		// modified value
		di.set(k,(void*)k);
		v=(uintptr_t)di.get(k);
		assert(k==v);
	}
	wyc_print("insertion OK (%d/%d elements)",di.size(),di.capacity());
	// delete
	std::vector<uintptr_t> already_del;
	already_del.reserve(ELEMENT_COUNT);
	unsigned cnt=unsigned(ELEMENT_COUNT*0.6f);
	wyc_print("try del [%d] elements",cnt);
	while(cnt>0) {
		k=uintptr_t(random()*(ELEMENT_COUNT-1));
		ret=di.del(k);
		if(ret) {
			assert(std::find(already_del.begin(),already_del.end(),k)==already_del.end());
			already_del.push_back(k);
		}
		else assert(std::find(already_del.begin(),already_del.end(),k)!=already_del.end());
		--cnt;
	}
	// pop
	cnt=unsigned(ELEMENT_COUNT*0.6f);
	wyc_print("try pop [%d] elements",cnt);
	while(cnt>0) {
		k=uintptr_t(random()*(ELEMENT_COUNT-1));
		if(di.contain(k)) {
			v=(uintptr_t)di.pop(k);
			assert(k==v);
			assert(std::find(already_del.begin(),already_del.end(),k)==already_del.end());
			already_del.push_back(k);
		}
		else assert(std::find(already_del.begin(),already_del.end(),k)!=already_del.end());
		--cnt;
	}
	// trigger shrink
	size_t cap=di.capacity();
	k=already_del.back();
	di.add(k,(void*)k);
	already_del.pop_back();
	wyc_print("remove [%d] elements (%d/%d remain) (no shrink)",already_del.size(),di.size(),di.capacity());
	assert(cap==di.capacity());
	di.set_auto_shrink(true);
	k=already_del.back();
	di.add(k,(void*)k);
	already_del.pop_back();
	assert(cap>di.capacity());
	wyc_print("add one element to trigger shrink (%d/%d remain)",di.size(),di.capacity());
	assert(di.size()==ELEMENT_COUNT-already_del.size());
	// contain 
	for(size_t i=0; i<already_del.size(); ++i) {
		assert(!di.contain(already_del[i]));
	}
	// iteration after deletion	
	elem_cnt=0;
	for(con_iter=di.begin(), con_end=di.end(); con_iter!=con_end; ++con_iter) {
		assert(std::find(already_del.begin(),already_del.end(),con_iter->first)==already_del.end());
		assert(con_iter->first==(uintptr_t)con_iter->second);
		elem_cnt+=1;
	}
	assert(elem_cnt==di.size());
	// re-insert with operator[]
	cnt=unsigned(already_del.size()*0.2);
	wyc_print("re-insert [%d] elements",cnt);
	while(cnt>0) {
		k=already_del.back();
		already_del.pop_back();
		di[k]=(void*)VALUE(k);
		v=(uintptr_t)(void*)di[k];
		assert(VALUE(k)==v);
		di[k]=(void*)k;
		--cnt;
	}
	// final check
	elem_cnt=0;
	for(k=0; k<ELEMENT_COUNT; ++k) {
		if(di.contain(k)) {
			v=(uintptr_t)di.get(k);
			assert(std::find(already_del.begin(),already_del.end(),k)==already_del.end());
			assert(k==v);
			elem_cnt+=1;
		}
		else assert(std::find(already_del.begin(),already_del.end(),k)!=already_del.end());
	}
	assert(elem_cnt==di.size());
	wyc_print("final check OK (%d/%d remain)",di.size(),di.capacity());
	// clear all
	di.clear();
	assert(0==di.size());
	assert(di.begin()==di.end());
	wyc_print("All tests are passed");
}

void test_set()
{
	xset set;
	bool ret;
	uintptr_t v;
	wyc_print("Testing hash set...");
	// insert
	for(v=0; v<ELEMENT_COUNT; ++v) {
		ret=set.add((xset::value_t)v);
		assert(ret);
	}
	// invalid value
	ret=set.add(HT_EMPTY_POINTER);
	assert(!ret);
	ret=set.add(HT_DUMMY_POINTER);
	assert(!ret);
	// duplicated is not allow!
	for(v=0; v<ELEMENT_COUNT; ++v) {
		assert(set.contain((xset::value_t)v));
		ret=set.add((xset::value_t)v);
		assert(!ret);
	}
	// const iteration
	unsigned elem_cnt=0;
	xset::const_iterator con_iter, con_end;
	for(con_iter=set.begin(), con_end=set.end(); con_iter!=con_end; ++con_iter) {
		assert(*con_iter==(xset::value_t)elem_cnt);
		elem_cnt+=1;
	}
	assert(elem_cnt==set.size());
	wyc_print("insertion OK (%d/%d elements)",set.size(),set.capacity());
	// delete
	std::vector<uintptr_t> already_del;
	already_del.reserve(ELEMENT_COUNT);
	unsigned cnt=unsigned(ELEMENT_COUNT*0.9f);
	wyc_print("try del [%d] elements",cnt);
	while(cnt>0) {
		v=uintptr_t(random()*(ELEMENT_COUNT-1));
		ret=set.del((xset::value_t)v);
		if(ret) {
			assert(std::find(already_del.begin(),already_del.end(),v)==already_del.end());
			already_del.push_back(v);
		}
		--cnt;
	}
	wyc_print("remove [%d] elements (%d/%d remain)",already_del.size(),set.size(),set.capacity());
	assert(set.size()==ELEMENT_COUNT-already_del.size());
	// final check
	elem_cnt=0;
	for(v=0; v<ELEMENT_COUNT; ++v) {
		if(set.contain((xset::value_t)v)) {
			assert(std::find(already_del.begin(),already_del.end(),v)==already_del.end());
			elem_cnt+=1;
		}
		else assert(std::find(already_del.begin(),already_del.end(),v)!=already_del.end());
	}
	assert(elem_cnt==set.size());
	wyc_print("final check OK (%d/%d remain)",set.size(),set.capacity());
	// clear all
	set.clear();
	assert(0==set.size());
	assert(set.begin()==set.end());
	wyc_print("All tests are passed");

}

//-------------------------------------------------------------------------

#include "wyc/util/strutil.h"
#define STRING_DICT_MEMORY_DEBUG
#include "wyc/util/string_dict.h"
STRING_DICT_MEMORY_DEBUG_INIT()

void test_string_dict()
{
	printf("testing string dict");
	void *p;
	std::string str;
	std::wstring wstr;
	xstring_dict sdict;
	xwstring_dict wsdict;
	unsigned leaks;

	sdict.reserve(32);
	sdict["A"]=(void*)1;
	sdict["Quick"]=(void*)2;
	sdict["Brown"]=(void*)3;
	p=sdict["A"];
	assert(p==(void*)1);
	p=sdict["Quick"];
	assert(p==(void*)2);
	p=sdict["Brown"];
	assert(p==(void*)3);
	sdict["A"]=(void*)4;
	p=(int*)sdict["A"];
	assert(p==(void*)4);
	p=sdict["Fox"];
	assert(p==0);
	str="Jumps";
	sdict[str]=(void*)5;
	assert(sdict[str]==(void*)5);
	printf(".");

	const xstring_dict &sdict_c = sdict;
	assert((void*)4==sdict_c["A"]);
	p=sdict_c["Quick"];
	assert(p==(void*)2);
	printf(".");

	sdict.clear();
	leaks=xstring_dict::detect_leaks();
	assert(!leaks);
	printf(".");

	
	wsdict.reserve(32);
	for(int i=0; i<100; ++i) {
		wyc::int2str(wstr,i);
		wsdict[wstr]=(void*)i;
	}
	for(int i=0; i<100; ++i) {
		wyc::int2str(wstr,i);
		assert(wsdict[wstr]==(void*)i);
	}
	printf(".");

	wchar_t ws[3];
	ws[2]=0;
	for(int i=0; i<10; ++i) {
		int val = int(random()*99);
		if(val>=10) {
			ws[0]=L'0'+wchar_t(val/10);
			ws[1]=L'0'+wchar_t(val%10);
		}
		else {
			ws[0]=L'0'+wchar_t(val%10);
			ws[1]=0;
		}
		assert(wsdict[ws]==(void*)val);
	}
	printf(".");

	wsdict.clear();
	leaks=xwstring_dict::detect_leaks();
	assert(!leaks);
	printf(".\n");
}
