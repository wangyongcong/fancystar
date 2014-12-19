#include <cassert>
#include <cstring>
#include <fstream>

//#define PYTHON_DEBUG
#ifdef PYTHON_DEBUG
	#include <Python.h>
	#define py_print(fmt,...)	{printf(fmt,__VA_ARGS__);}
#else
	#define py_print(fmt,...)
#endif

#include "quad_tree.h"

namespace wyc
{

inline void clamp (int &v, int lower, int upper)
{
	if(v<lower)
		v=lower;
	else if(v>upper)
		v=upper;
}

#define MAX_LEVEL 32 // ×î´ó²ãÊý

xquad_tree::linked_node_t* xquad_tree::ms_dummy_entity = new xquad_tree::linked_node_t();

xquad_tree::xquad_tree()
{
	m_level_mask=0;
	m_width=m_height=0;
	m_translate.zero();
}

xquad_tree::~xquad_tree()
{
	clear();
}

bool xquad_tree::initialize(unsigned grid_w, unsigned grid_h, unsigned row, unsigned col)
{
	if(grid_w<=0 || grid_h<=0 || row<=0 || col<=0)
		return false;
	if((row & (row-1))!=0)
		row = power2(row);
	if((col & (col-1))!=0)
		col = power2(col);
	unsigned lod = log2p2(row>col?row:col)+1, child_count=0, total_grid_count=0;
	if(lod>MAX_LEVEL) {
		py_print("xquad_tree::initialize: LOD is clamp to %d",MAX_LEVEL);
		lod=MAX_LEVEL;
	}
	m_width = grid_w * col;
	m_height= grid_h * row;
	m_lod_info.resize(lod);
	lod_info_t *lod_iter=&m_lod_info[0], *prev_lod;
	for(unsigned i=0; i<lod; ++i, ++lod_iter)
	{
		lod_iter->row=row;
		lod_iter->col=col;
		lod_iter->grid_w=grid_w;
		lod_iter->grid_h=grid_h;
		lod_iter->grid_count=row*col;
		lod_iter->child_count=child_count;
		lod_iter->entity_count=0;
		total_grid_count+=lod_iter->grid_count;
		child_count=1;
		if(row>1) {
			row>>=1;
			child_count<<=1;
			grid_h<<=1;
		}
		if(col>1) {
			col>>=1;
			child_count<<=1;
			grid_w<<=1;
		}
	}
	m_grids.resize(total_grid_count);
	unsigned gidx=0;
	grid_t *grid, *grid_row;
	lod_iter=&m_lod_info[0];
	prev_lod=0;
	for(unsigned i=0; i<lod; ++i, prev_lod=lod_iter, ++lod_iter)
	{
		lod_iter->grids=&m_grids[gidx];
		gidx+=lod_iter->grid_count;
		grid=lod_iter->grids;
		for(unsigned j=0; j<lod_iter->grid_count; ++j, ++grid)
		{
			std::memset(grid,0,sizeof(grid_t));
			grid->lod=lod_iter;
		}
		if(0==lod_iter->child_count)
			continue;
		assert(prev_lod);
		grid=lod_iter->grids;
		grid_row = prev_lod->grids;
		if(4==lod_iter->child_count) {
			for(unsigned r=0; r<lod_iter->row; ++r, grid_row+=prev_lod->col)
			{
				for(unsigned c=0; c<lod_iter->col; ++c, ++grid, grid_row+=2)
				{
					grid->childs[0]=grid_row;
					grid->childs[1]=grid_row+1;
					grid->childs[2]=grid_row+prev_lod->col;
					grid->childs[3]=grid->childs[2]+1;
					grid->childs[0]->parent=grid->childs[1]->parent=grid->childs[2]->parent=grid->childs[3]->parent=grid;
				}
			}
		}
		else if(prev_lod->row==(lod_iter->row<<1))
		{
			assert(2==lod_iter->child_count);
			assert(1==lod_iter->col);
			assert(1==prev_lod->col);
			for(unsigned r=0; r<lod_iter->row; ++r, ++grid)
			{
				grid->childs[0]=grid_row++;
				grid->childs[1]=grid_row++;
				grid->childs[0]->parent=grid->childs[1]->parent=grid;
			}
		}
		else {
			assert(2==lod_iter->child_count);
			assert(1==lod_iter->row);
			assert(1==prev_lod->row);
			for(unsigned c=0; c<lod_iter->col; ++c, ++grid) {
				grid->childs[0]=grid_row++;
				grid->childs[1]=grid_row++;
				grid->childs[0]->parent=grid->childs[1]->parent=grid;
			}
		}
		assert(grid==lod_iter->grids+lod_iter->grid_count);
	}
	return true;
}

void xquad_tree::add_entity(entity_t *en)
{
	if(en->parent)
	{
		if(en->parent==this)
			return;
		en->parent->del_entity(en);
	}
	en->parent=this;
	xvec2f_t center=(en->lower+en->upper)*0.5f, size=en->upper-en->lower;
	grid_t *grid=_lod_search(center,size);
	en->grid_id=grid-&m_grids[0];
	en->next=grid->sentinel.next;
	if(en->next)
		en->next->prev=en;
	en->prev=&grid->sentinel;
	grid->sentinel.next=en;
	grid->lod->entity_count+=1;
	unsigned level = grid->lod-&m_lod_info[0];
	m_level_mask |= 1<<level;

	py_print("add entity: LOD %d, count %d, mask %d\n",level,grid->lod->entity_count,m_level_mask);
}

void xquad_tree::del_entity(entity_t *en)
{
	if(en->parent!=this)
		return;
	assert(en->prev);
	en->prev->next=en->next;
	if(en->next)
		en->next->prev=en->prev;
	en->parent=0;
	en->prev=0;
	en->next=0;
	lod_info_t *lod = m_grids[en->grid_id].lod;
	en->grid_id=0;
	lod->entity_count-=1;
	unsigned level = lod-&m_lod_info[0];
	if(0==lod->entity_count)
	{
		m_level_mask &= ~(1<<level);
	}
	py_print("del entity: LOD %d, count %d, mask %d\n",level,lod->entity_count,m_level_mask);
}

void xquad_tree::update_entity(entity_t *en)
{
	assert(en->parent==this);
	grid_t *old_grid = &m_grids[en->grid_id];
	lod_info_t *lod = old_grid->lod;
	xvec2f_t size;
	size.sub(en->upper,en->lower);
	// find the proper lod level
	int lod_idx= lod-&m_lod_info[0], lod_count=m_lod_info.size();
	if(size.x>lod->grid_w || size.y>lod->grid_h) { 
		// traverse upward
		for(lod_idx+=1; lod_idx<lod_count; ++lod_idx) {
			++lod;
			if(size.x<=lod->grid_w && size.y<=lod->grid_h)
				break;
		}
	}
	else { // traverse downward
		for(lod_idx-=1; lod_idx>=0; --lod_idx, --lod) {
			lod_info_t *prev_lod=lod-1;
			if(size.x>prev_lod->grid_w || size.y>prev_lod->grid_h)
				break;
		}
	}
	// find the proper grid
	xvec2f_t center = (en->lower+en->upper)*0.5f-m_translate;
	unsigned r, c;
	if(center.x<0)
		c=0;
	else {
		c=unsigned(center.x/lod->grid_w);
		if(c>=lod->col)
			c=lod->col-1;
	}
	if(center.y<0)
		r=0;
	else {
		r = unsigned(center.y/lod->grid_h);
		if(r>=lod->row)
			r=lod->row-1;
	}
	grid_t *new_grid = &lod->grids[r*lod->col+c];
	if(new_grid==old_grid) 
		return;
	// deleted from old grid
	en->prev->next=en->next;
	if(en->next)
		en->next->prev=en->prev;
	// put into new grid
	en->grid_id = new_grid-&m_grids[0];
	en->next=new_grid->sentinel.next;
	if(en->next)
		en->next->prev=en;
	en->prev=&new_grid->sentinel;
	new_grid->sentinel.next=en;
	// update lod info
	if(old_grid->lod != new_grid->lod) {
		unsigned level;
		lod = old_grid->lod;
		lod->entity_count-=1;
		if(0==lod->entity_count) {
			level = lod-&m_lod_info[0];
			m_level_mask &= ~(1<<level);
		}
		lod = new_grid->lod;
		lod->entity_count+=1;
		level = lod-&m_lod_info[0];
		m_level_mask |= 1<<level;
	}
}

void xquad_tree::clear()
{
	linked_node_t *next_node;
	entity_t *en;
	for(std::vector<grid_t>::iterator iter=m_grids.begin(), 
		end=m_grids.end(); iter!=end; ++iter) 
	{
		next_node=iter->sentinel.next;
		iter->sentinel.next=0;
		while(next_node) {
			en=(entity_t*)next_node;
			next_node=next_node->next;
			en->parent=0;
			en->grid_id=0;
			en->prev=en->next=0;
		}
	}
	for(std::vector<lod_info_t>::iterator iter=m_lod_info.begin(),
		end=m_lod_info.end(); iter!=end; ++iter)
	{
		iter->entity_count=0;
	}
	m_level_mask=0;
}

inline bool is_overlap(wyc::xquad_tree::entity_t *en1, wyc::xquad_tree::entity_t *en2)
{
	return !(en1->lower.x>en2->upper.x || en1->upper.x<en2->lower.x \
		|| en1->lower.y>en2->upper.y || en1->upper.y<en2->lower.y);
}

void xquad_tree::_find_neighbors(const xvec2f_t &lower, const xvec2f_t &upper, unsigned filter, linked_node_t **en_head)
{
	assert(m_lod_info.size());
	assert(lower.x<=upper.x && lower.y<=upper.y);
	lod_info_t *lod;
	int r1, c1, r2, c2;
	float delta_x, delta_y, inv_w, inv_h;
	unsigned level_mask = m_level_mask;
	xvec2f_t tran_lower, tran_upper;
	tran_lower.sub(lower,m_translate);
	tran_upper.sub(upper,m_translate);
	py_print("trasnlate (%f,%f), lower (%f,%f), upper (%f,%f)\n",m_translate.x, m_translate.y, tran_lower.x,tran_lower.y,tran_upper.x,tran_upper.y);
	grid_t *row_grid;
	for(unsigned i=0, max_lod=m_lod_info.size(); i<max_lod; ++i)
	{
		py_print("[%d] level mask %d\n",i,level_mask);
		if(!level_mask) 
			break;
		level_mask >>= 1;
		lod=&m_lod_info[i];
		py_print("LOD[%d]: entity %d, grid size (%d,%d)\n",i,lod->entity_count,lod->grid_w,lod->grid_h);
		if(!lod->entity_count)
			continue;
		delta_x = lod->grid_w*0.5f+EPSILON_E4, delta_y = lod->grid_h*0.5f+EPSILON_E4;
		inv_w = 1.0f/lod->grid_w, inv_h = 1.0f/lod->grid_h;
		r1 = int( std::floor((tran_lower.y-delta_y)*inv_h) );
		c1 = int( std::floor((tran_lower.x-delta_x)*inv_w) );
		r2 = int( std::ceil ((tran_upper.y+delta_y)*inv_h) );
		c2 = int( std::ceil ((tran_upper.x+delta_x)*inv_w) );
		clamp(r1,0,lod->row);
		clamp(r2,1,lod->row);
		clamp(c1,0,lod->col);
		clamp(c2,1,lod->col);
		py_print("find neighbors: row %d~%d, col %d~%d\n",r1,r2,c1,c2);
		row_grid = lod->grids+r1*lod->col;
		for(int r=r1; r<r2; ++r, row_grid+=lod->col)
		{
			for(int c=c1; c<c2; ++c)
			{
				for( entity_t *en = (entity_t*)(row_grid[c].sentinel.next); en; en=(entity_t*)(en->next) )
				{
					py_print("\tentity: aabb (%f,%f,%f,%f), group %d\n",en->lower.x,en->lower.y, en->upper.x, en->upper.y, en->mask);
					if( en->mask&filter && !(lower.x>en->upper.x || upper.x<en->lower.x || lower.y>en->upper.y || upper.y<en->lower.y) )
					{
						en->local_next=*en_head;
						*en_head=en;
					}
				} // entities in grid
			} // col
		} // row
	}
}

void xquad_tree::_find_neighbors(const xvec2f_t &beg, const xvec2f_t &end, const xvec2f_t &radius, unsigned filter, linked_node_t **en_head)
{
	assert(m_lod_info.size());
	lod_info_t *lod;
	int r1, c1, r2, c2;
	float delta_x, delta_y, inv_w, inv_h;
	unsigned level_mask = m_level_mask;
	xvec2f_t tran_lower, tran_upper;
	tran_lower.sub(beg,radius);
	tran_lower.sub(m_translate);
	tran_upper.add(end,radius);
	tran_upper.sub(m_translate);
	grid_t *row_grid;
	for(unsigned i=0, max_lod=m_lod_info.size(); i<max_lod; ++i)
	{
		if(!level_mask)
			break;
		level_mask >>= 1;
		lod=&m_lod_info[i];
		if(!lod->entity_count)
			continue;
		delta_x = lod->grid_w*0.5f+EPSILON_E4, delta_y = lod->grid_h*0.5f+EPSILON_E4;
		inv_w = 1.0f/lod->grid_w, inv_h = 1.0f/lod->grid_h;
		r1 = int( std::floor((tran_lower.y-delta_y)*inv_h) );
		c1 = int( std::floor((tran_lower.x-delta_x)*inv_w) );
		r2 = int( std::ceil ((tran_upper.y+delta_y)*inv_h) );
		c2 = int( std::ceil ((tran_upper.x+delta_x)*inv_w) );
		clamp(r1,0,lod->row);
		clamp(r2,0,lod->row);
		clamp(c1,0,lod->col);
		clamp(c2,0,lod->col);
		if(r1>r2)
			std::swap(r1,r2);
		if(c1>c2)
			std::swap(c1,c2);
		row_grid = lod->grids+r1*lod->col;
		for(int r=r1; r<r2; ++r, row_grid+=lod->col)
		{
			for(int c=c1; c<c2; ++c)
			{
				for( entity_t *en = (entity_t*)(row_grid[c].sentinel.next); en; en=(entity_t*)(en->next) )
				{
					if( en->mask&filter )
					{
						en->local_next=*en_head;
						*en_head=en;
					}
				} // entities in grid
			} // col
		} // row
	}
}

xquad_tree::grid_t* xquad_tree::_lod_search (const xvec2f_t &pos, const xvec2f_t &size)
{
	lod_info_t *lod = &m_lod_info[0];
	xvec2f_t center;
	center.sub(pos,m_translate);
	unsigned r, c;
	if(center.x<0)
		c=0;
	else {
		c=unsigned(center.x/lod->grid_w);
		if(c>=lod->col)
			c=lod->col-1;
	}
	if(center.y<0)
		r=0;
	else {
		r = unsigned(center.y/lod->grid_h);
		if(r>=lod->row)
			r=lod->row-1;
	}
	grid_t *grid = &m_grids[r*lod->col+c];
	for(; grid->parent; grid=grid->parent) 
	{
		lod=grid->lod;
		if(size.x<=lod->grid_w && size.y<=lod->grid_h)
			break;
	}
	return grid;
}

void xquad_tree::get_filled_grids(unsigned lod, std::vector<std::pair<unsigned,unsigned> > &grid_coords) const
{
	if(lod>=m_lod_info.size())
		return;
	const lod_info_t *lod_info = &m_lod_info[lod];
	if(!lod_info->entity_count)
		return;
	grid_t *grid = lod_info->grids;
	for(unsigned r=0; r<lod_info->row; ++r)
	{
		for(unsigned c=0; c<lod_info->col; ++c, ++grid) {
			if(grid->sentinel.next)
				grid_coords.push_back(std::pair<unsigned,unsigned>(r,c));
		}
	}
}

//-------------------------------------------------------------------------

#ifdef _DEBUG

void xquad_tree::debug_verify()
{
	std::vector<lod_info_t*> records;
	records.resize(m_grids.size(),0);
	grid_t *beg = &m_grids[0], *iter;
	iter=beg+m_grids.size()-1;
	std::vector<grid_t*> stack;
	stack.push_back(iter);
	unsigned idx;
	while(!stack.empty())
	{
		iter=stack.back();
		stack.pop_back();
		idx=iter-beg;
		assert(idx<records.size());
		assert(0==records[idx]);
		records[idx]=iter->lod;
		int i;
		for(i=0; i<4; ++i)
		{
			if(iter->childs[i]==0)
				break;
			assert(iter->childs[i]->parent==iter);
			stack.push_back(iter->childs[i]);
		}
		assert(i==iter->lod->child_count);
	}
	lod_info_t *lod = &m_lod_info[0];
	unsigned grid_cnt=0;
	for(idx=0; idx<records.size(); ++idx, ++grid_cnt)
	{
		if(grid_cnt==lod->grid_count) {
			++lod;
			grid_cnt=0;
		}
		assert(records[idx]!=0);
		assert(records[idx]==lod);
	}
	lod = &m_lod_info[0];
	lod_info_t *parent_lod = lod+1;
	for(idx=1; idx<m_lod_info.size(); ++idx, ++lod, ++parent_lod)
	{
		iter = lod->grids;
		for(unsigned r=0; r<lod->row; ++r)
		{
			for(unsigned c=0; c<lod->col; ++c, ++iter)
			{
				unsigned parent_idx = (r>>1)*parent_lod->col+(c>>1);
				assert(parent_lod->grids+parent_idx==iter->parent);
			}
		}
	}
}

void xquad_tree::debug_compare(const xquad_tree *other)
{
	assert(this->m_width == other->m_width);
	assert(this->m_height== other->m_height);
	assert(this->m_translate == other->m_translate);
	assert(this->m_level_mask== other->m_level_mask);
	assert(this->m_lod_info.size()==other->m_lod_info.size());
	assert(this->m_grids.size()==other->m_grids.size());
	const lod_info_t *lod1, *lod2;
	for(unsigned i=0, cnt=m_lod_info.size(); i<cnt; ++i)
	{
		lod1=&this->m_lod_info[i];
		lod2=&other->m_lod_info[i];
		assert(lod1->grid_w==lod2->grid_w);
		assert(lod1->grid_h==lod2->grid_h);
		assert(lod1->row==lod2->row);
		assert(lod1->col==lod2->col);
		assert(lod1->child_count==lod2->child_count);
		assert(lod1->grid_count==lod2->grid_count);
		assert(lod1->grids-&this->m_grids[0] == lod2->grids-&other->m_grids[0]);
	}
	const grid_t *g1, *g2;
	for(unsigned i=0, cnt=m_grids.size(); i<cnt; ++i)
	{
		g1=&this->m_grids[i];
		g2=&other->m_grids[i];
		assert(g1->lod-&this->m_lod_info[0] == g2->lod-&other->m_lod_info[0]);
		if(g1->parent)
			assert(g1->parent-&this->m_grids[0] == g2->parent-&other->m_grids[0]);
		else
			assert(g2->parent==0);
		for(unsigned j=0; j<4; ++j)
		{
			if(g1->childs[j])
				assert(g1->childs[j]-&this->m_grids[0] == g2->childs[j]-&other->m_grids[0]);
			else
				assert(g2->childs[j]==0);
		}
	}
}

bool xquad_tree::save(const char* filename) const
{
	assert(filename);
	std::fstream fs;
	fs.open(filename,std::ios_base::out|std::ios_base::binary);
	if(!fs.is_open())
		return false;
	struct {
		int version;
		unsigned width, height;
		xvec2f_t translate;
		unsigned level_mask;
		unsigned lod_count;
		unsigned grid_count;
	} header = {
		1, m_width, m_height, 
		m_translate, m_level_mask,
		m_lod_info.size(), m_grids.size(),
	};
	fs.write((char*)&header,sizeof(header));
	const lod_info_t *lod_beg, *lod_iter;
	const grid_t *grid_beg, *grid_iter;
	unsigned v, entity_count=0;
	if(m_grids.size()>0)
		grid_beg=&m_grids[0];
	else grid_beg=0;
	grid_iter=grid_beg;
	if(m_lod_info.size()>0)
		lod_beg=&m_lod_info[0];
	else
		lod_beg=0;
	lod_iter=lod_beg;
	for(unsigned i=0, cnt=m_lod_info.size(); i<cnt; ++i, ++lod_iter)
	{
		fs.write((char*)lod_iter,sizeof(lod_info_t)-sizeof(grid_t*));
		v = lod_iter->grids-grid_beg;
		fs.write((char*)&v,sizeof(unsigned));
		entity_count += lod_iter->entity_count;
	}
	for(unsigned i=0, cnt=m_grids.size(); i<cnt; ++i, ++grid_iter)
	{
		v = grid_iter->lod-lod_beg;
		fs.write((char*)&v,sizeof(unsigned));
		v = grid_iter->parent?grid_iter->parent-grid_beg:-1;
		fs.write((char*)&v,sizeof(unsigned));
		for(unsigned j=0; j<grid_iter->lod->child_count; ++j)
		{
			v = grid_iter->childs[j]-grid_beg;
			fs.write((char*)&v,sizeof(unsigned));
		}

	}
	fs.write((char*)&entity_count,sizeof(unsigned));
	fs.close();
	return true;
}

bool xquad_tree::load(const char* filename)
{
	assert(filename);
	std::fstream fs;
	fs.open(filename,std::ios_base::in|std::ios_base::binary);
	if(!fs.is_open())
		return false;
	struct {
		int version;
		unsigned width, height;
		xvec2f_t translate;
		unsigned level_mask;
		unsigned lod_count;
		unsigned grid_count;
	} header;
	fs.read((char*)&header,sizeof(header));
	if(header.version!=1)
	{
		fs.close();
		return false;
	}
	m_width = header.width;
	m_height= header.height;
	m_level_mask = header.level_mask;
	m_translate = header.translate;
	unsigned lod_count = header.lod_count;
	unsigned grid_count= header.grid_count;
	lod_info_t *lod_beg=0, *lod_iter=0;
	grid_t *grid_beg=0, *grid_iter=0;
	if(lod_count>0) {
		m_lod_info.resize(lod_count);
		lod_beg = &m_lod_info[0];
		lod_iter= lod_beg;
	}
	else m_lod_info.clear();
	if(grid_count>0) {
		m_grids.resize(grid_count);
		grid_beg = &m_grids[0];
		grid_iter= grid_beg;
	}
	else m_grids.clear();
	unsigned v;
	// read LOD info
	for(unsigned i=0; i<lod_count && fs; ++i, ++lod_iter)
	{
		fs.read((char*)lod_iter,sizeof(lod_info_t)-sizeof(grid_t*));
		lod_iter->entity_count = 0;
		fs.read((char*)&v,sizeof(unsigned));
		lod_iter->grids = grid_beg + v;
	}
	// read grid info
	for(unsigned i=0; i<grid_count && fs; ++i, ++grid_iter)
	{
		std::memset(grid_iter,0,sizeof(grid_t));
		fs.read((char*)&v,sizeof(unsigned));
		grid_iter->lod = lod_beg + v;
		fs.read((char*)&v,sizeof(unsigned));
		if(v!=unsigned(-1))
			grid_iter->parent = grid_beg + v;
		else
			grid_iter->parent = 0;
		for(unsigned j=0; j<grid_iter->lod->child_count; ++j)
		{
			fs.read((char*)&v,sizeof(unsigned));
			grid_iter->childs[j] = grid_beg + v;
		}		
	}
	unsigned entity_count=0;
	fs.read((char*)&entity_count,sizeof(unsigned));
	// read entities
	if(entity_count) {
	}
	fs.close();
	return true;
}


#endif //_DEBUG

//-------------------------------------------------------------------------

}; // namespace wyc

