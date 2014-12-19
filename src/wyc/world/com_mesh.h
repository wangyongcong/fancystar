#ifndef __HEADER_WYC_COM_MESH
#define __HEADER_WYC_COM_MESH

#include "wyc/math/vecmath.h"
#include "wyc/world/component.h"
#include "wyc/render/vertexbatch.h"

namespace wyc
{

class xcom_mesh : public xcomponent
{
	USE_RTTI;
	xpointer<xvertex_batch> m_batch;
public:
	xcom_mesh();
	virtual void on_destroy();
	bool visible() const 
	{
		return m_batch && m_batch->is_complete();
	}
	xvertex_batch* get_mesh() 
	{
		return m_batch;
	}
	void load_mesh(const char *name);
private:
	static void on_load_mesh_ok(xrefobj *notifier, xresbase *res);
};

}; // namespace wyc

#endif // __HEADER_WYC_COM_MESH


