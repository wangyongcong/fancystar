#ifndef __HEADER_WYC_COMPONENT
#define __HEADER_WYC_COMPONENT

#include "wyc/obj/object.h"

namespace wyc
{

class xgameobj;

class xcomponent : public xobject
{
	USE_RTTI;
	xgameobj *m_parentGameobj;
	xcomponent *m_pNextCom;
	bool m_enabled;
	friend class xgameobj;
public:
	xcomponent() 
	{
		m_pNextCom=0;
	}
	xgameobj* gameobj() 
	{
		return m_parentGameobj;
	}
	inline void enable() 
	{
		m_enabled=true;
	}
	inline void disable()
	{
		m_enabled=false;
	}
	inline bool is_enabled() const
	{
		return m_enabled;
	}
	virtual void update(double accum_time, double frame_time) {accum_time, frame_time;}
protected:
	virtual void on_attach() {}
	virtual void on_detach() {}
};

} // namespace wyc

#endif // __HEADER_WYC_COMPONENT