#ifdef _WYC_DEBUG

#include "wyc/util/util.h"
#include "wyc/util/time.h"
#include "wyc/util/hash.h"

struct test_handler_t
{
	unsigned counter;
	wyc::xset records;

	test_handler_t() 
	{
		counter=0;
	}
	void init(size_t s)
	{
		records.reserve(s);
	}
	void operator () ( wyc::xquad_tree::entity_t *en )
	{
		counter+=1;
		assert(!records.contain(en));
		records.add(en);
	}
	void reset() 
	{
		records.clear();
	}
	bool check( wyc::xquad_tree::entity_t *en) const
	{
		return records.contain(en);
	}
};

void test_static_detect(wyc::xquad_tree &qtree, wyc::xquad_tree::entity_t *entities, unsigned count, test_handler_t &handler, double &time)
{
	wyc::xcode_timer ct;
	wyc::xquad_tree::entity_t *en;
	for(unsigned i=0; i<count; ++i)
	{
		en = entities+i;
		ct.start();
		qtree.find_neighbors(en->lower,en->upper,1,handler);
		ct.stop();
		time += ct.get_time();
		for(unsigned j=0; j<count; ++j)
		{
			if(i!=j && is_overlap(en,entities+j))
			{
				if(!handler.check(entities+j))
					assert(0);
			}
		}
		handler.reset();
	}
}

