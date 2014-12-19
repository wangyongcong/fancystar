#include <cassert>
#include <list>
#include "xutil.h"
#include "xraster.h"

using namespace std;

#pragma warning(disable: 4244)

//===================================================================================================
// xraster 任意多边形填充(扫描线填充算法)
//===================================================================================================

class xpolygon_filler
{
public:
	struct EDGE
	{
		int		m_yupper;
		float	m_xlower;
		float	m_dxdy;
		inline bool operator< (const EDGE &edge) const { 
			return m_xlower<edge.m_xlower; 
		}
	};
	typedef std::list<EDGE>	ActiveList;
	struct NODE
	{
		ActiveList m_list;
		int	m_scan;
		NODE(int scan=0) : m_list(), m_scan(scan) {}
		inline bool operator< (const NODE &node) const { 
			return m_scan<node.m_scan; 
		}
	};
	typedef std::list<NODE> EdgeList;
private:
	EdgeList m_edgeList;
	int m_begx, m_begy;
	int m_prex, m_prey;
	int m_curx, m_cury;
	int m_numPoints;
	bool m_bwait;
	EDGE *m_pedge;
public:
	xpolygon_filler() 
	{
		m_numPoints=0;
		m_bwait=false;
		m_pedge=0;
	}
	inline void clear() 
	{
		m_edgeList.clear();
		m_numPoints=0;
		m_bwait=false;
		m_pedge=0;
	}
	inline void beg_poly() {
		clear();
	}
	void push(int x, int y) {
		++m_numPoints;
		if(m_numPoints>2) {
			EDGE edge;
			if(m_bwait) {
				m_bwait=false;
				// lower=(m_prex,m_prey), upper=(m_curx,m_cury), cmp=(x,y)
				edge.m_yupper=m_cury<y?m_cury-1:m_cury;
				edge.m_xlower=m_prex;
				edge.m_dxdy=float(m_curx-m_prex)/(m_cury-m_prey);
				insert_edge(edge,m_prey);
			}
			if(y!=m_cury) {
				if(y<m_cury) {
					// lower=(x,y), upper=(m_curx,m_cury), cmp=(m_prex,m_prey)
					edge.m_yupper=m_cury<m_prey?m_cury-1:m_cury;
					edge.m_xlower=x;
					edge.m_dxdy=float(m_curx-x)/(m_cury-y);
					insert_edge(edge,y);
				}
				else m_bwait=true;
			}
			m_prex=m_curx;
			m_prey=m_cury;
			m_curx=x;
			m_cury=y;
			return;
		}
		if(m_numPoints>1) {
			m_curx=x;
			m_cury=y;
			EDGE edge;
			if(m_cury!=m_prey) {
				if(m_cury<m_prey) {
					edge.m_yupper=m_prey;
					edge.m_xlower=m_curx;
					edge.m_dxdy=float(m_prex-m_curx)/(m_prey-m_cury);
					m_pedge=insert_edge(edge,m_cury);
				}
				else m_bwait=true;
			}
			return;
		}
		m_begx=m_prex=x;
		m_begy=m_prey=y;
	}
	void end_poly() {
		push(m_begx,m_begy);
		EDGE edge;
		if(m_bwait) {
			m_bwait=false;
			// lower=(m_prex,m_prey), upper=(m_curx,m_cury)
			edge.m_yupper=m_pedge?m_cury:m_cury-1;
			edge.m_xlower=m_prex;
			edge.m_dxdy=float(m_curx-m_prex)/(m_cury-m_prey);
			insert_edge(edge,m_prey);
		}
		else if(m_pedge)
			m_pedge->m_yupper-=1;
	}
	inline operator bool () const {
		return !m_edgeList.empty();
	}
	inline unsigned num_points() const {
		return m_numPoints;
	}
	ActiveList& get(int &scan)
	{
		EdgeList::iterator active=m_edgeList.begin();
	//	active->m_list.sort();
		scan=active->m_scan;
		return active->m_list;
	}
	void next_scanline()
	{
		if(m_edgeList.empty()) 
			return;
		EdgeList::iterator node=m_edgeList.begin(), node_end=m_edgeList.end();
		ActiveList &active=node->m_list;
		// node->m_list必定不为空
		assert(!active.empty());
		node->m_scan+=1;
		ActiveList::iterator iter=active.begin(), end=active.end(), prev;
		while(iter!=end)
		{
			if(node->m_scan>iter->m_yupper) {
				iter=active.erase(iter);
				continue;
			}
			iter->m_xlower+=iter->m_dxdy;
			if(iter!=active.begin()) {
				prev=iter;
				if(iter->m_xlower<(--prev)->m_xlower) {
					while(prev!=active.begin()) {
						if(iter->m_xlower>=(--prev)->m_xlower) {
							++prev;
							break;
						}
					}
					active.insert(prev,*iter);
					iter=active.erase(iter);
					continue;
				}
			}
			++iter;
		}
		if(active.empty())
		{
			m_edgeList.erase(node);
			return;
		}
	/*	for(ActiveList::iterator t=active.begin(), t2=active.begin(); 
			++t2!=active.end(); ++t)
			if(t2->m_xlower<t->m_xlower) {
				Err("error!");
			}*/
		EdgeList::iterator node_next=node;
		++node_next;
		if(node_next!=m_edgeList.end() && node_next->m_scan==node->m_scan)
		{
			// merge操作必须确保每个链表有序
			node_next->m_list.merge(active);
			m_edgeList.erase(node);
		}
	}
#ifdef _DEBUG
	// 输出有序边表
	void dump_edge()
	{
		int cnt=0;
		for(EdgeList::iterator node=m_edgeList.begin(); node!=m_edgeList.end(); ++node)
		{
			print("node%d  scan=%d",cnt++,node->m_scan);
			int cnt2=0;
			for(ActiveList::iterator edge=node->m_list.begin();
				edge!=node->m_list.end(); ++edge)
			{
				print("edge%d: yupper=%d, xlower=%f, dxdy=%f",
					cnt2++,edge->m_yupper,edge->m_xlower,edge->m_dxdy);
			}
		}
	}
#endif
private:
	void make_edge(EDGE &edge, const xpt2i_t &lower, const xpt2i_t &upper, int ycmp)
	{
		edge.m_yupper=upper.y<ycmp?upper.y-1:upper.y;
		edge.m_xlower=float(lower.x);
		edge.m_dxdy=float(upper.x-lower.x)/(upper.y-lower.y);
	}
	EDGE* insert_edge(const EDGE &edge, int scan)
	{
		EdgeList::iterator iter, end;
		for(iter=m_edgeList.begin(), end=m_edgeList.end(); iter!=end; ++iter) 
			if(iter->m_scan>=scan)
				break;
		EDGE *pedge;
		if(iter!=end && iter->m_scan==scan)
		{
			ActiveList::iterator edge_iter, edge_end;
			for(edge_iter=iter->m_list.begin(), edge_end=iter->m_list.end(); 
				edge_iter!=edge_end && edge_iter->m_xlower<=edge.m_xlower; ++edge_iter);
			iter->m_list.insert(edge_iter,edge);
			pedge=&iter->m_list.back();
		}
		else
		{
			EdgeList::iterator new_node=m_edgeList.insert(iter,NODE(scan));
			new_node->m_list.push_back(edge);
			pedge=&new_node->m_list.back();
		}
		return pedge;
	}
};

