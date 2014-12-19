#ifndef __HEADER_WYC_AOI_MGR
#define __HEADER_WYC_AOI_MGR

#include "aoi3d.h"

#include "wyc/math/vecmath.h"

using namespace wyc;

class xaoi_object : public xaoi_entity
{
	void *m_scene_obj;
public:
	xaoi_object(void *scene_obj, const xvec3f_t &radius, unsigned group, unsigned group_filter);
protected:
	virtual void on_enter(xaoi_entity *entity);
	virtual void on_leave(xaoi_entity *entity);
};

xaoi_manager* aoi_manager_create(int dimension);

void aoi_manager_destroy(xaoi_manager *mgr);

xaoi_entity* aoi_entity_create(void *scene_obj, const xvec3f_t &radius, unsigned group, unsigned group_filter);

void aoi_entity_destroy(xaoi_entity *en);

void aoi_entity_join (xaoi_manager *mgr, xaoi_entity *en, const xvec3f_t &pos);

void aoi_entity_leave(xaoi_entity *en);

void aoi_entity_move(xaoi_entity *en, const xvec3f_t &pos);


#endif // __HEADER_WYC_AOI_MGR


