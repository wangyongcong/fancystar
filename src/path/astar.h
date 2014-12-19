#ifndef __HEADER_ASTAR
#define __HEADER_ASTAR

#include <cassert>
#include <vector>
#include "priorque.h"
#include "basedef.h"

namespace wyc 
{

enum NAVNODE_STATE {
	NAV_NORMAL=0,
	NAV_OPENED=1,
	NAV_CLOSED=2,
	NAV_LIST_FIELD=~(NAV_OPENED+NAV_CLOSED),
};

typedef struct navnode
{
	float cost;
	float estimate_cost; // estimate_cost=cost+heuristic_cost
	navnode *connect;
	uint32_t st;
	uint32_t index;
#ifdef _DEBUG
	uint32_t size;
#endif
}*pnavnode;

struct navnode_comparer
{
	inline bool operator () (const navnode *pnode1, const navnode *pnode2) {
		return pnode1->estimate_cost<pnode2->estimate_cost;
	}
};

class navmap
{
public:
	typedef std::vector<std::pair<uint32_t,float> > connectlist_t;
	virtual pnavnode get_node(uint32_t nodeid) const=0;
	virtual uint32_t coord2node(float x, float y, float z) const=0;
	virtual void node2coord(uint32_t nodeid, float &x, float &y, float &z) const=0;
	virtual unsigned get_connections(uint32_t nodeid, connectlist_t &connects) const=0;
	virtual float heuristic(uint32_t fr, uint32_t to) const=0;
};

class navigator
{
public:
	typedef std::vector<uint32_t> path_t;
	typedef path_t::iterator iterator;
	typedef path_t::const_iterator const_iterator;
	typedef priorque<navnode*,navnode_comparer> openlist_t;
	navigator(navmap *pmap);
	~navigator();
	bool find_path(uint32_t begnode, uint32_t endnode);
	void smooth();
	const_iterator begin() const;
	const_iterator end() const;
private:
	navmap *m_pmap;
	openlist_t m_open;
	path_t m_path;
	
};

};

#endif // end of __HEADER_ASTAR


