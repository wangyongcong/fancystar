#ifndef __HEADER_WYC_SPACE_TREE
#define __HEADER_WYC_SPACE_TREE

#include <vector>

#ifdef USE_LIBWYC
#include "wyc/util/util.h"
#include "wyc/math/vecmath.h"
#else
#include "vecmath.h"
#endif

namespace wyc
{

class xquad_tree
{
public:
	struct linked_node_t 
	{
		linked_node_t *prev, *next, *local_next;
	};
	struct entity_t : public linked_node_t
	{
		union {
			struct {
				xvec2f_t lower, upper;
			};
			float verts[4]; // (lower.x, lower.y, upper.x, upper.y)
		};
		unsigned mask;
		xquad_tree *parent;
		unsigned grid_id;
		void *data;
		entity_t()
		{
			prev=next=local_next=0;
			parent=0;
			grid_id=0;
			mask=0;
			data=0;
		}
	};
	friend struct xquad_tree::entity_t;

	xquad_tree();
	~xquad_tree();
	bool save(const char* filename) const;
	bool load(const char* filename);
	bool initialize(unsigned grid_w, unsigned grid_h, unsigned row, unsigned col);
	void add_entity(entity_t *en);
	void del_entity(entity_t *en);
	void update_entity (entity_t *en);
	void clear();
	template<typename T>
	void find_neighbors (const xvec2f_t &position, unsigned filter, T &handler);
	template<typename T>
	void find_neighbors (const xvec2f_t &lower, const xvec2f_t &upper, unsigned filter, T &handler);

	unsigned width() const;
	unsigned height() const;
	void set_translate(const xvec2f_t &trans);
	const xvec2f_t& get_translate() const;
	unsigned lod_count() const;
	void get_lod_info(unsigned lod, unsigned &row, unsigned &col, unsigned &grid_w, unsigned &grid_h) const;
	void get_filled_grids(unsigned lod, std::vector<std::pair<unsigned,unsigned> > &grid_coords) const;

//--------------------------------------------------------------
#ifdef _DEBUG
	void debug_verify();
	void debug_compare(const xquad_tree *other);
#endif // _DEBUG
private:
	struct lod_info_t;
	struct grid_t
	{
		lod_info_t *lod;
		grid_t *parent;
		grid_t *childs[4];
		linked_node_t sentinel;
	};
	struct lod_info_t
	{
		unsigned short grid_w, grid_h;
		unsigned short row, col;
		unsigned short child_count;
		unsigned short grid_count;
		unsigned entity_count;
		grid_t *grids;
	};
	unsigned m_width, m_height;
	xvec2f_t m_translate;
	unsigned m_level_mask;
	std::vector<lod_info_t> m_lod_info;
	std::vector<grid_t> m_grids;
	static linked_node_t *ms_dummy_entity;

	// assignment is not allowed
	xquad_tree(const xquad_tree &qtree);
	xquad_tree& operator = (const xquad_tree &qtree);

	grid_t* _lod_search (const xvec2f_t &center, const xvec2f_t &size);
	void _find_neighbors(const xvec2f_t &lower, const xvec2f_t &upper, unsigned filter, linked_node_t **en_head);
	void _find_neighbors (const xvec2f_t &beg, const xvec2f_t &end, const xvec2f_t &radius, unsigned filter, linked_node_t **en_head);
};

inline unsigned xquad_tree::width() const
{
	return m_width;
}

inline unsigned xquad_tree::height() const
{
	return m_height;
}

inline void xquad_tree::set_translate(const xvec2f_t &trans)
{
	m_translate = trans;
}

inline const xvec2f_t& xquad_tree::get_translate() const
{
	return m_translate;
}

inline unsigned xquad_tree::lod_count() const
{
	return m_lod_info.size();
}

inline void xquad_tree::get_lod_info(unsigned lod, unsigned &row, unsigned &col, unsigned &grid_w, unsigned &grid_h) const
{
	if(lod<m_lod_info.size()) {
		const lod_info_t &info = m_lod_info[lod];
		row=info.row;
		col=info.col;
		grid_w=info.grid_w;
		grid_h=info.grid_h;
	}
}

template<typename T>
void xquad_tree::find_neighbors (const xvec2f_t &position, unsigned filter, T &handler)
{
	linked_node_t *en_head=ms_dummy_entity;
	_find_neighbors(position,position,filter,&en_head);
	for(entity_t *en=(entity_t*)en_head; en!=ms_dummy_entity; en=(entity_t*)en_head)
	{
		en_head=en->local_next;
		en->local_next=0;
		handler((entity_t*)en);
	}
}

template<typename T>
void xquad_tree::find_neighbors (const xvec2f_t &lower, const xvec2f_t &upper, unsigned filter, T &handler)
{
	linked_node_t *en_head=ms_dummy_entity;
	_find_neighbors(lower,upper,filter,&en_head);
	for(entity_t *en=(entity_t*)en_head; en!=ms_dummy_entity; en=(entity_t*)en_head)
	{
		en_head=en->local_next;
		en->local_next=0;
		handler((entity_t*)en);
	}
}

}; // namespace wyc

#endif // __HEADER_WYC_SPACE_TREE
