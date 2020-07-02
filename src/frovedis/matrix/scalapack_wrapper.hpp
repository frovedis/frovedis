#ifndef _SCALAPACK_WRAPPER_
#define _SCALAPACK_WRAPPER_

#include "sliced_vector.hpp"

// ======== SCALAPACK Interface Prototypes ========

extern "C" {

void psgesv_(int* N, int* NRHS,
             float* A, int* IA, int* JA, int* DESCA,
             int* IPIV,
             float* B, int* IB, int* JB, int* DESCB,
             int* INFO);

void pdgesv_(int* N, int* NRHS,
             double* A, int* IA, int* JA, int* DESCA,
             int* IPIV,
             double* B, int* IB, int* JB, int* DESCB,
             int* INFO);

void psgels_(char* TRANS,
             int* M, int* N, int* NRHS,
             float* A, int* IA, int* JA, int* DESCA,
             float* B, int* IB, int* JB, int* DESCB,
             float* WORK, int* LWORK, int* INFO);

void pdgels_(char* TRANS,
             int* M, int* N, int* NRHS,
             double* A, int* IA, int* JA, int* DESCA,
             double* B, int* IB, int* JB, int* DESCB,
             double* WORK, int* LWORK, int* INFO);

void psgesvd_(char* JOBU, char* JOBVT,
              int* M, int* N,
              float* A, int* IA, int* JA, int* DESCA,
              float* S,
              float* U, int* IU, int* JU, int* DESCU,
              float* VT, int* IVT, int* JVT, int* DESCVT,
              float* WORK, int* LWORK, int* INFO);

void pdgesvd_(char* JOBU, char* JOBVT,
              int* M, int* N,
              double* A, int* IA, int* JA, int* DESCA,
              double* S,
              double* U, int* IU, int* JU, int* DESCU,
              double* VT, int* IVT, int* JVT, int* DESCVT,
              double* WORK, int* LWORK, int* INFO);

void psgetrf_(int* M, int* N,
              float* A, int* IA, int* JA, int* DESCA,
              int* IPIV, int* INFO);

void pdgetrf_(int* M, int* N,
              double* A, int* IA, int* JA, int* DESCA,
              int* IPIV, int* INFO);

void psgetri_(int* N,
              float* A, int* IA, int* JA, int* DESCA,
              int* IPIV,
              float* WORK, int* LWORK,
              int* IWORK,  int* LIWORK,
              int* INFO);

void pdgetri_(int* N,
              double* A, int* IA, int* JA, int* DESCA,
              int* IPIV,
              double* WORK, int* LWORK,
              int* IWORK,  int* LIWORK,
              int* INFO);

void psgetrs_(char* TRANS,
              int* N, int* NRHS,
              float* A, int* IA, int* JA, int* DESCA,
              int* IPIV,
              float* B, int* IB, int* JB, int* DESCB,
              int* INFO);

void pdgetrs_(char* TRANS,
              int* N, int* NRHS,
              double* A, int* IA, int* JA, int* DESCA,
              int* IPIV,
              double* B, int* IB, int* JB, int* DESCB,
              int* INFO);

void pslacpy_(char* UPLO,
              int* M, int* N,
              float* A, int* IA, int* JA, int* DESCA,
              float* B, int* IB, int* JB, int* DESCB);

void pdlacpy_(char* UPLO,
              int* M, int* N,
              double* A, int* IA, int* JA, int* DESCA,
              double* B, int* IB, int* JB, int* DESCB);
}

