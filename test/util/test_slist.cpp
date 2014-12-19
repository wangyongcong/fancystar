#include "wyc/util/util.h"
#include "wyc/util/time.h"
#include "wyc/util/list.h"

using namespace wyc;

void test_singlelist()
{
	xsingle_list<int> sl;
	int i;
	for(i=500; i>0; --i) 
		sl.push_front(i);
	for(i=501; i<=1000; ++i)
		sl.push_back(i);
	if(sl.size()==1000) {
		wyc_print("push ok: memop=%d",sl.new_node());
	}
	else {
		wyc_error("push err: size=%d",sl.size());
	}
	xsingle_list<int>::iterator iter;
	xsingle_list<int>::const_iterator con_iter=sl.begin();
	i=1;
	while(con_iter!=sl.end()) 
	{
		if(*con_iter!=i) {
			wyc_error("check err:%d,%d",i,*con_iter);
			return;
		}
		++i;
		++con_iter;
	}
	wyc_print("check end 1");
	i=1;
	while(i<=500) {
		sl.pop_front();
		i+=1;
	}
	if(sl.size()==500) {
		wyc_print("pop ok: memop=%d",sl.new_node());
	}
	else {
		wyc_error("pop err: size=%d",sl.size());
	}
	con_iter=sl.begin();
	while(con_iter!=sl.end()) 
	{
		if(*con_iter!=i) {
			wyc_error("check err:%d,%d",i,*con_iter);
			return;
		}
		++i;
		++con_iter;
	}
	wyc_print("check end 2");
	iter=sl.begin();
	bool badd=true;
	while(iter!=sl.end()) {
		if(badd) 
			iter=sl.insert(iter,1000-*iter);
		badd=!badd;
		iter++;
	}
	sl.insert(iter,1001);
	if(sl.size()==1001) {
		wyc_print("insert ok: memop=%d",sl.new_node());
	}
	else {
		wyc_error("insert err: size=%d",sl.size());
	}
	iter=sl.begin();
	badd=true;
	while(iter!=sl.end())
	{
		if(badd)
			iter=sl.erase(iter);
		else iter++;
		badd=!badd;
	}
	if(sl.size()==500) {
		wyc_print("erase ok: memop=%d",sl.new_node());
	}
	else {
		wyc_error("erase err: size=%d",sl.size());
	}
	i=501;
	con_iter=sl.begin();
	while(con_iter!=sl.end()) 
	{
		if(*con_iter!=i) {
			wyc_error("check err:%d,%d",i,*con_iter);
			return;
		}
		++i;
		++con_iter;
	}
	wyc_print("check end 3");
	sl.clear();
	wyc_print("clear: size=%d",sl.size());
	if(sl.new_node()!=0) 
		wyc_error("memory leak!");
}
