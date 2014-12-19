#include "astar.h"

using namespace wyc;

bool navigator::find_path(uint32_t begnode, uint32_t endnode) {
	m_path.clear();
	if(!m_pmap)
		return false;
	pnavnode pend, pcur, pto;
	pend=m_pmap->get_node(endnode);
	pcur=m_pmap->get_node(begnode);
	if(pend==0 || pcur==0)
		return false;
	if(pcur==pend) {
		m_path.push_back(begnode);
		return true;
	}
	m_open.clear();
	pcur->cost=0;
	pcur->estimate_cost=m_pmap->heuristic(pcur->index,pend->index);
	pcur->connect=0;
	pcur->st|=NAV_OPENED;
	m_open.push(pcur);
	float exact_cost, estimate;
	navmap::connectlist_t connections;
	navmap::connectlist_t::iterator iter, end;
	while(!m_open.empty()) {
		m_open.pop(pcur);
		if(pcur==pend) {
			// we arrive the end
			break;
		}
		m_pmap->get_connections(pcur->index,connections);
		iter=connections.begin();
		end=connections.end();
		for(; iter!=end; ++iter) {
			pto=m_pmap->get_node(iter->first);
			assert(pto);
			exact_cost=pcur->cost+iter->second;
			if(pto->st|NAV_OPENED) {
				// in open list
				if(exact_cost>=pto->cost) 
					continue;
				estimate=pto->estimate_cost-pto->cost;
			}
			else if(pto->st|NAV_CLOSED) {
				// in close list
				if(exact_cost>=pto->cost)
					continue;
				estimate=pto->estimate_cost-pto->cost;
				pto->st&=NAV_LIST_FIELD;
			}
			else {
				// new node
				estimate=m_pmap->heuristic(pto->index,pend->index);
			}
			pto->cost=exact_cost;
			pto->estimate_cost=exact_cost+estimate;
			pto->connect=pcur;
			if(pto->st|NAV_OPENED)
				m_open.refresh(pto);
			else {
				pto->st|=NAV_OPENED;
				m_open.push(pto);
			}
		}
		pcur->st&=NAV_LIST_FIELD;
		pcur->st|=NAV_CLOSED;
	}
	pto=pcur;
	while(pcur) {
		m_path.push_back(pcur->index);
		pcur=pcur->connect;
	}
	return pto==pend;
}

