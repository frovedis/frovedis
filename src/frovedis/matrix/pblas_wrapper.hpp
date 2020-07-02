#ifndef _PBLAS_WRAPPER_
#define _PBLAS_WRAPPER_

#include "sliced_vector.hpp"

// ======== PBLAS Interface Prototypes ========

extern "C" {

void psswap_(int* N,
             float* X, int* IX, int* JX, int* DESCX, int* INCX,
	     float* Y, int* IY, int* JY, int* DESCY, int* INCY);

void pdswap_(int* N,
             double* X, int* IX, int* JX, int* DESCX, int* INCX,
	     double* Y, int* IY, int* JY, int* DESCY, int* INCY);

void pscopy_(int* N,
             float* X, int* IX, int* JX, int* DESCX, int* INCX,
	     float* Y, int* IY, int* JY, int* DESCY, int* INCY);

void pdcopy_(int* N,
             double* X, int* IX, int* JX, int* DESCX, int* INCX,
	     double* Y, int* IY, int* JY, int* DESCY, int* INCY);

void psscal_(int* N, 
             float* ALPHA,
             float* X, int* IX, int* JX, int* DESCX, int* INCX);

void pdscal_(int* N, 
             double* ALPHA,
             double* X, int* IX, int* JX, int* DESCX, int* INCX);

void psaxpy_(int* N, float* ALPHA,
             float* X, int* IX, int* JX, int* DESCX, int* INCX,
	     float* Y, int* IY, int* JY, int* DESCY, int* INCY);

void pdaxpy_(int* N, double* ALPHA,
             double* X, int* IX, int* JX, int* DESCX, int* INCX,
	     double* Y, int* IY, int* JY, int* DESCY, int* INCY);

void psdot_(int* N, float* DOT,
            float* X, int* IX, int* JX, int* DESCX, int* INCX,
	    float* Y, int* IY, int* JY, int* DESCY, int* INCY);

void pddot_(int* N, double* DOT,
            double* X, int* IX, int* JX, int* DESCX, int* INCX,
	    double* Y, int* IY, int* JY, int* DESCY, int* INCY);

void psnrm2_(int* N, 
             float* NORM2,
             float* X, int* IX, int* JX, int* DESCX, int* INCX);

void pdnrm2_(int* N, 
             double* NORM2,
             double* X, int* IX, int* JX, int* DESCX, int* INCX);

void psgemv_(char* TRANS, 
             int* M,  int* N,  
             float* ALPHA,
             float* A,    int* IA, int* JA, int* DESCA,
             float* X,    int* IX, int* JX, int* DESCX, int* INCX,
    	     float* BETA,
	     float* Y,    int* IY, int* JY, int* DESCY, int* INCY);

void pdgemv_(char* TRANS, 
             int* M,  int* N,  
	     double* ALPHA,
             double* A,   int* IA, int* JA, int* DESCA,
             double* X,   int* IX, int* JX, int* DESCX, int* INCX,
	     double* BETA,
	     double* Y,   int* IY, int* JY, int* DESCY, int* INCY);

void psger_(int* M,   int* N,  
            float* ALPHA,
            float* X, int* IX, int* JX, int* DESCX, int* INCX,
	    float* Y, int* IY, int* JY, int* DESCY, int* INCY,
            float* A, int* IA, int* JA, int* DESCA);

void pdger_(int* M,   int* N,  
            double* ALPHA,
            double* X, int* IX, int* JX, int* DESCX, int* INCX,
	    double* Y, int* IY, int* JY, int* DESCY, int* INCY,
            double* A, int* IA, int* JA, int* DESCA);

void psgemm_(char* TRANSA, char* TRANSB,
             int* M, int* N, int* K, 
	     float* ALPHA,
             float* A, int* IA, int* JA, int* DESCA,
             float* B, int* IB, int* JB, int* DESCB,
	     float* BETA,
             float* C, int* IC, int* JC, int* DESCC);

void pdgemm_(char* TRANSA, char* TRANSB,
             int* M, int* N, int* K, 
	     double* ALPHA,
	     double* A, int* IA, int* JA, int* DESCA,
             double* B, int* IB, int* JB, int* DESCB,
	     double* BETA,
             double* C, int* IC, int* JC, int* DESCC);

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

// ======== Node-Local Parallel Interface Prototypes ========

template <class T>
void pswap (sliced_blockcyclic_vector_local<T>& vec1,
            sliced_blockcyclic_vector_local<T>& vec2);

template <>
void pswap (sliced_blockcyclic_vector_local<float>& vec1,
            sliced_blockcyclic_vector_local<float>& vec2);

template <>
void pswap (sliced_blockcyclic_vector_local<double>& vec1,
            sliced_blockcyclic_vector_local<double>& vec2);

template <class T>
void pcopy (sliced_blockcyclic_vector_local<T>& vec1,
            sliced_blockcyclic_vector_local<T>& vec2);

template <>
void pcopy (sliced_blockcyclic_vector_local<float>& vec1,
            sliced_blockcyclic_vector_local<float>& vec2);

template <>
void pcopy (sliced_blockcyclic_vector_local<double>& vec1,
            sliced_blockcyclic_vector_local<double>& vec2);

template <class T>
void pscal (sliced_blockcyclic_vector_local<T>& vec,
            T ALPHA);

template <>
void pscal (sliced_blockcyclic_vector_local<float>& vec,
            float ALPHA);

template <>
void pscal (sliced_blockcyclic_vector_local<double>& vec,
            double ALPHA);

template <class T>
void paxpy (sliced_blockcyclic_vector_local<T>& inVec,
            sliced_blockcyclic_vector_local<T>& outVec,
	    T ALPHA);

template <>
void paxpy (sliced_blockcyclic_vector_local<float>& inVec,
            sliced_blockcyclic_vector_local<float>& outVec,
	    float ALPHA);

template <>
void paxpy (sliced_blockcyclic_vector_local<double>& inVec,
            sliced_blockcyclic_vector_local<double>& outVec,
	    double ALPHA);

template <class T>
T pdot  (sliced_blockcyclic_vector_local<T>& inVec1,
         sliced_blockcyclic_vector_local<T>& inVec2);

template <>
float pdot  (sliced_blockcyclic_vector_local<float>& inVec1,
             sliced_blockcyclic_vector_local<float>& inVec2);

template <>
double pdot (sliced_blockcyclic_vector_local<double>& inVec1,
             sliced_blockcyclic_vector_local<double>& inVec2);

template <class T>
T pnrm2  (sliced_blockcyclic_vector_local<T>& inVec);

template <>
float pnrm2  (sliced_blockcyclic_vector_local<float>& inVec);

template <>
double pnrm2 (sliced_blockcyclic_vector_local<double>& inVec);


template <class T>
struct pgemv{
  pgemv() : TRANS('N'), ALPHA(1.0), BETA(0.0) {}
  pgemv(char t, T al, T be) : TRANS(t), ALPHA(al), BETA(be) {} 

  void operator() (sliced_blockcyclic_matrix_local<T>& inMat,
                   sliced_blockcyclic_vector_local<T>& inVec,
		   sliced_blockcyclic_vector_local<T>& outVec);

  char TRANS;
  T ALPHA, BETA;
  SERIALIZE(TRANS, ALPHA, BETA)
};

template <> 
void pgemv<float>::operator() (sliced_blockcyclic_matrix_local<float>& inMat,
                               sliced_blockcyclic_vector_local<float>& inVec,
                               sliced_blockcyclic_vector_local<float>& outVec); 
template <> 
void pgemv<double>::operator() (sliced_blockcyclic_matrix_local<double>& inMat,
                                sliced_blockcyclic_vector_local<double>& inVec,
                                sliced_blockcyclic_vector_local<double>& outVec); 


template <class T>
struct pger {
  pger()     : ALPHA(1.0) {}
  pger(T al) : ALPHA (al) {}

  void operator() (sliced_blockcyclic_vector_local<T>& inVec1,
                   sliced_blockcyclic_vector_local<T>& inVec2,
		   sliced_blockcyclic_matrix_local<T>& outMat);

  T ALPHA;
  SERIALIZE(ALPHA)
};

template <> 
void pger<float>::operator() (sliced_blockcyclic_vector_local<float>& inVec1,
                              sliced_blockcyclic_vector_local<float>& inVec2,
                              sliced_blockcyclic_matrix_local<float>& outMat);
template <> 
void pger<double>::operator() (sliced_blockcyclic_vector_local<double>& inVec1,
                               sliced_blockcyclic_vector_local<double>& inVec2,
                               sliced_blockcyclic_matrix_local<double>& outMat);


template <class T>
struct pgemm {
  pgemm() : TRANS_M1('N'), TRANS_M2('N'), ALPHA(1.0), BETA(0.0) {}

  pgemm(char t1, char t2, T al, T be) : TRANS_M1(t1), TRANS_M2(t2),
                                        ALPHA(al),    BETA(be) {}

  void operator() (sliced_blockcyclic_matrix_local<T>& inMat1,
                   sliced_blockcyclic_matrix_local<T>& inMat2,
		   sliced_blockcyclic_matrix_local<T>& outMat);

  char TRANS_M1, TRANS_M2;
  T ALPHA, BETA;
  SERIALIZE(TRANS_M1, TRANS_M2, ALPHA, BETA)
};

template <>
void pgemm<float>::operator() (sliced_blockcyclic_matrix_local<float>& inMat1,
                               sliced_blockcyclic_matrix_local<float>& inMat2,
                               sliced_blockcyclic_matrix_local<float>& outMat);
template <>
void pgemm<double>::operator() (sliced_blockcyclic_matrix_local<double>& inMat1,
                                sliced_blockcyclic_matrix_local<double>& inMat2,
                                sliced_blockcyclic_matrix_local<double>& outMat);


template <class T>
struct pgeadd {
  pgeadd() : TRANS('N'), ALPHA(1.0), BETA(1.0) {}

  pgeadd(char t, T al, T be) : TRANS(t), ALPHA(al), BETA(be) {}

  void operator() (sliced_blockcyclic_matrix_local<T>& inMat,
		   sliced_blockcyclic_matrix_local<T>& outMat);

  char TRANS;
  T ALPHA, BETA;
  SERIALIZE(TRANS, ALPHA, BETA)
};

template <> 
void pgeadd<float>::operator() (sliced_blockcyclic_matrix_local<float>& inMat,
                                sliced_blockcyclic_matrix_local<float>& outMat);
template <> 
void pgeadd<double>::operator() (sliced_blockcyclic_matrix_local<double>& inMat,
                                 sliced_blockcyclic_matrix_local<double>& outMat);


// ======== User Interface Prototypes ========

template <class T>
void swap (const sliced_blockcyclic_vector<T>& vec1,
           const sliced_blockcyclic_vector<T>& vec2) {
   auto& v1 = const_cast<sliced_blockcyclic_vector<T>&> (vec1);
   auto& v2 = const_cast<sliced_blockcyclic_vector<T>&> (vec2);
   v1.data.mapv(pswap<T>, v2.data);
}

template <class T>
void copy (const sliced_blockcyclic_vector<T>& vec1,
           const sliced_blockcyclic_vector<T>& vec2) {
   auto& v1 = const_cast<sliced_blockcyclic_vector<T>&> (vec1);
   auto& v2 = const_cast<sliced_blockcyclic_vector<T>&> (vec2);
   v1.data.mapv(pcopy<T>, v2.data);
}

template <class T>
void scal (const sliced_blockcyclic_vector<T>& vec,
           T ALPHA) {
   auto& v = const_cast<sliced_blockcyclic_vector<T>&> (vec);
   auto dist_alpha = make_node_local_broadcast(ALPHA);
   v.data.mapv(pscal<T>, dist_alpha);
}

template <class T>
void axpy (const sliced_blockcyclic_vector<T>& inVec,
           const sliced_blockcyclic_vector<T>& outVec,
	   T ALPHA = 1.0) {
   auto& v1 = const_cast<sliced_blockcyclic_vector<T>&> (inVec);
   auto& v2 = const_cast<sliced_blockcyclic_vector<T>&> (outVec);
   auto dist_alpha = make_node_local_broadcast(ALPHA);
   v1.data.mapv(paxpy<T>, v2.data, dist_alpha);
}

template <class T>
T dot (const sliced_blockcyclic_vector<T>& inVec1,
       const sliced_blockcyclic_vector<T>& inVec2) {
   auto& v1 = const_cast<sliced_blockcyclic_vector<T>&> (inVec1);
   auto& v2 = const_cast<sliced_blockcyclic_vector<T>&> (inVec2);
   auto gathered_dot = v1.data.map(pdot<T>, v2.data).gather();
   T ret_dot = 0;

   for(auto i : gathered_dot) {
     if (i != 0) {
       ret_dot = i;
       break;
     }
   }

   return ret_dot;
}

template <class T>
T nrm2 (const sliced_blockcyclic_vector<T>& inVec) {
   auto& v = const_cast<sliced_blockcyclic_vector<T>&> (inVec);
   auto gathered_norm = v.data.map(pnrm2<T>).gather();
   T ret_norm = 0;

   for(auto i : gathered_norm) {
     if (i != 0) {
       ret_norm = i;
       break;
     }
   }

   return ret_norm;
}

template <class T>
void gemv(const sliced_blockcyclic_matrix<T>& inMat,
          const sliced_blockcyclic_vector<T>& inVec,
          const sliced_blockcyclic_vector<T>& outVec,
          char TRANS = 'N',
          T ALPHA = 1.0,
          T BETA = 0.0) {
   auto& m  = const_cast<sliced_blockcyclic_matrix<T>&> (inMat);
   auto& v1 = const_cast<sliced_blockcyclic_vector<T>&> (inVec);
   auto& v2 = const_cast<sliced_blockcyclic_vector<T>&> (outVec);
   m.data.mapv(pgemv<T>(TRANS, ALPHA, BETA), v1.data, v2.data);
}

template <class T>
void ger(const sliced_blockcyclic_vector<T>& inVec1,
         const sliced_blockcyclic_vector<T>& inVec2,
         const sliced_blockcyclic_matrix<T>& outMat,
         T ALPHA = 1.0) {
   auto& v1 = const_cast<sliced_blockcyclic_vector<T>&> (inVec1);
   auto& v2 = const_cast<sliced_blockcyclic_vector<T>&> (inVec2);
   auto& m  = const_cast<sliced_blockcyclic_matrix<T>&> (outMat);
   v1.data.mapv(pger<T>(ALPHA), v2.data, m.data);
}

// ======== p[sd]gemm wrapper routine ========
template <class T>
void gemm(const sliced_blockcyclic_matrix<T>& inMat1,
          const sliced_blockcyclic_matrix<T>& inMat2,
          const sliced_blockcyclic_matrix<T>& outMat,
          char TRANS_M1 = 'N',
          char TRANS_M2 = 'N',
          T ALPHA = 1.0,
          T BETA = 0.0) {
   auto &m1 = const_cast<sliced_blockcyclic_matrix<T>&> (inMat1);
   auto &m2 = const_cast<sliced_blockcyclic_matrix<T>&> (inMat2);
   auto &m3 = const_cast<sliced_blockcyclic_matrix<T>&> (outMat);
   m1.data.mapv(pgemm<T>(TRANS_M1, TRANS_M2, ALPHA, BETA),
                m2.data, m3.data);
}

// ======== Overloaded operator * ========
// This routine can be used to perform the below operation:
//   (*) sliced_blockcyclic_matrix * sliced_blockcyclic_matrix
template <class T>
blockcyclic_matrix<T>
operator*(const sliced_blockcyclic_matrix<T>& inMat1,
          const sliced_blockcyclic_matrix<T>& inMat2) {
   auto &m1 = const_cast<sliced_blockcyclic_matrix<T>&> (inMat1);
   auto &m2 = const_cast<sliced_blockcyclic_matrix<T>&> (inMat2);
  
   size_t nrowa = m1.data.map(get_local_num_row<T>).get(0);
   size_t ncola = m1.data.map(get_local_num_col<T>).get(0);
   size_t nrowb = m2.data.map(get_local_num_row<T>).get(0);
   size_t ncolb = m2.data.map(get_local_num_col<T>).get(0);

   if(ncola != nrowb)
     REPORT_ERROR(USER_ERROR,
           "Incompatible input sizes: Matrix-multiplication not possible!!\n");

   blockcyclic_matrix<T> outMat(nrowa, ncolb);
   sliced_blockcyclic_matrix<T> m3(outMat);
   m1.data.mapv(pgemm<T>('N', 'N', 1.0, 0.0), m2.data, m3.data);
   return outMat;  
}

// This routine can be used to perform the below operation:
//   (*) sliced_blockcyclic_matrix * blockcyclic_matrix
template <class T>
blockcyclic_matrix<T>
operator*(const sliced_blockcyclic_matrix<T>& inMat1,
          const blockcyclic_matrix<T>& inMat2) {
  sliced_blockcyclic_matrix<T> m2(inMat2);
  return inMat1 * m2;
}

// This routine can be used to perform the below operation:
//   (*) blockcyclic_matrix * sliced_blockcyclic_matrix
template <class T>
blockcyclic_matrix<T>
operator*(const blockcyclic_matrix<T>& inMat1,
          const sliced_blockcyclic_matrix<T>& inMat2) {
  sliced_blockcyclic_matrix<T> m1(inMat1);
  return m1 * inMat2;
}

// This routine can be used to perform the below operation:
//   (*) blockcyclic_matrix * blockcyclic_matrix
template <class T>
blockcyclic_matrix<T>
operator*(const blockcyclic_matrix<T>& inMat1,
          const blockcyclic_matrix<T>& inMat2) {
  sliced_blockcyclic_matrix<T> m1(inMat1);
  sliced_blockcyclic_matrix<T> m2(inMat2);
  return m1 * m2;
}

// ======== p[sd]geadd wrapper routine ========
template <class T>
void geadd(const sliced_blockcyclic_matrix<T>& inMat,
           const sliced_blockcyclic_matrix<T>& outMat,
           char TRANS = 'N',
           T ALPHA = 1.0,
           T BETA = 1.0) {
   auto &m1 = const_cast<sliced_blockcyclic_matrix<T>&> (inMat);
   auto &m2 = const_cast<sliced_blockcyclic_matrix<T>&> (outMat);
   m1.data.mapv(pgeadd<T>(TRANS,ALPHA,BETA), m2.data);
}

// ======== Overloaded operator + ========
template <class T>
blockcyclic_matrix<T>
add_copy(const sliced_blockcyclic_matrix<T>& inMat1,
         const sliced_blockcyclic_matrix<T>& inMat2) {

   // DEBUG
   //std::cout << "add_copy() is called!!\n";
   auto &m1 = const_cast<sliced_blockcyclic_matrix<T>&> (inMat1);
   auto &m2 = const_cast<sliced_blockcyclic_matrix<T>&> (inMat2);

   size_t nrowa = m1.data.map(get_local_num_row<T>).get(0);
   size_t ncola = m1.data.map(get_local_num_col<T>).get(0);
   size_t nrowb = m2.data.map(get_local_num_row<T>).get(0);
   size_t ncolb = m2.data.map(get_local_num_col<T>).get(0);

   if(nrowa != nrowb || ncola != ncolb)
     REPORT_ERROR(USER_ERROR,
           "Incompatible matrix sizes: Matrix-addition is not possible!!\n");

   blockcyclic_matrix<T> outMat(nrowa, ncola);
   sliced_blockcyclic_matrix<T> m3(outMat);

   // copying m1 in m3 [m3 <= m1]
   m1.data.mapv(pgeadd<T>('N', 1.0, 0.0), m3.data);

   // adding m2 with m3 [m3 <= m2 + m3]
   m2.data.mapv(pgeadd<T>('N', 1.0, 1.0), m3.data);

   return outMat;
}


template <class T>
blockcyclic_matrix<T>
add_nocopy(const blockcyclic_matrix<T>& inMat1,
           const sliced_blockcyclic_matrix<T>& inMat2) {
   
   // DEBUG
   //std::cout << "add_nocopy() is called!!\n";
   blockcyclic_matrix<T> outMat;
   outMat.data = std::move(inMat1.data);
   outMat.set_num(inMat1.num_row, inMat1.num_col);
   sliced_blockcyclic_matrix<T> m1(outMat);
   auto &m2 = const_cast<sliced_blockcyclic_matrix<T>&> (inMat2);   

   size_t nrowa = m1.data.map(get_local_num_row<T>).get(0);
   size_t ncola = m1.data.map(get_local_num_col<T>).get(0);
   size_t nrowb = m2.data.map(get_local_num_row<T>).get(0);
   size_t ncolb = m2.data.map(get_local_num_col<T>).get(0);

   if(nrowa != nrowb || ncola != ncolb)
     REPORT_ERROR(USER_ERROR,
           "Incompatible matrix sizes: Matrix-addition is not possible!!\n");

   // adding m2 with m1 [m1 <= m2 + m1]
   m2.data.mapv(pgeadd<T>('N', 1.0, 1.0), m1.data);

   return outMat;
}


// This routine can be used to perform the below operation:
//   (*) sliced_blockcyclic_matrix + sliced_blockcyclic_matrix
template <class T>
blockcyclic_matrix<T>
operator+ (const sliced_blockcyclic_matrix<T>& inMat1,
           const sliced_blockcyclic_matrix<T>& inMat2) {
  return add_copy<T>(inMat1, inMat2);
}

// This routine can be used to perform the below operation:
//   (*) sliced_blockcyclic_matrix + (lval)blockcyclic_matrix
template <class T>
blockcyclic_matrix<T>
operator+ (const sliced_blockcyclic_matrix<T>& inMat1,
           blockcyclic_matrix<T>& inMat2) {
  return add_copy<T>(inMat1, inMat2);
}

// This routine can be used to perform the below operation:
//   (*) (lVal)blockcyclic_matrix + sliced_blockcyclic_matrix
template <class T>
blockcyclic_matrix<T>
operator+ (blockcyclic_matrix<T>& inMat1,
           const sliced_blockcyclic_matrix<T>& inMat2) {
  return add_copy<T>(inMat1, inMat2);
}

// This routine can be used to perform the below operation:
//   (*) (lVal)blockcyclic_matrix + (lVal)blockcyclic_matrix
template <class T>
blockcyclic_matrix<T>
operator+ (blockcyclic_matrix<T>& inMat1,
           blockcyclic_matrix<T>& inMat2) {
  return add_copy<T>(inMat1, inMat2);
}

// This routine can be used to perform the below operation:
//   (*) sliced_blockcyclic_matrix + (rVal)blockcyclic_matrix
template <class T>
blockcyclic_matrix<T>
operator+ (const sliced_blockcyclic_matrix<T>& inMat1,
           const blockcyclic_matrix<T>& inMat2) {
  return add_nocopy<T>(inMat2,inMat1);
}

// This routine can be used to perform the below operation:
//   (*) (rVal)blockcyclic_matrix + sliced_blockcyclic_matrix
template <class T>
blockcyclic_matrix<T>
operator+ (const blockcyclic_matrix<T>& inMat1,
           const sliced_blockcyclic_matrix<T>& inMat2) {
  return add_nocopy<T>(inMat1,inMat2);
}

// This routine can be used to perform the below operation:
//   (*) (rVal)blockcyclic_matrix + (lVal)blockcyclic_matrix
template <class T>
blockcyclic_matrix<T>
operator+ (const blockcyclic_matrix<T>& inMat1,
           blockcyclic_matrix<T>& inMat2) {
  return add_nocopy<T>(inMat1,inMat2);
}

// This routine can be used to perform the below operation:
//   (*) (lVal)blockcyclic_matrix + (rVal)blockcyclic_matrix
template <class T>
blockcyclic_matrix<T>
operator+ (blockcyclic_matrix<T>& inMat1,
           const blockcyclic_matrix<T>& inMat2) {
  return add_nocopy<T>(inMat2,inMat1);
}

// This routine can be used to perform the below operation:
//   (*) (rVal)blockcyclic_matrix + (rVal)blockcyclic_matrix
template <class T>
blockcyclic_matrix<T>
operator+ (const blockcyclic_matrix<T>& inMat1,
           const blockcyclic_matrix<T>& inMat2) {
  return add_nocopy<T>(inMat1,inMat2);
}


// ======== transpose using p[sd]geadd wrapper routine ========
template <class T>
blockcyclic_matrix<T>
trans(const sliced_blockcyclic_matrix<T>& inMat) {
   auto &m1 = const_cast<sliced_blockcyclic_matrix<T>&> (inMat);
   size_t nrow = m1.data.map(get_local_num_row<T>).get(0);
   size_t ncol = m1.data.map(get_local_num_col<T>).get(0);
   blockcyclic_matrix<T> outMat(ncol,nrow);
   sliced_blockcyclic_matrix<T> m2(outMat);

   // transposing m1 in m2 [m2 <= m1**T]
   m1.data.mapv(pgeadd<T>('T', 1.0, 0.0), m2.data);
   return outMat;
}

// ======== Overloaded operator ~ ========
// transpose of a sliced_blockcyclic_matrix
template <class T>
blockcyclic_matrix<T>
operator~ (const sliced_blockcyclic_matrix<T>& inMat) {
   return trans(inMat);
}

// transpose of a blockcyclic_matrix
template <class T>
blockcyclic_matrix<T>
operator~ (const blockcyclic_matrix<T>& inMat) {
   sliced_blockcyclic_matrix<T> m2(inMat);
   return trans(m2);
}

}
#endif
