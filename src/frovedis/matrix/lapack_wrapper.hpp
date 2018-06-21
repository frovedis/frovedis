#ifndef _LAPACK_WRAPPER_
#define _LAPACK_WRAPPER_

#include "sliced_vector.hpp"

extern "C" {

void sgesv_(int *N, int *NRHS,
            float *A, int *LDA,
            int *IPIV,
            float *B, int *LDB,
            int *INFO );

void dgesv_(int *N, int *NRHS,
            double *A, int *LDA,
            int *IPIV,
            double *B, int *LDB,
            int *INFO );

void sgels_(char *TRANS,
            int *M, int *N, int *NRHS,
            float *A, int *LDA,
            float *B, int *LDB,
            float *WORK, int *LWORK,
            int *INFO );

void dgels_(char *TRANS,
            int *M, int *N, int *NRHS,
            double *A, int *LDA,
            double *B, int *LDB,
            double *WORK, int *LWORK,
            int *INFO );

void sgelsd_(int *M, int *N, int *NRHS,
             float *A, int *LDA,
             float *B, int *LDB,
             float *S,
             float *RCOND, int *RANK,
             float *WORK, int *LWORK,
             int *IWORK,
             int *INFO);

void dgelsd_(int *M, int *N, int *NRHS,
             double *A, int *LDA,
             double *B, int *LDB,
             double *S,
             double *RCOND, int *RANK,
             double *WORK, int *LWORK,
             int *IWORK,
             int *INFO);

void sgeev_(char *JOBVL, char *JOBVR,
            int *N,
            float *A, int *LDA,
            float *WR, float *WI,
            float *VL, int *LDVL,
            float *VR, int *LDVR,
            float *WORK, int *LWORK,
            int *INFO );

void dgeev_(char *JOBVL, char *JOBVR,
            int *N,
            double *A, int *LDA,
            double *WR, double *WI,
            double *VL, int *LDVL,
            double *VR, int *LDVR,
            double *WORK, int *LWORK,
            int *INFO );

void sgesvd_(char *JOBU, char *JOBVT,
             int *M, int *N,
             float *A, int *LDA,
             float *S,
             float *U, int *LDU,
             float *VT, int *LDVT,
             float *WORK, int *LWORK,
             int *INFO );

void dgesvd_(char *JOBU, char *JOBVT,
             int *M, int *N,
             double *A, int *LDA,
             double *S,
             double *U, int *LDU,
             double *VT, int *LDVT,
             double *WORK, int *LWORK,
             int *INFO );

void sgesdd_(char *JOBZ,
             int *M, int *N,
             float *A, int *LDA,
             float *S,
             float *U, int *LDU,
             float *VT, int *LDVT,
             float *WORK, int *LWORK, int *IWORK,
             int *INFO );

void dgesdd_(char *JOBZ,
             int *M, int *N,
             double *A, int *LDA,
             double *S,
             double *U, int *LDU,
             double *VT, int *LDVT,
             double *WORK, int *LWORK, int *IWORK,
             int *INFO );

void sgetrf_(int *M, int *N,
             float *A, int *LDA,
             int *IPIV,
             int *INFO );

void dgetrf_(int *M, int *N,
             double *A, int *LDA,
             int *IPIV,
             int *INFO );

void sgetri_(int *N,
             float *A, int *LDA,
             const int *IPIV,
             float *WORK, int *LWORK,
             int *INFO );

void dgetri_(int *N,
             double *A, int *LDA,
             const int *IPIV,
             double *WORK, int *LWORK,
             int *INFO );

void sgetrs_(char *TRANS, 
             int *N, int *NRHS,
             float *A, int *LDA,
             const int *IPIV,
             float *B, int *LDB,
             int *INFO );

void dgetrs_(char *TRANS,
             int *N, int *NRHS,
             double *A, int *LDA,
             const int *IPIV,
             double *B, int *LDB,
             int *INFO );

void sgelsy_(int *M, int *N, int *NRHS,
             float *A, int *LDA,
             float *B, int *LDB,
             int *JPVT,
             float *RCOND,
             int *RANK,
             float *WORK, int *LWORK,
             int *INFO );

void dgelsy_(int *M, int *N, int *NRHS,
             double *A, int *LDA,
             double *B, int *LDB,
             int *JPVT,
             double *RCOND,
             int *RANK,
             double *WORK, int *LWORK, 
             int *INFO );

void sgelss_(int *M, int *N, int *NRHS,
             float *A, int *LDA,
             float *B, int *LDB,
             float *S,
             float *RCOND,
             int *RANK,
             float *WORK, int *LWORK,
             int *INFO );

void dgelss_(int *M, int *N, int *NRHS,
             double *A, int *LDA,
             double *B, int *LDB,
             double *S,
             double *RCOND,
             int *RANK,
             double *WORK, int *LWORK,
             int *INFO );

}

