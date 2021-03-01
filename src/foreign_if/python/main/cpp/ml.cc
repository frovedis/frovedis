#include "python_client_headers.hpp"
#include "exrpc_ml.hpp"
#include "short_hand_dense_type.hpp"
#include "short_hand_sparse_type.hpp"

extern "C" {

  // --- (1) Logistic Regression ---
  void lr_sgd(const char* host, int port, long xptr, long yptr,
              int iter, double al, int rtype, double rprm, 
              bool mult, bool icpt, double tol,
              int vb, int mid, short dtype, short itype, 
              bool dense, bool shrink) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);
    double mbf = 1.0;   // default
    bool mvbl = false; // auto-managed at python side
    try {
    if(!shrink) {
      if(dense) {
        switch(dtype) {
          case FLOAT:  
	    exrpc_oneway(fm_node,(frovedis_lr_sgd<DT2,D_MAT2>),f_dptr,iter,al,
                         mbf,rtype,rprm,mult,icpt,tol,vb,mid,mvbl); 
            break;
          case DOUBLE: 
	    exrpc_oneway(fm_node,(frovedis_lr_sgd<DT1,D_MAT1>),f_dptr,iter,al,
                         mbf,rtype,rprm,mult,icpt,tol,vb,mid,mvbl); 
            break;
          default: REPORT_ERROR(USER_ERROR, 
                   "Unsupported dtype of input dense data for training!\n");
        }
      }
      else {
        switch(dtype) {
          case FLOAT: 
            if(itype == INT) 
              exrpc_oneway(fm_node,(frovedis_lr_sgd<DT2,S_MAT24>),f_dptr,iter,al,
                           mbf,rtype,rprm,mult,icpt,tol,vb,mid,mvbl);
            else if(itype == LONG) 
              exrpc_oneway(fm_node,(frovedis_lr_sgd<DT2,S_MAT25>),f_dptr,iter,al,
                           mbf,rtype,rprm,mult,icpt,tol,vb,mid,mvbl);        
            else REPORT_ERROR(USER_ERROR, 
                 "Unsupported itype of input sparse data for training!\n");
            break;
          case DOUBLE: 
            if(itype == INT) 
              exrpc_oneway(fm_node,(frovedis_lr_sgd<DT1,S_MAT14>),f_dptr,iter,al,
                           mbf,rtype,rprm,mult,icpt,tol,vb,mid,mvbl);
            else if(itype == LONG) 
              exrpc_oneway(fm_node,(frovedis_lr_sgd<DT1,S_MAT15>),f_dptr,iter,al,
                           mbf,rtype,rprm,mult,icpt,tol,vb,mid,mvbl);        
            else REPORT_ERROR(USER_ERROR, 
                 "Unsupported itype of input sparse data for training!\n");
            break;
          default: REPORT_ERROR(USER_ERROR, 
                   "Unsupported dtype of input sparse data for training!\n");
        }
      }
    }
    else {
      if(dense) {
        switch(dtype) {
          case FLOAT:  
	    exrpc_oneway(fm_node,(frovedis_lr_shrink_sgd<DT2,D_MAT2>),f_dptr,iter,al,
                         mbf,rtype,rprm,mult,icpt,tol,vb,mid,mvbl); 
            break;
          case DOUBLE: 
	    exrpc_oneway(fm_node,(frovedis_lr_shrink_sgd<DT1,D_MAT1>),f_dptr,iter,al,
                         mbf,rtype,rprm,mult,icpt,tol,vb,mid,mvbl); 
            break;
          default: REPORT_ERROR(USER_ERROR, 
                   "Unsupported dtype of input dense data for training!\n");
        }
      }
      else {
        switch(dtype) {
          case FLOAT: 
            if(itype == INT) 
              exrpc_oneway(fm_node,(frovedis_lr_shrink_sgd<DT2,S_MAT24>),f_dptr,iter,al,
                           mbf,rtype,rprm,mult,icpt,tol,vb,mid,mvbl);
            else if(itype == LONG) 
              exrpc_oneway(fm_node,(frovedis_lr_shrink_sgd<DT2,S_MAT25>),f_dptr,iter,al,
                           mbf,rtype,rprm,mult,icpt,tol,vb,mid,mvbl);        
            else REPORT_ERROR(USER_ERROR, 
                 "Unsupported itype of input sparse data for training!\n");
            break;
          case DOUBLE: 
            if(itype == INT) 
              exrpc_oneway(fm_node,(frovedis_lr_shrink_sgd<DT1,S_MAT14>),f_dptr,iter,al,
                           mbf,rtype,rprm,mult,icpt,tol,vb,mid,mvbl);
            else if(itype == LONG) 
              exrpc_oneway(fm_node,(frovedis_lr_shrink_sgd<DT1,S_MAT15>),f_dptr,iter,al,
                           mbf,rtype,rprm,mult,icpt,tol,vb,mid,mvbl);        
            else REPORT_ERROR(USER_ERROR, 
                 "Unsupported itype of input sparse data for training!\n");
            break;
          default: REPORT_ERROR(USER_ERROR, 
                   "Unsupported dtype of input sparse data for training!\n");
        }
      }
    }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    } 
  }

  void lr_lbfgs(const char* host, int port, long xptr, long yptr,
                int iter, double al, int rtype, double rprm, 
                bool mult, bool icpt, double tol,
                int vb, int mid, short dtype, short itype, bool dense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);
    int hs = 10;   // default
    bool mvbl = false; // auto-managed at python side
    try {
      if(dense) {
        switch(dtype) {
          case FLOAT:
            exrpc_oneway(fm_node,(frovedis_lr_lbfgs<DT2,D_MAT2>),f_dptr,iter,al,
                         hs,rtype,rprm,mult,icpt,tol,vb,mid,mvbl);
            break;
          case DOUBLE:
            exrpc_oneway(fm_node,(frovedis_lr_lbfgs<DT1,D_MAT1>),f_dptr,iter,al,
                         hs,rtype,rprm,mult,icpt,tol,vb,mid,mvbl);
            break;
          default: REPORT_ERROR(USER_ERROR, 
                   "Unsupported dtype of input dense data for training!\n");
        }
      }
      else {
        switch(dtype) {
          case FLOAT:
            if(itype == INT)
              exrpc_oneway(fm_node,(frovedis_lr_lbfgs<DT2,S_MAT24>),f_dptr,iter,al,
                           hs,rtype,rprm,mult,icpt,tol,vb,mid,mvbl);
            else if(itype == LONG)
              exrpc_oneway(fm_node,(frovedis_lr_lbfgs<DT2,S_MAT25>),f_dptr,iter,al,
                           hs,rtype,rprm,mult,icpt,tol,vb,mid,mvbl);
            else REPORT_ERROR(USER_ERROR, 
                 "Unsupported itype of input sparse data for training!\n");
            break;
          case DOUBLE:
            if(itype == INT)
              exrpc_oneway(fm_node,(frovedis_lr_lbfgs<DT1,S_MAT14>),f_dptr,iter,al,
                           hs,rtype,rprm,mult,icpt,tol,vb,mid,mvbl);
            else if(itype == LONG)
              exrpc_oneway(fm_node,(frovedis_lr_lbfgs<DT1,S_MAT15>),f_dptr,iter,al,
                           hs,rtype,rprm,mult,icpt,tol,vb,mid,mvbl);
            else REPORT_ERROR(USER_ERROR, 
                 "Unsupported itype of input sparse data for training!\n");
            break;
          default: REPORT_ERROR(USER_ERROR, "Unsupported dtype of input sparse data for training!\n");
        }
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  // --- (2) Linear SVM ---
  void svm_sgd(const char* host, int port, long xptr, long yptr,
               int iter, double al, 
               int rtype, double rprm, 
               bool icpt, double tol, int vb, int mid, 
               short dtype, short itype, bool dense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);
    double mbf = 1.0;   // default
    bool mvbl = false; // auto-managed at python side
    try {
      if(dense) {
        switch(dtype) {
          case FLOAT:
            exrpc_oneway(fm_node,(frovedis_svm_sgd<DT2,D_MAT2>),f_dptr,iter,al,mbf,rtype,rprm,icpt,tol,vb,mid,mvbl);
            break;
          case DOUBLE:
            exrpc_oneway(fm_node,(frovedis_svm_sgd<DT1,D_MAT1>),f_dptr,iter,al,mbf,rtype,rprm,icpt,tol,vb,mid,mvbl);
            break;
          default: REPORT_ERROR(USER_ERROR, "Unsupported dtype of input dense data for training!\n");
        }
      }
      else {
        switch(dtype) {
          case FLOAT:
            if(itype == INT)
              exrpc_oneway(fm_node,(frovedis_svm_sgd<DT2,S_MAT24>),f_dptr,iter,al,mbf,rtype,rprm,icpt,tol,vb,mid,mvbl);
            else if(itype == LONG)
              exrpc_oneway(fm_node,(frovedis_svm_sgd<DT2,S_MAT25>),f_dptr,iter,al,mbf,rtype,rprm,icpt,tol,vb,mid,mvbl);
            else REPORT_ERROR(USER_ERROR, "Unsupported itype of input sparse data for training!\n");
            break;
          case DOUBLE:
            if(itype == INT)
              exrpc_oneway(fm_node,(frovedis_svm_sgd<DT1,S_MAT14>),f_dptr,iter,al,mbf,rtype,rprm,icpt,tol,vb,mid,mvbl);
            else if(itype == LONG)
              exrpc_oneway(fm_node,(frovedis_svm_sgd<DT1,S_MAT15>),f_dptr,iter,al,mbf,rtype,rprm,icpt,tol,vb,mid,mvbl);
            else REPORT_ERROR(USER_ERROR, "Unsupported itype of input sparse data for training!\n");
            break;
          default: REPORT_ERROR(USER_ERROR, "Unsupported dtype of input sparse data for training!\n");
        }  
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  void svm_lbfgs(const char* host, int port, long xptr, long yptr,
                 int iter, double al, 
                 int rtype, double rprm, 
                 bool icpt, double tol, int vb, int mid, 
                 short dtype, short itype, bool dense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);
    int hs = 10;   // default
    bool mvbl = false; // auto-managed at python side
    try {
      if(dense) {
        switch(dtype) {
          case FLOAT:
            exrpc_oneway(fm_node,(frovedis_svm_lbfgs<DT2,D_MAT2>),f_dptr,iter,al,hs,rtype,rprm,icpt,tol,vb,mid,mvbl);
            break;
          case DOUBLE:
            exrpc_oneway(fm_node,(frovedis_svm_lbfgs<DT1,D_MAT1>),f_dptr,iter,al,hs,rtype,rprm,icpt,tol,vb,mid,mvbl);
            break;
          default: REPORT_ERROR(USER_ERROR, "Unsupported dtype of input dense data for training!\n");
        }
      }
      else {
        switch(dtype) {
          case FLOAT:
            if(itype == INT)
              exrpc_oneway(fm_node,(frovedis_svm_lbfgs<DT2,S_MAT24>),f_dptr,iter,al,hs,rtype,rprm,icpt,tol,vb,mid,mvbl);
            else if(itype == LONG)
              exrpc_oneway(fm_node,(frovedis_svm_lbfgs<DT2,S_MAT25>),f_dptr,iter,al,hs,rtype,rprm,icpt,tol,vb,mid,mvbl);
            else REPORT_ERROR(USER_ERROR, "Unsupported itype of input sparse data for training!\n");
            break;
          case DOUBLE:
            if(itype == INT)
              exrpc_oneway(fm_node,(frovedis_svm_lbfgs<DT1,S_MAT14>),f_dptr,iter,al,hs,rtype,rprm,icpt,tol,vb,mid,mvbl);
            else if(itype == LONG)
              exrpc_oneway(fm_node,(frovedis_svm_lbfgs<DT1,S_MAT15>),f_dptr,iter,al,hs,rtype,rprm,icpt,tol,vb,mid,mvbl);
            else REPORT_ERROR(USER_ERROR, "Unsupported itype of input sparse data for training!\n");
            break;
          default: REPORT_ERROR(USER_ERROR, "Unsupported dtype of input sparse data for training!\n");
        }
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  // --- Linear SVM Regressor ---
  void svm_regressor_sgd(const char* host, int port, long xptr, long yptr,
                         int iter, double alpha, double eps, 
                         int rtype, double rprm,
                         bool icpt, double tol, int intLoss, int vb, int mid,
                         short dtype, short itype, bool dense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);
    double mbf = 1.0;   // default
    bool mvbl = false; // auto-managed at python side
    try {
      if(dense) {
        switch(dtype) {
          case FLOAT:
            exrpc_oneway(fm_node,(frovedis_svm_regressor_sgd<DT2,D_MAT2>),f_dptr,iter,alpha,mbf,
                         rtype,rprm,icpt,tol,eps,intLoss,vb,mid,mvbl);
            break;
          case DOUBLE:
            exrpc_oneway(fm_node,(frovedis_svm_regressor_sgd<DT1,D_MAT1>),f_dptr,iter,alpha,mbf,
                         rtype,rprm,icpt,tol,eps,intLoss,vb,mid,mvbl);
            break;
          default: REPORT_ERROR(USER_ERROR, "Unsupported dtype of input dense data for training!\n");
        }
      }
      else {
        switch(dtype) {
          case FLOAT:
            if(itype == INT)
              exrpc_oneway(fm_node,(frovedis_svm_regressor_sgd<DT2,S_MAT24>),f_dptr,iter,alpha,mbf,
                                    rtype,rprm,icpt,tol,eps,intLoss,vb,mid,mvbl);
            else if(itype == LONG)
              exrpc_oneway(fm_node,(frovedis_svm_regressor_sgd<DT2,S_MAT25>),f_dptr,iter,alpha,mbf,
                                    rtype,rprm,icpt,tol,eps,intLoss,vb,mid,mvbl);
            else REPORT_ERROR(USER_ERROR, "Unsupported itype of input sparse data for training!\n");
            break;
          case DOUBLE:
            if(itype == INT)
              exrpc_oneway(fm_node,(frovedis_svm_regressor_sgd<DT1,S_MAT14>),f_dptr,iter,alpha,mbf,
                                    rtype,rprm,icpt,tol,eps,intLoss,vb,mid,mvbl);
            else if(itype == LONG)
              exrpc_oneway(fm_node,(frovedis_svm_regressor_sgd<DT1,S_MAT15>),f_dptr,iter,alpha,mbf,
                                    rtype,rprm,icpt,tol,eps,intLoss,vb,mid,mvbl);
            else REPORT_ERROR(USER_ERROR, "Unsupported itype of input sparse data for training!\n");
            break;
          default: REPORT_ERROR(USER_ERROR, "Unsupported dtype of input sparse data for training!\n");
        }
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  // --- SVM Kernel ---
  void frovedis_svc(const char* host, int port, long xptr, long yptr,
                    double tol, double C, int cache, int iter,
                    const char* kernel_type, double gamma, double coef, int degree,
                    int vb, int mid, short dtype, short itype, bool dense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host, port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr, f_yptr);
    auto kernel = std::string(kernel_type);
    bool mvbl = false; // auto-managed at python side
    try {
      if(dense) {
        switch(dtype) {
          case FLOAT:
            exrpc_oneway(fm_node,(frovedis_svc<DT2,R_MAT2>),f_dptr,tol,C,cache,iter,
                         kernel,gamma,coef,degree,vb,mid,mvbl);
            break;
          case DOUBLE:
            exrpc_oneway(fm_node,(frovedis_svc<DT1,R_MAT1>),f_dptr,tol,C,cache,iter,
                         kernel,gamma,coef,degree,vb,mid,mvbl);
            break;
          default: REPORT_ERROR(USER_ERROR, "Unsupported dtype of input dense data for training!\n");
        }
      }
      else REPORT_ERROR(USER_ERROR, "Frovedis doesn't support input sparse data for SVM Kernel!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  // --- (3) Linear Regression ---
  PyObject* lnr_lapack(const char* host, int port, long xptr, long yptr,
                       bool icpt, int vb, int mid, short dtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);
    bool mvbl = false;
    PyObject* retptr = NULL;
    try {
      switch(dtype) {
        case FLOAT: {
          std::vector<DT2> sval;
          sval = exrpc_async(fm_node,(frovedis_lnr_lapack<DT2,D_MAT2>),f_dptr,icpt,vb,mid,mvbl).get();
          retptr = to_python_float_list(sval);
          break;
        }
        case DOUBLE: {
          std::vector<DT1> sval;
          sval = exrpc_async(fm_node,(frovedis_lnr_lapack<DT1,D_MAT1>),f_dptr,icpt,vb,mid,mvbl).get();
          retptr = to_python_double_list(sval);
          break;
        }
        default: REPORT_ERROR(USER_ERROR, "Unsupported dtype of input dense data for training!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return retptr;
  }

  void lnr_scalapack(const char* host, int port, long xptr, long yptr,
                     bool icpt, int vb, int mid, short dtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);
    bool mvbl = false;
    try {
      switch(dtype) {
        case FLOAT:
          exrpc_oneway(fm_node,(frovedis_lnr_scalapack<DT2,D_MAT2>),f_dptr,icpt,vb,mid,mvbl);
          break;
        case DOUBLE:
          exrpc_oneway(fm_node,(frovedis_lnr_scalapack<DT1,D_MAT1>),f_dptr,icpt,vb,mid,mvbl);
          break;
        default: REPORT_ERROR(USER_ERROR, "Unsupported dtype of input dense data for training!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  void lnr_sgd(const char* host, int port, long xptr, long yptr,
               int iter, double al,
               bool icpt, double tol, int vb, int mid, 
               short dtype, short itype, bool dense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);
    double mbf = 1.0;  // default
    bool mvbl = false;  // auto-managed at python side
    try {
      if(dense) {
        switch(dtype) {
          case FLOAT:
            exrpc_oneway(fm_node,(frovedis_lnr_sgd<DT2,D_MAT2>),f_dptr,iter,al,mbf,icpt,tol,vb,mid,mvbl);
            break;
          case DOUBLE:
            exrpc_oneway(fm_node,(frovedis_lnr_sgd<DT1,D_MAT1>),f_dptr,iter,al,mbf,icpt,tol,vb,mid,mvbl);
            break;
          default: REPORT_ERROR(USER_ERROR, "Unsupported dtype of input dense data for training!\n");
        }
      }
      else {
        switch(dtype) {
          case FLOAT:
            if(itype == INT)
              exrpc_oneway(fm_node,(frovedis_lnr_sgd<DT2,S_MAT24>),f_dptr,iter,al,mbf,icpt,tol,vb,mid,mvbl);
            else if(itype == LONG)
              exrpc_oneway(fm_node,(frovedis_lnr_sgd<DT2,S_MAT25>),f_dptr,iter,al,mbf,icpt,tol,vb,mid,mvbl);
            else REPORT_ERROR(USER_ERROR, "Unsupported itype of input sparse data for training!\n");
            break;
          case DOUBLE:
            if(itype == INT)
              exrpc_oneway(fm_node,(frovedis_lnr_sgd<DT1,S_MAT14>),f_dptr,iter,al,mbf,icpt,tol,vb,mid,mvbl);
            else if(itype == LONG)
              exrpc_oneway(fm_node,(frovedis_lnr_sgd<DT1,S_MAT15>),f_dptr,iter,al,mbf,icpt,tol,vb,mid,mvbl);
            else REPORT_ERROR(USER_ERROR, "Unsupported itype of input sparse data for training!\n");
            break;
          default: REPORT_ERROR(USER_ERROR, "Unsupported dtype of input sparse data for training!\n");
        }
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  void lnr_lbfgs(const char* host, int port, long xptr, long yptr,
                 int iter, double al,
                 bool icpt, double tol, int vb, int mid, 
                 short dtype, short itype, bool dense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);
    int hs = 10;  // default
    bool mvbl = false; // auto-managed at python side
    try {
      if(dense) {
        switch(dtype) {
          case FLOAT:
            exrpc_oneway(fm_node,(frovedis_lnr_lbfgs<DT2,D_MAT2>),f_dptr,iter,al,hs,icpt,tol,vb,mid,mvbl);
            break;
          case DOUBLE:
            exrpc_oneway(fm_node,(frovedis_lnr_lbfgs<DT1,D_MAT1>),f_dptr,iter,al,hs,icpt,tol,vb,mid,mvbl);
            break;
          default: REPORT_ERROR(USER_ERROR, "Unsupported dtype of input dense data for training!\n");
        }
      }
      else {
        switch(dtype) {
          case FLOAT:
            if(itype == INT)
              exrpc_oneway(fm_node,(frovedis_lnr_lbfgs<DT2,S_MAT24>),f_dptr,iter,al,hs,icpt,tol,vb,mid,mvbl);
            else if(itype == LONG)
              exrpc_oneway(fm_node,(frovedis_lnr_lbfgs<DT2,S_MAT25>),f_dptr,iter,al,hs,icpt,tol,vb,mid,mvbl);
            else REPORT_ERROR(USER_ERROR, "Unsupported itype of input sparse data for training!\n");
            break;
          case DOUBLE:
            if(itype == INT)
              exrpc_oneway(fm_node,(frovedis_lnr_lbfgs<DT1,S_MAT14>),f_dptr,iter,al,hs,icpt,tol,vb,mid,mvbl);
            else if(itype == LONG)
              exrpc_oneway(fm_node,(frovedis_lnr_lbfgs<DT1,S_MAT15>),f_dptr,iter,al,hs,icpt,tol,vb,mid,mvbl);
            else REPORT_ERROR(USER_ERROR, "Unsupported itype of input sparse data for training!\n");
            break;
          default: REPORT_ERROR(USER_ERROR, "Unsupported dtype of input sparse data for training!\n");
        }
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  // --- (4) Lasso Regression ---
  void lasso_sgd(const char* host, int port, long xptr, long yptr,
                 int iter, double al, double rprm, 
                 bool icpt, double tol,
                 int vb, int mid, short dtype, short itype, bool dense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);
    double mbf = 1.0;   // default
    bool mvbl = false; // auto-managed at python side
    try {
      if(dense) {
        switch(dtype) {
          case FLOAT:
            exrpc_oneway(fm_node,(frovedis_lasso_sgd<DT2,D_MAT2>),f_dptr,iter,al,mbf,rprm,icpt,tol,vb,mid,mvbl);
            break;
          case DOUBLE:
            exrpc_oneway(fm_node,(frovedis_lasso_sgd<DT1,D_MAT1>),f_dptr,iter,al,mbf,rprm,icpt,tol,vb,mid,mvbl);
            break;
          default: REPORT_ERROR(USER_ERROR, "Unsupported dtype of input dense data for training!\n");
        }
      }
      else {
        switch(dtype) {
          case FLOAT:
            if(itype == INT)
              exrpc_oneway(fm_node,(frovedis_lasso_sgd<DT2,S_MAT24>),f_dptr,iter,al,mbf,rprm,icpt,tol,vb,mid,mvbl);
            else if(itype == LONG)
              exrpc_oneway(fm_node,(frovedis_lasso_sgd<DT2,S_MAT25>),f_dptr,iter,al,mbf,rprm,icpt,tol,vb,mid,mvbl);
            else REPORT_ERROR(USER_ERROR, "Unsupported itype of input sparse data for training!\n");
            break;
          case DOUBLE:
            if(itype == INT)
              exrpc_oneway(fm_node,(frovedis_lasso_sgd<DT1,S_MAT14>),f_dptr,iter,al,mbf,rprm,icpt,tol,vb,mid,mvbl);
            else if(itype == LONG)
              exrpc_oneway(fm_node,(frovedis_lasso_sgd<DT1,S_MAT15>),f_dptr,iter,al,mbf,rprm,icpt,tol,vb,mid,mvbl);
            else REPORT_ERROR(USER_ERROR, "Unsupported itype of input sparse data for training!\n");
            break;
          default: REPORT_ERROR(USER_ERROR, "Unsupported dtype of input sparse data for training!\n");
        }
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  void lasso_lbfgs(const char* host, int port, long xptr, long yptr,
                   int iter, double al, double rprm, 
                   bool icpt, double tol,
                   int vb, int mid, short dtype, short itype, bool dense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);
    int hs = 10;   // default
    bool mvbl = false; // auto-managed at python side
    try {
      if(dense) {
        switch(dtype) {
          case FLOAT:
            exrpc_oneway(fm_node,(frovedis_lasso_lbfgs<DT2,D_MAT2>),f_dptr,iter,al,hs,rprm,icpt,tol,vb,mid,mvbl);
            break;
          case DOUBLE:
            exrpc_oneway(fm_node,(frovedis_lasso_lbfgs<DT1,D_MAT1>),f_dptr,iter,al,hs,rprm,icpt,tol,vb,mid,mvbl);
            break;
          default: REPORT_ERROR(USER_ERROR, "Unsupported dtype of input dense data for training!\n");
        }
      }
      else {
        switch(dtype) {
          case FLOAT:
            if(itype == INT)
              exrpc_oneway(fm_node,(frovedis_lasso_lbfgs<DT2,S_MAT24>),f_dptr,iter,al,hs,rprm,icpt,tol,vb,mid,mvbl);
            else if(itype == LONG)
              exrpc_oneway(fm_node,(frovedis_lasso_lbfgs<DT2,S_MAT25>),f_dptr,iter,al,hs,rprm,icpt,tol,vb,mid,mvbl);
            else REPORT_ERROR(USER_ERROR, "Unsupported itype of input sparse data for training!\n");
            break;
          case DOUBLE:
            if(itype == INT)
              exrpc_oneway(fm_node,(frovedis_lasso_lbfgs<DT1,S_MAT14>),f_dptr,iter,al,hs,rprm,icpt,tol,vb,mid,mvbl);
            else if(itype == LONG)
              exrpc_oneway(fm_node,(frovedis_lasso_lbfgs<DT1,S_MAT15>),f_dptr,iter,al,hs,rprm,icpt,tol,vb,mid,mvbl);
            else REPORT_ERROR(USER_ERROR, "Unsupported itype of input sparse data for training!\n");
            break;
          default: REPORT_ERROR(USER_ERROR, "Unsupported dtype of input sparse data for training!\n");
        }
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  // --- (5) Ridge Regression ---
  void ridge_sgd(const char* host, int port, long xptr, long yptr,
                 int iter, double al, double rprm,
                 bool icpt, double tol,
                 int vb, int mid, short dtype, short itype, bool dense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);
    double mbf = 1.0;   // default
    bool mvbl = false; // auto-managed at python side
    try {
      if(dense) {
        switch(dtype) {
          case FLOAT:
            exrpc_oneway(fm_node,(frovedis_ridge_sgd<DT2,D_MAT2>),f_dptr,iter,al,mbf,rprm,icpt,tol,vb,mid,mvbl);
            break;
          case DOUBLE:
            exrpc_oneway(fm_node,(frovedis_ridge_sgd<DT1,D_MAT1>),f_dptr,iter,al,mbf,rprm,icpt,tol,vb,mid,mvbl);
            break;
          default: REPORT_ERROR(USER_ERROR, "Unsupported dtype of input dense data for training!\n");
        }
      }
      else {
        switch(dtype) {
          case FLOAT:
            if(itype == INT)
              exrpc_oneway(fm_node,(frovedis_ridge_sgd<DT2,S_MAT24>),f_dptr,iter,al,mbf,rprm,icpt,tol,vb,mid,mvbl);
            else if(itype == LONG)
              exrpc_oneway(fm_node,(frovedis_ridge_sgd<DT2,S_MAT25>),f_dptr,iter,al,mbf,rprm,icpt,tol,vb,mid,mvbl);
            else REPORT_ERROR(USER_ERROR, "Unsupported itype of input sparse data for training!\n");
            break;
          case DOUBLE:
            if(itype == INT)
              exrpc_oneway(fm_node,(frovedis_ridge_sgd<DT1,S_MAT14>),f_dptr,iter,al,mbf,rprm,icpt,tol,vb,mid,mvbl);
            else if(itype == LONG)
              exrpc_oneway(fm_node,(frovedis_ridge_sgd<DT1,S_MAT15>),f_dptr,iter,al,mbf,rprm,icpt,tol,vb,mid,mvbl);
            else REPORT_ERROR(USER_ERROR, "Unsupported itype of input sparse data for training!\n");
            break;
          default: REPORT_ERROR(USER_ERROR, "Unsupported dtype of input sparse data for training!\n");
        }
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  void ridge_lbfgs(const char* host, int port, long xptr, long yptr,
                   int iter, double al, double rprm, 
                   bool icpt, double tol,
                   int vb, int mid, short dtype, short itype, bool dense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);
    int hs = 10;   // default
    bool mvbl = false; // auto-managed at python side
    try {
      if(dense) {
        switch(dtype) {
          case FLOAT:
            exrpc_oneway(fm_node,(frovedis_ridge_lbfgs<DT2,D_MAT2>),
                         f_dptr,iter,al,hs,rprm,icpt,tol,vb,mid,mvbl);
            break;
          case DOUBLE:
            exrpc_oneway(fm_node,(frovedis_ridge_lbfgs<DT1,D_MAT1>),
                         f_dptr,iter,al,hs,rprm,icpt,tol,vb,mid,mvbl);
            break;
          default: REPORT_ERROR(USER_ERROR, 
                   "Unsupported dtype of input dense data for training!\n");
        }
      }
      else {
        switch(dtype) {
          case FLOAT:
            if(itype == INT)
              exrpc_oneway(fm_node,(frovedis_ridge_lbfgs<DT2,S_MAT24>),
                           f_dptr,iter,al,hs,rprm,icpt,tol,vb,mid,mvbl);
            else if(itype == LONG)
              exrpc_oneway(fm_node,(frovedis_ridge_lbfgs<DT2,S_MAT25>),
                           f_dptr,iter,al,hs,rprm,icpt,tol,vb,mid,mvbl);
            else REPORT_ERROR(USER_ERROR, 
                 "Unsupported itype of input sparse data for training!\n");
            break;
          case DOUBLE:
            if(itype == INT)
              exrpc_oneway(fm_node,(frovedis_ridge_lbfgs<DT1,S_MAT14>),
                           f_dptr,iter,al,hs,rprm,icpt,tol,vb,mid,mvbl);
            else if(itype == LONG)
              exrpc_oneway(fm_node,(frovedis_ridge_lbfgs<DT1,S_MAT15>),
                           f_dptr,iter,al,hs,rprm,icpt,tol,vb,mid,mvbl);
            else REPORT_ERROR(USER_ERROR, 
                 "Unsupported itype of input sparse data for training!\n");
            break;
          default: REPORT_ERROR(USER_ERROR, 
                   "Unsupported dtype of input sparse data for training!\n");
        }
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  // case: SGDClassifier with "squared_loss"
  void lnr2_sgd(const char* host, int port, long xptr, long yptr,
               int iter, double al,
               int rtype, double rprm,
               bool icpt, double tol, int vb, int mid,
               short dtype, short itype, bool dense) {
    switch(rtype) {
      case 0: lnr_sgd(host, port, xptr, yptr, iter, al, icpt, 
                      tol, vb, mid, dtype, itype, dense);
              break;
      case 1: lasso_sgd(host, port, xptr, yptr, iter, al, rprm, 
                        icpt, tol, vb, mid, dtype, itype, dense);
              break;
      case 2: ridge_sgd(host, port, xptr, yptr, iter, al, rprm, 
                        icpt, tol, vb, mid, dtype, itype, dense);
              break;
      default: REPORT_ERROR(USER_ERROR, 
               "Unsupported regularization type is encountered!\n");
    }
  }

  // --- (6) Kmeans ---
  PyObject* kmeans_fit(const char* host, int port, long xptr, int k,
                       int iter, int n_init, double eps, long seed,
                       int vb, int mid, 
                       short dtype, short itype, 
                       bool dense, bool shrink) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    bool mvbl = false; // auto-managed at python side
    kmeans_result ret;
    try {
      if(dense) {
        switch(dtype) {
          case FLOAT: 
            ret = exrpc_async(fm_node,(frovedis_kmeans_fit<DT2,R_MAT2>),
                              f_xptr,k,iter,n_init,eps,seed,vb,mid,shrink,mvbl).get();
            break;
          case DOUBLE: 
            ret = exrpc_async(fm_node,(frovedis_kmeans_fit<DT1,R_MAT1>),
                              f_xptr,k,iter,n_init,eps,seed,vb,mid,shrink,mvbl).get();
            break;
          default: REPORT_ERROR(USER_ERROR, 
                   "Unsupported dtype of input dense data for training!\n");
        }
      }
      else {
        switch(dtype) {
          case FLOAT: 
            if(itype == INT) 
              ret = exrpc_async(fm_node,(frovedis_kmeans_fit<DT2,S_MAT24>),
                                f_xptr,k,iter,n_init,eps,seed,vb,mid,shrink,mvbl).get();
            else if(itype == LONG) 
              ret = exrpc_async(fm_node,(frovedis_kmeans_fit<DT2,S_MAT25>),
                                f_xptr,k,iter,n_init,eps,seed,vb,mid,shrink,mvbl).get();
            else REPORT_ERROR(USER_ERROR, 
                              "Unsupported itype of input sparse data for training!\n");
            break;
          case DOUBLE: 
            if(itype == INT) 
              ret = exrpc_async(fm_node,(frovedis_kmeans_fit<DT1,S_MAT14>),
                                f_xptr,k,iter,n_init,eps,seed,vb,mid,shrink,mvbl).get();
            else if(itype == LONG) 
              ret = exrpc_async(fm_node,(frovedis_kmeans_fit<DT1,S_MAT15>),
                                f_xptr,k,iter,n_init,eps,seed,vb,mid,shrink,mvbl).get();
            else REPORT_ERROR(USER_ERROR, 
                 "Unsupported itype of input sparse data for training!\n");
            break;
          default: REPORT_ERROR(USER_ERROR, 
                   "Unsupported dtype of input sparse data for training!\n");
        }
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_kmeans_result(ret);
  }

  PyObject* kmeans_fit_transform(const char* host, int port, 
                                 long xptr, int k,
                                 int iter, int n_init, double eps, long seed,
                                 int vb, int mid,
                                 short dtype, short itype,
                                 bool dense, bool shrink) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    bool mvbl = false; // auto-managed at python side
    kmeans_result ret;
    try {
      if(dense) {
        switch(dtype) {
          case FLOAT:
            ret = exrpc_async(fm_node,(frovedis_kmeans_fit_transform<DT2,R_MAT2>),
                              f_xptr,k,iter,n_init,eps,seed,vb,mid,shrink,mvbl).get();
            break;
          case DOUBLE:
            ret = exrpc_async(fm_node,(frovedis_kmeans_fit_transform<DT1,R_MAT1>),
                              f_xptr,k,iter,n_init,eps,seed,vb,mid,shrink,mvbl).get();
            break;
          default: REPORT_ERROR(USER_ERROR,
                   "Unsupported dtype of input dense data for training!\n");
        }
      }
      else {
        switch(dtype) {
          case FLOAT:
            if(itype == INT)
              ret = exrpc_async(fm_node,(frovedis_kmeans_fit_transform<DT2,S_MAT24>),
                                f_xptr,k,iter,n_init,eps,seed,vb,mid,shrink,mvbl).get();
            else if(itype == LONG)
              ret = exrpc_async(fm_node,(frovedis_kmeans_fit_transform<DT2,S_MAT25>),
                                f_xptr,k,iter,n_init,eps,seed,vb,mid,shrink,mvbl).get();
            else REPORT_ERROR(USER_ERROR,
                              "Unsupported itype of input sparse data for training!\n");
            break;
          case DOUBLE:
            if(itype == INT)
              ret = exrpc_async(fm_node,(frovedis_kmeans_fit_transform<DT1,S_MAT14>),
                                f_xptr,k,iter,n_init,eps,seed,vb,mid,shrink,mvbl).get();
            else if(itype == LONG)
              ret = exrpc_async(fm_node,(frovedis_kmeans_fit_transform<DT1,S_MAT15>),
                                f_xptr,k,iter,n_init,eps,seed,vb,mid,shrink,mvbl).get();
            else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data for training!\n");
            break;
          default: REPORT_ERROR(USER_ERROR,
                   "Unsupported dtype of input sparse data for training!\n");
        }
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_kmeans_result(ret);
  }

  // (7) --- Agglomerative Clustering ---
  void aca_train(const char* host, int port, long xptr, int k, 
                 const char* linkage, double threshold,
                 long* ret, long len,
                 int vb, int mid, 
                 short dtype, short itype, bool dense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");   
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    bool mvbl = false; // auto-managed at python side
    std::string linkages = linkage;
    std::vector<int> pred;
    try {
      if(dense) {
        switch(dtype) {
          case FLOAT:
            pred = exrpc_async(fm_node,(frovedis_aca<DT2,R_MAT2>),
                               f_xptr,mid,linkages,k,(float)threshold,vb,mvbl).get();
            break;
          case DOUBLE: 
            pred = exrpc_async(fm_node,(frovedis_aca<DT1,R_MAT1>),
                               f_xptr,mid,linkages,k,threshold,vb,mvbl).get();
            break;
          default: REPORT_ERROR(USER_ERROR, 
                   "Unsupported dtype of input dense data for training!\n");
        }
      }
      else  REPORT_ERROR(USER_ERROR, 
            "Frovedis doesn't support input sparse data for agglomerative clustering!\n");
      auto sz = pred.size();
      checkAssumption(len == sz);
      for(size_t i=0; i<sz; ++i) {ret[i] = static_cast<long>(pred[i]);}
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  // (8) --- Spectral Embedding --
  void sea_train(const char* host, int port, long xptr,
                 int k, double gamma, 
                 bool precomputed, bool norm_laplacian,
                 int mode, bool drop_first, int vb, int mid,
                 short dtype, short itype, bool dense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    bool mvbl = false; // auto-managed at python side
    try {
      if(dense) {
        switch(dtype) {
          case FLOAT:
            exrpc_oneway(fm_node,(frovedis_sea<DT2,R_MAT2>),f_xptr,k,
                         gamma,norm_laplacian,mid,vb,precomputed,mode,
                         drop_first,mvbl);
            break;
          case DOUBLE:
            exrpc_oneway(fm_node,(frovedis_sea<DT1,R_MAT1>),f_xptr,k,
                         gamma,norm_laplacian,mid,vb,precomputed,mode,
                         drop_first,mvbl);
            break;
          default: REPORT_ERROR(USER_ERROR, 
                   "Unsupported dtype of input dense data for training!\n");
        }
      }
      else  REPORT_ERROR(USER_ERROR, 
            "Frovedis doesn't support input sparse data for spectral embedding!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  // (9) --- Spectral Clustering ---
  void sca_train(const char* host, int port, long xptr,
                 int k,int n_comp, int n_iter, double eps, double gamma,
                 bool precomputed, bool norm_laplacian, int mode,
                 bool drop_first, int* ret, long len, int vb, int mid,
                 short dtype, short itype, bool dense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    bool mvbl = false; // auto-managed at python side
    std::vector<int> pred;
    try {
      if(dense) {
        switch(dtype) {
          case FLOAT:
            pred = exrpc_async(fm_node,(frovedis_sca<DT2,R_MAT2>),
                               f_xptr,k,n_iter,n_comp,eps,gamma,
                               norm_laplacian,mid,vb,precomputed,
                               mode,drop_first,mvbl).get();
            break;
          case DOUBLE:
            pred = exrpc_async(fm_node,(frovedis_sca<DT1,R_MAT1>),
                               f_xptr,k,n_iter,n_comp,eps,gamma,
                               norm_laplacian,mid,vb,precomputed,
                               mode,drop_first,mvbl).get();
            break;
          default: REPORT_ERROR(USER_ERROR, 
                   "Unsupported dtype of input dense data for training!\n");
        }
      }
      else  REPORT_ERROR(USER_ERROR, 
            "Frovedis doesn't support input sparse data for spectral clustering!\n");
      auto sz = pred.size();
      checkAssumption(len == sz);
      for(size_t i=0; i<len && i<sz; ++i) ret[i] = pred[i];
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  // --- (10) DBSCAN ---
  void dbscan_train(const char* host, int port, long xptr, 
                    double* sample_weight_ptr, long sample_weight_len,
                    double eps, int min_pts,
                    int* ret, long len, int vb, int mid, 
                    short dtype, short itype, bool dense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    std::vector<int> pred;
    try {
      if(dense) {
        switch(dtype) {
          case FLOAT:
            {
            auto sample_weight = double_to_float_vector(sample_weight_ptr,
                                                  sample_weight_len);
            pred = exrpc_async(fm_node,(frovedis_dbscan<DT2,R_MAT2>),
                               f_xptr,sample_weight,eps,min_pts,vb,mid).get();
            }
            break;
          case DOUBLE:
            {
            auto sample_weight = to_double_vector(sample_weight_ptr,
                                                  sample_weight_len);
            pred = exrpc_async(fm_node,(frovedis_dbscan<DT1,R_MAT1>),
                               f_xptr,sample_weight,eps,min_pts,vb,mid).get();
            }
            break;
          default: REPORT_ERROR(USER_ERROR, 
                   "Unsupported dtype of input dense data for training!\n");
        }
      }
      else REPORT_ERROR(USER_ERROR, 
           "Frovedis doesn't support input sparse data for DBSCAN!\n");
      auto sz = pred.size();
      checkAssumption(len == sz);
      for(size_t i=0; i<sz; ++i) ret[i] = pred[i];
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  // --- (11) Matrix Factorization using ALS ---
  void als_train(const char* host, int port, long dptr, int rank,
                 int iter, double al, double rprm, long seed,
                 int vb, int mid, short dtype, short itype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    bool mvbl = false; // auto-managed at python side
    try {
      switch(dtype) {
        case FLOAT:
          if(itype == INT) 
            exrpc_oneway(fm_node,(frovedis_mf_als<DT2,S_MAT24>),f_dptr,rank,
                         iter,al,rprm,seed,vb,mid,mvbl);
          else if(itype == LONG) 
            exrpc_oneway(fm_node,(frovedis_mf_als<DT2,S_MAT25>),f_dptr,rank,
                         iter,al,rprm,seed,vb,mid,mvbl);
          else REPORT_ERROR(USER_ERROR, 
               "Unsupported itype of input sparse data for training!\n");
          break;
        case DOUBLE:
          if(itype == INT) 
            exrpc_oneway(fm_node,(frovedis_mf_als<DT1,S_MAT14>),f_dptr,rank,
                         iter,al,rprm,seed,vb,mid,mvbl);
          else if(itype == LONG) 
            exrpc_oneway(fm_node,(frovedis_mf_als<DT1,S_MAT15>),f_dptr,rank,
                         iter,al,rprm,seed,vb,mid,mvbl);
          else REPORT_ERROR(USER_ERROR, 
               "Unsupported itype of input sparse data for training!\n");
          break;
        default: REPORT_ERROR(USER_ERROR, 
                 "Unsupported dtype of input sparse data for training!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  // --- (12) Naive Bayes ---
  void nb_trainer(const char* host, int port, long xptr,
                 long yptr, double alpha, bool fit_prior, 
                 double* class_prior_ptr, long len_class_prior, 
                 double* sample_weight_ptr, long len_sample_weight,
                 int mid,
                 const char* algo, double binarize, int verbose, 
		 short dtype, short itype, bool dense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);
    std::string algos = algo;
    bool mvbl = false; // auto-managed at python side
    try {
      if(dense) {
        switch(dtype) {
          case FLOAT:
            {
              auto class_prior = double_to_float_vector(class_prior_ptr,
                                                        len_class_prior);
              auto sample_weight = double_to_float_vector(sample_weight_ptr,
                                                          len_sample_weight);
              exrpc_oneway(fm_node,(frovedis_nb<DT2,D_MAT2,D_LMAT2>),
                           f_dptr,algos,alpha,fit_prior,class_prior,sample_weight,
                           binarize,verbose,mid,mvbl);
            }
            break;
          case DOUBLE:
            {
              auto class_prior = to_double_vector(class_prior_ptr,
                                                  len_class_prior);
              auto sample_weight = to_double_vector(sample_weight_ptr,
                                                    len_sample_weight);
              exrpc_oneway(fm_node,(frovedis_nb<DT1,D_MAT1,D_LMAT1>),
                           f_dptr,algos,alpha,fit_prior,class_prior, sample_weight,
                           binarize,verbose,mid,mvbl);
            }
            break;
          default: REPORT_ERROR(USER_ERROR, 
                   "Unsupported dtype of input dense data for training!\n");
        }
      }
      else {
        switch(dtype) {
          case FLOAT:
            {
              auto class_prior = double_to_float_vector(class_prior_ptr,
                                                        len_class_prior);
              auto sample_weight = double_to_float_vector(sample_weight_ptr,
                                                          len_sample_weight);
              if(itype == INT)
                exrpc_oneway(fm_node,(frovedis_nb<DT2,S_MAT24,S_LMAT24>),
                             f_dptr,algos,alpha,fit_prior,class_prior,sample_weight,
                             binarize,verbose,mid,mvbl);
              else if(itype == LONG)
                exrpc_oneway(fm_node,(frovedis_nb<DT2,S_MAT25,S_LMAT25>),
                             f_dptr,algos,alpha,fit_prior,class_prior,sample_weight,
                             binarize,verbose,mid,mvbl);
              else REPORT_ERROR(USER_ERROR, 
                   "Unsupported itype of input sparse data for training!\n");
            }
            break;
          case DOUBLE:
            {
              auto class_prior = to_double_vector(class_prior_ptr,
                                                  len_class_prior);
              auto sample_weight = to_double_vector(sample_weight_ptr,
                                                    len_sample_weight);
              if(itype == INT)
                exrpc_oneway(fm_node,(frovedis_nb<DT1,S_MAT14,S_LMAT14>),
                             f_dptr,algos,alpha,fit_prior,class_prior,sample_weight,
                             binarize,verbose,mid,mvbl);
              else if(itype == LONG)
                exrpc_oneway(fm_node,(frovedis_nb<DT1,S_MAT15,S_LMAT15>),
                             f_dptr,algos,alpha,fit_prior,class_prior,sample_weight,
                             binarize,verbose,mid,mvbl);
              else REPORT_ERROR(USER_ERROR, 
                   "Unsupported itype of input sparse data for training!\n");
            }
            break;
          default: REPORT_ERROR(USER_ERROR, 
                   "Unsupported dtype of input sparse data for training!\n");
        }
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  // --- (13) Factorization Machine ---
  void fm_trainer(const char* host, int port, long xptr,
                  long yptr, double std_dev, int iter,
                  double init_learn_rate, char* optimizer, bool dim1,
                  bool dim2, int dim3, double reg1,
                  double reg2, double reg3, int batch_size,
                  int mid, bool isregressor, int verbose,
                  short dtype, short itype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);
    std::string opt = optimizer;
    bool mvbl = false; // auto-managed at python side
    try {
      switch(dtype) {
        case FLOAT: {
          auto stddev = static_cast<float>(std_dev);
          auto lrate  = static_cast<float>(init_learn_rate);
          auto r1 = static_cast<float>(reg1);
          auto r2 = static_cast<float>(reg2);
          auto r3 = static_cast<float>(reg3);
          fm::fm_config<float> conf(dim1, dim2, dim3, stddev, iter, lrate,
                                    r1, r2, r3, isregressor, batch_size);
          if(itype == INT)
            exrpc_oneway(fm_node,(frovedis_fm<DT2,S_MAT24>),f_dptr,opt,conf,verbose,mid,mvbl);
          else if(itype == LONG)
            exrpc_oneway(fm_node,(frovedis_fm<DT2,S_MAT25>),f_dptr,opt,conf,verbose,mid,mvbl);
          else REPORT_ERROR(USER_ERROR,"Supported itypes are either int or long!\n");
          break;
        }
        case DOUBLE: { 
          fm::fm_config<double> conf(dim1, dim2, dim3, std_dev, iter, init_learn_rate,
                                    reg1, reg2, reg3, isregressor, batch_size);
          if(itype == INT)
            exrpc_oneway(fm_node,(frovedis_fm<DT1,S_MAT14>),f_dptr,opt,conf,verbose,mid,mvbl);
          else if(itype == LONG)
            exrpc_oneway(fm_node,(frovedis_fm<DT1,S_MAT15>),f_dptr,opt,conf,verbose,mid,mvbl);
          else REPORT_ERROR(USER_ERROR,"Supported itypes are either int or long!\n");
          break;
        }
        default: REPORT_ERROR(USER_ERROR,"Unsupported dtypes for the given sparse data!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  // --- (14) Decision Tree ---
  void dt_trainer(const char* host, int port, long xptr,
                  long yptr, char* algo, char* impurity,
                  int max_depth, int num_classes, int max_bins,
                  int min_instance, double min_info_gain,
                  int verbose, int mid, 
                  short dtype, short itype, bool dense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);
    auto algorithm = tree::algorithm::Regression;
    if(std::string(algo) == "Classification") 
      algorithm = tree::algorithm::Classification;
    else algorithm = tree::algorithm::Regression;
    auto imp = std::string(impurity);
    auto impt = tree::impurity_type::Default;
    if (imp == "DEFAULT")  impt = tree::impurity_type::Default;
    else if (imp == "GINI")  impt = tree::impurity_type::Gini;
    else if (imp == "ENTROPY")  impt = tree::impurity_type::Entropy;
    else if (imp == "MSE") impt = tree::impurity_type::Variance;
    else REPORT_ERROR(USER_ERROR, "Unsupported impurity is provided!\n");
    bool mvbl = false; // auto-managed at python side
    try {
      if(dense) {
        switch(dtype) {
          case FLOAT: { 
            auto min_info_gain_cp = static_cast<float>(min_info_gain);
            tree::strategy<float> float_str(
                  algorithm, impt,
                  max_depth, num_classes, max_bins,
                  tree::quantile_strategy::ApproxHist,
                  tree::categorize_strategy::Single,
                  std::unordered_map<size_t,size_t>(),
                  min_instance, min_info_gain_cp);
            exrpc_oneway(fm_node,(frovedis_dt<DT2,D_MAT2>),f_dptr,float_str,verbose,mid,mvbl); 
            break;
          }
          case DOUBLE: {
            tree::strategy<double> double_str(
                  algorithm, impt,
                  max_depth, num_classes, max_bins,
                  tree::quantile_strategy::ApproxHist,
                  tree::categorize_strategy::Single,
                  std::unordered_map<size_t,size_t>(),
                  min_instance, min_info_gain);
            exrpc_oneway(fm_node,(frovedis_dt<DT1,D_MAT1>),f_dptr,double_str,verbose,mid,mvbl);
            break;
          }
          default: REPORT_ERROR(USER_ERROR, "Unsupported dtype of input dense data for training!\n");
        }
      }
      else REPORT_ERROR(USER_ERROR, "Frovedis doesn't support input sparse data for DT training!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  // --- (15) FP Growth ---
  void fpgrowth_trainer(const char* host, int port, long fdata, 
                        int mid, double minSupport, int verbose) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    //std::cout<<"Inside fpgrowth_trainer: --------------- \n";
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) (fdata);
    bool mvbl = false; // auto-managed at python side
    int vb = verbose; // no log (default)
    try {
      exrpc_oneway(fm_node, frovedis_fp_growth<dftable>, f_dptr, minSupport, vb, mid, mvbl);
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  void fpgrowth_fpr(const char* host, int port,
                        int mid, int midr, double con) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    try {
      exrpc_oneway(fm_node, frovedis_fpr<fp_growth_model>,con, mid, midr);
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  // --- (16) Word2Vector ---
  void w2v_build_vocab_and_dump(const char* text, const char* encode,
                                const char* vocab, const char* count,
                                int minCount) {
    ASSERT_PTR(text); ASSERT_PTR(encode);
    ASSERT_PTR(vocab); ASSERT_PTR(count);
    std::string txt(text), enc(encode), voc(vocab), cnt(count);
    // x86 side vocab creation
    try {
      w2v::build_vocab_and_dump(txt,enc,voc,cnt,minCount);
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  void w2v_train(const char* host, int port,
                 const char* encode, const char* weight,
                 const char* count, int hiddenSize, int window,
                 float thr, int neg, int niter, float lr,
                 float syncPeriod, int syncWords, 
                 int syncTimes, int msgsz, int nthread) {
    ASSERT_PTR(weight); ASSERT_PTR(encode); ASSERT_PTR(count);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    std::string enc(encode), cnt(count), wght(weight);

    int are_supposed_parameters =    \
       hiddenSize <= 512 &&          \
       hiddenSize % 2 == 0 &&        \
       neg == 5 &&                   \
       window <= 8;
    if (!are_supposed_parameters) {
       std::cout << "===============  CAUTION  ===============\n"
                 << "Optimized computation is not supported for the specified arguments so this program fallbacks to slower version.\n"
                 << "Recommended arguments to enable optimization are \n"
                 << "    size <= 512 && " << std::endl
                 << "    size % 2 == 0 && " << std::endl
                 << "    negative == 5 && " << std::endl
                 << "    window <= 8" << std::endl
                 << "=========================================\n";
    }
    w2v::train_config config = {
       hiddenSize, window, thr, neg, niter,
       lr, syncPeriod, syncWords, syncTimes, 
       msgsz, nthread
    };

    try {
      exrpc_oneway(fm_node,frovedis_w2v_train,enc,wght,cnt,config);
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  void w2v_save_model(const char* weight, const char* vocab,
                      const char* out, int minCount,
                      bool isBinary) {
    ASSERT_PTR(weight); ASSERT_PTR(vocab); ASSERT_PTR(out); 
    std::string wght(weight), voc(vocab), output(out);
    // x86 side model saving
    try {
      w2v::save_model(wght, voc, out, minCount, isBinary);
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  // --- (17) K-Nearest Neighbor (KNN) ---
  void knn_fit(const char* host, int port, long xptr, int k,
               float radius, const char* algorithm, const char* metric,
               float chunk_size, int vb, int mid, 
               short dtype, short itype, bool dense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    std::string algorithm_ = algorithm;
    std::string metric_ = metric;
    try {
      if(dense) {
        switch(dtype) {
          case FLOAT: 
            exrpc_oneway(fm_node,(frovedis_knn<DT2,R_MAT2>), f_xptr, k, radius,
                         algorithm_, metric_, chunk_size, vb, mid);
            break;
          case DOUBLE: 
            exrpc_oneway(fm_node,(frovedis_knn<DT1,R_MAT1>), f_xptr, k, radius,
                         algorithm_, metric_, chunk_size, vb, mid);
            break;
          default: REPORT_ERROR(USER_ERROR, "Unsupported dtype of input dense data for training!\n");
        }
      }
      else{
        REPORT_ERROR(USER_ERROR, "frovedis Nearest Neighbors currently supports only dense data. \n");
      } 
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  // --- (18) K-Nearest Neighbor Classifier (KNC) ---
  void knc_fit(const char* host, int port, long xptr, long yptr, int k,
               const char* algorithm, const char* metric,
               float chunk_size, int vb, int mid, 
               short dtype, short itype, bool dense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);

    std::string algorithm_ = algorithm;
    std::string metric_ = metric;
    try {
      if(dense) {
        switch(dtype) {
          case FLOAT: 
            exrpc_oneway(fm_node,(frovedis_knc<DT2,R_MAT2>), f_dptr, k,
                         algorithm_, metric_, chunk_size, vb, mid);
            break;
          case DOUBLE: 
            exrpc_oneway(fm_node,(frovedis_knc<DT1,R_MAT1>), f_dptr, k,
                         algorithm_, metric_, chunk_size, vb, mid);
            break;
          default: REPORT_ERROR(USER_ERROR, "Unsupported dtype of input dense data for training!\n");
        }
      }
      else{
        REPORT_ERROR(USER_ERROR, "frovedis Nearest Neighbors currently supports only dense data. \n");
      } 
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  // --- (19) K-Nearest Neighbor Regressor (KNR) ---
  void knr_fit(const char* host, int port, long xptr, long yptr, int k,
               const char* algorithm, const char* metric,
               float chunk_size, int vb, int mid, 
               short dtype, short itype, bool dense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);

    std::string algorithm_ = algorithm;
    std::string metric_ = metric;
    try {
      if(dense) {
        switch(dtype) {
          case FLOAT: 
            exrpc_oneway(fm_node,(frovedis_knr<DT2,R_MAT2>), f_dptr, k,
                         algorithm_, metric_, chunk_size, vb, mid);
            break;
          case DOUBLE: 
            exrpc_oneway(fm_node,(frovedis_knr<DT1,R_MAT1>), f_dptr, k,
                         algorithm_, metric_, chunk_size, vb, mid);
            break;
          default: REPORT_ERROR(USER_ERROR, "Unsupported dtype of input dense data for training!\n");
        }
      }
      else{
        REPORT_ERROR(USER_ERROR, "frovedis Nearest Neighbors currently supports only dense data. \n");
      } 
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  // --- (20) Latent Dirichlet Allocation (LDA) ---
  void compute_lda_train(const char* host, int port,
                         long dptr, double alpha,
                         double beta, int num_topics,
                         int num_iter, const char* algorithm,
                         int num_explore_iter,  int num_eval_cycle,
                         short dtype, short itype, int verbose, int mid) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    std::string algo(algorithm);
    try {
      switch(dtype) {
        case INT:
          if (itype == INT)
            //exrpc_async(fm_node,(frovedis_lda_train<DT4,S_MAT44>), f_dptr, alpha,
            //             beta, num_topics, num_iter, algo, num_explore_iter, num_eval_cycle,
            //             verbose, mid);
            REPORT_ERROR(USER_ERROR,
                         "Currently frovedis LDA doesn't support int itype for sparse data!\n");
          else if (itype == LONG )
            exrpc_async(fm_node,(frovedis_lda_train<DT4,S_MAT45>), f_dptr, alpha,
                         beta, num_topics, num_iter, algo, num_explore_iter, num_eval_cycle,
                         verbose, mid);
          else REPORT_ERROR(USER_ERROR, "Unsupported itype for input sparse datain LDA train!\n");
          break;
        case LONG:
          if (itype == INT)
            //exrpc_async(fm_node,(frovedis_lda_train<DT3,S_MAT34>), f_dptr, alpha,
            //             beta, num_topics, num_iter, algo, num_explore_iter, num_eval_cycle,
            //             verbose, mid);
            REPORT_ERROR(USER_ERROR,
                         "Currently frovedis LDA doesn't support int itype for sparse data!\n");
          else if (itype == LONG)
            exrpc_async(fm_node,(frovedis_lda_train<DT3,S_MAT35>), f_dptr, alpha,
                         beta, num_topics, num_iter, algo, num_explore_iter, num_eval_cycle,
                         verbose, mid);
          else REPORT_ERROR(USER_ERROR, "Unsupported itype for input sparse datain LDA train!\n");
          break;
        default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input matrix!\n");
      }
   }
   catch (std::exception& e) {
     set_status(true, e.what());
   }
 }

  // --- (21) Random Forest ---
  // subsampling rate ?
  void rf_trainer(const char* host, int port, long xptr,
                  long yptr, char* algo, char* impurity,
                  int num_trees, int max_depth, int num_classes,
                  const char* feature_subset_strategy,
                  double feature_subset_rate, int max_bins,
                  int min_instance, double min_impurity_decrease,
                  long random_state, int verbose, int mid,
                  short dtype, short itype, bool dense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);
    auto algorithm = tree::algorithm::Regression;
    if(std::string(algo) == "Classification")
      algorithm = tree::algorithm::Classification;
    else algorithm = tree::algorithm::Regression;
    auto imp = std::string(impurity);
    auto feature_subset_strategy_ = std::string(feature_subset_strategy);
    bool mvbl = false; // auto-managed at python side
    try {
      if(dense) {
        switch(dtype) {
          case FLOAT: {
            auto min_impurity_decrease_float = static_cast<float>(min_impurity_decrease);
            auto feature_subset_rate_float = static_cast<float>(feature_subset_rate);
            tree::sampling_strategy<float> subsample_strat;
            subsample_strat.set_feature_subset_strategy(feature_subset_strategy_)
                           .set_feature_subset_rate(feature_subset_rate_float);
            tree::strategy<float> float_str;
            float_str.set_algorithm(algorithm)
                 .set_impurity_type(imp)
                 .set_max_depth(max_depth)
                 .set_max_bins(max_bins)
                 .set_min_instances_per_node(min_instance)
                 .set_min_info_gain(min_impurity_decrease_float)
                 .set_num_trees(num_trees)
                 .set_sampling_strategy(std::move(subsample_strat));
            if(std::string(algo) == "Classification")
              float_str.set_num_classes(num_classes);
            if(random_state != -1)
              float_str.set_seed(random_state);
            exrpc_oneway(fm_node,(frovedis_rf<DT2,D_MAT2>),f_dptr,float_str,verbose,mid,mvbl);
            break;
          }
          case DOUBLE: {
            tree::sampling_strategy<double> subsample_strat;
            subsample_strat.set_feature_subset_strategy(feature_subset_strategy_)
                           .set_feature_subset_rate(feature_subset_rate);
            tree::strategy<double> double_str;
            double_str.set_algorithm(algorithm)
                 .set_impurity_type(imp)
                 .set_max_depth(max_depth)
                 .set_max_bins(max_bins)
                 .set_min_instances_per_node(min_instance)
                 .set_min_info_gain(min_impurity_decrease)
                 .set_num_trees(num_trees)
                 .set_sampling_strategy(std::move(subsample_strat));
            if(std::string(algo) == "Classification")
              double_str.set_num_classes(num_classes);
            if(random_state != -1)
              double_str.set_seed(random_state);
            exrpc_oneway(fm_node,(frovedis_rf<DT1,D_MAT1>),f_dptr,double_str,verbose,mid,mvbl);
            break;
          }
          default: REPORT_ERROR(USER_ERROR, 
                   "Unsupported dtype of input dense data for training!\n");
        }
      }
      else REPORT_ERROR(USER_ERROR, 
           "Frovedis doesn't support input sparse data for Random Forest training!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  // --- (22) GBT ---
  void gbt_trainer(const char* host, int port, long xptr, long yptr, 
                   const char* algo, const char* loss, const char *impurity, 
                   double learning_rate,
                   int max_depth, double min_impurity_decrease,
                   int random_state, double tol, int max_bins,
                   double subsampling_rate, 
                   const char* feature_subset_strategy, 
                   double feature_subset_rate,
                   int n_estimators, int nclasses,
                   int verbose, int mid, 
                   short dtype, short itype, bool dense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);
    auto algorithm = tree::algorithm::Regression;
    if(std::string(algo) == "Classification") 
      algorithm = tree::algorithm::Classification;
    else algorithm = tree::algorithm::Regression;
    auto impurity_ = std::string(impurity);
    auto feature_subset_strategy_ = std::string(feature_subset_strategy);
    auto loss_ = std::string(loss);
    bool mvbl = false; // auto-managed at python side
    if (nclasses > 2) 
      REPORT_ERROR(USER_ERROR, 
      "Currently frovedis GBTClassifier supports only binary classification!\n");
    try {
      if(dense) {
        switch(dtype) {
          case FLOAT: { 
            auto min_impurity_decrease_float = static_cast<float>(min_impurity_decrease);
            auto subsampling_rate_float = static_cast<float>(subsampling_rate);
            auto feature_subset_rate_float = static_cast<float>(feature_subset_rate);
            tree::sampling_strategy<float> subsample_strat;
            subsample_strat.set_subsampling_rate(subsampling_rate_float)
                           .set_feature_subset_strategy(feature_subset_strategy_)
                           .set_feature_subset_rate(feature_subset_rate_float);
            // tol wont be used right now
            tree::strategy<float> strat;
            strat.set_algorithm(algorithm)
                 .set_impurity_type(impurity_)
                 .set_loss_type(loss_)
                 .set_learning_rate(learning_rate)
                 .set_max_depth(max_depth)
                 .set_min_info_gain(min_impurity_decrease_float)
                 .set_max_bins(max_bins)
                 .set_num_iterations(n_estimators)
                 .set_sampling_strategy(std::move(subsample_strat));
            if(std::string(algo) == "Classification") 
              strat.set_num_classes(nclasses);
            if(random_state != -1)
              strat.set_seed(random_state);
            exrpc_oneway(fm_node,(frovedis_gbt<DT2,D_MAT2>),f_dptr,strat,verbose,mid,mvbl);
            break;
          }
          case DOUBLE: {
            tree::sampling_strategy<double> subsample_strat;
            subsample_strat.set_subsampling_rate(subsampling_rate)
                           .set_feature_subset_strategy(feature_subset_strategy_)
                           .set_feature_subset_rate(feature_subset_rate);
            // tol wont be used right now
            tree::strategy<double> strat;
            strat.set_algorithm(algorithm)
                 .set_impurity_type(impurity_)
                 .set_loss_type(loss_)
                 .set_learning_rate(learning_rate)
                 .set_max_depth(max_depth)
                 .set_min_info_gain(min_impurity_decrease)
                 .set_max_bins(max_bins)
                 .set_num_iterations(n_estimators)
                 .set_sampling_strategy(std::move(subsample_strat));
            if(std::string(algo) == "Classification") 
              strat.set_num_classes(nclasses);
            if(random_state != -1)
              strat.set_seed(random_state);
            exrpc_oneway(fm_node,(frovedis_gbt<DT1,D_MAT1>),f_dptr,strat,verbose,mid,mvbl);
            break;
          }
          default: REPORT_ERROR(USER_ERROR, 
                   "Unsupported dtype of input dense data for training!\n");
        }
      }
      else REPORT_ERROR(USER_ERROR, 
           "Frovedis doesn't support input sparse data for GBT training!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

}
