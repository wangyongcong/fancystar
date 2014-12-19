#ifndef __HEADER_WYC_AOI3D
#define __HEADER_WYC_AOI3D

#include "wyc/math/vecmath.h"

namespace wyc
{

class xaoi_entity;

class xaoi_manager;

struct xaoi_anchor
{
	float pos;
	xaoi_entity *parent;
	xaoi_anchor *next;
	xaoi_anchor *prev;

#ifdef _DEBUG
	bool is_passed;
#endif // _DEBUG
};

class xaoi_entity
{
	friend class xaoi_manager;
	xaoi_manager *m_mgr;
	xvec3f_t m_center;
	xvec3f_t m_radius;
	xaoi_anchor *m_anchors[6];	
	xaoi_entity *m_local_next;
	unsigned m_mask;
	unsigned m_filter;
	unsigned char m_recursion;
public:
	xaoi_entity(const xvec3f_t &radius);
	virtual ~xaoi_entity();
	void move_to (const xvec3f_t &pos);
	virtual void on_enter(xaoi_entity *entity) {}
	virtual void on_leave(xaoi_entity *entity) {}
	void set_mask(unsigned mask);
	unsigned get_mask() const;
	void set_filter(unsigned filter);
	unsigned get_filter() const;
	const xvec3f_t& get_pos() const;
	const xvec3f_t& get_radius() const;
	xaoi_manager* get_manager();
private:
	xaoi_entity();
	static xaoi_entity ms_dummy_entity;
	static xaoi_anchor* alloc_anchors (unsigned count);
	static void free_anchors (xaoi_anchor *anchors);
	static xaoi_entity* _step(xaoi_anchor **anchor_head, xaoi_anchor *anchor, float old_pos, xaoi_entity *head);
	typedef void (xaoi_entity::*notify_t) (xaoi_entity*);
	static void _detect_overlap(xaoi_entity *en, notify_t functor);

//--------------------------------------------------------------------
// internal interface, do not use it!
//--------------------------------------------------------------------
#ifdef _DEBUG
	void _verify();
#endif // _DEBUG
};

inline void xaoi_entity::set_mask(unsigned mask)
{
	m_mask=mask;
}

inline unsigned xaoi_entity::get_mask() const
{
	return m_mask;
}

inline void xaoi_entity::set_filter(unsigned filter)
{
	m_filter=filter;
}

inline unsigned xaoi_entity::get_filter() const
{
	return m_filter;
}

inline const xvec3f_t& xaoi_entity::get_pos() const
{
	return m_center;
}

inline const xvec3f_t& xaoi_entity::get_radius() const
{
	return m_radius;
}

inline xaoi_manager* xaoi_entity::get_manager()
{
	return m_mgr;
}

class xaoi_manager
{
	xaoi_anchor *m_heads[3];
public:
	xaoi_manager();
	~xaoi_manager();
	void add (xaoi_entity *en, const xvec3f_t &pos);
	void remove (xaoi_entity *en, bool need_notify=true);
	void clear();
private:
	static xaoi_anchor* _insert_anchor (xaoi_anchor **next_ptr, xaoi_anchor *anchor, xaoi_anchor *prev=0);

//--------------------------------------------------------------------
// internal interface, do not use it!
//--------------------------------------------------------------------
public:
	xaoi_anchor** _get_head(int d);
#ifdef _DEBUG
	void _verify();
#endif // _DEBUG
};

inline xaoi_anchor** xaoi_manager::_get_head(int d)
{
	assert(d<3);
	return m_heads+d;
}


} // namespace wyc

#endif // __HEADER_WYC_AOI3D