namespace frovedis {

// ======== Node-Local Parallel Interface Prototypes ========

// -------- PLACPY --------  
template <class T>
void placpy(sliced_blockcyclic_matrix_local<T>& inMat,
            sliced_blockcyclic_matrix_local<T>& outMat,
            char UPLO);

template <>
void placpy(sliced_blockcyclic_matrix_local<float>& inMat,
            sliced_blockcyclic_matrix_local<float>& outMat,
            char UPLO);

template <>
void placpy(sliced_blockcyclic_matrix_local<double>& inMat,
            sliced_blockcyclic_matrix_local<double>& outMat,
            char UPLO);


// -------- PGETRF --------  
template <class T>
int pgetrf(sliced_blockcyclic_matrix_local<T>& inMat,
           std::vector<int>& IPIV);

template <>
int pgetrf(sliced_blockcyclic_matrix_local<float>& inMat,
           std::vector<int>& IPIV);

template <>
int pgetrf(sliced_blockcyclic_matrix_local<double>& inMat,
           std::vector<int>& IPIV);


// -------- PGETRI --------  
template <class T>
int pgetri(sliced_blockcyclic_matrix_local<T>& inMat,
           std::vector<int>& IPIV);

template <>
int pgetri(sliced_blockcyclic_matrix_local<float>& inMat,
           std::vector<int>& IPIV);

template <>
int pgetri(sliced_blockcyclic_matrix_local<double>& inMat,
           std::vector<int>& IPIV);


// -------- PGETRS --------  
template <class T>
int pgetrs(sliced_blockcyclic_matrix_local<T>& inMat,
           sliced_blockcyclic_matrix_local<T>& outMat,
           std::vector<int>& IPIV,
           char TRANS);

template <>
int pgetrs(sliced_blockcyclic_matrix_local<float>& inMat,
           sliced_blockcyclic_matrix_local<float>& outMat,
           std::vector<int>& IPIV,
           char TRANS);

template <>
int pgetrs(sliced_blockcyclic_matrix_local<double>& inMat,
           sliced_blockcyclic_matrix_local<double>& outMat,
           std::vector<int>& IPIV,
           char TRANS);


// -------- PGESV: PROTOTYPE I --------  
template <class T>
int pgesv1(sliced_blockcyclic_matrix_local<T>& inMat,
           sliced_blockcyclic_matrix_local<T>& outMat);

template <>
int pgesv1(sliced_blockcyclic_matrix_local<float>& inMat,
           sliced_blockcyclic_matrix_local<float>& outMat);

template <>
int pgesv1(sliced_blockcyclic_matrix_local<double>& inMat,
           sliced_blockcyclic_matrix_local<double>& outMat);

// -------- PGESV: PROTOTYPE II --------  
template <class T>
int pgesv2(sliced_blockcyclic_matrix_local<T>& inMat,
           sliced_blockcyclic_matrix_local<T>& outMat,
           std::vector<int>& IPIV);

template <>
int pgesv2(sliced_blockcyclic_matrix_local<float>& inMat,
           sliced_blockcyclic_matrix_local<float>& outMat,
           std::vector<int>& IPIV);

template <>
int pgesv2(sliced_blockcyclic_matrix_local<double>& inMat,
           sliced_blockcyclic_matrix_local<double>& outMat,
           std::vector<int>& IPIV);


// -------- PGELS --------  
template <class T>
int pgels(sliced_blockcyclic_matrix_local<T>& inMat,
          sliced_blockcyclic_matrix_local<T>& outMat,
          char TRANS);

template <>
int pgels(sliced_blockcyclic_matrix_local<float>& inMat,
          sliced_blockcyclic_matrix_local<float>& outMat,
          char TRANS);

template <>
int pgels(sliced_blockcyclic_matrix_local<double>& inMat,
          sliced_blockcyclic_matrix_local<double>& outMat,
          char TRANS);


// -------- PGESVD: PROTOTYPE I --------  
template <class T>
int pgesvd1(sliced_blockcyclic_matrix_local<T>& inMat,
            std::vector<T>& S_VAL);

template <>
int pgesvd1(sliced_blockcyclic_matrix_local<float>& inMat,
            std::vector<float>& S_VAL);

template <>
int pgesvd1(sliced_blockcyclic_matrix_local<double>& inMat,
            std::vector<double>& S_VAL);

// -------- PGESVD: PROTOTYPE II --------  
template <class T>
int pgesvd2(sliced_blockcyclic_matrix_local<T>& inMat,
            std::vector<T>& S_VAL,
            sliced_blockcyclic_matrix_local<T>& S_VEC,
            char VEC_TYPE);

template <>
int pgesvd2(sliced_blockcyclic_matrix_local<float>& inMat,
            std::vector<float>& S_VAL,
            sliced_blockcyclic_matrix_local<float>& S_VEC,
            char VEC_TYPE);

template <>
int pgesvd2(sliced_blockcyclic_matrix_local<double>& inMat,
            std::vector<double>& S_VAL,
            sliced_blockcyclic_matrix_local<double>& S_VEC,
            char VEC_TYPE);

// -------- PGESVD: PROTOTYPE III --------  
template <class T>
int pgesvd3(sliced_blockcyclic_matrix_local<T>& inMat,
            std::vector<T>& S_VAL,
            sliced_blockcyclic_matrix_local<T>& L_SVEC,
            sliced_blockcyclic_matrix_local<T>& R_SVEC);

template <>
int pgesvd3(sliced_blockcyclic_matrix_local<float>& inMat,
            std::vector<float>& S_VAL,
            sliced_blockcyclic_matrix_local<float>& L_SVEC,
            sliced_blockcyclic_matrix_local<float>& R_SVEC);

template <>
int pgesvd3(sliced_blockcyclic_matrix_local<double>& inMat,
            std::vector<double>& S_VAL,
            sliced_blockcyclic_matrix_local<double>& L_SVEC,
            sliced_blockcyclic_matrix_local<double>& R_SVEC);



// ======== User Interface Prototypes ========

// -------- GESV: PROTOTYPE I --------  
template <class T>
int gesv(const sliced_blockcyclic_matrix<T>& inMat,
         const sliced_blockcyclic_matrix<T>& outMat) {

  auto& m1 = const_cast<sliced_blockcyclic_matrix<T>&> (inMat);
  auto& m2 = const_cast<sliced_blockcyclic_matrix<T>&> (outMat);
  auto ret = m1.data.map(pgesv1<T>, m2.data).get(0);
  return ret;
}
  
// -------- GESV: PROTOTYPE II --------  
template <class T>
int gesv(const sliced_blockcyclic_matrix<T>& inMat,
         const sliced_blockcyclic_matrix<T>& outMat,
         lvec<int>& IPIV) {

  auto& m1 = const_cast<sliced_blockcyclic_matrix<T>&> (inMat);
  auto& m2 = const_cast<sliced_blockcyclic_matrix<T>&> (outMat);
  IPIV = make_node_local_allocate<std::vector<int>>();
  auto ret = m1.data.map(pgesv2<T>, m2.data, IPIV).get(0);
  return ret;
}
  
  
// -------- GELS --------  
template <class T>
int gels(const sliced_blockcyclic_matrix<T>& inMat,
         const sliced_blockcyclic_matrix<T>& outMat,
         char TRANS = 'N') {

  auto& m1 = const_cast<sliced_blockcyclic_matrix<T>&> (inMat);
  auto& m2 = const_cast<sliced_blockcyclic_matrix<T>&> (outMat);
  auto dist_TRANS = make_node_local_broadcast(TRANS);
  auto ret = m1.data.map(pgels<T>, m2.data, dist_TRANS).get(0);
  return ret;
}


// -------- GESVD: PROTOTYPE I --------  
template <class T>
int gesvd(const sliced_blockcyclic_matrix<T>& inMat,
          std::vector<T>& S_VAL) {

  auto& m1 = const_cast<sliced_blockcyclic_matrix<T>&> (inMat);
  auto dist_SVAL = make_node_local_allocate<std::vector<T>>();
  auto ret = m1.data.map(pgesvd1<T>, dist_SVAL).get(0);
  S_VAL = dist_SVAL.get(0); // global output
  return ret;
}


// -------- GESVD: PROTOTYPE II --------  
template <class T>
int gesvd(const sliced_blockcyclic_matrix<T>& inMat,
          std::vector<T>& S_VAL,
          const sliced_blockcyclic_matrix<T>& S_VEC,
          char VEC_TYPE = 'L') {

  auto& m1 = const_cast<sliced_blockcyclic_matrix<T>&> (inMat);
  auto& sv = const_cast<sliced_blockcyclic_matrix<T>&> (S_VEC);
  auto dist_SVAL = make_node_local_allocate<std::vector<T>>();
  auto dist_VEC_TYPE = make_node_local_broadcast(VEC_TYPE);
  auto ret = m1.data.map(pgesvd2<T>, dist_SVAL, 
                         sv.data, dist_VEC_TYPE).get(0);
  S_VAL = dist_SVAL.get(0); // global output
  return ret;
}


// -------- GESVD: PROTOTYPE III -------- 
template <class T>
int gesvd(const sliced_blockcyclic_matrix<T>& inMat,
          std::vector<T>& S_VAL,
          const sliced_blockcyclic_matrix<T>& L_SVEC,
          const sliced_blockcyclic_matrix<T>& R_SVEC) {

  auto& m1  = const_cast<sliced_blockcyclic_matrix<T>&> (inMat);
  auto& lsv = const_cast<sliced_blockcyclic_matrix<T>&> (L_SVEC);
  auto& rsv = const_cast<sliced_blockcyclic_matrix<T>&> (R_SVEC);
  auto dist_SVAL = make_node_local_allocate<std::vector<T>>();
  auto ret = m1.data.map(pgesvd3<T>, dist_SVAL, 
                         lsv.data, rsv.data).get(0);
  S_VAL = dist_SVAL.get(0); // global output
  return ret;
}


// -------- LACPY -------- 
template <class T>
void lacpy(const sliced_blockcyclic_matrix<T>& inMat,
           const sliced_blockcyclic_matrix<T>& outMat,
           char UPLO = 'A') {
  auto& m1 = const_cast<sliced_blockcyclic_matrix<T>&> (inMat);
  auto& m2 = const_cast<sliced_blockcyclic_matrix<T>&> (outMat);
  auto dist_UPLO = make_node_local_broadcast(UPLO);
  m1.data.mapv(placpy<T>, m2.data, dist_UPLO);
}

 
// -------- GETRF --------  
template <class T>
int getrf(const sliced_blockcyclic_matrix<T>& inMat,
          lvec<int>& IPIV) {

  auto& m1 = const_cast<sliced_blockcyclic_matrix<T>&> (inMat);
  IPIV = make_node_local_allocate<std::vector<int>>();
  auto ret = m1.data.map(pgetrf<T>, IPIV).get(0);
  return ret;
}


// -------- GETRI --------  
template <class T>
int getri(const sliced_blockcyclic_matrix<T>& inMat,
          const lvec<int>& IPIV) {

  auto& m1   = const_cast<sliced_blockcyclic_matrix<T>&> (inMat);
  auto& pivA = const_cast<lvec<int>&> (IPIV);
  auto ret = m1.data.map(pgetri<T>, pivA).get(0);
  return ret;
}


// -------- INV --------  
// When argument is a rValue
template <class T>
blockcyclic_matrix<T>
inv(const blockcyclic_matrix<T>& inMat) {
  
  if(inMat.num_row != inMat.num_col)
    REPORT_ERROR(USER_ERROR, "Input matrix is not a square matrix!!\n");

  //DEBUG
  //std::cout << "rValue version of inv() is called!!\n";

  // rValue: Instead of copying input matrix, moving its contents
  blockcyclic_matrix<T> outMat;
  outMat.data = std::move(inMat.data);
  outMat.set_num(inMat.num_row, inMat.num_col);
  sliced_blockcyclic_matrix<T> m2(outMat);

  int INFO = 0;
  lvec<int> IPIV = make_node_local_allocate<std::vector<int>>();

  INFO = m2.data.map(pgetrf<T>,IPIV).get(0);
  if(INFO)
    REPORT_ERROR(INTERNAL_ERROR,"LU Factorization failed!!\n");

  INFO = m2.data.map(pgetri<T>,IPIV).get(0);
  if(INFO)
    REPORT_ERROR(INTERNAL_ERROR,"Matrix inversion failed!!\n");

  return outMat;
}


// When argument is a lValue
template <class T>
blockcyclic_matrix<T>
inv(blockcyclic_matrix<T>& inMat) {
  //DEBUG
  // std::cout << "lValue version of inv() is called!!\n";
  // lValue: Input matrix will be copied, before the inverse operation
  sliced_blockcyclic_matrix<T> m2(inMat);
  return inv(m2);
}


// When argument is a sliced blockcyclic matrix (lvalue)
template <class T>
blockcyclic_matrix<T>
inv(const sliced_blockcyclic_matrix<T>& inMat) {

  auto& m1 = const_cast<sliced_blockcyclic_matrix<T>&> (inMat);  
  size_t nrow = m1.data.map(get_local_num_row<T>).get(0);
  size_t ncol = m1.data.map(get_local_num_col<T>).get(0);

  if(nrow != ncol)
    REPORT_ERROR(USER_ERROR, "Input matrix is not a square matrix!!\n");

  // lvalue: copying input matrix
  blockcyclic_matrix<T> outMat(nrow, ncol);
  sliced_blockcyclic_matrix<T> m2(outMat);
  auto dist_UPLO = make_node_local_broadcast('A');
  m1.data.mapv(placpy<T>, m2.data, dist_UPLO);

  int INFO = 0;
  lvec<int> IPIV = make_node_local_allocate<std::vector<int>>();

  INFO = m2.data.map(pgetrf<T>,IPIV).get(0);
  if(INFO)
    REPORT_ERROR(INTERNAL_ERROR,"LU Factorization failed!!\n");

  INFO = m2.data.map(pgetri<T>,IPIV).get(0);
  if(INFO)
    REPORT_ERROR(INTERNAL_ERROR,"Matrix inversion failed!!\n");

  return outMat;
}


// -------- GETRS --------  
template <class T>
int getrs(const sliced_blockcyclic_matrix<T>& inMat,
          const sliced_blockcyclic_matrix<T>& outMat,
          const lvec<int>& IPIV,
          char TRANS = 'N') {

  auto& m1   = const_cast<sliced_blockcyclic_matrix<T>&> (inMat);
  auto& m2   = const_cast<sliced_blockcyclic_matrix<T>&> (outMat);
  auto& pivA = const_cast<lvec<int>&> (IPIV);
  auto dist_TRANS = make_node_local_broadcast(TRANS);
  auto ret = m1.data.map(pgetrs<T>, m2.data, pivA, dist_TRANS).get(0);
  return ret;
}

}

#endif