void test_quad_tree()
{
	using namespace wyc;
	const unsigned grid_w=32, grid_h=32, row=32, col=32;
	const unsigned map_w=grid_w*col, map_h=grid_h*row;
	const char *filename = "map-1024x1024.cfg";
	double time;
	random_seed(unsigned(wyc::xtime_source::singleton().get_time()));

	xquad_tree qtree;
	test_handler_t handler;
	wyc_print("building tree...\n\tgrid size: %dx%d\n\trow: %d\n\tcol: %d\n\tmap size: %dx%d",
		grid_w,grid_h,row,col,map_w,map_h);
	qtree.initialize(grid_w,grid_h,row,col);
	qtree.debug_verify();

#define TEST_ENTITY_COUNT 100
	xquad_tree::entity_t entities[TEST_ENTITY_COUNT], *en;
	// add entity
	wyc_print("add %d entities...",TEST_ENTITY_COUNT);
	xvec2f_t center, size;
	for(unsigned i=0; i<TEST_ENTITY_COUNT; ++i)
	{
		en = entities+i;
		en->mask=1;
		center.set(random()*map_w,random()*map_h);
		size.set(16+random()*500,16+random()*500);
		en->lower=center-size*0.5f;
		en->upper=en->lower+size;
		qtree.add_entity(en);
	}
	// detect collision
	wyc_print("static test (%d times)...",TEST_ENTITY_COUNT);
	handler.counter = 0;
	time = 0;
	test_static_detect(qtree,entities,TEST_ENTITY_COUNT,handler,time);
	// update entity
	wyc_print("update %d entities...",TEST_ENTITY_COUNT);
	for(unsigned i=0; i<TEST_ENTITY_COUNT; ++i)
	{
		en = entities+i;
		center.set(random()*map_w,random()*map_h);
		size.set(16+random()*500,16+random()*500);
		en->lower=center-size*0.5f;
		en->upper=en->lower+size;
		qtree.update_entity(en);
	}
	// detect again
	test_static_detect(qtree,entities,TEST_ENTITY_COUNT,handler,time);
	wyc_print("\tcollisions: %d",handler.counter);
	wyc_print("\ttime used:  %f (ms)",time*1000);
	wyc_print("\tper call:   %f (ms)",time*1000/handler.counter);
	// save/load
	wyc_print("serializing...");
	if(!qtree.save(filename))
	{
		wyc_error("can't save data");
		assert(0);
	}
	wyc_print("save OK");
	xquad_tree tmp_tree;
	if(!tmp_tree.load(filename))
	{
		wyc_error("can't load data");
		assert(0);
	}
	wyc_print("load OK");
	tmp_tree.debug_verify();
	tmp_tree.debug_compare(&qtree);
	// remove entity
	wyc_print("remove %d entities...",TEST_ENTITY_COUNT/2);
	for(unsigned i=0, removal=TEST_ENTITY_COUNT/2; i<removal; ++i)
	{
		qtree.del_entity(entities+i);
	}
	wyc_print("clear remaining entities...");
	qtree.clear();
	for(unsigned i=0; i<TEST_ENTITY_COUNT; ++i)
		assert(entities[i].parent==0);

	wyc_print("All tests are done successfully!");
}

#endif // _WYC_DEBUG
