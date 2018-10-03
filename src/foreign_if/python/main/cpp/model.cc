#include "python_client_headers.hpp"
#include "exrpc_model.hpp"
#include "short_hand_dense_type.hpp"
#include "short_hand_sparse_type.hpp"
#include "short_hand_model_type.hpp"

extern "C" {

  void show_frovedis_model(const char* host, int port, 
                           int mid, short mkind, short mdtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    try {
      if (mdtype == FLOAT) {
        switch(mkind) {
          case LRM:    exrpc_oneway(fm_node,show_model<LRM2>,mid); break;
          case SVM:    exrpc_oneway(fm_node,show_model<SVM2>,mid); break;
          case LNRM:   exrpc_oneway(fm_node,show_model<LNRM2>,mid); break;
          case MFM:    exrpc_oneway(fm_node,show_model<MFM2>,mid); break;
          case KMEANS: exrpc_oneway(fm_node,show_model<KMM2>,mid); break;
          case DTM:    exrpc_oneway(fm_node,show_model<DTM2>,mid); break;
          case FMM:    REPORT_ERROR(USER_ERROR,"currently Frovedis fm_model can't be displayed!");
          case NBM:    exrpc_oneway(fm_node,show_model<NBM2>,mid); break;
          default:     REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
        }
      }
      else if (mdtype == DOUBLE) {
        switch(mkind) {
          case LRM:    exrpc_oneway(fm_node,show_model<LRM1>,mid); break;
          case SVM:    exrpc_oneway(fm_node,show_model<SVM1>,mid); break;
          case LNRM:   exrpc_oneway(fm_node,show_model<LNRM1>,mid); break;
          case MFM:    exrpc_oneway(fm_node,show_model<MFM1>,mid); break;
          case KMEANS: exrpc_oneway(fm_node,show_model<KMM1>,mid); break;
          case DTM:    exrpc_oneway(fm_node,show_model<DTM1>,mid); break;
          case FMM:    REPORT_ERROR(USER_ERROR,"currently Frovedis fm_model can't be displayed!");
          case NBM:    exrpc_oneway(fm_node,show_model<NBM1>,mid); break;
          default:     REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
        }
      }
      else REPORT_ERROR(USER_ERROR,"model dtype can either be float or double!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  void release_frovedis_model(const char* host, int port, 
                              int mid, short mkind, short mdtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    try {
      if (mdtype == FLOAT) {
        switch(mkind) {
          case LRM:    exrpc_oneway(fm_node,release_model<LRM2>,mid); break;
          case SVM:    exrpc_oneway(fm_node,release_model<SVM2>,mid); break;
          case LNRM:   exrpc_oneway(fm_node,release_model<LNRM2>,mid);break;
          case MFM:    exrpc_oneway(fm_node,release_model<MFM2>,mid); break;
          case KMEANS: exrpc_oneway(fm_node,release_model<KMM2>,mid); break;
          case DTM:    exrpc_oneway(fm_node,release_model<DTM2>,mid); break;
          case FMM:    exrpc_oneway(fm_node,release_model<FMM2>,mid); break;
          case NBM:    exrpc_oneway(fm_node,release_model<NBM2>,mid); break;
          default:     REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
        }
      }
      else if (mdtype == DOUBLE) {
        switch(mkind) {
          case LRM:    exrpc_oneway(fm_node,release_model<LRM1>,mid); break;
          case SVM:    exrpc_oneway(fm_node,release_model<SVM1>,mid); break;
          case LNRM:   exrpc_oneway(fm_node,release_model<LNRM1>,mid);break;
          case MFM:    exrpc_oneway(fm_node,release_model<MFM1>,mid); break;
          case KMEANS: exrpc_oneway(fm_node,release_model<KMM1>,mid); break;
          case DTM:    exrpc_oneway(fm_node,release_model<DTM1>,mid); break;
          case FMM:    exrpc_oneway(fm_node,release_model<FMM1>,mid); break;
          case NBM:    exrpc_oneway(fm_node,release_model<NBM1>,mid); break;
          default:     REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
        }
      }
      else REPORT_ERROR(USER_ERROR,"model dtype can either be float or double!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  void save_frovedis_model(const char* host, int port, 
                           int mid, short mkind, 
                           short mdtype, const char* path) {
    ASSERT_PTR(path);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    std::string fs_path(path);
    try {
      if (mdtype == FLOAT) {
        switch(mkind) {
          case LRM:    exrpc_oneway(fm_node,save_model<LRM2>,mid,fs_path); break;
          case SVM:    exrpc_oneway(fm_node,save_model<SVM2>,mid,fs_path); break;
          case LNRM:   exrpc_oneway(fm_node,save_model<LNRM2>,mid,fs_path); break;
          case MFM:    exrpc_oneway(fm_node,save_model<MFM2>,mid,fs_path); break;
          case KMEANS: exrpc_oneway(fm_node,save_model<KMM2>,mid,fs_path); break;
          case DTM:    exrpc_oneway(fm_node,save_model<DTM2>,mid,fs_path); break;
          case FMM:    exrpc_oneway(fm_node,save_fmm<DT2>,mid,fs_path); break;
          case NBM:    exrpc_oneway(fm_node,save_model<NBM2>,mid,fs_path); break;
          default:     REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
        }
      }
      else if (mdtype == DOUBLE) {
        switch(mkind) {
          case LRM:    exrpc_oneway(fm_node,save_model<LRM1>,mid,fs_path); break;
          case SVM:    exrpc_oneway(fm_node,save_model<SVM1>,mid,fs_path); break;
          case LNRM:   exrpc_oneway(fm_node,save_model<LNRM1>,mid,fs_path); break;
          case MFM:    exrpc_oneway(fm_node,save_model<MFM1>,mid,fs_path); break;
          case KMEANS: exrpc_oneway(fm_node,save_model<KMM1>,mid,fs_path); break;
          case DTM:    exrpc_oneway(fm_node,save_model<DTM1>,mid,fs_path); break;
          case FMM:    exrpc_oneway(fm_node,save_fmm<DT1>,mid,fs_path); break;
          case NBM:    exrpc_oneway(fm_node,save_model<NBM1>,mid,fs_path); break;
          default:     REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
        }
      }
      else REPORT_ERROR(USER_ERROR,"model dtype can either be float or double!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  // TODO: handle return values for linear models
  void load_frovedis_model(const char* host, int port, 
                           int mid, short mkind, 
                           short mdtype, const char* path) {
    ASSERT_PTR(path);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    std::string fs_path(path);
    try {
      if (mdtype == FLOAT) {
        switch(mkind) {
          case LRM:    exrpc_async(fm_node,load_glm<LRM2>,mid,LRM,fs_path).get(); break;
          case SVM:    exrpc_async(fm_node,load_glm<SVM2>,mid,SVM,fs_path).get(); break;
          case LNRM:   exrpc_async(fm_node,load_lnrm<DT2>,mid,LNRM,fs_path).get(); break;
          case KMEANS: exrpc_async(fm_node,load_kmm<DT2>,mid,KMEANS,fs_path).get(); break;
          case DTM:    exrpc_oneway(fm_node,load_model<DTM2>,mid,DTM,fs_path); break;
          case FMM:    REPORT_ERROR(USER_ERROR,"currently frovedis fm_model can't be loaded!");
          default:   REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
        }
      }
      else if (mdtype == DOUBLE) {
        switch(mkind) {
          case LRM:    exrpc_async(fm_node,load_glm<LRM1>,mid,LRM,fs_path).get(); break;
          case SVM:    exrpc_async(fm_node,load_glm<SVM1>,mid,SVM,fs_path).get(); break;
          case LNRM:   exrpc_async(fm_node,load_lnrm<DT1>,mid,LNRM,fs_path).get(); break;
          case KMEANS: exrpc_async(fm_node,load_kmm<DT1>,mid,KMEANS,fs_path).get(); break;
          case DTM:    exrpc_oneway(fm_node,load_model<DTM1>,mid,DTM,fs_path); break;
          case FMM:    REPORT_ERROR(USER_ERROR,"currently frovedis fm_model can't be loaded!");
          default:   REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
        }
      }
      else REPORT_ERROR(USER_ERROR,"model dtype can either be float or double!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  PyObject* load_frovedis_nbm(const char* host, int port,
                              int mid, short mdtype, const char* path) {
    ASSERT_PTR(path);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    std::string fs_path(path);
    std::string ret = " ";
    try {
      switch(mdtype) {
        case FLOAT: 
          ret = exrpc_async(fm_node,load_nbm<DT2>,mid,NBM,fs_path).get();
          break;
        case DOUBLE:
          ret = exrpc_async(fm_node,load_nbm<DT1>,mid,NBM,fs_path).get();
          break;
        default: REPORT_ERROR(USER_ERROR,"model dtype can either be float or double!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_python_string_object(ret);
  }

  PyObject* load_frovedis_mfm(const char* host, int port,
                            int mid, short mdtype, const char* path) {
    ASSERT_PTR(path);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    std::string fs_path(path);
    dummy_mfm ret;
    try {
      switch(mdtype) {
        case FLOAT:
          ret = exrpc_async(fm_node,load_mfm<DT2>,mid,MFM,fs_path).get();
          break;
        case DOUBLE:
          ret = exrpc_async(fm_node,load_mfm<DT1>,mid,MFM,fs_path).get();
          break;
        default: REPORT_ERROR(USER_ERROR,"model dtype can either be float or double!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_mfm_info(ret);
  }

  void parallel_float_glm_predict(const char* host, int port,
                                  int mid, short mkind, long dptr, 
                                  float* ret, ulong len, bool prob,
                                  short itype, bool isDense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    std::vector<float> pred;
    
    try {
      if(isDense) {
        switch(mkind) {
          case LRM:  pred = exrpc_async(fm_node,(pgp2<DT2,R_MAT2,R_LMAT2,LRM2>),f_dptr,mid,prob).get(); break;
          case SVM:  pred = exrpc_async(fm_node,(pgp2<DT2,R_MAT2,R_LMAT2,SVM2>),f_dptr,mid,prob).get(); break;
          case LNRM: pred = exrpc_async(fm_node,(p_lnrm_p2<DT2,R_MAT2,R_LMAT2>),f_dptr,mid).get(); break;
          case DTM:  pred = exrpc_async(fm_node,(parallel_dtm_predict_with_broadcast<DT2,R_MAT2,R_LMAT2>),
                                        f_dptr,mid,prob).get(); break;
          case FMM:  REPORT_ERROR(USER_ERROR,"currently Frovedis doesn't support dense test data for FM!\n");
          case NBM:  pred = exrpc_async(fm_node,(parallel_nbm_predict_with_broadcast<DT2,R_MAT2,R_LMAT2>),
                                        f_dptr,mid,prob).get(); break;
          default:   REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
        }
      }
      else {
        switch(mkind) {
          case LRM: { 
            if(itype == INT)
              pred = exrpc_async(fm_node,(pgp2<DT2,S_MAT24,S_LMAT24,LRM2>),f_dptr,mid,prob).get(); 
            else if(itype == LONG)
              pred = exrpc_async(fm_node,(pgp2<DT2,S_MAT25,S_LMAT25,LRM2>),f_dptr,mid,prob).get(); 
            else REPORT_ERROR(USER_ERROR,"Unsupported itype for sparse data!\n");
            break;
          }
          case SVM: {
            if(itype == INT)
              pred = exrpc_async(fm_node,(pgp2<DT2,S_MAT24,S_LMAT24,SVM2>),f_dptr,mid,prob).get(); 
            else if(itype == LONG)
              pred = exrpc_async(fm_node,(pgp2<DT2,S_MAT25,S_LMAT25,SVM2>),f_dptr,mid,prob).get();
            else REPORT_ERROR(USER_ERROR,"Unsupported itype for sparse data!\n");
            break;
          }
          case LNRM: {
            if(itype == INT)
               pred = exrpc_async(fm_node,(p_lnrm_p2<DT2,S_MAT24,S_LMAT24>),f_dptr,mid).get(); 
            else if(itype == LONG)
               pred = exrpc_async(fm_node,(p_lnrm_p2<DT2,S_MAT25,S_LMAT25>),f_dptr,mid).get(); 
            else REPORT_ERROR(USER_ERROR,"Unsupported itype for sparse data!\n");
            break;
          }
          case DTM: {
            if(itype == INT)
               pred = exrpc_async(fm_node,(parallel_dtm_predict_with_broadcast<DT2,S_MAT24,S_LMAT24>),f_dptr,mid,prob).get();  
            else if(itype == LONG)
               pred = exrpc_async(fm_node,(parallel_dtm_predict_with_broadcast<DT2,S_MAT25,S_LMAT25>),f_dptr,mid,prob).get(); 
            else REPORT_ERROR(USER_ERROR,"Unsupported itype for sparse data!\n");
            break;
          }
          case FMM: {
            if(itype == INT)
              pred = exrpc_async(fm_node,(parallel_fmm_predict_with_broadcast<DT2,S_MAT24,S_LMAT24>),f_dptr,mid).get();
            else if(itype == LONG)
              pred = exrpc_async(fm_node,(parallel_fmm_predict_with_broadcast<DT2,S_MAT25,S_LMAT25>),f_dptr,mid).get();
            else REPORT_ERROR(USER_ERROR,"Unsupported itype for sparse data!\n");
            break;
          }
          case NBM: {
            if(itype == INT)
              pred = exrpc_async(fm_node,(parallel_nbm_predict_with_broadcast<DT2,S_MAT24,S_LMAT24>),f_dptr,mid,prob).get(); 
            else if(itype == LONG)
              pred = exrpc_async(fm_node,(parallel_nbm_predict_with_broadcast<DT2,S_MAT25,S_LMAT25>),f_dptr,mid,prob).get(); 
           else REPORT_ERROR(USER_ERROR,"Unsupported itype for sparse data!\n");
           break;
          }
          default:   REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
        }
      }
      auto sz = pred.size();
      checkAssumption(len == sz);
      for(size_t i=0; i<sz; ++i) ret[i] = pred[i];
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  void parallel_double_glm_predict(const char* host, int port,
                                   int mid, short mkind, long dptr, 
                                   double* ret, ulong len, bool prob, short itype , bool isDense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    std::vector<double> pred;
    try {
      if(isDense) {
        switch(mkind) {
          case LRM:  pred = exrpc_async(fm_node,(pgp2<DT1,R_MAT1,R_LMAT1,LRM1>),f_dptr,mid,prob).get(); break;
          case SVM:  pred = exrpc_async(fm_node,(pgp2<DT1,R_MAT1,R_LMAT1,SVM1>),f_dptr,mid,prob).get(); break;
          case LNRM: pred = exrpc_async(fm_node,(p_lnrm_p2<DT1,R_MAT1,R_LMAT1>),f_dptr,mid).get(); break;
          case DTM:  pred = exrpc_async(fm_node,(parallel_dtm_predict_with_broadcast<DT1,R_MAT1,R_LMAT1>),
                                        f_dptr,mid,prob).get(); break;
          case FMM:  REPORT_ERROR(USER_ERROR,"currently Frovedis doesn't support dense test data for FM!\n");
          case NBM:  pred = exrpc_async(fm_node,(parallel_nbm_predict_with_broadcast<DT1,R_MAT1,R_LMAT1>),
                                        f_dptr,mid,prob).get(); break;
          default:   REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
        }
      }
      else {
        switch(mkind) {
          case LRM: {
            if(itype == INT)
              pred = exrpc_async(fm_node,(pgp2<DT1,S_MAT14,S_LMAT14,LRM1>),f_dptr,mid,prob).get();
            else if(itype == LONG)
              pred = exrpc_async(fm_node,(pgp2<DT1,S_MAT15,S_LMAT15,LRM1>),f_dptr,mid,prob).get();
            else REPORT_ERROR(USER_ERROR,"Unsupported itype for sparse data!\n");
            break;
          }
          case SVM: {
            if(itype == INT)
              pred = exrpc_async(fm_node,(pgp2<DT1,S_MAT14,S_LMAT14,SVM1>),f_dptr,mid,prob).get();
            else if(itype == LONG)
              pred = exrpc_async(fm_node,(pgp2<DT1,S_MAT15,S_LMAT15,SVM1>),f_dptr,mid,prob).get();
            else REPORT_ERROR(USER_ERROR,"Unsupported itype for sparse data!\n");
            break;
          }
          case LNRM: {
            if(itype == INT)
               pred = exrpc_async(fm_node,(p_lnrm_p2<DT1,S_MAT14,S_LMAT14>),f_dptr,mid).get();
            else if(itype == LONG)
               pred = exrpc_async(fm_node,(p_lnrm_p2<DT1,S_MAT15,S_LMAT15>),f_dptr,mid).get();
            else REPORT_ERROR(USER_ERROR,"Unsupported itype for sparse data!\n");
            break;
          }
          case DTM: {
            if(itype == INT)
               pred = exrpc_async(fm_node,(parallel_dtm_predict_with_broadcast<DT1,S_MAT14,S_LMAT14>),f_dptr,mid,prob).get();
             else if(itype == LONG)
               pred = exrpc_async(fm_node,(parallel_dtm_predict_with_broadcast<DT1,S_MAT15,S_LMAT15>),f_dptr,mid,prob).get();
             else REPORT_ERROR(USER_ERROR,"Unsupported itype for sparse data!\n");
             break;
          }
          case FMM: {
            if(itype == INT)
              pred = exrpc_async(fm_node,(parallel_fmm_predict_with_broadcast<DT1,S_MAT14,S_LMAT14>),f_dptr,mid).get();
            else if(itype == LONG)
              pred = exrpc_async(fm_node,(parallel_fmm_predict_with_broadcast<DT1,S_MAT15,S_LMAT15>),f_dptr,mid).get();
            else REPORT_ERROR(USER_ERROR,"Unsupported itype for sparse data!\n");
            break;
          }
          case NBM: {
            if(itype == INT)
               pred = exrpc_async(fm_node,(parallel_nbm_predict_with_broadcast<DT1,S_MAT14,S_LMAT14>),f_dptr,mid,prob).get();
            else if(itype == LONG)
               pred = exrpc_async(fm_node,(parallel_nbm_predict_with_broadcast<DT1,S_MAT15,S_LMAT15>),f_dptr,mid,prob).get();
            else REPORT_ERROR(USER_ERROR,"Unsupported itype for sparse data!\n");
            break;
          }
          default:   REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
        }
      }
      auto sz = pred.size();
      checkAssumption(len == sz);
      for(size_t i=0; i<sz; ++i) ret[i] = pred[i];
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  void parallel_kmeans_predict(const char* host, int port,
                               int mid, short mdtype, 
                               long dptr, int* ret, ulong len, short itype , bool isDense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    std::vector<int> pred;
    try {
      if(isDense){
        switch(mdtype) {
          case FLOAT:
            pred = exrpc_async(fm_node,(pkp2<R_MAT2,R_LMAT2,KMM2>),f_dptr,mid).get();
            break;
          case DOUBLE:
            pred = exrpc_async(fm_node,(pkp2<R_MAT1,R_LMAT1,KMM1>),f_dptr,mid).get();
            break;
          default: REPORT_ERROR(USER_ERROR, "model dtype can be either float or double!\n");
         }
      }
      else {
        if(mdtype == FLOAT){
          switch(itype) {
            case INT:  
              pred = exrpc_async(fm_node,(pkp2<S_MAT24,S_LMAT24,KMM2>),f_dptr,mid).get();
              break;
            case LONG: 
              pred = exrpc_async(fm_node,(pkp2<S_MAT25,S_LMAT25,KMM2>),f_dptr,mid).get();
              break;
            default: REPORT_ERROR(USER_ERROR, "model itype can be either int or long!\n");
          }
        }
        else if(mdtype == DOUBLE){
          switch(itype) {
            case INT:
               pred = exrpc_async(fm_node,(pkp2<S_MAT14,S_LMAT14,KMM1>),f_dptr,mid).get();
               break;
            case LONG:
               pred = exrpc_async(fm_node,(pkp2<S_MAT15,S_LMAT15,KMM1>),f_dptr,mid).get();
               break;
            default: REPORT_ERROR(USER_ERROR, "model itype can be either int or long!\n");
           }
         } 
        else REPORT_ERROR(USER_ERROR, "model dtype can be either float or double!\n");
      }
      auto sz = pred.size();
      checkAssumption(len == sz);
      for(size_t i=0; i<sz; ++i) ret[i] = pred[i];
    }  
    catch (std::exception& e) {
      set_status(true, e.what());
    } 
  }

  void als_float_predict(const char* host, int port, int mid, 
                         int* ids, float* ret, ulong sz) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    std::vector<std::pair<size_t,size_t>> ID(sz);
    for(size_t i=0; i<sz; ++i) {
      ID[i] = std::make_pair(ids[2*i],ids[2*i+1]);
    }
    std::vector<float> pd;
    try {
      pd = exrpc_async(fm_node,frovedis_als_predict<DT2>,mid,ID).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    for(size_t i=0; i<sz; ++i) ret[i] = pd[i]; 
  }

  void als_double_predict(const char* host, int port, int mid, 
                          int* ids, double* ret, ulong sz) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    std::vector<std::pair<size_t,size_t>> ID(sz);
    for(size_t i=0; i<sz; ++i) {
      ID[i] = std::make_pair(ids[2*i],ids[2*i+1]);
    }
    std::vector<double> pd;
    try {
      pd = exrpc_async(fm_node,frovedis_als_predict<DT1>,mid,ID).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    for(size_t i=0; i<sz; ++i) ret[i] = pd[i]; 
  }

  void als_float_rec_users(const char* host, int port, int mid, 
                           int pid, int k,
                           int* uids, float* scores) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    std::vector<std::pair<size_t,float>> pd;
    try {
      pd = exrpc_async(fm_node,recommend_users<DT2>,mid,pid,k).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    for(size_t i=0; i<k; ++i) {
      uids[i] = pd[i].first;
      scores[i] = pd[i].second;
    }
  }

  void als_double_rec_users(const char* host, int port, int mid, 
                            int pid, int k, 
                            int* uids, double* scores) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    std::vector<std::pair<size_t,double>> pd;
    try {
      pd = exrpc_async(fm_node,recommend_users<DT1>,mid,pid,k).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    for(size_t i=0; i<k; ++i) {
      uids[i] = pd[i].first;
      scores[i] = pd[i].second;
    }
  }

  void als_float_rec_prods(const char* host, int port, 
                           int mid, int uid, int k, 
                           int* pids, float* scores) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    std::vector<std::pair<size_t,float>> pd;
    try {
      pd = exrpc_async(fm_node,recommend_products<DT2>,mid,uid,k).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    for(size_t i=0; i<k; ++i) {
      pids[i] = pd[i].first;
      scores[i] = pd[i].second;
    }
  }

  void als_double_rec_prods(const char* host, int port, 
                            int mid, int uid, int k,
                            int* pids, double* scores) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    std::vector<std::pair<size_t,double>> pd;
    try {
      pd = exrpc_async(fm_node,recommend_products<DT1>,mid,uid,k).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    for(size_t i=0; i<k; ++i) {
      pids[i] = pd[i].first;
      scores[i] = pd[i].second;
    }
  }

}
