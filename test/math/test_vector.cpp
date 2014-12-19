#ifndef _LIB

/****************************************************
// 向量相关运算的测试和速度比较
// 用于测试和比较的程序库：
//	Eigen-3.0: http://eigen.tuxfamily.org/index.php?title=Main_Page#Download
//	Armadillo-1.1.90: http://arma.sourceforge.net/
****************************************************/

#include "test_math.h"

using std::cout;
using std::endl;
using wyc::xmother_random;

//--------------------------------------------------
// 随机向量
//--------------------------------------------------

template<typename T, int D>
inline void random_vector(Eigen::Matrix<T,D,1> &vec)
{
	vec=Eigen::Matrix<T,D,1>::Random();
}

template<int D>
inline void random_vector(arma::fvec::fixed<D> &vec)
{
	vec.randu();
}

//--------------------------------------------------
// 向量点积
//--------------------------------------------------

template<typename T, int D>
inline T dot(const Eigen::Matrix<T,D,1> &v1, const Eigen::Matrix<T,D,1> &v2)
{
	return v1.dot(v2);
}

template<int D>
inline float dot(const arma::fvec::fixed<D> &v1, const arma::fvec::fixed<D> &v2)
{
	return arma::dot(v1,v2);
}

//--------------------------------------------------
// 向量叉积
//--------------------------------------------------

template<typename T, int D>
inline Eigen::Matrix<T,D,1> cross(const Eigen::Matrix<T,D,1> &v1, const Eigen::Matrix<T,D,1> &v2)
{
	Eigen::Matrix<T,D,1> ret;
	ret=v1.cross(v2);
	return ret;
}

template<int D>
inline arma::fvec::fixed<D> cross(const arma::fvec::fixed<D> &v1, const arma::fvec::fixed<D> &v2)
{
	arma::fvec::fixed<D> ret;
	ret=arma::cross(v1,v2);
	return ret;
}


//--------------------------------------------------
// 测试模板
// BASE_VECTOR_T：作为测试基准的向量类型
// VECTOR_T：待测试向量类型
// ROUND：测试轮数
// 所有向量类必须重载常用的运算符
//--------------------------------------------------

template<typename BASE_VECTOR_T, typename VECTOR_T, int ROUND>
void _test_vector()
{
	float fval1, fval2;
	BASE_VECTOR_T baseVec1, baseVec2, baseVec3;
	VECTOR_T vec1, vec2, vec3;
	for(int i=0; i<ROUND; ++i) {
		random_vector(baseVec1);
		random_vector(baseVec2);
		printf("(%d) vector[0]:\n",i);
		cout<<baseVec1<<endl;
		printf("(%d) vector[1]:\n",i);
		cout<<baseVec2<<endl;
		vector_assign(vec1,baseVec1);
		vector_assign(vec2,baseVec2);
		assert(vec1==baseVec1);
		assert(vec2==baseVec2);
		// reverse
		baseVec3=-baseVec1;
		vec3=-vec1;
		assert(vec3==baseVec3);
		vec3.reverse();
		assert(vec3==vec1);
		assert(vec3!=vec2);
		// add
		baseVec3=baseVec1+baseVec2;
		vec3=vec1+vec2;
		assert(vec3==baseVec3);
		// sub
		baseVec3=baseVec1-baseVec2;
		vec3=vec1-vec2;
		assert(vec3==baseVec3);
		// scale
		fval1=mrand();
		baseVec3=baseVec1*fval1;
		vec3=vec1*fval1;
		assert(vec3==baseVec3);
		// dot
		fval1=dot(baseVec1,baseVec2);
		fval2=vec1*vec2;
		if(!FCMP(fval1,fval2)) {
			printf("Dot product failed\n");
			fprint(fval1,fval2);
			assert(0);
		}
		fval2=vec1.dot(vec2);
		if(!FCMP(fval1,fval2)) {
			printf("Dot product failed\n");
			fprint(fval1,fval2);
			assert(0);
		}
		// unary operators
		baseVec3=baseVec1;
		vec3=vec1;
		baseVec3+=baseVec2;
		vec3+=vec2;
		assert(vec3==baseVec3);
		baseVec3-=baseVec1;
		vec3-=vec1;
		assert(vec3==baseVec3);
		fval1=mrand();
		baseVec3*=fval1;
		vec3*=fval1;
		assert(vec3==baseVec3);
		// length
		fval1=vec1.length2();
		fval2=vec1.length();
		fval2*=fval2;
		if(!FCMP(fval1,fval2)) {
			printf("Length failed\n");
			fprint(fval1,fval2);
			assert(0);
		}
		// normalize
		vec3=vec1;
		vec3.normalize();
		fval1=vec3.length();
		if(!FCMP(fval1,1)) {
			printf("Normalize failed\n");
			assert(0);
		}
	}
	printf("all tests are passed\n");
	s_statistics.report();
}

