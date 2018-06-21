#include "scalapack_redist.hpp"

namespace frovedis {

template <>
void scalapack_redist(int* NROW, int* NCOL, 
                      float* A, int* IA, int* JA, 
                      int* DESCA,
                      float* B, int* IB, int* JB, 
                      int* DESCB, int* ICONTEXT) {

   psgemr2d_(NROW, NCOL, 
             A, IA, JA, DESCA, 
             B, IB, JB, DESCB, ICONTEXT);
}


template <>
void scalapack_redist(int* NROW, int* NCOL, 
                      double* A, int* IA, int* JA, 
                      int* DESCA,
                      double* B, int* IB, int* JB, 
                      int* DESCB, int* ICONTEXT) {

   pdgemr2d_(NROW, NCOL, 
             A, IA, JA, DESCA, 
             B, IB, JB, DESCB, ICONTEXT);
}

template <>
void geadd_wrapper(char *t, int* m, int* n, float* al,
                   float* aptr, int* IA, int* JA, int* descA,
                   float* be,
                   float* bptr, int* IB, int* JB, int* descB) {
  psgeadd_(t,m,n,
           al,
           aptr,IA,JA,descA,
           be,
           bptr,IB,JB,descB);
}

template <>
void geadd_wrapper(char *t, int* m, int* n, double* al,
                   double* aptr, int* IA, int* JA, int* descA,
                   double* be,
                   double* bptr, int* IB, int* JB, int* descB) {
  pdgeadd_(t,m,n,
           al,
           aptr,IA,JA,descA,
           be,
           bptr,IB,JB,descB);
}

}
