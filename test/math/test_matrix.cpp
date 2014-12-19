#ifndef _LIB

/****************************************************
// 矩阵相关运算的测试和速度比较
// 用于测试和比较的程序库：
//	Eigen-3.0: http://eigen.tuxfamily.org/index.php?title=Main_Page#Download
//	Armadillo-1.1.90: http://arma.sourceforge.net/
****************************************************/

#include "test_math.h"

using wyc::xmother_random;

//--------------------------------------------------
// 随机矩阵
//--------------------------------------------------

template<typename T, int R, int C>
inline void rand_matrix(Eigen::Matrix<T,R,C> &mat)
{
	mat=Eigen::Matrix<T,R,C>::Random();
}

template<int R, int C>
inline void rand_matrix(arma::Mat<float>::fixed<R,C> &mat)
{
	for(int i=0; i<R; ++i) {
		for(int j=0; j<C; ++j)
			mat(i,j)=mrand();
	}
}

//--------------------------------------------------
// 求行列式
//--------------------------------------------------

template<typename T, int R, int C>
inline T determinant(Eigen::Matrix<T,R,C> &mat)
{
	return mat.determinant();
}

template<int R, int C>
inline float determinant(arma::Mat<float>::fixed<R,C> &mat)
{
	return arma::det(mat);
}

//--------------------------------------------------
// 求逆阵
//--------------------------------------------------

template<typename T, int R, int C>
inline void inverse(Eigen::Matrix<T,R,C> &ret, const Eigen::Matrix<T,R,C> &mat)
{
	ret=mat.inverse();
}

template<int R, int C>
inline void inverse(arma::Mat<float>::fixed<R,C> &ret, const arma::Mat<float>::fixed<R,C> &mat)
{
	ret=arma::inv(mat);
}

//--------------------------------------------------
// 测试模板
// BASE_MATRIX_T：作为测试基准的矩阵类型
// MATRIX_T：待测试矩阵类型
// ROUND：测试轮数
// 所有矩阵类必须重载常用的运算符
//--------------------------------------------------

template<typename BASE_MATRIX_T, typename MATRIX_T>
void _test_square_matrix(unsigned round) 
{
	float fval1, fval2;
	BASE_MATRIX_T baseMat1, baseMat2, baseMat3;
	MATRIX_T mat1, mat2, mat3;
	s_statistics.reset();
	printf("executing %d caculations...\n",round);
	for(unsigned i=0; i<round; ++i) {
		rand_matrix(baseMat1);
		rand_matrix(baseMat2);
		matrix_assign(mat1,baseMat1);
		matrix_assign(mat2,baseMat2);
		assert(mat1==baseMat1);
		assert(mat2==baseMat2);
		// identity and compare
		mat3.identity();
		mat3.mul(mat1);
		assert(mat1==mat3);
		assert(mat2!=mat3);
		// add
		baseMat3=baseMat1+baseMat2;
		mat3=mat1+mat2;
		assert(mat3==baseMat3);
		// sub
		baseMat3=baseMat1-baseMat2;
		mat3=mat1-mat2;
		assert(mat3==baseMat3);
		// scale
		fval1=mrand();
		baseMat3=baseMat1/fval1;
		mat3=mat1/fval1;
		assert(mat3==baseMat3);
		fval1=mrand();
		baseMat3=baseMat2*fval1;
		mat3=mat2*fval1;
		assert(mat3==baseMat3);
		// mul
		baseMat3=baseMat1*baseMat2;
		mat3=mat1*mat2;
		assert(mat3==baseMat3);
		// unary
		baseMat1+=baseMat2;
		mat1+=mat2;
		assert(mat1==baseMat1);
		baseMat1-=baseMat2;
		mat1-=mat2;
		assert(mat1==baseMat1);
		fval1=mrand();
		baseMat1*=fval1;
		mat1*=fval1;
		assert(mat1==baseMat1);
		baseMat1/=fval1;
		mat1/=fval1;
		assert(mat1==baseMat1);
		baseMat1*=baseMat2;
		mat1*=mat2;
		assert(mat1==baseMat1);
		// transpose
		mat3.transpose(mat2);
		mat3.transpose();
		assert(mat3==baseMat2);
		// determinant
		fval1=determinant(baseMat2);
		fval2=mat2.det();
		if(!FCMP(fval1,fval2)) {
			printf("determinant failed:\n");
			fprint(fval1,fval2);
			assert(0);
		}
	}
	s_statistics.report();
}