void xraster::poly_fill_beg()
{
	if(m_pfiller==0) 
		m_pfiller=new xpolygon_filler;
	((xpolygon_filler*)m_pfiller)->beg_poly();
}

void xraster::poly_fill_add(int x, int y)
{
	((xpolygon_filler*)m_pfiller)->push(x,y);
}

void xraster::poly_fill_end()
{
	xpolygon_filler &filler=*((xpolygon_filler*)m_pfiller);
	if(filler.num_points()>2) {
		filler.end_poly();
		int scan;
		xpolygon_filler::ActiveList::iterator iter, next, end;
		while(filler) {
			xpolygon_filler::ActiveList &active=filler.get(scan);
			if(active.size()>1) {
				iter=active.begin();
				next=iter;
				end=active.end();
				while(iter!=end && ++next!=end) {
					scan_line_s(scan,fast_round(iter->m_xlower),fast_round(next->m_xlower));
					iter=++next;
				}
			}
			else if(active.size()>0) {
				iter=active.begin();
				plot_s(fast_round(iter->m_xlower),scan);
			}
			filler.next_scanline();
		}
	}
	delete (xpolygon_filler*)m_pfiller;
	m_pfiller=0;
}

#ifdef _DEBUG

bool xraster::poly_fill_start()
{
	if(m_pfiller) {
		xpolygon_filler &filler=*((xpolygon_filler*)m_pfiller);
		if(filler.num_points()>2) {
			filler.end_poly();
			return true;
		}
	}
	return false;
}

