#ifndef __HEADER_WYC_SYSTEM2D
#define __HEADER_WYC_SYSTEM2D

#include "share_buffer.h"
#include "editor2d.h"

namespace wyc
{

class xsystem2d : public xobject
{
	USE_EVENT_MAP;
	static xthread_local ms_tlsSystem2D;
	static xsystem2d *ms_pSystem2d;
	// object2d picker 
	xdict m_pickmap;
	std::vector<uint32_t> m_codepool;
	uint32_t m_code;
	// TODO:  π”√share buffer
	uint32_t *m_pickbuff;
	unsigned m_buffw, m_buffh;
	uint32_t m_prevCode;
	uint32_t m_capture;
	// object2d editor
	xpointer<xcamera> m_spCamera;
	xpointer<xeditor2d> m_spEditor;
	bool m_editMode;
public:
	static void init_system2d(unsigned viewport_w, unsigned viewport_h, xcamera *pcam);
	static void clear_system2d();
	static xsystem2d* get_system2d();

	xsystem2d();
	virtual void on_destroy();
	void initialize(unsigned viewport_w, unsigned viewport_h, xcamera *pcam);
	virtual void update(double accum_time, double frame_time);
	xobject2d* get_object_at(int x, int y, uint32_t &code);

	bool dispatch_mouse_event(int evid, int x, int y, unsigned button);
	bool on_mouse_event(xobject2d *receiver, int evid, int x, int y, unsigned button);
	
	void alloc_pick_code(unsigned count, uint32_t *pc);
	void free_pick_code(unsigned count, uint32_t *pc);
	void add_mouse_handler(uint32_t code, xobject2d *pobj);
	void remove_mouse_handler(uint32_t code);

	void set_capture(uint32_t pc, bool b);
	inline void set_camera(xcamera *pcam) {
		m_spCamera=pcam;
	}
	void open_editor();
	void close_editor();

	//-- debug interface -----------------------------
	void debug_add_object(const char *pev);
protected:
	bool on_mouse_move(xobject2d *receiver, int x, int y, unsigned button);
	void edit_on_mouse_move(xobject2d *receiver, uint32_t code, int x, int y);
	bool edit_on_mouse_button(xobject2d *receiver, uint32_t code, int evid, int x, int y, unsigned button);

};

}; // namespace wyc

#endif // __HEADER_WYC_SYSTEM2D
