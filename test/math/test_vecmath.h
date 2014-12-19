#include <eigen/dense>
#include <armadillo>
#include "wyc/math/vecmath.h"

//--------------------------------------------------
// 矩阵打印
//--------------------------------------------------
template<typename T, int R, int C>
std::ostream& operator << (std::ostream& out, const wyc::xmatrix<T,R,C> &mat)
{
	for(int i=0; i<R; ++i) {
		for(int j=0; j<C; ++j) 
			out<<std::setw(12)<<setiosflags(std::ios::right)<<mat(i,j);
		out<<std::endl;
	}
	return out;
}

//--------------------------------------------------
// 向量赋值
//--------------------------------------------------

template<typename T, int D>
void vector_assign(wyc::xvector<T,D> &v1, const Eigen::Matrix<T,D,1> &v2) 
{
	for(int i=0; i<D; ++i)
		v1[i]=v2[i];
}

template<typename T, int D>
void vector_assign_t(wyc::xvector<T,D> &v1, const Eigen::Matrix<T,1,D> &v2) 
{
	for(int i=0; i<D; ++i)
		v1[i]=v2[i];
}


template<int D>
void vector_assign(wyc::xvector<float,D> &v1, const arma::fvec::fixed<D> &v2) 
{
	for(int i=0; i<D; ++i)
		v1[i]=v2[i];
}

//--------------------------------------------------
// 向量比较
//--------------------------------------------------

template<typename T, int D>
bool vector_equal(const wyc::xvector<T,D> &v1, const Eigen::Matrix<T,D,1> &v2) 
{
	for(int i=0; i<D; ++i)
		if(!FCMP(v1[i],v2[i])) {
			printf("vector[%d]:\n",i);
			fprint(v1[i],v2[i]);
			return false;
		}
	return true;
} 

template<typename T, int D>
bool vector_equal_t(const wyc::xvector<T,D> &v1, const Eigen::Matrix<T,1,D> &v2) 
{
	for(int i=0; i<D; ++i)
		if(!FCMP(v1[i],v2[i])) {
			printf("vector[%d]:\n",i);
			fprint(v1[i],v2[i]);
			return false;
		}
	return true;
} 

template<int D>
bool vector_equal(const wyc::xvector<float,D> &v1, const arma::fvec::fixed<D> &v2) 
{
	for(int i=0; i<D; ++i)
		if(!FCMP(v1[i],v2[i])) {
			printf("vector[%d]:\n",i);
			fprint(v1[i],v2[i]);
			return false;
		}
	return true;
} 

template<typename T, int D>
inline bool operator == (const wyc::xvector<T,D> &v1, const Eigen::Matrix<T,D,1> &v2)
{
	return vector_equal(v1,v2);
}

template<int D>
inline bool operator == (const wyc::xvector<float,D> &v1, const arma::fvec::fixed<D> &v2)
{
	return vector_equal(v1,v2);
}

//--------------------------------------------------
// 矩阵赋值
//--------------------------------------------------

template<typename T, int R, int C>
void matrix_assign (wyc::xmatrix<T,R,C> &mat1, const Eigen::Matrix<T,R,C> &mat2)
{
	for(int i=0; i<R; ++i) {
		for(int j=0; j<C; ++j)
			mat1(i,j)=mat2(i,j);
	}
}

template<int R, int C>
void matrix_assign (wyc::xmatrix<float,R,C> &mat1, const typename arma::Mat<float>::fixed<R,C> &mat2)
{
	for(int i=0; i<R; ++i) {
		for(int j=0; j<C; ++j)
			mat1(i,j)=mat2(i,j);
	}
}

//--------------------------------------------------
// 矩阵比较
//--------------------------------------------------

template<typename T, int R, int C>
bool matrix_equal (const wyc::xmatrix<T,R,C> &mat, const Eigen::Matrix<T,R,C> &baseMat)
{
	for(int i=0; i<R; ++i) {
		for(int j=0; j<C; ++j)
			if(!FCMP(mat(i,j),baseMat(i,j))) {
				printf("matrix(%d,%d):\n",i,j);
				fprint(mat(i,j),baseMat(i,j));
				return false;
			}
	}
	s_statistics.cmpcnt+=R*C;
	return true;
}

template<int R, int C>
bool matrix_equal (const wyc::xmatrix<float,R,C> &mat, const typename arma::Mat<float>::fixed<R,C> &baseMat)
{
	for(int i=0; i<R; ++i) {
		for(int j=0; j<C; ++j)
			if(!FCMP(mat(i,j),baseMat(i,j))) {
				printf("matrix(%d,%d):\n",i,j);
				fprint(mat(i,j),baseMat(i,j));
				return false;
			}
	}
	s_statistics.cmpcnt+=R*C;
	return true;
}

template<typename T, int R, int C>
inline bool operator == (const wyc::xmatrix<T,R,C> &mat, const Eigen::Matrix<T,R,C> &baseMat)
{
	return matrix_equal<T,R,C>(mat,baseMat);
}

template<typename T, int R, int C>
inline bool operator == (const Eigen::Matrix<T,R,C> &baseMat, const wyc::xmatrix<T,R,C> &mat)
{
	return matrix_equal<T,R,C>(mat,baseMat);
}

template<int R, int C>
inline bool operator == (const wyc::xmatrix<float,R,C> &mat, const typename arma::Mat<float>::fixed<R,C> &baseMat)
{
	return matrix_equal<R,C>(mat,baseMat);
}

template<int R, int C>
inline bool operator == (const typename arma::Mat<float>::fixed<R,C> &baseMat, const wyc::xmatrix<float,R,C> &mat)
{
	return matrix_equal<R,C>(mat,baseMat);
}

