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
          case MLR:    exrpc_oneway(fm_node,show_model<MLR2>,mid); break;
          case SVM:    exrpc_oneway(fm_node,show_model<SVM2>,mid); break;
          case LNRM:   exrpc_oneway(fm_node,show_model<LNRM2>,mid); break;
          case MFM:    exrpc_oneway(fm_node,show_model<MFM2>,mid); break;
          case KMEANS: exrpc_oneway(fm_node,show_model<KMM2>,mid); break;
          case ACM:    exrpc_oneway(fm_node,show_model<ACM2>,mid); break;
          case SEM:    exrpc_oneway(fm_node,show_model<SEM2>,mid); break;
          case SCM:    exrpc_oneway(fm_node,show_model<SCM2>,mid); break;
          //case DBSCAN: exrpc_oneway(fm_node,show_model<DBM2>,mid); break; 
          case DTM:    exrpc_oneway(fm_node,show_model<DTM2>,mid); break;
          case FPM:    exrpc_oneway(fm_node,show_model<FPM1>,mid); break; // not template based
          case FPR:    exrpc_oneway(fm_node, show_model<FPR1>, mid); break;
          case FMM:    REPORT_ERROR(USER_ERROR,"currently Frovedis fm_model can't be displayed!");
          case NBM:    exrpc_oneway(fm_node,show_model<NBM2>,mid); break;
          case RFM:    exrpc_oneway(fm_node,show_model<RFM2>,mid); break;
          case GBT:    exrpc_oneway(fm_node,show_model<GBT2>,mid); break;
          default:     REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
        }
      }
      else if (mdtype == DOUBLE) {
        switch(mkind) {
          case LRM:    exrpc_oneway(fm_node,show_model<LRM1>,mid); break;
          case MLR:    exrpc_oneway(fm_node,show_model<MLR1>,mid); break;
          case SVM:    exrpc_oneway(fm_node,show_model<SVM1>,mid); break;
          case LNRM:   exrpc_oneway(fm_node,show_model<LNRM1>,mid); break;
          case MFM:    exrpc_oneway(fm_node,show_model<MFM1>,mid); break;
          case KMEANS: exrpc_oneway(fm_node,show_model<KMM1>,mid); break;
          case ACM:    exrpc_oneway(fm_node,show_model<ACM1>,mid); break;
          case SEM:    exrpc_oneway(fm_node,show_model<SEM1>,mid); break;
          case SCM:    exrpc_oneway(fm_node,show_model<SCM1>,mid); break;
          //case DBSCAN: exrpc_oneway(fm_node,show_model<DBM1>,mid); break; 
          case DTM:    exrpc_oneway(fm_node,show_model<DTM1>,mid); break;
          case FPM:    exrpc_oneway(fm_node,show_model<FPM1>,mid); break; // not template based
          case FPR:    exrpc_oneway(fm_node, show_model<FPR1>, mid); break;
          case FMM:    REPORT_ERROR(USER_ERROR,"currently Frovedis fm_model can't be displayed!");
          case NBM:    exrpc_oneway(fm_node,show_model<NBM1>,mid); break;
          case RFM:    exrpc_oneway(fm_node,show_model<RFM1>,mid); break;
          case GBT:    exrpc_oneway(fm_node,show_model<GBT1>,mid); break;
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
          case MLR:    exrpc_oneway(fm_node,release_model<MLR2>,mid); break;
          case SVM:    exrpc_oneway(fm_node,release_model<SVM2>,mid); break;
          case LNRM:   exrpc_oneway(fm_node,release_model<LNRM2>,mid);break;
          case MFM:    exrpc_oneway(fm_node,release_model<MFM2>,mid); break;
          case KMEANS: exrpc_oneway(fm_node,release_model<KMM2>,mid); break;
          case ACM:    exrpc_oneway(fm_node,release_model<ACM2>,mid); break;
          case SEM:    exrpc_oneway(fm_node,release_model<SEM2>,mid); break;
          case SCM:    exrpc_oneway(fm_node,release_model<SCM2>,mid); break;
          case DBSCAN: exrpc_oneway(fm_node,release_model<DBSCAN1>,mid); break; // not template based
          case DTM:    exrpc_oneway(fm_node,release_model<DTM2>,mid); break;
          case FPM:    exrpc_oneway(fm_node,release_model<FPM1>,mid); break; // not template based
          case FPR:    exrpc_oneway(fm_node, release_model<FPR1>, mid); break;
          case FMM:    exrpc_oneway(fm_node,release_model<FMM2>,mid); break;
          case NBM:    exrpc_oneway(fm_node,release_model<NBM2>,mid); break;
          case KNN:    exrpc_oneway(fm_node,release_model<KNN2>,mid); break;
          case KNR:    exrpc_oneway(fm_node,release_model<KNR2>,mid); break;
          case KNC:    exrpc_oneway(fm_node,release_model<KNC2>,mid); break;
          case RFM:    exrpc_oneway(fm_node,release_model<RFM2>,mid); break;
          case GBT:    exrpc_oneway(fm_node,release_model<GBT2>,mid); break;
          default:     REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
        }
      }
      else if (mdtype == DOUBLE) {
        switch(mkind) {
          case LRM:    exrpc_oneway(fm_node,release_model<LRM1>,mid); break;
          case MLR:    exrpc_oneway(fm_node,release_model<MLR1>,mid); break;
          case SVM:    exrpc_oneway(fm_node,release_model<SVM1>,mid); break;
          case LNRM:   exrpc_oneway(fm_node,release_model<LNRM1>,mid);break;
          case MFM:    exrpc_oneway(fm_node,release_model<MFM1>,mid); break;
          case DTM:    exrpc_oneway(fm_node,release_model<DTM1>,mid); break;
          case KMEANS: exrpc_oneway(fm_node,release_model<KMM1>,mid); break;
          case ACM:    exrpc_oneway(fm_node,release_model<ACM1>,mid); break;
          case SEM:    exrpc_oneway(fm_node,release_model<SEM1>,mid); break;
          case SCM:    exrpc_oneway(fm_node,release_model<SCM1>,mid); break;
          case DBSCAN: exrpc_oneway(fm_node,release_model<DBSCAN1>,mid); break; // not template based
          case FPM:    exrpc_oneway(fm_node,release_model<FPM1>,mid); break; // not template based
          case FPR:    exrpc_oneway(fm_node, release_model<FPR1>, mid); break;
          case FMM:    exrpc_oneway(fm_node,release_model<FMM1>,mid); break;
          case NBM:    exrpc_oneway(fm_node,release_model<NBM1>,mid); break;
          case KNN:    exrpc_oneway(fm_node,release_model<KNN1>,mid); break;
          case KNR:    exrpc_oneway(fm_node,release_model<KNR1>,mid); break;
          case KNC:    exrpc_oneway(fm_node,release_model<KNC1>,mid); break;
          case RFM:    exrpc_oneway(fm_node,release_model<RFM1>,mid); break;
          case GBT:    exrpc_oneway(fm_node,release_model<GBT1>,mid); break;
          default:     REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
        }
      }
      else if (mdtype == INT){
        switch(mkind) {
          case LDA:    exrpc_oneway(fm_node,release_model<LDA4>,mid); break;
          default:     REPORT_ERROR(USER_ERROR,
                       "Unknown Model Kind of int mdtype is encountered!\n");
        }
      }
      else if (mdtype == LONG){
        switch(mkind) {
          case LDA:    exrpc_oneway(fm_node,release_model<LDA3>,mid); break;
          default:     REPORT_ERROR(USER_ERROR,
                       "Unknown Model Kind of int mdtype is encountered!\n");
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
          case MLR:    exrpc_oneway(fm_node,save_model<MLR2>,mid,fs_path); break;
          case SVM:    exrpc_oneway(fm_node,save_model<SVM2>,mid,fs_path); break;
          case LNRM:   exrpc_oneway(fm_node,save_model<LNRM2>,mid,fs_path); break;
          case MFM:    exrpc_oneway(fm_node,save_model<MFM2>,mid,fs_path); break;
          case KMEANS: exrpc_oneway(fm_node,save_model<KMM2>,mid,fs_path); break;
          case ACM:    exrpc_oneway(fm_node,save_model<ACM2>,mid,fs_path); break;
          case SEM:    exrpc_oneway(fm_node,save_model<SEM2>,mid,fs_path); break;
          case SCM:    exrpc_oneway(fm_node,save_model<SCM2>,mid,fs_path); break;
          case DTM:    exrpc_oneway(fm_node,save_model<DTM2>,mid,fs_path); break;
          case FPM:    exrpc_oneway(fm_node,save_model<FPM1>,mid,fs_path); break; // not template based
          case FPR:    exrpc_oneway(fm_node,save_model<FPR1>,mid,fs_path); break;
          case FMM:    exrpc_oneway(fm_node,save_fmm<DT2>,mid,fs_path); break;
          case NBM:    exrpc_oneway(fm_node,save_model<NBM2>,mid,fs_path); break;
          case RFM:    exrpc_oneway(fm_node,save_model<RFM2>,mid,fs_path); break;
          case GBT:    exrpc_oneway(fm_node,save_model<GBT2>,mid,fs_path); break;
          default:     REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
        }
      }
      else if (mdtype == DOUBLE) {
        switch(mkind) {
          case LRM:    exrpc_oneway(fm_node,save_model<LRM1>,mid,fs_path); break;
          case MLR:    exrpc_oneway(fm_node,save_model<MLR1>,mid,fs_path); break;
          case SVM:    exrpc_oneway(fm_node,save_model<SVM1>,mid,fs_path); break;
          case LNRM:   exrpc_oneway(fm_node,save_model<LNRM1>,mid,fs_path); break;
          case MFM:    exrpc_oneway(fm_node,save_model<MFM1>,mid,fs_path); break;
          case KMEANS: exrpc_oneway(fm_node,save_model<KMM1>,mid,fs_path); break;
          case ACM:    exrpc_oneway(fm_node,save_model<ACM1>,mid,fs_path); break;
          case SEM:    exrpc_oneway(fm_node,save_model<SEM1>,mid,fs_path); break;
          case SCM:    exrpc_oneway(fm_node,save_model<SCM1>,mid,fs_path); break;
          case DTM:    exrpc_oneway(fm_node,save_model<DTM1>,mid,fs_path); break;
          case FPR:    exrpc_oneway(fm_node,save_model<FPR1>,mid,fs_path); break;
          case FPM:    exrpc_oneway(fm_node,save_model<FPM1>,mid,fs_path); break; // not template based
          case FMM:    exrpc_oneway(fm_node,save_fmm<DT1>,mid,fs_path); break;
          case NBM:    exrpc_oneway(fm_node,save_model<NBM1>,mid,fs_path); break;
          case RFM:    exrpc_oneway(fm_node,save_model<RFM1>,mid,fs_path); break;
          case GBT:    exrpc_oneway(fm_node,save_model<GBT1>,mid,fs_path); break;
          default:     REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
        }
      }
      else if (mdtype == INT) {
        switch(mkind) {
          case LDA:  exrpc_oneway(fm_node,save_model<LDA4>,mid,fs_path); break;
          default:   REPORT_ERROR(USER_ERROR,
                     "Unknown Model Kind for int mdtype is encountered!\n");
        }
      }
      else if (mdtype == LONG) {
        switch(mkind) {
          case LDA:  exrpc_oneway(fm_node,save_model<LDA3>,mid,fs_path); break;
          default:   REPORT_ERROR(USER_ERROR,
                     "Unknown Model Kind for long mdtype is encountered!\n");
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
          case MLR:    exrpc_async(fm_node,load_glm<MLR2>,mid,MLR,fs_path).get(); break;
          case LRM:    exrpc_async(fm_node,load_glm<LRM2>,mid,LRM,fs_path).get(); break;
          case SVM:    exrpc_async(fm_node,load_glm<SVM2>,mid,SVM,fs_path).get(); break;
          case LNRM:   exrpc_async(fm_node,load_lnrm<DT2>,mid,LNRM,fs_path).get(); break;
          case KMEANS: exrpc_async(fm_node,load_kmm<DT2>,mid,KMEANS,fs_path).get(); break;
          case SEM:    exrpc_oneway(fm_node,load_model<SEM2>,mid,SEM,fs_path); break;
          case DTM:    exrpc_oneway(fm_node,load_model<DTM2>,mid,DTM,fs_path); break;
          case FMM:    REPORT_ERROR(USER_ERROR,"currently frovedis fm_model can't be loaded!");
          case FPM:    exrpc_oneway(fm_node,load_model<FPM1>,mid,FPM,fs_path); break; // not template based
          case FPR:  exrpc_oneway(fm_node,load_model<FPR1>,mid,FPM,fs_path); break;
          case RFM:    exrpc_oneway(fm_node,load_model<RFM2>,mid,RFM,fs_path); break;
          case GBT:    exrpc_oneway(fm_node,load_model<GBT2>,mid,GBT,fs_path); break;
          default:   REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
        }
      }
      else if (mdtype == DOUBLE) {
        switch(mkind) {
          case MLR:    exrpc_async(fm_node,load_glm<MLR1>,mid,MLR,fs_path).get(); break;
          case LRM:    exrpc_async(fm_node,load_glm<LRM1>,mid,LRM,fs_path).get(); break;; 
          case SVM:    exrpc_async(fm_node,load_glm<SVM1>,mid,SVM,fs_path).get(); break;
          case LNRM:   exrpc_async(fm_node,load_lnrm<DT1>,mid,LNRM,fs_path).get(); break;
          case KMEANS: exrpc_async(fm_node,load_kmm<DT1>,mid,KMEANS,fs_path).get(); break;
          case SEM:    exrpc_oneway(fm_node,load_model<SEM1>,mid,SEM,fs_path); break;
          case DTM:    exrpc_oneway(fm_node,load_model<DTM1>,mid,DTM,fs_path); break;
          case FPR:    exrpc_oneway(fm_node,load_model<FPR1>,mid,FPM,fs_path); break;
          case FMM:    REPORT_ERROR(USER_ERROR,"currently frovedis fm_model can't be loaded!");
          case FPM:    exrpc_oneway(fm_node,load_model<FPM1>,mid,FPM,fs_path); break; // not template based
          case RFM:    exrpc_oneway(fm_node,load_model<RFM1>,mid,RFM,fs_path); break;
          case GBT:    exrpc_oneway(fm_node,load_model<GBT1>,mid,GBT,fs_path); break;
          default:   REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
        }
      }
      else if (mdtype == INT) {
        switch(mkind) {
          case LDA:  exrpc_oneway(fm_node,load_model<LDA4>,mid,LDA,fs_path); break;
          default:   REPORT_ERROR(USER_ERROR,
                     "Unknown Model Kind for int mdtype is encountered!\n");
        }
      }
      else if (mdtype == LONG) {
        switch(mkind) {
          case LDA:  exrpc_oneway(fm_node,load_model<LDA3>,mid,LDA,fs_path); break;
          default:   REPORT_ERROR(USER_ERROR,
                     "Unknown Model Kind for long mdtype is encountered!\n");
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
  
   PyObject* load_frovedis_scm(const char* host, int port,
                              int mid, short mdtype, const char* path) {
    ASSERT_PTR(path);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    std::string fs_path(path);
    std::vector<int> ret;
    try {
      switch(mdtype) {
        case FLOAT:
          ret = exrpc_async(fm_node,load_scm<DT2>,mid,fs_path).get();
          break;
        case DOUBLE:
          ret = exrpc_async(fm_node,load_scm<DT1>,mid,fs_path).get();
          break;
        default: REPORT_ERROR(USER_ERROR,"model dtype can either be float or double!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_python_int_list(ret);
  }

  void acm_predict(const char* host, int port,
                   int mid, short mdtype, int ncluster, 
                   int* ret, long ret_len) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    std::vector<int> label;
    try {
      switch(mdtype) {
        case FLOAT:
          label = exrpc_async(fm_node,frovedis_acm_pred<DT2>,mid,ncluster).get(); break;
        case DOUBLE:
          label = exrpc_async(fm_node,frovedis_acm_pred<DT1>,mid,ncluster).get(); break;
        default: REPORT_ERROR(USER_ERROR,"model dtype can either be float or double!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    checkAssumption(label.size() == ret_len);
    for(size_t i = 0; i < ret_len; ++i) ret[i] = label[i];
  }

  int load_frovedis_acm(const char* host, int port,
                         int mid, short mdtype, const char* path) {
    ASSERT_PTR(path);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    std::string fs_path(path);
    int nsamples = 0;
    try {
      switch(mdtype) {
        case FLOAT:
          nsamples = exrpc_async(fm_node,load_acm<DT2>,mid,fs_path).get(); break;
        case DOUBLE:
          nsamples = exrpc_async(fm_node,load_acm<DT1>,mid,fs_path).get(); break;
        default: REPORT_ERROR(USER_ERROR,"model dtype can either be float or double!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return nsamples;
  }

  PyObject* get_scm_aff_matrix(const char* host, int port,
                               int mid, short mdtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    dummy_matrix dmat;
    try {
      switch(mdtype) {
        case FLOAT:
          dmat = exrpc_async(fm_node,get_scm_affinity_matrix<DT2>,mid).get();
          break;
        case DOUBLE:
          dmat = exrpc_async(fm_node,get_scm_affinity_matrix<DT1>,mid).get();
          break;
        default: REPORT_ERROR(USER_ERROR,"model dtype can either be float or double!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_matrix(dmat);
  }

   PyObject* get_sem_aff_matrix(const char* host, int port,
                                int mid, short mdtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    dummy_matrix dmat;
    try {
      switch(mdtype) {
        case FLOAT:
          dmat = exrpc_async(fm_node,get_sem_affinity_matrix<DT2>,mid).get();
          break;
        case DOUBLE:
          dmat = exrpc_async(fm_node,get_sem_affinity_matrix<DT1>,mid).get();
          break;
        default: REPORT_ERROR(USER_ERROR,"model dtype can either be float or double!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_matrix(dmat);
  }

   PyObject* get_sem_embed_matrix(const char* host, int port,
                                  int mid, short mdtype) {
    exrpc_node fm_node(host,port);
    dummy_matrix dmat;
    try {
      switch(mdtype) {
        case FLOAT:
          dmat = exrpc_async(fm_node,get_sem_embedding_matrix<DT2>,mid).get();
          break;
        case DOUBLE:
          dmat = exrpc_async(fm_node,get_sem_embedding_matrix<DT1>,mid).get();
          break;
        default: REPORT_ERROR(USER_ERROR,"model dtype can either be float or double!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_matrix(dmat);
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

  PyObject* get_frovedis_weight_vector(const char* host, int port,
                                       int mid, short mkind, 
                                       short mdtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host, port);
    PyObject* ret_ptr = NULL;
    try { 
      if (mdtype == FLOAT) {
        std::vector<float> ret;
        switch(mkind) {
          case LRM:  ret = exrpc_async(fm_node, (get_weight_vector<DT2,LRM2>), mid).get(); break;
          case MLR:  ret = exrpc_async(fm_node, (get_weight_as_vector<DT2,MLR2>), mid).get(); break;
          case LNRM: ret = exrpc_async(fm_node, (get_weight_vector<DT2,LNRM2>), mid).get(); break;
          case SVM:  ret = exrpc_async(fm_node, (get_weight_vector<DT2,SVM2>), mid).get(); break;
          default: REPORT_ERROR(USER_ERROR, "Unknown model for weight vector extraction!\n");
        }
        ret_ptr = to_python_float_list(ret);
      }
      else if (mdtype == DOUBLE) {
        std::vector<double> ret;
        switch(mkind) {
          case LRM:  ret = exrpc_async(fm_node, (get_weight_vector<DT1,LRM1>), mid).get(); break;
          case MLR:  ret = exrpc_async(fm_node, (get_weight_as_vector<DT1,MLR1>), mid).get(); break;
          case LNRM: ret = exrpc_async(fm_node, (get_weight_vector<DT1,LNRM1>), mid).get(); break;
          case SVM:  ret = exrpc_async(fm_node, (get_weight_vector<DT1,SVM1>), mid).get(); break;
          default: REPORT_ERROR(USER_ERROR, "Unknown model for weight vector extraction!\n");
        }
        ret_ptr = to_python_double_list(ret);
      }
      else REPORT_ERROR(USER_ERROR,"model dtype can either be float or double!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return ret_ptr;
  }

  PyObject* get_frovedis_intercept_vector(const char* host, int port,
                                          int mid, short mkind, 
                                          short mdtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host, port);
    PyObject* ret_ptr = NULL;
    try { 
      if (mdtype == FLOAT) {
        std::vector<float> ret;
        switch(mkind) {
          case LRM:  ret = exrpc_async(fm_node, (get_intercept_as_vector<DT2,LRM2>), mid).get(); break;
          case MLR:  ret = exrpc_async(fm_node, (get_intercept_vector<DT2,MLR2>), mid).get(); break;
          case LNRM: ret = exrpc_async(fm_node, (get_intercept_as_vector<DT2,LNRM2>), mid).get(); break;
          case SVM:  ret = exrpc_async(fm_node, (get_intercept_as_vector<DT2,SVM2>), mid).get(); break;
          default: REPORT_ERROR(USER_ERROR, "Unknown model for intercept vector extraction!\n");
        }
        ret_ptr = to_python_float_list(ret);
      }
      else if (mdtype == DOUBLE) {
        std::vector<double> ret;
        switch(mkind) {
          case LRM:  ret = exrpc_async(fm_node, (get_intercept_as_vector<DT1,LRM1>), mid).get(); break;
          case MLR:  ret = exrpc_async(fm_node, (get_intercept_vector<DT1,MLR1>), mid).get(); break;
          case LNRM: ret = exrpc_async(fm_node, (get_intercept_as_vector<DT1,LNRM1>), mid).get(); break;
          case SVM:  ret = exrpc_async(fm_node, (get_intercept_as_vector<DT1,SVM1>), mid).get(); break;
          default: REPORT_ERROR(USER_ERROR, "Unknown model for intercept vector extraction!\n");
        }
        ret_ptr = to_python_double_list(ret);
      }
      else REPORT_ERROR(USER_ERROR,"model dtype can either be float or double!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return ret_ptr;
  }

  PyObject* get_frovedis_pi_vector(const char* host, int port,
                                   int mid, short mkind,
                                   short mdtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host, port);
    PyObject* ret_ptr = NULL;
    try {
      if (mdtype == FLOAT) {
        std::vector<float> ret;
        switch(mkind) {
          case NBM: ret = exrpc_async(fm_node, (get_pi_vector<DT2,NBM2>), mid).get(); break;
          default: REPORT_ERROR(USER_ERROR, "Unknown model for class_log_prior vector extraction!\n");
        }
        ret_ptr = to_python_float_list(ret);
      }
      else if (mdtype == DOUBLE) {
        std::vector<double> ret;
        switch(mkind) {
          case NBM: ret = exrpc_async(fm_node, (get_pi_vector<DT1,NBM1>), mid).get(); break;
          default: REPORT_ERROR(USER_ERROR, "Unknown model for class_log_prior vector extraction!\n");
        }
        ret_ptr = to_python_double_list(ret);
      }
      else REPORT_ERROR(USER_ERROR,"model dtype can either be float or double!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return ret_ptr;
  }

  PyObject* get_frovedis_theta_vector(const char* host, int port,
                                      int mid, short mkind,
                                      short mdtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host, port);
    PyObject* ret_ptr = NULL;
    try {
      if (mdtype == FLOAT) {
        std::vector<float> ret;
        switch(mkind) {
          case NBM: ret = exrpc_async(fm_node, (get_theta_vector<DT2,NBM2>), mid).get(); break;
          default: REPORT_ERROR(USER_ERROR, "Unknown model for feature_log_prob vector extraction!\n");
        }
        ret_ptr = to_python_float_list(ret);
      }
      else if (mdtype == DOUBLE) {
        std::vector<double> ret;
        switch(mkind) {
          case NBM: ret = exrpc_async(fm_node, (get_theta_vector<DT1,NBM1>), mid).get(); break;
          default: REPORT_ERROR(USER_ERROR, "Unknown model for feature_log_prob vector extraction!\n");
        }
        ret_ptr = to_python_double_list(ret);
      }
      else REPORT_ERROR(USER_ERROR,"model dtype can either be float or double!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return ret_ptr;
  }

  PyObject* get_frovedis_cls_counts_vector(const char* host, int port,
                                           int mid, short mkind,
                                           short mdtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host, port);
    std::vector<size_t> ret;
    try {
      if (mdtype == FLOAT) {
        switch(mkind) {
          case NBM: ret = exrpc_async(fm_node, (get_cls_counts_vector<DT5,NBM2>), mid).get(); break;
          default: REPORT_ERROR(USER_ERROR, "Unknown model for class_count vector extraction!\n");
        }
      }
      else if (mdtype == DOUBLE) {
        switch(mkind) {
          case NBM: ret = exrpc_async(fm_node, (get_cls_counts_vector<DT5,NBM1>), mid).get(); break;
          default: REPORT_ERROR(USER_ERROR, "Unknown model for class_count vector extraction!\n");
        }
      }
      else REPORT_ERROR(USER_ERROR,"model dtype can either be float or double!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_python_llong_list(ret);
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
          case MLR:  pred = exrpc_async(fm_node,(pgp2<DT2,R_MAT2,R_LMAT2,MLR2>),f_dptr,mid,prob).get(); break;
          case SVM:  pred = exrpc_async(fm_node,(pgp2<DT2,R_MAT2,R_LMAT2,SVM2>),f_dptr,mid,prob).get(); break;
          case LNRM: pred = exrpc_async(fm_node,(p_lnrm_p2<DT2,R_MAT2,R_LMAT2>),f_dptr,mid).get(); break;
          case DTM:  pred = exrpc_async(fm_node,(parallel_dtm_predict_with_broadcast<DT2,R_MAT2,R_LMAT2>),
                                        f_dptr,mid,prob).get(); break;
          case FMM:  REPORT_ERROR(USER_ERROR,"currently Frovedis doesn't support dense test data for FM!\n");
          case NBM:  pred = exrpc_async(fm_node,(parallel_nbm_predict_with_broadcast<DT2,R_MAT2,R_LMAT2>),
                                        f_dptr,mid,prob).get(); break;
          case RFM:  pred = exrpc_async(fm_node,(parallel_rfm_predict_with_broadcast<DT2,R_MAT2,R_LMAT2>),
                                        f_dptr,mid,prob).get(); break;
          case GBT:  pred = exrpc_async(fm_node,(parallel_gbt_predict_with_broadcast<DT2,R_MAT2,R_LMAT2>),
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
          case MLR: {
            if(itype == INT)
              pred = exrpc_async(fm_node,(pgp2<DT2,S_MAT24,S_LMAT24,MLR2>),f_dptr,mid,prob).get();
            else if(itype == LONG)
              pred = exrpc_async(fm_node,(pgp2<DT2,S_MAT25,S_LMAT25,MLR2>),f_dptr,mid,prob).get();
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
          case MLR:  pred = exrpc_async(fm_node,(pgp2<DT1,R_MAT1,R_LMAT1,MLR1>),f_dptr,mid,prob).get(); break;
          case SVM:  pred = exrpc_async(fm_node,(pgp2<DT1,R_MAT1,R_LMAT1,SVM1>),f_dptr,mid,prob).get(); break;
          case LNRM: pred = exrpc_async(fm_node,(p_lnrm_p2<DT1,R_MAT1,R_LMAT1>),f_dptr,mid).get(); break;
          case DTM:  pred = exrpc_async(fm_node,(parallel_dtm_predict_with_broadcast<DT1,R_MAT1,R_LMAT1>),
                                        f_dptr,mid,prob).get(); break;
          case FMM:  REPORT_ERROR(USER_ERROR,"currently Frovedis doesn't support dense test data for FM!\n");
          case NBM:  pred = exrpc_async(fm_node,(parallel_nbm_predict_with_broadcast<DT1,R_MAT1,R_LMAT1>),
                                        f_dptr,mid,prob).get(); break;
          case RFM:  pred = exrpc_async(fm_node,(parallel_rfm_predict_with_broadcast<DT1,R_MAT1,R_LMAT1>),
                                        f_dptr,mid,prob).get(); break;
          case GBT:  pred = exrpc_async(fm_node,(parallel_gbt_predict_with_broadcast<DT1,R_MAT1,R_LMAT1>),
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
          case MLR: {
            if(itype == INT)
              pred = exrpc_async(fm_node,(pgp2<DT1,S_MAT14,S_LMAT14,MLR1>),f_dptr,mid,prob).get();
            else if(itype == LONG)
              pred = exrpc_async(fm_node,(pgp2<DT1,S_MAT15,S_LMAT15, MLR1>),f_dptr,mid,prob).get();
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

  PyObject* knn_kneighbors(const char* host, int port,
                           long tptr, int k, int mid,
                           bool need_distance, short dtype) { // short itype, bool dense
      if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
      exrpc_node fm_node(host,port);
      auto test_dptr = (exrpc_ptr_t) tptr;
      knn_result res;
      try {
        switch(dtype) {
          case FLOAT:
            res = exrpc_async(fm_node, (frovedis_kneighbors<DT2,DT5,R_MAT2,KNN2>), test_dptr, 
                              mid, k, need_distance ).get();
            break;
          case DOUBLE:
            res = exrpc_async(fm_node, (frovedis_kneighbors<DT1,DT5,R_MAT1,KNN1>), test_dptr, 
                              mid, k, need_distance ).get();
            break;
          default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input dense matrix!\n");
        }
      }
      catch (std::exception& e) {
        set_status(true, e.what());
      }

      char mtype = 'R'; // indices, distances => rowmajor matrix
      return to_py_knn_result(res, mtype);
    }

  // knc
  PyObject* knc_kneighbors(const char* host, int port,
                           long tptr, int k, int mid,
                           bool need_distance, short dtype) { 
      if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
      exrpc_node fm_node(host,port);
      auto test_dptr = (exrpc_ptr_t) tptr;
      knn_result res;
      try {
        switch(dtype) {
          case FLOAT:
            res = exrpc_async(fm_node, (frovedis_kneighbors<DT2,DT5,R_MAT2,KNC2>), test_dptr, 
                              mid, k, need_distance ).get();
            break;
          case DOUBLE:
            res = exrpc_async(fm_node, (frovedis_kneighbors<DT1,DT5,R_MAT1,KNC1>), test_dptr, 
                              mid, k, need_distance ).get();
            break;
          default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input dense matrix!\n");
        }
      }
      catch (std::exception& e) {
        set_status(true, e.what());
      }

      char mtype = 'R'; // indices, distances => rowmajor matrix
      return to_py_knn_result(res, mtype);
    }

  // knr
  PyObject* knr_kneighbors(const char* host, int port,
                           long tptr, int k, int mid,
                           bool need_distance, short dtype) { 
      if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
      exrpc_node fm_node(host,port);
      auto test_dptr = (exrpc_ptr_t) tptr;
      knn_result res;
      try {
        switch(dtype) {
          case FLOAT:
            res = exrpc_async(fm_node, (frovedis_kneighbors<DT2,DT5,R_MAT2,KNR2>), test_dptr, 
                              mid, k, need_distance ).get();
            break;
          case DOUBLE:
            res = exrpc_async(fm_node, (frovedis_kneighbors<DT1,DT5,R_MAT1,KNR1>), test_dptr, 
                              mid, k, need_distance ).get();
            break;
          default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input dense matrix!\n");
        }
      }
      catch (std::exception& e) {
        set_status(true, e.what());
      }

      char mtype = 'R'; // indices, distances => rowmajor matrix
      return to_py_knn_result(res, mtype);
    }


  PyObject* knn_kneighbors_graph(const char* host, int port,
                                 long tptr, int k, int mid,
                                 const char* mode, short dtype) { // itype, dense
      if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
      exrpc_node fm_node(host,port);
      auto test_dptr = (exrpc_ptr_t) tptr;
      std::string mode_ = mode;
      dummy_matrix dmat;
      try {
        switch(dtype) {
          case FLOAT:
            dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,R_MAT2,KNN2,S_MAT25,S_LMAT25>), test_dptr, 
                              mid, k, mode_).get();
            break;
          case DOUBLE:
            dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,R_MAT1,KNN1,S_MAT15,S_LMAT15>), test_dptr, 
                              mid, k, mode_).get();
            break;
          default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input dense matrix!\n");
        }
      }
      catch (std::exception& e) {
        set_status(true, e.what());
      }
      return to_py_dummy_matrix(dmat);
    }
  
  // knc
  PyObject* knc_kneighbors_graph(const char* host, int port,
                                 long tptr, int k, int mid,
                                 const char* mode, short dtype) { 
      if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
      exrpc_node fm_node(host,port);
      auto test_dptr = (exrpc_ptr_t) tptr;
      std::string mode_ = mode;
      dummy_matrix dmat;
      try {
        switch(dtype) {
          case FLOAT:
            dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,R_MAT2,KNC2,S_MAT25,S_LMAT25>), test_dptr, 
                              mid, k, mode_).get();
            break;
          case DOUBLE:
            dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,R_MAT1,KNC1,S_MAT15,S_LMAT15>), test_dptr, 
                              mid, k, mode_).get();
            break;
          default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input dense matrix!\n");
        }
      }
      catch (std::exception& e) {
        set_status(true, e.what());
      }
      return to_py_dummy_matrix(dmat);
    }

  // knr
  PyObject* knr_kneighbors_graph(const char* host, int port,
                                 long tptr, int k, int mid,
                                 const char* mode, short dtype) {
      if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
      exrpc_node fm_node(host,port);
      auto test_dptr = (exrpc_ptr_t) tptr;
      std::string mode_ = mode;
      dummy_matrix dmat;
      try {
        switch(dtype) {
          case FLOAT:
            dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,R_MAT2,KNR2,S_MAT25,S_LMAT25>), test_dptr, 
                              mid, k, mode_).get();
            break;
          case DOUBLE:
            dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,R_MAT1,KNR1,S_MAT15,S_LMAT15>), test_dptr, 
                              mid, k, mode_).get();
            break;
          default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input dense matrix!\n");
        }
      }
      catch (std::exception& e) {
        set_status(true, e.what());
      }
      return to_py_dummy_matrix(dmat);
    }

  PyObject* knn_radius_neighbors(const char* host, int port,
                                 long tptr, float radius, int mid,
                                 bool need_distance, short dtype) { // itype, dense
      if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
      exrpc_node fm_node(host,port);
      auto test_dptr = (exrpc_ptr_t) tptr;
      dummy_matrix dmat;
      try {
        switch(dtype) {
          case FLOAT:
            dmat = exrpc_async(fm_node, (frovedis_radius_neighbors<DT5,R_MAT2,KNN2,S_MAT25,S_LMAT25>), test_dptr, 
                              mid, radius, need_distance ).get();
            break;
          case DOUBLE:
            dmat = exrpc_async(fm_node, (frovedis_radius_neighbors<DT5,R_MAT1,KNN1,S_MAT15,S_LMAT15>), test_dptr, 
                              mid, radius, need_distance ).get();
            break;
          default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input dense matrix!\n");
        }
      }
      catch (std::exception& e) {
        set_status(true, e.what());
      }
      return to_py_dummy_matrix(dmat);
    }

  PyObject* knn_radius_neighbors_graph(const char* host, int port,
                                       long tptr, float radius, int mid,
                                       const char* mode, short dtype) { // itype, dense
      if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
      exrpc_node fm_node(host,port);
      auto test_dptr = (exrpc_ptr_t) tptr;
      std::string mode_ = mode;
      dummy_matrix dmat;
      try {
        switch(dtype) {
          case FLOAT:
            dmat = exrpc_async(fm_node, (frovedis_radius_neighbors_graph<DT5,R_MAT2,KNN2,S_MAT25,S_LMAT25>), test_dptr, 
                              mid, radius, mode_).get();
            break;
          case DOUBLE:
            dmat = exrpc_async(fm_node, (frovedis_radius_neighbors_graph<DT5,R_MAT1,KNN1,S_MAT15,S_LMAT15>), test_dptr, 
                              mid, radius, mode_ ).get();
            break;
          default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input dense matrix!\n");
        }
      }
      catch (std::exception& e) {
        set_status(true, e.what());
      }
      return to_py_dummy_matrix(dmat);
    }

  // knc and knr predict
  void knc_double_predict(const char* host, int port, long tptr,
                          int mid, bool save_proba, 
                          double* ret, long ret_len) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto test_dptr = (exrpc_ptr_t) tptr;
    std::vector<double> label;
    try {
      label = exrpc_async(fm_node, (frovedis_knc_predict<DT1,DT5,R_MAT1,KNC1>), test_dptr, mid, save_proba).get(); 
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    checkAssumption(label.size() == ret_len);
    for(size_t i = 0; i < ret_len; ++i) ret[i] = label[i];
  }

  void knc_float_predict(const char* host, int port, long tptr,
                         int mid, bool save_proba, 
                         float* ret, long ret_len) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto test_dptr = (exrpc_ptr_t) tptr;
    std::vector<float> label;
    try {
      label = exrpc_async(fm_node, (frovedis_knc_predict<DT2,DT5,R_MAT2,KNC2>), test_dptr, mid, save_proba).get(); 
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    checkAssumption(label.size() == ret_len);
    for(size_t i = 0; i < ret_len; ++i) ret[i] = label[i];
  }

  void knr_double_predict(const char* host, int port, long tptr,
                          int mid, double* ret, long ret_len) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto test_dptr = (exrpc_ptr_t) tptr;
    std::vector<double> label;
    try {
      label = exrpc_async(fm_node, (frovedis_knr_predict<DT1,DT5,R_MAT1,KNR1>), test_dptr, mid).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    checkAssumption(label.size() == ret_len);
    for(size_t i = 0; i < ret_len; ++i) ret[i] = label[i];
  }

  void knr_float_predict(const char* host, int port, long tptr,
                         int mid, float* ret, long ret_len) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto test_dptr = (exrpc_ptr_t) tptr;
    std::vector<float> label;
    try {
      label = exrpc_async(fm_node, (frovedis_knr_predict<DT2,DT5,R_MAT2,KNR2>), test_dptr, mid).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    checkAssumption(label.size() == ret_len);
    for(size_t i = 0; i < ret_len; ++i) ret[i] = label[i];
  }

  // knc predict proba
  PyObject* knc_predict_proba(const char* host, int port,
                              long tptr, int mid, short dtype) {
      if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
      exrpc_node fm_node(host,port);
      auto test_dptr = (exrpc_ptr_t) tptr;
      dummy_matrix dmat;
      try {
        switch(dtype) {
          case FLOAT:
            dmat = exrpc_async(fm_node, (frovedis_knc_predict_proba<DT5,R_MAT2,KNC2,R_MAT2,R_LMAT2>), 
                               test_dptr, mid).get();
            break;
          case DOUBLE:
            dmat = exrpc_async(fm_node, (frovedis_knc_predict_proba<DT5,R_MAT1,KNC1,R_MAT1,R_LMAT1>), 
                               test_dptr, mid).get();
            break;
          default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input dense matrix!\n");
        }
      }
      catch (std::exception& e) {
        set_status(true, e.what());
      }
      return to_py_dummy_matrix(dmat);
    }

  float knr_model_score(const char* host, int port, long xptr,
                        long yptr, int mid, short dtype){
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto mptr = (exrpc_ptr_t) xptr;
    auto lblptr = (exrpc_ptr_t) yptr;
    float res = 0.0;
    try {
      switch(dtype) {
        case FLOAT:
          res = exrpc_async(fm_node, (frovedis_model_score<DT2,DT5,R_MAT2,KNR2>), mptr, lblptr, mid).get();
          break;
        case DOUBLE:
          res = exrpc_async(fm_node, (frovedis_model_score<DT1,DT5,R_MAT1,KNR1>), mptr, lblptr, mid).get();
          break;
        default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input dense matrix!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return res;
  }

  float knc_model_score(const char* host, int port, long xptr,
                    long yptr, int mid, short dtype){
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto mptr = (exrpc_ptr_t) xptr;
    auto lblptr = (exrpc_ptr_t) yptr;
    float res = 0 ;
    try {
      switch(dtype) {
        case FLOAT:
          res = exrpc_async(fm_node, (frovedis_model_score<DT2,DT5,R_MAT2,KNC2>), mptr, lblptr, mid).get();
          break;
        case DOUBLE:
          res = exrpc_async(fm_node, (frovedis_model_score<DT1,DT5,R_MAT1,KNC1>), mptr, lblptr, mid).get();
          break;
        default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input dense matrix!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return res;
  }


/*
void fpgrowth_freq_items(const char* host, int port,
                            int mid) {

    //std::cout<<"Entering model.cc \n\n";
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    //std::vector<dftable> freq;
    //int x=-1;
    try {
    auto x =  exrpc_async(fm_node,get_fis<FPM1>,mid).get();
    }
    catch (std::exception& e) {
     set_status(true, e.what());
    }
   // std::cout<<"size of vector recievd : \n "<<freq.size()<<"\n";
    //std::cout<<" Received frequest items Successfully \n"<< x << std::endl;
  }


void fpgrowth_rules(const char* host, int port,
                            int mid, double con) {

    //std::cout<<"Entering model.cc \n\n";
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    //std::vector<dftable> freq;
    //int x=-1;
    try {
    auto x =  exrpc_async(fm_node,get_fp_rules<FPM1>,mid,con).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
   // std::cout<<"size of vector recievd : \n "<<freq.size()<<"\n";
    //std::cout<<" Received association rules Successfully \n"<< x << std::endl;
  }
*/

  PyObject* compute_lda_component(const char* host, int port,
                                  int mid, short dtype){
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    dummy_matrix ret;
    try {
      switch(dtype) {
        case INT:
          ret = exrpc_async(fm_node,get_lda_component<LDA3>, mid).get();
          break;
        case LONG:
          ret = exrpc_async(fm_node,get_lda_component<LDA4>, mid).get();
          break;
        default: REPORT_ERROR(USER_ERROR, "Unsupported dtype for LDA model!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_matrix(ret);
  }

  PyObject* compute_lda_transform(const char* host, int port,
                                  long dptr, double alpha,
                                  double beta, int num_iter,
                                  const char* algorithm,
                                  int num_explore_iter, int mid,
                                  short dtype, short itype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    std::string algo(algorithm);
    dummy_lda_result ret;
    try {
      switch(dtype) {
        case INT:
          if(itype == INT)
            /* currently integer itype is not supported in the library */
            /*ret = exrpc_async(fm_node,(frovedis_lda_transform<DT4,S_MAT44,LDA4>),f_dptr, alpha,
                                beta, num_iter, algo, num_explore_iter, mid).get();*/
            REPORT_ERROR(USER_ERROR,
                         "Currently frovedis LDA doesn't support int itype for sparse data!\n");
          else if(itype == LONG)
            ret = exrpc_async(fm_node,(frovedis_lda_transform<DT4,S_MAT45,LDA4>),f_dptr, alpha,
                              beta, num_iter, algo, num_explore_iter, mid).get();
          else REPORT_ERROR(USER_ERROR, "Unsupported itype for input sparse datain LDA test!\n");
          break;
        case LONG:
          if(itype == INT)
            /* currently integer itype is not supported in the library */
            /*ret = exrpc_async(fm_node,(frovedis_lda_transform<DT3,S_MAT34,LDA3>),f_dptr, alpha,
                                beta, num_iter, algo, num_explore_iter, mid).get();*/
            REPORT_ERROR(USER_ERROR,
                         "Currently frovedis LDA doesn't support int itype for sparse data!\n");
          else if(itype == LONG)
            ret = exrpc_async(fm_node,(frovedis_lda_transform<DT3,S_MAT35,LDA3>),f_dptr, alpha,
                           beta, num_iter, algo, num_explore_iter, mid).get();
          else REPORT_ERROR(USER_ERROR, "Unsupported itype for input sparse datain LDA test!\n");
          break;
        default: REPORT_ERROR(USER_ERROR, "Unsupported dtype of input sparse data for LDA test!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_lda_result(ret);
  }

}