namespace frovedis{

// ================= GESV---PROTOTYPE-I =================

  template <class T>
  int gesv(const sliced_colmajor_matrix_local<T>& inMat,
           const sliced_colmajor_matrix_local<T>& outMat);

  template <>
  int gesv(const sliced_colmajor_matrix_local<float>& inMat,
           const sliced_colmajor_matrix_local<float>& outMat);

  template <>
  int gesv(const sliced_colmajor_matrix_local<double>& inMat,
           const sliced_colmajor_matrix_local<double>& outMat);

// ================= GESV---PROTOTYPE-II =================

  template <class T>
  int gesv(const sliced_colmajor_matrix_local<T>& inMat,
           const sliced_colmajor_matrix_local<T>& outMat,
           std::vector<int>& ipiv);

  template <>
  int gesv(const sliced_colmajor_matrix_local<float>& inMat,
           const sliced_colmajor_matrix_local<float>& outMat,
           std::vector<int>& ipiv);

  template <>
  int gesv(const sliced_colmajor_matrix_local<double>& inMat,
           const sliced_colmajor_matrix_local<double>& outMat,
           std::vector<int>& ipiv);

// ======================== GELS =======================

  template <class T>
  int gels(const sliced_colmajor_matrix_local<T>& inMat,
           const sliced_colmajor_matrix_local<T>& outMat,
           char TRANS = 'N');

  template <>
  int gels(const sliced_colmajor_matrix_local<float>& inMat,
           const sliced_colmajor_matrix_local<float>& outMat,
           char TRANS);

  template <>
  int gels(const sliced_colmajor_matrix_local<double>& inMat,
           const sliced_colmajor_matrix_local<double>& outMat,
           char TRANS );

// ================ GELSD---PROTOTYPE-I ==================

  template<class T>
  int gelsd(const sliced_colmajor_matrix_local<T>& inMat, 
            const sliced_colmajor_matrix_local<T>& outMat,
            T RCOND = -1);
  
  template<>
  int gelsd(const sliced_colmajor_matrix_local<float>& inMat, 
            const sliced_colmajor_matrix_local<float>& outMat,
            float RCOND);

  template<>
  int gelsd(const sliced_colmajor_matrix_local<double>& inMat, 
            const sliced_colmajor_matrix_local<double>& outMat,
            double RCOND);

// ================= GELSD---PROTOTYPE-II =================

  template<class T>
  int gelsd(const sliced_colmajor_matrix_local<T>& inMat, 
            const sliced_colmajor_matrix_local<T>& outMat,
            std::vector<T>& S_VAL,
            int& RANK,
            T RCOND = -1);

  template<>
  int gelsd(const sliced_colmajor_matrix_local<float>& inMat,
            const sliced_colmajor_matrix_local<float>& outMat,
            std::vector<float>& S_VAL,
            int& RANK,
            float RCOND);

  template<>
  int gelsd(const sliced_colmajor_matrix_local<double>& inMat,
            const sliced_colmajor_matrix_local<double>& outMat,
            std::vector<double>& S_VAL,
            int& RANK,
            double RCOND);

// ================= GEEV---PROTOTYPE-I ==================

  template<class T>
  int geev(const sliced_colmajor_matrix_local<T>& inMat, 
           std::vector<T>& E_VAL);
  
  template<>
  int geev(const sliced_colmajor_matrix_local<float>& inMat, 
           std::vector<float>& E_VAL);

  template<>
  int geev(const sliced_colmajor_matrix_local<double>& inMat, 
           std::vector<double>& E_VAL);

// ================ GEEV---PROTOTYPE-II =================
  
