#include "fscorepch.h"
#include "aoi_mgr.h"
#include "obj_base.h"

xaoi_object::xaoi_object(void *scene_obj, const xvec3f_t &radius, unsigned group, unsigned group_filter) : \
	xaoi_entity(radius)
{
	m_scene_obj = scene_obj;
	set_mask(group);
	set_filter(group_filter);
}

void xaoi_object::on_enter (xaoi_entity* en)
{
	assert(m_scene_obj);
	xaoi_object *obj = (xaoi_object*)en;
	assert(obj && obj->m_scene_obj);
	((xbase_object*)m_scene_obj)->aoi_on_enter((xbase_object*)obj->m_scene_obj);
}

void xaoi_object::on_leave (xaoi_entity* en)
{
	assert(m_scene_obj);
	xaoi_object *obj = (xaoi_object*)en;
	assert(obj && obj->m_scene_obj);
	((xbase_object*)m_scene_obj)->aoi_on_leave((xbase_object*)obj->m_scene_obj);
}

xaoi_manager* aoi_manager_create(int dimension)
{
	return new xaoi_manager();
}

void aoi_manager_destroy(xaoi_manager *mgr)
{
	delete mgr;
}

xaoi_entity* aoi_entity_create(void *scene_obj, const xvec3f_t &radius, unsigned group, unsigned group_filter)
{
	return new xaoi_object(scene_obj,radius,group,group_filter);
}

void aoi_entity_destroy(xaoi_entity *en)
{
	aoi_entity_leave(en);
	delete en;
}

void aoi_entity_join (xaoi_manager *mgr, xaoi_entity *en, const xvec3f_t &pos)
{
	mgr->add(en,pos);
}

void aoi_entity_leave (xaoi_entity *en)
{
	xaoi_manager *mgr = en->get_manager();
	if(mgr) mgr->remove(en);
}

void aoi_entity_move(xaoi_entity *en, const xvec3f_t &pos)
{
	en->move_to(pos);
}
