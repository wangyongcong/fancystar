#ifndef __HEADER_WYC_EDITOR2D
#define __HEADER_WYC_EDITOR2D

#include "xlayer.h"

namespace wyc
{

class xeditor2d : public xlayer
{
	USE_EVENT_MAP;
	unsigned m_frameObj;
	std::vector<xobject2d*> m_selectedObjs;
	xpointer<xobject2d> m_spCurSelObj;
	int m_pickx, m_picky;
	int m_resizeMode;
	bool m_dragMode;
	bool m_rotMode;
public:
	xeditor2d();
	virtual void on_destroy();
	virtual void update(double accum_time, double frame_time);
	virtual void create_proxy();
	void init_editor();
	void select(xobject2d *pobj, bool multiple=false);
	void unselect();
	bool is_selected(xobject2d *pobj) const;
	void begin_drag(xobject2d *pobj, uint32_t code, int x, int y);
	void end_drag();
	bool is_drag_mode() const;
	void begin_rotate(int x, int y);
	void end_rotate();
	void on_mouse_move(int x, int y);
	enum EVENT_ID
	{
	};
private:
};

inline bool xeditor2d::is_drag_mode() const {
	return m_dragMode;
}

}; // namespace wyc

#endif // __HEADER_WYC_EDITOR2D