template<typename BASE_MATRIX_T, typename MATRIX_T>
void _inverse_square_matrix(unsigned round) 
{
	BASE_MATRIX_T baseMat1, baseMat2;
	MATRIX_T mat1, mat2;
	s_statistics.reset();
	printf("executing %d caculations...\n",round);
	for(unsigned i=0; i<round; ++i) {
		rand_matrix(baseMat1);
		matrix_assign(mat1,baseMat1);
		assert(mat1==baseMat1);
		// 逆阵算法不够稳定
		// 当fval1很小的时候，浮点误差会很大
		// 当前matrix44的状况是：100W次严重误差次数小于10
		// TODO：使用不同的逆阵算法
		if(mat2.inverse(mat1)) {
			inverse(baseMat2,baseMat1);
			if(!(mat2==baseMat2)) {
				printf("[%d] error occured\n",i);
				printf("base matrix:\n");
				std::cout<<baseMat2<<std::endl;
				printf("test matrix:\n");
				std::cout<<mat2<<std::endl;
			}
		}
	}
	s_statistics.report();
}

//#define TEST_MAT2F
//#define TEST_MAT3F
#define TEST_MAT4F
//#define TEST_MAT5F

//#define ENABLE_ARMADILLO

void test_matrix() 
{
	unsigned round=1000000;

	//-------------------------------------------------------------
	// 基于lib Eigen进行测试
	//-------------------------------------------------------------
#ifdef TEST_MAT2F
	printf("matrix22 (base on Eigen)\n");
	_test_square_matrix<Eigen::Matrix2f,wyc::xmat2f_t>(round);
#endif // TEST_MAT2F
#ifdef TEST_MAT3F
	printf("matrix33 (base on Eigen)\n");
	_test_square_matrix<Eigen::Matrix3f,wyc::xmat3f_t>(round);
#endif // TEST_MAT3F
#ifdef TEST_MAT4F
	printf("matrix44 (base on Eigen)\n");
	_test_square_matrix<Eigen::Matrix4f,wyc::xmat4f_t>(round);
#endif // TEST_MAT4F
#ifdef TEST_MAT5F
	printf("matrix55 (base on Eigen)\n");
	_test_square_matrix<Eigen::Matrix<float,5,5>,wyc::xmatrix<float,5,5> >(round);
#endif // TEST_MAT5F

	//-------------------------------------------------------------
	// 基于lib Armadillo进行测试
	//-------------------------------------------------------------
#ifdef ENABLE_ARMADILLO
#ifdef TEST_MAT2F
	printf("matrix22 (base on Armadillo)\n");
	_test_square_matrix<arma::Mat<float>::fixed<2,2>,wyc::xmat2f_t>(round);
#endif // TEST_MAT2F
#ifdef TEST_MAT3F
	printf("matrix33 (base on Armadillo)\n");
	_test_square_matrix<arma::Mat<float>::fixed<3,3>,wyc::xmat3f_t>(round);
#endif // TEST_MAT3F
#ifdef TEST_MAT4F
	printf("matrix44 (base on Armadillo)\n");
	_test_square_matrix<arma::Mat<float>::fixed<4,4>,wyc::xmat4f_t>(round);
#endif // TEST_MAT4F
#endif // BASE_ON_ARMADILLO
}

void test_matrix_inverse()
{
	unsigned round=10000;
#ifdef TEST_MAT2F
	_inverse_square_matrix<Eigen::Matrix2f,wyc::xmat2f_t>(round);
#endif // TEST_MAT2F
#ifdef TEST_MAT3F
	_inverse_square_matrix<Eigen::Matrix3f,wyc::xmat3f_t>(round);
#endif // TEST_MAT3F
#ifdef TEST_MAT4F
	_inverse_square_matrix<Eigen::Matrix4f,wyc::xmat4f_t>(round);
#endif // TEST_MAT4F
#ifdef TEST_MAT5F
	_inverse_square_matrix<Eigen::Matrix<float,5,5>,wyc::xmatrix<float,5,5> >(round);
#endif // TEST_MAT5F
}

