#include <iostream>
#include <cmath>
#include "blas_wrapper.hpp"

namespace frovedis {

template <>
void swap(const sliced_colmajor_vector_local<float>& vec1,
          const sliced_colmajor_vector_local<float>& vec2){

#ifdef ERROR_CHK
    ASSERT_PTR(vec1.data && vec2.data);

    if(!vec1.is_valid() || !vec2.is_valid()) 
      REPORT_ERROR(USER_ERROR,"Invalid input vector!!\n");

    if(vec1.size != vec2.size)
      REPORT_ERROR(USER_ERROR,"Incompatible input vector sizes!!\n");
#endif

    int N = static_cast<int>(vec1.size); //vec2.size
    int INCX = static_cast<int>(vec1.stride);
    int INCY = static_cast<int>(vec2.stride);
    float* xptr = vec1.data;
    float* yptr = vec2.data;

    sswap_(&N,
           xptr, &INCX,
           yptr, &INCY);
}

template <>
void swap(const sliced_colmajor_vector_local<double>& vec1,
          const sliced_colmajor_vector_local<double>& vec2){

#ifdef ERROR_CHK
    ASSERT_PTR(vec1.data && vec2.data);

    if(!vec1.is_valid() || !vec2.is_valid()) 
      REPORT_ERROR(USER_ERROR,"Invalid input vector!!\n");

    if(vec1.size != vec2.size)
      REPORT_ERROR(USER_ERROR,"Incompatible input vector sizes!!\n");
#endif

    int N = static_cast<int>(vec1.size); //vec2.size
    int INCX = static_cast<int>(vec1.stride);
    int INCY = static_cast<int>(vec2.stride);
    double* xptr = vec1.data;
    double* yptr = vec2.data;

    dswap_(&N,
           xptr, &INCX,
           yptr, &INCY);
}

template <>
void scal(const sliced_colmajor_vector_local<float>& inVec,
          float alpha){

#ifdef ERROR_CHK 
    ASSERT_PTR(inVec.data);

    if(!inVec.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");
#endif

    size_t N = inVec.size;
    size_t INCX = inVec.stride;
    float* xptr = inVec.data;

    for(size_t i = 0; i < N*INCX; i += INCX)
      xptr[i] *= alpha;
}

template <>
void scal(const sliced_colmajor_vector_local<double>& inVec,
          double alpha){

#ifdef ERROR_CHK
    ASSERT_PTR(inVec.data);

    if(!inVec.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");
#endif

    size_t N = inVec.size;
    size_t INCX = inVec.stride;
    double* xptr = inVec.data;
    
    for(size_t i = 0; i < N*INCX; i += INCX)
      xptr[i] *= alpha;
}


template<>
void copy(const sliced_colmajor_vector_local<float>& inVec,
          const sliced_colmajor_vector_local<float>& outVec){

#ifdef ERROR_CHK
    ASSERT_PTR(inVec.data && outVec.data);

    if(!inVec.is_valid() || !outVec.is_valid())
      REPORT_ERROR(USER_ERROR, "Invalid input vector!!\n");

    if(inVec.size != outVec.size)
      REPORT_ERROR(USER_ERROR, "Incompatible input vector sizes!!\n");
#endif

    size_t N = inVec.size; //outVec.size
    size_t INCX = inVec.stride;
    size_t INCY = outVec.stride;
    float* xptr = inVec.data;
    float* yptr = outVec.data;

    // Code for equal stride
    if (INCX == INCY) {
       for(size_t i = 0; i < N*INCX; i+=INCX) 
         yptr[i] = xptr[i];
    }

    // Code for unequal stride
    else {
       size_t i = 0;
       size_t j = 0;
       for(size_t count = 0; count < N; count++) {
         yptr[j] = xptr[i];
         i += INCX;
         j += INCY;
       }
    }
}

template<>
void copy(const sliced_colmajor_vector_local<double>& inVec,
          const sliced_colmajor_vector_local<double>& outVec){

#ifdef ERROR_CHK
    ASSERT_PTR(inVec.data && outVec.data);

    if(!inVec.is_valid() || !outVec.is_valid())
      REPORT_ERROR(USER_ERROR, "Invalid input vector!!\n");

    if(inVec.size != outVec.size)
      REPORT_ERROR(USER_ERROR, "Incompatible input vector sizes!!\n");
#endif

    size_t N = inVec.size; //outVec.size
    size_t INCX = inVec.stride;
    size_t INCY = outVec.stride;
    double* xptr = inVec.data;
    double* yptr = outVec.data;

    // Code for equal stride
    if (INCX == INCY) {
       for(size_t i = 0; i < N*INCX; i+=INCX) 
         yptr[i] = xptr[i];
    }

    // Code for unequal stride
    else {
       size_t i = 0;
       size_t j = 0;
       for(size_t count = 0; count < N; count++) {
         yptr[j] = xptr[i];
         i += INCX;
         j += INCY;
       }
    }
}

template<>
void axpy(const sliced_colmajor_vector_local<float>& inVec,
          const sliced_colmajor_vector_local<float>& outVec,
          float alpha){

#ifdef ERROR_CHK
    ASSERT_PTR(inVec.data && outVec.data);

    if(!inVec.is_valid() || !outVec.is_valid())
      REPORT_ERROR(USER_ERROR, "Invalid input vector!!\n");

    if(inVec.size != outVec.size)
      REPORT_ERROR(USER_ERROR, "Incompatible input vector sizes!!\n");
#endif

    size_t N = inVec.size; //outVec.size
    size_t INCX = inVec.stride;
    size_t INCY = outVec.stride;
    float* xptr = inVec.data;
    float* yptr = outVec.data;

    // Code for equal increment
    if(INCX == INCY){
      for(size_t i = 0; i < N* INCX; i += INCX)
        yptr[i] += alpha * xptr[i];
    }

    // Code for unequal increment
    else{
       size_t i = 0;
       size_t j = 0;
       for(size_t count = 0; count < N; count++) {
         yptr[j] += alpha * xptr[i];
         i += INCX;
         j += INCY;
       }
    }
    
}

template<>
void axpy(const sliced_colmajor_vector_local<double>& inVec,
          const sliced_colmajor_vector_local<double>& outVec,
          double alpha){

#ifdef ERROR_CHK
    ASSERT_PTR(inVec.data && outVec.data);

    if(!inVec.is_valid() || !outVec.is_valid())
      REPORT_ERROR(USER_ERROR, "Invalid input vector!!\n");

    if(inVec.size != outVec.size)
      REPORT_ERROR(USER_ERROR, "Incompatible input vector sizes!!\n");
#endif

    size_t N = inVec.size; //outVec.size
    size_t INCX = inVec.stride;
    size_t INCY = outVec.stride;
    double* xptr = inVec.data;
    double* yptr = outVec.data;

    // Code for equal increment
    if(INCX == INCY){
      for(size_t i = 0; i < N* INCX; i += INCX)
        yptr[i] += alpha * xptr[i];
    }

    // Code for unequal increment
    else{
       size_t i = 0;
       size_t j = 0;
       for(size_t count = 0; count < N; count++) {
         yptr[j] += alpha * xptr[i];
         i += INCX;
         j += INCY;
       }
    }
}

template<>
float dot(const sliced_colmajor_vector_local<float>& inVec1,
          const sliced_colmajor_vector_local<float>& inVec2){

#ifdef ERROR_CHK         
    ASSERT_PTR(inVec1.data && inVec2.data);

    if(!inVec1.is_valid() || !inVec2.is_valid())
      REPORT_ERROR(USER_ERROR, "Invalid input vector!!\n");

    if(inVec1.size != inVec2.size)
      REPORT_ERROR(USER_ERROR, "Incompatible input vector sizes!!\n");
#endif

    size_t N = inVec1.size; //inVec2.size
    size_t INCX = inVec1.stride;
    size_t INCY = inVec2.stride;
    float* xptr = inVec1.data;
    float* yptr = inVec2.data;
    float prod = 0.0;

    //Code for equal increment
    if(INCX == INCY){
      for(size_t i = 0; i < N*INCX; i += INCX)
        prod += xptr[i] * yptr[i];
    }

    //Code for unequal increment
    else{
      size_t i = 0;
      size_t j = 0;
      for(size_t count = 0; count < N; count++){
        prod += xptr[i] * yptr[j];
        i += INCX;
        j += INCY;
      }
    }

 return prod;
}

template<>
double dot(const sliced_colmajor_vector_local<double>& inVec1,
           const sliced_colmajor_vector_local<double>& inVec2){

#ifdef ERROR_CHK
    ASSERT_PTR(inVec1.data && inVec2.data);

    if(!inVec1.is_valid() || !inVec2.is_valid())
      REPORT_ERROR(USER_ERROR, "Invalid input vector!!\n");

    if(inVec1.size != inVec2.size)
      REPORT_ERROR(USER_ERROR, "Incompatible input vector sizes!!\n");
#endif

    size_t N = inVec1.size; //inVec2.size
    size_t INCX = inVec1.stride;
    size_t INCY = inVec2.stride;
    double* xptr = inVec1.data;
    double* yptr = inVec2.data;
    double prod = 0.0;
   
    //Code for equal increment
    if(INCX == INCY){
      for(size_t i = 0; i < N*INCX; i += INCX)
        prod += xptr[i] * yptr[i];
    }

    //Code for unequal increment
    else{
      size_t i = 0;
      size_t j = 0;
      for(size_t count = 0; count < N; count++){
        prod += xptr[i] * yptr[j];
        i += INCX;
        j += INCY;
      }
    }
 return prod;
}

template<>
float nrm2(const sliced_colmajor_vector_local<float>& inVec){

#ifdef ERROR_CHK
    ASSERT_PTR(inVec.data);

    if(!inVec.is_valid())
      REPORT_ERROR(USER_ERROR, "Invalid input vector!!\n");
#endif
   
    size_t N = inVec.size;
    size_t INCX = inVec.stride;
    float* xptr = inVec.data;

    // quick return
    if(N == 1) return std::abs(xptr[0]);

/*
    float norm = 0.0f;
    for(size_t i = 0; i < N*INCX; i += INCX)
      norm += xptr[i] * xptr[i]; // may cause overflow here...
    return sqrt(norm);
*/
  auto maxval = std::abs(xptr[0]);
  for(size_t i = 0; i < N*INCX; i += INCX) {
    auto absval = xptr[i] * ((xptr[i] >= 0.0f) - (xptr[i] < 0.0f));
    if (absval > maxval) maxval = absval;
  }
  float one_by_max = 1.0f / maxval;
  float sqsum = 0.0f;
  for(size_t i = 0; i < N*INCX; i += INCX) {
    auto tmp = xptr[i] * one_by_max; // dividing with max to avoid overflow!
    sqsum += tmp * tmp;
  }
  return std::sqrt(sqsum) * maxval;
}

template<>
double nrm2(const sliced_colmajor_vector_local<double>& inVec){

#ifdef ERROR_CHK
    ASSERT_PTR(inVec.data);

    if(!inVec.is_valid())
      REPORT_ERROR(USER_ERROR, "Invalid input vector!!\n");
#endif
   
    size_t N = inVec.size;
    size_t INCX = inVec.stride;
    double* xptr = inVec.data;

    // quick return
    if(N == 1) return std::abs(xptr[0]);

/*
    double norm = 0.0;
    for(size_t i = 0; i < N*INCX; i += INCX)
      norm += xptr[i] * xptr[i]; // may cause overflow here...
    return sqrt(norm);
*/
  auto maxval = std::abs(xptr[0]);
  for(size_t i = 0; i < N*INCX; i += INCX) {
    auto absval = xptr[i] * ((xptr[i] >= 0.0) - (xptr[i] < 0.0));
    if (absval > maxval) maxval = absval;
  }
  double one_by_max = 1.0 / maxval;
  double sqsum = 0.0;
  for(size_t i = 0; i < N*INCX; i += INCX) {
    auto tmp = xptr[i] * one_by_max; // dividing with max to avoid overflow!
    sqsum += tmp * tmp;
  }
  return std::sqrt(sqsum) * maxval;
}

template<>
void gemv(const sliced_colmajor_matrix_local<float>& inMat,
          const sliced_colmajor_vector_local<float>& inVec,
          const sliced_colmajor_vector_local<float>& outVec,
          char TRANS,
          float alpha,
          float beta){

#ifdef ERROR_CHK    
    ASSERT_PTR(inVec.data && inMat.data);

    if(!outVec.data)
      REPORT_ERROR(USER_ERROR, "Unallocated output vector!!\n");

    if(!inMat.is_valid())
      REPORT_ERROR(USER_ERROR, "Invalid input matrix!!\n");

    if(!inVec.is_valid() || !outVec.is_valid())
      REPORT_ERROR(USER_ERROR, "Invalid input vector!!\n");
#endif

    size_t nrowa = 0, ncola = 0;
    if(TRANS == 'N') {
      nrowa = inMat.local_num_row;
      ncola = inMat.local_num_col;
    }
    else if (TRANS == 'T') {
      nrowa = inMat.local_num_col;
      ncola = inMat.local_num_row;
    }
    else
      REPORT_ERROR(USER_ERROR, "Invalid value for TRANS parameter!!\n");

    if(inVec.size < ncola || outVec.size < nrowa)
      REPORT_ERROR(USER_ERROR,
        "Incompatible input sizes: matrix-vector multiplication not possible!!\n");

    int M = static_cast<int>(inMat.local_num_row);
    int N = static_cast<int>(inMat.local_num_col);
    float* mptr = inMat.data;
    int LDM = static_cast<int>(inMat.ldm);

    float* xptr = inVec.data;
    int INCX = static_cast<int>(inVec.stride);

    float* yptr = outVec.data;
    int INCY = static_cast<int>(outVec.stride);

    sgemv_(&TRANS,
           &M, &N,
           &alpha,
           mptr, &LDM,
           xptr, &INCX,
           &beta,
           yptr, &INCY);
}

template<>
void gemv(const sliced_colmajor_matrix_local<double>& inMat,
          const sliced_colmajor_vector_local<double>& inVec,
          const sliced_colmajor_vector_local<double>& outVec,
          char TRANS,
          double alpha,
          double beta){

#ifdef ERROR_CHK    
    ASSERT_PTR(inVec.data && inMat.data);

    if(!outVec.data)
      REPORT_ERROR(USER_ERROR, "Unallocated output vector!!\n");

    if(!inMat.is_valid())
      REPORT_ERROR(USER_ERROR, "Invalid input matrix!!\n");

    if(!inVec.is_valid() || !outVec.is_valid())
      REPORT_ERROR(USER_ERROR, "Invalid input vector!!\n");
#endif

    size_t nrowa = 0, ncola = 0;
    if(TRANS == 'N') {
      nrowa = inMat.local_num_row;
      ncola = inMat.local_num_col;
    }
    else if (TRANS == 'T') {
      nrowa = inMat.local_num_col;
      ncola = inMat.local_num_row;
    }
    else
      REPORT_ERROR(USER_ERROR, "Invalid value for TRANS parameter!!\n");

    if(inVec.size < ncola || outVec.size < nrowa)
      REPORT_ERROR(USER_ERROR,
        "Incompatible input sizes: matrix-vector multiplication not possible!!\n");

    int M = static_cast<int>(inMat.local_num_row);
    int N = static_cast<int>(inMat.local_num_col);
    double* mptr = inMat.data;
    int LDM = static_cast<int>(inMat.ldm);

    double* xptr = inVec.data;
    int INCX = static_cast<int>(inVec.stride);

    double* yptr = outVec.data;
    int INCY = static_cast<int>(outVec.stride);

    dgemv_(&TRANS,
           &M, &N, 
           &alpha,
           mptr, &LDM,
           xptr, &INCX,
           &beta,
           yptr, &INCY);
}

template<>
void ger(const sliced_colmajor_vector_local<float>& inVec1,
         const sliced_colmajor_vector_local<float>& inVec2,
         const sliced_colmajor_matrix_local<float>& outMat,
         float alpha){

#ifdef ERROR_CHK
    ASSERT_PTR(inVec1.data && inVec2.data);

    if(!inVec1.is_valid() || !inVec2.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input vector!!\n");

    if(!outMat.data)
      REPORT_ERROR(USER_ERROR, "Unallocated output matrix!!\n");

    if(!outMat.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid output matrix!!\n");

    if(outMat.local_num_row > inVec1.size || 
       outMat.local_num_col > inVec2.size)
      REPORT_ERROR(USER_ERROR, 
        "Incompatible input sizes: multiplication not possible!!\n");
#endif

    int M = static_cast<int>(outMat.local_num_row);
    int N = static_cast<int>(outMat.local_num_col);
    float* mptr = outMat.data;
    int LDM = static_cast<int>(outMat.ldm);

    float* xptr = inVec1.data;
    int INCX = static_cast<int>(inVec1.stride);

    float* yptr = inVec2.data;
    int INCY = static_cast<int>(inVec2.stride);

    sger_(&M, &N, &alpha,
          xptr, &INCX,
          yptr, &INCY,
          mptr, &LDM);
}

template<>
void ger(const sliced_colmajor_vector_local<double>& inVec1,
         const sliced_colmajor_vector_local<double>& inVec2,
         const sliced_colmajor_matrix_local<double>& outMat,
         double alpha){

#ifdef ERROR_CHK
    ASSERT_PTR(inVec1.data && inVec2.data);

    if(!inVec1.is_valid() || !inVec2.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input vector!!\n");

    if(!outMat.data)
      REPORT_ERROR(USER_ERROR, "Unallocated output matrix!!\n");

    if(!outMat.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid output matrix!!\n");

    if(outMat.local_num_row > inVec1.size || 
       outMat.local_num_col > inVec2.size)
      REPORT_ERROR(USER_ERROR, 
        "Incompatible input sizes: multiplication not possible!!\n");
#endif

    int M = static_cast<int>(outMat.local_num_row);
    int N = static_cast<int>(outMat.local_num_col);
    double* mptr = outMat.data;
    int LDM = static_cast<int>(outMat.ldm);

    double* xptr = inVec1.data;
    int INCX = static_cast<int>(inVec1.stride);

    double* yptr = inVec2.data;
    int INCY = static_cast<int>(inVec2.stride);

    dger_(&M, &N, &alpha,
          xptr, &INCX,
          yptr, &INCY,
          mptr, &LDM);
}

template<>
void gemm(const sliced_colmajor_matrix_local<float>& inMat1,
          const sliced_colmajor_matrix_local<float>& inMat2,
          const sliced_colmajor_matrix_local<float>& outMat,
          char TRANS_M1,
          char TRANS_M2,
          float alpha,
          float beta) {

#ifdef ERROR_CHK
    ASSERT_PTR(inMat1.data && inMat2.data);

    if(!inMat1.is_valid() || !inMat2.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

    if(!outMat.data)
      REPORT_ERROR(USER_ERROR,"Unallocated output matrix!!\n");

    if(!outMat.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid output matrix!!\n");
#endif

    size_t nrowa=0, nrowb=0, nrowc=0, ncola=0, ncolb=0, ncolc=0;

    if(TRANS_M1 == 'N') {
      nrowa = inMat1.local_num_row;
      ncola = inMat1.local_num_col;
    }
    else if(TRANS_M1 == 'T') {
      nrowa = inMat1.local_num_col;
      ncola = inMat1.local_num_row;
    }
    else
      REPORT_ERROR(USER_ERROR,"Invalid value for TRANS parameter!!\n");

    if(TRANS_M2 == 'N') {
      nrowb = inMat2.local_num_row;
      ncolb = inMat2.local_num_col;
    }
    else if(TRANS_M2 == 'T') {
      nrowb = inMat2.local_num_col;
      ncolb = inMat2.local_num_row;
    }
    else
      REPORT_ERROR(USER_ERROR,"Invalid value for TRANS parameter!!\n");

    nrowc = outMat.local_num_row;
    ncolc = outMat.local_num_col;

    if (ncola != nrowb || nrowc < nrowa || ncolc < ncolb)
      REPORT_ERROR(USER_ERROR,
            "Incompatible input sizes: Matrix-multiplication not possible!!\n");
    
    int M = static_cast<int>(nrowa);
    int N = static_cast<int>(ncolb);
    int K = static_cast<int>(nrowb); // = ncola;

    float* mptr = outMat.data;
    int LDM = static_cast<int>(outMat.ldm);

    float* xptr = inMat1.data;
    int LDX = static_cast<int>(inMat1.ldm);

    float* yptr = inMat2.data;
    int LDY = static_cast<int>(inMat2.ldm);

    sgemm_(&TRANS_M1, &TRANS_M2,
           &M, &N, &K,
           &alpha,
           xptr, &LDX,
           yptr, &LDY,
           &beta,
           mptr, &LDM);
}

template<>
void gemm(const sliced_colmajor_matrix_local<double>& inMat1,
          const sliced_colmajor_matrix_local<double>& inMat2,
          const sliced_colmajor_matrix_local<double>& outMat,
          char TRANS_M1,
          char TRANS_M2,
          double alpha,
          double beta){

#ifdef ERROR_CHK   
    ASSERT_PTR(inMat1.data && inMat2.data);

    if(!inMat1.is_valid() || !inMat2.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

    if(!outMat.data)
      REPORT_ERROR(USER_ERROR,"Unallocated output matrix!!\n");

    if(!outMat.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid output matrix!!\n");
#endif

    size_t nrowa=0, nrowb=0, nrowc=0, ncola=0, ncolb=0, ncolc=0;

    if(TRANS_M1 == 'N') {
      nrowa = inMat1.local_num_row;
      ncola = inMat1.local_num_col;
    }
    else if(TRANS_M1 == 'T') {
      nrowa = inMat1.local_num_col;
      ncola = inMat1.local_num_row;
    }
    else
      REPORT_ERROR(USER_ERROR,"Invalid value for TRANS parameter!!\n");

    if(TRANS_M2 == 'N') {
      nrowb = inMat2.local_num_row;
      ncolb = inMat2.local_num_col;
    }
    else if(TRANS_M2 == 'T') {
      nrowb = inMat2.local_num_col;
      ncolb = inMat2.local_num_row;
    }
    else
      REPORT_ERROR(USER_ERROR,"Invalid value for TRANS parameter!!\n");

    nrowc = outMat.local_num_row;
    ncolc = outMat.local_num_col;

    if (ncola != nrowb || nrowc < nrowa || ncolc < ncolb)
      REPORT_ERROR(USER_ERROR,
            "Incompatible input sizes: Matrix-multiplication not possible!!\n");
    
    int M = static_cast<int>(nrowa);
    int N = static_cast<int>(ncolb);
    int K = static_cast<int>(nrowb); // = ncola;

    double* mptr = outMat.data;
    int LDM = static_cast<int>(outMat.ldm);

    double* xptr = inMat1.data;
    int LDX = static_cast<int>(inMat1.ldm);

    double* yptr = inMat2.data;
    int LDY = static_cast<int>(inMat2.ldm);

    dgemm_(&TRANS_M1, &TRANS_M2,
           &M, &N, &K,
           &alpha,
           xptr, &LDX,
           yptr, &LDY,
           &beta,
           mptr, &LDM);
}

}