  template<class T>
  int geev(const sliced_colmajor_matrix_local<T>& inMat, 
           std::vector<T>& E_VAL,
           const sliced_colmajor_matrix_local<T>& E_VEC,
           char JOBZ = 'L');

  template<>
  int geev(const sliced_colmajor_matrix_local<float>& inMat, 
           std::vector<float>& E_VAL,
           const sliced_colmajor_matrix_local<float>& E_VEC,
           char JOBZ);

  template<>
  int geev(const sliced_colmajor_matrix_local<double>& inMat, 
           std::vector<double>& E_VAL,
           const sliced_colmajor_matrix_local<double>& E_VEC,
           char JOBZ);

// ================ GEEV---PROTOTYPE-III ================

  template<class T>
  int geev(const sliced_colmajor_matrix_local<T>& inMat, 
           std::vector<T>& E_VAL,
           const sliced_colmajor_matrix_local<T>& L_VEC,
           const sliced_colmajor_matrix_local<T>& R_VEC);

  template<>
  int geev(const sliced_colmajor_matrix_local<float>& inMat, 
           std::vector<float>& E_VAL,
           const sliced_colmajor_matrix_local<float>& L_VEC,
           const sliced_colmajor_matrix_local<float>& R_VEC);
  
  template<>
  int geev(const sliced_colmajor_matrix_local<double>& inMat, 
           std::vector<double>& E_VAL,
           const sliced_colmajor_matrix_local<double>& L_VEC,
           const sliced_colmajor_matrix_local<double>& R_VEC );

// ================ GESVD---PROTOTYPE-I =================

  template<class T>
  int gesvd(const sliced_colmajor_matrix_local<T>& inMat, 
            std::vector<T>& S_VAL,
            char OPTION = 'N');

  template<>
  int gesvd(const sliced_colmajor_matrix_local<float>& inMat, 
            std::vector<float>& S_VAL,
            char OPTION);

  template<>
  int gesvd(const sliced_colmajor_matrix_local<double>& inMat, 
            std::vector<double>& S_VAL,
            char OPTION);

// ================ GESVD---PROTOTYPE-II ================

  template<class T>
  int gesvd(const sliced_colmajor_matrix_local<T>& inMat, 
            std::vector<T>& S_VAL,
            const sliced_colmajor_matrix_local<T>& S_VEC,
            char VEC_TYPE = 'L',
            char PART = 'A',
            char OPT_A = 'N');

  template<>
  int gesvd(const sliced_colmajor_matrix_local<float>& inMat, 
            std::vector<float>& S_VAL,
            const sliced_colmajor_matrix_local<float>& S_VEC,
            char VEC_TYPE,
            char PART,
            char OPT_A);

  template<>
  int gesvd(const sliced_colmajor_matrix_local<double>& inMat, 
            std::vector<double>& S_VAL,
            const sliced_colmajor_matrix_local<double>& S_VEC,
            char VEC_TYPE,
            char PART,
            char OPT_A);

// ================ GESVD---PROTOTYPE-III ===============

  template<class T>
  int gesvd(const sliced_colmajor_matrix_local<T>& inMat, 
            std::vector<T>& S_VAL,
            const sliced_colmajor_matrix_local<T>& L_VEC,
            const sliced_colmajor_matrix_local<T>& R_VEC,
            char PART_L = 'A',
            char PART_R = 'A');

  template<>
  int gesvd(const sliced_colmajor_matrix_local<float>& inMat, 
            std::vector<float>& S_VAL,
            const sliced_colmajor_matrix_local<float>& L_VEC,
            const sliced_colmajor_matrix_local<float>& R_VEC,
            char PART_L,
            char PART_R);

  template<>
  int gesvd(const sliced_colmajor_matrix_local<double>& inMat, 
            std::vector<double>& S_VAL,
            const sliced_colmajor_matrix_local<double>& L_VEC,
            const sliced_colmajor_matrix_local<double>& R_VEC,
            char PART_L,
            char PART_R);

// ================= GESDD---PROTOTYPE-I ================

  template<class T>
  int gesdd(const sliced_colmajor_matrix_local<T>& inMat, 
            std::vector<T>& S_VAL);