//--------------------------------------------------
// 向量-矩阵运算测试
// 以Eigen库作为测试基准
//--------------------------------------------------
void test_vecmat()
{
	int ROUND=1000000;
	bool b;
	Eigen::Matrix2f baseMat22;
	Eigen::Matrix3f baseMat33;
	Eigen::Matrix4f baseMat44;
	Eigen::Vector2f baseVec2, baseRet2;
	Eigen::Vector3f baseVec3, baseRet3;
	Eigen::Vector4f baseVec4, baseRet4;
	Eigen::RowVector2f baseVec2T, baseRet2T;
	Eigen::RowVector3f baseVec3T, baseRet3T;
	Eigen::RowVector4f baseVec4T, baseRet4T;
	wyc::xmat2f_t mat22;
	wyc::xmat3f_t mat33;
	wyc::xmat4f_t mat44;
	wyc::xvec2f_t vec2, ret2;
	wyc::xvec3f_t vec3, ret3;
	wyc::xvec4f_t vec4, ret4;
	printf("executing %d caculations...\n",ROUND);
	for(int i=0; i<ROUND; ++i)
	{
		// vector2 * matrix22
		baseMat22=Eigen::Matrix2f::Random();
		matrix_assign(mat22,baseMat22);
		baseVec2=Eigen::Vector2f::Random();
		vector_assign(vec2,baseVec2);
		baseRet2=baseMat22*baseVec2;
		ret2=mat22*vec2;
		assert(ret2==baseRet2);
		baseVec2T=Eigen::RowVector2f::Random();
		vector_assign_t<float,2>(vec2,baseVec2T);
		baseRet2T=baseVec2T*baseMat22;
		ret2=vec2*mat22;
		b=vector_equal_t<float,2>(ret2,baseRet2T);
		assert(b);
		// vector3 * matrix33
		baseMat33=Eigen::Matrix3f::Random();
		matrix_assign(mat33,baseMat33);
		baseVec3=Eigen::Vector3f::Random();
		vector_assign(vec3,baseVec3);
		baseRet3=baseMat33*baseVec3;
		ret3=mat33*vec3;
		assert(ret3==baseRet3);
		baseVec3T=Eigen::RowVector3f::Random();
		vector_assign_t<float,3>(vec3,baseVec3T);
		baseRet3T=baseVec3T*baseMat33;
		ret3=vec3*mat33;
		b=vector_equal_t<float,3>(ret3,baseRet3T);
		assert(b);
		// vector2 * matrix33
		baseVec3[2]=0;
		vec2.x=baseVec3[0];
		vec2.y=baseVec3[1];
		baseRet3=baseMat33*baseVec3;
		ret2=mat33*vec2;
		b=FCMP(ret2.x,baseRet3[0]) && FCMP(ret2.y,baseRet3[1]);
		assert(b);
		baseVec3T[2]=0;
		vec2.x=baseVec3T[0];
		vec2.y=baseVec3T[1];
		baseRet3T=baseVec3T*baseMat33;
		ret2=vec2*mat33;
		b=FCMP(ret2.x,baseRet3T[0]) && FCMP(ret2.y,baseRet3T[1]);
		assert(b);
		// vector4 * matrix44
		baseMat44=Eigen::Matrix4f::Random();
		matrix_assign(mat44,baseMat44);
		baseVec4=Eigen::Vector4f::Random();
		vector_assign(vec4,baseVec4);
		baseRet4=baseMat44*baseVec4;
		ret4=mat44*vec4;
		assert(ret4==baseRet4);
		baseVec4T=Eigen::RowVector4f::Random();
		vector_assign_t<float,4>(vec4,baseVec4T);
		baseRet4T=baseVec4T*baseMat44;
		ret4=vec4*mat44;
		b=vector_equal_t<float,4>(ret4,baseRet4T);
		assert(b);
		// vector3 * matrix44
		baseVec4[3]=0;
		vec3.x=baseVec4[0];
		vec3.y=baseVec4[1];
		vec3.z=baseVec4[2];
		baseRet4=baseMat44*baseVec4;
		ret3=mat44*vec3;
		b=FCMP(ret3.x,baseRet4[0]) && FCMP(ret3.y,baseRet4[1]) && FCMP(ret3.z,baseRet4[2]);
		assert(b);
		baseVec4T[3]=0;
		vec3.x=baseVec4T[0];
		vec3.y=baseVec4T[1];
		vec3.z=baseVec4T[2];
		baseRet4T=baseVec4T*baseMat44;
		ret3=vec3*mat44;
		b=FCMP(ret3.x,baseRet4T[0]) && FCMP(ret3.y,baseRet4T[1]) && FCMP(ret3.z,baseRet4T[2]);
		assert(b);
	}
	s_statistics.report();
}

#endif // _LIB

