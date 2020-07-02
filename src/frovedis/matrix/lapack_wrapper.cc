#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include "lapack_wrapper.hpp"

namespace frovedis {

template <>
int gesv(const sliced_colmajor_matrix_local<float>& inMat,
         const sliced_colmajor_matrix_local<float>& outMat,
         std::vector<int>& IPIV){

#ifdef ERROR_CHK
  ASSERT_PTR(inMat.data && outMat.data);

  if(!inMat.is_valid() || !outMat.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

  if(inMat.local_num_row != inMat.local_num_col)
    REPORT_ERROR(USER_ERROR,"The coefficient matrix is not a square matrix!!\n");

  if(outMat.local_num_row < inMat.local_num_row || outMat.local_num_col < 1)
    REPORT_ERROR(USER_ERROR,"Invalid right hand side matrix!!\n");
#endif 

  int N = static_cast<int>(inMat.local_num_row); // inMat.local_num_col;
  int NRHS = static_cast<int>(outMat.local_num_col);

  float* Aptr = inMat.data;
  int LDA = static_cast<int>(inMat.ldm);

  float* Bptr = outMat.data;
  int LDB = static_cast<int>(outMat.ldm);

  int INFO = 0;
  IPIV.resize(N,0);

  sgesv_(&N, &NRHS,
         Aptr, &LDA,
         &IPIV[0],
         Bptr, &LDB,
         &INFO);

  return INFO;
}

template<>
int gesv(const sliced_colmajor_matrix_local<float>& inMat,
         const sliced_colmajor_matrix_local<float>& outMat){

  std::vector<int> IPIV;
  return gesv<float>(inMat,outMat,IPIV);
}
       
  
template <>
int gesv(const sliced_colmajor_matrix_local<double>& inMat,
         const sliced_colmajor_matrix_local<double>& outMat,
         std::vector<int>& IPIV){
 
#ifdef ERROR_CHK 
  ASSERT_PTR(inMat.data && outMat.data);

  if(!inMat.is_valid() || !outMat.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

  if(inMat.local_num_row != inMat.local_num_col)
    REPORT_ERROR(USER_ERROR,"The coefficient matrix is not a square matrix!!\n");

  if(outMat.local_num_row < inMat.local_num_row || outMat.local_num_col < 1)
    REPORT_ERROR(USER_ERROR,"Invalid right hand side matrix!!\n");
#endif 

  int N = static_cast<int>(inMat.local_num_row); // inMat.local_num_col;
  int NRHS = static_cast<int>(outMat.local_num_col);

  double* Aptr = inMat.data;
  int LDA = static_cast<int>(inMat.ldm);

  double* Bptr = outMat.data;
  int LDB = static_cast<int>(outMat.ldm);

  int INFO = 0;
  IPIV.resize(N,0);

  dgesv_(&N, &NRHS,
         Aptr, &LDA,
         &IPIV[0],
         Bptr, &LDB,
         &INFO);

  return INFO;
}

template<>
int gesv(const sliced_colmajor_matrix_local<double>& inMat,
         const sliced_colmajor_matrix_local<double>& outMat){

  std::vector<int> IPIV;
  return gesv<double>(inMat,outMat,IPIV);
}


template <>
int gels(const sliced_colmajor_matrix_local<float>& inMat,
         const sliced_colmajor_matrix_local<float>& outMat,
         char TRANS){

#ifdef ERROR_CHK
  ASSERT_PTR(inMat.data && outMat.data);

  if(!inMat.is_valid() || !outMat.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

  if(TRANS != 'N' && TRANS != 'T')
    REPORT_ERROR(USER_ERROR,"Invalid value for TRANS parameter!!\n");

  size_t maxSize = std::max(inMat.local_num_row, inMat.local_num_col);
  if(outMat.local_num_row < maxSize || outMat.local_num_col < 1)
    REPORT_ERROR(USER_ERROR,"Invalid right hand side matrix!!\n");
#endif 
     
  int M = static_cast<int>(inMat.local_num_row);
  int N = static_cast<int>(inMat.local_num_col);
  int NRHS = static_cast<int>(outMat.local_num_col);

  float* Aptr = inMat.data;
  int LDA = static_cast<int>(inMat.ldm);

  float* Bptr = outMat.data;
  int LDB = static_cast<int>(outMat.ldm);

  int INFO = 0;
  int LWORK = -1; 
  std::vector<float> WORK(1);

  // LWORK query...
  sgels_(&TRANS,
         &M, &N, &NRHS,
         Aptr, &LDA,
         Bptr, &LDB,
         &WORK[0], &LWORK,
         &INFO);

  LWORK = WORK[0];
  WORK.resize(LWORK,0);

  // Actual call...
  sgels_(&TRANS,
         &M, &N, &NRHS,
         Aptr, &LDA,
         Bptr, &LDB,
         &WORK[0], &LWORK,
         &INFO);

  return INFO;
}
  
template <>
int gels(const sliced_colmajor_matrix_local<double>& inMat,
         const sliced_colmajor_matrix_local<double>& outMat,
         char TRANS ){

#ifdef ERROR_CHK
  ASSERT_PTR(inMat.data && outMat.data);

  if(!inMat.is_valid() || !outMat.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

  if(TRANS != 'N' && TRANS != 'T')
    REPORT_ERROR(USER_ERROR,"Invalid value for TRANS parameter!!\n");

  size_t maxSize = std::max(inMat.local_num_row, inMat.local_num_col);
  if(outMat.local_num_row < maxSize || outMat.local_num_col < 1)
    REPORT_ERROR(USER_ERROR,"Invalid right hand side matrix!!\n");
#endif 
     
  int M = static_cast<int>(inMat.local_num_row);
  int N = static_cast<int>(inMat.local_num_col);
  int NRHS = static_cast<int>(outMat.local_num_col);

  double* Aptr = inMat.data;
  int LDA = static_cast<int>(inMat.ldm);

  double* Bptr = outMat.data;
  int LDB = static_cast<int>(outMat.ldm);

  int INFO = 0;
  int LWORK = -1; 
  std::vector<double> WORK(1);

  // LWORK query...
  dgels_(&TRANS,
         &M, &N, &NRHS,
         Aptr, &LDA,
         Bptr, &LDB,
         &WORK[0], &LWORK,
         &INFO);

  LWORK = WORK[0];
  WORK.resize(LWORK,0);

  // Actual call...
  dgels_(&TRANS,
         &M, &N, &NRHS,
         Aptr, &LDA,
         Bptr, &LDB,
         &WORK[0], &LWORK,
         &INFO);

  return INFO;
}


template<>
int gelsd(const sliced_colmajor_matrix_local<float>& inMat,
          const sliced_colmajor_matrix_local<float>& outMat,
          std::vector<float>& S_VAL,
          int& RANK,
          float RCOND){

#ifdef ERROR_CHK
  ASSERT_PTR(inMat.data && outMat.data);

  if(!inMat.is_valid() || !outMat.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");
  
  size_t maxSize = std::max(inMat.local_num_row, inMat.local_num_col);
  if(outMat.local_num_row < maxSize || outMat.local_num_col < 1)
     REPORT_ERROR(USER_ERROR,"Invalid right hand side matrix!!\n");
#endif 

  size_t minSize = std::min(inMat.local_num_row, inMat.local_num_col);
  S_VAL.resize(minSize,0);

  int M = static_cast<int>(inMat.local_num_row);
  int N = static_cast<int>(inMat.local_num_col);
  int NRHS = static_cast<int>(outMat.local_num_col);

  float* Aptr = inMat.data;
  int LDA = static_cast<int>(inMat.ldm);

  float* Bptr = outMat.data;
  int LDB = static_cast<int>(outMat.ldm);

  int INFO = 0;
  int LWORK = -1, LIWORK = -1; 
  std::vector<float> WORK(1);
  std::vector<int> IWORK(1);

  // LWORK query...
  sgelsd_(&M, &N, &NRHS,
          Aptr, &LDA,
          Bptr, &LDB,
          &S_VAL[0],
          &RCOND, &RANK,
          &WORK[0], &LWORK, &IWORK[0],
          &INFO);
  
  LWORK = WORK[0];
  LIWORK = IWORK[0];
  WORK.resize(LWORK,0);
  IWORK.resize(LIWORK,0);
       
  // Actual call...
  sgelsd_(&M, &N, &NRHS,
          Aptr, &LDA,
          Bptr, &LDB,
          &S_VAL[0],
          &RCOND, &RANK,
          &WORK[0], &LWORK, &IWORK[0],
          &INFO);
         
  return INFO;
}

template<>
int gelsd(const sliced_colmajor_matrix_local<float>& inMat,
          const sliced_colmajor_matrix_local<float>& outMat,
          float RCOND){
   
  std::vector<float> S_VAL;
  int RANK = 0;

  return gelsd<float>(inMat, outMat, S_VAL, RANK, RCOND);
}
 

template<>
int gelsd(const sliced_colmajor_matrix_local<double>& inMat,
          const sliced_colmajor_matrix_local<double>& outMat,
          std::vector<double>& S_VAL,
          int& RANK,
          double RCOND){

#ifdef ERROR_CHK
  ASSERT_PTR(inMat.data && outMat.data);

  if(!inMat.is_valid() || !outMat.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");
  
  size_t maxSize = std::max(inMat.local_num_row, inMat.local_num_col);
  if(outMat.local_num_row < maxSize || outMat.local_num_col < 1)
     REPORT_ERROR(USER_ERROR,"Invalid right hand side matrix!!\n");
#endif 

  size_t minSize = std::min(inMat.local_num_row, inMat.local_num_col);
  S_VAL.resize(minSize,0);

  int M = static_cast<int>(inMat.local_num_row);
  int N = static_cast<int>(inMat.local_num_col);
  int NRHS = static_cast<int>(outMat.local_num_col);

  double* Aptr = inMat.data;
  int LDA = static_cast<int>(inMat.ldm);

  double* Bptr = outMat.data;
  int LDB = static_cast<int>(outMat.ldm);

  int INFO = 0;
  int LWORK = -1, LIWORK = -1; 
  std::vector<double> WORK(1);
  std::vector<int> IWORK(1);

  // LWORK query...
  dgelsd_(&M, &N, &NRHS,
          Aptr, &LDA,
          Bptr, &LDB,
          &S_VAL[0],
          &RCOND, &RANK,
          &WORK[0], &LWORK, &IWORK[0],
          &INFO);
  
  LWORK = WORK[0];
  LIWORK = IWORK[0];
  WORK.resize(LWORK,0);
  IWORK.resize(LIWORK,0);
       
  // Actual call...
  dgelsd_(&M, &N, &NRHS,
          Aptr, &LDA,
          Bptr, &LDB,
          &S_VAL[0],
          &RCOND, &RANK,
          &WORK[0], &LWORK, &IWORK[0],
          &INFO);
         
  return INFO;
}

 
template<>
int gelsd(const sliced_colmajor_matrix_local<double>& inMat,
          const sliced_colmajor_matrix_local<double>& outMat,
          double RCOND){
   
  std::vector<double> S_VAL;
  int RANK = 0;

  return gelsd<double>(inMat, outMat, S_VAL, RANK, RCOND);
}

template<>
int geev(const sliced_colmajor_matrix_local<float>& inMat,
         std::vector<float>& E_VAL){

#ifdef ERROR_CHK
  ASSERT_PTR(inMat.data);

  if(!inMat.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

  if(inMat.local_num_row != inMat.local_num_col)
    REPORT_ERROR(USER_ERROR,"The input matrix is not a square matrix!!\n");
#endif 

  char JOBZ = 'N';
  int N = static_cast<int>(inMat.local_num_row); // inMat.local_num_col;
  float* Aptr = inMat.data;
  int LDA = static_cast<int>(inMat.ldm);

  float *VL = NULL, *VR = NULL;
  int LDVL = 1, LDVR = 1;

  int INFO = 0;
  int LWORK = -1; 
  std::vector<float> WORK(1);
  
  E_VAL.resize(N,0);
  std::vector<float> WI(N,0);

  // LWORK query...
  sgeev_(&JOBZ, &JOBZ,
         &N,
         Aptr, &LDA,
         &E_VAL[0], &WI[0],
         VL, &LDVL,
         VR, &LDVR,
         &WORK[0], &LWORK,
         &INFO);

  LWORK = WORK[0];
  WORK.resize(LWORK,0);

  // Actual call...
  sgeev_(&JOBZ, &JOBZ,
         &N,
         Aptr, &LDA,
         &E_VAL[0], &WI[0],
         VL, &LDVL,
         VR, &LDVR,
         &WORK[0], &LWORK,
         &INFO);

  return INFO;
}

template<>
int geev(const sliced_colmajor_matrix_local<double>& inMat,
         std::vector<double>& E_VAL){

#ifdef ERROR_CHK
  ASSERT_PTR(inMat.data);

  if(!inMat.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

  if(inMat.local_num_row != inMat.local_num_col)
    REPORT_ERROR(USER_ERROR,"The input matrix is not a square matrix!!\n");
#endif 

  char JOBZ = 'N';
  int N = static_cast<int>(inMat.local_num_row); // inMat.local_num_col;
  double* Aptr = inMat.data;
  int LDA = static_cast<int>(inMat.ldm);

  double *VL = NULL, *VR = NULL;
  int LDVL = 1, LDVR = 1;

  int INFO = 0;
  int LWORK = -1; 
  std::vector<double> WORK(1);
  
  E_VAL.resize(N,0);
  std::vector<double> WI(N,0);

  // LWORK query...
  dgeev_(&JOBZ, &JOBZ,
         &N,
         Aptr, &LDA,
         &E_VAL[0], &WI[0],
         VL, &LDVL,
         VR, &LDVR,
         &WORK[0], &LWORK,
         &INFO);

  LWORK = WORK[0];
  WORK.resize(LWORK,0);

  // Actual call...
  dgeev_(&JOBZ, &JOBZ,
         &N,
         Aptr, &LDA,
         &E_VAL[0], &WI[0],
         VL, &LDVL,
         VR, &LDVR,
         &WORK[0], &LWORK,
         &INFO);

  return INFO;
}


template<>
int geev(const sliced_colmajor_matrix_local<float>& inMat,
         std::vector<float>& E_VAL,
         const sliced_colmajor_matrix_local<float>& E_VEC,
         char JOBZ){

#ifdef ERROR_CHK  
  ASSERT_PTR(inMat.data);

  if(!inMat.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

  if(!E_VEC.data)
    REPORT_ERROR(USER_ERROR,
                      "Output matrix for eigen vectors is not allocated!!\n");

  if(!E_VEC.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid output matrix for eigen vectors !!\n");

  if(inMat.local_num_row != inMat.local_num_col)
    REPORT_ERROR(USER_ERROR,"The input matrix is not a square matrix!!\n");

  if(E_VEC.local_num_row < inMat.local_num_row ||
     E_VEC.local_num_col < inMat.local_num_row)
    REPORT_ERROR(USER_ERROR,"Incompatible size of the 3rd argument matrix!!\n");
#endif 

  char JOBVL = 'N', JOBVR = 'N';
  float *VL = NULL, *VR = NULL;
  int LDVL = 1, LDVR = 1;

  if(JOBZ == 'L'){
    JOBVL = 'V';
    VL = E_VEC.data;
    LDVL = E_VEC.ldm;
  }
  else if (JOBZ == 'R'){
    JOBVR = 'V';
    VR = E_VEC.data;
    LDVR = E_VEC.ldm;
  }
  else
    REPORT_ERROR(USER_ERROR,"Illegal value provided for the 4th argument!!\n");

  int N = static_cast<int>(inMat.local_num_row); // inMat.local_num_col;
  float* Aptr = inMat.data;
  int LDA = static_cast<int>(inMat.ldm);

  int INFO = 0;
  int LWORK = -1; 
  std::vector<float> WORK(1);

  E_VAL.resize(N,0);
  std::vector<float> WI(N,0);

  // LWORK query...
  sgeev_(&JOBVL, &JOBVR,
         &N,
         Aptr, &LDA,
         &E_VAL[0], &WI[0],
         VL, &LDVL,
         VR, &LDVR,
         &WORK[0], &LWORK,
         &INFO );

  LWORK = WORK[0];
  WORK.resize(LWORK,0);

  // Actual call...
  sgeev_(&JOBVL, &JOBVR,
         &N,
         Aptr, &LDA,
         &E_VAL[0], &WI[0],
         VL, &LDVL,
         VR, &LDVR,
         &WORK[0], &LWORK,
         &INFO );

  return INFO;
}

template<>
int geev(const sliced_colmajor_matrix_local<double>& inMat,
         std::vector<double>& E_VAL,
         const sliced_colmajor_matrix_local<double>& E_VEC,
         char JOBZ ){

#ifdef ERROR_CHK  
  ASSERT_PTR(inMat.data);

  if(!inMat.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

  if(!E_VEC.data)
    REPORT_ERROR(USER_ERROR,
                      "Output matrix for eigen vectors is not allocated!!\n");

  if(!E_VEC.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid output matrix for eigen vectors!!\n");

  if(inMat.local_num_row != inMat.local_num_col)
    REPORT_ERROR(USER_ERROR,"The input matrix is not a square matrix!!\n");

  if(E_VEC.local_num_row < inMat.local_num_row ||
     E_VEC.local_num_col < inMat.local_num_row)
    REPORT_ERROR(USER_ERROR,"Incompatible size of the 3rd argument matrix!!\n");
#endif 

  char JOBVL = 'N', JOBVR = 'N';
  double *VL = NULL, *VR = NULL;
  int LDVL = 1, LDVR = 1;

  if(JOBZ == 'L'){
    JOBVL = 'V';
    VL = E_VEC.data;
    LDVL = E_VEC.ldm;
  }
  else if (JOBZ == 'R'){
    JOBVR = 'V';
    VR = E_VEC.data;
    LDVR = E_VEC.ldm;
  }
  else
    REPORT_ERROR(USER_ERROR,"Illegal value provided for the 4th argument!!\n");

  int N = static_cast<int>(inMat.local_num_row); // inMat.local_num_col;
  double* Aptr = inMat.data;
  int LDA = static_cast<int>(inMat.ldm);

  int INFO = 0;
  int LWORK = -1; 
  std::vector<double> WORK(1);

  E_VAL.resize(N,0);
  std::vector<double> WI(N,0);

  // LWORK query...
  dgeev_(&JOBVL, &JOBVR,
         &N,
         Aptr, &LDA,
         &E_VAL[0], &WI[0],
         VL, &LDVL,
         VR, &LDVR,
         &WORK[0], &LWORK,
         &INFO );

  LWORK = WORK[0];
  WORK.resize(LWORK,0);

  // Actual call...
  dgeev_(&JOBVL, &JOBVR,
         &N,
         Aptr, &LDA,
         &E_VAL[0], &WI[0],
         VL, &LDVL,
         VR, &LDVR,
         &WORK[0], &LWORK,
         &INFO );

  return INFO;
}


template<>
int geev(const sliced_colmajor_matrix_local<float>& inMat,
         std::vector<float>& E_VAL,
         const sliced_colmajor_matrix_local<float>& L_VEC,
         const sliced_colmajor_matrix_local<float>& R_VEC){

#ifdef ERROR_CHK
  ASSERT_PTR(inMat.data);

  if(!inMat.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");
    
  if(!L_VEC.data || !R_VEC.data)
    REPORT_ERROR(USER_ERROR,
           "Output matrix for eigen vectors is not allocated!!\n");

  if(!L_VEC.is_valid() || !R_VEC.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid output matrix for eigen vectors!!\n");

  if(inMat.local_num_row != inMat.local_num_col)
    REPORT_ERROR(USER_ERROR,"The input matrix is not a square matrix!!\n");

  if(L_VEC.local_num_row < inMat.local_num_row ||
     L_VEC.local_num_col < inMat.local_num_row)
    REPORT_ERROR(USER_ERROR,"Incompatible size of the 3rd argument matrix!!\n");

  if(R_VEC.local_num_row < inMat.local_num_row ||
     R_VEC.local_num_col < inMat.local_num_row)
    REPORT_ERROR(USER_ERROR,"Incompatible size of the 4th argument matrix!!\n");
#endif 

  char JOBZ = 'V';
  int N = static_cast<int>(inMat.local_num_row); // inMat.local_num_col;
  float* Aptr = inMat.data;
  int LDA = static_cast<int>(inMat.ldm);

  float* VL = L_VEC.data;
  int LDVL = static_cast<int>(L_VEC.ldm);
  float* VR = R_VEC.data;
  int LDVR = static_cast<int>(R_VEC.ldm);

  int INFO = 0;
  int LWORK = -1; 
  std::vector<float> WORK(1);

  E_VAL.resize(N,0);
  std::vector<float> WI(N,0);

  // LWORK query...
  sgeev_(&JOBZ, &JOBZ,
         &N,
         Aptr, &LDA,
         &E_VAL[0], &WI[0],
         VL, &LDVL,
         VR, &LDVR,
         &WORK[0], &LWORK,
         &INFO);

  LWORK = WORK[0];
  WORK.resize(LWORK,0);

  // Actual call...
  sgeev_(&JOBZ, &JOBZ,
         &N,
         Aptr, &LDA,
         &E_VAL[0], &WI[0],
         VL, &LDVL,
         VR, &LDVR,
         &WORK[0], &LWORK,
         &INFO);

  return INFO;
}


template<>
int geev(const sliced_colmajor_matrix_local<double>& inMat,
         std::vector<double>& E_VAL,
         const sliced_colmajor_matrix_local<double>& L_VEC,
         const sliced_colmajor_matrix_local<double>& R_VEC){

#ifdef ERROR_CHK
  ASSERT_PTR(inMat.data);

  if(!inMat.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");
    
  if(!L_VEC.data || !R_VEC.data)
    REPORT_ERROR(USER_ERROR,
           "Output matrix for eigen vectors is not allocated!!\n");

  if(!L_VEC.is_valid() || !R_VEC.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid output matrix for eigen vectors!!\n");

  if(inMat.local_num_row != inMat.local_num_col)
    REPORT_ERROR(USER_ERROR,"The input matrix is not a square matrix!!\n");

  if(L_VEC.local_num_row < inMat.local_num_row ||
     L_VEC.local_num_col < inMat.local_num_row)
    REPORT_ERROR(USER_ERROR,"Incompatible size of the 3rd argument matrix!!\n");

  if(R_VEC.local_num_row < inMat.local_num_row ||
     R_VEC.local_num_col < inMat.local_num_row)
    REPORT_ERROR(USER_ERROR,"Incompatible size of the 4th argument matrix!!\n");
#endif 

  char JOBZ = 'V';
  int N = static_cast<int>(inMat.local_num_row); // inMat.local_num_col;
  double* Aptr = inMat.data;
  int LDA = static_cast<int>(inMat.ldm);

  double* VL = L_VEC.data;
  int LDVL = static_cast<int>(L_VEC.ldm);
  double* VR = R_VEC.data;
  int LDVR = static_cast<int>(R_VEC.ldm);

  int INFO = 0;
  int LWORK = -1; 
  std::vector<double> WORK(1);

  E_VAL.resize(N,0);
  std::vector<double> WI(N,0);

  // LWORK query...
  dgeev_(&JOBZ, &JOBZ,
         &N,
         Aptr, &LDA,
         &E_VAL[0], &WI[0],
         VL, &LDVL,
         VR, &LDVR,
         &WORK[0], &LWORK,
         &INFO);

  LWORK = WORK[0];
  WORK.resize(LWORK,0);

  // Actual call...
  dgeev_(&JOBZ, &JOBZ,
         &N,
         Aptr, &LDA,
         &E_VAL[0], &WI[0],
         VL, &LDVL,
         VR, &LDVR,
         &WORK[0], &LWORK,
         &INFO);

  return INFO;
}

template<>
int gesvd(const sliced_colmajor_matrix_local<float>& inMat,
          std::vector<float>& S_VAL,
          char OPTION ){

#ifdef ERROR_CHK
  ASSERT_PTR(inMat.data);

  if(!inMat.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");
#endif 
  
  char JOBU = 'N', JOBVT ='N';

  if(OPTION == 'L')
    JOBU = 'O';
  else if(OPTION == 'R')
    JOBVT = 'O';
  else if(OPTION != 'N')
    REPORT_ERROR(USER_ERROR,"Illegal value provided for 3rd argument!!\n");

  int M = static_cast<int>(inMat.local_num_row);
  int N = static_cast<int>(inMat.local_num_col);
  float* Aptr = inMat.data;
  int LDA = static_cast<int>(inMat.ldm);

  float *U = NULL, *VT = NULL;
  int LDU = 1, LDVT = 1;

  int INFO = 0;
  int LWORK = -1; 
  std::vector<float> WORK(1);
  S_VAL.resize(std::min(M,N),0);

  // LWORK query...
  sgesvd_(&JOBU, &JOBVT,
          &M, &N,
          Aptr, &LDA,
          &S_VAL[0],
          U, &LDU,
          VT, &LDVT,
          &WORK[0], &LWORK,
          &INFO);

  LWORK = WORK[0];
  WORK.resize(LWORK,0);

  // Actual call...
  sgesvd_(&JOBU, &JOBVT,
          &M, &N,
          Aptr, &LDA,
          &S_VAL[0],
          U, &LDU,
          VT, &LDVT,
          &WORK[0], &LWORK,
          &INFO);

  return INFO;
}

template<>
int gesvd(const sliced_colmajor_matrix_local<double>& inMat,
          std::vector<double>& S_VAL,
          char OPTION ){

#ifdef ERROR_CHK
  ASSERT_PTR(inMat.data);

  if(!inMat.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");
#endif 
  
  char JOBU = 'N', JOBVT ='N';

  if(OPTION == 'L')
    JOBU = 'O';
  else if(OPTION == 'R')
    JOBVT = 'O';
  else if(OPTION != 'N')
    REPORT_ERROR(USER_ERROR,"Illegal value provided for 3rd argument!!\n");

  int M = static_cast<int>(inMat.local_num_row);
  int N = static_cast<int>(inMat.local_num_col);
  double* Aptr = inMat.data;
  int LDA = static_cast<int>(inMat.ldm);

  double *U = NULL, *VT = NULL;
  int LDU = 1, LDVT = 1;

  int INFO = 0;
  int LWORK = -1; 
  std::vector<double> WORK(1);
  S_VAL.resize(std::min(M,N),0);

  // LWORK query...
  dgesvd_(&JOBU, &JOBVT,
          &M, &N,
          Aptr, &LDA,
          &S_VAL[0],
          U, &LDU,
          VT, &LDVT,
          &WORK[0], &LWORK,
          &INFO);

  LWORK = WORK[0];
  WORK.resize(LWORK,0);

  // Actual call...
  dgesvd_(&JOBU, &JOBVT,
          &M, &N,
          Aptr, &LDA,
          &S_VAL[0],
          U, &LDU,
          VT, &LDVT,
          &WORK[0], &LWORK,
          &INFO);

  return INFO;
}


template<>
int gesvd(const sliced_colmajor_matrix_local<float>& inMat,
          std::vector<float>& S_VAL,
          const sliced_colmajor_matrix_local<float>& S_VEC,
          char VEC_TYPE,
          char PART,
          char OPT_A){

#ifdef ERROR_CHK
  ASSERT_PTR(inMat.data);

  if(!inMat.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");
    
  if(!S_VEC.data)
    REPORT_ERROR(USER_ERROR,
      "Output matrix for singular vectors is not allocated!!\n");

  if(!S_VEC.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid 3rd argument matrix!!\n");

  if(OPT_A != 'O' && OPT_A != 'N')
    REPORT_ERROR(USER_ERROR,"Illegal value provided for 6th argument!!\n");

  if(PART != 'A' && PART != 'S')
    REPORT_ERROR(USER_ERROR,"Illegal value provided for 5th argument!!\n");
#endif 

  char JOBU = 'A', JOBVT = 'A';
  float *U = NULL, *VT = NULL;
  int LDU = 1, LDVT = 1;
  size_t MIN_MN = std::min(inMat.local_num_row, inMat.local_num_col);

  if(VEC_TYPE == 'L') {

    if(PART == 'A') { 
      if(S_VEC.local_num_row < inMat.local_num_row ||
         S_VEC.local_num_col < inMat.local_num_row)
        REPORT_ERROR(USER_ERROR, 
           "Incompatible size for the 3rd argument matrix!!\n");
    }
    else {
      if(S_VEC.local_num_row < inMat.local_num_row ||
         S_VEC.local_num_col < MIN_MN)
        REPORT_ERROR(USER_ERROR, 
           "Incompatible size for the 3rd argument matrix!!\n");
    }
    
    U = S_VEC.data;
    LDU = S_VEC.ldm;
    JOBU = PART;
    JOBVT = OPT_A;
  }
  else if(VEC_TYPE == 'R') {

    if(PART == 'A') {
      if(S_VEC.local_num_row < inMat.local_num_col ||
         S_VEC.local_num_col < inMat.local_num_col)
        REPORT_ERROR(USER_ERROR, 
           "Incompatible size for the 3rd argument matrix!!\n");
    }
    else {
      if(S_VEC.local_num_row < MIN_MN ||
         S_VEC.local_num_col < inMat.local_num_col)
        REPORT_ERROR(USER_ERROR, 
           "Incompatible size for the 3rd argument matrix!!\n");
    }

    VT = S_VEC.data;
    LDVT = S_VEC.ldm;
    JOBVT = PART;
    JOBU = OPT_A;
  }
  else
    REPORT_ERROR(USER_ERROR,"Illegal value provided for 4th argument!!\n");

  int M = static_cast<int>(inMat.local_num_row);
  int N = static_cast<int>(inMat.local_num_col);
  float* Aptr = inMat.data;
  int LDA = static_cast<int>(inMat.ldm);

  int INFO = 0;
  int LWORK = -1; 
  std::vector<float> WORK(1);

  S_VAL.resize(MIN_MN,0);

  // LWORK query...
  sgesvd_(&JOBU, &JOBVT,
          &M, &N,
          Aptr, &LDA,
          &S_VAL[0],
          U, &LDU,
          VT, &LDVT,
          &WORK[0], &LWORK,
          &INFO);
  
  LWORK = WORK[0];
  WORK.resize(LWORK,0);

  // Actual call...
  sgesvd_(&JOBU, &JOBVT,
          &M, &N,
          Aptr, &LDA,
          &S_VAL[0],
          U, &LDU,
          VT, &LDVT,
          &WORK[0], &LWORK,
          &INFO);

  return INFO;
}


template<>
int gesvd(const sliced_colmajor_matrix_local<double>& inMat,
          std::vector<double>& S_VAL,
          const sliced_colmajor_matrix_local<double>& S_VEC,
          char VEC_TYPE,
          char PART,
          char OPT_A){

#ifdef ERROR_CHK
  ASSERT_PTR(inMat.data);

  if(!inMat.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");
    
  if(!S_VEC.data)
    REPORT_ERROR(USER_ERROR,
      "Output matrix for singular vectors is not allocated!!\n");

  if(!S_VEC.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid 3rd argument matrix!!\n");

  if(OPT_A != 'O' && OPT_A != 'N')
    REPORT_ERROR(USER_ERROR,"Illegal value provided for 6th argument!!\n");

  if(PART != 'A' && PART != 'S')
    REPORT_ERROR(USER_ERROR,"Illegal value provided for 5th argument!!\n");
#endif 

  char JOBU = 'A', JOBVT = 'A';
  double *U = NULL, *VT = NULL;
  int LDU = 1, LDVT = 1;
  size_t MIN_MN = std::min(inMat.local_num_row, inMat.local_num_col);

  if(VEC_TYPE == 'L') {

    if(PART == 'A') { 
      if(S_VEC.local_num_row < inMat.local_num_row ||
         S_VEC.local_num_col < inMat.local_num_row)
        REPORT_ERROR(USER_ERROR, 
           "Incompatible size for the 3rd argument matrix!!\n");
    }
    else { 
      if(S_VEC.local_num_row < inMat.local_num_row ||
         S_VEC.local_num_col < MIN_MN)
        REPORT_ERROR(USER_ERROR, 
           "Incompatible size for the 3rd argument matrix!!\n");
    }
    
    U   = S_VEC.data;
    LDU = S_VEC.ldm;
    JOBU = PART;
    JOBVT = OPT_A;
  }
  else if(VEC_TYPE == 'R') {

    if(PART == 'A') {
      if(S_VEC.local_num_row < inMat.local_num_col ||
         S_VEC.local_num_col < inMat.local_num_col)
        REPORT_ERROR(USER_ERROR, 
           "Incompatible size for the 3rd argument matrix!!\n");
    }
    else {
      if(S_VEC.local_num_row < MIN_MN ||
         S_VEC.local_num_col < inMat.local_num_col)
        REPORT_ERROR(USER_ERROR, 
           "Incompatible size for the 3rd argument matrix!!\n");
    }

    VT = S_VEC.data;
    LDVT = S_VEC.ldm;
    JOBVT = PART;
    JOBU = OPT_A;
  }
  else
    REPORT_ERROR(USER_ERROR,"Illegal value provided for 4th argument!!\n");

  int M = static_cast<int>(inMat.local_num_row);
  int N = static_cast<int>(inMat.local_num_col);
  double* Aptr = inMat.data;
  int LDA = static_cast<int>(inMat.ldm);

  int INFO = 0;
  int LWORK = -1; 
  std::vector<double> WORK(1);

  S_VAL.resize(MIN_MN,0);

  // LWORK query...
  dgesvd_(&JOBU, &JOBVT,
          &M, &N,
          Aptr, &LDA,
          &S_VAL[0],
          U, &LDU,
          VT, &LDVT,
          &WORK[0], &LWORK,
          &INFO);
  
  LWORK = WORK[0];
  WORK.resize(LWORK,0);

  // Actual call...
  dgesvd_(&JOBU, &JOBVT,
          &M, &N,
          Aptr, &LDA,
          &S_VAL[0],
          U, &LDU,
          VT, &LDVT,
          &WORK[0], &LWORK,
          &INFO);

  return INFO;
}


template<>
int gesvd(const sliced_colmajor_matrix_local<float>& inMat,
          std::vector<float>& S_VAL,
          const sliced_colmajor_matrix_local<float>& L_VEC,
          const sliced_colmajor_matrix_local<float>& R_VEC,
          char PART_L,
          char PART_R) {

#ifdef ERROR_CHK
  ASSERT_PTR(inMat.data);

  if(!inMat.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");
    
  if(!L_VEC.data)
    REPORT_ERROR(USER_ERROR,
           "Output matrix for left singular vectors is not allocated!!\n");

  if(!R_VEC.data)
    REPORT_ERROR(USER_ERROR,
           "Output matrix for right singular vectors is not allocated!!\n");

  if(!L_VEC.is_valid() || !R_VEC.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid output matrix for singular vectors!!\n");
  
  size_t MIN_MN = std::min(inMat.local_num_row, inMat.local_num_col);

  if(PART_L == 'A') { 
     if(L_VEC.local_num_row < inMat.local_num_row ||
        L_VEC.local_num_col < inMat.local_num_row)
       REPORT_ERROR(USER_ERROR,
             "Incompatible size for the 3rd argument matrix!!\n");
  }
  else if(PART_L == 'S') {
     if(L_VEC.local_num_row < inMat.local_num_row ||
        L_VEC.local_num_col < MIN_MN)
       REPORT_ERROR(USER_ERROR,
             "Incompatible size for the 3rd argument matrix!!\n");
  }
  else
    REPORT_ERROR(USER_ERROR,"Illegal value provided for 5th argument!!\n");


  if(PART_R == 'A') { 
     if(R_VEC.local_num_row < inMat.local_num_col ||
        R_VEC.local_num_col < inMat.local_num_col)
       REPORT_ERROR(USER_ERROR,
             "Incompatible size for the 4th argument matrix!!\n");
  }
  else if(PART_R == 'S') {
     if(R_VEC.local_num_row < MIN_MN ||
        R_VEC.local_num_col < inMat.local_num_col)
       REPORT_ERROR(USER_ERROR,
             "Incompatible size for the 4th argument matrix!!\n");
  }
  else 
    REPORT_ERROR(USER_ERROR,"Illegal value provided for 6th argument!!\n");
#endif 

  int M = static_cast<int>(inMat.local_num_row);
  int N = static_cast<int>(inMat.local_num_col);
  float* Aptr = inMat.data;
  int LDA = static_cast<int>(inMat.ldm);

  float* U = L_VEC.data;
  int LDU = static_cast<int>(L_VEC.ldm);
  float* VT = R_VEC.data ;
  int LDVT = static_cast<int>(R_VEC.ldm);

  int INFO = 0;
  int LWORK = -1; 
  std::vector<float> WORK(1);
  S_VAL.resize(std::min(M,N),0);

  // LWORK query...
  sgesvd_(&PART_L, &PART_R,
          &M, &N,
          Aptr, &LDA,
          &S_VAL[0],
          U, &LDU,
          VT, &LDVT,
          &WORK[0], &LWORK,
          &INFO);

  LWORK = WORK[0];
  WORK.resize(LWORK,0);

  // Actual call...
  sgesvd_(&PART_L, &PART_R,
          &M, &N,
          Aptr, &LDA,
          &S_VAL[0],
          U, &LDU,
          VT, &LDVT,
          &WORK[0], &LWORK,
          &INFO);

  return INFO;
}


template<>
int gesvd(const sliced_colmajor_matrix_local<double>& inMat,
          std::vector<double>& S_VAL,
          const sliced_colmajor_matrix_local<double>& L_VEC,
          const sliced_colmajor_matrix_local<double>& R_VEC,
          char PART_L,
          char PART_R) {

#ifdef ERROR_CHK
  ASSERT_PTR(inMat.data);

  if(!inMat.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");
    
  if(!L_VEC.data)
    REPORT_ERROR(USER_ERROR,
           "Output matrix for left singular vectors is not allocated!!\n");

  if(!R_VEC.data)
    REPORT_ERROR(USER_ERROR,
           "Output matrix for right singular vectors is not allocated!!\n");

  if(!L_VEC.is_valid() || !R_VEC.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid output matrix for singular vectors!!\n");
  
  size_t MIN_MN = std::min(inMat.local_num_row, inMat.local_num_col);

  if(PART_L == 'A') { 
     if(L_VEC.local_num_row < inMat.local_num_row ||
        L_VEC.local_num_col < inMat.local_num_row)
       REPORT_ERROR(USER_ERROR,
             "Incompatible size for the 3rd argument matrix!!\n");
  }
  else if(PART_L == 'S') {
     if(L_VEC.local_num_row < inMat.local_num_row ||
        L_VEC.local_num_col < MIN_MN)
       REPORT_ERROR(USER_ERROR,
             "Incompatible size for the 3rd argument matrix!!\n");
  }
  else
    REPORT_ERROR(USER_ERROR,"Illegal value provided for 5th argument!!\n");


  if(PART_R == 'A') {
     if(R_VEC.local_num_row < inMat.local_num_col ||
        R_VEC.local_num_col < inMat.local_num_col)
       REPORT_ERROR(USER_ERROR,
             "Incompatible size for the 4th argument matrix!!\n");
  }
  else if(PART_R == 'S') {
     if(R_VEC.local_num_row < MIN_MN ||
        R_VEC.local_num_col < inMat.local_num_col)
       REPORT_ERROR(USER_ERROR,
             "Incompatible size for the 4th argument matrix!!\n");
  }
  else
    REPORT_ERROR(USER_ERROR,"Illegal value provided for 6th argument!!\n");
#endif 

  int M = static_cast<int>(inMat.local_num_row);
  int N = static_cast<int>(inMat.local_num_col);
  double* Aptr = inMat.data;
  int LDA = static_cast<int>(inMat.ldm);

  double* U = L_VEC.data;
  int LDU = static_cast<int>(L_VEC.ldm);
  double* VT = R_VEC.data ;
  int LDVT = static_cast<int>(R_VEC.ldm);

  int INFO = 0;
  int LWORK = -1; 
  std::vector<double> WORK(1);
  S_VAL.resize(std::min(M,N),0);

  // LWORK query...
  dgesvd_(&PART_L, &PART_R,
          &M, &N,
          Aptr, &LDA,
          &S_VAL[0],
          U, &LDU,
          VT, &LDVT,
          &WORK[0], &LWORK,
          &INFO);

  LWORK = WORK[0];
  WORK.resize(LWORK,0);

  // Actual call...
  dgesvd_(&PART_L, &PART_R,
          &M, &N,
          Aptr, &LDA,
          &S_VAL[0],
          U, &LDU,
          VT, &LDVT,
          &WORK[0], &LWORK,
          &INFO);

  return INFO;
}

template<>
int gesdd(const sliced_colmajor_matrix_local<float>& inMat,
          std::vector<float>& S_VAL){

#ifdef ERROR_CHK
  ASSERT_PTR(inMat.data);

  if(!inMat.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");
#endif 
  
  char JOBZ = 'N';
  int M = static_cast<int>(inMat.local_num_row);
  int N = static_cast<int>(inMat.local_num_col);

  int MIN_MN = std::min(M,N);
  std::vector<int> IWORK(8*MIN_MN,0);
  S_VAL.resize(MIN_MN,0);

  float* Aptr = inMat.data;
  int LDA = static_cast<int>(inMat.ldm);

  float *U = NULL, *VT = NULL;
  int LDU = 1, LDVT = 1;

  int LWORK = -1;
  int INFO = 0;
  std::vector<float> WORK(1);

  // LWORK query...
  sgesdd_(&JOBZ,
          &M, &N,
          Aptr, &LDA,
          &S_VAL[0],
          U, &LDU,
          VT, &LDVT,
          &WORK[0], &LWORK, &IWORK[0],
          &INFO);

  LWORK = WORK[0];
  WORK.resize(LWORK,0);

  // Actual call...
  sgesdd_(&JOBZ,
          &M, &N,
          Aptr, &LDA,
          &S_VAL[0],
          U, &LDU,
          VT, &LDVT,
          &WORK[0], &LWORK, &IWORK[0],
          &INFO);

  return INFO;
}


template<>
int gesdd(const sliced_colmajor_matrix_local<double>& inMat,
          std::vector<double>& S_VAL){

#ifdef ERROR_CHK
  ASSERT_PTR(inMat.data);

  if(!inMat.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");
#endif 
  
  char JOBZ = 'N';
  int M = static_cast<int>(inMat.local_num_row);
  int N = static_cast<int>(inMat.local_num_col);

  int MIN_MN = std::min(M,N);
  std::vector<int> IWORK(8*MIN_MN,0);
  S_VAL.resize(MIN_MN,0);

  double* Aptr = inMat.data;
  int LDA = static_cast<int>(inMat.ldm);

  double *U = NULL, *VT = NULL;
  int LDU = 1, LDVT = 1;

  int LWORK = -1;
  int INFO = 0;
  std::vector<double> WORK(1);

  // LWORK query...
  dgesdd_(&JOBZ,
          &M, &N,
          Aptr, &LDA,
          &S_VAL[0],
          U, &LDU,
          VT, &LDVT,
          &WORK[0], &LWORK, &IWORK[0],
          &INFO);

  LWORK = WORK[0];
  WORK.resize(LWORK,0);

  // Actual call...
  dgesdd_(&JOBZ,
          &M, &N,
          Aptr, &LDA,
          &S_VAL[0],
          U, &LDU,
          VT, &LDVT,
          &WORK[0], &LWORK, &IWORK[0],
          &INFO);

  return INFO;
}


template<>
int gesdd(const sliced_colmajor_matrix_local<float>& inMat,
          std::vector<float>& S_VAL,
          const sliced_colmajor_matrix_local<float>& S_VEC){

#ifdef ERROR_CHK
  ASSERT_PTR(inMat.data);

  if(!inMat.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

  if(!S_VEC.data)
    REPORT_ERROR(USER_ERROR,
          "Output matrix for singular vectors is not allocated!!\n");

  if(!S_VEC.is_valid())
   REPORT_ERROR(USER_ERROR,"Invalid 3rd argument matrix!!\n");
#endif 
  
  int M = static_cast<int>(inMat.local_num_row);
  int N = static_cast<int>(inMat.local_num_col);

  char JOBZ = 'O';
  float *U = NULL, *VT = NULL;
  int LDU = 1, LDVT = 1;

  if(M >= N){
    if(S_VEC.local_num_row < inMat.local_num_col ||
       S_VEC.local_num_col < inMat.local_num_col)
      REPORT_ERROR(USER_ERROR,"Incompatible size of the 3rd argument matrix!!\n");
    VT = S_VEC.data;
    LDVT = S_VEC.ldm;
  }
  else{
    if(S_VEC.local_num_row < inMat.local_num_row ||
       S_VEC.local_num_col < inMat.local_num_row)
      REPORT_ERROR(USER_ERROR,"Incompatible size of the 3rd argument matrix!!\n");
    U = S_VEC.data;
    LDU = S_VEC.ldm;
  }

  int MIN_MN = std::min(M,N);
  std::vector<int> IWORK(8*MIN_MN,0);
  S_VAL.resize(MIN_MN,0);
 
  float *Aptr = inMat.data;
  int LDA = static_cast<int>(inMat.ldm);

  int INFO = 0;
  int LWORK = -1;
  std::vector<float> WORK(1);

  // LWORK query...
  sgesdd_(&JOBZ,
          &M, &N,
          Aptr, &LDA,
          &S_VAL[0],
          U, &LDU,
          VT, &LDVT,
          &WORK[0], &LWORK, &IWORK[0],
          &INFO);

  LWORK = WORK[0];
  WORK.resize(LWORK,0);

  // Actual call...
  sgesdd_(&JOBZ,
          &M, &N,
          Aptr, &LDA,
          &S_VAL[0],
          U, &LDU,
          VT, &LDVT,
          &WORK[0], &LWORK, &IWORK[0],
          &INFO);

  return INFO;
}

template<>
int gesdd(const sliced_colmajor_matrix_local<double>& inMat,
          std::vector<double>& S_VAL,
          const sliced_colmajor_matrix_local<double>& S_VEC){

#ifdef ERROR_CHK
  ASSERT_PTR(inMat.data);

  if(!inMat.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

  if(!S_VEC.data)
    REPORT_ERROR(USER_ERROR,
          "Output matrix for singular vectors is not allocated!!\n");

  if(!S_VEC.is_valid())
   REPORT_ERROR(USER_ERROR,"Invalid 3rd argument matrix!!\n");
#endif 
  
  int M = static_cast<int>(inMat.local_num_row);
  int N = static_cast<int>(inMat.local_num_col);

  char JOBZ = 'O';
  double *U = NULL, *VT = NULL;
  int LDU = 1, LDVT = 1;

  if(M >= N){
    if(S_VEC.local_num_row < inMat.local_num_col ||
       S_VEC.local_num_col < inMat.local_num_col)
      REPORT_ERROR(USER_ERROR,"Incompatible size of the 3rd argument matrix!!\n");
    VT = S_VEC.data;
    LDVT = S_VEC.ldm;
  }
  else{
    if(S_VEC.local_num_row < inMat.local_num_row ||
       S_VEC.local_num_col < inMat.local_num_row)
      REPORT_ERROR(USER_ERROR,"Incompatible size of the 3rd argument matrix!!\n");
    U = S_VEC.data;
    LDU = S_VEC.ldm;
  }

  int MIN_MN = std::min(M,N);
  std::vector<int> IWORK(8*MIN_MN,0);
  S_VAL.resize(MIN_MN,0);
 
  double *Aptr = inMat.data;
  int LDA = static_cast<int>(inMat.ldm);

  int INFO = 0;
  int LWORK = -1;
  std::vector<double> WORK(1);

  // LWORK query...
  dgesdd_(&JOBZ,
          &M, &N,
          Aptr, &LDA,
          &S_VAL[0],
          U, &LDU,
          VT, &LDVT,
          &WORK[0], &LWORK, &IWORK[0],
          &INFO);

  LWORK = WORK[0];
  WORK.resize(LWORK,0);

  // Actual call...
  dgesdd_(&JOBZ,
          &M, &N,
          Aptr, &LDA,
          &S_VAL[0],
          U, &LDU,
          VT, &LDVT,
          &WORK[0], &LWORK, &IWORK[0],
          &INFO);

  return INFO;
}


template<>
int gesdd(const sliced_colmajor_matrix_local<float>& inMat,
          std::vector<float>& S_VAL,
          const sliced_colmajor_matrix_local<float>& L_VEC,
          const sliced_colmajor_matrix_local<float>& R_VEC,
          char PART_LR ){

#ifdef ERROR_CHK
  ASSERT_PTR(inMat.data);

  if(!inMat.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");
    
  if(!L_VEC.data || !R_VEC.data)
    REPORT_ERROR(USER_ERROR,
          "Output matrix for singular vectors is not allocated!!\n");

  if(!L_VEC.is_valid() || !R_VEC.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid output matrix for singular vectors!!\n");

  size_t MIN_MN = std::min(inMat.local_num_row,inMat.local_num_col);

  if(PART_LR == 'A') {
    if(L_VEC.local_num_row < inMat.local_num_row ||
       L_VEC.local_num_col < inMat.local_num_row)
      REPORT_ERROR(USER_ERROR,"Incompatible size of the 3rd argument matrix!!\n");

    if(R_VEC.local_num_row < inMat.local_num_col ||
       R_VEC.local_num_col < inMat.local_num_col)
      REPORT_ERROR(USER_ERROR,"Incompatible size of the 4th argument matrix!!\n");
  }
  else if(PART_LR == 'S') {
    if(L_VEC.local_num_row < inMat.local_num_row ||
       L_VEC.local_num_col < MIN_MN)
      REPORT_ERROR(USER_ERROR,"Incompatible size of the 3rd argument matrix!!\n");

    if(R_VEC.local_num_row < MIN_MN ||
       R_VEC.local_num_col < inMat.local_num_col)
      REPORT_ERROR(USER_ERROR,"Incompatible size of the 4th argument matrix!!\n");
  }
  else
    REPORT_ERROR(USER_ERROR,"Illegal value provided for the 5th argument!!\n");
#endif 
    
  int M = static_cast<int>(inMat.local_num_row);
  int N = static_cast<int>(inMat.local_num_col);
  int min = std::min(M,N);

  float* Aptr = inMat.data;
  int LDA = static_cast<int>(inMat.ldm);

  float* U = L_VEC.data;
  int LDU = static_cast<int>(L_VEC.ldm);

  float* VT = R_VEC.data;
  int LDVT = static_cast<int>(R_VEC.ldm);

  std::vector<int> IWORK(8*min,0);
  S_VAL.resize(min,0);

  int INFO = 0;
  int LWORK = -1;
  std::vector<float> WORK(1);

  // LWORK query...
  sgesdd_(&PART_LR,
          &M, &N,
          Aptr, &LDA,
          &S_VAL[0],
          U, &LDU,
          VT, &LDVT,
          &WORK[0], &LWORK, &IWORK[0],
          &INFO);

  LWORK = WORK[0];
  WORK.resize(LWORK,0);

  // Actual call...
  sgesdd_(&PART_LR,
          &M, &N,
          Aptr, &LDA,
          &S_VAL[0],
          U, &LDU,
          VT, &LDVT,
          &WORK[0], &LWORK, &IWORK[0],
          &INFO);

  return INFO;
}

template<>
int gesdd(const sliced_colmajor_matrix_local<double>& inMat,
          std::vector<double>& S_VAL,
          const sliced_colmajor_matrix_local<double>& L_VEC,
          const sliced_colmajor_matrix_local<double>& R_VEC,
          char PART_LR ){

#ifdef ERROR_CHK
  ASSERT_PTR(inMat.data);

  if(!inMat.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");
    
  if(!L_VEC.data || !R_VEC.data)
    REPORT_ERROR(USER_ERROR,
          "Output matrix for singular vectors is not allocated!!\n");

  if(!L_VEC.is_valid() || !R_VEC.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid output matrix for singular vectors!!\n");

  size_t MIN_MN = std::min(inMat.local_num_row,inMat.local_num_col);

  if(PART_LR == 'A') {
    if(L_VEC.local_num_row < inMat.local_num_row ||
       L_VEC.local_num_col < inMat.local_num_row)
      REPORT_ERROR(USER_ERROR,"Incompatible size of the 3rd argument matrix!!\n");

    if(R_VEC.local_num_row < inMat.local_num_col ||
       R_VEC.local_num_col < inMat.local_num_col)
      REPORT_ERROR(USER_ERROR,"Incompatible size of the 4th argument matrix!!\n");
  }
  else if(PART_LR == 'S') {
    if(L_VEC.local_num_row < inMat.local_num_row ||
       L_VEC.local_num_col < MIN_MN)
      REPORT_ERROR(USER_ERROR,"Incompatible size of the 3rd argument matrix!!\n");

    if(R_VEC.local_num_row < MIN_MN ||
       R_VEC.local_num_col < inMat.local_num_col)
      REPORT_ERROR(USER_ERROR,"Incompatible size of the 4th argument matrix!!\n");
  }
  else
    REPORT_ERROR(USER_ERROR,"Illegal value provided for the 5th argument!!\n");
#endif 

  int M = static_cast<int>(inMat.local_num_row);
  int N = static_cast<int>(inMat.local_num_col);
  int min = std::min(M,N);

  double* Aptr = inMat.data;
  int LDA = static_cast<int>(inMat.ldm);

  double* U = L_VEC.data;
  int LDU = static_cast<int>(L_VEC.ldm);

  double* VT = R_VEC.data;
  int LDVT = static_cast<int>(R_VEC.ldm);

  std::vector<int> IWORK(8*min,0);
  S_VAL.resize(min,0);

  int INFO = 0;
  int LWORK = -1;
  std::vector<double> WORK(1);

  // LWORK query...
  dgesdd_(&PART_LR,
          &M, &N,
          Aptr, &LDA,
          &S_VAL[0],
          U, &LDU,
          VT, &LDVT,
          &WORK[0], &LWORK, &IWORK[0],
          &INFO);

  LWORK = WORK[0];
  WORK.resize(LWORK,0);

  // Actual call...
  dgesdd_(&PART_LR,
          &M, &N,
          Aptr, &LDA,
          &S_VAL[0],
          U, &LDU,
          VT, &LDVT,
          &WORK[0], &LWORK, &IWORK[0],
          &INFO);

  return INFO;
}
  
template<>
int gelsy(const sliced_colmajor_matrix_local<float>& inMat,
          const sliced_colmajor_matrix_local<float>& outMat,
          int& RANK,
          float RCOND){
 
#ifdef ERROR_CHK 
  ASSERT_PTR(inMat.data && outMat.data);

  if(!inMat.is_valid() || !outMat.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

  size_t maxSize = std::max(inMat.local_num_row,inMat.local_num_col);
  if(outMat.local_num_row < maxSize || outMat.local_num_col < 1)
     REPORT_ERROR(USER_ERROR,"Invalid right hand side matrix!!\n");
#endif 
 
  int M = static_cast<int>(inMat.local_num_row);
  int N = static_cast<int>(inMat.local_num_col);
  int NRHS = static_cast<int>(outMat.local_num_col);

  float *Aptr = inMat.data;
  int LDA = static_cast<int>(inMat.ldm);

  float *Bptr = outMat.data;
  int LDB = static_cast<int>(outMat.ldm);

  int INFO = 0;
  int LWORK = -1; 
  std::vector<float> WORK(1);

  // Initialize JPVT to be zero so that all columns are free
  std::vector<int> JPVT(N,0);

  // LWORK query...
  sgelsy_(&M, &N, &NRHS,
          Aptr, &LDA,
          Bptr, &LDB,
          &JPVT[0],
          &RCOND, &RANK,
          &WORK[0], &LWORK,
          &INFO);

  LWORK = WORK[0];
  WORK.resize(LWORK,0);

  // Actual call...
  sgelsy_(&M, &N, &NRHS,
          Aptr, &LDA,
          Bptr, &LDB,
          &JPVT[0],
          &RCOND, &RANK,
          &WORK[0], &LWORK,
          &INFO);

  return INFO;
}

template<>
int gelsy(const sliced_colmajor_matrix_local<float>& inMat,
          const sliced_colmajor_matrix_local<float>& outMat,
	  float RCOND){

   int RANK = 0;
   return gelsy<float>(inMat,outMat,RANK,RCOND);
}

template<>
int gelsy(const sliced_colmajor_matrix_local<double>& inMat,
          const sliced_colmajor_matrix_local<double>& outMat,
          int& RANK,
          double RCOND){

#ifdef ERROR_CHK
  ASSERT_PTR(inMat.data && outMat.data);

  if(!inMat.is_valid() || !outMat.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

  size_t maxSize = std::max(inMat.local_num_row,inMat.local_num_col);
  if(outMat.local_num_row < maxSize || outMat.local_num_col < 1)
     REPORT_ERROR(USER_ERROR,"Invalid right hand side matrix!!\n");
#endif 
 
  int M = static_cast<int>(inMat.local_num_row);
  int N = static_cast<int>(inMat.local_num_col);
  int NRHS = static_cast<int>(outMat.local_num_col);

  double* Aptr = inMat.data;
  int LDA = static_cast<int>(inMat.ldm);

  double* Bptr = outMat.data;
  int LDB = static_cast<int>(outMat.ldm);

  int INFO = 0;
  int LWORK = -1; 
  std::vector<double> WORK(1);

  // Initialize JPVT to be zero so that all columns are free
  std::vector<int> JPVT(N,0);

  // LWORK query...
  dgelsy_(&M, &N, &NRHS,
          Aptr, &LDA,
          Bptr, &LDB,
          &JPVT[0],
          &RCOND, &RANK,
          &WORK[0], &LWORK,
          &INFO);

  LWORK = WORK[0];
  WORK.resize(LWORK,0);

  // Actual call...
  dgelsy_(&M, &N, &NRHS,
          Aptr, &LDA,
          Bptr, &LDB,
          &JPVT[0],
          &RCOND, &RANK,
          &WORK[0], &LWORK,
          &INFO);

  return INFO;
}

template<>
int gelsy(const sliced_colmajor_matrix_local<double>& inMat,
          const sliced_colmajor_matrix_local<double>& outMat,
	  double RCOND){

   int RANK = 0;
   return gelsy<double>(inMat,outMat,RANK,RCOND);
}

template<>
int gelss(const sliced_colmajor_matrix_local<float>& inMat,
          const sliced_colmajor_matrix_local<float>& outMat,
	  std::vector<float>& S_VAL,
          int& RANK,
          float RCOND){

#ifdef ERROR_CHK
  ASSERT_PTR(inMat.data && outMat.data);

  if(!inMat.is_valid() || !outMat.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

  size_t maxSize = std::max(inMat.local_num_row,inMat.local_num_col);
  if(outMat.local_num_row < maxSize || outMat.local_num_col < 1)
     REPORT_ERROR(USER_ERROR,"Invalid right hand side matrix!!\n");
#endif 
     
  int M = static_cast<int>(inMat.local_num_row);
  int N = static_cast<int>(inMat.local_num_col);
  int NRHS = static_cast<int>(outMat.local_num_col);

  int MIN_MN = std::min(M,N);
  S_VAL.resize(MIN_MN,0);

  float *Aptr = inMat.data;
  int LDA = static_cast<int>(inMat.ldm);

  float *Bptr = outMat.data;
  int LDB = static_cast<int>(outMat.ldm);

  int INFO = 0;
  int LWORK = -1;
  std::vector<float> WORK(1);

  // LWORK query...
  sgelss_(&M, &N, &NRHS,
          Aptr, &LDA,
          Bptr, &LDB,
          &S_VAL[0],
          &RCOND, &RANK,
          &WORK[0], &LWORK,
          &INFO);

  LWORK = WORK[0];
  WORK.resize(LWORK,0);

  // Actual call...
  sgelss_(&M, &N, &NRHS,
          Aptr, &LDA,
          Bptr, &LDB,
          &S_VAL[0],
          &RCOND, &RANK,
          &WORK[0], &LWORK,
          &INFO);

  return INFO;
}

template<>
int gelss(const sliced_colmajor_matrix_local<float>& inMat,
          const sliced_colmajor_matrix_local<float>& outMat,
          float RCOND ){

  std::vector<float> S_VAL;
  int RANK = 0;
  return gelss<float>(inMat,outMat,S_VAL,RANK,RCOND);
}

template<>
int gelss(const sliced_colmajor_matrix_local<double>& inMat,
          const sliced_colmajor_matrix_local<double>& outMat,
          std::vector<double>& S_VAL,
          int& RANK,
          double RCOND ){

#ifdef ERROR_CHK
  ASSERT_PTR(inMat.data && outMat.data);

  if(!inMat.is_valid() || !outMat.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

  size_t maxSize = std::max(inMat.local_num_row,inMat.local_num_col);
  if(outMat.local_num_row < maxSize || outMat.local_num_col < 1)
     REPORT_ERROR(USER_ERROR,"Invalid right hand side matrix!!\n");
#endif 
     
  int M = static_cast<int>(inMat.local_num_row);
  int N = static_cast<int>(inMat.local_num_col);
  int NRHS = static_cast<int>(outMat.local_num_col);

  int MIN_MN = std::min(M,N);
  S_VAL.resize(MIN_MN,0);

  double *Aptr = inMat.data;
  int LDA = static_cast<int>(inMat.ldm);

  double *Bptr = outMat.data;
  int LDB = static_cast<int>(outMat.ldm);

  int INFO = 0;
  int LWORK = -1;
  std::vector<double> WORK(1);

  // LWORK query...
  dgelss_(&M, &N, &NRHS,
          Aptr, &LDA,
          Bptr, &LDB,
          &S_VAL[0],
          &RCOND, &RANK,
          &WORK[0], &LWORK,
          &INFO);

  LWORK = WORK[0];
  WORK.resize(LWORK,0);

  // Actual call...
  dgelss_(&M, &N, &NRHS,
          Aptr, &LDA,
          Bptr, &LDB,
          &S_VAL[0],
          &RCOND, &RANK,
          &WORK[0], &LWORK,
          &INFO);

  return INFO;
}

template<>
int gelss(const sliced_colmajor_matrix_local<double>& inMat,
          const sliced_colmajor_matrix_local<double>& outMat,
          double RCOND ){

  std::vector<double> S_VAL;
  int RANK = 0;
  return gelss<double>(inMat,outMat,S_VAL,RANK,RCOND);
}

template<>
int getrf(const sliced_colmajor_matrix_local<float>& inMat,
          std::vector<int>& IPIV){

#ifdef ERROR_CHK
  ASSERT_PTR(inMat.data);

  if(!inMat.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");
#endif 

  int M = static_cast<int>(inMat.local_num_row);
  int N = static_cast<int>(inMat.local_num_col);
  int MIN_MN = std::min(M,N);

  float* Aptr = inMat.data;
  int LDA = static_cast<int>(inMat.ldm);

  int INFO = 0;
  IPIV.resize(MIN_MN,0);

  sgetrf_(&M, &N,
          Aptr, &LDA,
          &IPIV[0], &INFO);

  return INFO;
}

template<>
int getrf(const sliced_colmajor_matrix_local<double>& inMat,
          std::vector<int>& IPIV){

#ifdef ERROR_CHK
  ASSERT_PTR(inMat.data);

  if(!inMat.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");
#endif 

  int M = static_cast<int>(inMat.local_num_row);
  int N = static_cast<int>(inMat.local_num_col);
  int MIN_MN = std::min(M,N);

  double* Aptr = inMat.data;
  int LDA = static_cast<int>(inMat.ldm);

  int INFO = 0;
  IPIV.resize(MIN_MN,0);

  dgetrf_(&M, &N,
          Aptr, &LDA,
          &IPIV[0], &INFO);

  return INFO;
}

template<>
int getri(const sliced_colmajor_matrix_local<float>& inMat,
          const std::vector<int>& IPIV){

#ifdef ERROR_CHK
  ASSERT_PTR(inMat.data);

  if(!inMat.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

  if(inMat.local_num_row != inMat.local_num_col)
    REPORT_ERROR(USER_ERROR,"The input matrix is not a square matrix!!\n");

  if(IPIV.size() < inMat.local_num_row)
    REPORT_ERROR(USER_ERROR,"Incompatible size of the input pivot array!!\n");
#endif 

  int N = static_cast<int>(inMat.local_num_row); // inMat.local_num_col;
  float* Aptr = inMat.data;
  int LDA = static_cast<int>(inMat.ldm);

  int INFO = 0;
  int LWORK = -1; 
  std::vector<float> WORK(1);

  // LWORK query...
  sgetri_(&N,
          Aptr, &LDA,
          &IPIV[0],
          &WORK[0], &LWORK,
          &INFO);

  LWORK = WORK[0];
  WORK.resize(LWORK,0);

  // Actual call...
  sgetri_(&N,
          Aptr, &LDA,
          &IPIV[0],
          &WORK[0], &LWORK,
          &INFO);

  return INFO;
}

template<>
int getri(const sliced_colmajor_matrix_local<double>& inMat,
          const std::vector<int>& IPIV){

#ifdef ERROR_CHK
  ASSERT_PTR(inMat.data);

  if(!inMat.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

  if(inMat.local_num_row != inMat.local_num_col)
    REPORT_ERROR(USER_ERROR,"The input matrix is not a square matrix!!\n");

  if(IPIV.size() < inMat.local_num_row)
    REPORT_ERROR(USER_ERROR,"Incompatible size of the input pivot array!!\n");
#endif 

  int N = static_cast<int>(inMat.local_num_row); // inMat.local_num_col;
  double* Aptr = inMat.data;
  int LDA = static_cast<int>(inMat.ldm);

  int INFO = 0;
  int LWORK = -1; 
  std::vector<double> WORK(1);

  // LWORK query...
  dgetri_(&N,
          Aptr, &LDA,
          &IPIV[0],
          &WORK[0], &LWORK,
          &INFO);

  LWORK = WORK[0];
  WORK.resize(LWORK,0);

  // Actual call...
  dgetri_(&N,
          Aptr, &LDA,
          &IPIV[0],
          &WORK[0], &LWORK,
          &INFO);

  return INFO;
}


template<>
int getrs(const sliced_colmajor_matrix_local<float>& inMat,
          const sliced_colmajor_matrix_local<float>& outMat,
          const std::vector<int>& IPIV,
          char TRANS ){

#ifdef ERROR_CHK
  ASSERT_PTR(inMat.data && outMat.data);

  if(!inMat.is_valid() || !outMat.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

  if(inMat.local_num_row != inMat.local_num_col)
    REPORT_ERROR(USER_ERROR,"The input matrix is not a square matrix!!\n");

  if(outMat.local_num_row < inMat.local_num_row ||
     outMat.local_num_col < 1)
    REPORT_ERROR(USER_ERROR, 
       "Incompatible size of the right hand side matrix!!\n");

  if(IPIV.size() < inMat.local_num_row)
    REPORT_ERROR(USER_ERROR,"Incompatible size of the input pivot array!!\n");

  if((TRANS != 'N') && (TRANS != 'T'))
    REPORT_ERROR(USER_ERROR,"Illegal value for the TRANS parameter!!\n");
#endif 

  int N = static_cast<int>(inMat.local_num_row); // inMat.local_num_col;
  int NRHS = static_cast<int>(outMat.local_num_col);

  float* Aptr = inMat.data;
  int LDA = static_cast<int>(inMat.ldm);

  float* Bptr = outMat.data;
  int LDB = static_cast<int>(outMat.ldm);

  int INFO = 0;

  sgetrs_(&TRANS,
          &N, &NRHS,
          Aptr, &LDA,
          &IPIV[0],
          Bptr, &LDB,
          &INFO);

  return INFO;
}

template<>
int getrs(const sliced_colmajor_matrix_local<double>& inMat,
          const sliced_colmajor_matrix_local<double>& outMat,
          const std::vector<int>& IPIV,
          char TRANS ){

#ifdef ERROR_CHK
  ASSERT_PTR(inMat.data && outMat.data);

  if(!inMat.is_valid() || !outMat.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

  if(inMat.local_num_row != inMat.local_num_col)
    REPORT_ERROR(USER_ERROR,"The input matrix is not a square matrix!!\n");

  if(outMat.local_num_row < inMat.local_num_row ||
     outMat.local_num_col < 1)
    REPORT_ERROR(USER_ERROR, 
       "Incompatible size of the right hand side matrix!!\n");

  if(IPIV.size() < inMat.local_num_row)
    REPORT_ERROR(USER_ERROR,"Incompatible size of the input pivot array!!\n");

  if((TRANS != 'N') && (TRANS != 'T'))
    REPORT_ERROR(USER_ERROR,"Illegal value for the TRANS parameter!!\n");
#endif 

  int N = static_cast<int>(inMat.local_num_row); // inMat.local_num_col;
  int NRHS = static_cast<int>(outMat.local_num_col);

  double* Aptr = inMat.data;
  int LDA = static_cast<int>(inMat.ldm);

  double* Bptr = outMat.data;
  int LDB = static_cast<int>(outMat.ldm);

  int INFO = 0;

  dgetrs_(&TRANS,
          &N, &NRHS,
          Aptr, &LDA,
          &IPIV[0],
          Bptr, &LDB,
          &INFO);

  return INFO;
}

}
