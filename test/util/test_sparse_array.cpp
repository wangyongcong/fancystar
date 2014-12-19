#include <iostream>
#include <ctime>
#include "wyc/util/sparse_array.h"

void print_sparse_array(const wyc::xsparse_array &spa) 
{
	std::cout<<"size(cap):\t"<<spa.size()<<"("<<spa.capacity()<<")"<<std::endl;
	std::cout<<"gap(elem):\t"<<spa.gap()<<"("<<spa.size()-spa.gap()<<")"<<std::endl;
	std::cout<<"vacancy:\t"<<spa.vacancy()<<std::endl;
	for(size_t i=0; i<spa.size(); ++i) 
		std::cout<<uintptr_t(spa[i])<<" ";
	std::cout<<std::endl;
}

void test_sparse_array()
{
	srand(clock());
	wyc::xsparse_array spa;
	unsigned i=1, idx;
	for(; i<=32; ++i) 
		spa.push_back((void*)i);
	assert(spa.size()==32);
	assert(spa.gap()==0);
	std::cout<<"==init============================"<<std::endl;
	print_sparse_array(spa);
	for(i=0; i<10; ++i) {
		idx=rand()%spa.size();
		spa.erase(idx);
	}
	assert(spa.size()==32);
	assert(spa.gap()>0);
	std::cout<<"==del============================="<<std::endl;
	print_sparse_array(spa);
	for(i=0; i<10; ++i) {
		spa.insert((void*)99);
	}
	assert(spa.gap()==0);
	std::cout<<"==insert=========================="<<std::endl;
	print_sparse_array(spa);
	for(i=0; i<10; ++i) {
		idx=rand()%spa.size();
		spa.erase(idx);
	}
	assert(spa.gap()>0);
	unsigned elem=spa.size()-spa.gap();
	std::cout<<"==del============================="<<std::endl;
	print_sparse_array(spa);
	spa.pack(0);
	assert(spa.gap()==0);
	assert(spa.size()==elem);
	std::cout<<"==pack============================"<<std::endl;
	print_sparse_array(spa);
}