template<typename BASE_VECTOR_T, typename VECTOR_T, int ROUND>
void _test_cross_product()
{
	BASE_VECTOR_T baseVec1, baseVec2, baseVec3;
	VECTOR_T vec1, vec2, vec3;
	for(int i=0; i<ROUND; ++i) {
		random_vector(baseVec1);
		random_vector(baseVec2);
		printf("(%d) vector[0]:\n",i);
		cout<<baseVec1<<endl;
		printf("(%d) vector[1]:\n",i);
		cout<<baseVec2<<endl;
		vector_assign(vec1,baseVec1);
		vector_assign(vec2,baseVec2);
		assert(vec1==baseVec1);
		assert(vec2==baseVec2);
		// cross product
		baseVec3=cross(baseVec1,baseVec2);
		vec3.cross(vec1,vec2);
		assert(vec3==baseVec3);
	}
}

void test_vecmat_assignment()
{
	printf("testing vector/matrix assignment...\n");
	wyc::xvec2f_t ret1;
	wyc::xvec3f_t ret2;
	wyc::xvec4f_t ret3;
	wyc::xvector<float,5> ret4;

	// vector assignment
	float val=mrand();
	wyc::xvec2f_t vec1(mrand(),mrand());
	wyc::xvec3f_t vec2(mrand(),mrand(),mrand());
	wyc::xvec4f_t vec3(mrand(),mrand(),mrand(),mrand());
	wyc::xvector<float,5> vec4;
	// vector5
	vec4=val;
	assert(vec4[0]==val && vec4[1]==val && vec4[2]==val && vec4[3]==val && vec4[4]==val);
	vec4=vec1;
	assert(vec4[0]==vec1.x && vec4[1]==vec1.y);
	vec4=vec2;
	assert(vec4[0]==vec2.x && vec4[1]==vec2.y && vec4[2]==vec2.z);
	vec4=vec3;
	assert(vec4[0]==vec3.x && vec4[1]==vec3.y && vec4[2]==vec3.z && vec4[3]==vec3.w);
	// vector4
	vec3=val;
	assert(vec3[0]==val && vec3[1]==val && vec3[2]==val && vec3[3]==val);
	vec3=vec1;
	assert(vec3[0]==vec1.x && vec3[1]==vec1.y);
	vec3=vec2;
	assert(vec3[0]==vec2.x && vec3[1]==vec2.y && vec3[2]==vec2.z);
	vec3=vec4;
	assert(vec3[0]==vec4[0] && vec3[1]==vec4[1] && vec3[2]==vec4[2] && vec3[3]==vec4[3]);
	// vector3
	vec2=val;
	assert(vec2.x==val && vec2.y==val && vec2.z==val);
	vec2=vec1;
	assert(vec2.x==vec1.x && vec2.y==vec1.y);
	vec2=vec3;
	assert(vec2.x==vec3.x && vec2.y==vec3.y && vec2.z==vec3.z);
	vec2=vec4;
	assert(vec2.x==vec4[0] && vec2.y==vec4[1] && vec2.z==vec4[2]);
	// vector2
	vec1=val;
	assert(vec1.x==val && vec1.y==val);
	vec1=vec2;
	assert(vec1.x==vec2.x && vec1.y==vec2.y);
	vec1=vec3;
	assert(vec1.x==vec3.x && vec1.y==vec3.y);
	vec1=vec4;
	assert(vec1.x==vec4[0] && vec1.y==vec4[1]);

	// matrix assignment
	wyc::xmat2f_t mat1;
	wyc::xmat3f_t mat2;
	wyc::xmat4f_t mat3;
	wyc::xmatrix<float,5,5> mat4;
	// matrix 5x5
	mat4.set_row(0,vec1);
	mat4.get_row(0,ret4);
	assert(vec1[0]==ret4[0] && vec1[1]==ret4[1]);
	mat4.set_row(1,vec2);
	mat4.get_row(1,ret4);
	assert(vec2[0]==ret4[0] && vec2[1]==ret4[1] && vec2[2]==ret4[2]);
	mat4.set_row(2,vec3);
	mat4.get_row(2,ret4);
	assert(vec3[0]==ret4[0] && vec3[1]==ret4[1] && vec3[2]==ret4[2] && vec3[3]==ret4[3]);
	mat4.set_row(3,vec4);
	mat4.get_row(3,ret4);
	assert(vec4[0]==ret4[0] && vec4[1]==ret4[1] && vec4[2]==ret4[2] && vec4[3]==ret4[3] && vec4[4]==ret4[4]);
	mat4.set_col(0,vec1);
	mat4.get_col(0,ret4);
	assert(vec1[0]==ret4[0] && vec1[1]==ret4[1]);
	mat4.set_col(1,vec2);
	mat4.get_col(1,ret4);
	assert(vec2[0]==ret4[0] && vec2[1]==ret4[1] && vec2[2]==ret4[2]);
	mat4.set_col(2,vec3);
	mat4.get_col(2,ret4);
	assert(vec3[0]==ret4[0] && vec3[1]==ret4[1] && vec3[2]==ret4[2] && vec3[3]==ret4[3]);
	mat4.set_col(3,vec4);
	mat4.get_col(3,ret4);
	assert(vec4[0]==ret4[0] && vec4[1]==ret4[1] && vec4[2]==ret4[2] && vec4[3]==ret4[3] && vec4[4]==ret4[4]);
	// matrix 4x4
	mat3.set_row(0,vec1);
	mat3.get_row(0,ret3);
	assert(vec1[0]==ret3[0] && vec1[1]==ret3[1]);
	mat3.set_row(1,vec2);
	mat3.get_row(1,ret3);
	assert(vec2[0]==ret3[0] && vec2[1]==ret3[1] && vec2[2]==ret3[2]);
	mat3.set_row(2,vec3);
	mat3.get_row(2,ret3);
	assert(vec3[0]==ret3[0] && vec3[1]==ret3[1] && vec3[2]==ret3[2] && vec3[3]==ret3[3]);
	mat3.set_col(0,vec1);
	mat3.get_col(0,ret3);
	assert(vec1[0]==ret3[0] && vec1[1]==ret3[1]);
	mat3.set_col(1,vec2);
	mat3.get_col(1,ret3);
	assert(vec2[0]==ret3[0] && vec2[1]==ret3[1] && vec2[2]==ret3[2]);
	mat3.set_col(2,vec3);
	mat3.get_col(2,ret3);
	assert(vec3[0]==ret3[0] && vec3[1]==ret3[1] && vec3[2]==ret3[2] && vec3[3]==ret3[3]);
	// matrix 3x3
	mat2.set_row(0,vec1);
	mat2.get_row(0,ret2);
	assert(vec1[0]==ret2[0] && vec1[1]==ret2[1]);
	mat2.set_row(1,vec2);
	mat2.get_row(1,ret2);
	assert(vec2[0]==ret2[0] && vec2[1]==ret2[1] && vec2[2]==ret2[2]);
	mat2.set_col(0,vec1);
	mat2.get_col(0,ret2);
	assert(vec1[0]==ret2[0] && vec1[1]==ret2[1]);
	mat2.set_col(1,vec2);
	mat2.get_col(1,ret2);
	assert(vec2[0]==ret2[0] && vec2[1]==ret2[1] && vec2[2]==ret2[2]);
	// matrix 2x2
	mat1.set_row(0,vec1);
	mat1.get_row(0,ret1);
	assert(vec1[0]==ret1[0] && vec1[1]==ret1[1]);
	mat1.set_col(0,vec1);
	mat1.get_col(0,ret1);
	assert(vec1[0]==ret1[0] && vec1[1]==ret1[1]);
}

