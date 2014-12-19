#ifndef __HEADER_WYC_GAMEOBJ
#define __HEADER_WYC_GAMEOBJ

#include "wyc/world/component.h"
#include "wyc/world/com_transform.h"
#include "wyc/world/com_renderer.h"
#include "wyc/world/com_material.h"

#ifdef _DEBUG
	#define CHECK_UNIQUE(com,new_com,com_type) \
	if(new_com && com) {\
		wyc_error("["#com_type"] already exist");\
		assert(0);\
	}
#else
	#define CHECK_UNIQUE(com,new_com,com_type)
#endif

namespace wyc
{

class xgameobj : public xobject
{
	USE_RTTI;
	std::string m_name;
	xcomponent *m_pComHead, **m_pComTail;
	wyc::xdict m_components;
	xgameobj *m_parent;
	std::vector<xgameobj*> m_child;
	bool m_lockChildList;
protected:
	// build-in components
	xcom_transform *m_transform;
	xcom_renderer *m_renderer;
	xcom_material *m_material;
public:
	xgameobj();
	virtual void on_destroy();
	virtual void update(double accum_time, double frame_time);
	virtual void on_update(double accum_time, double frame_time) {
		// do custom update
		accum_time,frame_time;
	}
	virtual void render(xrenderer *pRenderer);
	inline void set_name(const std::string &name) {
		m_name=name;
	}
	inline const std::string& get_name() const {
		return m_name;
	}
	xcomponent* get_component(const char *type_name);
	xcomponent* add_component(const char *type_name);
	void remove_component(const char *type_name);
	void remove_component(xcomponent *pcom);
	template<typename T>
	T* add_component() {
		const char *name=T::get_class()->name;
		T *pcom = (T*)m_components.get((xdict::key_t)name);
		if(pcom)
			return pcom;
		pcom=wycnew T;
		pcom->incref();
		pcom->m_parentGameobj=this;
		pcom->m_pNextCom=m_pComHead;
		m_pComHead=pcom;
		m_components.add((xdict::key_t)name,(xdict::value_t)pcom);
		set_build_in(T::get_class(),pcom);
		pcom->on_attach();
		return pcom;
	}
	template<typename T>
	inline void remove_component() 
	{
		xcomponent *pcom;
		if(m_components.pop((xdict::key_t)T::get_class()->name,(xdict::value_t&)pcom)) {
			set_build_in(T::get_class(),0);
			pcom->on_detach();
			pcom->m_parentGameobj=0;
		}
	}
	template<typename T>
	inline T* get_component()
	{
		T *pcom = (T*) m_components.get((xdict::key_t)T::get_class()->name);
		return pcom;
	}
	template<>
	inline xcom_transform* get_component<xcom_transform>() {
		return m_transform;
	}
	template<>
	inline xcom_renderer* get_component<xcom_renderer>() {
		return m_renderer;
	}
	template<>
	inline xcom_material* get_component<xcom_material>() {
		return m_material;
	}

	inline xgameobj* parent() {
		return m_parent;
	}
	void add_child(xgameobj *gobj);
	void remove_child(xgameobj *gobj);
	inline unsigned child_count() const {
		return m_child.size();
	}
	xgameobj* get_child(const char *name);

	inline xcom_transform* get_transform() {
		return m_transform;
	}
	//---------------------------------------------------------
	// debug interfaces
	//---------------------------------------------------------
	void debug_dump_components(unsigned &alive, unsigned &dead);
protected:
	void set_build_in(const xobj_class *pci, xcomponent *pcom);
	void update_component(double accum_time, double frame_time);
	void update_children(double accum_time, double frame_time);
	void update_transform(xcom_transform *parent_trans=0, bool rebuild=false);
};

}; // namespace wyc

#endif // __HEADER_WYC_GAMEOBJ
