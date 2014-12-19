#ifndef __HEADER_WYC_XVECMATH
#define __HEADER_WYC_XVECMATH

/////////////////////////////////////////////////////////////////////////////
//
// 名称：XMath Lib
// 作者：Blackbird
// 版本：1.02
// 描述：提供3D图形运算的基本数学支持。包括三角运算，向量运算和矩阵运算。
//
/////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include "wyc/math/mathex.h"

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable:4201) //使用了非标准扩展 : 无名称的结构/联合
#endif // _MSC_VER

#define FAST_MATRIX4_INVERSE // 快速矩阵求逆

namespace wyc
{

/////////////////////////////////////////////////////////////
//
// 向量
//

//
// 通用向量模板
//
template<class T, int D>
class xvector
{
//
// 数据
//
public:
	T	elem[D];

//
// 构造函数
//
public:
	xvector() 
	{
		assert(D>1);
	}
	xvector(const xvector &v)
	{
		assert(D>1);
		*this=v;
	}
	inline xvector& operator = (const xvector &v)
	{
		memcpy(elem,v.elem,sizeof(T)*D);
		return *this;
	}
	xvector(T val) 
	{
		assert(D>1);
		*this=v;
	}
	xvector& operator = (T val)
	{
		unsigned i=0;
		while(i<D)
			elem[i++]=val;
		return *this;
	}
	template<int D2>
	xvector(const xvector<T,D2> &v) {
		assert(D>1);
		*this=v;
	}
	template<int D2>
	inline xvector& operator = (const xvector<T,D2> &v)
	{
		assert(D!=D2);
		memcpy(elem,v.elem,sizeof(T)*(D<D2?D:D2));
		return *this;
	}
//
// 外部接口
//
public:
	// 返回维数
	inline int dimension () const
	{
		return D;
	}
	// 元素置零
	inline void zero()
	{
		memset(elem,0,sizeof(T)*D);
	}
	// 安全的元素存取
	inline void set (unsigned i, T val)
	{
		if(i<D) elem[i]=val;
	}
	inline void get (unsigned i, T &val) const
	{
		if(i<D) val=elem[i];
	}
	// 快速元素存取
	inline T operator () (unsigned i) const
	{
		assert(i<D);
		return elem[i];
	}
	inline T& operator () (unsigned i)
	{
		assert(i<D);
		return elem[i];
	}
	inline T operator [] (unsigned i) const
	{
		assert(i<D);
		return elem[i];
	}
	inline T& operator [] (unsigned i)
	{
		assert(i<D);
		return elem[i];
	}
	// 一元运算符
	inline xvector& operator += (const xvector &v)
	{
		return add(v);
	}
	inline xvector& operator -= (const xvector &v)
	{
		return sub(v);
	}
	inline xvector& operator *= (T val)
	{
		return scale(val);
	}
	// 向量取反
	xvector& reverse()
	{
		for(int i=0; i<D; ++i)
			elem[i]=-elem[i];
		return *this;
	}
	xvector& reverse(const xvector &v)
	{
		for(int i=0; i<D; ++i)
			elem[i]=-v.elem[i];
		return *this;
	}
	// 向量加
	xvector& add (const xvector &v)
	{
		for(int i=0; i<D; ++i)
			elem[i]+=v.elem[i];
		return *this;
	}
	xvector& add (const xvector &v1, const xvector &v2)
	{
		for(int i=0; i<D; ++i)
			elem[i]=v1.elem[i]+v2.elem[i];
		return *this;
	}
	// 向量减
	xvector& sub (const xvector &v)
	{
		for(int i=0; i<D; ++i)
			elem[i]-=v.elem[i];
		return *this;
	}
	xvector& sub (const xvector &v1, const xvector &v2)
	{
		for(int i=0; i<D; ++i)
			elem[i]=v1.elem[i]-v2.elem[i];
		return *this;
	}
	// 标量积
	xvector& scale (T val)
	{
		for(int i=0; i<D; ++i)
			elem[i]*=val;
		return *this;
	}
	xvector& scale (const xvector &v, T val)
	{
		for(int i=0; i<D; ++i)
			elem[i]=v.elem[i]*val;
		return *this;
	}
	// 点积
	T dot (const xvector &v) const
	{
		T sum=0;
		for(int i=0; i<D; ++i)
			sum+=elem[i]*v.elem[i];
		return sum;
	}
	// 向量长度的平方
	T length2 () const
	{
		T sum=0;
		for(int i=0; i<D; ++i)
			sum+=elem[i]*elem[i];
		return sum;
	}
	// 向量长度
	inline T length () const
	{
		return (sqrt(length2()));
	}
	// 归一化为单位向量
	void normalize ()
	{
		T len=length();
		if(fequal(len,T(0)))
			return;
		len=T(1)/len;
		for(int i=0; i<D; ++i)
			elem[i]*=len;
	}
	// 比较
	bool operator == (const xvector& v) const 
	{
		for(int i=0; i<D; ++i)
			if(elem[i]!=v.elem[i])
				return false;
		return true;
	}
	bool operator != (const xvector& v) const 
	{
		for(int i=0; i<D; ++i)
			if(elem[i]!=v.elem[i])
				return true;
		return false;
	}
};

/////////////////////////////////////////////////////////////
//
// 2D向量
//
template<class T>
class xvector<T,2>
{
//
// 数据
//
public:
	union
	{
		T	elem[2];
		struct
		{
			T	x, y;
		};
	};
//
// 构造函数
//
public:
	xvector() {}

	xvector(T v1, T v2) : x(v1), y(v2) {}

	xvector(const xvector &v)
	{
		*this=v;
	}
	inline xvector& operator = (const xvector &v)
	{
		x=v.x;
		y=v.y;
		return *this;
	}
	xvector(T val) 
	{
		x=y=val;
	}
	inline xvector& operator = (T val)
	{
		x=y=val;
		return *this;
	}
	template<int D2>
	xvector(const xvector<T,D2> &v) {
		*this=v;
	}
	template<int D2>
	inline xvector& operator = (const xvector<T,D2> &v)
	{
		assert(1<D2);
		x=v[0];
		y=v[1];
		return *this;
	}
//
// 外部接口
//
public:
	// 返回维数
	inline int dimension () const
	{
		return 2;
	}
	inline void zero()
	{
		x=y=0;
	}
	inline void set(T vx, T vy)
	{
		x=vx;
		y=vy;
	}
	// 快速元素存取
	inline T operator () (unsigned i) const
	{
		assert(i<2);
		return elem[i];
	}
	inline T& operator () (unsigned i)
	{
		assert(i<2);
		return elem[i];
	}
	inline T operator [] (unsigned i) const
	{
		assert(i<2);
		return elem[i];
	}
	inline T& operator [] (unsigned i)
	{
		assert(i<2);
		return elem[i];
	}
	// 向量运算
	inline xvector& operator += (const xvector &v)
	{
		return add(v);
	}
	inline xvector& operator -= (const xvector &v)
	{
		return sub(v);
	}
	inline xvector& operator *= (T val)
	{
		return scale(val);
	}
	inline xvector& reverse()
	{
		x=-x;
		y=-y;
		return *this;
	}
	inline xvector& reverse (const xvector &v)
	{
		x=-v.x;
		y=-v.y;
		return *this;
	}
	inline xvector& add (const xvector &v)
	{
		x+=v.x;
		y+=v.y;
		return *this;
	}
	inline xvector& add (const xvector &v1, const xvector &v2)
	{
		x=v1.x+v2.x;
		y=v1.y+v2.y;
		return *this;
	}
	inline xvector& sub (const xvector &v)
	{
		x-=v.x;
		y-=v.y;
		return *this;
	}
	inline xvector& sub (const xvector &v1, const xvector &v2)
	{
		x=v1.x-v2.x;
		y=v1.y-v2.y;
		return *this;
	}
	inline xvector& scale (T val)
	{
		x*=val;
		y*=val;
		return *this;
	}
	inline xvector& scale (const xvector &v, T val)
	{
		x=v.x*val;
		y=v.y*val;
		return *this;
	}
	inline T dot (const xvector &v) const
	{
		return (x*v.x+y*v.y);
	}
	inline T length2() const
	{
		return (x*x+y*y);
	}
	inline T length() const
	{
		return (sqrt(x*x+y*y));
	}
	void normalize()
	{
		T len=length();
		if(fequal(len,T(0)))
			return;
		len=1/len;
		x*=len;
		y*=len;
	}
	// 2D向量差积
	T cross (const xvector &v1) const
	{
		return x*v1.y-y*v1.x;
	}
	// this逆时针到v的夹角的余弦
	inline T cos_of_angle(const xvector &v) const
	{
		return (this->dot(v)/(length()*v.length()));
	}
	bool operator == (const xvector& v) const 
	{
		return x==v.x && y==v.y;
	}
	bool operator != (const xvector& v) const 
	{
		return x!=v.x || y!=v.y;
	}
};

//////////////////////////////////////////////////////////////////////////
//
// 3D向量
//
template<class T>
class xvector<T,3>
{
//
// 数据
//
public:
	union
	{
		T	elem[3];
		struct
		{
			T	x,y,z;
		};
	};
//
// 构造函数
//
public:
	xvector() {}

	xvector(T v1, T v2, T v3) : x(v1), y(v2), z(v3) {}

	xvector(const xvector &v)
	{
		x=v.x;
		y=v.y;
		z=v.z;
	}
	inline xvector& operator = (const xvector &v)
	{
		x=v.x;
		y=v.y;
		z=v.z;
		return *this;
	}
	xvector(T val) 
	{
		x=y=z=val;
	}
	inline xvector& operator = (T val)
	{
		x=y=z=val;
		return *this;
	}
	template<int D2>
	xvector(const xvector<T,D2> &v) {
		*this=v;
	}
	template<int D2>
	xvector& operator = (const xvector<T,D2> &v)
	{
		assert(3!=D2);
		memcpy(elem,v.elem,sizeof(T)*(3<D2?3:D2));
		return *this;
	}
//
// 外部接口
//
public:
	// 返回维数
	inline int dimension () const
	{
		return 3;
	}
	inline void zero()
	{
		x=y=z=0;
	}
	inline void set(T vx, T vy, T vz)
	{
		x=vx;
		y=vy;
		z=vz;
	}
	// 快速元素存取
	inline T operator () (unsigned i) const
	{
		assert(i<3);
		return elem[i];
	}
	inline T& operator () (unsigned i)
	{
		assert(i<3);
		return elem[i];
	}
	inline T operator [] (unsigned i) const
	{
		assert(i<3);
		return elem[i];
	}
	inline T& operator [] (unsigned i)
	{
		assert(i<3);
		return elem[i];
	}
	// 向量运算
	inline xvector& operator += (const xvector &v)
	{
		return add(v);
	}
	inline xvector& operator -= (const xvector &v)
	{
		return sub(v);
	}
	inline xvector& operator *= (T val)
	{
		return scale(val);
	}
	inline xvector& reverse()
	{
		x=-x;
		y=-y;
		z=-z;
		return *this;
	}
	inline xvector& reverse (const xvector &v)
	{
		x=-v.x;
		y=-v.y;
		z=-v.z;
		return *this;
	}
	inline xvector& add (const xvector &v)
	{
		x+=v.x;
		y+=v.y;
		z+=v.z;
		return *this;
	}
	inline xvector& add (const xvector &v1, const xvector &v2)
	{
		x=v1.x+v2.x;
		y=v1.y+v2.y;
		z=v1.z+v2.z;
		return *this;
	}
	inline xvector& sub (const xvector &v)
	{
		x-=v.x;
		y-=v.y;
		z-=v.z;
		return *this;
	}
	inline xvector& sub (const xvector &v1, const xvector &v2)
	{
		x=v1.x-v2.x;
		y=v1.y-v2.y;
		z=v1.z-v2.z;
		return *this;
	}
	inline xvector& scale (T val)
	{
		x*=val;
		y*=val;
		z*=val;
		return *this;
	}
	inline xvector& scale (const xvector &v, T val)
	{
		x=v.x*val;
		y=v.y*val;
		z=v.z*val;
		return *this;
	}
	inline T dot (const xvector &v) const
	{
		return (x*v.x+y*v.y+z*v.z);
	}
	inline T length2() const
	{
		return (x*x+y*y+z*z);
	}
	inline T length() const
	{
		return (sqrt(x*x+y*y+z*z));
	}
	void normalize()
	{
		T len=length();
		if(fequal(len,T(0)))
			return;
		len=1/len;
		x*=len;
		y*=len;
		z*=len;
	}
	// 叉积，this = v1 cross v2
	xvector& cross (const xvector &v1, const xvector &v2)
	{
		x=v1.y*v2.z-v1.z*v2.y;
		y=v1.z*v2.x-v1.x*v2.z;
		z=v1.x*v2.y-v1.y*v2.x;
		return *this;
	}
	// this逆时针到v的夹角的余弦
	inline T cos_of_angle(const xvector &v) const
	{
		return (this->dot(v)/(length()*v.length()));
	}
	bool operator == (const xvector& v) const 
	{
		return x==v.x && y==v.y && z==v.z;
	}
	bool operator != (const xvector& v) const 
	{
		return x!=v.x || y!=v.y || z!=v.z;
	}
};

//////////////////////////////////////////////////////////////////////////
//
// 4D齐次向量(homogeneous vector)
//
template<class T>
class xvector<T,4>
{
//
// 数据
//
public:
	union
	{
		T	elem[4];
		struct
		{
			T	x,y,z,w;
		};
	};
//
// 构造函数
//
public:
	xvector() {}

