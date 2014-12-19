#include "aoi3d.h"
#include <cstring>
#include <cstdlib>
#include <cstdio>

namespace wyc
{

inline void init_anchor (xaoi_anchor *anchor, xaoi_entity *parent, float pos)
{
	anchor->pos=pos;
	anchor->parent=parent;
	anchor->next=0;
	anchor->prev=0;
#ifdef _DEBUG
	anchor->is_passed=false;
#endif
}

xaoi_entity xaoi_entity::ms_dummy_entity;

xaoi_entity::xaoi_entity()
{
	m_mgr=0;
	m_center.zero();
	m_radius.zero();
	m_local_next=0;
	memset(m_anchors,0,sizeof(m_anchors));
	m_mask=0;
	m_filter=0;
	m_recursion=0;
}

xaoi_entity::xaoi_entity(const xvec3f_t &radius)
{
	m_mgr=0;
	m_center.zero();
	m_radius=radius;
	m_local_next=0;
	m_mask=0;
	m_filter=0;
	m_recursion=0;
	
	if(radius.x<0 || fequal(radius.x,0))
		m_radius.x=0;
	if(radius.y<0 || fequal(radius.y,0))
		m_radius.y=0;
	if(radius.z<0 || fequal(radius.z,0))
		m_radius.z=0;

	unsigned anchor_count = 3;
	float zero=0;
	for(unsigned d=0; d<3; ++d)
	{
		if(m_radius.elem[d]>0) 
			anchor_count+=1;
	}
	xaoi_anchor *anchors = alloc_anchors(anchor_count), *iter;
	unsigned dst=0;
	iter=anchors;
	for(unsigned d=0; d<3; ++d)
	{
		if(m_radius.elem[d]>0)
		{
			init_anchor(iter,this,-m_radius.elem[d]);
			m_anchors[dst++]=iter++;
			init_anchor(iter,this,m_radius.elem[d]);
			m_anchors[dst++]=iter++;
		}
		else 
		{
			init_anchor(iter,this,0);
			m_anchors[dst++]=iter;
			m_anchors[dst++]=iter++;
		}
	}
	assert(6==dst);
	assert(iter==anchors+anchor_count);
	assert(m_anchors[0]==anchors);
}

xaoi_entity::~xaoi_entity()
{
	if(m_mgr)
		m_mgr->remove(this,false);
	free_anchors(m_anchors[0]);
}

xaoi_anchor* xaoi_entity::alloc_anchors (unsigned count)
{
	xaoi_anchor *anchors = (xaoi_anchor*) malloc(sizeof(xaoi_anchor)*count);
	return anchors;
}

void xaoi_entity::free_anchors (xaoi_anchor *anchors)
{
	::free(anchors);
}

inline bool is_overlap (const xvec3f_t &lower1, const xvec3f_t &upper1, const xvec3f_t &lower2, const xvec3f_t &upper2)
{
	return !(lower1.x>upper2.x || upper1.x<lower2.x \
		|| lower1.y>upper2.y || upper1.y<lower2.y \
		|| lower1.z>upper2.z || upper1.z<lower2.z);
}

void xaoi_entity::move_to (const xvec3f_t &pos)
{
	if(!m_mgr)
	{
		m_center = pos;
		return;
	}
	if(m_recursion)
	{
		fprintf(stderr,"xaoi_entity::move_to: recursive call is not allowed !");
		return;
	}
	++m_recursion;
	xaoi_anchor *iter, *next, **anchor_head;
	xaoi_entity *head=&ms_dummy_entity;
	assert(0==head->m_local_next);
	float old_pos;
	// find out all potential overlaps
	for(unsigned d=0, i=0; d<3; d+=1, i+=2)
	{
		if(fequal(pos.elem[d],m_center.elem[d]))
			continue;
		iter = m_anchors[i];
		next = m_anchors[i+1];
		old_pos = iter->pos;
		iter->pos = pos.elem[d]-m_radius.elem[d];
		anchor_head = m_mgr->_get_head(d);
		head = _step(anchor_head,iter,old_pos,head);
		if(next!=iter) {
			old_pos = next->pos;
			next->pos = pos.elem[d]+m_radius.elem[d];
			head=_step(anchor_head,next,old_pos,head);
			assert(next->prev);
		}
	}
	// update position
	xvec3f_t old_center = m_center;
	m_center = pos;
	if(head==&ms_dummy_entity)
	{
		--m_recursion;
		return;
	}
	// handle enter/leave events
	xaoi_entity *entity;
	xvec3f_t my_lower = m_center-m_radius, my_upper = m_center+m_radius;
	xvec3f_t my_old_lower = old_center-m_radius, my_old_upper = old_center+m_radius;
	xvec3f_t lower, upper;
	bool was_in_range, is_in_range;
	for(entity=head; entity!=&ms_dummy_entity; entity=head)
	{
		assert(entity);
		lower=entity->m_center-entity->m_radius;
		upper=entity->m_center+entity->m_radius;
		was_in_range=is_overlap(my_old_lower,my_old_upper,lower,upper);
		is_in_range=is_overlap(my_lower,my_upper,lower,upper);
		if(was_in_range!=is_in_range)
		{
			if(is_in_range) {
				if(this->m_filter&entity->m_mask)
					this->on_enter(entity);
				if(entity->m_filter&this->m_mask)
					entity->on_enter(this);
			}
			else {
				if(this->m_filter&entity->m_mask)
					this->on_leave(entity);
				if(entity->m_filter&this->m_mask)
					entity->on_leave(this);
			}
		}
		head = entity->m_local_next;
		entity->m_local_next=0;
	}
	--m_recursion;
}

xaoi_entity* xaoi_entity::_step(xaoi_anchor **anchor_head, xaoi_anchor *anchor, float old_pos, xaoi_entity *head)
{
	assert(anchor_head);
	assert(anchor);
	assert(head);
	xaoi_anchor *iter;
	// step to left
	for(iter=anchor->prev; iter && (iter->pos>=anchor->pos || iter->pos==old_pos); )
	{
		if(iter->parent!=anchor->parent && 0==iter->parent->m_local_next)
		{
			if(iter->parent->m_filter&anchor->parent->m_mask || anchor->parent->m_filter&iter->parent->m_mask)
			{
				iter->parent->m_local_next=head;
				head=iter->parent;
			}
		}
		if(iter->pos > anchor->pos) // swap
		{
			iter->next=anchor->next;
			if(anchor->next)
				anchor->next->prev=iter;
			anchor->next=iter;
			anchor->prev=iter->prev;
			if(iter->prev)
				iter->prev->next=anchor;
			else // it's head
				*anchor_head=anchor;
			iter->prev=anchor;
		
			iter=anchor->prev;
		}
		else iter = iter->prev;
	}
	// step to right
	for(iter=anchor->next; iter && (iter->pos<=anchor->pos || iter->pos==old_pos); )
	{
		if(iter->parent!=anchor->parent && 0==iter->parent->m_local_next)
		{
			if(iter->parent->m_filter&anchor->parent->m_mask || anchor->parent->m_filter&iter->parent->m_mask)
			{
				iter->parent->m_local_next=head;
				head=iter->parent;
			}
		}
		if(iter->pos < anchor->pos) // swap
		{
			anchor->next=iter->next;
			if(iter->next)
				iter->next->prev=anchor;
			iter->next=anchor;
			if(anchor->prev)
				anchor->prev->next=iter;
			else  // it's head
				*anchor_head=iter;
			iter->prev=anchor->prev;
			anchor->prev=iter;

			iter=anchor->next;
		}
		else iter = iter->next;
	}
	return head;
}

void xaoi_entity::_detect_overlap(xaoi_entity *self, notify_t functor)
{
	assert(self);
	xaoi_anchor *iter;
	xaoi_entity *head=&ms_dummy_entity;
	assert(0==head->m_local_next);
	// find out all potential overlaps
	for(iter = self->m_anchors[1]->prev; iter; iter=iter->prev)
	{
		if(iter->parent!=self && 0==iter->parent->m_local_next)
		{
			if(iter->parent->m_filter&self->m_mask || self->m_filter&iter->parent->m_mask)
			{
				iter->parent->m_local_next=head;
				head=iter->parent;
			}
		}
	}
	for(iter = self->m_anchors[1]->next; iter && iter->pos==self->m_anchors[1]->pos; iter=iter->next)
	{
		if(iter->parent!=self && 0==iter->parent->m_local_next)
		{
			if(iter->parent->m_filter&self->m_mask || self->m_filter&iter->parent->m_mask)
			{
				iter->parent->m_local_next=head;
				head=iter->parent;
			}
		}
	}
	// handle enter/leave events
	xaoi_entity *entity;
	xvec3f_t my_lower = self->m_center-self->m_radius, my_upper = self->m_center+self->m_radius;
	xvec3f_t lower, upper;
	bool is_in_range;
	for(entity=head; entity!=&ms_dummy_entity; entity=head)
	{
		assert(entity);
		lower=entity->m_center-entity->m_radius;
		upper=entity->m_center+entity->m_radius;
		is_in_range=is_overlap(my_lower,my_upper,lower,upper);
		if(is_in_range) {
			if(self->m_filter&entity->m_mask)
				(self->*functor)(entity);
			if(entity->m_filter&self->m_mask)
				(entity->*functor)(self);
		}
		head = entity->m_local_next;
		entity->m_local_next=0;
	}
}

//------------------------------------------------------------------------------------------

xaoi_manager::xaoi_manager()
{
	memset(m_heads,0,sizeof(m_heads));
}

xaoi_manager::~xaoi_manager()
{
	if(m_heads[0])
		clear();
}

void xaoi_manager::clear()
{
	xaoi_anchor *iter = m_heads[0];
	while(iter)
	{
		iter->parent->m_mgr=0;
		iter=iter->next;
	}
	memset(m_heads,0,sizeof(m_heads));
}

void xaoi_manager::add (xaoi_entity *en, const xvec3f_t &pos)
{
	if(en->m_mgr)
	{
		if(this==en->m_mgr)
			return;
		en->m_mgr->remove(en);
	}
	en->m_mgr=this;
	xaoi_anchor *iter, *next;
	en->m_center=pos;
	for(unsigned d=0, i=0; d<3; ++d, i+=2)
	{
		iter = en->m_anchors[i];
		next = en->m_anchors[i+1];
		iter->pos = en->m_center.elem[d]-en->m_radius.elem[d];
		_insert_anchor(m_heads+d,iter,0);
		if(next!=iter) {
			next->pos = en->m_center.elem[d]+en->m_radius.elem[d];
			assert(next->pos>iter->pos);
			_insert_anchor(&iter->next,next,iter);
		}
	}
	xaoi_entity::_detect_overlap(en,&xaoi_entity::on_enter);
}

void xaoi_manager::remove (xaoi_entity *en, bool need_notify)
{
	if(this!=en->m_mgr)
		return;
	if(need_notify)
		xaoi_entity::_detect_overlap(en,&xaoi_entity::on_leave);
	en->m_mgr=0;
	xaoi_anchor *iter, *prev=0;
	for(int i=0; i<6; ++i)
	{
		iter=en->m_anchors[i];
		assert(iter);
		if(iter!=prev)
		{
			if(iter->prev)
				iter->prev->next=iter->next;
			else { // it's head
				assert(m_heads[i>>1]==iter);
				m_heads[i>>1]=iter->next;
			}
			if(iter->next)
				iter->next->prev=iter->prev;
			iter->prev=0;
			iter->next=0;
			prev=iter;
		}
	}
}

xaoi_anchor* xaoi_manager::_insert_anchor (xaoi_anchor **next_ptr, xaoi_anchor *anchor, xaoi_anchor *prev)
{
	assert(next_ptr);
	assert(anchor);

	xaoi_anchor *iter;
	for(iter=*next_ptr; iter && iter->pos<=anchor->pos; next_ptr=&iter->next, prev=iter, iter=*next_ptr);

	anchor->next = iter;
	anchor->prev = prev;
	if(iter) 
		iter->prev = anchor;
	*next_ptr=anchor;

	return anchor;
}


//------------------------------------------------------------------------------------------
// DEBUG interfaces

#ifdef _DEBUG

void xaoi_entity::_verify()
{
	xaoi_anchor *iter, *next;
	for(int d=0, i=0; d<3; ++d, i+=2)
	{
		iter = m_anchors[i];
		next = m_anchors[i+1];
		assert(iter->is_passed);
		iter->is_passed=false;
		assert(iter->pos==float(m_center.elem[d]-m_radius.elem[d]));
		if(next!=iter)
		{
			assert(next->is_passed);
			assert(m_radius.elem[d]>0);
			assert(next->pos==float(m_center.elem[d]+m_radius.elem[d]));
			next->is_passed=false;
		}
	}
}

void xaoi_manager::_verify()
{
	xaoi_entity *childs = &xaoi_entity::ms_dummy_entity;
	// check linked list
	xaoi_anchor *head, *iter, *prev;
	for(int i=0; i<3; ++i)
	{
		head = m_heads[i];
		for(iter=head, prev=0; iter; iter=iter->next)
		{
			assert(iter->prev==prev);
			assert(!iter->is_passed);
			if(prev)
				assert(prev->pos<=iter->pos);
			prev=iter;
			iter->is_passed=true;

			if(!iter->parent->m_local_next)
			{
				iter->parent->m_local_next=childs;
				childs=iter->parent;
			}
		}
	}	
	for(xaoi_entity *entity=childs, *end=&xaoi_entity::ms_dummy_entity; entity != end; entity=childs)
	{
		entity->_verify();
		childs=entity->m_local_next;
		entity->m_local_next=0;
	}
}

#endif // _DEBUG

} // namespace wyc

