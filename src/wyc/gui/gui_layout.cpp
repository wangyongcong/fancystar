#include "fscorepch.h"
#include "wyc/gui/gui_layout.h"
#include "wyc/gui/guiobj.h"
#include "wyc/util/util.h"

namespace wyc
{

REG_RTTI(xlayout,xobject)

struct xguiobj_zcompare
{
	int operator() (const xguiobj *left, const xguiobj *right) const
	{
		if(left->get_zorder()==right->get_zorder()) {
			if(left==right)
				return 0;
			return left<right?-1:1;
		}
		return left->get_zorder()<right->get_zorder()?-1:1;
	}
};

struct xguiobj_zpair_compare
{
	int operator() (const xguiobj *left, const std::pair<xguiobj*,float> &right) const
	{
		if(left->get_zorder()==right.second) {
			if(left==right.first)
				return 0;
			return left<right.first?-1:1;
		}
		return left->get_zorder()<right.second?-1:1;
	}
};

xlayout::xlayout() 
{
	m_tiles=wycnew xtile_buffer;
	m_rebuild=false;
	m_redraw=false;
	m_remove_elem=false;
}

void xlayout::on_destroy()
{
	for(unsigned i=0, cnt=m_elements.size(); i<cnt; ++i)
		_del_element_impl(m_elements[i]);
	m_elements.clear();
	m_tiles=0;
}

void xlayout::update()
{
	if(m_elem_reorder.size())
	{
		reorder_elements();
		m_redraw=true;
	}
	if(m_remove_elem) {
		m_remove_elem=false;
		remove_elements();
		m_redraw=true;
	}
}

void xlayout::render(xrenderer *rd)
{
	if(m_rebuild) {
		m_rebuild=false;
		update_elements();
	}
	if(m_redraw) {
		m_redraw=false;
		m_tiles->clear_draw_buffer();
		xguiobj *gui_obj;
		for(unsigned i=0, cnt=m_elements.size(); i<cnt; ++i) 
		{
			gui_obj=m_elements[i];
			if(gui_obj->visible()) 
				gui_obj->draw();
		}
	}
	m_tiles->render(rd);
}

xguiobj* xlayout::get_element(const char *name)
{
	for(size_t i=0, cnt=m_elements.size(); i<cnt; ++i) {
		if(m_elements[i]->name()==name)
			return m_elements[i];
	}
	return 0;
}

xguiobj* xlayout::new_element(const char *type, const char *name, float x, float y, float z)
{
	if(!type || !name)
		return 0;
	xobject *obj=xobject::create_object(type);
	if(!obj) return 0;
	xguiobj *gui_obj=wyc_safecast(xguiobj,obj);
	if(!gui_obj) {
		wyc_error("xlayout::new_element: [%s] is not a xguiobj",type);
		obj->delthis();
		return 0;
	}
	gui_obj->incref();
	gui_obj->m_layer=this;
	gui_obj->m_name=name;
	gui_obj->set_id(strhash(name));
	gui_obj->m_pos.set(x,y,z);
	if(!m_name2obj.add(gui_obj->id(),(xdict::value_t)gui_obj)) {
		wyc_error("xlayout::new_element: [%s] already exist, name conflict",name);
		gui_obj->decref();
		return 0;
	}
	if(m_elements.size()) {
		int idx;
		binary_search<xguiobj*,xguiobj_zcompare>(&m_elements[0],m_elements.size(),gui_obj,idx);
		m_elements.insert(m_elements.begin()+idx,gui_obj);
	}
	else m_elements.push_back(gui_obj);
	return gui_obj;
}

void xlayout::del_element(xguiobj *obj)
{
	assert(obj->get_layer()==this);
	m_remove_elem=true;
}

void xlayout::remove_elements()
{
	unsigned i=0, cnt=m_elements.size();
	while(i<cnt)
	{
		if(m_elements[i]->is_removed()) {
			_del_element_impl(m_elements[i]);
			break;
		}
		++i;
	}
	unsigned j=i+1;
	while(j<cnt)
	{
		if(m_elements[j]->is_removed())
			_del_element_impl(m_elements[j]);
		else
			m_elements[i++]=m_elements[j];
		++j;
	}
	if(i<cnt)
	{
		m_elements.erase(m_elements.begin()+i,m_elements.end());
	}
}

void xlayout::_del_element_impl(xguiobj *gui_obj)
{
	assert(gui_obj->get_layer()==this);
	m_name2obj.del(gui_obj->id());
	if(gui_obj->m_mesh) {
		gui_obj->m_mesh->delthis();
		gui_obj->m_mesh=0;
	}
	gui_obj->m_layer=0;
	gui_obj->decref();
}

void xlayout::reset_zorder(xguiobj *obj, float z)
{
	if(obj->get_layer()!=this)
		return;
	m_elem_reorder.push_back(std::pair<xguiobj*,float>(obj,z));
}

void xlayout::reorder_elements()
{
	int pre_idx, new_idx;
	typedef std::pair<xguiobj*,float> pair_t;
	std::vector<pair_t>::iterator iter, end;
	for(iter=m_elem_reorder.begin(), end=m_elem_reorder.end();
		iter!=end; ++iter)
	{
		if(!iter->first->need_reorder() || 
			!binary_search<xguiobj*,pair_t,xguiobj_zpair_compare>(&m_elements[0],m_elements.size(),*iter,pre_idx))
			continue;
		iter->first->reset_reorder_bit();
		binary_search<xguiobj*,xguiobj_zcompare>(&m_elements[0],m_elements.size(),iter->first,new_idx);
		if(pre_idx==new_idx)
			continue;
		if(pre_idx<new_idx) {
			for(int i=pre_idx, j=i+1; j<new_idx; ++i, ++j)
				m_elements[i]=m_elements[j];
		}
		else {
			for(int i=pre_idx, j=i-1; j>=0; --i, --j)
				m_elements[i]=m_elements[j];
		}
		m_elements[new_idx]=iter->first;
	}
	m_elem_reorder.clear();
}

void xlayout::update_elements()
{
	xguiobj *gui_obj;
	for(unsigned i=0, cnt=m_elements.size(); i<cnt; ++i) 
	{
		gui_obj=m_elements[i];
		if(!gui_obj->need_rebuild()) 
			continue;
		gui_obj->reset_rebuild_bit();
		gui_obj->build_mesh();
	}
}

} // namespace wyc