	xvector(T v1, T v2, T v3, T v4=1) : x(v1), y(v2), z(v3), w(v4) {}

	xvector(const xvector &v)
	{
		*this=v;
	}
	inline xvector& operator = (const xvector &v)
	{
		x=v.x;
		y=v.y;
		z=v.z;
		w=v.w;
		return *this;
	}
	xvector(T val) 
	{
		x=y=z=w=val;
	}
	inline xvector& operator = (T val)
	{
		x=y=z=w=val;
		return *this;
	}
	template<int D2>
	xvector(const xvector<T,D2> &v) {
		*this=v;
	}
	template<int D2>
	xvector& operator = (const xvector<T,D2> &v)
	{
		assert(4!=D2);
		memcpy(elem,v.elem,sizeof(T)*(4<D2?4:D2));
		return *this;
	}
//
// 外部接口
//
public:
	// 返回维数
	inline int dimension () const
	{
		return 4;
	}
	inline void zero()
	{
		x=y=z=w=0;
	}
	inline void set(T vx, T vy, T vz, T vw=1)
	{
		x=vx;
		y=vy;
		z=vz;
		w=vw;
	}
	// 快速元素存取
	inline T operator () (unsigned i) const
	{
		assert(i<4);
		return elem[i];
	}
	inline T& operator () (unsigned i)
	{
		assert(i<4);
		return elem[i];
	}
	inline T operator [] (unsigned i) const
	{
		assert(i<4);
		return elem[i];
	}
	inline T& operator [] (unsigned i)
	{
		assert(i<4);
		return elem[i];
	}
	// 向量运算
	inline xvector& operator += (const xvector &v)
	{
		return add(v);
	}
	inline xvector& operator -= (const xvector &v)
	{
		return sub(v);
	}
	inline xvector& operator *= (T val)
	{
		return scale(val);
	}
	inline xvector& reverse()
	{
		x=-x;
		y=-y;
		z=-z;
		w=-w;
		return *this;
	}
	inline xvector& reverse (const xvector &v)
	{
		x=-v.x;
		y=-v.y;
		z=-v.z;
		w=-v.w;
		return *this;
	}
	inline xvector& add (const xvector &v)
	{
		x+=v.x;
		y+=v.y;
		z+=v.z;
		w+=v.w;
		return *this;
	}
	inline xvector& add (const xvector &v1, const xvector &v2)
	{
		x=v1.x+v2.x;
		y=v1.y+v2.y;
		z=v1.z+v2.z;
		w=v1.w+v2.w;
		return *this;
	}
	inline xvector& sub (const xvector &v)
	{
		x-=v.x;
		y-=v.y;
		z-=v.z;
		w-=v.w;
		return *this;
	}
	inline xvector& sub (const xvector &v1, const xvector &v2)
	{
		x=v1.x-v2.x;
		y=v1.y-v2.y;
		z=v1.z-v2.z;
		w=v1.w-v2.w;
		return *this;
	}
	inline xvector& scale (T val)
	{
		x*=val;
		y*=val;
		z*=val;
		w*=val;
		return *this;
	}
	inline xvector& scale (const xvector &v, T val)
	{
		x=v.x*val;
		y=v.y*val;
		z=v.z*val;
		w=v.w*val;
		return *this;
	}
	inline T dot (const xvector &v) const
	{
		return (x*v.x+y*v.y+z*v.z+w*v.w);
	}
	inline T length2() const
	{
		return (x*x+y*y+z*z+w*w);
	}
	inline T length() const
	{
		return (sqrt(x*x+y*y+z*z+w*w));
	}
	void normalize()
	{
		T len=length();
		if(fequal(len,T(0)))
			return;
		len=1/len;
		x*=len;
		y*=len;
		z*=len;
		w*=len;
	}
	void homogenize()
	{
		if(fequal(w,T(0)) || fequal(w,T(1)))
			return;
		w=1/w;
		x*=w;
		y*=w;
		z*=w;
		w=1;
	}
	// 执行3D叉积，w不变
	xvector& cross (const xvector &v1, const xvector &v2)
	{
		x=v1.y*v2.z-v1.z*v2.y;
		y=v1.z*v2.x-v1.x*v2.z;
		z=v1.x*v2.y-v1.y*v2.x;
		return *this;
	}
	// this逆时针到v的夹角的余弦
	inline T cos_of_angle(const xvector &v) const
	{
		return (this->dot(v)/(length()*v.length()));
	}
	bool operator == (const xvector& v) const 
	{
		return x==v.x && y==v.y && z==v.z && w==v.w;
	}
	bool operator != (const xvector& v) const 
	{
		return x!=v.x || y!=v.y || z!=v.z || w!=v.w;
	}
};

///////////////////////////////////////////////////////////////////////////////
//
// 向量运算符重载
// 

template<class T, int D>
xvector<T,D> operator - (const xvector<T,D> &v1)
{
	xvector<T,D> r;
	r.reverse(v1);
	return r;
}

template<class T, int D>
xvector<T,D> operator + (const xvector<T,D> &v1, const xvector<T,D> &v2)
{
	xvector<T,D> r;
	r.add(v1,v2);
	return r;
}

template<class T, int D>
xvector<T,D> operator - (const xvector<T,D> &v1, const xvector<T,D> &v2)
{
	xvector<T,D> r;
	r.sub(v1,v2);
	return r;
}

template<class T, int D>
xvector<T,D> operator * (const xvector<T,D> &v1, T val)
{
	xvector<T,D> r;
	r.scale(v1,val);
	return r;
}

template<class T, int D>
xvector<T,D> operator * (T val, const xvector<T,D> &v1)
{
	xvector<T,D> r;
	r.scale(v1,val);
	return r;
}

template<class T, int D>
T operator * (const xvector<T,D> &v1, const xvector<T,D> &v2)
{
	return v1.dot(v2);
}

///////////////////////////////////////////////////////////////////////////////
//
// 矩阵
//

//
// 通用矩阵模板
//
template<class T, int R, int C>
class xmatrix
{
//
// 数据
//
public:
	T	elem[R][C];	// 按行优先的次序存储矩阵元素
//
// 构造/析构函数
//
public:
	xmatrix() 
	{
		assert(R>0 && C>0);
	};
	xmatrix(const xmatrix &m)
	{
		assert(R>0 && C>0);
		*this=m;
	}
	inline xmatrix& operator = (const xmatrix &m)
	{
		memcpy(elem[0],m.elem[0],sizeof(T)*R*C);
		return *this;
	}
//
// 外部接口
//
public:
	// 返回行数和列数
	inline int row() const
	{
		return R;
	}
	inline int col() const
	{
		return C;
	}
	// 检测是否方阵
	inline bool square() const
	{
		return (R==C);
	}
	// 设置为单位矩阵
	void identity()
	{
		memset(&(elem[0][0]),0,sizeof(T)*R*C);
		assert(R==C);
		for(int i=0; i<R; ++i)
			elem[i][i]=1;
	}
	// 返回内部数据
	inline T* data() 
	{
		return elem[0];
	}
	inline const T* data() const
	{
		return elem[0];
	}
	// 安全的存取操作
	inline void set(unsigned r, unsigned c, T val)
	{
		if(r<R && c<C)
			elem[r][c]=val;
	}
	inline void get(unsigned r, unsigned c, T &val) const
	{
		if(r<R && c<C)
			val=elem[r][c];
	}
	// 以下的运算符重载使得存取元素更方便
	// 由于不作任何检查，小心内存越界
	inline T operator () (unsigned r, unsigned c) const
	{
		return elem[r][c];
	}
	inline T& operator () (unsigned r, unsigned c)
	{
		return elem[r][c];
	}
	// 设置行/列向量
	template<int D>
	inline void set_row(unsigned r, const xvector<T,D> &v)
	{
		assert(r<R);
		memcpy(elem[r],v.elem,sizeof(T)*(C<D?C:D));
	}
	template<int D>
	void set_col(unsigned c, const xvector<T,D> &v)
	{
		assert(c<C);
		for(int i=0; i<(R<D?R:D); ++i)
			elem[i][c]=v.elem[i];
	}
	inline void get_row(unsigned r, xvector<T,C> &v) const
	{
		assert(r<R);
		memcpy(v.elem,elem[r],sizeof(T)*C);
	}
	void get_col(unsigned c, xvector<T,R> &v) const
	{
		assert(c<C);
		for(int i=0; i<R; ++i)
			v[i]=elem[i][c];
	}
	//
	// 矩阵的基本运算
	//
	inline xmatrix& operator += (const xmatrix &m)
	{
		return this->add(m);
	}
	inline xmatrix& operator -= (const xmatrix &m)
	{
		return this->sub(m);
	}
	inline xmatrix& operator *= (T val)
	{
		return this->scale(val);
	}
	inline xmatrix& operator /= (T val)
	{
		return this->div(val);
	}
	inline xmatrix& operator *= (const xmatrix<T,C,R> &m) 
	{
		return this->mul(m);
	}
	// 矩阵加
	xmatrix& add (const xmatrix &m)
	{
		T *dst=elem[0];
		const T *src=m.elem[0];
		for(int i=R*C; i>0; --i)
		{
			*dst+=*src;
			++dst;
			++src;
		}
		return *this;
	}
	xmatrix& add (const xmatrix &m1, const xmatrix &m2)
	{
		T *dst=elem[0];
		const T *src1=m1.elem[0], *src2=m2.elem[0];
		for(int i=R*C; i>0; --i)
		{
			*dst=*src1+*src2;
			++dst;
			++src1;
			++src2;
		}
		return *this;
	}
	// 矩阵减
	xmatrix& sub (const xmatrix &m)
	{
		T *dst=elem[0];
		const T *src=m.elem[0];
		for(int i=R*C; i>0; --i)
		{
			*dst-=*src;
			++dst;
			++src;
		}
		return *this;
	}
	xmatrix& sub (const xmatrix &m1, const xmatrix &m2)
	{
		T *dst=elem[0];
		const T *src1=m1.elem[0], *src2=m2.elem[0];
		for(int i=R*C; i>0; --i)
		{
			*dst=*src1-*src2;
			++dst;
			++src1;
			++src2;
		}
		return *this;
	}
	// 标量乘法
	xmatrix& scale (T val)
	{
		T *dst=elem[0];
		for(int i=R*C; i>0; --i)
		{
			*dst*=val;
			++dst;
		}
		return *this;
	}
	xmatrix& scale (const xmatrix &m, T val)
	{
		T *dst=elem[0];
		const T *src=m.elem[0];
		for(int i=R*C; i>0; --i)
		{
			*dst=*src*val;
			++dst;
			++src;
		}
		return *this;
	}
	// 标量除法
	xmatrix& div (T val)
	{
		T *dst=elem[0];
		for(int i=R*C; i>0; --i)
		{
			*dst/=val;
			++dst;
		}
		return *this;
	}
	xmatrix& div (const xmatrix &m, T val)
	{
		T *dst=elem[0];
		const T *src=m.elem[0];
		for(int i=R*C; i>0; --i)
		{
			*dst=*src/val;
			++dst;
			++src;
		}
		return *this;
	}
	// 矩阵乘法
	xmatrix& mul (const xmatrix<T,C,R> &m) 
	{
		T tmp[C], *dst;
		int i, j, k;
		for(i=0; i<R; ++i)
		{
			dst=elem[i];
			for(j=0; j<C; ++j)
			{
				tmp[j]=0;
				for(k=0; k<C; ++k) 
					tmp[j]+=dst[k]*m.elem[k][j];
			}
			memcpy(dst,tmp,sizeof(T)*C);
		}
		return *this;
	}
	template<int I>
	xmatrix& mul (const xmatrix<T,R,I> &m1, const xmatrix<T,I,C> &m2)
	{
		T sum, *dst;
		const T *src;
		for(int i=0; i<R; ++i)
		{
			dst=elem[i];
			src=m1.elem[i];
			for(int j=0; j<C; ++j)
			{
				sum=0;
				for(int k=0; k<I; ++k)
					sum+=src[k]*m2(k,j);
				dst[j]=sum;
			}
		}
		return *this;
	}
	// 转置
	xmatrix& transpose() 
	{
		T *dst;
		for(int i=0; i<R; ++i)
		{
			dst=elem[i];
			for(int j=i+1; j<C; ++j)
				SWAP(dst[j],elem[j][i]);
		}
		return *this;
	}
	xmatrix& transpose (const xmatrix<T,C,R> &m)
	{
		T *dst;
		for(int i=0; i<R; ++i)
		{
			dst=elem[i];
			for(int j=0; j<C; ++j)
				dst[j]=m.elem[j][i];
		}
		return *this;
	}
	// 根据拉普拉斯展开式计算行列式
	// 注意，只有方阵才能调用该函数
	T det () const
	{
		assert(square());
		// 1/2/3阶方阵直接计算
		if(row()==1)
		{
			return elem[0][0];
		}
		else if(row()==2)
		{
			return (elem[0][0]*elem[1][1]-elem[0][1]*elem[1][0]);
		}
		else if(row()==3)
		{
			return (
				elem[0][0]*elem[1][1]*elem[2][2]+
				elem[0][1]*elem[1][2]*elem[2][0]+
				elem[0][2]*elem[1][0]*elem[2][1]-
				elem[2][0]*elem[1][1]*elem[0][2]-
				elem[2][1]*elem[1][2]*elem[0][0]-
				elem[2][2]*elem[1][0]*elem[0][1]
				);
		}
		// 4阶(或以上)递归展开
		xmatrix<T,R-1,C-1> sub;
		T v, r=0;
		for(unsigned i=0;i<C;++i)
		{
			// 填充子矩阵
			_get_sub_matrix(sub,0,i);
			// 计算该子阵的代数余子式
			v=(i%2) ? (-elem[0][i]) : (elem[0][i]);
			v*=sub.det();
			// 将结果累加
			r+=v;
		}
		return r;
	}
	// 基于Gauss-Jordan消元法计算m的逆阵
	// 结果保存在this中，如果成功返回true，否则返回false
	// 只有方阵才能调用该函数
	bool inverse (const xmatrix &m)
	{
		assert(square());
		*this=m;
		int i, j, k;
		int row[R], col[C];
		T det=1;
		T *src, *dst;
		for(k=0; k<R; ++k)
		{
			T max=0;
			// 寻找主元
			for(i=k;i<R;++i)
			{
				src=elem[i];
				for(j=k;j<C;++j)
				{
					if(abs(src[j])>max)
					{
						max=src[j];
						row[k]=i;
						col[k]=j;
					}
				}
			}
			// 如果主元为0，逆阵不存在
			if(fequal(max,0))
				return false;
			if(row[k]!=k)
			{
				// 行交换
				det=-det;
				src=elem[row[k]];
				dst=elem[k];
				for(j=0;j<C;++j)
					SWAP(src[j], dst[j]);
			}
			if(col[k]!=k)
			{
				// 列交换
				det=-det;
				for(j=col[k],i=0;i<R;++i)
					SWAP(elem[i][j], elem[i][k]);
			}
			T mkk=elem[k][k];
			// 计算行列式
			det *= mkk;
			// 计算逆阵元素
			mkk=1/mkk;
			elem[k][k]=mkk;
			src=elem[k];
			for(j=0;j<k;++j)
				src[j]*=mkk;
			for(++j; j<C; ++j)
				src[j]*=mkk;
			for(i=0;i<R;++i)
			{
				if(i==k)
					continue;
				dst=elem[i];
				for(j=0;j<C;++j)
					if(j!=k)
						dst[j]-=src[j]*dst[k];
			}
			mkk=-mkk;
			for(i=0;i<R;++i)
				if(i!=k)
					elem[i][k]*=mkk;
		}
		// 如果之前执行了行/列交换，则需要执行逆交换
		// 交换次序相反，且行（列）交换用列（行）交换代替
		for(k=R-1;k>=0;--k)
		{
			if(col[k]!=k)
			{
				src=elem[col[k]];
				dst=elem[k];
				for(j=0;j<C;++j)
					SWAP(src[j], dst[j]);
			}
			if(row[k]!=k)
			{
				for(j=row[k],i=0;i<R;++i)
					SWAP(elem[i][j], elem[i][k]);
			}
		}
		return true;
	}
	// 矩阵比较
	bool operator == (const xmatrix &m) const
	{
		const T *iter1=elem[0], *iter2=m.elem[0];
		for(int i=R*C; i>0; --i) {
			if(*iter1!=*iter2)
				return false;
			++iter1;
			++iter2;
		}
		return true;
	}
	bool operator != (const xmatrix &m) const
	{
		const T *iter1=elem[0], *iter2=m.elem[0];
		for(int i=R*C; i>0; --i) {
			if(*iter1!=*iter2)
				return true;
			++iter1;
			++iter2;
		}
		return false;
	}

