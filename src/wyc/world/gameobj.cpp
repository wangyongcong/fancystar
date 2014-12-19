#include "fscorepch.h"
#include "wyc/world/gameobj.h"

namespace wyc
{

REG_RTTI(xcomponent,xobject)

REG_RTTI(xgameobj,xobject)

xgameobj::xgameobj() 
{
	m_pComHead=0;
	m_parent=0;
	m_lockChildList=false;
	// build-in component
	m_transform=0;
	m_renderer=0;
	m_material=0;
}

void xgameobj::on_destroy() 
{
	// clear components
	xcomponent *iter=m_pComHead, *del;
	while(iter) {
		del=iter;
		iter=iter->m_pNextCom;
		del->m_pNextCom=0;
		del->decref();
	}
	m_components.clear();
	m_pComHead=0;
	// build-in components
	m_transform=0;
	m_renderer=0;
	m_material=0;
	// clear child
	for(unsigned i=0; i<m_child.size(); ++i) {
		m_child[i]->m_parent=0;
		m_child[i]->decref();
	}
	m_child.clear();
	m_parent=0;
}

xcomponent* xgameobj::add_component(const char *type_name) 
{
	uint32_t cid=strhash(type_name);
	xcomponent *pcom=m_pComHead;
	while(pcom) {
		if(pcom->my_class()->id==cid && pcom->m_parentGameobj)
			return pcom;
		pcom=pcom->m_pNextCom;
	}
	xobject *pobj=xobject::create_object(type_name);
	if(!pobj) 
		return 0;
	assert(xcomponent::is_kind(pobj));
	pcom=(xcomponent*)pobj;
	pcom->incref();
	pcom->m_parentGameobj=this;
	pcom->m_pNextCom=m_pComHead;
	m_pComHead=pcom;
	const xobj_class *pci=pcom->my_class();
	m_components.add((xdict::key_t)pci->name,(xdict::value_t)pcom);
	set_build_in(pci,pcom);
	pcom->on_attach();
	return pcom;
}

void xgameobj::remove_component(const char *type_name) 
{
	uint32_t cid=strhash(type_name);
	const xobj_class *pci;
	xcomponent *pcom=m_pComHead;
	while(pcom) {
		pci=pcom->my_class();
		if(pci->id==cid && pcom->m_parentGameobj) {
			m_components.del((xdict::key_t)pci->name);
			set_build_in(pci,0);
			pcom->on_detach();
			pcom->m_parentGameobj=0;
			return;
		}
		pcom=pcom->m_pNextCom;
	}
}

xcomponent* xgameobj::get_component(const char *type_name)
{
	uint32_t cid=strhash(type_name);
	xcomponent *pcom=m_pComHead;
	while(pcom) {
		if(pcom->my_class()->id==cid && pcom->m_parentGameobj)
			return pcom;
		pcom=pcom->m_pNextCom;
	}
	return 0;
}

void xgameobj::remove_component(xcomponent *pcom)
{
	xcomponent *iter=m_pComHead;
	while(iter) {
		if(iter==pcom) {
			const xobj_class *pci=pcom->my_class();
			m_components.del((xdict::key_t)pci->name);
			set_build_in(pci,0);
			pcom->on_detach();
			pcom->m_parentGameobj=0;
			return;
		}
		iter=iter->m_pNextCom;
	}
}

void xgameobj::set_build_in(const xobj_class *pci, xcomponent *pcom) 
{
	if(pci==xcom_transform::get_class()) {
		CHECK_UNIQUE(m_transform,pcom,xcom_transform);
		m_transform=(xcom_transform*)pcom;
	}
	else if(is_derived(xcom_renderer::get_class(),pci)) {
		CHECK_UNIQUE(m_renderer,pcom,xcom_renderer);
		m_renderer=(xcom_renderer*)pcom;
	}
	else if(is_derived(xcom_material::get_class(),pci)) {
		CHECK_UNIQUE(m_material,pcom,xcom_material);
		m_material=(xcom_material*)pcom;
	}
}

void xgameobj::add_child(xgameobj *gobj)
{
	assert(!m_lockChildList && "禁止在遍历列表过程中修改列表");
	if(this==gobj->m_parent)
		return;
	gobj->incref();
	if(gobj->m_parent) 
		gobj->m_parent->remove_child(gobj);
	gobj->m_parent=this;
	m_child.push_back(gobj);
}

void xgameobj::remove_child(xgameobj *gobj)
{
	assert(!m_lockChildList && "禁止在遍历列表过程中修改列表");
	if(this!=gobj->m_parent)
		return;
	unsigned cnt=m_child.size();
	for(unsigned i=0; i<cnt; ++i) {
		if(m_child[i]==gobj) {
			m_child[i]=m_child.back();
			m_child.pop_back();
			gobj->m_parent=0;
			gobj->decref();
			return;
		}
	}
}

xgameobj* xgameobj::get_child(const char *name)
{
	unsigned cnt=m_child.size();
	for(unsigned i=0; i<cnt; ++i) {
		if(m_child[i]->get_name() == name) 
			return m_child[i];
	}
	return 0;
}

void xgameobj::update_transform(xcom_transform *parent_trans, bool rebuild)
{
	if(!m_transform) 
		return;
	rebuild=m_transform->update(parent_trans,rebuild);
	for(unsigned i=0, cnt=m_child.size(); i<cnt; ++i) {
		m_child[i]->update_transform(parent_trans,rebuild);
	}
}

void xgameobj::update(double accum_time, double frame_time)
{
	update_component(accum_time,frame_time);
	on_update(accum_time,frame_time);
	update_children(accum_time,frame_time);
	update_transform();
}

void xgameobj::update_component(double accum_time, double frame_time)
{
	// update components
	// 在遍历过程中允许添加/删除component: 
	//		添加component只在head插入, 唯一的副作用就是
	//		当(prev==&m_pComHead)的时候, 新插入的component会马上update
	//		所有的删除接口都只是激活一个标记,真正的弹出队列操作延迟到update的遍历中执行
	xcomponent *pcom=m_pComHead, **prev=&m_pComHead;
	while(pcom) {
		if(!pcom->m_parentGameobj) {
			*prev=pcom->m_pNextCom;
			pcom->m_pNextCom=0;
			pcom->decref();
		}
		else {
			prev=&pcom->m_pNextCom;
			pcom->update(accum_time,frame_time);
		}
		pcom=*prev;
	}
}

void xgameobj::update_children(double accum_time, double frame_time)
{
	// 在遍历child list的过程中不允许修改child list
	m_lockChildList=true;
	xgameobj *gobj;
	unsigned cnt=m_child.size();
	unsigned i=0;
	while(i<cnt) {
		gobj=m_child[i];
		gobj->update_component(accum_time,frame_time);
		gobj->on_update(accum_time,frame_time);
		gobj->update_children(accum_time,frame_time);
		++i;
	}
	m_lockChildList=false;
}

void xgameobj::render(xrenderer *pRenderer)
{
	if(!m_renderer || !m_renderer->is_enabled()) 
		return;
	m_renderer->before_render(pRenderer);
	m_renderer->render(pRenderer);
	m_lockChildList=true;
	xgameobj *gobj;
	for(unsigned i=0, cnt=m_child.size(); i<cnt; ++i) {
		gobj=m_child[i];
		gobj->render(pRenderer);
	}
	m_lockChildList=false;
	m_renderer->after_render(pRenderer);
}

void xgameobj::debug_dump_components(unsigned &alive, unsigned &dead)
{
	alive=0;
	dead=0;
	char splitter[80];
	memset(splitter,'-',sizeof(splitter));
	splitter[0]='+';
	splitter[78]='\n';
	splitter[79]=0;
	wyc_print(splitter);
	xcomponent *iter=m_pComHead;
	if(iter) {
		printf("| GOBJ[%p] components\n",this);
		do {
			printf("|\t%s: %s\n",iter->my_class()->name,iter->m_parentGameobj?"alive":"removed");
			if(iter->m_parentGameobj) {
				assert(this==iter->m_parentGameobj);
				alive+=1;
			}
			else
				dead+=1;
			iter=iter->m_pNextCom;
		} while(iter);
	}
	else printf("| gobj[%p] no components\n",this);
	printf(splitter);
}

}; // namespace wyc

