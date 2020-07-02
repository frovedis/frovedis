#include "pblas_wrapper.hpp"

namespace frovedis {

template <>
void pswap (sliced_blockcyclic_vector_local<float>& vec1,
            sliced_blockcyclic_vector_local<float>& vec2) {

  if(!is_same_distribution(vec1,vec2))
    REPORT_FATAL(USER_ERROR,"Incompatible distribution of the inputs\n");
  // since inputs are of same distribution, activeness check on one input is ok
  if(is_active(vec1)) { 
#ifdef ERROR_CHK
    ASSERT_PTR(vec1.data && vec2.data);

    if(!vec1.is_valid() || !vec2.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input vector!!\n");

    if(vec1.size != vec2.size)
      REPORT_ERROR(USER_ERROR,"Incompatible length of the input vectors!!\n");
#endif

    int N = static_cast<int>(vec1.size); // vec2.size;

    float* xptr = vec1.data;
    int IX = static_cast<int>(vec1.IA);
    int JX = static_cast<int>(vec1.JA);
    int* DESCX = vec1.descA;
    int INCX = static_cast<int>(vec1.stride);

    float* yptr = vec2.data;
    int IY = static_cast<int>(vec2.IA);
    int JY = static_cast<int>(vec2.JA);
    int* DESCY = vec2.descA;
    int INCY = static_cast<int>(vec2.stride);

    psswap_(&N,
            xptr, &IX, &JX, DESCX, &INCX,
            yptr, &IY, &JY, DESCY, &INCY);
  }
}

template <>
void pswap (sliced_blockcyclic_vector_local<double>& vec1,
            sliced_blockcyclic_vector_local<double>& vec2) {

  if(!is_same_distribution(vec1,vec2))
    REPORT_FATAL(USER_ERROR,"Incompatible distribution of the inputs\n");
  // since inputs are of same distribution, activeness check on one input is ok
  if(is_active(vec1)) { 
#ifdef ERROR_CHK
    ASSERT_PTR(vec1.data && vec2.data);

    if(!vec1.is_valid() || !vec2.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input vector!!\n");

    if(vec1.size != vec2.size)
      REPORT_ERROR(USER_ERROR,"Incompatible length of the input vectors!!\n");
#endif

    int N = static_cast<int>(vec1.size); // vec2.size;

    double* xptr = vec1.data;
    int IX = static_cast<int>(vec1.IA);
    int JX = static_cast<int>(vec1.JA);
    int* DESCX = vec1.descA;
    int INCX = static_cast<int>(vec1.stride);

    double* yptr = vec2.data;
    int IY = static_cast<int>(vec2.IA);
    int JY = static_cast<int>(vec2.JA);
    int* DESCY = vec2.descA;
    int INCY = static_cast<int>(vec2.stride);

    pdswap_(&N,
            xptr, &IX, &JX, DESCX, &INCX,
            yptr, &IY, &JY, DESCY, &INCY);
  }
}

template <>
void pcopy (sliced_blockcyclic_vector_local<float>& vec1,
            sliced_blockcyclic_vector_local<float>& vec2) {

  if(!is_same_distribution(vec1,vec2))
    REPORT_FATAL(USER_ERROR,"Incompatible distribution of the inputs\n");
  // since inputs are of same distribution, activeness check on one input is ok
  if(is_active(vec1)) { 
#ifdef ERROR_CHK
    ASSERT_PTR(vec1.data && vec2.data);

    if(!vec1.is_valid() || !vec2.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input vector!!\n");

    if(vec1.size != vec2.size)
      REPORT_ERROR(USER_ERROR,"Incompatible length of the input vectors!!\n");
#endif
  
    int N = static_cast<int>(vec1.size); // vec2.size;

    float* xptr = vec1.data;
    int IX = static_cast<int>(vec1.IA);
    int JX = static_cast<int>(vec1.JA);
    int* DESCX = vec1.descA;
    int INCX = static_cast<int>(vec1.stride);

    float* yptr = vec2.data;
    int IY = static_cast<int>(vec2.IA);
    int JY = static_cast<int>(vec2.JA);
    int* DESCY = vec2.descA;
    int INCY = static_cast<int>(vec2.stride);

    pscopy_(&N,
            xptr, &IX, &JX, DESCX, &INCX,
            yptr, &IY, &JY, DESCY, &INCY);
  }
}

template <>
void pcopy (sliced_blockcyclic_vector_local<double>& vec1,
            sliced_blockcyclic_vector_local<double>& vec2) {

  if(!is_same_distribution(vec1,vec2))
    REPORT_FATAL(USER_ERROR,"Incompatible distribution of the inputs\n");
  // since inputs are of same distribution, activeness check on one input is ok
  if(is_active(vec1)) { 
#ifdef ERROR_CHK
    ASSERT_PTR(vec1.data && vec2.data);

    if(!vec1.is_valid() || !vec2.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input vector!!\n");

    if(vec1.size != vec2.size)
      REPORT_ERROR(USER_ERROR,"Incompatible length of the input vectors!!\n");
#endif
  
    int N = static_cast<int>(vec1.size); // vec2.size;

    double* xptr = vec1.data;
    int IX = static_cast<int>(vec1.IA);
    int JX = static_cast<int>(vec1.JA);
    int* DESCX = vec1.descA;
    int INCX = static_cast<int>(vec1.stride);

    double* yptr = vec2.data;
    int IY = static_cast<int>(vec2.IA);
    int JY = static_cast<int>(vec2.JA);
    int* DESCY = vec2.descA;
    int INCY = static_cast<int>(vec2.stride);

    pdcopy_(&N,
            xptr, &IX, &JX, DESCX, &INCX,
            yptr, &IY, &JY, DESCY, &INCY);
  }
}

template <>
void pscal (sliced_blockcyclic_vector_local<float>& vec,
            float ALPHA) {

  if(is_active(vec)) { 
#ifdef ERROR_CHK
    ASSERT_PTR(vec.data);

    if(!vec.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input vector!!\n");
#endif

    int N = static_cast<int>(vec.size);
    float* xptr = vec.data;
    int IX = static_cast<int>(vec.IA);
    int JX = static_cast<int>(vec.JA);
    int* DESCX = vec.descA;
    int INCX = static_cast<int>(vec.stride);

    psscal_(&N, &ALPHA,
            xptr, &IX, &JX, DESCX, &INCX);
  }
}

template <>
void pscal (sliced_blockcyclic_vector_local<double>& vec,
            double ALPHA) {

  if(is_active(vec)) { 
#ifdef ERROR_CHK
    ASSERT_PTR(vec.data);

    if(!vec.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input vector!!\n");
#endif

    int N = static_cast<int>(vec.size);
    double* xptr = vec.data;
    int IX = static_cast<int>(vec.IA);
    int JX = static_cast<int>(vec.JA);
    int* DESCX = vec.descA;
    int INCX = static_cast<int>(vec.stride);

    pdscal_(&N, &ALPHA,
            xptr, &IX, &JX, DESCX, &INCX);
  }
}

template <>
void paxpy (sliced_blockcyclic_vector_local<float>& inVec,
            sliced_blockcyclic_vector_local<float>& outVec,
	    float ALPHA) {

  if(!is_same_distribution(inVec,outVec))
    REPORT_FATAL(USER_ERROR,"Incompatible distribution of the inputs\n");
  // since inputs are of same distribution, activeness check on one input is ok
  if(is_active(inVec)) {
#ifdef ERROR_CHK
    ASSERT_PTR(inVec.data && outVec.data);

    if(!inVec.is_valid() || !outVec.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input vector!!\n");

    if(inVec.size != outVec.size)
      REPORT_ERROR(USER_ERROR,"Incompatible length of the input vectors!!\n");
#endif

    int N = static_cast<int>(inVec.size); // outVec.size;

    float* xptr = inVec.data;
    int IX = static_cast<int>(inVec.IA);
    int JX = static_cast<int>(inVec.JA);
    int* DESCX = inVec.descA;
    int INCX = static_cast<int>(inVec.stride);

    float* yptr = outVec.data;
    int IY = static_cast<int>(outVec.IA);
    int JY = static_cast<int>(outVec.JA);
    int* DESCY = outVec.descA;
    int INCY = static_cast<int>(outVec.stride);

    psaxpy_(&N, &ALPHA,
            xptr, &IX, &JX, DESCX, &INCX,
	    yptr, &IY, &JY, DESCY, &INCY);
  }
}

template <>
void paxpy (sliced_blockcyclic_vector_local<double>& inVec,
            sliced_blockcyclic_vector_local<double>& outVec,
	    double ALPHA) {

  if(!is_same_distribution(inVec,outVec))
    REPORT_FATAL(USER_ERROR,"Incompatible distribution of the inputs\n");
  // since inputs are of same distribution, activeness check on one input is ok
  if(is_active(inVec)) {
#ifdef ERROR_CHK
    ASSERT_PTR(inVec.data && outVec.data);

    if(!inVec.is_valid() || !outVec.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input vector!!\n");

    if(inVec.size != outVec.size)
      REPORT_ERROR(USER_ERROR,"Incompatible length of the input vectors!!\n");
#endif

    int N = static_cast<int>(inVec.size); // outVec.size;

    double* xptr = inVec.data;
    int IX = static_cast<int>(inVec.IA);
    int JX = static_cast<int>(inVec.JA);
    int* DESCX = inVec.descA;
    int INCX = static_cast<int>(inVec.stride);

    double* yptr = outVec.data;
    int IY = static_cast<int>(outVec.IA);
    int JY = static_cast<int>(outVec.JA);
    int* DESCY = outVec.descA;
    int INCY = static_cast<int>(outVec.stride);

    pdaxpy_(&N, &ALPHA,
            xptr, &IX, &JX, DESCX, &INCX,
	    yptr, &IY, &JY, DESCY, &INCY);
  }
}

template <>
float pdot (sliced_blockcyclic_vector_local<float>& inVec1,
            sliced_blockcyclic_vector_local<float>& inVec2) {

  float DOT = 0;

  if(!is_same_distribution(inVec1,inVec2))
    REPORT_FATAL(USER_ERROR,"Incompatible distribution of the inputs\n");
  // since inputs are of same distribution, activeness check on one input is ok
  if(is_active(inVec1)) {
#ifdef ERROR_CHK
    ASSERT_PTR(inVec1.data && inVec2.data);

    if(!inVec1.is_valid() || !inVec2.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input vector!!\n");

    if(inVec1.size != inVec2.size)
      REPORT_ERROR(USER_ERROR,"Incompatible length of the input vectors!!\n");
#endif

    int N = static_cast<int>(inVec1.size); // inVec2.size;

    float* xptr = inVec1.data;
    int IX = static_cast<int>(inVec1.IA);
    int JX = static_cast<int>(inVec1.JA);
    int* DESCX = inVec1.descA;
    int INCX = static_cast<int>(inVec1.stride);

    float* yptr = inVec2.data;
    int IY = static_cast<int>(inVec2.IA);
    int JY = static_cast<int>(inVec2.JA);
    int* DESCY = inVec2.descA;
    int INCY = static_cast<int>(inVec2.stride);

    psdot_(&N, &DOT,
           xptr, &IX, &JX, DESCX, &INCX,
	   yptr, &IY, &JY, DESCY, &INCY);
  }

  return DOT;
}

template <>
double pdot (sliced_blockcyclic_vector_local<double>& inVec1,
             sliced_blockcyclic_vector_local<double>& inVec2) {

  double DOT = 0;

  if(!is_same_distribution(inVec1,inVec2))
    REPORT_FATAL(USER_ERROR,"Incompatible distribution of the inputs\n");
  // since inputs are of same distribution, activeness check on one input is ok
  if(is_active(inVec1)) {
#ifdef ERROR_CHK
    ASSERT_PTR(inVec1.data && inVec2.data);

    if(!inVec1.is_valid() || !inVec2.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input vector!!\n");

    if(inVec1.size != inVec2.size)
      REPORT_ERROR(USER_ERROR,"Incompatible length of the input vectors!!\n");
#endif

    int N = static_cast<int>(inVec1.size); // inVec2.size;

    double* xptr = inVec1.data;
    int IX = static_cast<int>(inVec1.IA);
    int JX = static_cast<int>(inVec1.JA);
    int* DESCX = inVec1.descA;
    int INCX = static_cast<int>(inVec1.stride);

    double* yptr = inVec2.data;
    int IY = static_cast<int>(inVec2.IA);
    int JY = static_cast<int>(inVec2.JA);
    int* DESCY = inVec2.descA;
    int INCY = static_cast<int>(inVec2.stride);

    pddot_(&N, &DOT,
           xptr, &IX, &JX, DESCX, &INCX,
	   yptr, &IY, &JY, DESCY, &INCY);
  }

  return DOT;
}

template <>
float pnrm2 (sliced_blockcyclic_vector_local<float>& inVec) {

  float NORM = 0;

  if(is_active(inVec)) {
#ifdef ERROR_CHK
    ASSERT_PTR(inVec.data);

    if(!inVec.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input vector!!\n");
#endif

    int N = static_cast<int>(inVec.size);

    float* xptr = inVec.data;
    int IX = static_cast<int>(inVec.IA);
    int JX = static_cast<int>(inVec.JA);
    int* DESCX = inVec.descA;
    int INCX = static_cast<int>(inVec.stride);

    psnrm2_(&N, &NORM,
            xptr, &IX, &JX, DESCX, &INCX);
  }

  return NORM;
}

template <>
double pnrm2 (sliced_blockcyclic_vector_local<double>& inVec) {

  double NORM = 0;

  if(is_active(inVec)) {
#ifdef ERROR_CHK
    ASSERT_PTR(inVec.data);

    if(!inVec.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input vector!!\n");
#endif

    int N = static_cast<int>(inVec.size);

    double* xptr = inVec.data;
    int IX = static_cast<int>(inVec.IA);
    int JX = static_cast<int>(inVec.JA);
    int* DESCX = inVec.descA;
    int INCX = static_cast<int>(inVec.stride);

    pdnrm2_(&N, &NORM,
            xptr, &IX, &JX, DESCX, &INCX);
  }

  return NORM;
}

template <>
void 
pgemv<float>::operator() (sliced_blockcyclic_matrix_local<float>& inMat,
                          sliced_blockcyclic_vector_local<float>& inVec,
  		          sliced_blockcyclic_vector_local<float>& outVec) {

  if(!is_same_distribution(inMat,inVec,outVec))
    REPORT_FATAL(USER_ERROR,"Incompatible distribution of the inputs\n");
  // since inputs are of same distribution, activeness check on one input is ok
  if(is_active(inMat)) {
#ifdef ERROR_CHK
    ASSERT_PTR(inMat.data && inVec.data);

    if(!outVec.data)
      REPORT_ERROR(USER_ERROR,"Output vector is not allocated!!\n");

    if(!inMat.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

    if(!inVec.is_valid() || !outVec.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input vector!!\n");

    size_t nrowa, ncola;
    if(TRANS == 'N') {
      nrowa = inMat.local_num_row;
      ncola = inMat.local_num_col;
    }
    else if(TRANS == 'T') {
      nrowa = inMat.local_num_col;
      ncola = inMat.local_num_row;
    }
    else
      REPORT_ERROR(USER_ERROR,"Illegal value provided for TRANS parameter!!\n");

    if(inVec.size < ncola || outVec.size < nrowa)
      REPORT_ERROR(USER_ERROR,
        "Incompatible input sizes: matrix-vector multiplication not possible!!\n");
#endif

    int M = static_cast<int>(inMat.local_num_row);
    int N = static_cast<int>(inMat.local_num_col);

    float* aptr = inMat.data;
    int IA = static_cast<int>(inMat.IA);
    int JA = static_cast<int>(inMat.JA);
    int* DESCA = inMat.descA;

    float* xptr = inVec.data;
    int IX = static_cast<int>(inVec.IA);
    int JX = static_cast<int>(inVec.JA);
    int* DESCX = inVec.descA;
    int INCX = static_cast<int>(inVec.stride);

    float* yptr = outVec.data;
    int IY = static_cast<int>(outVec.IA);
    int JY = static_cast<int>(outVec.JA);
    int* DESCY = outVec.descA;
    int INCY = static_cast<int>(outVec.stride);

    psgemv_(&TRANS,
            &M, &N,
            &ALPHA,
            aptr, &IA, &JA, DESCA,
            xptr, &IX, &JX, DESCX, &INCX,
            &BETA,
            yptr, &IY, &JY, DESCY, &INCY);
  }
}

template <>
void 
pgemv<double>::operator() (sliced_blockcyclic_matrix_local<double>& inMat,
                           sliced_blockcyclic_vector_local<double>& inVec,
	         	   sliced_blockcyclic_vector_local<double>& outVec) {

  if(!is_same_distribution(inMat,inVec,outVec))
    REPORT_FATAL(USER_ERROR,"Incompatible distribution of the inputs\n");
  // since inputs are of same distribution, activeness check on one input is ok
  if(is_active(inMat)) {
#ifdef ERROR_CHK
    ASSERT_PTR(inMat.data && inVec.data);

    if(!outVec.data)
      REPORT_ERROR(USER_ERROR,"Output vector is not allocated!!\n");

    if(!inMat.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

    if(!inVec.is_valid() || !outVec.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input vector!!\n");

    size_t nrowa, ncola;
    if(TRANS == 'N') {
      nrowa = inMat.local_num_row;
      ncola = inMat.local_num_col;
    }
    else if(TRANS == 'T') {
      nrowa = inMat.local_num_col;
      ncola = inMat.local_num_row;
    }
    else
      REPORT_ERROR(USER_ERROR,"Illegal value provided for TRANS parameter!!\n");

    if(inVec.size < ncola || outVec.size < nrowa)
      REPORT_ERROR(USER_ERROR,
        "Incompatible input sizes: matrix-vector multiplication not possible!!\n");
#endif

    int M = static_cast<int>(inMat.local_num_row);
    int N = static_cast<int>(inMat.local_num_col);
  
    double* aptr = inMat.data;
    int IA = static_cast<int>(inMat.IA);
    int JA = static_cast<int>(inMat.JA);
    int* DESCA = inMat.descA;

    double* xptr = inVec.data;
    int IX = static_cast<int>(inVec.IA);
    int JX = static_cast<int>(inVec.JA);
    int* DESCX = inVec.descA;
    int INCX = static_cast<int>(inVec.stride);

    double* yptr = outVec.data;
    int IY = static_cast<int>(outVec.IA);
    int JY = static_cast<int>(outVec.JA);
    int* DESCY = outVec.descA;
    int INCY = static_cast<int>(outVec.stride);

    pdgemv_(&TRANS,
            &M, &N,
            &ALPHA,
            aptr, &IA, &JA, DESCA,
            xptr, &IX, &JX, DESCX, &INCX,
            &BETA,
            yptr, &IY, &JY, DESCY, &INCY);
  }
}


template <>
void
pger<float>::operator() (sliced_blockcyclic_vector_local<float>& inVec1,
                         sliced_blockcyclic_vector_local<float>& inVec2,
		         sliced_blockcyclic_matrix_local<float>& outMat) {

  if(!is_same_distribution(inVec1,inVec2,outMat))
    REPORT_FATAL(USER_ERROR,"Incompatible distribution of the inputs\n");
  // since inputs are of same distribution, activeness check on one input is ok
  if(is_active(inVec1)) {
#ifdef ERROR_CHK
    ASSERT_PTR(inVec1.data && inVec2.data);

    if(!outMat.data)
      REPORT_ERROR(USER_ERROR,"Output matrix is not allocated!!\n");

    if(!outMat.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid output matrix!!\n");

    if(!inVec1.is_valid() || !inVec2.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input vector!!\n");

    if(inVec1.size < outMat.local_num_row || 
       inVec2.size < outMat.local_num_col)
      REPORT_ERROR(USER_ERROR, 
        "Incompatible input sizes: multiplication not possible!!\n");
#endif

    int M = static_cast<int>(outMat.local_num_row);
    int N = static_cast<int>(outMat.local_num_col);

    float* aptr = outMat.data;
    int IA = static_cast<int>(outMat.IA);
    int JA = static_cast<int>(outMat.JA);
    int* DESCA = outMat.descA;

    float* xptr = inVec1.data;
    int IX = static_cast<int>(inVec1.IA);
    int JX = static_cast<int>(inVec1.JA);
    int* DESCX = inVec1.descA;
    int INCX = static_cast<int>(inVec1.stride);

    float* yptr = inVec2.data;
    int IY = static_cast<int>(inVec2.IA);
    int JY = static_cast<int>(inVec2.JA);
    int* DESCY = inVec2.descA;
    int INCY = static_cast<int>(inVec2.stride);

    psger_(&M, &N,
           &ALPHA,
           xptr, &IX, &JX, DESCX, &INCX,
           yptr, &IY, &JY, DESCY, &INCY,
           aptr, &IA, &JA, DESCA);
  }
}

template <>
void
pger<double>::operator() (sliced_blockcyclic_vector_local<double>& inVec1,
                          sliced_blockcyclic_vector_local<double>& inVec2,
		          sliced_blockcyclic_matrix_local<double>& outMat) {

  if(!is_same_distribution(inVec1,inVec2,outMat))
    REPORT_FATAL(USER_ERROR,"Incompatible distribution of the inputs\n");
  // since inputs are of same distribution, activeness check on one input is ok
  if(is_active(inVec1)) {
#ifdef ERROR_CHK
    ASSERT_PTR(inVec1.data && inVec2.data);

    if(!outMat.data)
      REPORT_ERROR(USER_ERROR,"Output matrix is not allocated!!\n");

    if(!outMat.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid output matrix!!\n");

    if(!inVec1.is_valid() || !inVec2.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input vector!!\n");

    if(inVec1.size < outMat.local_num_row || 
       inVec2.size < outMat.local_num_col)
      REPORT_ERROR(USER_ERROR, 
        "Incompatible input sizes: multiplication not possible!!\n");
#endif

    int M = static_cast<int>(outMat.local_num_row);
    int N = static_cast<int>(outMat.local_num_col);

    double* aptr = outMat.data;
    int IA = static_cast<int>(outMat.IA);
    int JA = static_cast<int>(outMat.JA);
    int* DESCA = outMat.descA;

    double* xptr = inVec1.data;
    int IX = static_cast<int>(inVec1.IA);
    int JX = static_cast<int>(inVec1.JA);
    int* DESCX = inVec1.descA;
    int INCX = static_cast<int>(inVec1.stride);

    double* yptr = inVec2.data;
    int IY = static_cast<int>(inVec2.IA);
    int JY = static_cast<int>(inVec2.JA);
    int* DESCY = inVec2.descA;
    int INCY = static_cast<int>(inVec2.stride);

    pdger_(&M, &N,
           &ALPHA,
           xptr, &IX, &JX, DESCX, &INCX,
           yptr, &IY, &JY, DESCY, &INCY,
           aptr, &IA, &JA, DESCA);
  }
}

template <>
void
pgemm<float>::operator() (sliced_blockcyclic_matrix_local<float>& inMat1,
                          sliced_blockcyclic_matrix_local<float>& inMat2,
		          sliced_blockcyclic_matrix_local<float>& outMat) {

  if(!is_same_distribution(inMat1,inMat2,outMat))
    REPORT_FATAL(USER_ERROR,"Incompatible distribution of the inputs\n");
  // since inputs are of same distribution, activeness check on one input is ok
  if(is_active(inMat1)) {
#ifdef ERROR_CHK
    ASSERT_PTR(inMat1.data && inMat2.data);

    if(!outMat.data)
      REPORT_ERROR(USER_ERROR,"Output matrix is not allocated!!\n");

    if(!inMat1.is_valid() || !inMat2.is_valid() || !outMat.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");
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
      REPORT_ERROR(USER_ERROR,"Illegal value provided for TRANS parameter!!\n");

    if(TRANS_M2 == 'N') {
      nrowb = inMat2.local_num_row;
      ncolb = inMat2.local_num_col;
    }
    else if(TRANS_M2 == 'T') {
      nrowb = inMat2.local_num_col;
      ncolb = inMat2.local_num_row;
    }
    else
      REPORT_ERROR(USER_ERROR,"Illegal value provided for TRANS parameter!!\n");
  
    nrowc = outMat.local_num_row;
    ncolc = outMat.local_num_col;

    if (ncola != nrowb || nrowc < nrowa || ncolc < ncolb)
      REPORT_ERROR(USER_ERROR,
            "Incompatible input sizes: Matrix-multiplication not possible!!\n");

    int M = static_cast<int>(nrowa);
    int N = static_cast<int>(ncolb);
    int K = static_cast<int>(ncola); //nrowb;

    float* aptr = inMat1.data;
    int IA = static_cast<int>(inMat1.IA);
    int JA = static_cast<int>(inMat1.JA);
    int* DESCA = inMat1.descA;

    float* bptr = inMat2.data;
    int IB = static_cast<int>(inMat2.IA);
    int JB = static_cast<int>(inMat2.JA);
    int* DESCB = inMat2.descA;

    float* cptr = outMat.data;
    int IC = static_cast<int>(outMat.IA);
    int JC = static_cast<int>(outMat.JA);
    int* DESCC = outMat.descA;

    psgemm_(&TRANS_M1, &TRANS_M2,
            &M, &N, &K,
            &ALPHA,
            aptr, &IA, &JA, DESCA,
            bptr, &IB, &JB, DESCB,
            &BETA,
            cptr, &IC, &JC, DESCC);
  }
}

template <>
void
pgemm<double>::operator() (sliced_blockcyclic_matrix_local<double>& inMat1,
                           sliced_blockcyclic_matrix_local<double>& inMat2,
		           sliced_blockcyclic_matrix_local<double>& outMat) {

  if(!is_same_distribution(inMat1,inMat2,outMat))
    REPORT_FATAL(USER_ERROR,"Incompatible distribution of the inputs\n");
  // since inputs are of same distribution, activeness check on one input is ok
  if(is_active(inMat1)) {
#ifdef ERROR_CHK
    ASSERT_PTR(inMat1.data && inMat2.data);

    if(!outMat.data)
      REPORT_ERROR(USER_ERROR,"Output matrix is not allocated!!\n");

    if(!inMat1.is_valid() || !inMat2.is_valid() || !outMat.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");
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
      REPORT_ERROR(USER_ERROR,"Illegal value provided for TRANS parameter!!\n");

    if(TRANS_M2 == 'N') {
      nrowb = inMat2.local_num_row;
      ncolb = inMat2.local_num_col;
    }
    else if(TRANS_M2 == 'T') {
      nrowb = inMat2.local_num_col;
      ncolb = inMat2.local_num_row;
    }
    else
      REPORT_ERROR(USER_ERROR,"Illegal value provided for TRANS parameter!!\n");
    
    nrowc = outMat.local_num_row;
    ncolc = outMat.local_num_col;

    if (ncola != nrowb || nrowc < nrowa || ncolc < ncolb)
      REPORT_ERROR(USER_ERROR,
            "Incompatible input sizes: Matrix-multiplication not possible!!\n");

    int M = static_cast<int>(nrowa);
    int N = static_cast<int>(ncolb);
    int K = static_cast<int>(ncola); //nrowb;

    double* aptr = inMat1.data;
    int IA = static_cast<int>(inMat1.IA);
    int JA = static_cast<int>(inMat1.JA);
    int* DESCA = inMat1.descA;

    double* bptr = inMat2.data;
    int IB = static_cast<int>(inMat2.IA);
    int JB = static_cast<int>(inMat2.JA);
    int* DESCB = inMat2.descA;

    double* cptr = outMat.data;
    int IC = static_cast<int>(outMat.IA);
    int JC = static_cast<int>(outMat.JA);
    int* DESCC = outMat.descA;

    pdgemm_(&TRANS_M1, &TRANS_M2,
            &M, &N, &K,
            &ALPHA,
            aptr, &IA, &JA, DESCA,
            bptr, &IB, &JB, DESCB,
            &BETA,
            cptr, &IC, &JC, DESCC);
  }
}

template <>
void 
pgeadd<float>::operator() (sliced_blockcyclic_matrix_local<float>& inMat,
                           sliced_blockcyclic_matrix_local<float>& outMat) {

  if(!is_same_distribution(inMat,outMat))
    REPORT_FATAL(USER_ERROR,"Incompatible distribution of the inputs\n");
  // since inputs are of same distribution, activeness check on one input is ok
  if(is_active(inMat)) {
#ifdef ERROR_CHK
    ASSERT_PTR(inMat.data && outMat.data);

    if(!inMat.is_valid() || !outMat.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");
#endif

    size_t nrowa=0, nrowb=0, ncola=0, ncolb=0;
    if(TRANS == 'N') {
      nrowa = inMat.local_num_row;
      ncola = inMat.local_num_col;
    }
    else if(TRANS == 'T') {
      nrowa = inMat.local_num_col;
      ncola = inMat.local_num_row;
    }
    else
      REPORT_ERROR(USER_ERROR,"Illegal value provided for TRANS parameter!!\n");

    nrowb = outMat.local_num_row;
    ncolb = outMat.local_num_col;

    if (nrowa != nrowb || ncola != ncolb)
      REPORT_ERROR(USER_ERROR,
            "Incompatible input sizes: Matrix-addition not possible!!\n");

    int M = static_cast<int>(nrowa); // nrowb;
    int N = static_cast<int>(ncola); // ncolb;

    float* aptr = inMat.data;
    int IA = static_cast<int>(inMat.IA);
    int JA = static_cast<int>(inMat.JA);
    int* DESCA = inMat.descA;

    float* bptr = outMat.data;
    int IB = static_cast<int>(outMat.IA);
    int JB = static_cast<int>(outMat.JA);
    int* DESCB = outMat.descA;

    psgeadd_(&TRANS,
             &M, &N,
             &ALPHA,
             aptr, &IA, &JA, DESCA,
             &BETA,
             bptr, &IB, &JB, DESCB);
  }
}

template <>
void
pgeadd<double>::operator() (sliced_blockcyclic_matrix_local<double>& inMat,
	                    sliced_blockcyclic_matrix_local<double>& outMat) {

  if(!is_same_distribution(inMat,outMat))
    REPORT_FATAL(USER_ERROR,"Incompatible distribution of the inputs\n");
  // since inputs are of same distribution, activeness check on one input is ok
  if(is_active(inMat)) {
#ifdef ERROR_CHK
    ASSERT_PTR(inMat.data && outMat.data);

    if(!inMat.is_valid() || !outMat.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");
#endif

    size_t nrowa=0, nrowb=0, ncola=0, ncolb=0;
    if(TRANS == 'N') {
      nrowa = inMat.local_num_row;
      ncola = inMat.local_num_col;
    }
    else if(TRANS == 'T') {
      nrowa = inMat.local_num_col;
      ncola = inMat.local_num_row;
    }
    else
      REPORT_ERROR(USER_ERROR,"Illegal value provided for TRANS parameter!!\n");

    nrowb = outMat.local_num_row;
    ncolb = outMat.local_num_col;

    if (nrowa != nrowb || ncola != ncolb)
      REPORT_ERROR(USER_ERROR,
            "Incompatible input sizes: Matrix-addition not possible!!\n");
    int M = static_cast<int>(nrowa); // nrowb;
    int N = static_cast<int>(ncola); // ncolb;

    double* aptr = inMat.data;
    int IA = static_cast<int>(inMat.IA);
    int JA = static_cast<int>(inMat.JA);
    int* DESCA = inMat.descA;

    double* bptr = outMat.data;
    int IB = static_cast<int>(outMat.IA);
    int JB = static_cast<int>(outMat.JA);
    int* DESCB = outMat.descA;

    pdgeadd_(&TRANS,
             &M, &N,
             &ALPHA,
             aptr, &IA, &JA, DESCA,
             &BETA,
             bptr, &IB, &JB, DESCB);
  }
}

}