	template<int R2, int C2>
	xmatrix& copy (const xmatrix<T,R2,C2> &m) 
	{
		int r=std::min(R2,R);
		int c=std::min(C2,C);
		for(int i=0; i<r; ++i)
		{
			for(int j=0; j<c; ++j)
				elem[i][j]=m.elem[i][j];
		}
		return *this;
	}
//
// 内部接口
//
protected:
	// 获取去除第rr行rc列后的子阵
	void _get_sub_matrix (xmatrix<T,R-1,C-1> &m, int rr, int rc) const
	{
		unsigned i,j,k,l;
		for(i=0,k=0;i<R;++i)
		{
			if(i==rr)
				continue;
			for(j=0,l=0;j<C;++j)
			{
				if(j==rc)
					continue;
				m.elem[k][l]=elem[i][j];
				++l;
			}
			++k;
		}
	}
};

//////////////////////////////////////////////////////////////////////////
//
// 2X2方阵
//
template<class T>
class xmatrix<T,2,2>
{
// 数据
public:
	// 按行优先的次序存储矩阵元素
	union
	{
		T	elem[2][2];
		struct
		{
			T 
			m00, m01,
			m10, m11;
		};
	};

// 构造/析构函数
public:
	xmatrix() {};

	xmatrix(const xmatrix &m)
	{
		*this=m;
	}

	xmatrix& operator = (const xmatrix &m)
	{
		m00=m.m00; m01=m.m01;
		m10=m.m10; m11=m.m11;
		return *this;
	}

// 外部接口
public:
	// 返回行数和列数
	inline int row() const
	{
		return 2;
	}
	inline int col() const
	{
		return 2;
	}
	// 检测是否方阵
	inline bool square() const
	{
		return true;
	}
	inline void identity ()
	{
		m00=m11=1;
		m10=m01=0;
	}
	// 返回内部数据
	inline T* data() 
	{
		return elem[0];
	}
	inline const T* data() const
	{
		return elem[0];
	}
	// 安全的存取操作
	inline void set(unsigned r, unsigned c, T val)
	{
		if(r<2 && c<2)
			elem[r][c]=val;
	}
	inline void get(unsigned r, unsigned c, T &val) const
	{
		if(r<2 &&c<2)
			val=elem[r][c];
	}
	// 以下的运算符重载使得存取元素更方便
	// 由于不作任何检查，小心内存越界
	inline T operator () (unsigned r, unsigned c) const
	{
		assert(r<2 && c<2);
		return elem[r][c];
	}
	inline T& operator () (unsigned r, unsigned c)
	{
		assert(r<2 && c<2);
		return elem[r][c];
	}
	// 设置行/列向量
	template<int D>
	inline void set_row(unsigned r, const xvector<T,D> &v)
	{
		assert(r<2);
		elem[r][0]=v[0];
		elem[r][1]=v[1];
	}
	template<int D>
	inline void set_col(unsigned c, const xvector<T,D> &v)
	{
		assert(c<2);
		elem[0][c]=v[0];
		elem[1][c]=v[1];
	}
	inline void get_row(unsigned r, xvector<T,2> &v) const 
	{
		assert(r<2);
		v.set(elem[r][0],elem[r][1]);
	}
	inline void get_col(unsigned c, xvector<T,2> &v) const
	{
		assert(c<2);
		v.set(elem[0][c],elem[1][c]);
	}
	//
	// 矩阵的基本运算
	//
	inline xmatrix& operator += (const xmatrix &m)
	{
		return this->add(m);
	}
	inline xmatrix& operator -= (const xmatrix &m)
	{
		return this->sub(m);
	}
	inline xmatrix& operator *= (T val)
	{
		return this->scale(val);
	}
	inline xmatrix& operator /= (T val)
	{
		return this->div(val);
	}
	inline xmatrix& operator *= (const xmatrix &m)
	{
		return this->mul(m);
	}
	xmatrix& add (const xmatrix &m)
	{
		m00+=m.m00; m01+=m.m01; 
		m10+=m.m10; m11+=m.m11; 
		return *this;
	}
	xmatrix& add (const xmatrix &m1, const xmatrix &m2)
	{
		m00=m1.m00+m2.m00; m01=m1.m01+m2.m01; 
		m10=m1.m10+m2.m10; m11=m1.m11+m2.m11; 
		return *this;
	}
	xmatrix& sub (const xmatrix &m)
	{
		m00-=m.m00; m01-=m.m01; 
		m10-=m.m10; m11-=m.m11; 
		return *this;
	}
	xmatrix& sub (const xmatrix &m1, const xmatrix &m2)
	{
		m00=m1.m00-m2.m00; m01=m1.m01-m2.m01; 
		m10=m1.m10-m2.m10; m11=m1.m11-m2.m11; 
		return *this;
	}
	xmatrix& scale (T val)
	{
		m00*=val; m01*=val;
		m10*=val; m11*=val;
		return *this;
	}
	xmatrix& scale (const xmatrix &m, T val)
	{
		m00=m.m00*val; m01=m.m01*val;
		m10=m.m10*val; m11=m.m11*val;
		return *this;
	}
	xmatrix& div (T val)
	{
		m00/=val; m01/=val;
		m10/=val; m11/=val;
		return *this;
	}
	xmatrix& div (const xmatrix &m, T val)
	{
		m00=m.m00/val; m01=m.m01/val;
		m10=m.m10/val; m11=m.m11/val;
		return *this;
	}
	xmatrix& mul (const xmatrix &m) 
	{
		T tmp0, tmp1;
		tmp0=m00*m.m00+m01*m.m10;
		tmp1=m00*m.m01+m01*m.m11;
		m00=tmp0;
		m01=tmp1;
		tmp0=m10*m.m00+m11*m.m10;
		tmp1=m10*m.m01+m11*m.m11;
		m10=tmp0;
		m11=tmp1;
		return *this;
	}
	xmatrix& mul (const xmatrix &m1, const xmatrix &m2)
	{
		m00=m1.m00*m2.m00+m1.m01*m2.m10;
		m01=m1.m00*m2.m01+m1.m01*m2.m11;
		m10=m1.m10*m2.m00+m1.m11*m2.m10;
		m11=m1.m10*m2.m01+m1.m11*m2.m11;
		return *this;
	}
	inline xmatrix& transpose() 
	{
		SWAP(m01,m10);
		return *this;
	}
	xmatrix& transpose (const xmatrix &m)
	{
		m00=m.m00; m01=m.m10;
		m10=m.m01; m11=m.m11;
		return *this;
	}
	inline T det () const
	{
		return (m00*m11-m01*m10); 
	}
	bool inverse (const xmatrix &m)
	{
		T d=m.det();
		if(fequal(d,0))
			return false;
		T invdet=T(1)/d;
		m00=m.m11*invdet; m01=-m.m01*invdet;
		m10=-m.m10*invdet; m11=m.m00*invdet;
		return true;
	}
	bool operator == (const xmatrix &mat) const
	{
		return m00==mat.m00 && m01==mat.m01 
			&& m10==mat.m10 && m11==mat.m11;
	}
	bool operator != (const xmatrix &mat) const
	{
		return m00!=mat.m00 || m01!=mat.m01 
			|| m10!=mat.m10 || m11!=mat.m11;
	}
};

/////////////////////////////////////////////////////////////////////////////////////
//
// 3x3方阵
//
template<class T>
class xmatrix<T,3,3>
{
// 数据
public:
	// 按行优先的次序存储矩阵元素
	union
	{
		T	elem[3][3];
		struct
		{
			T 
			m00, m01, m02,
			m10, m11, m12,
			m20, m21, m22;
		};
	};

// 构造/析构函数
public:
	xmatrix() {};

