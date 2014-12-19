#ifndef __HEADER_WYC_XOBJECT2D
#define __HEADER_WYC_XOBJECT2D

#include "xrenderobj.h"
#include "xvecmath.h"
#include "xcamera.h"

namespace wyc
{

class xsystem2d;

class xobject2d : public xobject
{
	USE_EVENT_MAP;
protected:
	xobject2d *m_parent;
	typedef std::vector<xobject2d*> child_list_t;
	child_list_t m_children;
	xvec3f_t m_pos;
	xvec2f_t m_size;
	float m_rotate;
	uint32_t m_color[4];
	xvec2f_t m_kp;
	uint32_t m_pickCode;
	float m_alphaFilter;
	uint32_t *m_pFrameCode;
	enum {
		OBJ2D_BLIT_MASK=0xF,
		OBJ2D_TRANSFORM=0x10,
		OBJ2D_CHG_IMAGE=0x20,
		OBJ2D_CHG_KEYPOINT=0x40,
		OBJ2D_CHG_COLOR=0x80,
		OBJ2D_REBUILD_MESH=0xF0,
		OBJ2D_CHG_PARENT=0x100,
		OBJ2D_CHG_CUSTOM=0x800,
		OBJ2D_CHG_MASK=0xFF0,
		OBJ2D_PICK_MASK=0x3000,
		OBJ2D_PICK_SHIFT=12,
		OBJ2D_CHG_PICK=0x4000,
		OBJ2D_CHG_VISIBLE=0x8000,
		OBJ2D_HIDE=0x10000,
		OBJ2D_EDIT_MODE=0x20000,
		OBJ2D_FRAME_ENABLE=0x40000,
	};
	unsigned m_flag;
	std::string m_imgName;
public:
	xobject2d();
	virtual void on_destroy();
	virtual void create_proxy();
	void remove_proxy();
	// parent
	void add_child(xobject2d *pobj);
	void remove_child(xobject2d *pobj);
	xobject2d* parent();
	// position	
	void set_pos(float x, float y, float z=0);
	void set_pos(const xvec3f_t &v);
	const xvec3f_t& get_pos() const;
	void get_world_pos(xvec2f_t &pos) const;
	// size
	void set_size(float w, float h);
	void set_size(const xvec2f_t &v);
	const xvec2f_t& get_size() const;
	float width() const;
	float height() const;
	// rotate
	void rotate(float angle);
	float get_rotate() const;
	// key point
	void set_keypoint(float x, float y);
	void set_keypoint(const xvec2f_t &v);
	const xvec2f_t& keypoint() const;
	// image
	void set_image(const std::string& imgName, BLIT_TYPE blit=NO_BLIT);
	const std::string& get_image() const;
	// color
	void set_color(uint32_t lb, uint32_t rb, uint32_t rt, uint32_t lt);
	void set_color(uint32_t c);
	const uint32_t* get_color() const;
	// pick code
	void set_pick(bool enable, PICK_TYPE pick=NO_PICK, float filter=0);
	bool pickable() const;
	PICK_TYPE pick_type() const;
	uint32_t pick_code() const;
	// visible
	bool visible() const;
	void show();
	void hide();
	// set dirty bits
	void set_dirty(unsigned bit=OBJ2D_CHG_CUSTOM);
	// input message
	bool send_mouse_event(xobject2d *receiver, int evid, int x, int y, unsigned button);
	virtual bool on_mouse_event(xobject2d *receiver, int evid, int x, int y, unsigned button);
	void set_capture(bool b);
	// editor support
	virtual void on_edit_mode(bool enter) {enter;}
	bool edit_mode() const;
	void set_edit_mode(bool b);
	// object frame
	void show_frame();
	void hide_frame();
	void enable_frame();
	void disable_frame();
	bool frame_enabled() const;
	void highlight_frame(int frame_id, bool b);
	int get_frame(uint32_t code) const;
	// events id
	enum EVENT_ID
	{
		EV_SET_PARENT = 0x46D81F91,
		EV_TRANSFORM = 0x4170D01,
		EV_SET_COLOR = 0x81023823,
		EV_SET_IMAGE = 0x22697495,
		EV_SET_KEYPOINT = 0xB386A79C,
		EV_PREPARE_DRAW = 0xB2E41D8,
		EV_SET_PICK = 0x7590C295,
		EV_SET_VISIBLE = 0x1BA041F5,
		EV_SHOW_FRAME = 0x685A125E,
		EV_HIDE_FRAME = 0xD44EDB3,
		EV_ENABLE_FRAME = 0xB4162536,
		EV_DISABLE_FRAME = 0x7A93D97A,
		EV_SET_FRAME_COLOR = 0x4ED7DE6,
	};
	// static functions
	static unsigned ms_renderPassID;
	static unsigned ms_pickerPassID;
protected:
	void update_children(double accum_time, double frame_time);
	void sync_data();
	void notify_renderer(int evid, xobjevent *pev);
};

//------------------------------------------------------------

inline xobject2d* xobject2d::parent()
{
	return m_parent;
}

inline void xobject2d::set_pos(float x, float y, float z) {
	m_pos.x=x;
	m_pos.y=y;
	m_pos.z=z;
	m_flag|=OBJ2D_TRANSFORM;
}

inline void xobject2d::set_pos(const xvec3f_t &pos) {
	m_pos=pos;
	m_flag|=OBJ2D_TRANSFORM;
}

inline const xvec3f_t& xobject2d::get_pos() const {
	return m_pos;
}

inline void xobject2d::set_size(float w, float h) {
	m_size.set(w,h);
	m_flag|=OBJ2D_TRANSFORM;
}

inline void xobject2d::set_size(const xvec2f_t &v) {
	m_size=v;
	m_flag|=OBJ2D_TRANSFORM;
}

inline const xvec2f_t& xobject2d::get_size() const {
	return m_size;
}

inline float xobject2d::width() const {
	return m_size.x;
}

inline float xobject2d::height() const {
	return m_size.y;
}

inline void xobject2d::rotate(float angle) {
	m_rotate=angle;
	m_flag|=OBJ2D_TRANSFORM;
}

inline float xobject2d::get_rotate() const {
	return m_rotate;
}

inline void xobject2d::set_image(const std::string& imgName, BLIT_TYPE blitType)
{
	m_imgName=imgName;
	m_flag=(m_flag&~OBJ2D_BLIT_MASK)+blitType;
	m_flag|=OBJ2D_CHG_IMAGE;
}

inline void xobject2d::set_keypoint(float x, float y) {
	m_kp.set(x,y);
	m_flag|=OBJ2D_CHG_KEYPOINT;
}

inline void xobject2d::set_keypoint(const xvec2f_t &v) {
	m_kp=v;
	m_flag|=OBJ2D_CHG_KEYPOINT;
}

inline const xvec2f_t& xobject2d::keypoint() const {
	return m_kp;
}

inline void xobject2d::set_color(uint32_t lb, uint32_t rb, uint32_t rt, uint32_t lt) {
	m_color[0]=lb;
	m_color[1]=rb;
	m_color[2]=rt;
	m_color[3]=lt;
	m_flag|=OBJ2D_CHG_COLOR;
}

inline void xobject2d::set_color(uint32_t c) {
	m_color[0]=c;
	m_color[1]=c;
	m_color[2]=c;
	m_color[3]=c;
	m_flag|=OBJ2D_CHG_COLOR;
}

inline const uint32_t* xobject2d::get_color() const {
	return m_color;
}

inline bool xobject2d::pickable() const
{
	return 0!=(m_flag&OBJ2D_PICK_MASK);
}

inline PICK_TYPE xobject2d::pick_type() const
{
	return PICK_TYPE((m_flag&OBJ2D_PICK_MASK)>>OBJ2D_PICK_SHIFT);
}

inline uint32_t xobject2d::pick_code() const
{
	return m_pickCode;
}

inline void xobject2d::set_dirty(unsigned bit)
{
	add_state(m_flag,(OBJ2D_CHG_MASK&bit));
}

inline bool xobject2d::visible() const
{
	return 0==(m_flag&OBJ2D_HIDE);
}

inline void xobject2d::show()
{
	if(0==(m_flag&OBJ2D_HIDE))
		return;
	m_flag&=~OBJ2D_HIDE;
	m_flag|=OBJ2D_CHG_VISIBLE;
}

inline void xobject2d::hide()
{
	if(m_flag&OBJ2D_HIDE)
		return;
	m_flag|=OBJ2D_HIDE|OBJ2D_CHG_VISIBLE;
}

inline bool xobject2d::edit_mode() const
{
	return have_state(m_flag,OBJ2D_EDIT_MODE);
}

inline void xobject2d::set_edit_mode(bool b)
{
	if(b)
		add_state(m_flag,OBJ2D_EDIT_MODE);
	else
		remove_state(m_flag,OBJ2D_EDIT_MODE);
}

inline void xobject2d::show_frame()
{
	xrenderer::get_renderer()->send_event(xrenderer::EV_NOTIFY_PROXY,xpackev::pack("dde",id(),EV_SHOW_FRAME,0));
	add_state(m_flag,OBJ2D_CHG_CUSTOM);
}

inline void xobject2d::hide_frame()
{
	xrenderer::get_renderer()->send_event(xrenderer::EV_NOTIFY_PROXY,xpackev::pack("dde",id(),EV_HIDE_FRAME,0));	
	add_state(m_flag,OBJ2D_CHG_CUSTOM);
}

inline bool xobject2d::frame_enabled() const
{
	return have_state(m_flag,OBJ2D_FRAME_ENABLE);
}

inline void xobject2d::notify_renderer(int evid, xobjevent *pev)
{
	xrenderer::get_renderer()->send_event(xrenderer::EV_NOTIFY_PROXY,xpackev::pack("dde",id(),evid,pev));
}

};// namespace wyc

#endif // __HEADER_WYC_XOBJECT2D