bool xraster::poly_fill_step()
{
	if(m_pfiller==0) 
		return false;
	xpolygon_filler &filler=*((xpolygon_filler*)m_pfiller);
	if(!filler) {
		delete (xpolygon_filler*)m_pfiller;
		m_pfiller=0;
		return false;
	}
	int scan;
	xpolygon_filler::ActiveList::iterator iter, next, end;
	xpolygon_filler::ActiveList &active=filler.get(scan);
	if(active.size()>1) {
		iter=active.begin();
		next=iter;
		end=active.end();
		while(iter!=end && ++next!=end) {
			scan_line_s(scan,fast_round(iter->m_xlower),fast_round(next->m_xlower));
			iter=++next;
		}
	}
	else if(active.size()>0) {
		iter=active.begin();
		plot_s(fast_round(iter->m_xlower),scan);
	}
	filler.next_scanline();
	return true;
}

void xraster::poly_fill_clear()
{
	if(m_pfiller) {
		delete (xpolygon_filler*)m_pfiller;
		m_pfiller=0;
	}
}

void xraster::poly_dump()
{
	if(m_pfiller) {
		xpolygon_filler &filler=*((xpolygon_filler*)m_pfiller);
		filler.dump_edge();
	}
}
#endif

//===================================================================================================
// 种子填充
//===================================================================================================
class xflood_filler
{
public:
	struct SHADOW
	{
		int	m_lx, m_rx;
		int	m_dadlx, m_dadrx;
		int	m_y;
		int	m_dir;
	};
private:
	typedef std::list<SHADOW>		Stack;
	typedef Stack::iterator			iterator;
	typedef Stack::const_iterator	const_iterator;
	Stack	m_stack;
public:
	xflood_filler() {}
	inline operator bool () const {
		return !m_stack.empty();
	}
	void begin(int x, int y) 
	{
		SHADOW shadow;
		shadow.m_dadlx=shadow.m_lx=x;
		shadow.m_dadrx=shadow.m_rx=x+1;
		shadow.m_y=y;
		shadow.m_dir=1;
		m_stack.push_back(shadow);
		shadow.m_y=y-1;
		shadow.m_dir=-1;
		m_stack.push_back(shadow);
	}
	inline void end() {
		m_stack.clear();
	}
	void push(int lx, int rx, int dadlx, int dadrx, int y, int dir)
	{
		SHADOW tmp;
		tmp.m_lx=lx;
		tmp.m_rx=rx;
		tmp.m_dadlx=lx;
		tmp.m_dadrx=rx;
		tmp.m_dir=dir;
		tmp.m_y=y+dir;
		m_stack.push_back(tmp);
		
		tmp.m_y=y-dir;
		tmp.m_dir=-dir;
		if(lx<dadlx)
		{
			tmp.m_lx=lx;
			tmp.m_rx=dadlx;
			m_stack.push_back(tmp);
		}
		if(rx>dadrx)
		{
			tmp.m_lx=dadrx;
			tmp.m_rx=rx;
			m_stack.push_back(tmp);
		}
	}
	inline void pop(SHADOW &shadow)
	{
		shadow=m_stack.back();
		m_stack.pop_back();
	}
	inline size_t size() const {
		return m_stack.size();
	}
};

void xraster::flood_fill(int x, int y)
{
	if(x<m_xmin || x>=m_xmax || y<m_ymin || y>=m_ymax)
		return;
	xflood_filler filler;
	xflood_filler::SHADOW shadow;
	filler.begin(x,y);
	uint32_t c=read_pixel(x,y);
	while(filler)
	{
		filler.pop(shadow);
		x=shadow.m_lx+1;
		y=shadow.m_y;
		if(y<m_ymin || y>=m_ymax) 
			continue;
		for(; shadow.m_lx>=m_xmin && c==read_pixel(shadow.m_lx,y); --shadow.m_lx);
		shadow.m_lx+=1;
		bool in=(shadow.m_lx!=x);
		while(x<m_xmax)
		{
			if(in) { // set the span
				if(c!=read_pixel(x,y)) {
					scan_line(y,shadow.m_lx,x-1);
					filler.push(shadow.m_lx,x,shadow.m_dadlx,shadow.m_dadrx,y,shadow.m_dir);
					in=false;
				}
			}
			else { // skip the gap
				if(x>=shadow.m_rx) 
					break;
				if(c==read_pixel(x,y)) {
					shadow.m_lx=x;
					in=true;
				}
			}
			++x;
		}
		if(in) {
			scan_line(y,shadow.m_lx,x-1);
			filler.push(shadow.m_lx,x,shadow.m_dadlx,shadow.m_dadrx,y,shadow.m_dir);
		}
	}
	filler.end();
}