	xmatrix(const xmatrix &m)
	{
		*this=m;
	}

	xmatrix(const xmatrix<T,2,2> &m)
	{
		*this=m;
	}

	xmatrix(const xmatrix<T,4,4> &m)
	{
		*this=m;
	}

	xmatrix& operator = (const xmatrix &m)
	{
		memcpy(elem[0],m.elem[0],sizeof(T)*9);
		return *this;
	}

	xmatrix& operator = (const xmatrix<T,2,2> &m)
	{
		for(int i=0; i<2; ++i)
		{
			for(int j=0; j<2; ++j)
				elem[i][j]=m.elem[i][j];
		}
		elem[0][2]=0;
		elem[1][2]=0;
		T *row=elem[2];
		row[0]=0;
		row[1]=0;
		row[2]=1;
		return *this;
	}

	xmatrix& operator = (const xmatrix<T,4,4> &m)
	{
		for(int i=0; i<3; ++i)
		{
			for(int j=0; j<3; ++j)
				elem[i][j]=m.elem[i][j];
		}
		return *this;
	}

// 外部接口
public:
	// 返回行数和列数
	inline int row() const
	{
		return 3;
	}
	inline int col() const
	{
		return 3;
	}
	// 检测是否方阵
	inline bool square() const
	{
		return true;
	}
	// 构造单位矩阵
	inline void identity ()
	{
		m00=m11=m22=1;
		m01=m02=m10=m12=m20=m21=0;
	}
	// 返回内部数据
	inline T* data() 
	{
		return elem[0];
	}
	inline const T* data() const
	{
		return elem[0];
	}
	// 安全的存取操作
	inline void set(unsigned r, unsigned c, T val)
	{
		if(r<3 && c<3)
			elem[r][c]=val;
	}
	inline void get(unsigned r, unsigned c, T &val) const
	{
		if(r<3 && c<3)
			val=elem[r][c];
	}
	// 以下的运算符重载使得存取元素更方便
	// 由于不作任何检查，小心内存越界
	inline T operator () (unsigned r, unsigned c) const
	{
		assert(r<3 && c<3);
		return elem[r][c];
	}
	inline T& operator () (unsigned r, unsigned c)
	{
		assert(r<3 && c<3);
		return elem[r][c];
	}
	// 设置行/列向量
	template<int D>
	inline void set_row(unsigned r, const xvector<T,D> &v)
	{
		assert(r<3);
		assert(D>2);
		T *row=elem[r];
		row[0]=v[0];
		row[1]=v[1];
		row[2]=v[2];
	}
	template<int D>
	inline void set_col(unsigned c, const xvector<T,D> &v)
	{
		assert(c<3);
		assert(D>2);
		elem[0][c]=v[0];
		elem[1][c]=v[1];
		elem[2][c]=v[2];
	}
	inline void set_row(unsigned r, const xvector<T,2> &v)
	{
		assert(r<3);
		elem[r][0]=v.x;
		elem[r][1]=v.y;
	}
	inline void set_col(unsigned c, const xvector<T,2> &v)
	{
		assert(c<3);
		elem[0][c]=v.x;
		elem[1][c]=v.y;
	}
	inline void get_row(unsigned r, xvector<T,3> &v) const 
	{
		assert(r<3);
		const T *row=elem[r];
		v.set(row[0],row[1],row[2]);
	}
	inline void get_col(unsigned c, xvector<T,3> &v) const
	{
		assert(c<3);
		v.set(elem[0][c],elem[1][c],elem[2][c]);
	}
	//
	// 矩阵的基本运算
	//
	inline xmatrix& operator += (const xmatrix &m)
	{
		return this->add(m);
	}
	inline xmatrix& operator -= (const xmatrix &m)
	{
		return this->sub(m);
	}
	inline xmatrix& operator *= (T val)
	{
		return this->scale(val);
	}
	inline xmatrix& operator /= (T val)
	{
		return this->div(val);
	}
	inline xmatrix& operator *= (const xmatrix &m) 
	{
		return this->mul(m);
	}
	xmatrix& add (const xmatrix &m)
	{
		m00+=m.m00; m01+=m.m01; m02+=m.m02;
		m10+=m.m10; m11+=m.m11; m12+=m.m12;
		m20+=m.m20; m21+=m.m21; m22+=m.m22;
		return *this;
	}
	xmatrix& add (const xmatrix &m1, const xmatrix &m2)
	{
		m00=m1.m00+m2.m00; m01=m1.m01+m2.m01; m02=m1.m02+m2.m02;
		m10=m1.m10+m2.m10; m11=m1.m11+m2.m11; m12=m1.m12+m2.m12;
		m20=m1.m20+m2.m20; m21=m1.m21+m2.m21; m22=m1.m22+m2.m22;
		return *this;
	}
	xmatrix& sub (const xmatrix &m)
	{
		m00-=m.m00; m01-=m.m01; m02-=m.m02;
		m10-=m.m10; m11-=m.m11; m12-=m.m12;
		m20-=m.m20; m21-=m.m21; m22-=m.m22;
		return *this;
	}
	xmatrix& sub (const xmatrix &m1, const xmatrix &m2)
	{
		m00=m1.m00-m2.m00; m01=m1.m01-m2.m01; m02=m1.m02-m2.m02;
		m10=m1.m10-m2.m10; m11=m1.m11-m2.m11; m12=m1.m12-m2.m12;
		m20=m1.m20-m2.m20; m21=m1.m21-m2.m21; m22=m1.m22-m2.m22;
		return *this;
	}
	xmatrix& scale (T val)
	{
		m00*=val; m01*=val; m02*=val;
		m10*=val; m11*=val; m12*=val;
		m20*=val; m21*=val; m22*=val;
		return *this;
	}
	xmatrix& scale (const xmatrix &m, T val) 
	{
		m00=m.m00*val; m01=m.m01*val; m02=m.m02*val;
		m10=m.m10*val; m11=m.m11*val; m12=m.m12*val;
		m20=m.m20*val; m21=m.m21*val; m22=m.m22*val;
		return *this;
	}
	inline xmatrix& div (T val)
	{
		T inv=T(1)/val;
		return scale(inv);
	}
	inline xmatrix& div (const xmatrix &m, T val) 
	{
		T inv=T(1)/val;
		return scale(m,inv);
	}
	xmatrix& mul (const xmatrix &mat)
	{
		T temp[3];
		temp[0]=m00*mat.m00+m01*mat.m10+m02*mat.m20;
		temp[1]=m00*mat.m01+m01*mat.m11+m02*mat.m21;
		temp[2]=m00*mat.m02+m01*mat.m12+m02*mat.m22;
		m00=temp[0];
		m01=temp[1];
		m02=temp[2];
		temp[0]=m10*mat.m00+m11*mat.m10+m12*mat.m20;
		temp[1]=m10*mat.m01+m11*mat.m11+m12*mat.m21;
		temp[2]=m10*mat.m02+m11*mat.m12+m12*mat.m22;
		m10=temp[0];
		m11=temp[1];
		m12=temp[2];
		temp[0]=m20*mat.m00+m21*mat.m10+m22*mat.m20;
		temp[1]=m20*mat.m01+m21*mat.m11+m22*mat.m21;
		temp[2]=m20*mat.m02+m21*mat.m12+m22*mat.m22;
		m20=temp[0];
		m21=temp[1];
		m22=temp[2];
		return *this;
	}
	xmatrix& mul (const xmatrix &m1, const xmatrix &m2)
	{
		m00=m1.m00*m2.m00+m1.m01*m2.m10+m1.m02*m2.m20;
		m01=m1.m00*m2.m01+m1.m01*m2.m11+m1.m02*m2.m21;
		m02=m1.m00*m2.m02+m1.m01*m2.m12+m1.m02*m2.m22;
		m10=m1.m10*m2.m00+m1.m11*m2.m10+m1.m12*m2.m20;
		m11=m1.m10*m2.m01+m1.m11*m2.m11+m1.m12*m2.m21;
		m12=m1.m10*m2.m02+m1.m11*m2.m12+m1.m12*m2.m22;
		m20=m1.m20*m2.m00+m1.m21*m2.m10+m1.m22*m2.m20;
		m21=m1.m20*m2.m01+m1.m21*m2.m11+m1.m22*m2.m21;
		m22=m1.m20*m2.m02+m1.m21*m2.m12+m1.m22*m2.m22;
		return *this;
	}
	xmatrix& transpose() 
	{
		SWAP(m01,m10); 
		SWAP(m02,m20); 
		SWAP(m12,m21);
		return *this;
	}
	xmatrix& transpose (const xmatrix &m)
	{
		m00=m.m00; m01=m.m10; m02=m.m20;
		m10=m.m01; m11=m.m11; m12=m.m21;
		m20=m.m02; m21=m.m12; m22=m.m22;
		return *this;
	}
	T det () const
	{
		return (
			m00*m11*m22+
			m01*m12*m20+
			m02*m10*m21-
			m20*m11*m02-
			m21*m12*m00-
			m22*m10*m01
			);
	}
	bool inverse (const xmatrix &m)
	{
		T d=det();
		if(fequal(d,0))
			return false;
		// 计算伴随矩阵
		T adj[9];
		adj[0]=m.m11*m.m22-m.m12*m.m21;
		adj[3]=m.m12*m.m20-m.m10*m.m22;
		adj[6]=m.m10*m.m21-m.m11*m.m20;
		adj[1]=m.m02*m.m21-m.m01*m.m22;
		adj[4]=m.m00*m.m22-m.m02*m.m20;
		adj[7]=m.m01*m.m20-m.m00*m.m21;
		adj[2]=m.m01*m.m12-m.m02*m.m11;
		adj[5]=m.m02*m.m10-m.m00*m.m12;
		adj[8]=m.m00*m.m11-m.m01*m.m10;
		// 计算逆阵
		T *dst=elem[0];
		T invdet=T(1)/d;
		for(int i=0; i<9; ++i)
			dst[i]=adj[i]*invdet;
		return true;
	}
	bool operator == (const xmatrix &m) const
	{
		const T *iter1=elem[0], *iter2=m.elem[0];
		for(int i=9; i>0; --i) {
			if(*iter1!=*iter2)
				return false;
			++iter1;
			++iter2;
		}
		return true;
	}
	bool operator != (const xmatrix &m) const
	{
		const T *iter1=elem[0], *iter2=m.elem[0];
		for(int i=9; i>0; --i) {
			if(*iter1!=*iter2)
				return true;
			++iter1;
			++iter2;
		}
		return false;
	}

};

/////////////////////////////////////////////////////////////////////////////////////
//
// 4x4方阵
//
template<class T>
class xmatrix<T,4,4>
{
// 数据
public:
	// 按行优先的次序存储矩阵元素
	union
	{
		T	elem[4][4];
		struct
		{
			T 
			m00, m01, m02, m03,
			m10, m11, m12, m13,
			m20, m21, m22, m23,
			m30, m31, m32, m33;
		};
	};

// 构造/析构函数
public:
	xmatrix() {};

