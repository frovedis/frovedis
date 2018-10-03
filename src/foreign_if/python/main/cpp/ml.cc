#include "python_client_headers.hpp"
#include "exrpc_ml.hpp"
#include "short_hand_dense_type.hpp"
#include "short_hand_sparse_type.hpp"

extern "C" {

  // --- (1) Logistic Regression ---
  void lr_sgd(const char* host, int port, long xptr, long yptr,
              int iter, double al, int rtype, bool icpt, double tol,
              int vb, int mid, short dtype, short itype, bool dense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);
    double mbf = 1.0;   // default
    double rprm = 0.01; // default
    bool mvbl = false; // auto-managed at python side
    try {
      if(dense) {
        switch(dtype) {
          case FLOAT:  
	    exrpc_oneway(fm_node,(frovedis_lr_sgd<DT2,D_MAT2>),f_dptr,iter,al,mbf,rtype,rprm,icpt,tol,vb,mid,mvbl); 
            break;
          case DOUBLE: 
	    exrpc_oneway(fm_node,(frovedis_lr_sgd<DT1,D_MAT1>),f_dptr,iter,al,mbf,rtype,rprm,icpt,tol,vb,mid,mvbl); 
            break;
          default: REPORT_ERROR(USER_ERROR, "Unsupported dtype of input dense data for training!\n");
        }
      }
      else {
        switch(dtype) {
          case FLOAT: 
            if(itype == INT) 
              exrpc_oneway(fm_node,(frovedis_lr_sgd<DT2,S_MAT24>),f_dptr,iter,al,mbf,rtype,rprm,icpt,tol,vb,mid,mvbl);
            else if(itype == LONG) 
              exrpc_oneway(fm_node,(frovedis_lr_sgd<DT2,S_MAT25>),f_dptr,iter,al,mbf,rtype,rprm,icpt,tol,vb,mid,mvbl);          
            else REPORT_ERROR(USER_ERROR, "Unsupported itype of input sparse data for training!\n");
            break;
          case DOUBLE: 
            if(itype == INT) 
              exrpc_oneway(fm_node,(frovedis_lr_sgd<DT1,S_MAT14>),f_dptr,iter,al,mbf,rtype,rprm,icpt,tol,vb,mid,mvbl);
            else if(itype == LONG) 
              exrpc_oneway(fm_node,(frovedis_lr_sgd<DT1,S_MAT15>),f_dptr,iter,al,mbf,rtype,rprm,icpt,tol,vb,mid,mvbl);          
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

  void lr_lbfgs(const char* host, int port, long xptr, long yptr,
                int iter, double al, int rtype, bool icpt, double tol,
                int vb, int mid, short dtype, short itype, bool dense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);
    int hs = 10;   // default
    double rprm = 0.01; // default
    bool mvbl = false; // auto-managed at python side
    try {
      if(dense) {
        switch(dtype) {
          case FLOAT:
            exrpc_oneway(fm_node,(frovedis_lr_lbfgs<DT2,D_MAT2>),f_dptr,iter,al,hs,rtype,rprm,icpt,tol,vb,mid,mvbl);
            break;
          case DOUBLE:
            exrpc_oneway(fm_node,(frovedis_lr_lbfgs<DT1,D_MAT1>),f_dptr,iter,al,hs,rtype,rprm,icpt,tol,vb,mid,mvbl);
            break;
          default: REPORT_ERROR(USER_ERROR, "Unsupported dtype of input dense data for training!\n");
        }
      }
      else {
        switch(dtype) {
          case FLOAT:
            if(itype == INT)
              exrpc_oneway(fm_node,(frovedis_lr_lbfgs<DT2,S_MAT24>),f_dptr,iter,al,hs,rtype,rprm,icpt,tol,vb,mid,mvbl);
            else if(itype == LONG)
              exrpc_oneway(fm_node,(frovedis_lr_lbfgs<DT2,S_MAT25>),f_dptr,iter,al,hs,rtype,rprm,icpt,tol,vb,mid,mvbl);
            else REPORT_ERROR(USER_ERROR, "Unsupported itype of input sparse data for training!\n");
            break;
          case DOUBLE:
            if(itype == INT)
              exrpc_oneway(fm_node,(frovedis_lr_lbfgs<DT1,S_MAT14>),f_dptr,iter,al,hs,rtype,rprm,icpt,tol,vb,mid,mvbl);
            else if(itype == LONG)
              exrpc_oneway(fm_node,(frovedis_lr_lbfgs<DT1,S_MAT15>),f_dptr,iter,al,hs,rtype,rprm,icpt,tol,vb,mid,mvbl);
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

  // --- (2) Linear SVM ---
  void svm_sgd(const char* host, int port, long xptr, long yptr,
               int iter, double al, int rtype, bool icpt, double tol,
               int vb, int mid, short dtype, short itype, bool dense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);
    double mbf = 1.0;   // default
    double rprm = 0.01; // default
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
                 int iter, double al, int rtype, bool icpt, double tol,
                 int vb, int mid, short dtype, short itype, bool dense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);
    int hs = 10;   // default
    double rprm = 0.01; // default
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
 
  // --- (3) Linear Regression ---
  void lnr_sgd(const char* host, int port, long xptr, long yptr,
               bool icpt, int vb, int mid, short dtype, short itype, bool dense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);
    int iter = 1000;  //default
    double al = 0.01;  //default
    double mbf = 1.0;  // default
    bool mvbl = false;  // auto-managed at python side
    try {
      if(dense) {
        switch(dtype) {
          case FLOAT:
            exrpc_oneway(fm_node,(frovedis_lnr_sgd<DT2,D_MAT2>),f_dptr,iter,al,mbf,icpt,vb,mid,mvbl);
            break;
          case DOUBLE:
            exrpc_oneway(fm_node,(frovedis_lnr_sgd<DT1,D_MAT1>),f_dptr,iter,al,mbf,icpt,vb,mid,mvbl);
            break;
          default: REPORT_ERROR(USER_ERROR, "Unsupported dtype of input dense data for training!\n");
        }
      }
      else {
        switch(dtype) {
          case FLOAT:
            if(itype == INT)
              exrpc_oneway(fm_node,(frovedis_lnr_sgd<DT2,S_MAT24>),f_dptr,iter,al,mbf,icpt,vb,mid,mvbl);
            else if(itype == LONG)
              exrpc_oneway(fm_node,(frovedis_lnr_sgd<DT2,S_MAT25>),f_dptr,iter,al,mbf,icpt,vb,mid,mvbl);
            else REPORT_ERROR(USER_ERROR, "Unsupported itype of input sparse data for training!\n");
            break;
          case DOUBLE:
            if(itype == INT)
              exrpc_oneway(fm_node,(frovedis_lnr_sgd<DT1,S_MAT14>),f_dptr,iter,al,mbf,icpt,vb,mid,mvbl);
            else if(itype == LONG)
              exrpc_oneway(fm_node,(frovedis_lnr_sgd<DT1,S_MAT15>),f_dptr,iter,al,mbf,icpt,vb,mid,mvbl);
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
                 bool icpt, int vb, int mid, short dtype, short itype, bool dense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);
    int iter = 1000;  //default
    double al = 0.01;  //default
    int hs = 10;  // default
    bool mvbl = false; // auto-managed at python side
    try {
      if(dense) {
        switch(dtype) {
          case FLOAT:
            exrpc_oneway(fm_node,(frovedis_lnr_lbfgs<DT2,D_MAT2>),f_dptr,iter,al,hs,icpt,vb,mid,mvbl);
            break;
          case DOUBLE:
            exrpc_oneway(fm_node,(frovedis_lnr_lbfgs<DT1,D_MAT1>),f_dptr,iter,al,hs,icpt,vb,mid,mvbl);
            break;
          default: REPORT_ERROR(USER_ERROR, "Unsupported dtype of input dense data for training!\n");
        }
      }
      else {
        switch(dtype) {
          case FLOAT:
            if(itype == INT)
              exrpc_oneway(fm_node,(frovedis_lnr_lbfgs<DT2,S_MAT24>),f_dptr,iter,al,hs,icpt,vb,mid,mvbl);
            else if(itype == LONG)
              exrpc_oneway(fm_node,(frovedis_lnr_lbfgs<DT2,S_MAT25>),f_dptr,iter,al,hs,icpt,vb,mid,mvbl);
            else REPORT_ERROR(USER_ERROR, "Unsupported itype of input sparse data for training!\n");
            break;
          case DOUBLE:
            if(itype == INT)
              exrpc_oneway(fm_node,(frovedis_lnr_lbfgs<DT1,S_MAT14>),f_dptr,iter,al,hs,icpt,vb,mid,mvbl);
            else if(itype == LONG)
              exrpc_oneway(fm_node,(frovedis_lnr_lbfgs<DT1,S_MAT15>),f_dptr,iter,al,hs,icpt,vb,mid,mvbl);
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
                 int iter, double al, bool icpt, double tol,
                 int vb, int mid, short dtype, short itype, bool dense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);
    double mbf = 1.0;   // default
    double rprm = 0.01; // default
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
                   int iter, double al, bool icpt, double tol,
                   int vb, int mid, short dtype, short itype, bool dense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);
    int hs = 10;   // default
    double rprm = 0.01; // default
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
                 int iter, double al, bool icpt, double tol,
                 int vb, int mid, short dtype, short itype, bool dense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);
    double mbf = 1.0;   // default
    double rprm = 0.01; // default
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
                   int iter, double al, bool icpt, double tol,
                   int vb, int mid, short dtype, short itype, bool dense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    auto f_yptr = (exrpc_ptr_t) yptr;
    auto f_dptr = frovedis_mem_pair(f_xptr,f_yptr);
    int hs = 10;   // default
    double rprm = 0.01; // default
    bool mvbl = false; // auto-managed at python side
    try {
      if(dense) {
        switch(dtype) {
          case FLOAT:
            exrpc_oneway(fm_node,(frovedis_ridge_lbfgs<DT2,D_MAT2>),f_dptr,iter,al,hs,rprm,icpt,tol,vb,mid,mvbl);
            break;
          case DOUBLE:
            exrpc_oneway(fm_node,(frovedis_ridge_lbfgs<DT1,D_MAT1>),f_dptr,iter,al,hs,rprm,icpt,tol,vb,mid,mvbl);
            break;
          default: REPORT_ERROR(USER_ERROR, "Unsupported dtype of input dense data for training!\n");
        }
      }
      else {
        switch(dtype) {
          case FLOAT:
            if(itype == INT)
              exrpc_oneway(fm_node,(frovedis_ridge_lbfgs<DT2,S_MAT24>),f_dptr,iter,al,hs,rprm,icpt,tol,vb,mid,mvbl);
            else if(itype == LONG)
              exrpc_oneway(fm_node,(frovedis_ridge_lbfgs<DT2,S_MAT25>),f_dptr,iter,al,hs,rprm,icpt,tol,vb,mid,mvbl);
            else REPORT_ERROR(USER_ERROR, "Unsupported itype of input sparse data for training!\n");
            break;
          case DOUBLE:
            if(itype == INT)
              exrpc_oneway(fm_node,(frovedis_ridge_lbfgs<DT1,S_MAT14>),f_dptr,iter,al,hs,rprm,icpt,tol,vb,mid,mvbl);
            else if(itype == LONG)
              exrpc_oneway(fm_node,(frovedis_ridge_lbfgs<DT1,S_MAT15>),f_dptr,iter,al,hs,rprm,icpt,tol,vb,mid,mvbl);
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

  // --- (6) Kmeans ---
  void kmeans_train(const char* host, int port, long xptr, int k,
                    int iter, long seed, double eps,
                    int vb, int mid, 
                    short dtype, short itype, bool dense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_xptr = (exrpc_ptr_t) xptr;
    bool mvbl = false; // auto-managed at python side
    try {
      if(dense) {
        switch(dtype) {
          case FLOAT: 
            exrpc_oneway(fm_node,(frovedis_kmeans<DT2,R_MAT2>),f_xptr,k,iter,seed,eps,vb,mid,mvbl);
            break;
          case DOUBLE: 
            exrpc_oneway(fm_node,(frovedis_kmeans<DT1,R_MAT1>),f_xptr,k,iter,seed,eps,vb,mid,mvbl);
            break;
          default: REPORT_ERROR(USER_ERROR, "Unsupported dtype of input dense data for training!\n");
        }
      }
      else {
        switch(dtype) {
          case FLOAT: 
            if(itype == INT) 
              exrpc_oneway(fm_node,(frovedis_kmeans<DT2,S_MAT24>),f_xptr,k,iter,seed,eps,vb,mid,mvbl);
            else if(itype == LONG) 
              exrpc_oneway(fm_node,(frovedis_kmeans<DT2,S_MAT25>),f_xptr,k,iter,seed,eps,vb,mid,mvbl);
            else REPORT_ERROR(USER_ERROR, "Unsupported itype of input sparse data for training!\n");
            break;
          case DOUBLE: 
            if(itype == INT) 
              exrpc_oneway(fm_node,(frovedis_kmeans<DT1,S_MAT14>),f_xptr,k,iter,seed,eps,vb,mid,mvbl);
            else if(itype == LONG) 
              exrpc_oneway(fm_node,(frovedis_kmeans<DT1,S_MAT15>),f_xptr,k,iter,seed,eps,vb,mid,mvbl);
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

  // --- (7) Matrix Factorization using ALS ---
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
            exrpc_oneway(fm_node,(frovedis_mf_als<DT2,S_MAT24>),f_dptr,rank,iter,al,rprm,seed,vb,mid,mvbl);
          else if(itype == LONG) 
            exrpc_oneway(fm_node,(frovedis_mf_als<DT2,S_MAT25>),f_dptr,rank,iter,al,rprm,seed,vb,mid,mvbl);
          else REPORT_ERROR(USER_ERROR, "Unsupported itype of input sparse data for training!\n");
          break;
        case DOUBLE:
          if(itype == INT) 
            exrpc_oneway(fm_node,(frovedis_mf_als<DT1,S_MAT14>),f_dptr,rank,iter,al,rprm,seed,vb,mid,mvbl);
          else if(itype == LONG) 
            exrpc_oneway(fm_node,(frovedis_mf_als<DT1,S_MAT15>),f_dptr,rank,iter,al,rprm,seed,vb,mid,mvbl);
          else REPORT_ERROR(USER_ERROR, "Unsupported itype of input sparse data for training!\n");
          break;
        default: REPORT_ERROR(USER_ERROR, "Unsupported dtype of input sparse data for training!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  // --- (8) Naive Bayes ---
  void nb_trainer(const char* host, int port, long xptr,
                 long yptr, double alpha, int mid,
                 const char* algo, int verbose, 
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
            exrpc_oneway(fm_node,(frovedis_nb<DT2,D_MAT2,D_LMAT2>),f_dptr,algos,alpha,verbose,mid,mvbl);
            break;
          case DOUBLE:
            exrpc_oneway(fm_node,(frovedis_nb<DT1,D_MAT1,D_LMAT1>),f_dptr,algos,alpha,verbose,mid,mvbl);
            break;
          default: REPORT_ERROR(USER_ERROR, "Unsupported dtype of input dense data for training!\n");
        }
      }
      else {
        switch(dtype) {
          case FLOAT:
            if(itype == INT)
              exrpc_oneway(fm_node,(frovedis_nb<DT2,S_MAT24,S_LMAT24>),f_dptr,algos,alpha,verbose,mid,mvbl);
            else if(itype == LONG)
              exrpc_oneway(fm_node,(frovedis_nb<DT2,S_MAT25,S_LMAT25>),f_dptr,algos,alpha,verbose,mid,mvbl);
            else REPORT_ERROR(USER_ERROR, "Unsupported itype of input sparse data for training!\n");
            break;
          case DOUBLE:
            if(itype == INT)
              exrpc_oneway(fm_node,(frovedis_nb<DT1,S_MAT14,S_LMAT14>),f_dptr,algos,alpha,verbose,mid,mvbl);
            else if(itype == LONG)
              exrpc_oneway(fm_node,(frovedis_nb<DT1,S_MAT15,S_LMAT15>),f_dptr,algos,alpha,verbose,mid,mvbl);
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

  // --- (9) Factorization Machine ---
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

  // --- (10) Decision Tree ---
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
}
