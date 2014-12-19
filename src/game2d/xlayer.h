#ifndef __HEADER_WYC_LAYER
#define __HEADER_WYC_LAYER

#include "renderer.h"
#include "xobject2d.h"
#include "xcamera.h"

namespace wyc
{

class xlayer : public xobject2d
{
	USE_EVENT_MAP;
public:
	xlayer();
	virtual void on_destroy();
	virtual void update(double accum_time, double frame_time);
	virtual void create_proxy();
};

}; // namespace wyc

#endif // __HEADER_WYC_LAYER