  template<>
  int gesdd(const sliced_colmajor_matrix_local<float>& inMat, 
            std::vector<float>& S_VAL);

  template<>
  int gesdd(const sliced_colmajor_matrix_local<double>& inMat, 
            std::vector<double>& S_VAL);

// ================= GESDD---PROTOTYPE-II ===============

  template<class T>
  int gesdd(const sliced_colmajor_matrix_local<T>& inMat, 
            std::vector<T>& S_VAL,
            const sliced_colmajor_matrix_local<T>& S_VEC);

  template<>
  int gesdd(const sliced_colmajor_matrix_local<float>& inMat, 
            std::vector<float>& S_VAL,
            const sliced_colmajor_matrix_local<float>& S_VEC);

  template<>
  int gesdd(const sliced_colmajor_matrix_local<double>& inMat, 
            std::vector<double>& S_VAL,
            const sliced_colmajor_matrix_local<double>& S_VEC);

// ================= GESDD---PROTOTYPE-III ==============

  template<class T>
  int gesdd(const sliced_colmajor_matrix_local<T>& inMat, 
            std::vector<T>& S_VAL,
            const sliced_colmajor_matrix_local<T>& L_VEC,
            const sliced_colmajor_matrix_local<T>& R_VEC,
            char PART_LR = 'A');

  template<>
  int gesdd(const sliced_colmajor_matrix_local<float>& inMat, 
            std::vector<float>& S_VAL,
            const sliced_colmajor_matrix_local<float>& L_VEC,
            const sliced_colmajor_matrix_local<float>& R_VEC,
            char PART_LR);

  template<>
  int gesdd(const sliced_colmajor_matrix_local<double>& inMat, 
            std::vector<double>& S_VAL,
            const sliced_colmajor_matrix_local<double>& L_VEC,
            const sliced_colmajor_matrix_local<double>& R_VEC,
            char PART_LR);

//================== GETRF ===============

  template<class T>
  int getrf(const sliced_colmajor_matrix_local<T>& inMat,
            std::vector<int>& IPIV);

  template<>
  int getrf(const sliced_colmajor_matrix_local<float>& inMat,
            std::vector<int>& IPIV);

  template<>
  int getrf(const sliced_colmajor_matrix_local<double>& inMat,
            std::vector<int>& IPIV );
 
// ==================== GETRI ===========================

  template<class T>
  int getri(const sliced_colmajor_matrix_local<T>& inMat,
            const std::vector<int>& IPIV);

  template<>
  int getri(const sliced_colmajor_matrix_local<float>& inMat,
            const std::vector<int>& IPIV);

  template<>
  int getri(const sliced_colmajor_matrix_local<double>& inMat,
            const std::vector<int>& IPIV);

// ================= INV ==================

  // When argument of inv() is a rValue
  template <class T>
  colmajor_matrix_local<T>
  inv(const colmajor_matrix_local<T>& inMat) {

    if(inMat.local_num_row != inMat.local_num_col)
      REPORT_ERROR(USER_ERROR, "Input matrix is not a square matrix!!\n");

    // rValue: Instead of copying input matrix, moving its contents
    colmajor_matrix_local<T> ret; 
    ret.val.swap(inMat.val);
    ret.local_num_row = inMat.local_num_row;
    ret.local_num_col = inMat.local_num_col;

    int INFO = 0;
    std::vector<int> IPIV;

    INFO = getrf<T>(ret,IPIV);
    if(INFO)
      REPORT_ERROR(INTERNAL_ERROR, "LU factorization failed!!\n");

    INFO = getri<T>(ret,IPIV);
    if(INFO)
      REPORT_ERROR(INTERNAL_ERROR, "Matrix inversion failed!!\n");

    return ret;
  }

  // When argument of inv() is a lValue
  template <class T>
  colmajor_matrix_local<T>
  inv(colmajor_matrix_local<T>& inMat) {
    // lValue: Input matrix will be copied, before the inverse operation
    sliced_colmajor_matrix_local<T> m2(inMat);
    return inv(m2);
  }

