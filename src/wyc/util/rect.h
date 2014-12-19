#ifndef __HEADER_WYC_XRECT
#define __HEADER_WYC_XRECT

namespace wyc
{

template<class T>
struct xrect
{
	union {
		T xmin;
		T left;
	};
	union {
		T ymin;
		T top;
	};
	union {
		T xmax;
		T right;
	};
	union {
		T ymax;
		T bottom;
	};

	xrect() {}
	xrect(T xMin, T yMin, T xMax, T yMax) : xmin(xMin), ymin(yMin), xmax(xMax), ymax(yMax) {}
	
	inline void zero() {
		xmin=ymin=xmax=ymax=0;
	}
	inline void set(T xMin, T yMin, T xMax, T yMax) {
		xmin=xMin;
		ymin=yMin;
		xmax=xMax;
		ymax=yMax;
	}
	inline void offset(T offx, T offy) {
		xmin+=offx;
		ymin+=offy;
		xmax+=offx;
		ymax+=offy;
	}
	inline void move_to(T x, T y) {
		xmax=x+width(); xmin=x;
		ymax=y+height(); ymin=y;
	}
	inline T width() const { 
		return xmax-xmin; 
	}
	inline T height() const { 
		return ymax-ymin; 
	}
	inline bool is_point_in(T x, T y) const {
		return (x>=xmin && x<xmax && y>=ymin && y<ymax);
	}
	void intersect(const xrect &rect)
	{
		xmin=MAX(xmin,rect.xmin);
		ymin=MAX(ymin,rect.ymin);
		xmax=MIN(xmax,rect.xmax);
		ymax=MIN(ymax,rect.ymax);
	}
	void intersect(const xrect &rect1, const xrect &rect2)
	{
		xmin=MAX(rect1.xmin,rect2.xmin);
		ymin=MAX(rect1.ymin,rect2.ymin);
		xmax=MIN(rect1.xmax,rect2.xmax);
		ymax=MIN(rect1.ymax,rect2.ymax);
	}
	inline bool is_intersect(const xrect &rect) const
	{
		return (rect.ymin<ymax && rect.ymax>ymin && 
			rect.xmin<xmax && rect.xmax>xmin);
	}
	inline bool is_rect_in(const xrect &rect) const {
		return (rect.ymin>=ymin && rect.ymax<=ymax &&
			rect.xmin>=xmin && rect.xmax<=xmax);
	}
	void get_union(const xrect &rect)
	{
		xmin=MIN(xmin,rect.xmin);
		ymin=MIN(ymin,rect.ymin);
		xmax=MAX(xmax,rect.xmax);
		ymax=MAX(ymax,rect.ymax);
	}
	void get_union(const xrect &rect1, const xrect &rect2)
	{
		xmin=MIN(rect1.xmin,rect2.xmin);
		ymin=MIN(rect1.ymin,rect2.ymin);
		xmax=MAX(rect1.xmax,rect2.xmax);
		ymax=MAX(rect1.ymax,rect2.ymax);
	}
	inline void inflate(T hs, T vs) {
		xmin-=hs; xmax+=hs;
		ymin-=vs; ymax+=vs;
	}
	inline void deflate(T hs, T vs) {
		xmin+=hs; xmax-=hs;
		ymin+=vs; ymax-=vs;
	}
	// 用rect来分解,分解得到的矩形保存在szout中
	// 返回分解得到的矩形个数,-1表示无交集,0表示被rect覆盖
	int split(const xrect &rect, xrect szout[4]) 
	{
		int ret=0;
		if(rect.ymin>=ymax || rect.ymax<=ymin || 
			rect.xmin>=xmax || rect.xmax<=xmin) 
			return -1; 
		if(rect.ymin>ymin) {
			xrect<T> &tmp=szout[ret];
			tmp.xmin=xmin;
			tmp.xmax=xmax;
			tmp.ymin=ymin;
			tmp.ymax=rect.ymin;
			ret+=1;
		}
		if(rect.ymax<ymax) {
			xrect<T> &tmp=szout[ret];
			tmp.xmin=xmin;
			tmp.xmax=xmax;
			tmp.ymin=rect.ymax;
			tmp.ymax=ymax;
			ret+=1;
		}
		int t=max(ymin,rect.ymin);
		int b=min(ymax,rect.ymax);
		if(rect.xmin>xmin) {
			xrect<T> &tmp=szout[ret];
			tmp.xmin=xmin;
			tmp.xmax=rect.xmin;
			tmp.ymin=t;
			tmp.ymax=b;
			ret+=1;
		}
		if(rect.xmax<xmax) {
			xrect<T> &tmp=szout[ret];
			tmp.xmin=rect.xmax;
			tmp.xmax=xmax;
			tmp.ymin=t;
			tmp.ymax=b;
			ret+=1;
		}
		return ret;
	}
	inline bool empty() const
	{
		return xmin>=xmax || ymin>=ymax;
	}
};
template<class T>
bool operator == (const xrect<T> &rect1, const xrect<T> &rect2)
{
	return (rect1.xmin==rect2.xmin && rect1.ymin==rect2.ymin 
		&& rect1.xmax==rect2.xmax && rect1.ymax==rect2.ymax);
}
template<class T>
bool operator != (const xrect<T> &rect1, const xrect<T> &rect2)
{
	return (rect1.xmin!=rect2.xmin || rect1.ymin!=rect2.ymin 
		|| rect1.xmax!=rect2.xmax || rect1.ymax!=rect2.ymax);
}

typedef xrect<int> xrecti_t;
typedef xrect<float> xrectf_t;
typedef xrect<double> xrectd_t;

} // namespace wyc

#endif // end of __HEADER_WYC_XRECT


