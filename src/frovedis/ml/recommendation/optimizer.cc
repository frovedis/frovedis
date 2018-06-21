#if (defined(_SX) || defined(__ve__)) && defined(USE_ASL)
#include <asl.h>
#endif

extern "C" {
  // Driver function to solve system of linear equation: AX=B 
  void dgesv_(int* N, int *NRHS, double* A, int* LDA, int* IPIV, double* B,
              int* LDB, int* INFO);
  void sgesv_(int* N, int *NRHS, float* A, int* LDA, int* IPIV, float* B,
              int* LDB, int* INFO);

  // perform multiplication of two matrices
  void dgemm_(const char *TRANSA, const char *TRANSB, const int *M,
              const int *N, const int *K, double *ALPHA, double *A,
              const int *LDA, double *B, const int *LDB, double *BETA,
              double *C, const int *LDC);
  void sgemm_(const char *TRANSA, const char *TRANSB, const int *M,
              const int *N, const int *K, float *ALPHA, float *A,
              const int *LDA, float *B, const int *LDB, float *BETA,
              float *C, const int *LDC);
}

namespace frovedis {

template <class T>
void als_typed_gemm(const char *TRANSA, const char *TRANSB, const int *M,
                    const int *N, const int *K, T *ALPHA, T *A,
                    const int *LDA, T *B, const int *LDB, T *BETA,
                    T *C, const int *LDC);

template<>
void als_typed_gemm(const char *TRANSA, const char *TRANSB, const int *M,
                    const int *N, const int *K, double *ALPHA, double *A,
                    const int *LDA, double *B, const int *LDB,
                    double *BETA, double *C, const int *LDC) {
  dgemm_(TRANSA, TRANSB, M, N, K, ALPHA, A, LDA, B, LDB, BETA, C, LDC);
}

template<>
void als_typed_gemm(const char *TRANSA, const char *TRANSB, const int *M,
                    const int *N, const int *K, float *ALPHA, float *A,
                    const int *LDA, float *B, const int *LDB,
                    float *BETA, float *C, const int *LDC) {
  sgemm_(TRANSA, TRANSB, M, N, K, ALPHA, A, LDA, B, LDB, BETA, C, LDC);
}

template <class T>
void als_typed_gesv(int* N, int *NRHS, T* A, int* LDA, int* IPIV, T* B,
                    int* LDB, int* INFO);

template<>
void als_typed_gesv(int* N, int *NRHS, double* A, int* LDA, int* IPIV, 
                    double* B, int* LDB, int* INFO) {
  dgesv_(N, NRHS, A, LDA, IPIV, B, LDB, INFO);
}

template<>
void als_typed_gesv(int* N, int *NRHS, float* A, int* LDA, int* IPIV, float* B,
                    int* LDB, int* INFO) {
  sgesv_(N, NRHS, A, LDA, IPIV, B, LDB, INFO);
}

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
                     double* w) {
  return ASL_dam1ms(a,lma,m,n,b,lnb,k,c,lmc,w);
}

template<>
int als_typed_am1ms(float* a, int lma, int m, int n,
                    float* b, int lnb, int k,
                    float* c, int lmc,
                    float* w) {
  return ASL_ram1ms(a,lma,m,n,b,lnb,k,c,lmc,w);
}

template <class T>
int als_typed_bgmsl(T* a, int lna, int n,
                    T* b, int* ipvt);

template <>
int als_typed_bgmsl(double* a, int lna, int n,
                    double* b, int* ipvt) {
  return ASL_dbgmsl(a,lna,n,b,ipvt);
}

template <>
int als_typed_bgmsl(float* a, int lna, int n,
                    float* b, int* ipvt) {
  return ASL_rbgmsl(a,lna,n,b,ipvt);
}

#endif
}
