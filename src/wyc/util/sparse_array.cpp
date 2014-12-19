#include "wyc/util/sparse_array.h"
#include <algorithm>

namespace wyc {

xsparse_array::xsparse_array() {
	m_deleted=0;
}

void xsparse_array::insert(void *pdata) {
	size_t i=0, len=m_array.size();
	if(m_deleted==0 || len==0) {
		push_back(pdata);
		return;
	}
	for(; i<len && m_array[i]!=0; ++i);
	assert(i<len);
	m_array[i]=pdata;
	m_deleted-=1;
}

void xsparse_array::pack(float threshold) {
	if(!m_deleted || float(m_deleted)/m_array.size()<=threshold) 
		return;
	std::vector<void*>::iterator iter, end=m_array.end();
	iter=std::remove(m_array.begin(),end,(void*)0);
	m_array.erase(iter,end);
	m_deleted=0;
}

}; // namespace wyc

