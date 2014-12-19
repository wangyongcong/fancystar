#ifndef _LIB

#include "test_math.h"

template<typename MATRIX_T, int R, int C>
void rand_matrix(MATRIX_T &mat)
{
	for(int i=0; i<R; ++i) {
		for(int j=0; j<C; ++j) {
			mat(i,j)=mrand();
		}
	}
}

enum MATRIX_OP {
	MAT_OP_ADD=0,
	MAT_OP_MUL,
	MAT_OP_INV,
	MAT_OP_COUNT
};

static const char *s_matrix_op_name[MAT_OP_COUNT]={
	"addition",
	"multiplication",
	"inversion",
};

template<typename MATRIX_T>
struct matrix_add
{
	inline void operator() (MATRIX_T &ret, const MATRIX_T &mat)
	{
		ret+=mat;
	}
	inline const char* name() {
		return s_matrix_op_name[MAT_OP_ADD];
	}
};

template<typename MATRIX_T>
struct matrix_mul
{
	inline void operator() (MATRIX_T &ret, const MATRIX_T &mat)
	{
		ret*=mat;
	}
	inline const char* name() {
		return s_matrix_op_name[MAT_OP_MUL];
	}
};

template<typename MATRIX_T>
struct wyc_inverse
{
	inline void operator() (MATRIX_T &ret, const MATRIX_T &mat)
	{
		ret.inverse(mat);
	}
	inline const char* name() {
		return s_matrix_op_name[MAT_OP_INV];
	}
};

template<typename MATRIX_T>
struct eigen_inverse
{
	inline void operator() (MATRIX_T &ret, const MATRIX_T &mat)
	{
		ret=mat.inverse();
	}
	inline const char* name() {
		return s_matrix_op_name[MAT_OP_INV];
	}
};

template<typename MATRIX_T>
struct arma_inverse
{
	inline void operator() (MATRIX_T &ret, const MATRIX_T &mat)
	{
		ret=arma::inv(mat);
	}
	inline const char* name() {
		return s_matrix_op_name[MAT_OP_INV];
	}
};

template<typename MATRIX_T, int R, int C, typename FUNCTOR>
void _bm_matrix_op(unsigned round, double &timer)
{
	wyc::xcode_timer ct;
	MATRIX_T mat1, mat2;
	FUNCTOR ftor;
	timer=0;
	printf("executing %d matrix %s...\n",round,ftor.name());
	for(unsigned i=0; i<round; ++i) {
		rand_matrix<MATRIX_T,R,C>(mat1);
		rand_matrix<MATRIX_T,R,C>(mat2);
		ct.start();
		ftor(mat2,mat1);
		ct.stop();
		timer+=ct.get_time();
	}
}

template<typename MATRIX_T, int R, int C, typename FUNCTOR>
void _bm_dynmat_op(unsigned round, double &timer)
{
	wyc::xcode_timer ct;
	MATRIX_T mat1(R,C), mat2(R,C);
	FUNCTOR ftor;
	timer=0;
	printf("executing %d matrix %s...\n",round,ftor.name());
	for(unsigned i=0; i<round; ++i) {
		rand_matrix<MATRIX_T,R,C>(mat1);
		rand_matrix<MATRIX_T,R,C>(mat2);
		ct.start();
		ftor(mat2,mat1);
		ct.stop();
		timer+=ct.get_time();
	}
}
#define BENCHMARK_MATRIX_ADD
#define BENCHMARK_MATRIX_MUL
#define BENCHMARK_MATRIX_INV

