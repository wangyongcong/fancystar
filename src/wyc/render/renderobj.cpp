#include "fscorepch.h"
#include "wyc/render/renderobj.h"

namespace wyc
{

REG_RTTI(xrenderobj,xobject)

xrenderobj::xrenderobj()
{
	m_visible=true;
}

xrenderobj::~xrenderobj()
{
}

//----------------------------------------------

xrenderlist::xrenderlist()
{
}

xrenderlist::~xrenderlist()
{
	clear();
}

void xrenderlist::clear()
{
	render_list_t::iterator iter, end;
	for(iter=m_rlist.begin(), end=m_rlist.end(); iter!=end; ++iter) {
		iter->second->decref();
	}
	m_rlist.clear();
}

void xrenderlist::append(xrenderobj *pobj)
{
	// TODO: 还要检查唯一性？
	int priority=0;
	if(m_rlist.size()) 
		priority=m_rlist.back().first;
	m_rlist.push_back(std::pair<int,xrenderobj*>(priority,pobj));
	pobj->incref();
}

void xrenderlist::insert(xrenderobj *pobj, int priority)
{
	// TODO: 这是有序列表,使用二分搜索提高效率
	// 还要检查唯一性？
	render_list_t::iterator iter, end;
	for(iter=m_rlist.begin(), end=m_rlist.end(); iter!=end; ++iter) {
		if(iter->first>priority) {
			pobj->incref();
			m_rlist.insert(iter,std::pair<int,xrenderobj*>(priority,pobj));
			break;
		}
	}
	if(iter==end) {
		pobj->incref();
		m_rlist.push_back(std::pair<int,xrenderobj*>(priority,pobj));
	}
}

void xrenderlist::remove(xrenderobj *pobj)
{
	// TODO: 这是有序列表,使用二分搜索提高效率
	render_list_t::iterator iter, end;
	for(iter=m_rlist.begin(), end=m_rlist.end(); iter!=end; ++iter) {
		if(iter->second==pobj) {
			pobj->decref();
			iter->second=0;
			break;
		}
	}
}

void xrenderlist::draw(xrenderer *pRenderer)
{
	xrenderobj *pobj;
	int erase=0;
	render_list_t::iterator iter, end;
	for(iter=m_rlist.begin(), end=m_rlist.end(); iter!=end; ++iter) {
		pobj=iter->second;
		if(!pobj) {
			erase+=1;
			continue;
		}
		if(pobj->dead()) {
			pobj->decref();
			iter->second=0;
			erase+=1;
			continue;
		}
		if(pobj->visible())
			pobj->draw(pRenderer);
	}
	if(erase>0) {
		iter=std::remove_if(m_rlist.begin(),end,is_null_obj());
		assert(erase==end-iter);
		m_rlist.erase(iter,end);
	}
}

}; // namespace wyc