  template <class T>
  colmajor_matrix_local<T>
  inv(const sliced_colmajor_matrix_local<T>& inMat) {

    if(!inMat.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");
    
    if(inMat.sliced_num_row != inMat.sliced_num_col)
      REPORT_ERROR(USER_ERROR, "Input matrix is not a square matrix!!\n");

    size_t N = inMat.sliced_num_row; // inMat.sliced_num_col;
    colmajor_matrix_local<T> ret(N,N);

    T *retp = &ret.val[0];
    const T *valp = inMat.data;

    // copying input matrix
    for(size_t i=0; i<N; i++)
      for(size_t j=0; j<N; j++)
         retp[i*N+j] = valp[i*inMat.ldm+j];

    int INFO = 0;
    std::vector<int> IPIV;

    INFO = getrf<T>(ret,IPIV);
    if(INFO)
      REPORT_ERROR(INTERNAL_ERROR, "LU factorization failed!!\n");

    INFO = getri<T>(ret,IPIV);
    if(INFO)
      REPORT_ERROR(INTERNAL_ERROR, "Matrix inversion failed!!\n");

    return ret;
  }

// ================ GETRS =================

  template<class T>
  int getrs(const sliced_colmajor_matrix_local<T>& inMat,
            const sliced_colmajor_matrix_local<T>& outMat,
            const std::vector<int>& IPIV,
            char TRANS = 'N');

  template<>
  int getrs(const sliced_colmajor_matrix_local<float>& inMat,
            const sliced_colmajor_matrix_local<float>& outMat,
            const std::vector<int>& IPIV,
            char TRANS);

  template<>
  int getrs(const sliced_colmajor_matrix_local<double>& inMat,
            const sliced_colmajor_matrix_local<double>& outMat,
            const std::vector<int>& IPIV,
            char TRANS);

// ================= GELSY---PROTOTYPE-I ================
  template<class T>
  int gelsy(const sliced_colmajor_matrix_local<T>& inMat, 
            const sliced_colmajor_matrix_local<T>& outMat,
	    T RCOND = -1);

  template<>
  int gelsy(const sliced_colmajor_matrix_local<float>& inMat, 
            const sliced_colmajor_matrix_local<float>& outMat,
	    float RCOND);

  template<>
  int gelsy(const sliced_colmajor_matrix_local<double>& inMat, 
            const sliced_colmajor_matrix_local<double>& outMat,
	    double RCOND);

// ================= GELSY---PROTOTYPE-II ================

  template<class T>
  int gelsy(const sliced_colmajor_matrix_local<T>& inMat, 
            const sliced_colmajor_matrix_local<T>& outMat,
            int& RANK,
            T RCOND = -1);

  template<>
  int gelsy(const sliced_colmajor_matrix_local<float>& inMat, 
            const sliced_colmajor_matrix_local<float>& outMat,
            int& RANK,
            float RCOND);

  template<>
  int gelsy(const sliced_colmajor_matrix_local<double>& inMat, 
            const sliced_colmajor_matrix_local<double>& outMat,
            int& RANK,
            double RCOND);

// ================= GELSS---PROTOTYPE-I ================

  template<class T>
  int gelss(const sliced_colmajor_matrix_local<T>& inMat, 
            const sliced_colmajor_matrix_local<T>& outMat,
            T RCOND = -1);

  template<>
  int gelss(const sliced_colmajor_matrix_local<float>& inMat, 
            const sliced_colmajor_matrix_local<float>& outMat,
            float RCOND);

  template<>
  int gelss(const sliced_colmajor_matrix_local<double>& inMat, 
            const sliced_colmajor_matrix_local<double>& outMat,
            double RCOND);

// ================ GELSS---PROTOTYPE-II ================

  template<class T>
  int gelss(const sliced_colmajor_matrix_local<T>& inMat, 
            const sliced_colmajor_matrix_local<T>& outMat,
            std::vector<T>& S_VAL,
            int& RANK,
            T RCOND = -1);

  template<>
  int gelss(const sliced_colmajor_matrix_local<float>& inMat, 
            const sliced_colmajor_matrix_local<float>& outMat,
            std::vector<float>& S_VAL,
            int& RANK,
            float RCOND);

  template<>
  int gelss(const sliced_colmajor_matrix_local<double>& inMat, 
            const sliced_colmajor_matrix_local<double>& outMat,
            std::vector<double>& S_VAL,
            int& RANK,
            double RCOND);
}

#endif