template<int R, int C>
void _benchmark_matrix()
{
	int round=1000000;
	unsigned seed=rand_seed();
	BENCHMARK_MATRIX rec1, rec2, rec3;
	rec1.round=round;
	rec2.round=round;
	rec3.round=round;

	// add
#ifdef BENCHMARK_MATRIX_ADD
	mrand_init(seed);
	_bm_matrix_op<wyc::xmatrix<float,R,C>,R,C,matrix_add<wyc::xmatrix<float,R,C> > >(round,rec1.tmatadd);
	mrand_init(seed);
	_bm_matrix_op<Eigen::Matrix<float,R,C>,R,C,matrix_add<Eigen::Matrix<float,R,C> > >(round,rec2.tmatadd);
	mrand_init(seed);
	_bm_matrix_op<arma::Mat<float>::fixed<R,C>,R,C,matrix_add<arma::Mat<float>::fixed<R,C> > >(round,rec3.tmatadd);
#endif // BENCHMARK_MATRIX_ADD
	// mul
#ifdef BENCHMARK_MATRIX_MUL
	mrand_init(seed);
	_bm_matrix_op<wyc::xmatrix<float,R,C>,R,C,matrix_mul<wyc::xmatrix<float,R,C> > >(round,rec1.tmatmul);
	mrand_init(seed);
	_bm_matrix_op<Eigen::Matrix<float,R,C>,R,C,matrix_mul<Eigen::Matrix<float,R,C> > >(round,rec2.tmatmul);
	mrand_init(seed);
	_bm_matrix_op<arma::Mat<float>::fixed<R,C>,R,C,matrix_mul<arma::Mat<float>::fixed<R,C> > >(round,rec3.tmatmul);
#endif // BENCHMARK_MATRIX_MUL
	// inverse
#ifdef BENCHMARK_MATRIX_INV
	mrand_init(seed);
	_bm_matrix_op<wyc::xmatrix<float,R,C>,R,C,wyc_inverse<wyc::xmatrix<float,R,C> > >(round,rec1.tmatinv);
	mrand_init(seed);
	_bm_matrix_op<Eigen::Matrix<float,R,C>,R,C,eigen_inverse<Eigen::Matrix<float,R,C> > >(round,rec2.tmatinv);
	mrand_init(seed);
	_bm_matrix_op<arma::Mat<float>::fixed<R,C>,R,C,arma_inverse<arma::Mat<float>::fixed<R,C> > >(round,rec3.tmatinv);
#endif // BENCHMARK_MATRIX_INV

	printf("//-------------------------\n");
	printf("// matrix %dx%d benchmark\n",R,C);
	printf("//-------------------------\n");
	rec1.report("wyc matrix");
	rec2.report("eigne matrix");
	rec3.report("arma matrix");
	printf("\nbenchmark is done\n");
}

template<int R, int C>
void _benchmark_bigmatrix()
{
	int round=1000000;
	unsigned seed=rand_seed();
	BENCHMARK_MATRIX rec1, rec2, rec3;
	rec1.round=round;
	rec2.round=round;
	rec3.round=round;

	// add
#ifdef BENCHMARK_MATRIX_ADD
	mrand_init(seed);
	_bm_matrix_op<wyc::xmatrix<float,R,C>,R,C,matrix_add<wyc::xmatrix<float,R,C> > >(round,rec1.tmatadd);
	mrand_init(seed);
	_bm_dynmat_op<Eigen::MatrixXf,R,C,matrix_add<Eigen::MatrixXf> >(round,rec2.tmatadd);
	mrand_init(seed);
	_bm_dynmat_op<arma::Mat<float>,R,C,matrix_add<arma::Mat<float> > >(round,rec3.tmatadd);
#endif // BENCHMARK_MATRIX_ADD
	// mul
#ifdef BENCHMARK_MATRIX_MUL
	mrand_init(seed);
	_bm_matrix_op<wyc::xmatrix<float,R,C>,R,C,matrix_mul<wyc::xmatrix<float,R,C> > >(round,rec1.tmatmul);
	mrand_init(seed);
	_bm_dynmat_op<Eigen::MatrixXf,R,C,matrix_mul<Eigen::MatrixXf> >(round,rec2.tmatmul);
	mrand_init(seed);
	_bm_dynmat_op<arma::Mat<float>,R,C,matrix_mul<arma::Mat<float> > >(round,rec3.tmatmul);
#endif // BENCHMARK_MATRIX_MUL
	// inverse
#ifdef BENCHMARK_MATRIX_INV
	mrand_init(seed);
	_bm_matrix_op<wyc::xmatrix<float,R,C>,R,C,wyc_inverse<wyc::xmatrix<float,R,C> > >(round,rec1.tmatinv);
	mrand_init(seed);
	_bm_dynmat_op<Eigen::MatrixXf,R,C,eigen_inverse<Eigen::MatrixXf> >(round,rec2.tmatinv);
//	mrand_init(seed);
//	_bm_dynmat_op<arma::Mat<float>,R,C,arma_inverse<arma::Mat<float> > >(round,rec3.tmatinv);
#endif // BENCHMARK_MATRIX_INV

	printf("//-------------------------\n");
	printf("// matrix %dx%d benchmark\n",R,C);
	printf("//-------------------------\n");
	rec1.report("wyc matrix");
	rec2.report("eigne matrix");
	rec3.report("arma matrix");
	printf("\nbenchmark is done\n");
}

void benchmark_matrix()
{
//	_benchmark_matrix<2,2>();
//	_benchmark_matrix<3,3>();
//	_benchmark_matrix<4,4>();
	_benchmark_bigmatrix<5,5>();
}

#endif // _LIB


