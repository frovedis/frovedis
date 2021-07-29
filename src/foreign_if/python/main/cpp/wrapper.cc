#include "python_client_headers.hpp"
#include "exrpc_svd.hpp"
#include "exrpc_pblas.hpp"
#include "exrpc_scalapack.hpp"
#include "short_hand_dense_type.hpp"
#include "short_hand_sparse_type.hpp"
#include "exrpc_pca.hpp"
#include "exrpc_tsne.hpp"
#include "exrpc_data_storage.hpp"
#include "exrpc_eigen.hpp"
extern "C" {

  // --- Frovedis PBLAS L1 Wrapper ---
  void pswap(const char* host, int port, long vptr1, 
             long vptr2, short dtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto v1 = (exrpc_ptr_t) vptr1;
    auto v2 = (exrpc_ptr_t) vptr2;
    try {
      switch(dtype) {
        case FLOAT:  
                     exrpc_oneway(fm_node,(frovedis_swap<DT2,B_MAT2>),v1,v2);
                     break;
        case DOUBLE: 
                     exrpc_oneway(fm_node,(frovedis_swap<DT1,B_MAT1>),v1,v2);
                     break;
        default: REPORT_ERROR(USER_ERROR, "swap: Only supports float/double typed input!");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  void pcopy(const char* host, int port, long vptr1, 
             long vptr2, short dtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto v1 = (exrpc_ptr_t) vptr1;
    auto v2 = (exrpc_ptr_t) vptr2;
    try {
      switch(dtype) {
        case FLOAT:  
                     exrpc_oneway(fm_node,(frovedis_copy<DT2,B_MAT2>),v1,v2);
                     break;
        case DOUBLE: 
                     exrpc_oneway(fm_node,(frovedis_copy<DT1,B_MAT1>),v1,v2);
                     break;
        default: REPORT_ERROR(USER_ERROR, "copy: Only supports float/double typed input!");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  void pscal(const char* host, int port, long vptr, 
             double al, short dtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto v = (exrpc_ptr_t) vptr;
    try {
      switch(dtype) {
        case FLOAT: { 
                      auto al_ = static_cast<float>(al);
                      exrpc_oneway(fm_node,(frovedis_scal<DT2,B_MAT2,B_LMAT2>),v,al_);
                      break;
                    }
        case DOUBLE: 
                     exrpc_oneway(fm_node,(frovedis_scal<DT1,B_MAT1,B_LMAT1>),v,al);
                     break;
        default: REPORT_ERROR(USER_ERROR, "scal: Only supports float/double typed input!");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  void paxpy(const char* host, int port, long vptr1, 
             long vptr2, double al, short dtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto v1 = (exrpc_ptr_t) vptr1;
    auto v2 = (exrpc_ptr_t) vptr2;
    try {
      switch(dtype) {
        case FLOAT: {
                      auto al_ = static_cast<float>(al);
                      exrpc_oneway(fm_node,(frovedis_axpy<DT2,B_MAT2>),v1,v2,al_);
                      break;
                    }
        case DOUBLE: 
                     exrpc_oneway(fm_node,(frovedis_axpy<DT1,B_MAT1>),v1,v2,al);
                     break;
        default: REPORT_ERROR(USER_ERROR, "axpy: Only supports float/double typed input!");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  double pdot(const char* host, int port, long vptr1, 
              long vptr2, short dtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto v1 = (exrpc_ptr_t) vptr1;
    auto v2 = (exrpc_ptr_t) vptr2;
    double ret = 0;
    try {
      switch(dtype) {
        case FLOAT:  
                     ret = exrpc_async(fm_node,(frovedis_dot<DT2,B_MAT2>),v1,v2).get();
                     break;
        case DOUBLE: 
                     ret = exrpc_async(fm_node,(frovedis_dot<DT1,B_MAT1>),v1,v2).get();
                     break;
        default: REPORT_ERROR(USER_ERROR, "dot: Only supports float/double typed input!");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return ret;
  }

  double pnrm2(const char* host, int port, long vptr, short dtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto v = (exrpc_ptr_t) vptr;
    double ret = 0;
    try {
      switch(dtype) {
        case FLOAT:  
                     ret = exrpc_async(fm_node,(frovedis_nrm2<DT2,B_MAT2>),v).get();
                     break;
        case DOUBLE: 
                     ret = exrpc_async(fm_node,(frovedis_nrm2<DT1,B_MAT1>),v).get();
                     break;
        default: REPORT_ERROR(USER_ERROR, "nrm2: Only supports float/double typed input!");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return ret;
  }

  // --- Frovedis PBLAS L2 Wrapper ---
  PyObject* pgemv(const char* host, int port, 
                  long mptr, long vptr, 
                  bool trans, double al, double be, short dtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto m = (exrpc_ptr_t) mptr;
    auto v = (exrpc_ptr_t) vptr;
    dummy_matrix ret;
    try {
      switch(dtype) {
        case FLOAT: { 
                      auto al_ = static_cast<float>(al);
                      auto be_ = static_cast<float>(be);
                      ret = exrpc_async(fm_node,(frovedis_gemv<DT2,B_MAT2>),m,v,trans,al_,be_).get();
                      break;
                    }
        case DOUBLE: 
                     ret = exrpc_async(fm_node,(frovedis_gemv<DT1,B_MAT1>),m,v,trans,al,be).get();
                     break;
        default: REPORT_ERROR(USER_ERROR, "gemv: Only supports float/double typed input!");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_matrix(ret);
  }

  PyObject* pger(const char* host, int port, 
                 long vptr1, long vptr2, double al, short dtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto v1 = (exrpc_ptr_t) vptr1;
    auto v2 = (exrpc_ptr_t) vptr2;
    dummy_matrix ret;
    try {
      switch(dtype) {
        case FLOAT: { 
                      auto al_ = static_cast<float>(al);
                      ret = exrpc_async(fm_node,(frovedis_ger<DT2,B_MAT2>),v1,v2,al_).get();
                      break;
                    }
        case DOUBLE: 
                     ret = exrpc_async(fm_node,(frovedis_ger<DT1,B_MAT1>),v1,v2,al).get();
                     break;
        default: REPORT_ERROR(USER_ERROR, "ger: Only supports float/double typed input!");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_matrix(ret);
  }

  // --- Frovedis PBLAS L3 Wrapper ---
  PyObject* pgemm(const char* host, int port, 
                  long mptr1, long mptr2, 
                  bool trans1, bool trans2,
                  double al, double be, short dtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto m1 = (exrpc_ptr_t) mptr1;
    auto m2 = (exrpc_ptr_t) mptr2;
    dummy_matrix ret;
    try {
      switch(dtype) {
        case FLOAT: { 
                      auto al_ = static_cast<float>(al);
                      auto be_ = static_cast<float>(be);
                      ret = exrpc_async(fm_node,(frovedis_gemm<DT2,B_MAT2>),m1,m2,trans1,trans2,al_,be_).get();
                      break;
                    }
        case DOUBLE: 
                     ret = exrpc_async(fm_node,(frovedis_gemm<DT1,B_MAT1>),m1,m2,trans1,trans2,al,be).get();
                     break;
        default: REPORT_ERROR(USER_ERROR, "gemm: Only supports float/double typed input!");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_matrix(ret);
  }

  void pgeadd(const char* host, int port, 
              long mptr1, long mptr2, 
              bool trans, double al, double be, short dtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto m1 = (exrpc_ptr_t) mptr1;
    auto m2 = (exrpc_ptr_t) mptr2;
    try {
      switch(dtype) {
        case FLOAT: { 
                      auto al_ = static_cast<float>(al);
                      auto be_ = static_cast<float>(be);
                      exrpc_oneway(fm_node,(frovedis_geadd<DT2,B_MAT2>),m1,m2,trans,al_,be_);
                      break;
                    }
        case DOUBLE: 
                     exrpc_oneway(fm_node,(frovedis_geadd<DT1,B_MAT1>),m1,m2,trans,al,be);
                     break;
        default: REPORT_ERROR(USER_ERROR, "geadd: Only supports float/double typed input!");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  // --- Frovedis SCALAPACK Wrapper ---
  PyObject* pgetrf(const char* host, int port, long mptr, short dtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto m = (exrpc_ptr_t) mptr;
    lu_fact_result ret;
    try {
      switch(dtype) {
        case FLOAT:  
                     ret = exrpc_async(fm_node,(frovedis_getrf<DT2,B_MAT2,lvec<int>>),m).get();
                     break;
        case DOUBLE: 
                     ret = exrpc_async(fm_node,(frovedis_getrf<DT1,B_MAT1,lvec<int>>),m).get();
                     break;
        default: REPORT_ERROR(USER_ERROR, "getrf: Only supports float/double typed input!");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_lu_fact_result(ret,'B');
  }

  int pgetri(const char* host, int port, 
             long mptr, long ipiv_ptr, short dtype) { 
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto m = (exrpc_ptr_t) mptr;
    auto ipiv = (exrpc_ptr_t) ipiv_ptr;
    int ret = 0;
    try {
      switch(dtype) {
        case FLOAT:  
                     ret = exrpc_async(fm_node,(frovedis_getri<DT2,B_MAT2,lvec<int>>),m,ipiv).get();
                     break;
        case DOUBLE: 
                     ret = exrpc_async(fm_node,(frovedis_getri<DT1,B_MAT1,lvec<int>>),m,ipiv).get();
                     break;
        default: REPORT_ERROR(USER_ERROR, "getri: Only supports float/double typed input!");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return ret;
  }

  int pgetrs(const char* host, int port, 
             long mptrA, long mptrB, 
             long ipiv_ptr, bool trans, short dtype) { 
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto mA = (exrpc_ptr_t) mptrA;
    auto mB = (exrpc_ptr_t) mptrB;
    auto ipiv = (exrpc_ptr_t) ipiv_ptr;
    int ret = 0;
    try {
      switch(dtype) {
        case FLOAT:  
                     ret = exrpc_async(fm_node,(frovedis_getrs<DT2,B_MAT2,lvec<int>>),mA,mB,ipiv,trans).get();
                     break;
        case DOUBLE: 
                     ret = exrpc_async(fm_node,(frovedis_getrs<DT1,B_MAT1,lvec<int>>),mA,mB,ipiv,trans).get();
                     break;
        default: REPORT_ERROR(USER_ERROR, "getrs: Only supports float/double typed input!");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return ret;
  }

  int pgesv(const char* host, int port, 
            long mptrA, long mptrB, short dtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto mA = (exrpc_ptr_t) mptrA;
    auto mB = (exrpc_ptr_t) mptrB;
    int ret = 0;
    try {
      switch(dtype) {
        case FLOAT:  
                     ret = exrpc_async(fm_node,(frovedis_gesv<DT2,B_MAT2,lvec<int>>),mA,mB).get();
                     break;
        case DOUBLE: 
                     ret = exrpc_async(fm_node,(frovedis_gesv<DT1,B_MAT1,lvec<int>>),mA,mB).get();
                     break;
        default: REPORT_ERROR(USER_ERROR, "gesv: Only supports float/double typed input!");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return ret;
  }

  int pgels(const char* host, int port, 
            long mptrA, long mptrB, bool trans, short dtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto mA = (exrpc_ptr_t) mptrA;
    auto mB = (exrpc_ptr_t) mptrB;
    int ret = 0;
    try {
      switch(dtype) {
        case FLOAT:  
                     ret = exrpc_async(fm_node,(frovedis_gels<DT2,B_MAT2>),mA,mB,trans).get();
                     break;
        case DOUBLE: 
                     ret = exrpc_async(fm_node,(frovedis_gels<DT1,B_MAT1>),mA,mB,trans).get();
                     break;
        default: REPORT_ERROR(USER_ERROR, "gels: Only supports float/double typed input!");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return ret;
  }

  PyObject* pgesvd(const char* host, int port, 
                   long mptr, bool isU, bool isV, short dtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto m = (exrpc_ptr_t) mptr;
    svd_result ret;
    try {
      switch(dtype) {
        case FLOAT:  
                     ret = exrpc_async(fm_node,(frovedis_gesvd<DT2,B_MAT2>),m,isU,isV).get();
                     break;
        case DOUBLE: 
                     ret = exrpc_async(fm_node,(frovedis_gesvd<DT1,B_MAT1>),m,isU,isV).get();
                     break;
        default: REPORT_ERROR(USER_ERROR, "gesvd: Only supports float/double typed input!");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_svd_result(ret,'B',isU,isV);
  }

  // computes sum of variance in col direction of input matrix
  double sum_of_variance(const char* host, int port, 
                         long mptr, bool sample_variance, 
                         bool isDense, short dtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto m = (exrpc_ptr_t) mptr;
    double sum = 0;
    try {
      if(!isDense) REPORT_ERROR(USER_ERROR, 
        "Frovedis doesn't support variance computation on sparse data\n");
      switch(dtype) {
        case FLOAT: sum = exrpc_async(fm_node, (compute_var_sum<DT2,R_MAT2>), m, sample_variance).get();
                    break;
        case DOUBLE: sum = exrpc_async(fm_node, (compute_var_sum<DT1,R_MAT1>), m, sample_variance).get();
                     break;
        default: REPORT_ERROR(USER_ERROR, "compute_var_sum: Supported dtypes are float/double only!");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return sum;
  }

  PyObject* compute_truncated_svd(const char* host, int port, 
                                  long mptr, int k,
                                  short dtype, short itype,
                                  bool isDense, bool shrink) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto m = (exrpc_ptr_t) mptr;
    bool wantU = true;
    bool wantV = true;
    bool mvbl = false;
    char mtype = 'C';
    svd_result ret;
    try {
      if(isDense) { 
        switch(dtype) {
          case FLOAT: 
            ret = exrpc_async(fm_node,(frovedis_dense_truncated_svd<R_MAT2,DT2>),
                              m,k,mvbl,true).get(); 
            break;
          case DOUBLE: 
            ret = exrpc_async(fm_node,(frovedis_dense_truncated_svd<R_MAT1,DT1>),
                              m,k,mvbl,true).get(); 
            break;
          default: REPORT_ERROR(USER_ERROR,
                   "Unsupported dtype for input dense matrix!\n");
        }
      }
      else {
        switch(dtype) {
          case FLOAT: 
            if (itype == INT) 
              ret = exrpc_async(fm_node,(frovedis_sparse_truncated_svd<S_MAT24,DT2,DT4>),
                                m,k,shrink,mvbl,true).get();
            else if (itype == LONG) 
              ret = exrpc_async(fm_node,(frovedis_sparse_truncated_svd<S_MAT25,DT2,DT5>),
                                m,k,shrink,mvbl,true).get();
            else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype for input sparse matrix!\n");
            break;
          case DOUBLE: 
            if (itype == INT) 
              ret = exrpc_async(fm_node,(frovedis_sparse_truncated_svd<S_MAT14,DT1,DT4>),
                                m,k,shrink,mvbl,true).get();
            else if (itype == LONG) 
              ret = exrpc_async(fm_node,(frovedis_sparse_truncated_svd<S_MAT15,DT1,DT5>),
                                m,k,shrink,mvbl,true).get();
            else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype for input sparse matrix!\n");
            break;
          default: REPORT_ERROR(USER_ERROR,
                   "Unsupported dtype for input sparse matrix!\n");
        }
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_svd_result(ret,mtype,wantU,wantV);
  }

  PyObject* compute_svd_self_transform(const char* host, int port,
                                       long uptr, long sptr, short dtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto u = (exrpc_ptr_t) uptr;
    auto s = (exrpc_ptr_t) sptr;
    dummy_matrix ret;
    try {
      switch(dtype) {
        case FLOAT:  ret = exrpc_async(fm_node, frovedis_svd_self_transform<DT2>, u, s).get(); break;
        case DOUBLE: ret = exrpc_async(fm_node, frovedis_svd_self_transform<DT1>, u, s).get(); break;
        default:     REPORT_ERROR(USER_ERROR, "Unsupported dtype for input dense matrix!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_matrix(ret);
  }

  PyObject* compute_svd_transform(const char* host, int port,
                                  long mptr, short dtype, short itype,
                                  bool isDense, long vptr) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto m = (exrpc_ptr_t) mptr;
    auto v = (exrpc_ptr_t) vptr;
    dummy_matrix ret;
    try {
      if(isDense) {
        switch(dtype) {
          case FLOAT:
            ret = exrpc_async(fm_node,(frovedis_svd_transform<R_MAT2,DT2>),m,v).get();
            break;
          case DOUBLE:
            ret = exrpc_async(fm_node,(frovedis_svd_transform<R_MAT1,DT1>),m,v).get();
            break;
          default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input dense matrix!\n");
        }
      }
      else {
        switch(dtype) {
          case FLOAT:
            if (itype == INT)
              ret = exrpc_async(fm_node,(frovedis_svd_transform<S_MAT24,DT2>),m,v).get();
            else if (itype == LONG)
              ret = exrpc_async(fm_node,(frovedis_svd_transform<S_MAT25,DT2>),m,v).get();
            else REPORT_ERROR(USER_ERROR,"Unsupported itype for input sparse matrix!\n");
            break;
          case DOUBLE:
            if (itype == INT)
              ret = exrpc_async(fm_node,(frovedis_svd_transform<S_MAT14,DT1>),m,v).get();
            else if (itype == LONG)
              ret = exrpc_async(fm_node,(frovedis_svd_transform<S_MAT15,DT1>),m,v).get();
            else REPORT_ERROR(USER_ERROR,"Unsupported itype for input sparse matrix!\n");
            break;
          default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input sparse matrix!\n");
        }
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_matrix(ret); 
  }

  PyObject* compute_svd_inverse_transform(const char* host, int port,
                                          long mptr, short dtype, short itype,
                                          bool isDense, long vptr) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto m = (exrpc_ptr_t) mptr;
    auto v = (exrpc_ptr_t) vptr;
    dummy_matrix ret;
    try {
      if(isDense) {
        switch(dtype) {
          case FLOAT:
            ret = exrpc_async(fm_node,(frovedis_svd_inv_transform<R_MAT2,DT2>),m,v).get();
            break;
          case DOUBLE:
            ret = exrpc_async(fm_node,(frovedis_svd_inv_transform<R_MAT1,DT1>),m,v).get();
            break;
          default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input dense matrix!\n");
        }
      }
      else {
        switch(dtype) {
          case FLOAT:
            if (itype == INT)
              ret = exrpc_async(fm_node,(frovedis_svd_inv_transform<S_MAT24,DT2>),m,v).get();
            else if (itype == LONG)
              ret = exrpc_async(fm_node,(frovedis_svd_inv_transform<S_MAT25,DT2>),m,v).get();
            else REPORT_ERROR(USER_ERROR,"Unsupported itype for input sparse matrix!\n");
            break;
          case DOUBLE:
            if (itype == INT)
              ret = exrpc_async(fm_node,(frovedis_svd_inv_transform<S_MAT14,DT1>),m,v).get();
            else if (itype == LONG)
              ret = exrpc_async(fm_node,(frovedis_svd_inv_transform<S_MAT15,DT1>),m,v).get();
            else REPORT_ERROR(USER_ERROR,"Unsupported itype for input sparse matrix!\n");
            break;
          default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input sparse matrix!\n");
        }
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_matrix(ret); 
  }

  // ----- Frovedis Arpack Wrapper ----
  PyObject* eigsh(const char* host, int port,
                  long mptr, int k,
                  const char* which, float sigma,
                  int maxiter, bool wantEv,
                  float tol, short dtype,
                  short itype, bool isDense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto m = (exrpc_ptr_t) mptr;
    bool mvbl = false;
    std::string order = which;
    eigen_result ret;
    try {
      if(isDense) {
        switch(dtype) {
          case FLOAT: 
            ret = exrpc_async(fm_node,(frovedis_dense_eigsh<R_MAT2,DT2>),
                              m, k, order, sigma, maxiter, tol, mvbl).get(); 
            break;
          case DOUBLE: 
            ret = exrpc_async(fm_node,(frovedis_dense_eigsh<R_MAT1,DT1>),
                              m, k, order, sigma,
                              maxiter, tol, mvbl).get();
            break;
          default: REPORT_ERROR(USER_ERROR,
                                "Unsupported dtype for input dense matrix!\n");
        }
      }
      else {
        switch(dtype) {
          case FLOAT: 
            if (itype == INT){
              ret = exrpc_async(fm_node,(frovedis_sparse_eigsh<S_MAT24,DT2,DT4>),
                                m, k, order, maxiter, tol, mvbl).get();}
            else if (itype == LONG) {
              ret = exrpc_async(fm_node,(frovedis_sparse_eigsh<S_MAT25,DT2,DT5>),
                                m, k, order, maxiter, tol, mvbl).get();}
            else REPORT_ERROR(USER_ERROR,
                              "Unsupported itype for input sparse matrix!\n");
            break;
          case DOUBLE: 
            if (itype == INT) {
              ret = exrpc_async(fm_node,(frovedis_sparse_eigsh<S_MAT14,DT1,DT4>),
                                m, k, order, maxiter, tol, mvbl).get();}
            else if (itype == LONG) {
              ret = exrpc_async(fm_node,(frovedis_sparse_eigsh<S_MAT15,DT1,DT5>),
                                m, k, order, maxiter, tol, mvbl).get();}
            else REPORT_ERROR(USER_ERROR,
                              "Unsupported itype for input sparse matrix!\n");
              break;
          default: REPORT_ERROR(USER_ERROR,
                                "Unsupported dtype for input sparse matrix!\n");
        }
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_eigen_result(ret, wantEv);
  }

  // --- Frovedis Scalapack Wrapper Results ---
  void release_ipiv(const char* host, int port, char mtype, long ipiv_ptr) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) ipiv_ptr;
    try {
      switch(mtype) {
        case 'C': exrpc_oneway(fm_node,(release_data<std::vector<int>>),f_dptr); break;
        case 'B': exrpc_oneway(fm_node,(release_data<lvec<int>>),f_dptr); break;
        default:  REPORT_ERROR(USER_ERROR,"Unknown input matrix kind is encountered!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  void get_float_array(const char* host, int port,
                       long dptr, float* ret) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    std::vector<float> arr;
    try {
      arr = exrpc_async(fm_node,vec_to_array<DT2>,f_dptr).get(); // SVAL/SVEC
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    float *arrp = &arr[0];
    // filling the output array (avoiding memory leak)
    for(size_t i=0; i<arr.size(); ++i) ret[i] = arrp[i];
  }

  void get_double_array(const char* host, int port, 
                        long dptr, double* ret) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    std::vector<double> arr;
    try {
      arr = exrpc_async(fm_node,vec_to_array<DT1>,f_dptr).get(); // SVAL/SVEC
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    double *arrp = &arr[0];
    // filling the output array (avoiding memory leak)
    for(size_t i=0; i<arr.size(); ++i) ret[i] = arrp[i];
  }

  PyObject* create_frovedis_int_vector(const char* host, int port,
                                       int* vec, ulong size, short dtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    std::vector<int> vv(size);
    for(size_t i = 0; i < size; ++i) vv[i] = vec[i];
    dummy_vector dv;
    try {
      dv = exrpc_async(fm_node,create_frovedis_vector<int>,vv,dtype).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_vector(dv);
  }

  PyObject* create_frovedis_long_vector(const char* host, int port,
                                        long* vec, ulong size, short dtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    std::vector<long> vv(size);
    for(size_t i = 0; i < size; ++i) vv[i] = vec[i];
    dummy_vector dv;
    try {
      dv = exrpc_async(fm_node,create_frovedis_vector<long>,vv,dtype).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_vector(dv);
  }

  PyObject* create_frovedis_float_vector(const char* host, int port,
                                         float* vec, ulong size, short dtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    std::vector<float> vv(size);
    for(size_t i = 0; i < size; ++i) vv[i] = vec[i];
    dummy_vector dv;
    try {
      dv = exrpc_async(fm_node,create_frovedis_vector<float>,vv,dtype).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_vector(dv);
  }

  PyObject* create_frovedis_double_vector(const char* host, int port,
                                          double* vec, ulong size, short dtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    std::vector<double> vv(size);
    for(size_t i = 0; i < size; ++i) vv[i] = vec[i];
    dummy_vector dv;
    try {
      dv = exrpc_async(fm_node,create_frovedis_vector<double>,vv,dtype).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_vector(dv);
  }

  PyObject* create_frovedis_string_vector(const char* host, int port,
                                          const char** vec, ulong size, 
                                          short dtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    std::vector<std::string> vv(size);
    for(size_t i = 0; i < size; ++i) vv[i] = vec[i];
    dummy_vector dv;
    try {
      dv = exrpc_async(fm_node,create_frovedis_vector<std::string>,vv,dtype).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_vector(dv);
  }

  PyObject* get_frovedis_array(const char* host, int port, 
                               long dptr, short dtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    PyObject* ret_ptr = NULL;
    try {
      switch(dtype) {
        case INT:    { 
                       auto ret = exrpc_async(fm_node,vec_to_array<int>,f_dptr).get();
                       ret_ptr = to_python_int_list(ret);
                       break;
                     }
        case LONG:   {
                       auto ret = exrpc_async(fm_node,vec_to_array<long>,f_dptr).get();
                       ret_ptr = to_python_long_list(ret);
                       break;
                     }
        case FLOAT:  {
                       auto ret = exrpc_async(fm_node,vec_to_array<float>,f_dptr).get();
                       ret_ptr = to_python_float_list(ret);
                       break;
                     }
        case DOUBLE: {
                       auto ret = exrpc_async(fm_node,vec_to_array<double>,f_dptr).get();
                       ret_ptr = to_python_double_list(ret);
                       break;
                     }
        case STRING: {
                       auto ret = exrpc_async(fm_node,vec_to_array<std::string>,f_dptr).get();
                       ret_ptr = to_python_string_list(ret);
                       break;
                     }
        default: REPORT_ERROR(USER_ERROR, "Unsupported dtype is encountered!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return ret_ptr;
  }

  //frovedis vector save
  void save_frovedis_vector_client(const char* host, int port, long dptr,
                                   const char* path, bool isBinary, short dtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    std::vector<double> arr;
    std::string fname(path);
    try {
      switch(dtype) {
        case INT:    exrpc_oneway(fm_node,save_frovedis_vector<int>,f_dptr,fname,isBinary); break;
        case LONG:   exrpc_oneway(fm_node,save_frovedis_vector<long>,f_dptr,fname,isBinary); break;
        case FLOAT:  exrpc_oneway(fm_node,save_frovedis_vector<float>,f_dptr,fname,isBinary); break;
        case DOUBLE: exrpc_oneway(fm_node,save_frovedis_vector<double>,f_dptr,fname,isBinary); break;
        //case STRING: exrpc_oneway(fm_node,save_frovedis_vector<std::string>,f_dptr,fname,isBinary); break;
        default: REPORT_ERROR(USER_ERROR, "Unsupported dtype for frovedis vector save!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  //frovedis vector load
  PyObject* load_frovedis_vector_client(const char* host, int port, 
                                        const char* path, bool isBinary, 
                                        short dtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    std::string fname(path);
    dummy_vector dv;
    try {
      switch(dtype) {
        case INT:    dv = exrpc_async(fm_node,load_frovedis_vector<int>,fname,isBinary,dtype).get(); break;
        case LONG:   dv = exrpc_async(fm_node,load_frovedis_vector<long>,fname,isBinary,dtype).get();  break;
        case FLOAT:  dv = exrpc_async(fm_node,load_frovedis_vector<float>,fname,isBinary,dtype).get();  break;
        case DOUBLE: dv = exrpc_async(fm_node,load_frovedis_vector<double>,fname,isBinary,dtype).get();  break;
        //case STRING: dv = exrpc_async(fm_node,load_frovedis_vector<std::string>,fname,isBinary,dtype).get();  break;
        default: REPORT_ERROR(USER_ERROR, "Unsupported dtype for frovedis vector load!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_vector(dv);
  }

  void release_frovedis_array(const char* host, int port, long dptr, short dtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    try {
      switch(dtype) {
        case INT:    exrpc_oneway(fm_node,(release_data<std::vector<int>>),f_dptr); break;
        case LONG:   exrpc_oneway(fm_node,(release_data<std::vector<long>>),f_dptr); break;
        case FLOAT:  exrpc_oneway(fm_node,(release_data<std::vector<float>>),f_dptr); break;
        case DOUBLE: exrpc_oneway(fm_node,(release_data<std::vector<double>>),f_dptr); break;
        case STRING: exrpc_oneway(fm_node,(release_data<std::vector<std::string>>),f_dptr); break;
        default: REPORT_ERROR(USER_ERROR, "Unsupported dtype for array release!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  void save_as_diag_matrix(const char* host, int port, long dptr,
                           const char* path, bool bin,
                           char dtype) {
    ASSERT_PTR(path);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    std::string fs_path(path);
    try {
      if (dtype == 'D') 
        exrpc_oneway(fm_node,save_as_diag_matrix_local<DT1>,f_dptr,fs_path,bin); // SVEC
      else if (dtype == 'F') 
        exrpc_oneway(fm_node,save_as_diag_matrix_local<DT2>,f_dptr,fs_path,bin); // SVEC
      else
        REPORT_ERROR(USER_ERROR, "Unsupported dtype is encountered!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  PyObject* get_svd_results_from_file(const char* host, int port,
                                      const char* spath, 
                                      const char* upath, 
                                      const char* vpath, 
                                      bool bin, bool isU, bool isV,
                                      char mtype,
                                      char dtype) {
    // upath/vpath can be empty (""), but not null
    ASSERT_PTR(spath); ASSERT_PTR(upath); ASSERT_PTR(vpath);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    std::string sfl(spath);
    std::string ufl(upath);
    std::string vfl(vpath);
    svd_result ret;
    try {
      if(dtype == 'D') {
        switch(mtype) {
          case 'C': ret = exrpc_async(fm_node,load_cmm_svd_results<DT1>,sfl,ufl,vfl,isU,isV,bin).get(); break;
          case 'B': ret = exrpc_async(fm_node,load_bcm_svd_results<DT1>,sfl,ufl,vfl,isU,isV,bin).get(); break;
          default:  REPORT_ERROR(USER_ERROR,"Unknown dense matrix kind is encountered!\n");
        }
      }
      else if (dtype == 'F') {
        switch(mtype) {
          case 'C': ret = exrpc_async(fm_node,load_cmm_svd_results<DT2>,sfl,ufl,vfl,isU,isV,bin).get(); break;
          case 'B': ret = exrpc_async(fm_node,load_bcm_svd_results<DT2>,sfl,ufl,vfl,isU,isV,bin).get(); break;
          default:  REPORT_ERROR(USER_ERROR,"Unknown dense matrix kind is encountered!\n");
        }
      }
      else REPORT_ERROR(USER_ERROR, "Unsupported dtype is encountered!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_svd_result(ret,mtype,isU,isV);    
  }


  PyObject* compute_pca(const char* host, int port,
                        long dptr,
                        int k,
                        bool whiten,
                        short dtype,
                        bool to_copy,
                        bool mvbl){
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    bool rearrange_out = true;
    bool need_scores = true;
    bool need_eig = true;
    bool need_var = true;
    bool need_sval = true;
    bool need_mean = true;

    pca_result res;
    try {
      switch(dtype) {
        case FLOAT:
          res = exrpc_async(fm_node,(frovedis_pca<R_MAT2,DT2>),f_dptr,k,
                            whiten, to_copy, rearrange_out,
                            need_scores, need_eig, need_var,
                            need_sval, need_mean, mvbl).get();
	  break;
        case DOUBLE:
	  res = exrpc_async(fm_node,(frovedis_pca<R_MAT1,DT1>),f_dptr,k,
                            whiten, to_copy, rearrange_out,
                            need_scores, need_eig, need_var,
                            need_sval, need_mean, mvbl).get();
	  break;
      	default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input dense matrix!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }

    char mtype = 'C'; // components/scores etc. are colmajor matrix
    return to_py_pca_result(res, mtype);
  }

PyObject* pca_transform(const char* host, int port,
                        long dptr,
                        long pc_ptr,
                        long var_ptr,
                        long mean_ptr,
                        short dtype,
                        bool whiten){
  if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
  exrpc_node fm_node(host,port);
  auto f_dptr = (exrpc_ptr_t) dptr;
  auto pc_ptr_exrpc = (exrpc_ptr_t) pc_ptr;
  auto var_ptr_exrpc = (exrpc_ptr_t) var_ptr;
  auto mean_ptr_exrpc = (exrpc_ptr_t) mean_ptr;
  dummy_matrix res;

  try {
    switch(dtype) {
      case FLOAT:
        res = exrpc_async(fm_node,(frovedis_pca_transform<R_MAT2,R_LMAT2,DT2>),f_dptr,
                          pc_ptr_exrpc, var_ptr_exrpc, mean_ptr_exrpc,
                          whiten).get();
        break;
      case DOUBLE:
        res = exrpc_async(fm_node,(frovedis_pca_transform<R_MAT1,R_LMAT1,DT1>),f_dptr,
                          pc_ptr_exrpc, var_ptr_exrpc, mean_ptr_exrpc,
                          whiten).get();
        break;
      default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input dense matrix!\n");
    }
  }
  catch (std::exception& e) {
    set_status(true, e.what());
  }
  return to_py_dummy_matrix(res);  
}

PyObject* pca_inverse_transform(const char* host, int port,
                                long dptr,
                                long pc_ptr,
                                long var_ptr,
                                long mean_ptr,
                                short dtype,
                                bool whiten){
  if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
  exrpc_node fm_node(host,port);
  auto f_dptr = (exrpc_ptr_t) dptr;
  auto pc_ptr_exrpc = (exrpc_ptr_t) pc_ptr;
  auto var_ptr_exrpc = (exrpc_ptr_t) var_ptr;
  auto mean_ptr_exrpc = (exrpc_ptr_t) mean_ptr;
  dummy_matrix res;

  try {
    switch(dtype) {
      case FLOAT:
        res = exrpc_async(fm_node,(frovedis_pca_inverse_transform<R_MAT2,R_LMAT2,DT2>),f_dptr,
                          pc_ptr_exrpc, var_ptr_exrpc, mean_ptr_exrpc,
                          whiten).get();
        break;
      case DOUBLE:
        res = exrpc_async(fm_node,(frovedis_pca_inverse_transform<R_MAT1,R_LMAT1,DT1>),f_dptr,
                          pc_ptr_exrpc, var_ptr_exrpc, mean_ptr_exrpc,
                          whiten).get();
        break;
      default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input dense matrix!\n");
    }
  } 
  catch (std::exception& e) {
    set_status(true, e.what());
  }
  return to_py_dummy_matrix(res);  
}

  PyObject* compute_tsne(const char* host, int port,
                         long dptr,
                         double perplexity,
                         double early_exaggeration,
                         double min_grad_norm,
                         double learning_rate,
                         int ncomponents,
                         int niter,
                         int niter_without_progress,
                         const char* metr,
                         const char* meth,
                         const char* ini,
                         bool verbose,
                         short dtype){
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    std::string metric(metr);
    std::string method(meth);
    std::string init(ini);
    size_t n_iter = (size_t)niter;
    size_t n_iter_without_progress = (size_t)niter_without_progress;
    size_t n_components = (size_t)ncomponents;

    tsne_result res;
    try {
      switch(dtype) {
        case FLOAT:
          res = exrpc_async(fm_node, (frovedis_tsne<R_MAT2,DT2>), f_dptr, 
                            perplexity, early_exaggeration, min_grad_norm, 
                            learning_rate, n_components, n_iter, 
                            n_iter_without_progress, metric, method, init, 
                            verbose).get();
          break;
        case DOUBLE:
          res = exrpc_async(fm_node, (frovedis_tsne<R_MAT1,DT1>), f_dptr,
                            perplexity, early_exaggeration, min_grad_norm, 
                            learning_rate, n_components, n_iter, 
                            n_iter_without_progress, metric, method, init, 
                            verbose).get();
          break;
        default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input dense matrix!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }

    return to_py_tsne_result(res);
  }

}
