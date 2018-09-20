#ifndef _ALS_OPT_H_
#define _ALS_OPT_H_

#include <iostream>
#include <string>
#include "../../matrix/crs_matrix.hpp"
#include "../../core/exceptions.hpp"

namespace frovedis {

class optimizer {
private:
  size_t factor;
  double alpha;
  double regParam;

public:
  optimizer() : factor(0), alpha(0.01), regParam(0.01) {}
  optimizer(size_t f, double al = 0.01, double regPrm = 0.01) :
    factor(f), alpha(al), regParam(regPrm) {}

  template <class T, class I, class O>
  std::vector<T> 
  optimize (crs_matrix_local<T,I,O> &data, std::vector<T> &inMat);

  template <class T, class I, class O>
  void compute_model_row (const T *inMat, 
                          const T *regMtM, 
                          T *outVec, 
                          crs_matrix_local<T,I,O>& data,
                          size_t current_row,
                          size_t inRows);

  SERIALIZE(factor, alpha, regParam)
};

// typed lapack routines: gemm / gesv
template <class T>
void als_typed_gemm(const char *TRANSA, const char *TRANSB, const int *M,
                    const int *N, const int *K, T *ALPHA, T *A,
                    const int *LDA, T *B, const int *LDB, T *BETA,
                    T *C, const int *LDC);

template<>
void als_typed_gemm(const char *TRANSA, const char *TRANSB, const int *M,
                    const int *N, const int *K, double *ALPHA, double *A,
                    const int *LDA, double *B, const int *LDB,
                    double *BETA, double *C, const int *LDC);

template<>
void als_typed_gemm(const char *TRANSA, const char *TRANSB, const int *M,
                    const int *N, const int *K, float *ALPHA, float *A,
                    const int *LDA, float *B, const int *LDB,
                    float *BETA, float *C, const int *LDC);

template <class T>
void als_typed_gesv(int* N, int *NRHS, T* A, int* LDA, int* IPIV, T* B,
                    int* LDB, int* INFO);

template<>
void als_typed_gesv(int* N, int *NRHS, double* A, int* LDA, int* IPIV,
                    double* B, int* LDB, int* INFO);
                

template<>
void als_typed_gesv(int* N, int *NRHS, float* A, int* LDA, int* IPIV, float* B,
                    int* LDB, int* INFO);

#if (defined(_SX) || defined(__ve__)) && defined(USE_ASL)
template <class T>
int als_typed_am1ms(T* a, int lma, int m, int n,
                    T* b, int lnb, int k,
                    T* c, int lmc,
                    T* w);

template<>
int als_typed_am1ms(double* a, int lma, int m, int n,
                    double* b, int lnb, int k,
                    double* c, int lmc,
                    double* w);

template<>
int als_typed_am1ms(float* a, int lma, int m, int n,
                    float* b, int lnb, int k,
                    float* c, int lmc,
                    float* w);

template <class T>
int als_typed_bgmsl(T* a, int lna, int n,
                    T* b, int* ipvt);

template <>
int als_typed_bgmsl(double* a, int lna, int n,
                    double* b, int* ipvt);

template <>
int als_typed_bgmsl(float* a, int lna, int n,
                    float* b, int* ipvt);

#endif

template <typename T>
void solve_matrix_equation (T *A, int orderA, T *B) {
  ASSERT_PTR(A && B);
  int LDA, LDB, INFO;
  int *IPIV = new int[orderA];
  LDA = orderA;
  LDB = orderA;
  /*
    convert row major to column major
    TODO: make the whole program column major to remove this transpose
   */
  T *transA = new T[orderA*orderA];
  for(int i = 0; i < orderA; i++)
#pragma cdir on_adb(A)
#pragma cdir on_adb(transA)
    for(int j = 0; j < orderA; j++)
      transA[j*orderA+i] = A[i*orderA+j];
#if (defined(_SX) || defined(__ve__)) && defined(USE_ASL)
  int ierr = als_typed_bgmsl(A, orderA, orderA, B, IPIV);
  if(ierr != 0 && ierr != 1000) {
    std::cerr << "ASL_[dr]bgmsl failed: " << ierr << std::endl;
    throw std::runtime_error("ASL_[dr]bgmsl failed");
  }
#else
  int nColsB = 1;
  als_typed_gesv<T>(&orderA, &nColsB, transA, &LDA, IPIV, B, &LDB,
                    &INFO);
  if(INFO) REPORT_ERROR(INTERNAL_ERROR,"Lapack Driver Routine Failed\n");
#endif
    
  delete[] transA;
  delete[] IPIV;
}

//-------------------------------------------------------------------
// Function to optimize the target model 
//-------------------------------------------------------------------
template <class T, class I, class O>
std::vector<T> 
optimizer::optimize(crs_matrix_local<T,I,O>& data,
                    std::vector<T>& inMat) {
  // Calculating number of rows in inMat
  size_t inRows = inMat.size() / factor;
  // number of rows in outMat
  size_t outRows = data.local_num_row;
  std::vector<T> outMat(outRows * factor);
  /*
    Computation of (Mt * M + (regParam*I))
   */
  T *regularizedMtM = new T[inRows*factor];
  char transN = 'N';
  char transT = 'T';
  T gemm_alpha = 1.0; 
  T beta = 0.0;
  int ifactor = factor;
  int iinRows = inRows;
  // currently, matrix is row major, but gemm assumes column major
  // utilizes the fact that At * Bt = (B * A)t
  als_typed_gemm<T>(&transN, &transT, &ifactor, &ifactor, &iinRows,
                    &gemm_alpha, &inMat[0], &ifactor,&inMat[0],
                    &ifactor, &beta, regularizedMtM, &ifactor);
  for (size_t i = 0; i < factor; i++) {	
    regularizedMtM[i*factor+i] += regParam;    // (MtM + (regParam*I))
  }    
  // This loop will compute each row of X or Y matrix 
  for (size_t i = 0; i < outRows; i++) {
     /*   This function will compute outMat, by solving 
      *   the equation = inv(Mt*M + Mt*(C-I)*M + regParam*I) * (Mt*C*P)
      */
    compute_model_row(&inMat[0], regularizedMtM, &outMat[i*factor],
                      data, i, inRows);  
  }
  delete[] regularizedMtM;
  return outMat;
}

template <class T, class I, class O>
void optimizer::compute_model_row(const T *Mp,
                                  const T *regMtMp,
                                  T *outVec,
                                  crs_matrix_local<T,I,O>& data,
                                  size_t current_row,
                                  size_t inRows) {
  ASSERT_PTR(Mp && regMtMp && outVec);

  size_t width = data.off[current_row+1] - data.off[current_row];
  const T* valBaseAddr = &data.val[data.off[current_row]];
  const I* posBaseAddr = &data.idx[data.off[current_row]];

  std::vector<T> transM_CminusI(factor*width);
  T* transM_CminusIp = &transM_CminusI[0];

  std::vector<T> A(factor*factor);
  std::vector<T> B(factor, 0);
  T* Ap = &A[0];
  T* Bp = &B[0];

  /*
    A = MtM + regParam*I
  */
#pragma cdir on_adb(Ap)
  for(size_t i = 0; i < A.size(); i++) {
    Ap[i] = regMtMp[i];
  }
  /*
     create M' (compressed M)
   */
  std::vector<T> Mcmp(width * factor);
  T* Mcmpp = &Mcmp[0];
  for(size_t j = 0; j < width; j++) {
    size_t pos = posBaseAddr[j];
#pragma cdir on_adb(Mcmpp)
    for(size_t i = 0; i < factor; i++) {
      Mcmpp[j * factor + i] = Mp[pos * factor + i];
    }
  }
  /*
    create Mt' * (C-I)', and B = Mt' * C' * p'
    (use M' instead of Mt')
   */
  for(size_t j = 0; j < width; j++) {
#pragma cdir on_adb(Mcmpp)
#pragma cdir on_adb(transM_CminusIp)
#pragma cdir on_adb(Bp)
    for(size_t i = 0; i < factor; i++) {
      transM_CminusIp[i * width + j] =
        Mcmpp[j * factor + i] * valBaseAddr[j] * alpha;
      Bp[i] += Mcmpp[j * factor + i] * (1 + alpha * valBaseAddr[j]); 
    }
  }
  /*
    create (Mt' * (C-I)' * M') + (MtM + regParam*I)
  */
  /* // changed to use gemm
  for(size_t k = 0; k < width; k++) {
    for(size_t i = 0; i < factor; i++) {
#pragma cdir on_adb(Ap)
#pragma cdir on_adb(Mcmpp)
#pragma cdir on_adb(transM_CminusIp)
      for(size_t j = 0; j < factor; j++) {
        Ap[i*factor+j] += transM_CminusIp[i*width+k] * Mcmpp[k*factor+j];
      }
    }
  }
  */
#if (defined(_SX) || defined(__ve__)) && defined(USE_ASL)
  if(width > 0) {
    std::vector<T> w((factor * width +
                      width * factor +
                      factor * factor)/3);
    std::vector<T> Atmp(factor*factor);
    T* wp = &w[0];
    T* Atmpp = &Atmp[0];
    if(width > 0) {
      int ierr = als_typed_am1ms<T>(Mcmpp, factor, factor, width, 
                                    transM_CminusIp, width, factor,
                                    Atmpp, factor, wp);
      if(ierr == 3000) {
        std::cerr << "ASL_[dr]am1ms failed: " << ierr << std::endl;
        throw std::runtime_error("ASL_[dr]am1ms failed");
      }
      for(size_t i = 0; i < factor * factor; i++) Ap[i] += Atmpp[i];
    }
  }
#else
  char transN = 'N';
  T gemm_alpha = 1.0;
  T beta = 1.0;
  int ifactor = factor;
  int iwidth = width;
  if(width > 0)
    als_typed_gemm<T>(&transN, &transN, &ifactor, &ifactor, &iwidth,
                      &gemm_alpha, Mcmpp, &ifactor, transM_CminusIp,
                      &iwidth, &beta, Ap, &ifactor);
#endif
  solve_matrix_equation(Ap,factor,Bp); 
#pragma cdir on_adb(Bp)
  for(size_t i = 0; i < factor; i++) outVec[i] = Bp[i];
}

}
#endif
