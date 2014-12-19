#include "fscorepch.h"
#include "wyc/util/util.h"
#include "wyc/util/time.h"
#include "wyc/world/gameobj.h"
#include "wyc/mem/memtracer.h"

using namespace wyc;

namespace 
{

class xcom_dummy : public xcomponent
{
	USE_RTTI;
	bool m_attached;
	bool m_detached;
public:
	xcom_dummy() {
		m_attached=false;
		m_detached=false;
	}
	virtual void on_attach() {
		assert(!m_attached);
		m_attached=true;
	}
	virtual void on_detach() {
		assert(m_attached);
		assert(!m_detached);
		m_detached=true;
	}
};

REG_RTTI(xcom_dummy, xcomponent)

} // anonymous namespace

void test_gameobj_base()
{
	xgameobj *gobj;
	xcomponent *pcom, *pcom2;
	
	wyc_print("Testing add/remove components...");
	gobj=wycnew xgameobj;
	pcom=gobj->add_component<xcom_dummy>();
	pcom2=gobj->add_component<xcom_dummy>();
	assert(pcom==pcom2);
	assert(pcom->gameobj()==gobj);
	pcom2=gobj->get_component<xcom_dummy>();
	assert(pcom==pcom2);
	gobj->remove_component<xcom_dummy>();
	pcom2=gobj->get_component<xcom_dummy>();
	assert(pcom2==0);
	
	pcom=gobj->add_component("xcom_dummy");
	pcom2=gobj->add_component("xcom_dummy");
	assert(pcom==pcom2);
	assert(pcom->gameobj()==gobj);
	pcom2=gobj->get_component("xcom_dummy");
	assert(pcom==pcom2);
	gobj->remove_component("xcom_dummy");
	pcom2=gobj->get_component("xcom_dummy");
	assert(pcom2==0);

	unsigned alive, dead;
	wyc_print("before update:");
	gobj->debug_dump_components(alive,dead);
	assert(alive==0);
	assert(dead==2);
	gobj->update(0,0);
	wyc_print("after update:");
	gobj->debug_dump_components(alive,dead);
	assert(alive+dead==0);

	wyc_print("Testing add/remove children...");
	xgameobj *child;
	std::vector<xgameobj*> pool;
	for(int i=0; i<10; ++i) {
		child=wycnew xgameobj;
		child->incref();
		pool.push_back(child);
		gobj->add_child(child);
		assert(child->parent()==gobj);
	}
	while(pool.size()>0) {
		assert(gobj->child_count()==pool.size());
		unsigned idx=rand()%pool.size();
		child=pool[idx];
		pool[idx]=pool.back();
		pool.pop_back();
		gobj->remove_child(child);
		assert(child->parent()==0);
		child->decref();
	}
	assert(gobj->child_count()==0);


	gobj->delthis();
}