	xmatrix(const xmatrix &m)
	{
		*this=m;
	}

	xmatrix& operator = (const xmatrix &m)
	{
		memcpy(elem[0],m.elem[0],sizeof(T)*16);
		return *this;
	}

// 外部接口
public:
	// 返回行数和列数
	inline int row() const
	{
		return 4;
	}
	inline int col() const
	{
		return 4;
	}
	// 检测是否方阵
	inline bool square() const
	{
		return true;
	}
	// 构造单位矩阵
	inline void identity ()
	{
		memset(elem[0],0,sizeof(elem));
		m00=m11=m22=m33=1;
	}
	// 返回内部数据
	inline T* data() 
	{
		return elem[0];
	}
	inline const T* data() const
	{
		return elem[0];
	}
	// 安全的存取操作
	inline void set(unsigned r, unsigned c, T val)
	{
		if(r<4 && c<4)
			elem[r][c]=val;
	}
	inline void get(unsigned r, unsigned c, T &val) const
	{
		if(r<4 && c<4)
			val=elem[r][c];
	}
	// 以下的运算符重载使得存取元素更方便
	// 由于不作任何检查，小心内存越界
	inline T operator () (unsigned r, unsigned c) const
	{
		assert(r<4 && c<4);
		return elem[r][c];
	}
	inline T& operator () (unsigned r, unsigned c)
	{
		assert(r<4 && c<4);
		return elem[r][c];
	}
	// 设置行/列向量
	template<int D>
	inline void set_row(unsigned r, const xvector<T,D> &v)
	{
		assert(r<4);
		assert(D>3);
		T *row=elem[r];
		row[0]=v[0];
		row[1]=v[1];
		row[2]=v[2];
		row[3]=v[3];
	}
	template<int D>
	inline void set_col(unsigned c, const xvector<T,D> &v)
	{
		assert(c<4);
		assert(D>3);
		elem[0][c]=v[0];
		elem[1][c]=v[1];
		elem[2][c]=v[2];
		elem[3][c]=v[3];
	}
	inline void set_row(unsigned r, const xvector<T,2> &v)
	{
		assert(r<4);
		elem[r][0]=v.x;
		elem[r][1]=v.y;
	}
	inline void set_col(unsigned c, const xvector<T,2> &v)
	{
		assert(c<4);
		elem[0][c]=v.x;
		elem[1][c]=v.y;
	}
	inline void set_row(unsigned r, const xvector<T,3> &v)
	{
		assert(r<4);
		T *row=elem[r];
		row[0]=v.x;
		row[1]=v.y;
		row[2]=v.z;
	}
	inline void set_col(unsigned c, const xvector<T,3> &v)
	{
		assert(c<4);
		elem[0][c]=v.x;
		elem[1][c]=v.y;
		elem[2][c]=v.z;
	}
	inline void get_row(unsigned r, xvector<T,4> &v) const 
	{
		assert(r<4);
		const T *row=elem[r];
		v.set(row[0],row[1],row[2],row[3]);
	}
	inline void get_col(unsigned c, xvector<T,4> &v) const
	{
		assert(c<4);
		v.set(elem[0][c],elem[1][c],elem[2][c],elem[3][c]);
	}
	//
	// 矩阵的基本运算
	//
	inline xmatrix& operator += (const xmatrix &m)
	{
		return this->add(m);
	}
	inline xmatrix& operator -= (const xmatrix &m)
	{
		return this->sub(m);
	}
	inline xmatrix& operator *= (T val)
	{
		return this->scale(val);
	}
	inline xmatrix& operator /= (T val)
	{
		return this->div(val);
	}
	inline xmatrix& operator *= (const xmatrix &m) 
	{
		return this->mul(m);
	}
	xmatrix& add (const xmatrix &m)
	{
		m00+=m.m00; m01+=m.m01; m02+=m.m02; m03+=m.m03;
		m10+=m.m10; m11+=m.m11; m12+=m.m12; m13+=m.m13;
		m20+=m.m20; m21+=m.m21; m22+=m.m22; m23+=m.m23;
		m30+=m.m30; m31+=m.m31; m32+=m.m32; m33+=m.m33;
		return *this;
	}
	xmatrix& add (const xmatrix &m1, const xmatrix &m2)
	{
		m00=m1.m00+m2.m00; m01=m1.m01+m2.m01; m02=m1.m02+m2.m02; m03=m1.m03+m2.m03;
		m10=m1.m10+m2.m10; m11=m1.m11+m2.m11; m12=m1.m12+m2.m12; m13=m1.m13+m2.m13;
		m20=m1.m20+m2.m20; m21=m1.m21+m2.m21; m22=m1.m22+m2.m22; m23=m1.m23+m2.m23;
		m30=m1.m30+m2.m30; m31=m1.m31+m2.m31; m32=m1.m32+m2.m32; m33=m1.m33+m2.m33;
		return *this;
	}
	xmatrix& sub (const xmatrix &m)
	{
		m00-=m.m00; m01-=m.m01; m02-=m.m02; m03-=m.m03;
		m10-=m.m10; m11-=m.m11; m12-=m.m12; m13-=m.m13;
		m20-=m.m20; m21-=m.m21; m22-=m.m22; m23-=m.m23;
		m30-=m.m30; m31-=m.m31; m32-=m.m32; m33-=m.m33;
		return *this;
	}
	xmatrix& sub (const xmatrix &m1, const xmatrix &m2)
	{
		m00=m1.m00-m2.m00; m01=m1.m01-m2.m01; m02=m1.m02-m2.m02; m03=m1.m03-m2.m03;
		m10=m1.m10-m2.m10; m11=m1.m11-m2.m11; m12=m1.m12-m2.m12; m13=m1.m13-m2.m13;
		m20=m1.m20-m2.m20; m21=m1.m21-m2.m21; m22=m1.m22-m2.m22; m23=m1.m23-m2.m23;
		m30=m1.m30-m2.m30; m31=m1.m31-m2.m31; m32=m1.m32-m2.m32; m33=m1.m33-m2.m33;
		return *this;
	}
	xmatrix& scale (T val)
	{
		m00*=val; m01*=val; m02*=val; m03*=val;
		m10*=val; m11*=val; m12*=val; m13*=val;
		m20*=val; m21*=val; m22*=val; m23*=val;
		m30*=val; m31*=val; m32*=val; m33*=val;
		return *this;
	}
	xmatrix& scale (const xmatrix &m, T val) 
	{
		m00=m.m00*val; m01=m.m01*val; m02=m.m02*val; m03=m.m03*val;
		m10=m.m10*val; m11=m.m11*val; m12=m.m12*val; m13=m.m13*val;
		m20=m.m20*val; m21=m.m21*val; m22=m.m22*val; m23=m.m23*val;
		m30=m.m30*val; m31=m.m31*val; m32=m.m32*val; m33=m.m33*val;
		return *this;
	}
	inline xmatrix& div (T val)
	{
		T inv=T(1)/val;
		return scale(inv);
	}
	inline xmatrix& div (const xmatrix &m, T val) 
	{
		T inv=T(1)/val;
		return scale(m,inv);
	}
	xmatrix& mul (const xmatrix &mat)
	{
		T temp[4];
		temp[0]=m00*mat.m00+m01*mat.m10+m02*mat.m20+m03*mat.m30;
		temp[1]=m00*mat.m01+m01*mat.m11+m02*mat.m21+m03*mat.m31;
		temp[2]=m00*mat.m02+m01*mat.m12+m02*mat.m22+m03*mat.m32;
		temp[3]=m00*mat.m03+m01*mat.m13+m02*mat.m23+m03*mat.m33;
		m00=temp[0];
		m01=temp[1];
		m02=temp[2];
		m03=temp[3];
		temp[0]=m10*mat.m00+m11*mat.m10+m12*mat.m20+m13*mat.m30;
		temp[1]=m10*mat.m01+m11*mat.m11+m12*mat.m21+m13*mat.m31;
		temp[2]=m10*mat.m02+m11*mat.m12+m12*mat.m22+m13*mat.m32;
		temp[3]=m10*mat.m03+m11*mat.m13+m12*mat.m23+m13*mat.m33;
		m10=temp[0];
		m11=temp[1];
		m12=temp[2];
		m13=temp[3];
		temp[0]=m20*mat.m00+m21*mat.m10+m22*mat.m20+m23*mat.m30;
		temp[1]=m20*mat.m01+m21*mat.m11+m22*mat.m21+m23*mat.m31;
		temp[2]=m20*mat.m02+m21*mat.m12+m22*mat.m22+m23*mat.m32;
		temp[3]=m20*mat.m03+m21*mat.m13+m22*mat.m23+m23*mat.m33;
		m20=temp[0];
		m21=temp[1];
		m22=temp[2];
		m23=temp[3];
		temp[0]=m30*mat.m00+m31*mat.m10+m32*mat.m20+m33*mat.m30;
		temp[1]=m30*mat.m01+m31*mat.m11+m32*mat.m21+m33*mat.m31;
		temp[2]=m30*mat.m02+m31*mat.m12+m32*mat.m22+m33*mat.m32;
		temp[3]=m30*mat.m03+m31*mat.m13+m32*mat.m23+m33*mat.m33;
		m30=temp[0];
		m31=temp[1];
		m32=temp[2];
		m33=temp[3];
		return *this;
	}
	xmatrix& mul (const xmatrix &m1, const xmatrix &m2)
	{
		m00=m1.m00*m2.m00+m1.m01*m2.m10+m1.m02*m2.m20+m1.m03*m2.m30;
		m01=m1.m00*m2.m01+m1.m01*m2.m11+m1.m02*m2.m21+m1.m03*m2.m31;
		m02=m1.m00*m2.m02+m1.m01*m2.m12+m1.m02*m2.m22+m1.m03*m2.m32;
		m03=m1.m00*m2.m03+m1.m01*m2.m13+m1.m02*m2.m23+m1.m03*m2.m33;
		m10=m1.m10*m2.m00+m1.m11*m2.m10+m1.m12*m2.m20+m1.m13*m2.m30;
		m11=m1.m10*m2.m01+m1.m11*m2.m11+m1.m12*m2.m21+m1.m13*m2.m31;
		m12=m1.m10*m2.m02+m1.m11*m2.m12+m1.m12*m2.m22+m1.m13*m2.m32;
		m13=m1.m10*m2.m03+m1.m11*m2.m13+m1.m12*m2.m23+m1.m13*m2.m33;
		m20=m1.m20*m2.m00+m1.m21*m2.m10+m1.m22*m2.m20+m1.m23*m2.m30;
		m21=m1.m20*m2.m01+m1.m21*m2.m11+m1.m22*m2.m21+m1.m23*m2.m31;
		m22=m1.m20*m2.m02+m1.m21*m2.m12+m1.m22*m2.m22+m1.m23*m2.m32;
		m23=m1.m20*m2.m03+m1.m21*m2.m13+m1.m22*m2.m23+m1.m23*m2.m33;
		m30=m1.m30*m2.m00+m1.m31*m2.m10+m1.m32*m2.m20+m1.m33*m2.m30;
		m31=m1.m30*m2.m01+m1.m31*m2.m11+m1.m32*m2.m21+m1.m33*m2.m31;
		m32=m1.m30*m2.m02+m1.m31*m2.m12+m1.m32*m2.m22+m1.m33*m2.m32;
		m33=m1.m30*m2.m03+m1.m31*m2.m13+m1.m32*m2.m23+m1.m33*m2.m33;
		return *this;
	}
	xmatrix& transpose() 
	{
		SWAP(m01,m10); 
		SWAP(m02,m20);
		SWAP(m03,m30);
		SWAP(m12,m21);
		SWAP(m13,m31);
		SWAP(m23,m32);
		return *this;
	}
	xmatrix& transpose (const xmatrix &m)
	{
		m00=m.m00; m01=m.m10; m02=m.m20; m03=m.m30;
		m10=m.m01; m11=m.m11; m12=m.m21; m13=m.m31;
		m20=m.m02; m21=m.m12; m22=m.m22; m23=m.m32;
		m30=m.m03; m31=m.m13; m32=m.m23; m33=m.m33;
		return *this;
	}
	T det () const
	{
		xmatrix<T,3,3> sub;
		T v, r=0;
		const T *row=elem[0];
		for(unsigned i=0;i<4;++i)
		{
			_get_sub_matrix(sub,i);
			v=(i&1) ? (-row[i]) : (row[i]);
			v*=sub.det();
			r+=v;
		}
		return r;
	}
#ifdef FAST_MATRIX4_INVERSE
	// Strassen's Method
	// 速度很快，但舍入误差会被累积，导致结果精度降低
	bool inverse (const xmatrix &m)
	{
		// r0=inverse(a00)
		double d=m.m00*m.m11-m.m01*m.m10;
		if(fequal(d,0))
			return false;
		d=double(1)/d;
		double r0[4]={
			 d*m.m11, -d*m.m01,
			-d*m.m10,  d*m.m00,
		};
		// r1=a10*r0
		double r1[4]={
			m.m20*r0[0]+m.m21*r0[2], m.m20*r0[1]+m.m21*r0[3],
			m.m30*r0[0]+m.m31*r0[2], m.m30*r0[1]+m.m31*r0[3],
		};
		// r2=r0*a01
		double r2[4]={
			m.m02*r0[0]+m.m12*r0[1], m.m03*r0[0]+m.m13*r0[1],
			m.m02*r0[2]+m.m12*r0[3], m.m03*r0[2]+m.m13*r0[3],
		};
		// r3=a10*r2
		double r3[4]={
			m.m20*r2[0]+m.m21*r2[2], m.m20*r2[1]+m.m21*r2[3],
			m.m30*r2[0]+m.m31*r2[2], m.m30*r2[1]+m.m31*r2[3],
		};
		// r4=r3-a11
		r3[0]-=m.m22, r3[1]-=m.m23,
		r3[2]-=m.m32, r3[3]-=m.m33,
		// r5=inverse(r4)
		d=r3[0]*r3[3]-r3[1]*r3[2];
		if(fequal(d,0))
			return false;
		d=1/d;
		double tmp=r3[0];
		r3[0]=r3[3]*d;
		r3[1]*=-d;
		r3[2]*=-d;
		r3[3]=tmp*d;
		// c01=r2*r5
		m02=T(r2[0]*r3[0]+r2[1]*r3[2]); m03=T(r2[0]*r3[1]+r2[1]*r3[3]);
		m12=T(r2[2]*r3[0]+r2[3]*r3[2]); m13=T(r2[2]*r3[1]+r2[3]*r3[3]);
		// c10=r5*r1
		m20=T(r3[0]*r1[0]+r3[1]*r1[2]); m21=T(r3[0]*r1[1]+r3[1]*r1[3]);
		m30=T(r3[2]*r1[0]+r3[3]*r1[2]); m31=T(r3[2]*r1[1]+r3[3]*r1[3]);
		// c11=-r5
		m22=T(-r3[0]); m23=T(-r3[1]);
		m32=T(-r3[2]); m33=T(-r3[3]);
		// r6=r2*c10
		r3[0]=r2[0]*m20+r2[1]*m30; r3[1]=r2[0]*m21+r2[1]*m31;
		r3[2]=r2[2]*m20+r2[3]*m30; r3[3]=r2[2]*m21+r2[3]*m31;
		// c00=r0-r6
		m00=T(r0[0]-r3[0]); m01=T(r0[1]-r3[1]);
		m10=T(r0[2]-r3[2]); m11=T(r0[3]-r3[3]);
		return true;
	}
#else
	bool inverse (const xmatrix &m)
	{
		T adj[16], *iter;
		double d=0;
		iter=adj;
		xmatrix<T,3,3> sub;
		const T *src=m.elem[0];
		int i, j;
		// 计算行列式
		for(i=0; i<4; ++i)
		{
			m._get_sub_matrix(sub,i);
			*iter=sub.det();
			if(i&1) 
				*iter=-*iter;
			d+=src[i]*(*iter);
			++iter;
		}
		if(fequal(d,0))
			return false;
		// 计算伴随矩阵
		for(i=1; i<4; ++i) {
			for(j=0; j<4; ++j) {
				m._get_sub_matrix(sub,i,j);
				*iter=sub.det();
				if((i+j)&1)
					*iter=-*iter;
				++iter;
			}
		}
		// 计算逆阵
		iter=adj;
		double invdet=1/d;
		for(i=0; i<4; ++i) {
			for(j=0; j<4; ++j) 
				elem[j][i]=T((*iter++)*invdet);
		}
		return true;
	}
#endif // FAST_MATRIX4_INVERSE
	bool operator == (const xmatrix &m) const
	{
		const T *iter1=elem[0], *iter2=m.elem[0];
		for(int i=16; i>0; --i) {
			if(*iter1!=*iter2)
				return false;
			++iter1;
			++iter2;
		}
		return true;
	}
	bool operator != (const xmatrix &m) const
	{
		const T *iter1=elem[0], *iter2=m.elem[0];
		for(int i=16; i>0; --i) {
			if(*iter1!=*iter2)
				return true;
			++iter1;
			++iter2;
		}
		return false;
	}
private:
	// 获取去除0行rc列后的子阵
	void _get_sub_matrix (xmatrix<T,3,3> &m, int rc) const
	{
		T *dst=m.elem[0];
		const T *src=elem[1];;
		unsigned i,j;
		for(i=1;i<4;++i)
		{
			for(j=0;j<4;++j)
			{
				if(j!=rc)
					*dst++=*src;
				++src;
			}
		}
	}
	// 获取去除第rr行rc列后的子阵
	void _get_sub_matrix (xmatrix<T,3,3> &m, int rr, int rc) const
	{
		T *dst=m.elem[0];
		const T *src;
		unsigned i,j;
		for(i=0;i<4;++i)
		{
			if(i==rr)
				continue;
			src=elem[i];
			for(j=0;j<4;++j)
			{
				if(j==rc)
					continue;
				*dst++=src[j];
			}
		}
	}
};

/////////////////////////////////////////////////////////////////////////////////////
//
// 矩阵运算符重载
//
template<typename T, int R, int C>
inline xmatrix<T,R,C> operator + (const xmatrix<T,R,C> &m1, const xmatrix<T,R,C> &m2)
{
	xmatrix<T,R,C> ret;
	ret.add(m1,m2);
	return ret;
}

template<typename T, int R, int C>
inline xmatrix<T,R,C> operator - (const xmatrix<T,R,C> &m1, const xmatrix<T,R,C> &m2)
{
	xmatrix<T,R,C> ret;
	ret.sub(m1,m2);
	return ret;
}

template<typename T, int R, int C>
inline xmatrix<T,R,C> operator * (const xmatrix<T,R,C> &m1, const xmatrix<T,R,C> &m2)
{
	xmatrix<T,R,C> ret;
	ret.mul(m1,m2);
	return ret;
}

template<typename T, int R, int C>
inline xmatrix<T,R,C> operator * (const xmatrix<T,R,C> &m1, T val)
{
	xmatrix<T,R,C> ret;
	ret.scale(m1,val);
	return ret;
}

template<typename T, int R, int C>
inline xmatrix<T,R,C> operator * (T val, const xmatrix<T,R,C> &m1)
{
	xmatrix<T,R,C> ret;
	ret.scale(m1,val);
	return ret;
}

template<typename T, int R, int C>
inline xmatrix<T,R,C> operator / (const xmatrix<T,R,C> &m1, T val)
{
	xmatrix<T,R,C> ret;
	ret.div(m1,val);
	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////
//
// 向量与矩阵之间的运算
//

// 向量与矩阵的乘法
template<class T, int R, int C>
void vec_mul_mat(const xvector<T,R> &v, const xmatrix<T,R,C> &m, xvector<T,C> &r)
{
	T sum;
	for(int j=0; j<C; ++j)
	{
		sum=0;
		for(int i=0; i<R; ++i)
			sum+=v(i)*m(i,j);
		r(j)=sum;
	}
}

template<class T, int R, int C>
void vec_mul_mat(const xmatrix<T,R,C> &m, const xvector<T,C> &v, xvector<T,C> &r)
{
	T sum;
	for(int i=0; i<R; ++i)
	{
		sum=0;
		for(int j=0; j<C; ++j)
			sum+=v(j)*m(i,j);
		r(i)=sum;
	}
}

template<class T, int R, int C>
inline xvector<T,C> operator * (const xvector<T,R> &v, const xmatrix<T,R,C> &m)
{
	xvector<T,C> ret;
	vec_mul_mat(v,m,ret);
	return ret;
}

template<class T, int R, int C>
inline xvector<T,C> operator * (const xmatrix<T,R,C> &m, const xvector<T,C> &v)
{
	xvector<T,C> ret;
	vec_mul_mat(m,v,ret);
	return ret;
}

// 2D向量和矩阵相乘
template<class T>
xvector<T,2> operator * (const xvector<T,2> &v, const xmatrix<T,2,2> &m)
{
	xvector<T,2> r;
	r.x=v.x*m.m00+v.y*m.m10;
	r.y=v.x*m.m01+v.y*m.m11;
	return r;
}

template<class T>
xvector<T,2> operator * (const xmatrix<T,2,2> &m, const xvector<T,2> &v)
{
	xvector<T,2> r;
	r.x=v.x*m.m00+v.y*m.m01;
	r.y=v.x*m.m10+v.y*m.m11;
	return r;
}

template<class T>
xvector<T,2> operator * (const xvector<T,2> &v, const xmatrix<T,3,3> &m)
{
	xvector<T,2> r;
	r.x=v.x*m.m00+v.y*m.m10;
	r.y=v.x*m.m01+v.y*m.m11;
	return r;
}

template<class T>
xvector<T,2> operator * (const xmatrix<T,3,3> &m, const xvector<T,2> &v)
{
	xvector<T,2> r;
	r.x=v.x*m.m00+v.y*m.m01;
	r.y=v.x*m.m10+v.y*m.m11;
	return r;
}

// 3D向量和矩阵相乘
template<class T>
xvector<T,3> operator * (const xvector<T,3> &v, const xmatrix<T,3,3> &m)
{
	xvector<T,3> r;
	r.x=v.x*m.m00+v.y*m.m10+v.z*m.m20;
	r.y=v.x*m.m01+v.y*m.m11+v.z*m.m21;
	r.z=v.x*m.m02+v.y*m.m12+v.z*m.m22;
	return r;
}

template<class T>
xvector<T,3> operator * (const xmatrix<T,3,3> &m, const xvector<T,3> &v)
{
	xvector<T,3> r;
	r.x=v.x*m.m00+v.y*m.m01+v.z*m.m02;
	r.y=v.x*m.m10+v.y*m.m11+v.z*m.m12;
	r.z=v.x*m.m20+v.y*m.m21+v.z*m.m22;
	return r;
}

template<class T>
xvector<T,3> operator * (const xvector<T,3> &v, const xmatrix<T,4,4> &m)
{
	xvector<T,3> r;
	r.x=v.x*m(0,0)+v.y*m(1,0)+v.z*m(2,0);
	r.y=v.x*m(0,1)+v.y*m(1,1)+v.z*m(2,1);
	r.z=v.x*m(0,2)+v.y*m(1,2)+v.z*m(2,2);
	return r;
}

template<class T>
xvector<T,3> operator * (const xmatrix<T,4,4> &m, const xvector<T,3> &v)
{
	xvector<T,3> r;
	r.x=v.x*m(0,0)+v.y*m(0,1)+v.z*m(0,2);
	r.y=v.x*m(1,0)+v.y*m(1,1)+v.z*m(1,2);
	r.z=v.x*m(2,0)+v.y*m(2,1)+v.z*m(2,2);
	return r;
}

// 4D向量与矩阵相乘
template<class T>
void vec_mul_mat(const xvector<T,4> &v, const xmatrix<T,4,4> &m, xvector<T,4> &r)
{
	r.x=v.x*m(0,0)+v.y*m(1,0)+v.z*m(2,0)+v.w*m(3,0);
	r.y=v.x*m(0,1)+v.y*m(1,1)+v.z*m(2,1)+v.w*m(3,1);
	r.z=v.x*m(0,2)+v.y*m(1,2)+v.z*m(2,2)+v.w*m(3,2);
	r.w=v.x*m(0,3)+v.y*m(1,3)+v.z*m(2,3)+v.w*m(3,3);
}

template<class T>
void vec_mul_mat(const xmatrix<T,4,4> &m, const xvector<T,4> &v, xvector<T,4> &r)
{
	r.x=v.x*m(0,0)+v.y*m(0,1)+v.z*m(0,2)+v.w*m(0,3);
	r.y=v.x*m(1,0)+v.y*m(1,1)+v.z*m(1,2)+v.w*m(1,3);
	r.z=v.x*m(2,0)+v.y*m(2,1)+v.z*m(2,2)+v.w*m(2,3);
	r.w=v.x*m(3,0)+v.y*m(3,1)+v.z*m(3,2)+v.w*m(3,3);
}

template<class T>
inline xvector<T,4> operator * (const xvector<T,4> &v, const xmatrix<T,4,4> &m)
{
	xvector<T,4> r;
	vec_mul_mat(v,m,r);
	return r;
}

template<class T>
inline xvector<T,4> operator * (const xmatrix<T,4,4> &m, const xvector<T,4> &v)
{
	xvector<T,4> r;
	vec_mul_mat(m,v,r);
	return r;
}

///////////////////////////////////////////////////////////////////////////////////
//
// 四元数
//

template<class T>
class xquaternion
{
//
// 数据
//
public:
	union
	{
		T	elem[4];
		struct
		{
			T	qa;
			xvector<T,3> qv;
		};
		struct
		{
			T	w,x,y,z;
		};
	};
//
// 构造/析构函数
//
public:
	xquaternion () {}

	~xquaternion () {}

	// COPY构造函数
	xquaternion (const xquaternion<T> &q)
	{
		*this=q;
	}
	xquaternion& operator = (const xquaternion<T> &q)
	{
		qa=q.qa;
		qv=q.qv;
		return *this;
	}

	// 根据旋转向量和旋转角构建四元数
	xquaternion (const xvector<T,3> &v, T theta)
	{
		from_vec_ang(v,theta);
	}

	// 与3D向量间的转换
	xquaternion (const xvector<T,3> &v):
		qa(0), qv(v)
	{}
	inline xquaternion& operator = (const xvector<T,3> &v)
	{
		qa = 0;
		qv = v;
		return *this;
	}

//
// 外部接口
//
public:
	//
	// 四元数基本运算
	//
	inline xquaternion& operator += (const xquaternion &q)
	{
		return add(q);
	}
	inline xquaternion& operator -= (const xquaternion &q)
	{
		return sub(q);
	}
	// 四元数加法
	inline xquaternion& add (const xquaternion &q)
	{
		w += q.w;
		x += q.x;
		y += q.y;
		z += q.z;
		return *this;
	}
	inline xquaternion& add (const xquaternion &q1, const xquaternion &q2)
	{
		w = q1.w + q2.w;
		x = q1.x + q2.x;
		y = q1.y + q2.y;
		z = q1.z + q2.z;
		return *this;
	}
	// 四元数减法
	inline xquaternion& sub (const xquaternion &q)
	{
		w -= q.w;
		x -= q.x;
		y -= q.y;
		z -= q.z;
		return *this;
	}
	inline xquaternion& sub (const xquaternion &q1, const xquaternion &q2)
	{
		w = q1.w - q2.w;
		x = q1.x - q2.x;
		y = q1.y - q2.y;
		z = q1.z - q2.z;
		return *this;
	}
	// 标量乘法
	inline xquaternion& scale (T val)
	{
		w *= val;
		x *= val;
		y *= val;
		z *= val;
		return *this;
	}
	// 点积
	inline T dot (const xquaternion &q)
	{
		return (w*q.w + x*q.x + y*q.y + z*q.z);
	}
	// 四元数乘法，this=q1*q2
	xquaternion& mul (const xquaternion &q1, const xquaternion &q2)
	{
		//
		// 公式：[ w1*w2 - (v1 dot v2), w1*v2 + w2*v1 + (v1 cross v2) ]
		//
		// 直接计算，16次乘法，12次加法
		/*
		w = q1.w*q2.w - q1.x*q2.x - q1.y*q2.y - q1.z*q2.z;
		x = q1.w*q2.x + q1.x*q2.w + q1.y*q2.z - q1.z*q2.y;
		y = q1.w*q2.y - q1.x*q2.z + q1.y*q2.w + q1.z*q2.x;
		z = q1.w*q2.z + q1.x*q2.y - q1.y*q2.x + q1.z*q2.w;
		*/
		
		// 通过提取公共因子的改进算法，9次乘法，27次加法

		T a0 = (q1.z - q1.y) * (q2.y - q2.z);
		T a1 = (q1.w + q1.x) * (q2.w + q2.x);
		T a2 = (q1.w - q1.x) * (q2.y + q2.z);
		T a3 = (q1.y + q1.z) * (q2.w - q2.x);
		T a4 = (q1.z - q1.x) * (q2.x - q2.y);
		T a5 = (q1.z + q1.x) * (q2.x + q2.y);
		T a6 = (q1.w + q1.y) * (q2.w - q2.z);
		T a7 = (q1.w - q1.y) * (q2.w + q2.z);

		T a8 = a5 + a6 + a7;
		T a9 = T(0.5 * (a4 + a8));

		w = a0 + a9 - a5;
		x = a1 + a9 - a8;
		y = a2 + a9 - a7;
		z = a3 + a9 - a6;
		
		return *this;
	}
	// 求q1到q2的角度变换，结果保存在this中
	xquaternion& differ (const xquaternion &q1, const xquaternion &q2)
	{
		xquaternion tmp=q1;
		tmp.inverse();
		mul(tmp,q2);
		return *this;
	}
	// 四元数的值
	inline T magnitude () const
	{
		return (sqrt(magnitude2()));
	}
	// 值的平方
	inline T magnitude2 () const
	{
		return (w*w + x*x + y*y + z*z);
	}
	// 将this设为它的共轭
	inline xquaternion& conjugate ()
	{
		x = -x;
		y = -y;
		z = -z;
		return *this;
	}
	// 将this设为它的逆
	xquaternion& inverse ()
	{
		T inv = magnitude();
		inv = 1/inv;
		conjugate().scale(inv);
		return *this;
	}
	// 单位四元数的逆
	inline xquaternion& unit_inverse()
	{
		conjugate();
	}
	// log(this) logarithm
	xquaternion& log (const xquaternion &q)
	{
		T a = acos(q.w);
		w = 0;
		x = y = z = a;
		return *this;
	}
	// e^this exponent
	xquaternion& exp  (const xquaternion &q)
	{
		T a = q.x;
		w = cos(a);
		x = y = z = sin(a);
		return *this;
	}
	// this^(t) power
	inline xquaternion& pow (T t)
	{
		return (log(*this).scale(t).exp (*this));
	}
	// 根据转轴和转角构造四元数
	// 参数normalized指出传入的向量是否单位向量
	void from_vec_ang (const xvector<T,3> &v, T angle, bool normalized = true)
	{
		angle /= 2;
		T sina;		
		if(!normalized)
		{
			// 如果不是单位向量，先要将其规整为单位向量
			T len = v.length();
			len = 1/len;
			sina = sin(angle) * len;
		}
		else
		{
			sina = sin(angle);
		}
		w = cos(angle);
		x = v.x * sina;
		y = v.y * sina;
		z = v.z * sina;
	}
	// 从四元数中提取转轴和转角
	void to_vec_ang (xvector<T,3> &v, T &angle) const
	{
		angle = acos(w);
		T inv_sina = sin(angle);
		if(fequal(inv_sina,T(0)))
		{
			v.x = v.y = v.z = 0;
		}
		else
		{
			inv_sina = 1/inv_sina;
			v.x = x * inv_sina;
			v.y = y * inv_sina;
			v.z = z * inv_sina;
		}
		angle *= 2;
	}
};

///////////////////////////////////////////////////////////////////////////////////
//
// 四元数运算符重载
//
template<class T>
inline xquaternion<T> operator + (const xquaternion<T> &q1, const xquaternion<T> &q2)
{
	xquaternion q;
	return q.add(q1,q2);
}

template<class T>
inline xquaternion<T> operator - (const xquaternion<T> &q1, const xquaternion<T> &q2)
{
	xquaternion q;
	return q.sub(q1,q2);
}

template<class T>
inline xquaternion<T> operator * (const xquaternion<T> &q1, const xquaternion<T> &q2)
{
	xquaternion q;
	return q.mul(q1,q2);
}

template<class T>
inline xquaternion<T> operator / (const xquaternion<T> &q1, const xquaternion<T> &q2)
{
	xquaternion q;
	return q.differ(q1,q2);
}

////////////////////////////////////////////////////////////////////////////////////////
//
// 坐标转换函数
//

// 2D极坐标
template<class T>
struct xpolar
{
	T	r;			// 半径
	T	theta;		// 角度
};

// 3D柱面坐标
template<class T>
struct xcylin
{
	T	r;			// 半径
	T	theta;		// 角度
	T	z;			// Z坐标
};

// 3D球坐标
template<class T>
struct xspherical
{
	T	r;			// 半径
	union {
		// 经度（向量在X-Y面上的投影与正X轴间的夹角）
		T	theta;
		T	longitude;
	};
	union {
		// 纬度（向量与正Z轴的夹角）
		T	phi;
		T	latitude;
	};
};

//
// 2D笛卡尔坐标与极坐标之间的转换
//
template<class T>
inline void to_cartesian  (const xpolar<T> &p, xvector<T,2> &v)
{
	v.x = p.r * cos(p.theta);
	v.y = p.r * sin(p.theta);
}

template<class T>
inline void to_polar (const xvector<T,2> &v, xpolar<T> &p)
{
	p.r = v.length();
	p.theta = atan2(v.y, v.x);
}

//
// 3D笛卡尔坐标与柱面坐标之间的转换
//
template<class T>
inline void to_cartesian (const xcylin<T> &c, xvector<T,3> &v)
{
	v.x = c.r * cos(c.theta);
	v.y = c.r * sin(c.theta);
	v.z = c.z;
}

template<class T>
inline void to_cylin (const xvector<T,3> &v, xcylin<T> &c)
{
	c.r = sqrt(v.x*v.x + v.y*v.y);
	c.theta = atan2(v.y,v.x);
	c.z = v.z;
}

//
// 3D笛卡尔坐标与球坐标之间的转换
//
template<class T>
void to_cartesian (const xspherical<T> &s, xvector<T,3> &v)
{
	T tmp = s.r * sin(s.phi);
	v.x = tmp * cos(s.theta);
	v.y = tmp * sin(s.theta);
	v.z = s.r * cos(s.phi);
}

template<class T>
void to_spherical (const  xvector<T,3> &v, xspherical<T> &s)
{
	s.r = v.length();
	s.theta = atan2(v.y, v.x);
	s.phi = asin(v.x/(s.r*cos(s.theta)));
}

///////////////////////////////////////////////////////////////////////////////
//
// 角度变换函数
//

//
// 欧拉角，默认使用roll-pitch-yaw（Z-X-Y）的旋转顺序
//
template<class T>
struct xeuler
{
	union
	{
		T elem[3];
		struct
		{
			T x, y, z;
		};
		struct
		{
			T pitch, yaw, roll;
		};
	};
};

//
// 欧拉角和旋转矩阵之间的转换
//
template<class T, int D>
void euler_to_matrix (const xeuler<T> &euler, xmatrix<T,D,D> &mat)
{
	T sinr = sin(euler.roll);
	T sinp = sin(euler.pitch);
	T siny = sin(euler.yaw);
	
	T cosr = cos(euler.roll);
	T cosp = cos(euler.pitch);
	T cosy = cos(euler.yaw);

	T cosy_cosr = cosy * cosr;
	T cosy_sinr = cosy * sinr;
	T siny_cosr = siny * cosr;
	T siny_sinr = siny * sinr;

	mat(0,0) = cosy_cosr + siny_sinr * sinp;
	mat(0,1) = sinr * cosp;
	mat(0,2) = cosy_sinr * sinp - siny_cosr;
	mat(1,0) = siny_cosr * sinp - cosy_sinr;
	mat(1,1) = cosr * cosp;
	mat(1,2) = siny_sinr + cosy_cosr * sinp;
	mat(2,0) = siny * cosp;
	mat(2,1) = - sinp;
	mat(2,2) = cosy * cosp;
}

template<class T, int D>
void matrix_to_euler (const xmatrix<T,D,D> &mat, xeuler<T> &euler)
{
	T M21 = mat(2,1);
	if (M21>=1.0)
		euler.pitch = PI_DIV_2;
	else if (M21<=-1.0)
		euler.pitch = -PI_DIV_2;
	else
		euler.pitch = asin(-mat(2,1));
	
	if (M21>0.99999)
	{
		// gimbal lock
		euler.roll = 0;
		euler.yaw = asin(-mat(0,2));
	}
	else
	{
		euler.roll = atan2(mat(0,1), mat(1,1));
		euler.yaw = atan2(mat(2,0), mat(2,2));
	}
}

//
// 四元数和旋转矩阵之间的转换
//
template<class T, int D>
void quat_to_matrix (const xquaternion<T> &quat, xmatrix<T,D,D> &mat)
{
	// 使用临时变量来减少乘法
	T xx = quat.x * quat.x;
	T yy = quat.y * quat.y;
	T zz = quat.z * quat.z;
	T ww = quat.w * quat.w;

	T xy = quat.x * quat.y;
	T xz = quat.x * quat.z;
	T xw = quat.x * quat.w;
	T yz = quat.y * quat.z;
	T yw = quat.y * quat.w;
	T zw = quat.z * quat.w;

	mat(0,0) = 1 - 2 * (yy+zz);
	mat(1,1) = 1 - 2 * (xx+zz);
	mat(2,2) = 1 - 2 * (xx+yy);
	
	mat(0,1) = 2 * (xy+zw);
	mat(0,2) = 2 * (xz-yw);
	mat(1,0) = 2 * (xy-zw);
	mat(1,2) = 2 * (yz+xw);
	mat(2,0) = 2 * (yw+xz);
	mat(2,1) = 2 * (yz-xw);
}

template<class T, int D>
void matrix_to_quat (const xmatrix<T,D,D> &mat, xquaternion<T> &quat)
{
	// 确定最大的分量
	T max_w = mat(0,0) + mat(1,1) + mat(2,2);
	T max_x = mat(0,0) - mat(1,1) - mat(2,2);
	T max_y = mat(1,1) - mat(2,2) - mat(0,0);
	T max_z = mat(2,2) - mat(0,0) - mat(1,1);
	int id=0;
	T max=max_w;
	if(max_x>max)
	{
		max=max_x;
		id=1;
	}
	if(max_y>max)
	{
		max=max_y;
		id=2;
	}
	if(max_z>max)
	{
		max=max_z;
		id=3;
	}
	// 计算四元数的值
	max = T(sqrt(max+1)*0.5);
	T mult = T(0.25/max);
	switch(id)
	{
	case 0:
		quat.w = max;
		quat.x = (mat(1,2) - mat(2,1)) * mult;
		quat.y = (mat(2,0) - mat(0,2)) * mult;
		quat.z = (mat(0,1) - mat(1,0)) * mult;
		break;
	case 1:
		quat.x = max;
		quat.w = (mat(1,2) - mat(2,1)) * mult;
		quat.y = (mat(0,1) + mat(1,0)) * mult;
		quat.z = (mat(2,0) + mat(0,2)) * mult;
		break;
	case 2:
		quat.y = max;
		quat.w = (mat(2,0) - mat(0,2)) * mult;
		quat.x = (mat(0,1) + mat(1,0)) * mult;
		quat.z = (mat(1,2) + mat(2,1)) * mult;
		break;
	case 3:
		quat.z = max;
		quat.w = (mat(0,1) - mat(1,0)) * mult;
		quat.x = (mat(2,0) + mat(0,2)) * mult;
		quat.y = (mat(1,2) + mat(2,1)) * mult;
		break;
	}
}

//
// 欧拉角和四元数之间的转换
//
template<class T>
void euler_to_quat (const xeuler<T> &euler, xquaternion<T> &quat)
{
	T r = T(euler.roll * 0.5);
	T p = T(euler.pitch * 0.5);
	T y = T(euler.yaw * 0.5);

	float cosr = cos(r);
	float cosp = cos(p);
	float cosy = cos(y);

	float sinr = sin(r);
	float sinp = sin(p);
	float siny = sin(y);

	quat.w = cosy*cosp*cosr + siny*sinp*sinr;
	quat.x = cosy*sinp*cosr + siny*cosp*sinr;
	quat.y = siny*cosp*cosr - cosy*sinp*sinr;
	quat.z = cosy*cosp*cosr - siny*sinp*cosr;
}

template<class T>
void quat_to_euler (const xquaternion<T> &quat, xeuler<T> &euler)
{
	T tmp = -2 * (quat.y*quat.z - quat.w*quat.x);
	if(abs(tmp)>0.99999f)
	{
		// gimbal lock
		euler.pitch = PI_DIV_2*tmp;
		euler.yaw = atan2(T(-quat.x*quat.z + quat.w*quat.y), T(-quat.y*quat.y - quat.z*quat.z));
		euler.roll = 0;
	}
	else
	{
		T xx = quat.x * quat.x;
		euler.pitch = asin(tmp);
		euler.yaw = atan2(T(quat.x*quat.z + quat.w*quat.y), T(0.5 - xx - quat.y*quat.y));
		euler.roll = atan2(T(quat.x*quat.y + quat.w*quat.z), T(0.5 - xx - quat.z*quat.z));
	}
}

//////////////////////////////////////////////////////////////////////////////////////
//
// 常用变换矩阵(均为行优先矩阵)
//

// 2D平移矩阵
template<class T, int D>
inline void matrix_translate2d(xmatrix<T,D,D> &matrix, T dx, T dy)
{
	matrix.identity();
	matrix(2,0)=dx; 
	matrix(2,1)=dy;
}

// 2D缩放矩阵
template<class T, int D>
inline void matrix_scale2d(xmatrix<T,D,D> &matrix, T sx, T sy)
{
	matrix.identity();
	matrix(0,0)=sx;
	matrix(1,1)=sy;
}

// 2D旋转矩阵,逆时针旋转radian弧度
template<class T, int D>
void matrix_rotate2d(xmatrix<T,D,D> &matrix, T radian)
{
	matrix.identity();
	matrix(0,0)=cos(radian);  matrix(0,1)=sin(radian);
	matrix(1,0)=-sin(radian); matrix(1,1)=cos(radian);
}

// 3D平移矩阵
template<class T, int D>
inline void matrix_translate3d(xmatrix<T,D,D> &matrix, T dx, T dy, T dz)
{
	matrix.identity();
	matrix(3,0)=dx; matrix(3,1)=dy; matrix(3,2)=dz;
}

// 3D缩放矩阵
template<class T, int D>
inline void matrix_scale3d(xmatrix<T,D,D> &matrix, T sx, T sy, T sz)
{
	matrix.identity();
	matrix(0,0)=sx; matrix(1,1)=sy; matrix(2,2)=sz;
}

// 3D旋转矩阵,绕轴n(单位向量)逆时针旋转radian弧度
template<class T, int D>
void matrix_rotate3d(xmatrix<T,D,D> &matrix, const xvector<T,3> &n, T radian)
{
	matrix.identity();
	T sina=sin(radian);
	T cosa=cos(radian);
	T t=1-cosa;
	matrix(0,0)=n.x*n.x*t+cosa; matrix(0,1)=n.x*n.y*t+n.z*sina; matrix(0,2)=n.x*n.z*t-n.y*sina;
	matrix(1,0)=n.x*n.y*t-n.z*sina; matrix(1,1)=n.y*n.y*t+cosa; matrix(1,2)=n.z*n.y*t+n.x*sina;
	matrix(2,0)=n.x*n.z*t+n.y*sina; matrix(2,1)=n.y*n.z*t-n.x*sina; matrix(2,2)=n.z*n.z*t+cosa;
}

// 3D旋转矩阵,绕x轴逆时针旋转radian弧度
template<class T, int D>
void matrix_xrotate3d(xmatrix<T,D,D> &matrix, T radian)
{
	matrix.identity();
	T sina=sin(radian);
	T cosa=cos(radian);
	matrix(1,1)=cosa; matrix(1,2)=sina;
	matrix(2,1)=-sina; matrix(2,2)=cosa;
}

// 3D旋转矩阵,绕y轴逆时针旋转radian弧度
template<class T, int D>
void matrix_yrotate3d(xmatrix<T,D,D> &matrix, T radian)
{
	matrix.identity();
	T sina=sin(radian);
	T cosa=cos(radian);
	matrix(0,0)=cosa; matrix(0,2)=-sina;
	matrix(2,0)=sina; matrix(2,2)=cosa;
}

// 3D旋转矩阵,绕z轴逆时针旋转radian弧度
template<class T, int D>
void matrix_zrotate3d(xmatrix<T,D,D> &matrix, T radian)
{
	matrix.identity();
	T sina=sin(radian);
	T cosa=cos(radian);
	matrix(0,0)=cosa; matrix(0,1)=sina;
	matrix(1,0)=-sina; matrix(1,1)=cosa;
}

//////////////////////////////////////////////////////////////////////////////////////
//
// 类型定义
//

typedef xvector<int,2> xpt2i_t;
typedef xvector<float,2> xpt2f_t;
typedef xvector<double,2> xpt2d_t;
typedef xvector<int,3> xpt3i_t;
typedef xvector<float,3> xpt3f_t;
typedef xvector<double,3> xpt3d_t;
typedef xvector<int,4> xpt4i_t;
typedef xvector<float,4> xpt4f_t;
typedef xvector<double,4> xpt4d_t;

typedef xvector<int,2> xvec2i_t;
typedef xvector<float,2> xvec2f_t;
typedef xvector<double,2> xvec2d_t;
typedef xvector<int,3> xvec3i_t;
typedef xvector<float,3> xvec3f_t;
typedef xvector<double,3> xvec3d_t;
typedef xvector<int,4> xvec4i_t;
typedef xvector<float,4> xvec4f_t;
typedef xvector<double,4> xvec4d_t;

typedef xmatrix<int,2,2> xmat2i_t;
typedef xmatrix<float,2,2> xmat2f_t;
typedef xmatrix<double,2,2>	xmat2d_t;
typedef xmatrix<int,3,3> xmat3i_t;
typedef xmatrix<float,3,3> xmat3f_t;
typedef xmatrix<double,3,3>	xmat3d_t;
typedef xmatrix<int,4,4> xmat4i_t;
typedef xmatrix<float,4,4> xmat4f_t;
typedef xmatrix<double,4,4>	xmat4d_t;

typedef xquaternion<int> xquati_t;
typedef xquaternion<float> xquatf_t;
typedef xquaternion<double>	xquatd_t;

typedef xeuler<int> xeuleri_t;
typedef xeuler<float> xeulerf_t;
typedef xeuler<double> xeulerd_t;

//////////////////////////////////////////////////////////////

} // namespace wyc

#ifdef _MSC_VER
#pragma warning (pop)
#endif // _MSC_VER

#endif // end of __HEADER_WYC_XVECMATH
