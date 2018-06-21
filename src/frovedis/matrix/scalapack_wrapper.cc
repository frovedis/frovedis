#include "scalapack_wrapper.hpp"
#include <boost/lexical_cast.hpp>
#define ITOS boost::lexical_cast<std::string>

namespace frovedis {

template <>
int pgetrf (sliced_blockcyclic_matrix_local<float>& inMat,
            std::vector<int>& IPIV) {

  int INFO = 0;

  if(is_active(inMat)) {
#ifdef ERROR_CHK
    ASSERT_PTR(inMat.data);

    if(!inMat.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");
#endif

    int M = static_cast<int>(inMat.sliced_num_row);
    int N = static_cast<int>(inMat.sliced_num_col);

    float* aptr = inMat.data;
    int IA = static_cast<int>(inMat.IA);
    int JA = static_cast<int>(inMat.JA);
    int* DESCA = inMat.descA;

    // --- checking for special conditions ---
    int MB_A, NB_A;
    MB_A = DESCA[4];
    NB_A = DESCA[5];

    int row_offsetA, col_offsetA;
    row_offsetA = (IA-1) % MB_A;
    col_offsetA = (JA-1) % NB_A;

    if (row_offsetA != 0 || col_offsetA != 0 || MB_A != NB_A) {
      std::string input_set = "\nIA: "   + ITOS(IA) +
                              ", JA: "   + ITOS(JA) +
                              ", MB_A: " + ITOS(MB_A) +
                              ", NB_A: " + ITOS(NB_A) + "\n";
      input_set += "IROFFA: "   + ITOS(row_offsetA) +
                   ", ICOFFA: " + ITOS(col_offsetA) + "\n";

      std::string msg = input_set;
      msg += "Input matrix internal distribution is incompatible for the ";
      msg += "requested operation!!\nPlease consider different number of ";
      msg += "parallel processes.\nOr you may change the internal distribution ";
      msg += "by specifying suitable blocksize while loading the global input matrix.\n";
      REPORT_ERROR(INTERNAL_ERROR, msg);
    }

    int ipivSize = DESCA[4] + DESCA[8];
    checkAssumption(ipivSize > 0);
    IPIV.resize(ipivSize, 0);

    psgetrf_(&M, &N,
             aptr, &IA, &JA, DESCA,
	     &IPIV[0], &INFO);
  }

  return INFO;
}

template <>
int pgetrf (sliced_blockcyclic_matrix_local<double>& inMat,
            std::vector<int>& IPIV) {

  int INFO = 0;

  if(is_active(inMat)) {
#ifdef ERROR_CHK
    ASSERT_PTR(inMat.data);

    if(!inMat.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");
#endif

    int M = static_cast<int>(inMat.sliced_num_row);
    int N = static_cast<int>(inMat.sliced_num_col);

    double* aptr = inMat.data;
    int IA = static_cast<int>(inMat.IA);
    int JA = static_cast<int>(inMat.JA);
    int* DESCA = inMat.descA;

    // --- checking for special conditions ---
    int MB_A, NB_A;
    MB_A = DESCA[4];
    NB_A = DESCA[5];

    int row_offsetA, col_offsetA;
    row_offsetA = (IA-1) % MB_A;
    col_offsetA = (JA-1) % NB_A;

    if (row_offsetA != 0 || col_offsetA != 0 || MB_A != NB_A) {
      std::string input_set = "\nIA: "   + ITOS(IA) +
                              ", JA: "   + ITOS(JA) +
                              ", MB_A: " + ITOS(MB_A) +
                              ", NB_A: " + ITOS(NB_A) + "\n";
      input_set += "IROFFA: "   + ITOS(row_offsetA) +
                   ", ICOFFA: " + ITOS(col_offsetA) + "\n";

      std::string msg = input_set;
      msg += "Input matrix internal distribution is incompatible for the ";
      msg += "requested operation!!\nPlease consider different number of ";
      msg += "parallel processes.\nOr you may change the internal distribution ";
      msg += "by specifying suitable blocksize while loading the global input matrix.\n";
      REPORT_ERROR(INTERNAL_ERROR, msg);
    }

    int ipivSize = DESCA[4] + DESCA[8];
    checkAssumption(ipivSize > 0);
    IPIV.resize(ipivSize, 0);

    pdgetrf_(&M, &N,
             aptr, &IA, &JA, DESCA,
	     &IPIV[0], &INFO);
  }

  return INFO;
}

template <>
int pgetri (sliced_blockcyclic_matrix_local<float>& inMat,
            std::vector<int>& IPIV) {

  int INFO = 0;

  if(is_active(inMat)) {
#ifdef ERROR_CHK
    ASSERT_PTR(inMat.data);

    if(!inMat.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

    if(inMat.sliced_num_row != inMat.sliced_num_col)
      REPORT_ERROR(USER_ERROR,
         "Matrix-inverse not possible: Input matrix is not a square matrix!!\n");
#endif

    int N = static_cast<int>(inMat.sliced_num_row); // inMat.sliced_num_col;

    float* aptr = inMat.data;
    int IA = static_cast<int>(inMat.IA);
    int JA = static_cast<int>(inMat.JA);
    int* DESCA = inMat.descA;

    // --- checking for special conditions ---
    int MB_A, NB_A;
    MB_A = DESCA[4];
    NB_A = DESCA[5];

    int row_offsetA, col_offsetA;
    row_offsetA = (IA-1) % MB_A;
    col_offsetA = (JA-1) % NB_A;

    if (row_offsetA != 0 || col_offsetA != 0 || MB_A != NB_A) {
      std::string input_set = "\nIA: "   + ITOS(IA) +
                              ", JA: "   + ITOS(JA) +
                              ", MB_A: " + ITOS(MB_A) +
                              ", NB_A: " + ITOS(NB_A) + "\n";
      input_set += "IROFFA: "   + ITOS(row_offsetA) +
                   ", ICOFFA: " + ITOS(col_offsetA) + "\n";

      std::string msg = input_set;
      msg += "Input matrix internal distribution is incompatible for the ";
      msg += "requested operation!!\nPlease consider different number of ";
      msg += "parallel processes.\nOr you may change the internal distribution ";
      msg += "by specifying suitable blocksize while loading the global input matrix.\n";
      REPORT_ERROR(INTERNAL_ERROR, msg);
    }

    int ipivSize = DESCA[4] + DESCA[8];
    checkAssumption(ipivSize > 0 && IPIV.size() >= ipivSize);

    int LWORK = -1, LIWORK = -1;
    std::vector<float> WORK(1); 
    std::vector<int> IWORK(1);  

    // LWORK and LIWORK query...
    psgetri_(&N,
             aptr, &IA, &JA, DESCA,
	     &IPIV[0], 
	     &WORK[0], &LWORK,
	     &IWORK[0], &LIWORK,
	     &INFO);

    LWORK  = WORK[0];
    LIWORK = IWORK[0];
    WORK.resize(LWORK, 0);
    IWORK.resize(LIWORK, 0);

    // Actual call...
    psgetri_(&N,
             aptr, &IA, &JA, DESCA,
	     &IPIV[0], 
	     &WORK[0], &LWORK,
	     &IWORK[0], &LIWORK,
	     &INFO);
  }

  return INFO;
}

template <>
int pgetri (sliced_blockcyclic_matrix_local<double>& inMat,
            std::vector<int>& IPIV) {

  int INFO = 0;

  if(is_active(inMat)) {
#ifdef ERROR_CHK
    ASSERT_PTR(inMat.data);

    if(!inMat.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

    if(inMat.sliced_num_row != inMat.sliced_num_col)
      REPORT_ERROR(USER_ERROR,
         "Matrix-inverse not possible: Input matrix is not a square matrix!!\n");
#endif

    int N = static_cast<int>(inMat.sliced_num_row); // inMat.sliced_num_col;

    double* aptr = inMat.data;
    int IA = static_cast<int>(inMat.IA);
    int JA = static_cast<int>(inMat.JA);
    int* DESCA = inMat.descA;

    // --- checking for special conditions ---
    int MB_A, NB_A;
    MB_A = DESCA[4];
    NB_A = DESCA[5];

    int row_offsetA, col_offsetA;
    row_offsetA = (IA-1) % MB_A;
    col_offsetA = (JA-1) % NB_A;

    if (row_offsetA != 0 || col_offsetA != 0 || MB_A != NB_A) {
      std::string input_set = "\nIA: "   + ITOS(IA) +
                              ", JA: "   + ITOS(JA) +
                              ", MB_A: " + ITOS(MB_A) +
                              ", NB_A: " + ITOS(NB_A) + "\n";
      input_set += "IROFFA: "   + ITOS(row_offsetA) +
                   ", ICOFFA: " + ITOS(col_offsetA) + "\n";

      std::string msg = input_set;
      msg += "Input matrix internal distribution is incompatible for the ";
      msg += "requested operation!!\nPlease consider different number of ";
      msg += "parallel processes.\nOr you may change the internal distribution ";
      msg += "by specifying suitable blocksize while loading the global input matrix.\n";
      REPORT_ERROR(INTERNAL_ERROR, msg);
    }

    int ipivSize = DESCA[4] + DESCA[8];
    checkAssumption(ipivSize > 0 && IPIV.size() >= ipivSize);

    int LWORK = -1, LIWORK = -1;
    std::vector<double> WORK(1); 
    std::vector<int> IWORK(1);  

    // LWORK and LIWORK query...
    pdgetri_(&N,
             aptr, &IA, &JA, DESCA,
	     &IPIV[0], 
	     &WORK[0], &LWORK,
	     &IWORK[0], &LIWORK,
	     &INFO);

    LWORK  = WORK[0];
    LIWORK = IWORK[0];
    WORK.resize(LWORK, 0);
    IWORK.resize(LIWORK, 0);

    // Actual call...
    pdgetri_(&N,
             aptr, &IA, &JA, DESCA,
	     &IPIV[0], 
	     &WORK[0], &LWORK,
	     &IWORK[0], &LIWORK,
	     &INFO);
  }

  return INFO;
}

template <>
int pgetrs (sliced_blockcyclic_matrix_local<float>& inMat,
            sliced_blockcyclic_matrix_local<float>& outMat,
            std::vector<int>& IPIV,
	    char TRANS ){

  int INFO = 0;

  if(!is_same_distribution(inMat,outMat))
    REPORT_FATAL(USER_ERROR,"Incompatible distribution of the inputs\n");
  // since inputs are of same distribution, activeness check on one input is ok
  if(is_active(inMat)) {
#ifdef ERROR_CHK
    ASSERT_PTR(inMat.data && outMat.data);

    if(!inMat.is_valid() || !outMat.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

    if(inMat.sliced_num_row != inMat.sliced_num_col)
      REPORT_ERROR(USER_ERROR,"The co-efficient matrix is not a square matrix!!\n");

    if(outMat.sliced_num_row < inMat.sliced_num_row ||
       outMat.sliced_num_col < 1)
      REPORT_ERROR(USER_ERROR,"Invalid right hand side matrix!!\n");
    
    if(TRANS != 'N' && TRANS != 'T')
      REPORT_ERROR(USER_ERROR,"Illegal value for the TRANS parameter!!\n");
#endif

    int N = static_cast<int>(inMat.sliced_num_row); // inMat.sliced_num_col;
    int NRHS = static_cast<int>(outMat.sliced_num_col);

    float* aptr = inMat.data;
    int IA = static_cast<int>(inMat.IA);
    int JA = static_cast<int>(inMat.JA);
    int* DESCA = inMat.descA;

    float* bptr = outMat.data;
    int IB = static_cast<int>(outMat.IA);
    int JB = static_cast<int>(outMat.JA);
    int* DESCB = outMat.descA;

    // --- checking for special conditions ---
    int MB_A, NB_A, MB_B;
    MB_A = DESCA[4];
    NB_A = DESCA[5];
    MB_B = DESCB[4];

    int row_offsetA, col_offsetA, row_offsetB;
    row_offsetA = (IA-1) % MB_A;
    col_offsetA = (JA-1) % NB_A;
    row_offsetB = (IB-1) % MB_B;

    if (row_offsetA != 0 || col_offsetA != 0 || col_offsetA != row_offsetB
        || MB_A != NB_A  || NB_A != MB_B) {
      std::string input_set = "\nIA: "   + ITOS(IA) +
                              ", JA: "   + ITOS(JA) +
                              ", MB_A: " + ITOS(MB_A) +
                              ", NB_A: " + ITOS(NB_A) +
                              ", IB: "   + ITOS(IB) +
                              ", MB_B: " + ITOS(MB_B) + "\n";
      input_set += "IROFFA: "   + ITOS(row_offsetA) +
                   ", ICOFFA: " + ITOS(col_offsetA) +
                   ", IROFFB: " + ITOS(row_offsetB) + "\n";

      std::string msg = input_set;
      msg += "Input matrix internal distribution is incompatible for the ";
      msg += "requested operation!!\nPlease consider different number of ";
      msg += "parallel processes.\nOr you may change the internal distribution ";
      msg += "by specifying suitable blocksize while loading the global input matrix.\n";
      REPORT_ERROR(INTERNAL_ERROR, msg);
    }

    int ipivSize = DESCA[4] + DESCA[8];
    checkAssumption(ipivSize > 0 && IPIV.size() >= ipivSize);

    psgetrs_(&TRANS,
             &N, &NRHS,
	     aptr, &IA, &JA, DESCA,
	     &IPIV[0],
	     bptr, &IB, &JB, DESCB,
	     &INFO);
  }

  return INFO;
}

template <>
int pgetrs (sliced_blockcyclic_matrix_local<double>& inMat,
            sliced_blockcyclic_matrix_local<double>& outMat,
            std::vector<int>& IPIV,
            char TRANS) {

  int INFO = 0;

  if(!is_same_distribution(inMat,outMat))
    REPORT_FATAL(USER_ERROR,"Incompatible distribution of the inputs\n");
  // since inputs are of same distribution, activeness check on one input is ok
  if(is_active(inMat)) {
#ifdef ERROR_CHK
    ASSERT_PTR(inMat.data && outMat.data);

    if(!inMat.is_valid() || !outMat.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

    if(inMat.sliced_num_row != inMat.sliced_num_col)
      REPORT_ERROR(USER_ERROR,"The co-efficient matrix is not a square matrix!!\n");

    if(outMat.sliced_num_row < inMat.sliced_num_row ||
       outMat.sliced_num_col < 1)
      REPORT_ERROR(USER_ERROR,"Invalid right hand side matrix!!\n");
    
    if(TRANS != 'N' && TRANS != 'T')
      REPORT_ERROR(USER_ERROR,"Illegal value for the TRANS parameter!!\n");
#endif

    int N = static_cast<int>(inMat.sliced_num_row); // inMat.sliced_num_col;
    int NRHS = static_cast<int>(outMat.sliced_num_col);

    double* aptr = inMat.data;
    int IA = static_cast<int>(inMat.IA);
    int JA = static_cast<int>(inMat.JA);
    int* DESCA = inMat.descA;

    double* bptr = outMat.data;
    int IB = static_cast<int>(outMat.IA);
    int JB = static_cast<int>(outMat.JA);
    int* DESCB = outMat.descA;

    // --- checking for special conditions ---
    int MB_A, NB_A, MB_B;
    MB_A = DESCA[4];
    NB_A = DESCA[5];
    MB_B = DESCB[4];

    int row_offsetA, col_offsetA, row_offsetB;
    row_offsetA = (IA-1) % MB_A;
    col_offsetA = (JA-1) % NB_A;
    row_offsetB = (IB-1) % MB_B;

    if (row_offsetA != 0 || col_offsetA != 0 || col_offsetA != row_offsetB
        || MB_A != NB_A  || NB_A != MB_B) {
      std::string input_set = "\nIA: "   + ITOS(IA) +
                              ", JA: "   + ITOS(JA) +
                              ", MB_A: " + ITOS(MB_A) +
                              ", NB_A: " + ITOS(NB_A) +
                              ", IB: "   + ITOS(IB) +
                              ", MB_B: " + ITOS(MB_B) + "\n";
      input_set += "IROFFA: "   + ITOS(row_offsetA) +
                   ", ICOFFA: " + ITOS(col_offsetA) +
                   ", IROFFB: " + ITOS(row_offsetB) + "\n";

      std::string msg = input_set;
      msg += "Input matrix internal distribution is incompatible for the ";
      msg += "requested operation!!\nPlease consider different number of ";
      msg += "parallel processes.\nOr you may change the internal distribution ";
      msg += "by specifying suitable blocksize while loading the global input matrix.\n";
      REPORT_ERROR(INTERNAL_ERROR, msg);
    }

    int ipivSize = DESCA[4] + DESCA[8];
    checkAssumption(ipivSize > 0 && IPIV.size() >= ipivSize);

    pdgetrs_(&TRANS,
             &N, &NRHS,
	     aptr, &IA, &JA, DESCA,
	     &IPIV[0],
	     bptr, &IB, &JB, DESCB,
	     &INFO);
  }

  return INFO;
}

template <>
int pgesv2 (sliced_blockcyclic_matrix_local<float>& inMat,
            sliced_blockcyclic_matrix_local<float>& outMat,
            std::vector<int>& IPIV) {

  int INFO = 0;
  
  if(!is_same_distribution(inMat,outMat))
    REPORT_FATAL(USER_ERROR,"Incompatible distribution of the inputs\n");
  // since inputs are of same distribution, activeness check on one input is ok
  if(is_active(inMat)) {
#ifdef ERROR_CHK
    ASSERT_PTR(inMat.data && outMat.data);

    if(!inMat.is_valid() || !outMat.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

    if(inMat.sliced_num_row != inMat.sliced_num_col)
      REPORT_ERROR(USER_ERROR,"The co-efficient matrix is not a square matrix!!\n");

    if(outMat.sliced_num_row < inMat.sliced_num_row ||
       outMat.sliced_num_col < 1)
      REPORT_ERROR(USER_ERROR,"Invalid right hand side matrix!!\n");
#endif
    
    int N = static_cast<int>(inMat.sliced_num_row); // inMat.sliced_num_col;
    int NRHS = static_cast<int>(outMat.sliced_num_col);

    float* aptr = inMat.data;
    int IA = static_cast<int>(inMat.IA);
    int JA = static_cast<int>(inMat.JA);
    int* DESCA = inMat.descA;

    int ipivSize = DESCA[4] + DESCA[8];
    checkAssumption(ipivSize > 0);
    IPIV.resize(ipivSize, 0);

    float* bptr = outMat.data;
    int IB = static_cast<int>(outMat.IA);
    int JB = static_cast<int>(outMat.JA);
    int* DESCB = outMat.descA;

    // --- checking for special conditions ---
    int MB_A, NB_A, MB_B;
    MB_A = DESCA[4];
    NB_A = DESCA[5];
    MB_B = DESCB[4];
    
    int row_offsetA, col_offsetA, row_offsetB;
    row_offsetA = (IA-1) % MB_A;
    col_offsetA = (JA-1) % NB_A;
    row_offsetB = (IB-1) % MB_B;

    if (row_offsetA != 0 || col_offsetA != 0 || col_offsetA != row_offsetB
        || MB_A != NB_A  || NB_A != MB_B) {
      std::string input_set = "\nIA: "   + ITOS(IA) +
                              ", JA: "   + ITOS(JA) +
                              ", MB_A: " + ITOS(MB_A) +
                              ", NB_A: " + ITOS(NB_A) +
                              ", IB: "   + ITOS(IB) +
                              ", MB_B: " + ITOS(MB_B) + "\n";
      input_set += "IROFFA: "   + ITOS(row_offsetA) +
                   ", ICOFFA: " + ITOS(col_offsetA) +
                   ", IROFFB: " + ITOS(row_offsetB) + "\n";

      std::string msg = input_set;
      msg += "Input matrix internal distribution is incompatible for the "; 
      msg += "requested operation!!\nPlease consider different number of ";
      msg += "parallel processes.\nOr you may change the internal distribution ";
      msg += "by specifying suitable blocksize while loading the global input matrix.\n";
      REPORT_ERROR(INTERNAL_ERROR, msg);
    }

    psgesv_(&N, &NRHS,
	    aptr, &IA, &JA, DESCA,
	    &IPIV[0],
	    bptr, &IB, &JB, DESCB,
	    &INFO);
  }
  
  return INFO;
}

template <>
int pgesv2 (sliced_blockcyclic_matrix_local<double>& inMat,
            sliced_blockcyclic_matrix_local<double>& outMat,
            std::vector<int>& IPIV) {

  int INFO = 0;

  if(!is_same_distribution(inMat,outMat))
    REPORT_FATAL(USER_ERROR,"Incompatible distribution of the inputs\n");
  // since inputs are of same distribution, activeness check on one input is ok
  if(is_active(inMat)) {
#ifdef ERROR_CHK
    ASSERT_PTR(inMat.data && outMat.data);

    if(!inMat.is_valid() || !outMat.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

    if(inMat.sliced_num_row != inMat.sliced_num_col)
      REPORT_ERROR(USER_ERROR,"The co-efficient matrix is not a square matrix!!\n");

    if(outMat.sliced_num_row < inMat.sliced_num_row ||
       outMat.sliced_num_col < 1)
      REPORT_ERROR(USER_ERROR,"Invalid right hand side matrix!!\n");
#endif
    
    int N = static_cast<int>(inMat.sliced_num_row); // inMat.sliced_num_col;
    int NRHS = static_cast<int>(outMat.sliced_num_col);

    double* aptr = inMat.data;
    int IA = static_cast<int>(inMat.IA);
    int JA = static_cast<int>(inMat.JA);
    int* DESCA = inMat.descA;

    int ipivSize = DESCA[4] + DESCA[8];
    checkAssumption(ipivSize > 0);
    IPIV.resize(ipivSize, 0);

    double* bptr = outMat.data;
    int IB = static_cast<int>(outMat.IA);
    int JB = static_cast<int>(outMat.JA);
    int* DESCB = outMat.descA;

    // --- checking for special conditions ---
    int MB_A, NB_A, MB_B;
    MB_A = DESCA[4];
    NB_A = DESCA[5];
    MB_B = DESCB[4];

    int row_offsetA, col_offsetA, row_offsetB;
    row_offsetA = (IA-1) % MB_A;
    col_offsetA = (JA-1) % NB_A;
    row_offsetB = (IB-1) % MB_B;

    if (row_offsetA != 0 || col_offsetA != 0 || col_offsetA != row_offsetB
        || MB_A != NB_A  || NB_A != MB_B) {
      std::string input_set = "\nIA: "   + ITOS(IA) +
                              ", JA: "   + ITOS(JA) +
                              ", MB_A: " + ITOS(MB_A) +
                              ", NB_A: " + ITOS(NB_A) +
                              ", IB: "   + ITOS(IB) +
                              ", MB_B: " + ITOS(MB_B) + "\n";
      input_set += "IROFFA: "   + ITOS(row_offsetA) +
                   ", ICOFFA: " + ITOS(col_offsetA) +
                   ", IROFFB: " + ITOS(row_offsetB) + "\n";

      std::string msg = input_set;
      msg += "Input matrix internal distribution is incompatible for the ";         
      msg += "requested operation!!\nPlease consider different number of ";
      msg += "parallel processes.\nOr you may change the internal distribution ";
      msg += "by specifying suitable blocksize while loading the global input matrix.\n";
      REPORT_ERROR(INTERNAL_ERROR, msg);
    }

    pdgesv_(&N, &NRHS,
	    aptr, &IA, &JA, DESCA,
	    &IPIV[0],
	    bptr, &IB, &JB, DESCB,
	    &INFO);
  }

  return INFO;
}

template <>
int pgesv1 (sliced_blockcyclic_matrix_local<float>& inMat,
            sliced_blockcyclic_matrix_local<float>& outMat) {

  std::vector<int> IPIV;
  return pgesv2<float>(inMat, outMat, IPIV);
}

template <>
int pgesv1 (sliced_blockcyclic_matrix_local<double>& inMat,
            sliced_blockcyclic_matrix_local<double>& outMat) {

  std::vector<int> IPIV;
  return pgesv2<double>(inMat, outMat, IPIV);
}

template <>
int pgels (sliced_blockcyclic_matrix_local<float>& inMat,
           sliced_blockcyclic_matrix_local<float>& outMat,
           char TRANS) {

  int INFO = 0;

  if(!is_same_distribution(inMat,outMat))
    REPORT_FATAL(USER_ERROR,"Incompatible distribution of the inputs\n");
  // since inputs are of same distribution, activeness check on one input is ok
  if(is_active(inMat)) {
#ifdef ERROR_CHK
    ASSERT_PTR(inMat.data && outMat.data);

    if(!inMat.is_valid() || !outMat.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

    if((TRANS != 'N') && (TRANS != 'T'))
      REPORT_ERROR(USER_ERROR, "Invalid value for TRANS parameter!!\n");

    size_t maxSize = std::max(inMat.sliced_num_row,inMat.sliced_num_col);
    if(outMat.sliced_num_row < maxSize || outMat.sliced_num_col < 1)
      REPORT_ERROR(USER_ERROR, "Invalid right hand side matrix!!\n");
#endif
    
    int M = static_cast<int>(inMat.sliced_num_row);
    int N = static_cast<int>(inMat.sliced_num_col);
    int NRHS = static_cast<int>(outMat.sliced_num_col);

    float* aptr = inMat.data;
    int IA = static_cast<int>(inMat.IA);
    int JA = static_cast<int>(inMat.JA);
    int* DESCA = inMat.descA;

    float* bptr = outMat.data;
    int IB = static_cast<int>(outMat.IA);
    int JB = static_cast<int>(outMat.JA);
    int* DESCB = outMat.descA;

    // --- checking for special conditions ---
    int MB_A, NB_A, MB_B;
    MB_A = DESCA[4];
    NB_A = DESCA[5];
    MB_B = DESCB[4];

    int row_offsetA, col_offsetA, row_offsetB;
    row_offsetA = (IA-1) % MB_A;
    col_offsetA = (JA-1) % NB_A;
    row_offsetB = (IB-1) % MB_B;

    if ( (M >= N && row_offsetA != row_offsetB) ||
         (M < N  && col_offsetA != row_offsetB) ||
         (M >= N && MB_A != MB_B) ||
         (M < N  && NB_A != MB_B) ) {
      std::string input_set = "\nM: "    + ITOS(M) +
                              ", N: "    + ITOS(N) +
                              ", IA: "   + ITOS(IA) +
                              ", JA: "   + ITOS(JA) +
                              ", MB_A: " + ITOS(MB_A) +
                              ", NB_A: " + ITOS(NB_A) +
                              ", IB: "   + ITOS(IB) +
                              ", MB_B: " + ITOS(MB_B) + "\n";
      input_set += "IROFFA: "   + ITOS(row_offsetA) +
                   ", ICOFFA: " + ITOS(col_offsetA) +
                   ", IROFFB: " + ITOS(row_offsetB) + "\n";

      std::string msg = input_set;
      msg += "Input matrix internal distribution is incompatible for the ";
      msg += "requested operation!!\nPlease consider different number of ";
      msg += "parallel processes.\nOr you may change the internal distribution ";
      msg += "by specifying suitable blocksize while loading the global input matrix.\n";
      REPORT_ERROR(INTERNAL_ERROR, msg);
    }

    int LWORK = -1;
    std::vector<float> WORK(1);

    // LWORK query...
    psgels_(&TRANS,
            &M, &N, &NRHS,
	    aptr, &IA, &JA, DESCA,
	    bptr, &IB, &JB, DESCB,
	    &WORK[0], &LWORK, &INFO);

    LWORK = WORK[0];
    WORK.resize(LWORK, 0);

    // Actual call...
    psgels_(&TRANS,
            &M, &N, &NRHS,
	    aptr, &IA, &JA, DESCA,
	    bptr, &IB, &JB, DESCB,
	    &WORK[0], &LWORK, &INFO);
  }

  return INFO;
}

template <>
int pgels (sliced_blockcyclic_matrix_local<double>& inMat,
           sliced_blockcyclic_matrix_local<double>& outMat,
           char TRANS) {

  int INFO = 0;

  if(!is_same_distribution(inMat,outMat))
    REPORT_FATAL(USER_ERROR,"Incompatible distribution of the inputs\n");
  // since inputs are of same distribution, activeness check on one input is ok
  if(is_active(inMat)) {
#ifdef ERROR_CHK
    ASSERT_PTR(inMat.data && outMat.data);

    if(!inMat.is_valid() || !outMat.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

    if((TRANS != 'N') && (TRANS != 'T'))
      REPORT_ERROR(USER_ERROR, "Invalid value for TRANS parameter!!\n");

    size_t maxSize = std::max(inMat.sliced_num_row,inMat.sliced_num_col);
    if(outMat.sliced_num_row < maxSize || outMat.sliced_num_col < 1)
      REPORT_ERROR(USER_ERROR, "Invalid right hand side matrix!!\n");
#endif
    
    int M = static_cast<int>(inMat.sliced_num_row);
    int N = static_cast<int>(inMat.sliced_num_col);
    int NRHS = static_cast<int>(outMat.sliced_num_col);

    double* aptr = inMat.data;
    int IA = static_cast<int>(inMat.IA);
    int JA = static_cast<int>(inMat.JA);
    int* DESCA = inMat.descA;

    double* bptr = outMat.data;
    int IB = static_cast<int>(outMat.IA);
    int JB = static_cast<int>(outMat.JA);
    int* DESCB = outMat.descA;

    // --- checking for special conditions ---
    int MB_A, NB_A, MB_B;
    MB_A = DESCA[4];
    NB_A = DESCA[5];
    MB_B = DESCB[4];

    int row_offsetA, col_offsetA, row_offsetB;
    row_offsetA = (IA-1) % MB_A;
    col_offsetA = (JA-1) % NB_A;
    row_offsetB = (IB-1) % MB_B;

    if ( (M >= N && row_offsetA != row_offsetB) ||
         (M < N  && col_offsetA != row_offsetB) ||
         (M >= N && MB_A != MB_B) ||
         (M < N  && NB_A != MB_B) ) {
      std::string input_set = "\nM: "    + ITOS(M) +
                              ", N: "    + ITOS(N) +
                              ", IA: "   + ITOS(IA) +
                              ", JA: "   + ITOS(JA) +
                              ", MB_A: " + ITOS(MB_A) +
                              ", NB_A: " + ITOS(NB_A) +
                              ", IB: "   + ITOS(IB) +
                              ", MB_B: " + ITOS(MB_B) + "\n";
      input_set += "IROFFA: "   + ITOS(row_offsetA) +
                   ", ICOFFA: " + ITOS(col_offsetA) +
                   ", IROFFB: " + ITOS(row_offsetB) + "\n";

      std::string msg = input_set;
      msg += "Input matrix internal distribution is incompatible for the ";
      msg += "requested operation!!\nPlease consider different number of ";
      msg += "parallel processes.\nOr you may change the internal distribution ";
      msg += "by specifying suitable blocksize while loading the global input matrix.\n";
      REPORT_ERROR(INTERNAL_ERROR, msg);
    }

    int LWORK = -1;
    std::vector<double> WORK(1);

    // LWORK query...
    pdgels_(&TRANS,
            &M, &N, &NRHS,
	    aptr, &IA, &JA, DESCA,
	    bptr, &IB, &JB, DESCB,
	    &WORK[0], &LWORK, &INFO);

    LWORK = WORK[0];
    WORK.resize(LWORK, 0);

    // Actual call...
    pdgels_(&TRANS,
            &M, &N, &NRHS,
	    aptr, &IA, &JA, DESCA,
	    bptr, &IB, &JB, DESCB,
	    &WORK[0], &LWORK, &INFO);
  }

  return INFO;
}

template <>
int pgesvd1 (sliced_blockcyclic_matrix_local<float>& inMat,
             std::vector<float>& S_VAL) {

  int INFO = 0;

  if(is_active(inMat)) {
#ifdef ERROR_CHK
    ASSERT_PTR(inMat.data);

    if(!inMat.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");
#endif

    char JOBZ = 'N';
    
    int M = static_cast<int>(inMat.sliced_num_row);
    int N = static_cast<int>(inMat.sliced_num_col);

    float* aptr = inMat.data;
    int IA = static_cast<int>(inMat.IA);
    int JA = static_cast<int>(inMat.JA);
    int* DESCA = inMat.descA;

    // checking for special conditions in order to avoid error msg from PSGEBRD
    int MB_A, NB_A;
    MB_A = DESCA[4];
    NB_A = DESCA[5];

    int row_offsetA, col_offsetA;
    row_offsetA = (IA-1) % MB_A; 
    col_offsetA = (JA-1) % NB_A;

    if (row_offsetA != col_offsetA || MB_A != NB_A) {
      std::string input_set = "\nIA: "   + ITOS(IA) +
                              ", JA: "   + ITOS(JA) +
                              ", MB_A: " + ITOS(MB_A) +
                              ", NB_A: " + ITOS(NB_A) + "\n";
      input_set += "IROFFA: "   + ITOS(row_offsetA) +
                   ", ICOFFA: " + ITOS(col_offsetA) + "\n";

      std::string msg = input_set;
      msg += "Input matrix internal distribution is incompatible for the ";
      msg += "requested operation!!\nPlease consider different number of ";
      msg += "parallel processes.\nOr you may change the internal distribution ";
      msg += "by specifying suitable blocksize while loading the global input matrix.\n";
      REPORT_ERROR(INTERNAL_ERROR, msg);
    }

    S_VAL.resize(std::min(M,N), 0);

    float* uptr = NULL;
    int IU = 1, JU = 1;
    int* DESCU = NULL;

    float* vtptr = NULL;
    int IVT = 1, JVT = 1;
    int* DESCVT = NULL;

    int LWORK = -1;
    std::vector<float> WORK(1);

    // LWORK query...
    psgesvd_(&JOBZ, &JOBZ,
             &M, &N,
	     aptr, &IA, &JA, DESCA,
	     &S_VAL[0],
	     uptr, &IU, &JU, DESCU,
	     vtptr, &IVT, &JVT, DESCVT,
	     &WORK[0], &LWORK, &INFO);

    LWORK = WORK[0];
    WORK.resize(LWORK, 0);

    // Actual call...
    psgesvd_(&JOBZ, &JOBZ,
             &M, &N,
	     aptr, &IA, &JA, DESCA,
	     &S_VAL[0],
	     uptr, &IU, &JU, DESCU,
	     vtptr, &IVT, &JVT, DESCVT,
	     &WORK[0], &LWORK, &INFO);
  }

  return INFO;
}

template <>
int pgesvd1 (sliced_blockcyclic_matrix_local<double>& inMat,
             std::vector<double>& S_VAL) {

  int INFO = 0;

  if(is_active(inMat)) {
#ifdef ERROR_CHK
    ASSERT_PTR(inMat.data);

    if(!inMat.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");
#endif

    char JOBZ = 'N';
    
    int M = static_cast<int>(inMat.sliced_num_row);
    int N = static_cast<int>(inMat.sliced_num_col);

    double* aptr = inMat.data;
    int IA = static_cast<int>(inMat.IA);
    int JA = static_cast<int>(inMat.JA);
    int* DESCA = inMat.descA;

    // checking for special conditions in order to avoid error msg from PDGEBRD
    int MB_A, NB_A;
    MB_A = DESCA[4];
    NB_A = DESCA[5];

    int row_offsetA, col_offsetA;
    row_offsetA = (IA-1) % MB_A;
    col_offsetA = (JA-1) % NB_A;

    if (row_offsetA != col_offsetA || MB_A != NB_A) {
      std::string input_set = "\nIA: "   + ITOS(IA) +
                              ", JA: "   + ITOS(JA) +
                              ", MB_A: " + ITOS(MB_A) +
                              ", NB_A: " + ITOS(NB_A) + "\n";
      input_set += "IROFFA: "   + ITOS(row_offsetA) +
                   ", ICOFFA: " + ITOS(col_offsetA) + "\n";

      std::string msg = input_set;
      msg += "Input matrix internal distribution is incompatible for the ";
      msg += "requested operation!!\nPlease consider different number of ";
      msg += "parallel processes.\nOr you may change the internal distribution ";
      msg += "by specifying suitable blocksize while loading the global input matrix.\n";
      REPORT_ERROR(INTERNAL_ERROR, msg);
    }

    S_VAL.resize(std::min(M,N), 0);

    double* uptr = NULL;
    int IU = 1, JU = 1;
    int* DESCU = NULL;

    double* vtptr = NULL;
    int IVT = 1, JVT = 1;
    int* DESCVT = NULL;

    int LWORK = -1;
    std::vector<double> WORK(1);

    // LWORK query...
    pdgesvd_(&JOBZ, &JOBZ,
             &M, &N,
	     aptr, &IA, &JA, DESCA,
	     &S_VAL[0],
	     uptr, &IU, &JU, DESCU,
	     vtptr, &IVT, &JVT, DESCVT,
	     &WORK[0], &LWORK, &INFO);

    LWORK = WORK[0];
    WORK.resize(LWORK, 0);

    // Actual call...
    pdgesvd_(&JOBZ, &JOBZ,
             &M, &N,
	     aptr, &IA, &JA, DESCA,
	     &S_VAL[0],
	     uptr, &IU, &JU, DESCU,
	     vtptr, &IVT, &JVT, DESCVT,
	     &WORK[0], &LWORK, &INFO);
  }

  return INFO;
}

template <>
int pgesvd2 (sliced_blockcyclic_matrix_local<float>& inMat,
             std::vector<float>& S_VAL,
	     sliced_blockcyclic_matrix_local<float>& S_VEC,
	     char VEC_TYPE) {

  int INFO = 0;

  if(!is_same_distribution(inMat,S_VEC))
    REPORT_FATAL(USER_ERROR,"Incompatible distribution of the inputs\n");
  // since inputs are of same distribution, activeness check on one input is ok
  if(is_active(inMat)) {
#ifdef ERROR_CHK
    ASSERT_PTR(inMat.data);

    if(!inMat.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");
#endif

    char JOBU = 'N', JOBVT = 'N';

    int M = static_cast<int>(inMat.sliced_num_row);
    int N = static_cast<int>(inMat.sliced_num_col);
    int MIN_MN = std::min(M,N);

    float* uptr = NULL;
    int IU = 1, JU = 1;
    int* DESCU = NULL;

    float* vtptr = NULL;
    int IVT = 1, JVT = 1;
    int* DESCVT = NULL;

    if(VEC_TYPE == 'L') {
      if(!S_VEC.data)
        REPORT_ERROR(USER_ERROR,
          "Outmat matrix for Left Singular Vectors is not allocated!!\n");

      if(!S_VEC.is_valid())
        REPORT_ERROR(USER_ERROR, 
	  "Invalid output matrix (left singular vector)!!\n");

      if (S_VEC.sliced_num_row < M || S_VEC.sliced_num_col < MIN_MN)
        REPORT_ERROR(USER_ERROR, 
                     "Incompatible size for the 3rd argument matrix!!\n");

      JOBU = 'V';
      uptr = S_VEC.data;
      IU = static_cast<int>(S_VEC.IA);
      JU = static_cast<int>(S_VEC.JA);
      DESCU = S_VEC.descA;
    }

    else if (VEC_TYPE == 'R') {
      if(!S_VEC.data)
        REPORT_ERROR(USER_ERROR,
          "Outmat matrix for Right Singular Vectors is not allocated!!\n");

      if(!S_VEC.is_valid())
        REPORT_ERROR(USER_ERROR,
	  "Invalid output matrix (right singular vector)!!\n");

      if (S_VEC.sliced_num_row < MIN_MN || S_VEC.sliced_num_col < N)
        REPORT_ERROR(USER_ERROR, 
                     "Incompatible size for the 3rd argument matrix!!\n");

      JOBVT = 'V';
      vtptr = S_VEC.data;
      IVT = static_cast<int>(S_VEC.IA);
      JVT = static_cast<int>(S_VEC.JA);
      DESCVT = S_VEC.descA;
    }

    else
      REPORT_ERROR(USER_ERROR,"Invalid vector type is specified!!\n");


    float* aptr = inMat.data;
    int IA = static_cast<int>(inMat.IA);
    int JA = static_cast<int>(inMat.JA);
    int* DESCA = inMat.descA;

    // checking for special conditions to avoid err msg from PSGEBRD & PSORMBR
    int MB_A, NB_A;
    MB_A = DESCA[4];
    NB_A = DESCA[5];

    int row_offsetA, col_offsetA;
    row_offsetA = (IA-1) % MB_A;
    col_offsetA = (JA-1) % NB_A;

    bool WANTU = (VEC_TYPE == 'L') ? true : false;
    int x_offsetC, IJ_C, xB_C;
    if(WANTU) {
      xB_C = DESCU[4];  // MB_U
      IJ_C = IU;
      x_offsetC =  (IJ_C-1) % xB_C; // row_offsetU
    }
    else {    
      xB_C = DESCVT[5]; // NB_VT
      IJ_C = JVT;
      x_offsetC = (IJ_C-1) % xB_C; // col_offsetVT
    }

    if ( row_offsetA != col_offsetA || MB_A != NB_A ||
         (WANTU  && row_offsetA != x_offsetC) ||
         (WANTU  && MB_A != xB_C) ||
         (!WANTU && col_offsetA != x_offsetC) ||
         (!WANTU && NB_A != xB_C) ){
      std::string input_set = "\nIA: "   + ITOS(IA) +
                              ", JA: "   + ITOS(JA) +
                              ", MB_A: " + ITOS(MB_A) +
                              ", NB_A: " + ITOS(NB_A) + "\n";
      input_set += "IROFFA: "   + ITOS(row_offsetA) +
                   ", ICOFFA: " + ITOS(col_offsetA) + "\n";
      input_set += "VEC_TYPE: ";
      input_set += VEC_TYPE ;
      input_set += ", IXOFFC: " + ITOS(x_offsetC) +
                   ", IJ_C: "   + ITOS(IJ_C) +
                   ", XB_C: "   + ITOS(xB_C) + "\n";

      std::string msg = input_set;
      msg += "Input matrix internal distribution is incompatible for the ";
      msg += "requested operation!!\nPlease consider different number of ";
      msg += "parallel processes.\nOr you may change the internal distribution ";
      msg += "by specifying suitable blocksize while loading the global input matrix.\n";
      REPORT_ERROR(INTERNAL_ERROR, msg);
    }

    S_VAL.resize(MIN_MN, 0);

    int LWORK = -1;
    std::vector<float> WORK(1);

    // LWORK query...
    psgesvd_(&JOBU, &JOBVT,
             &M, &N,
	     aptr, &IA, &JA, DESCA,
	     &S_VAL[0],
	     uptr, &IU, &JU, DESCU,
	     vtptr, &IVT, &JVT, DESCVT,
	     &WORK[0], &LWORK, &INFO);

    LWORK = WORK[0];
    WORK.resize(LWORK, 0);

    // Actual call...
    psgesvd_(&JOBU, &JOBVT,
             &M, &N,
	     aptr, &IA, &JA, DESCA,
	     &S_VAL[0],
	     uptr, &IU, &JU, DESCU,
	     vtptr, &IVT, &JVT, DESCVT,
	     &WORK[0], &LWORK, &INFO);
  }

  return INFO;
}

template <>
int pgesvd2 (sliced_blockcyclic_matrix_local<double>& inMat,
             std::vector<double>& S_VAL,
	     sliced_blockcyclic_matrix_local<double>& S_VEC,
	     char VEC_TYPE) {

  int INFO = 0;

  if(!is_same_distribution(inMat,S_VEC))
    REPORT_FATAL(USER_ERROR,"Incompatible distribution of the inputs\n");
  // since inputs are of same distribution, activeness check on one input is ok
  if(is_active(inMat)) {
#ifdef ERROR_CHK
    ASSERT_PTR(inMat.data);

    if(!inMat.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");
#endif

    char JOBU = 'N', JOBVT = 'N';

    int M = static_cast<int>(inMat.sliced_num_row);
    int N = static_cast<int>(inMat.sliced_num_col);
    int MIN_MN = std::min(M,N);

    double* uptr = NULL;
    int IU = 1, JU = 1;
    int* DESCU = NULL;

    double* vtptr = NULL;
    int IVT = 1, JVT = 1;
    int* DESCVT = NULL;

    if(VEC_TYPE == 'L') {
      if(!S_VEC.data)
        REPORT_ERROR(USER_ERROR,
          "Outmat matrix for Left Singular Vectors is not allocated!!\n");

      if(!S_VEC.is_valid())
        REPORT_ERROR(USER_ERROR, 
	  "Invalid output matrix (left singular vector)!!\n");

      if (S_VEC.sliced_num_row < M || S_VEC.sliced_num_col < MIN_MN)
        REPORT_ERROR(USER_ERROR, 
                     "Incompatible size for the 3rd argument matrix!!\n");

      JOBU = 'V';
      uptr = S_VEC.data;
      IU = static_cast<int>(S_VEC.IA);
      JU = static_cast<int>(S_VEC.JA);
      DESCU = S_VEC.descA;
    }

    else if (VEC_TYPE == 'R') {
      if(!S_VEC.data)
        REPORT_ERROR(USER_ERROR,
          "Outmat matrix for Right Singular Vectors is not allocated!!\n");

      if(!S_VEC.is_valid())
        REPORT_ERROR(USER_ERROR,
	  "Invalid output matrix (right singular vector)!!\n");

      if (S_VEC.sliced_num_row < MIN_MN || S_VEC.sliced_num_col < N)
        REPORT_ERROR(USER_ERROR, 
                     "Incompatible size for the 3rd argument matrix!!\n");

      JOBVT = 'V';
      vtptr = S_VEC.data;
      IVT = static_cast<int>(S_VEC.IA);
      JVT = static_cast<int>(S_VEC.JA);
      DESCVT = S_VEC.descA;
    }

    else
      REPORT_ERROR(USER_ERROR,"Invalid vector type is specified!!\n");


    double* aptr = inMat.data;
    int IA = static_cast<int>(inMat.IA);
    int JA = static_cast<int>(inMat.JA);
    int* DESCA = inMat.descA;

    // checking for special conditions to avoid err msg from PDGEBRD & PDORMBR
    int MB_A, NB_A;
    MB_A = DESCA[4];
    NB_A = DESCA[5];

    int row_offsetA, col_offsetA;
    row_offsetA = (IA-1) % MB_A;
    col_offsetA = (JA-1) % NB_A;

    bool WANTU = (VEC_TYPE == 'L') ? true : false;
    int x_offsetC, IJ_C, xB_C;
    if(WANTU) {
      xB_C = DESCU[4];  // MB_U
      IJ_C = IU;
      x_offsetC =  (IJ_C-1) % xB_C; // row_offsetU
    }
    else {
      xB_C = DESCVT[5]; // NB_VT
      IJ_C = JVT;
      x_offsetC = (IJ_C-1) % xB_C; // col_offsetVT
    }

    if ( row_offsetA != col_offsetA || MB_A != NB_A ||
         (WANTU  && row_offsetA != x_offsetC) ||
         (WANTU  && MB_A != xB_C) ||
         (!WANTU && col_offsetA != x_offsetC) ||
         (!WANTU && NB_A != xB_C) ){
      std::string input_set = "\nIA: "   + ITOS(IA) +
                              ", JA: "   + ITOS(JA) +
                              ", MB_A: " + ITOS(MB_A) +
                              ", NB_A: " + ITOS(NB_A) + "\n";
      input_set += "IROFFA: "   + ITOS(row_offsetA) +
                   ", ICOFFA: " + ITOS(col_offsetA) + "\n";
      input_set += "VEC_TYPE: ";
      input_set += VEC_TYPE ;
      input_set += ", IXOFFC: " + ITOS(x_offsetC) +
                   ", IJ_C: "   + ITOS(IJ_C) +
                   ", XB_C: "   + ITOS(xB_C) + "\n";

      std::string msg = input_set;
      msg += "Input matrix internal distribution is incompatible for the ";
      msg += "requested operation!!\nPlease consider different number of ";
      msg += "parallel processes.\nOr you may change the internal distribution ";
      msg += "by specifying suitable blocksize while loading the global input matrix.\n";
      REPORT_ERROR(INTERNAL_ERROR, msg);
    }

    S_VAL.resize(MIN_MN, 0);

    int LWORK = -1;
    std::vector<double> WORK(1);

    // LWORK query...
    pdgesvd_(&JOBU, &JOBVT,
             &M, &N,
	     aptr, &IA, &JA, DESCA,
	     &S_VAL[0],
	     uptr, &IU, &JU, DESCU,
	     vtptr, &IVT, &JVT, DESCVT,
	     &WORK[0], &LWORK, &INFO);

    LWORK = WORK[0];
    WORK.resize(LWORK, 0);

    // Actual call...
    pdgesvd_(&JOBU, &JOBVT,
             &M, &N,
	     aptr, &IA, &JA, DESCA,
	     &S_VAL[0],
	     uptr, &IU, &JU, DESCU,
	     vtptr, &IVT, &JVT, DESCVT,
	     &WORK[0], &LWORK, &INFO);
  }

  return INFO;
}

template <>
int pgesvd3 (sliced_blockcyclic_matrix_local<float>& inMat,
             std::vector<float>& S_VAL,
	     sliced_blockcyclic_matrix_local<float>& L_SVEC,
	     sliced_blockcyclic_matrix_local<float>& R_SVEC) {

  int INFO = 0;

  if(!is_same_distribution(inMat,L_SVEC,R_SVEC))
    REPORT_FATAL(USER_ERROR,"Incompatible distribution of the inputs\n");
  // since inputs are of same distribution, activeness check on one input is ok
  if(is_active(inMat)) {
#ifdef ERROR_CHK
    ASSERT_PTR(inMat.data);

    if(!inMat.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

    if(!L_SVEC.data || !R_SVEC.data)
      REPORT_ERROR(USER_ERROR,
            "Output matrix for singular vectors is not allocated!!\n");

    if(!L_SVEC.is_valid() || !R_SVEC.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid output matrix for singular vectors!!\n");

    int MIN_MN = std::min(inMat.sliced_num_row,inMat.sliced_num_col);

    if (L_SVEC.sliced_num_row < inMat.sliced_num_row || 
        L_SVEC.sliced_num_col < MIN_MN)
      REPORT_ERROR(USER_ERROR, 
                   "Incompatible size for the 3rd argument matrix!!\n");

    if (R_SVEC.sliced_num_row < MIN_MN || 
        R_SVEC.sliced_num_col < inMat.sliced_num_col)
      REPORT_ERROR(USER_ERROR, 
                   "Incompatible size for the 4th argument matrix!!\n");
#endif

    char JOBU = 'V', JOBVT = 'V';
    int M = static_cast<int>(inMat.sliced_num_row);
    int N = static_cast<int>(inMat.sliced_num_col);

    float* aptr = inMat.data;
    int IA = static_cast<int>(inMat.IA);
    int JA = static_cast<int>(inMat.JA);
    int* DESCA = inMat.descA;

    S_VAL.resize(std::min(M,N), 0);

    float* uptr = L_SVEC.data;
    int IU = static_cast<int>(L_SVEC.IA);
    int JU = static_cast<int>(L_SVEC.JA);
    int* DESCU = L_SVEC.descA;

    float* vtptr = R_SVEC.data;
    int IVT = static_cast<int>(R_SVEC.IA);
    int JVT = static_cast<int>(R_SVEC.JA);
    int* DESCVT = R_SVEC.descA;

    // checking for special conditions to avoid err msg from PSGEBRD & PSORMBR
    int MB_A, NB_A;
    MB_A = DESCA[4];
    NB_A = DESCA[5];

    int row_offsetA, col_offsetA;
    row_offsetA = (IA-1) % MB_A;
    col_offsetA = (JA-1) % NB_A;

    int row_offsetU, MB_U;
    MB_U = DESCU[4];
    row_offsetU = (IU-1) % MB_U;

    int col_offsetVT, NB_VT;
    NB_VT = DESCVT[5];
    col_offsetVT = (JVT-1) % NB_VT;

    if ( row_offsetA != col_offsetA  || MB_A != NB_A ||
         row_offsetA != row_offsetU  || MB_A != MB_U ||
         col_offsetA != col_offsetVT || NB_A != NB_VT ) {
      std::string input_set = "\nIA: "   + ITOS(IA) +
                              ", JA: "   + ITOS(JA) +
                              ", MB_A: " + ITOS(MB_A) +
                              ", NB_A: " + ITOS(NB_A) + "\n";
      input_set += "IROFFA: "   + ITOS(row_offsetA) +
                   ", ICOFFA: " + ITOS(col_offsetA) + "\n";
      input_set += "IROFFU: " + ITOS(row_offsetU) +
                   ", IA_U: " + ITOS(IU) +
                   ", MB_U: " + ITOS(MB_U)  + "\n";
      input_set += "ICOFFVT: " + ITOS(col_offsetVT) +
                   ", JA_VT: " + ITOS(JVT) +
                   ", NB_VT: " + ITOS(NB_VT)  + "\n";

      std::string msg = input_set;
      msg += "Input matrix internal distribution is incompatible for the ";
      msg += "requested operation!!\nPlease consider different number of ";
      msg += "parallel processes.\nOr you may change the internal distribution ";
      msg += "by specifying suitable blocksize while loading the global input matrix.\n";
      REPORT_ERROR(INTERNAL_ERROR, msg);
    }

    int LWORK = -1;
    std::vector<float> WORK(1);

    // LWORK query...
    psgesvd_(&JOBU, &JOBVT,
             &M, &N,
	     aptr, &IA, &JA, DESCA,
	     &S_VAL[0],
	     uptr, &IU, &JU, DESCU,
	     vtptr, &IVT, &JVT, DESCVT,
	     &WORK[0], &LWORK, &INFO);

    LWORK = WORK[0];
    WORK.resize(LWORK, 0);

    // Actual call...
    psgesvd_(&JOBU, &JOBVT,
             &M, &N,
	     aptr, &IA, &JA, DESCA,
	     &S_VAL[0],
	     uptr, &IU, &JU, DESCU,
	     vtptr, &IVT, &JVT, DESCVT,
	     &WORK[0], &LWORK, &INFO);

  }

  return INFO;
}

template <>
int pgesvd3 (sliced_blockcyclic_matrix_local<double>& inMat,
             std::vector<double>& S_VAL,
	     sliced_blockcyclic_matrix_local<double>& L_SVEC,
	     sliced_blockcyclic_matrix_local<double>& R_SVEC) {

  int INFO = 0;

  if(!is_same_distribution(inMat,L_SVEC,R_SVEC))
    REPORT_FATAL(USER_ERROR,"Incompatible distribution of the inputs\n");
  // since inputs are of same distribution, activeness check on one input is ok
  if(is_active(inMat)) {
#ifdef ERROR_CHK
    ASSERT_PTR(inMat.data);

    if(!inMat.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

    if(!L_SVEC.data || !R_SVEC.data)
      REPORT_ERROR(USER_ERROR,
            "Output matrix for singular vectors is not allocated!!\n");

    if(!L_SVEC.is_valid() || !R_SVEC.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid output matrix for singular vectors!!\n");

    int MIN_MN = std::min(inMat.sliced_num_row,inMat.sliced_num_col);

    if (L_SVEC.sliced_num_row < inMat.sliced_num_row ||
        L_SVEC.sliced_num_col < MIN_MN)
      REPORT_ERROR(USER_ERROR,
                   "Incompatible size for the 3rd argument matrix!!\n");

    if (R_SVEC.sliced_num_row < MIN_MN ||
        R_SVEC.sliced_num_col < inMat.sliced_num_col)
      REPORT_ERROR(USER_ERROR,
                   "Incompatible size for the 4th argument matrix!!\n");
#endif

    char JOBU = 'V', JOBVT = 'V';
    int M = static_cast<int>(inMat.sliced_num_row);
    int N = static_cast<int>(inMat.sliced_num_col);

    double* aptr = inMat.data;
    int IA = static_cast<int>(inMat.IA);
    int JA = static_cast<int>(inMat.JA);
    int* DESCA = inMat.descA;

    S_VAL.resize(std::min(M,N), 0);

    double* uptr = L_SVEC.data;
    int IU = static_cast<int>(L_SVEC.IA);
    int JU = static_cast<int>(L_SVEC.JA);
    int* DESCU = L_SVEC.descA;

    double* vtptr = R_SVEC.data;
    int IVT = static_cast<int>(R_SVEC.IA);
    int JVT = static_cast<int>(R_SVEC.JA);
    int* DESCVT = R_SVEC.descA;

    // checking for special conditions to avoid err msg from PDGEBRD & PDORMBR
    int MB_A, NB_A;
    MB_A = DESCA[4];
    NB_A = DESCA[5];

    int row_offsetA, col_offsetA;
    row_offsetA = (IA-1) % MB_A;
    col_offsetA = (JA-1) % NB_A;

    int row_offsetU, MB_U;
    MB_U = DESCU[4];
    row_offsetU = (IU-1) % MB_U;

    int col_offsetVT, NB_VT;
    NB_VT = DESCVT[5];
    col_offsetVT = (JVT-1) % NB_VT;

    if ( row_offsetA != col_offsetA  || MB_A != NB_A ||
         row_offsetA != row_offsetU  || MB_A != MB_U ||
         col_offsetA != col_offsetVT || NB_A != NB_VT ) {
      std::string input_set = "\nIA: "   + ITOS(IA) +
                              ", JA: "   + ITOS(JA) +
                              ", MB_A: " + ITOS(MB_A) +
                              ", NB_A: " + ITOS(NB_A) + "\n";
      input_set += "IROFFA: "   + ITOS(row_offsetA) +
                   ", ICOFFA: " + ITOS(col_offsetA) + "\n";
      input_set += "IROFFU: " + ITOS(row_offsetU) +
                   ", IA_U: " + ITOS(IU) +
                   ", MB_U: " + ITOS(MB_U)  + "\n";
      input_set += "ICOFFVT: " + ITOS(col_offsetVT) +
                   ", JA_VT: " + ITOS(JVT) +
                   ", NB_VT: " + ITOS(NB_VT)  + "\n";

      std::string msg = input_set;
      msg += "Input matrix internal distribution is incompatible for the ";
      msg += "requested operation!!\nPlease consider different number of ";
      msg += "parallel processes.\nOr you may change the internal distribution ";
      msg += "by specifying suitable blocksize while loading the global input matrix.\n";
      REPORT_ERROR(INTERNAL_ERROR, msg);
    }

    int LWORK = -1;
    std::vector<double> WORK(1);

    // LWORK query...
    pdgesvd_(&JOBU, &JOBVT,
             &M, &N,
	     aptr, &IA, &JA, DESCA,
	     &S_VAL[0],
	     uptr, &IU, &JU, DESCU,
	     vtptr, &IVT, &JVT, DESCVT,
	     &WORK[0], &LWORK, &INFO);

    LWORK = WORK[0];
    WORK.resize(LWORK, 0);

    // Actual call...
    pdgesvd_(&JOBU, &JOBVT,
             &M, &N,
	     aptr, &IA, &JA, DESCA,
	     &S_VAL[0],
	     uptr, &IU, &JU, DESCU,
	     vtptr, &IVT, &JVT, DESCVT,
	     &WORK[0], &LWORK, &INFO);
  }

  return INFO;
}

template <>
void placpy(sliced_blockcyclic_matrix_local<float>& inMat,
            sliced_blockcyclic_matrix_local<float>& outMat,
            char UPLO) {

  if(!is_same_distribution(inMat,outMat))
    REPORT_FATAL(USER_ERROR,"Incompatible distribution of the inputs\n");
  // since inputs are of same distribution, activeness check on one input is ok
  if(is_active(inMat)) {
#ifdef ERROR_CHK
    ASSERT_PTR(inMat.data);

    if(!inMat.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

    if(!outMat.data)
      REPORT_ERROR(USER_ERROR,"Output matrix is not allocated!!\n");

    if(!outMat.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid output matrix!!\n");

    if(inMat.sliced_num_row != outMat.sliced_num_row ||
       inMat.sliced_num_col != outMat.sliced_num_col)
      REPORT_ERROR(USER_ERROR,"Incompatible sizes of the input matrices!!\n");

    if(UPLO != 'U' && UPLO != 'L' && UPLO != 'A')
      REPORT_ERROR(USER_ERROR,"Illegal value provided for 3rd argument!!\n");
#endif
   
    int M = static_cast<int>(inMat.sliced_num_row);
    int N = static_cast<int>(inMat.sliced_num_col);

    float* aptr = inMat.data;
    int IA = static_cast<int>(inMat.IA);
    int JA = static_cast<int>(inMat.JA);
    int* DESCA = inMat.descA;

    float* bptr = outMat.data;
    int IB = static_cast<int>(outMat.IA);
    int JB = static_cast<int>(outMat.JA);
    int* DESCB = outMat.descA;

    pslacpy_(&UPLO,
             &M, &N,
             aptr, &IA, &JA, DESCA,
             bptr, &IB, &JB, DESCB);
  }
}

template <>
void placpy(sliced_blockcyclic_matrix_local<double>& inMat,
            sliced_blockcyclic_matrix_local<double>& outMat,
            char UPLO) {

  if(!is_same_distribution(inMat,outMat))
    REPORT_FATAL(USER_ERROR,"Incompatible distribution of the inputs\n");
  // since inputs are of same distribution, activeness check on one input is ok
  if(is_active(inMat)) {
#ifdef ERROR_CHK
    ASSERT_PTR(inMat.data);

    if(!inMat.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

    if(!outMat.data)
      REPORT_ERROR(USER_ERROR,"Output matrix is not allocated!!\n");

    if(!outMat.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid output matrix!!\n");

    if(inMat.sliced_num_row != outMat.sliced_num_row ||
       inMat.sliced_num_col != outMat.sliced_num_col)
      REPORT_ERROR(USER_ERROR,"Incompatible sizes of the input matrices!!\n");

    if(UPLO != 'U' && UPLO != 'L' && UPLO != 'A')
      REPORT_ERROR(USER_ERROR,"Illegal value provided for 3rd argument!!\n");
#endif
   
    int M = static_cast<int>(inMat.sliced_num_row);
    int N = static_cast<int>(inMat.sliced_num_col);

    double* aptr = inMat.data;
    int IA = static_cast<int>(inMat.IA);
    int JA = static_cast<int>(inMat.JA);
    int* DESCA = inMat.descA;

    double* bptr = outMat.data;
    int IB = static_cast<int>(outMat.IA);
    int JB = static_cast<int>(outMat.JA);
    int* DESCB = outMat.descA;

    pdlacpy_(&UPLO,
             &M, &N,
             aptr, &IA, &JA, DESCA,
             bptr, &IB, &JB, DESCB);
  }
}

}
