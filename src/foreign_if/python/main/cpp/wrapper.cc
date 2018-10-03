#include "python_client_headers.hpp"
#include "exrpc_svd.hpp"
#include "exrpc_pblas.hpp"
#include "exrpc_scalapack.hpp"
#include "short_hand_dense_type.hpp"
#include "short_hand_sparse_type.hpp"

extern "C" {

  // --- Frovedis PBLAS L1 Wrapper ---
  void pswap(const char* host, int port, long vptr1, long vptr2) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto v1 = (exrpc_ptr_t) vptr1;
    auto v2 = (exrpc_ptr_t) vptr2;
    try {
      exrpc_oneway(fm_node,(frovedis_swap<DT1,B_MAT1>),v1,v2);
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  void pcopy(const char* host, int port, long vptr1, long vptr2) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto v1 = (exrpc_ptr_t) vptr1;
    auto v2 = (exrpc_ptr_t) vptr2;
    try {
      exrpc_oneway(fm_node,(frovedis_copy<DT1,B_MAT1>),v1,v2);
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  void pscal(const char* host, int port, long vptr, double al) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto v = (exrpc_ptr_t) vptr;
    try {
      exrpc_oneway(fm_node,(frovedis_scal<DT1,B_MAT1>),v,al);
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  void paxpy(const char* host, int port, long vptr1, long vptr2, double al) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto v1 = (exrpc_ptr_t) vptr1;
    auto v2 = (exrpc_ptr_t) vptr2;
    try {
      exrpc_oneway(fm_node,(frovedis_axpy<DT1,B_MAT1>),v1,v2,al);
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  double pdot(const char* host, int port, long vptr1, long vptr2) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto v1 = (exrpc_ptr_t) vptr1;
    auto v2 = (exrpc_ptr_t) vptr2;
    double ret = 0;
    try {
      ret = exrpc_async(fm_node,(frovedis_dot<DT1,B_MAT1>),v1,v2).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return ret;
  }

  double pnrm2(const char* host, int port, long vptr) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto v = (exrpc_ptr_t) vptr;
    double ret = 0;
    try {
      ret = exrpc_async(fm_node,(frovedis_nrm2<DT1,B_MAT1>),v).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return ret;
  }

  // --- Frovedis PBLAS L2 Wrapper ---
  PyObject* pgemv(const char* host, int port, 
                  long mptr, long vptr, 
                  bool trans, double al, double be) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto m = (exrpc_ptr_t) mptr;
    auto v = (exrpc_ptr_t) vptr;
    dummy_matrix r;
    try {
      r = exrpc_async(fm_node,(frovedis_gemv<DT1,B_MAT1>),m,v,trans,al,be).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_matrix(r);
  }

  PyObject* pger(const char* host, int port, 
                 long vptr1, long vptr2, double al) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto v1 = (exrpc_ptr_t) vptr1;
    auto v2 = (exrpc_ptr_t) vptr2;
    dummy_matrix r;
    try {
      r = exrpc_async(fm_node,(frovedis_ger<DT1,B_MAT1>),v1,v2,al).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_matrix(r);
  }

  // --- Frovedis PBLAS L3 Wrapper ---
  PyObject* pgemm(const char* host, int port, 
                  long mptr1, long mptr2, 
                  bool trans1, bool trans2,
                  double al, double be) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto m1 = (exrpc_ptr_t) mptr1;
    auto m2 = (exrpc_ptr_t) mptr2;
    dummy_matrix r;
    try {
      r = exrpc_async(fm_node,(frovedis_gemm<DT1,B_MAT1>),m1,m2,trans1,trans2,al,be).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_matrix(r);
  }

  void pgeadd(const char* host, int port, 
              long mptr1, long mptr2, 
              bool trans, double al, double be) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto m1 = (exrpc_ptr_t) mptr1;
    auto m2 = (exrpc_ptr_t) mptr2;
    try {
      exrpc_oneway(fm_node,(frovedis_geadd<DT1,B_MAT1>),m1,m2,trans,al,be);
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  // --- Frovedis SCALAPACK Wrapper ---
  PyObject* pgetrf(const char* host, int port, long mptr) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto m = (exrpc_ptr_t) mptr;
    getrf_result ret;
    try {
      ret = exrpc_async(fm_node,(frovedis_getrf<DT1,B_MAT1,lvec<int>>),m).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_getrf_result(ret,'B');
  }

  int pgetri(const char* host, int port, 
             long mptr, long ipiv_ptr) { 
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto m = (exrpc_ptr_t) mptr;
    auto ipiv = (exrpc_ptr_t) ipiv_ptr;
    int ret = 0;
    try {
     ret = exrpc_async(fm_node,(frovedis_getri<DT1,B_MAT1,lvec<int>>),m,ipiv).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return ret;
  }

  int pgetrs(const char* host, int port, 
             long mptrA, long mptrB, 
             long ipiv_ptr, bool trans) { 
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto mA = (exrpc_ptr_t) mptrA;
    auto mB = (exrpc_ptr_t) mptrB;
    auto ipiv = (exrpc_ptr_t) ipiv_ptr;
    int ret = 0;
    try {
      ret = exrpc_async(fm_node,(frovedis_getrs<DT1,B_MAT1,lvec<int>>),mA,mB,ipiv,trans).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return ret;
  }

  int pgesv(const char* host, int port, 
            long mptrA, long mptrB) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto mA = (exrpc_ptr_t) mptrA;
    auto mB = (exrpc_ptr_t) mptrB;
    int ret = 0;
    try {
      ret = exrpc_async(fm_node,(frovedis_gesv<DT1,B_MAT1,lvec<int>>),mA,mB).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return ret;
  }

  int pgels(const char* host, int port, 
            long mptrA, long mptrB, bool trans) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto mA = (exrpc_ptr_t) mptrA;
    auto mB = (exrpc_ptr_t) mptrB;
    int ret = 0;
    try {
      ret = exrpc_async(fm_node,(frovedis_gels<DT1,B_MAT1>),mA,mB,trans).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return ret;
  }

  PyObject* pgesvd(const char* host, int port, 
                   long mptr, bool isU, bool isV) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto m = (exrpc_ptr_t) mptr;
    gesvd_result ret;
    try {
      ret = exrpc_async(fm_node,(frovedis_gesvd<DT1,B_MAT1>),m,isU,isV).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_gesvd_result(ret,'B',isU,isV);
  }

  // --- Frovedis ARPACK Wrapper ---
  PyObject* compute_sparse_svd(const char* host, int port, 
                               long mptr, int k,
                               short dtype, short itype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto m = (exrpc_ptr_t) mptr;
    bool wantU = true;
    bool wantV = true;
    bool mvbl = false;
    char mtype = 'C';
    gesvd_result ret;
    try {
      switch(dtype) {
        case FLOAT: 
          if (itype == INT) 
            ret = exrpc_async(fm_node,(frovedis_sparse_svd<S_MAT24,DT2,DT4>),m,k,mvbl).get();
          else if (itype == LONG) 
            ret = exrpc_async(fm_node,(frovedis_sparse_svd<S_MAT25,DT2,DT5>),m,k,mvbl).get();
          else REPORT_ERROR(USER_ERROR,"Unsupported itype for input sparse matrix!\n");
          break;
        case DOUBLE: 
          if (itype == INT) 
            ret = exrpc_async(fm_node,(frovedis_sparse_svd<S_MAT14,DT1,DT4>),m,k,mvbl).get();
          else if (itype == LONG) 
            ret = exrpc_async(fm_node,(frovedis_sparse_svd<S_MAT15,DT1,DT5>),m,k,mvbl).get();
          else REPORT_ERROR(USER_ERROR,"Unsupported itype for input sparse matrix!\n");
          break;
        default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input sparse matrix!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_gesvd_result(ret,mtype,wantU,wantV);
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

  void get_double_array(const char* host, int port, 
                        long dptr, double* ret) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    std::vector<double> arr;
    try {
      arr = exrpc_async(fm_node,vec_to_array<DT1>,f_dptr).get(); // SVAL
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    double *arrp = &arr[0];
    // filling the output array (avoiding memory leak)
    for(size_t i=0; i<arr.size(); ++i) ret[i] = arrp[i];
  }

  void release_double_array(const char* host, int port, long dptr) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    try {
      exrpc_oneway(fm_node,(release_data<std::vector<double>>),f_dptr); // SVAL
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  void save_as_diag_matrix(const char* host, int port, long dptr,
                           const char* path, bool bin) {
    ASSERT_PTR(path);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    std::string fs_path(path);
    try {
      exrpc_oneway(fm_node,save_as_diag_matrix_local<DT1>,f_dptr,fs_path,bin); // SVEC
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
                                      char mtype) {
    // upath/vpath can be empty (""), but not null
    ASSERT_PTR(spath); ASSERT_PTR(upath); ASSERT_PTR(vpath);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    std::string sfl(spath);
    std::string ufl(upath);
    std::string vfl(vpath);
    gesvd_result ret;
    try {
      switch(mtype) {
        case 'C': ret = exrpc_async(fm_node,load_cmm_svd_results<DT1>,sfl,ufl,vfl,isU,isV,bin).get(); break;
        case 'B': ret = exrpc_async(fm_node,load_bcm_svd_results<DT1>,sfl,ufl,vfl,isU,isV,bin).get(); break;
        default:  REPORT_ERROR(USER_ERROR,"Unknown dense matrix kind is encountered!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_gesvd_result(ret,mtype,isU,isV);    
  }
}
