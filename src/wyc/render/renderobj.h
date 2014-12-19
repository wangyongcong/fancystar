#ifndef __HEADER_WYC_RENDEROBJ
#define __HEADER_WYC_RENDEROBJ

#include "wyc/obj/object.h"
#include "wyc/render/renderer.h"

namespace wyc
{

enum PICK_TYPE
{
	NO_PICK=0,
	PICK_QUAD=1,
	PICK_ALPHA=2,
};

class xrenderobj : public xobject
{
	USE_RTTI;
protected:
	bool m_visible;
public:
	xrenderobj();
	virtual void on_create(xrenderer*, xobjevent*) {}
	virtual void draw(xrenderer*) {}
	inline bool visible() const {
		return m_visible;
	}
	inline void show() {
		m_visible=true;
	}
	inline void hide() {
		m_visible=false;
	}
protected:
	virtual ~xrenderobj();
};

class xrenderlist
{
	typedef std::vector<std::pair<int,xrenderobj*> > render_list_t;
	struct is_null_obj {
		inline bool operator() (const std::pair<int,xrenderobj*> &obj_pair) {
			return 0==obj_pair.second;
		}
	};
	render_list_t m_rlist;
public:
	typedef render_list_t::iterator iterator;
	xrenderlist();
	~xrenderlist();
	void clear();
	void append(xrenderobj *pobj);
	void insert(xrenderobj *pobj, int priority);
	void remove(xrenderobj *pobj);
	void draw(xrenderer *pRenderer);
	template<typename DRAW>
		void draw(xrenderer *pRenderer, DRAW draw);
	size_t size() const;
	iterator begin();
	iterator end();
};

inline size_t xrenderlist::size() const {
	return m_rlist.size();
}

inline xrenderlist::iterator xrenderlist::begin() {
	return m_rlist.begin();
}

inline xrenderlist::iterator xrenderlist::end() {
	return m_rlist.end();
}

template<typename DRAW>
void xrenderlist::draw(xrenderer *pRenderer, DRAW draw) 
{
	xrenderobj *pobj;
	int erase=0;
	render_list_t::iterator iter, end;
	for(iter=m_rlist.begin(), end=m_rlist.end(); iter!=end; ++iter) {
		pobj=iter->second;
		if(!pobj) {
			erase+=1;
			continue;
		}
		if(pobj->dead()) {
			pobj->decref();
			iter->second=0;
			erase+=1;
			continue;
		}
		if(pobj->visible())
			draw(pRenderer,pobj);
	}
	if(erase>0) {
		iter=std::remove_if(m_rlist.begin(),end,is_null_obj());
		assert(erase==end-iter);
		m_rlist.erase(iter,end);
	}
}

}; // namespace wyc

#endif //__HEADER_WYC_RENDEROBJ

