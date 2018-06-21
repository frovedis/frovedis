#ifndef _SCALAPACK_REDIST_
#define _SCALAPACK_REDIST_

extern "C" {
void psgemr2d_(int* M_A, int* N_A, 
               float* A, int* I_A, int* J_A, 
               int* DESC_A,
               float* B, int* I_B, int* J_B, 
               int* DESC_B, int* ICONTEXT);

void pdgemr2d_(int* M_A, int* N_A, 
               double* A, int* I_A, int* J_A, 
               int* DESC_A,
               double* B, int* I_B, int* J_B, 
               int* DESC_B, int* ICONTEXT);

void psgeadd_(char* TRANS,
              int* M, int* N,
              float* ALPHA,
              float* A, int* IA, int* JA, int* DESCA,
              float* BETA,
              float* B, int* IB, int* JB, int* DESCB);

void pdgeadd_(char* TRANS,
              int* M, int* N,
              double* ALPHA,
              double* A, int* IA, int* JA, int* DESCA,
              double* BETA,
              double* B, int* IB, int* JB, int* DESCB);
}

namespace frovedis {

template <class T>
void scalapack_redist(int* NROW, int* NCOL, 
                      T* A, int* IA, int* JA, 
                      int* DESCA,
                      T* B, int* IB, int* JB, 
                      int* DESCB, int* ICONTEXT);

template <>
void scalapack_redist(int* NROW, int* NCOL, 
                      float* A, int* IA, int* JA, 
                      int* DESCA,
                      float* B, int* IB, int* JB, 
                      int* DESCB, int* ICONTEXT);
 
template <>
void scalapack_redist(int* NROW, int* NCOL, 
                      double* A, int* IA, int* JA, 
                      int* DESCA,
                      double* B, int* IB, int* JB, 
                      int* DESCB, int* ICONTEXT); 

template <class T>
void geadd_wrapper(char *t, int* m, int* n, T* al,
                   T* aptr, int* IA, int* JA, int* descA,
                   T* be,
                   T* bptr, int* IB, int* JB, int* descB);

template <>
void geadd_wrapper(char *t, int* m, int* n, float* al,
                   float* aptr, int* IA, int* JA, int* descA,
                   float* be,
                   float* bptr, int* IB, int* JB, int* descB); 

template <>
void geadd_wrapper(char *t, int* m, int* n, double* al,
                   double* aptr, int* IA, int* JA, int* descA,
                   double* be,
                   double* bptr, int* IB, int* JB, int* descB); 

}
#endif