void test_vector()
{
	//-------------------------------------------------------------
	// 基于lib Eigen进行测试
	//-------------------------------------------------------------
	// vector2D
//	_test_vector<Eigen::Vector2f,wyc::xvec2f_t,1000>();
	// vector3D
	_test_cross_product<Eigen::Vector3f,wyc::xvec3f_t,1000>();
	_test_vector<Eigen::Vector3f,wyc::xvec3f_t,1000>();
	// vector4D
//	_test_vector<Eigen::Vector4f,wyc::xvec4f_t,1000>();
	// vector5D
//	_test_vector<Eigen::Matrix<float,5,1>,wyc::xvector<float,5>,1000>();
	//-------------------------------------------------------------
	// 基于lib Armadillo进行测试
	//-------------------------------------------------------------
	// vector2D
//	_test_vector<arma::fvec::fixed<2>,wyc::xvec2f_t,1000>();
	// vector3D
//	_test_cross_product<arma::fvec::fixed<3>,wyc::xvec3f_t,1000>();
//	_test_vector<arma::fvec::fixed<3>,wyc::xvec3f_t,1000>();
	// vector4D
//	_test_vector<arma::fvec::fixed<4>,wyc::xvec4f_t,1000>();
	// vector5D
//	_test_vector<arma::fvec::fixed<5>,wyc::xvector<float,5>,1000>();
}

#endif // _LIB

