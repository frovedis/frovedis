#include "python_client_headers.hpp"
#include "exrpc_model.hpp"
#include "exrpc_tsa.hpp"
#include "short_hand_dense_type.hpp"
#include "short_hand_sparse_type.hpp"
#include "short_hand_model_type.hpp"

extern "C" {

  float get_homogeneity_score(const char* host, int port,
                              long tlblp, long plblp,
                              ulong size, short dtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto tlbl = (exrpc_ptr_t) tlblp;
    auto plbl = (exrpc_ptr_t) plblp;

    float ret = 0;
    try {
      switch(dtype) {
        case INT:    ret = exrpc_async(fm_node, frovedis_homogeneity_score<DT4>,
                                       tlbl, plbl).get(); break;
        case LONG:   ret = exrpc_async(fm_node, frovedis_homogeneity_score<DT3>,
                                       tlbl, plbl).get(); break;
        case FLOAT:  ret = exrpc_async(fm_node, frovedis_homogeneity_score<DT2>,
                                       tlbl, plbl).get(); break;
        case DOUBLE: ret = exrpc_async(fm_node, frovedis_homogeneity_score<DT1>,
                                       tlbl, plbl).get(); break;
        default:     REPORT_ERROR(USER_ERROR, 
                     "homogeneity_score: expected either int, long, float or double type input!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return ret;
  }

  void show_frovedis_model(const char* host, int port, 
                           int mid, short mkind, short mdtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    
    try {
      if (mdtype == FLOAT) {
        switch(mkind) {
          case LR:    exrpc_oneway(fm_node,show_model<LR2>,mid); break;
          case SVM:    exrpc_oneway(fm_node,show_model<SVM2>,mid); break;
          case SVR:    exrpc_oneway(fm_node,show_model<SVR2>,mid); break;
          //case KSVC:   exrpc_oneway(fm_node,show_model<KSVC2>,mid); break;
          case RR:   exrpc_oneway(fm_node,show_model<RR2>,mid); break;
          case LSR:   exrpc_oneway(fm_node,show_model<LSR2>,mid); break;
          case LNRM:   exrpc_oneway(fm_node,show_model<LNRM2>,mid); break;
          case MFM:    exrpc_oneway(fm_node,show_model<MFM2>,mid); break;
          case KMEANS: exrpc_oneway(fm_node,show_model<KMM2>,mid); break;
          case ACM:    exrpc_oneway(fm_node,show_model<ACM2>,mid); break;
          case SEM:    exrpc_oneway(fm_node,show_model<SEM2>,mid); break;
          case SCM:    exrpc_oneway(fm_node,show_model<SCM2>,mid); break;
          //case DBSCAN: exrpc_oneway(fm_node,show_model<DBM2>,mid); break;
          case GMM:    exrpc_oneway(fm_node,show_model<GMM2>,mid); break;      
          case DTM:    exrpc_oneway(fm_node,show_model<DTM2>,mid); break;
          case FPM:    exrpc_oneway(fm_node,show_model<FPM1>,mid); break; // not template based
          case FPR:    exrpc_oneway(fm_node, show_model<FPR1>,mid); break; // not template based
          case FMM:    REPORT_ERROR(USER_ERROR,"currently Frovedis fm_model can't be displayed!");
          case NBM:    exrpc_oneway(fm_node,show_model<NBM2>,mid); break;
          case RFM:    exrpc_oneway(fm_node,show_model<RFM2>,mid); break;
          case GBT:    exrpc_oneway(fm_node,show_model<GBT2>,mid); break;
          default:     REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
        }
      }
      else if (mdtype == DOUBLE) {
        switch(mkind) {
          case LR:    exrpc_oneway(fm_node,show_model<LR1>,mid); break;
          case SVM:    exrpc_oneway(fm_node,show_model<SVM1>,mid); break;
          case SVR:    exrpc_oneway(fm_node,show_model<SVR1>,mid); break;
          //case KSVC:   exrpc_oneway(fm_node,show_model<KSVC1>,mid); break;
          case RR:   exrpc_oneway(fm_node,show_model<RR1>,mid); break;
          case LSR:   exrpc_oneway(fm_node,show_model<LSR1>,mid); break;
          case LNRM:   exrpc_oneway(fm_node,show_model<LNRM1>,mid); break;
          case MFM:    exrpc_oneway(fm_node,show_model<MFM1>,mid); break;
          case KMEANS: exrpc_oneway(fm_node,show_model<KMM1>,mid); break;
          case ACM:    exrpc_oneway(fm_node,show_model<ACM1>,mid); break;
          case SEM:    exrpc_oneway(fm_node,show_model<SEM1>,mid); break;
          case SCM:    exrpc_oneway(fm_node,show_model<SCM1>,mid); break;
          //case DBSCAN: exrpc_oneway(fm_node,show_model<DBM1>,mid); break;
          case GMM:    exrpc_oneway(fm_node,show_model<GMM1>,mid); break;      
          case DTM:    exrpc_oneway(fm_node,show_model<DTM1>,mid); break;
          case FPM:    exrpc_oneway(fm_node,show_model<FPM1>,mid); break; // not template based
          case FPR:    exrpc_oneway(fm_node, show_model<FPR1>,mid); break; // not template based
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
                              int mid, short mkind, short mdtype, 
                              short itype, bool dense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    try {
      if (mdtype == FLOAT) {
        switch(mkind) {
          case LR:    exrpc_oneway(fm_node,release_model<LR2>,mid); break;
          case SVM:    exrpc_oneway(fm_node,release_model<SVM2>,mid); break;
          case SVR:    exrpc_oneway(fm_node,release_model<SVR2>,mid); break;
          case KSVC:   exrpc_oneway(fm_node,release_model<KSVC2>,mid); break;
          case RR:   exrpc_oneway(fm_node,release_model<RR2>,mid);break;
          case LSR:   exrpc_oneway(fm_node,release_model<LSR2>,mid);break;
          case LNRM:   exrpc_oneway(fm_node,release_model<LNRM2>,mid);break;
          case MFM:    exrpc_oneway(fm_node,release_model<MFM2>,mid); break;
          case KMEANS: exrpc_oneway(fm_node,release_model<KMM2>,mid); break;
          case ACM:    exrpc_oneway(fm_node,release_model<ACM2>,mid); break;
          case SEM:    exrpc_oneway(fm_node,release_model<SEM2>,mid); break;
          case SCM:    exrpc_oneway(fm_node,release_model<SCM2>,mid); break;
          case DBSCAN: exrpc_oneway(fm_node,release_model<DBSCAN2>,mid); break;
          case GMM:    exrpc_oneway(fm_node,release_model<GMM2>,mid); break;      
          case DTM:    exrpc_oneway(fm_node,release_model<DTM2>,mid); break;
          case FPM:    exrpc_oneway(fm_node,release_model<FPM1>,mid); break; // not template based
          case FPR:    exrpc_oneway(fm_node, release_model<FPR1>,mid); break; // not template based
          case FMM:    exrpc_oneway(fm_node,release_model<FMM2>,mid); break;
          case NBM:    exrpc_oneway(fm_node,release_model<NBM2>,mid); break;
          case STANDARDSCALER:    exrpc_oneway(fm_node,release_model<STANDARDSCALER2>,mid); break;
          case ARM:    exrpc_oneway(fm_node,release_model<ARM2>,mid); break;
          case KNN: 
            if(dense) {   
              exrpc_oneway(fm_node,release_model<KNNR2>,mid); 
            }
            else {
              if(itype == INT) 
                exrpc_oneway(fm_node,release_model<KNNS24>,mid); 
              else if(itype == LONG) 
                exrpc_oneway(fm_node,release_model<KNNS25>,mid); 
            }
            break;
          case KNR:
            if(dense) {   
              exrpc_oneway(fm_node,release_model<KNRR2>,mid); 
            }
            else {
              if(itype == INT) 
                exrpc_oneway(fm_node,release_model<KNRS24>,mid); 
              else if(itype == LONG) 
                exrpc_oneway(fm_node,release_model<KNRS25>,mid); 
            }
            break;
          case KNC:   
            if(dense) {   
              exrpc_oneway(fm_node,release_model<KNCR2>,mid); 
            }
            else {
              if(itype == INT) 
                exrpc_oneway(fm_node,release_model<KNCS24>,mid); 
              else if(itype == LONG) 
                exrpc_oneway(fm_node,release_model<KNCS25>,mid); 
            }
            break;
          case RFM:    exrpc_oneway(fm_node,release_model<RFM2>,mid); break;
          case GBT:    exrpc_oneway(fm_node,release_model<GBT2>,mid); break;      
          default:     REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
        }
      }
      else if (mdtype == DOUBLE) {
        switch(mkind) {
          case LR:    exrpc_oneway(fm_node,release_model<LR1>,mid); break;
          case SVM:    exrpc_oneway(fm_node,release_model<SVM1>,mid); break;
          case SVR:    exrpc_oneway(fm_node,release_model<SVR1>,mid); break;
          case KSVC:   exrpc_oneway(fm_node,release_model<KSVC1>,mid); break;
          case RR:   exrpc_oneway(fm_node,release_model<RR1>,mid);break;
          case LSR:   exrpc_oneway(fm_node,release_model<LSR1>,mid);break;
          case LNRM:   exrpc_oneway(fm_node,release_model<LNRM1>,mid);break;
          case MFM:    exrpc_oneway(fm_node,release_model<MFM1>,mid); break;
          case DTM:    exrpc_oneway(fm_node,release_model<DTM1>,mid); break;
          case KMEANS: exrpc_oneway(fm_node,release_model<KMM1>,mid); break;
          case ACM:    exrpc_oneway(fm_node,release_model<ACM1>,mid); break;
          case SEM:    exrpc_oneway(fm_node,release_model<SEM1>,mid); break;
          case SCM:    exrpc_oneway(fm_node,release_model<SCM1>,mid); break;
          case DBSCAN: exrpc_oneway(fm_node,release_model<DBSCAN1>,mid); break;
          case GMM:    exrpc_oneway(fm_node,release_model<GMM1>,mid); break;      
          case FPM:    exrpc_oneway(fm_node,release_model<FPM1>,mid); break; // not template based
          case FPR:    exrpc_oneway(fm_node, release_model<FPR1>,mid); break; // not template based
          case FMM:    exrpc_oneway(fm_node,release_model<FMM1>,mid); break;
          case NBM:    exrpc_oneway(fm_node,release_model<NBM1>,mid); break;
          case STANDARDSCALER:    exrpc_oneway(fm_node,release_model<STANDARDSCALER1>,mid); break;
          case ARM:    exrpc_oneway(fm_node,release_model<ARM1>,mid); break;
          case KNN: 
            if(dense) {   
              exrpc_oneway(fm_node,release_model<KNNR1>,mid); 
            }
            else {
              if(itype == INT) 
                exrpc_oneway(fm_node,release_model<KNNS14>,mid); 
              else if(itype == LONG) 
                exrpc_oneway(fm_node,release_model<KNNS15>,mid); 
            }
            break;
          case KNR:
            if(dense) {   
              exrpc_oneway(fm_node,release_model<KNRR1>,mid); 
            }
            else {
              if(itype == INT) 
                exrpc_oneway(fm_node,release_model<KNRS14>,mid); 
              else if(itype == LONG) 
                exrpc_oneway(fm_node,release_model<KNRS15>,mid); 
            }
            break;

          case KNC: 
            if(dense) {   
              exrpc_oneway(fm_node,release_model<KNCR1>,mid); 
            }
            else {
              if(itype == INT) 
                exrpc_oneway(fm_node,release_model<KNCS14>,mid); 
              else if(itype == LONG) 
                exrpc_oneway(fm_node,release_model<KNCS15>,mid); 
            }
            break;
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
          case LR:    exrpc_oneway(fm_node,save_model<LR2>,mid,fs_path); break;
          case SVM:    exrpc_oneway(fm_node,save_model<SVM2>,mid,fs_path); break;
          case SVR:    exrpc_oneway(fm_node,save_model<SVR2>,mid,fs_path); break;
          case KSVC:   exrpc_oneway(fm_node,save_model<KSVC2>,mid,fs_path); break;
          case RR:   exrpc_oneway(fm_node,save_model<RR2>,mid,fs_path); break;
          case LSR:   exrpc_oneway(fm_node,save_model<LSR2>,mid,fs_path); break;
          case LNRM:   exrpc_oneway(fm_node,save_model<LNRM2>,mid,fs_path); break;
          case MFM:    exrpc_oneway(fm_node,save_model<MFM2>,mid,fs_path); break;
          case KMEANS: exrpc_oneway(fm_node,save_model<KMM2>,mid,fs_path); break;
          case ACM:    exrpc_oneway(fm_node,save_model<ACM2>,mid,fs_path); break;
          case SEM:    exrpc_oneway(fm_node,save_model<SEM2>,mid,fs_path); break;
          case SCM:    exrpc_oneway(fm_node,save_model<SCM2>,mid,fs_path); break;
          case GMM:    exrpc_oneway(fm_node,save_model<GMM2>,mid,fs_path); break;      
          case DTM:    exrpc_oneway(fm_node,save_model<DTM2>,mid,fs_path); break;
          case FPM:    exrpc_oneway(fm_node,save_model<FPM1>,mid,fs_path); break; // not template based
          case FPR:    exrpc_oneway(fm_node,save_model<FPR1>,mid,fs_path); break; // not template based
          case FMM:    exrpc_oneway(fm_node,save_fmm<DT2>,mid,fs_path); break;
          case NBM:    exrpc_oneway(fm_node,save_model<NBM2>,mid,fs_path); break;
          case RFM:    exrpc_oneway(fm_node,save_model<RFM2>,mid,fs_path); break;
          case GBT:    exrpc_oneway(fm_node,save_model<GBT2>,mid,fs_path); break;
          default:     REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
        }
      }
      else if (mdtype == DOUBLE) {
        switch(mkind) {
          case LR:    exrpc_oneway(fm_node,save_model<LR1>,mid,fs_path); break;
          case SVM:    exrpc_oneway(fm_node,save_model<SVM1>,mid,fs_path); break;
          case SVR:    exrpc_oneway(fm_node,save_model<SVR1>,mid,fs_path); break;
          case KSVC:   exrpc_oneway(fm_node,save_model<KSVC1>,mid,fs_path); break;
          case RR:   exrpc_oneway(fm_node,save_model<RR1>,mid,fs_path); break;
          case LSR:   exrpc_oneway(fm_node,save_model<LSR1>,mid,fs_path); break;
          case LNRM:   exrpc_oneway(fm_node,save_model<LNRM1>,mid,fs_path); break;
          case MFM:    exrpc_oneway(fm_node,save_model<MFM1>,mid,fs_path); break;
          case KMEANS: exrpc_oneway(fm_node,save_model<KMM1>,mid,fs_path); break;
          case ACM:    exrpc_oneway(fm_node,save_model<ACM1>,mid,fs_path); break;
          case SEM:    exrpc_oneway(fm_node,save_model<SEM1>,mid,fs_path); break;
          case SCM:    exrpc_oneway(fm_node,save_model<SCM1>,mid,fs_path); break;
          case GMM:    exrpc_oneway(fm_node,save_model<GMM1>,mid,fs_path); break;      
          case DTM:    exrpc_oneway(fm_node,save_model<DTM1>,mid,fs_path); break;
          case FPM:    exrpc_oneway(fm_node,save_model<FPM1>,mid,fs_path); break; // not template based
          case FPR:    exrpc_oneway(fm_node,save_model<FPR1>,mid,fs_path); break; // not template based
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
          case LR:    exrpc_async(fm_node,load_glm<LR2>,mid,LR,fs_path).get(); break;
          case SVM:    exrpc_async(fm_node,load_glm<SVM2>,mid,SVM,fs_path).get(); break;
          case SVR:    exrpc_async(fm_node,load_lnrm<SVR2>,mid,SVR,fs_path).get(); break;
          case KSVC:   exrpc_oneway(fm_node,load_model<KSVC2>,mid,KSVC,fs_path); break; 
          case RR:   exrpc_async(fm_node,load_lnrm<RR2>,mid,RR,fs_path).get(); break;
          case LSR:   exrpc_async(fm_node,load_lnrm<LSR2>,mid,LSR,fs_path).get(); break;
          case LNRM:   exrpc_async(fm_node,load_lnrm<LNRM1>,mid,LNRM,fs_path).get(); break;
          case KMEANS: exrpc_async(fm_node,load_kmm<DT2>,mid,KMEANS,fs_path).get(); break;
          case GMM:    exrpc_oneway(fm_node,load_model<GMM2>,mid,GMM,fs_path); break;      
          case SEM:    exrpc_oneway(fm_node,load_model<SEM2>,mid,SEM,fs_path); break;
          case DTM:    exrpc_oneway(fm_node,load_model<DTM2>,mid,DTM,fs_path); break;
          case FMM:    REPORT_ERROR(USER_ERROR,"currently frovedis fm_model can't be loaded!");
          case FPR:    exrpc_oneway(fm_node,load_model<FPR1>,mid,FPM,fs_path); break;
          case RFM:    exrpc_oneway(fm_node,load_model<RFM2>,mid,RFM,fs_path); break;
          case GBT:    exrpc_oneway(fm_node,load_model<GBT2>,mid,GBT,fs_path); break;
          default:     REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
        }
      }
      else if (mdtype == DOUBLE) {
        switch(mkind) {
          case LR:    exrpc_async(fm_node,load_glm<LR1>,mid,LR,fs_path).get(); break;; 
          case SVM:    exrpc_async(fm_node,load_glm<SVM1>,mid,SVM,fs_path).get(); break;
          case SVR:    exrpc_async(fm_node,load_lnrm<SVR1>,mid,SVR,fs_path).get(); break;
          case KSVC:   exrpc_oneway(fm_node,load_model<KSVC1>,mid,KSVC,fs_path); break; 
          case RR:   exrpc_async(fm_node,load_lnrm<RR1>,mid,RR,fs_path).get(); break;
          case LSR:   exrpc_async(fm_node,load_lnrm<LSR1>,mid,LSR,fs_path).get(); break;
          case LNRM:   exrpc_async(fm_node,load_lnrm<LNRM1>,mid,LNRM,fs_path).get(); break;
          case KMEANS: exrpc_async(fm_node,load_kmm<DT1>,mid,KMEANS,fs_path).get(); break;
          case GMM:    exrpc_oneway(fm_node,load_model<GMM1>,mid,GMM,fs_path); break;      
          case SEM:    exrpc_oneway(fm_node,load_model<SEM1>,mid,SEM,fs_path); break;
          case DTM:    exrpc_oneway(fm_node,load_model<DTM1>,mid,DTM,fs_path); break;
          case FPR:    exrpc_oneway(fm_node,load_model<FPR1>,mid,FPM,fs_path); break;
          case FMM:    REPORT_ERROR(USER_ERROR,"currently frovedis fm_model can't be loaded!");
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

  int load_fp_model(const char* host, int port, 
                    int mid, short mkind, 
                    const char* path) {
    ASSERT_PTR(path);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    std::string fs_path(path);
    int cnt = 0;
    try {
      switch(mkind) {
        case FPM: cnt = exrpc_async(fm_node,load_fpm<FPM1>,
                        mid,FPM,fs_path).get(); break;
        case FPR: cnt = exrpc_async(fm_node,load_fpm<FPR1>,
                        mid,FPR,fs_path).get(); break;
        default:  REPORT_ERROR(USER_ERROR,
                  "Unknown FP-model kind for is encountered!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return cnt;
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
                   long* ret, long ret_len) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    std::vector<int> label;
    try {
      switch(mdtype) {
        case FLOAT:
          label = exrpc_async(fm_node,frovedis_acm_reassign<DT2>,mid,ncluster).get(); break;
        case DOUBLE:
          label = exrpc_async(fm_node,frovedis_acm_reassign<DT1>,mid,ncluster).get(); break;
        default: REPORT_ERROR(USER_ERROR,"model dtype can either be float or double!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    checkAssumption(label.size() == ret_len);
    for(size_t i = 0; i < ret_len; ++i) ret[i] = static_cast<long>(label[i]);
  }

  void load_frovedis_acm(const char* host, int port,
                         int mid, short mdtype, const char* path,
                         long* ret, long ret_len) {
    ASSERT_PTR(path);
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    std::string fs_path(path);
    std::vector<int> labels;
    try {
      switch(mdtype) {
        case FLOAT:
          labels = exrpc_async(fm_node,load_acm<ACM2>,mid,fs_path).get(); break;
        case DOUBLE:
          labels = exrpc_async(fm_node,load_acm<ACM1>,mid,fs_path).get(); break;
        default: REPORT_ERROR(USER_ERROR,"model dtype can either be float or double!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    checkAssumption(labels.size() == ret_len);
    for(size_t i = 0; i < ret_len; ++i) ret[i] = static_cast<long>(labels[i]);
  }

  PyObject* get_acm_children_vector(const char* host, int port,
                                    int mid, short mdtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    std::vector<size_t> children_vector;
    try {
      switch(mdtype) {
        case FLOAT:
          children_vector = exrpc_async(fm_node,get_acm_children<DT2>,mid).get(); break;
        case DOUBLE:
          children_vector = exrpc_async(fm_node,get_acm_children<DT1>,mid).get(); break;
        default: REPORT_ERROR(USER_ERROR,"model dtype can either be float or double!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
     
    return to_python_llong_list(children_vector);  
  }

  PyObject* get_acm_distances_vector(const char* host, int port,
                                     int mid, short mdtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    PyObject* ret_ptr = NULL;
    try {
      if(mdtype == FLOAT) {
        std::vector<float> distance_vector;
        distance_vector = exrpc_async(fm_node,get_acm_distances<DT2>,mid).get();  
        ret_ptr = to_python_float_list(distance_vector);
      }             
      else if(mdtype == DOUBLE) {
        std::vector<double> distance_vector;
        distance_vector = exrpc_async(fm_node,get_acm_distances<DT1>,mid).get();
        ret_ptr =  to_python_double_list(distance_vector); 
      }
      else REPORT_ERROR(USER_ERROR,"model dtype can either be float or double!\n");      
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }     
    return ret_ptr; 
  }    

  int get_acm_n_connected_components(const char* host, int port,
                                     int mid, short mdtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    int ncc = 0;
    try {
      switch(mdtype) {
        case FLOAT:
          ncc = exrpc_async(fm_node,get_acm_n_components<DT2>,mid).get(); break;          
        case DOUBLE:
          ncc = exrpc_async(fm_node,get_acm_n_components<DT1>,mid).get(); break;         
        default: REPORT_ERROR(USER_ERROR,"model dtype can either be float or double!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
     
    return ncc;
  }

  int get_acm_no_clusters(const char* host, int port,
                          int mid, short mdtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    int nc = 0;
    try {
      switch(mdtype) {
        case FLOAT:
          nc = exrpc_async(fm_node,get_acm_n_clusters<DT2>,mid).get(); break;          
        case DOUBLE:
          nc = exrpc_async(fm_node,get_acm_n_clusters<DT1>,mid).get(); break;         
        default: REPORT_ERROR(USER_ERROR,"model dtype can either be float or double!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
     
    return nc;
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

  void gmm_predict(const char* host, int port,
                   int mid, short mdtype, 
                   long dptr,long* ret,ulong ret_len) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    std::vector<int> pred;      
    try {
      switch(mdtype) {
        case FLOAT:
          pred = exrpc_async(fm_node,(frovedis_gmm_predict<R_MAT2,GMM2>),f_dptr,mid).get(); break;
        case DOUBLE:
          pred = exrpc_async(fm_node,(frovedis_gmm_predict<R_MAT1,GMM1>),f_dptr,mid).get(); break;
        default: REPORT_ERROR(USER_ERROR,"model dtype can either be float or double!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    checkAssumption(pred.size() == ret_len);
    for(size_t i = 0; i < ret_len; ++i) ret[i] = static_cast<long>(pred[i]);
  }    

  PyObject* gmm_predict_proba(const char* host, int port,
                              int mid, short mdtype, long dptr) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    PyObject* ret_ptr = NULL;  
    try {
      if(mdtype == FLOAT) {
        std::vector<float> prob;
        prob = exrpc_async(fm_node,(frovedis_gmm_predict_proba<DT2,R_MAT2,GMM2>),f_dptr,mid).get();  
        ret_ptr = to_python_float_list(prob);  
      }    
      else if(mdtype == DOUBLE) {
        std::vector<double> prob;
        prob = exrpc_async(fm_node,(frovedis_gmm_predict_proba<DT1,R_MAT1,GMM1>),f_dptr,mid).get();
        ret_ptr = to_python_double_list(prob);  
      } 
      else REPORT_ERROR(USER_ERROR,"model dtype can either be float or double!\n");      
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }  
    return ret_ptr;  
  }    
     
  PyObject* get_gmm_weights_vector(const char* host, int port,
                                    int mid, short mdtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    PyObject* ret_ptr = NULL;
    try {
      if(mdtype == FLOAT) {
        std::vector<float> weights_vector;
        weights_vector = exrpc_async(fm_node,get_gmm_weights<DT2>,mid).get();
        ret_ptr = to_python_float_list(weights_vector);          
      } 
      else if(mdtype == DOUBLE) {
        std::vector<double> weights_vector;
        weights_vector = exrpc_async(fm_node,get_gmm_weights<DT1>,mid).get();
        ret_ptr = to_python_double_list(weights_vector);                    
      }
      else REPORT_ERROR(USER_ERROR,"model dtype can either be float or double!\n");
    }    
    catch (std::exception& e) {
      set_status(true, e.what());
    }     
    return ret_ptr;  
  }

  PyObject* get_gmm_covariances_vector(const char* host, int port,
                                    int mid, short mdtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    PyObject* ret_ptr = NULL;
    try {
      if(mdtype == FLOAT) {
        std::vector<float> covariances_vector;
        covariances_vector = exrpc_async(fm_node,get_gmm_covariances<DT2>,mid).get(); 
        ret_ptr = to_python_float_list(covariances_vector);
      }
      else if(mdtype == DOUBLE) {
        std::vector<double> covariances_vector;
        covariances_vector = exrpc_async(fm_node,get_gmm_covariances<DT1>,mid).get();
        ret_ptr = to_python_double_list(covariances_vector);          
      }
      else REPORT_ERROR(USER_ERROR,"model dtype can either be float or double!\n");      
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }     
    return ret_ptr;  
  }

  PyObject* get_gmm_means_vector(const char* host, int port,
                                    int mid, short mdtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    PyObject* ret_ptr = NULL;
    try {
      if(mdtype == FLOAT) {
        std::vector<float> means_vector;
        means_vector = exrpc_async(fm_node,get_gmm_means<DT2>,mid).get();
        ret_ptr = to_python_float_list(means_vector);          
      }
      else if(mdtype == DOUBLE) {
        std::vector<double> means_vector;
        means_vector = exrpc_async(fm_node,get_gmm_means<DT1>,mid).get();
        ret_ptr = to_python_double_list(means_vector);          
      }
      else REPORT_ERROR(USER_ERROR,"model dtype can either be float or double!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }     
    return ret_ptr;  
  }
    
  bool get_gmm_converged_bool(const char* host, int port,
                                    int mid, short mdtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    bool converged = false;
    try {
      switch(mdtype) {
        case FLOAT:
          converged = exrpc_async(fm_node,get_gmm_converged<DT2>,mid).get(); break;
        case DOUBLE:
          converged = exrpc_async(fm_node,get_gmm_converged<DT1>,mid).get(); break;
        default: REPORT_ERROR(USER_ERROR,"model dtype can either be float or double!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }     
    return converged;  
  }

  double get_gmm_lower_bound_val(const char* host, int port,
                                    int mid, short mdtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    double lower_bound = 0;
    try {
      switch(mdtype) {
        case FLOAT:
          lower_bound = exrpc_async(fm_node,get_gmm_lower_bound<DT2>,mid).get(); break;
        case DOUBLE:
          lower_bound = exrpc_async(fm_node,get_gmm_lower_bound<DT1>,mid).get(); break;
        default: REPORT_ERROR(USER_ERROR,"model dtype can either be float or double!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }     
    return lower_bound;  
  }

  double get_gmm_score_val(const char* host, int port,
                            int mid, short mdtype, long dptr) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;  
    double score = 0;
    try {
      switch(mdtype) {
        case FLOAT:
          score = exrpc_async(fm_node,(get_gmm_score<DT2,R_MAT2,GMM2>),
                              f_dptr,mid).get(); break;
        case DOUBLE:
          score = exrpc_async(fm_node,(get_gmm_score<DT1,R_MAT1,GMM1>),
                              f_dptr,mid).get(); break;
        default: REPORT_ERROR(USER_ERROR,"model dtype can either be float or double!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }     
    return score;  
  }     

  PyObject* get_gmm_score_samples_vec(const char* host, int port,
                                      int mid, short mdtype, long dptr) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    PyObject* ret = NULL;
    try {
      switch(mdtype) {
        case FLOAT: {
          auto scores = exrpc_async(fm_node,(get_gmm_score_samples<DT2,R_MAT2,GMM2>),
                                   f_dptr,mid).get(); 
          ret = to_python_float_list(scores);
          break;
        }
        case DOUBLE: {
          auto scores = exrpc_async(fm_node,(get_gmm_score_samples<DT1,R_MAT1,GMM1>),
                                    f_dptr,mid).get(); 
          ret = to_python_double_list(scores);
          break;
        }
        default: REPORT_ERROR(USER_ERROR, 
                 "model dtype can either be float or double!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return ret;
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
          case LR:  ret = exrpc_async(fm_node, (get_weight_vector<DT2,LR2>), mid).get(); break;
          case RR:  ret = exrpc_async(fm_node, (get_weight_vector<DT2,RR2>), mid).get(); break;
          case LSR:  ret = exrpc_async(fm_node, (get_weight_vector<DT2,LSR2>), mid).get(); break;
          case SVM:  ret = exrpc_async(fm_node, (get_weight_vector<DT2,SVM2>), mid).get(); break;
          case SVR:  ret = exrpc_async(fm_node, (get_weight_vector<DT2,SVR2>), mid).get(); break;
          case LNRM: ret = exrpc_async(fm_node, (get_weight_vector<DT2,LNRM2>), mid).get(); break;
          default: REPORT_ERROR(USER_ERROR, "Unknown model for weight vector extraction!\n");
        }
        ret_ptr = to_python_float_list(ret);
      }
      else if (mdtype == DOUBLE) {
        std::vector<double> ret;
        switch(mkind) {
          case LR:  ret = exrpc_async(fm_node, (get_weight_vector<DT1,LR1>), mid).get(); break;
          case RR:  ret = exrpc_async(fm_node, (get_weight_vector<DT1,RR1>), mid).get(); break;
          case LSR:  ret = exrpc_async(fm_node, (get_weight_vector<DT1,LSR1>), mid).get(); break;
          case SVM:  ret = exrpc_async(fm_node, (get_weight_vector<DT1,SVM1>), mid).get(); break;
          case SVR:  ret = exrpc_async(fm_node, (get_weight_vector<DT1,SVR1>), mid).get(); break;
          case LNRM: ret = exrpc_async(fm_node, (get_weight_vector<DT1,LNRM1>), mid).get(); break;
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
          case LR:  ret = exrpc_async(fm_node, (get_intercept_as_vector<DT2,LR2>), mid).get(); break;
          case RR:  ret = exrpc_async(fm_node, (get_intercept_as_vector<DT2,RR2>), mid).get(); break;
          case LSR:  ret = exrpc_async(fm_node, (get_intercept_as_vector<DT2,LSR2>), mid).get(); break;
          case SVM:  ret = exrpc_async(fm_node, (get_intercept_as_vector<DT2,SVM2>), mid).get(); break;
          case SVR:  ret = exrpc_async(fm_node, (get_intercept_as_vector<DT2,SVR2>), mid).get(); break;
          case LNRM: ret = exrpc_async(fm_node, (get_intercept_as_vector<DT2,LNRM2>), mid).get(); break;
          default: REPORT_ERROR(USER_ERROR, "Unknown model for intercept vector extraction!\n");
        }
        ret_ptr = to_python_float_list(ret);
      }
      else if (mdtype == DOUBLE) {
        std::vector<double> ret;
        switch(mkind) {
          case LR:  ret = exrpc_async(fm_node, (get_intercept_as_vector<DT1,LR1>), mid).get(); break;
          case RR:  ret = exrpc_async(fm_node, (get_intercept_as_vector<DT1,RR1>), mid).get(); break;
          case LSR:  ret = exrpc_async(fm_node, (get_intercept_as_vector<DT1,LSR1>), mid).get(); break;
          case SVM:  ret = exrpc_async(fm_node, (get_intercept_as_vector<DT1,SVM1>), mid).get(); break;
          case SVR:  ret = exrpc_async(fm_node, (get_intercept_as_vector<DT1,SVR1>), mid).get(); break;
          case LNRM: ret = exrpc_async(fm_node, (get_intercept_as_vector<DT1,LNRM1>), mid).get(); break;
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

  PyObject* get_frovedis_feature_count(const char* host, int port,
                                   int mid, short mkind,
                                   short mdtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host, port);
    PyObject* ret_ptr = NULL;
    try {
      if (mdtype == FLOAT) {
        std::vector<float> ret;
        switch(mkind) {
          case NBM: ret = exrpc_async(fm_node, (get_feature_count<DT2,NBM2>), mid).get(); break;
          default: REPORT_ERROR(USER_ERROR, "Unknown model for feature_count extraction!\n");
        }
        ret_ptr = to_python_float_list(ret);
      }
      else if (mdtype == DOUBLE) {
        std::vector<double> ret;
        switch(mkind) {
          case NBM: ret = exrpc_async(fm_node, (get_feature_count<DT1,NBM1>), mid).get(); break;
          default: REPORT_ERROR(USER_ERROR, "Unknown model for feature_count extraction!\n");
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
    PyObject* ret_ptr = NULL;
    try {
      if (mdtype == FLOAT) {
        std::vector<float> ret;
        switch(mkind) {
          case NBM: ret = exrpc_async(fm_node, (get_cls_counts_vector<DT2,NBM2>), mid).get(); break;
          default: REPORT_ERROR(USER_ERROR, "Unknown model for class_count vector extraction!\n");
        }
        ret_ptr = to_python_float_list(ret);
      }
      else if (mdtype == DOUBLE) {
        std::vector<double> ret;
        switch(mkind) {
          case NBM: ret = exrpc_async(fm_node, (get_cls_counts_vector<DT1,NBM1>), mid).get(); break;
          default: REPORT_ERROR(USER_ERROR, "Unknown model for class_count vector extraction!\n");
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

 void parallel_float_glm_predict(const char* host, int port,
                                  int mid, short mkind, long dptr, 
                                  float* ret, ulong len, bool prob,
                                  short itype, bool isDense) {
    if(!host) REPORT_ERROR(USER_ERROR, "Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    std::vector<float> pred;
    
    try {
      if(isDense) {
        switch(mkind) {
          case LR:  pred = exrpc_async(fm_node,(parallel_lrm_predict<DT2,R_MAT2,LR2>),
                                        f_dptr,mid,prob).get(); break;
          case SVM:  pred = exrpc_async(fm_node,(parallel_svm_predict<DT2,R_MAT2,R_LMAT2,SVM2>),
                                        f_dptr,mid,prob).get(); break;
          case SVR:  pred = exrpc_async(fm_node,(parallel_lnrm_predict<DT2,R_MAT2,R_LMAT2,SVR2>),
                                        f_dptr,mid,prob).get(); break;
          case KSVC:  pred = exrpc_async(fm_node,(ksvm_predict<DT2,R_MAT2,KSVC2>),
                                        f_dptr,mid,prob).get(); break;
          case RR: pred = exrpc_async(fm_node,(parallel_lnrm_predict<DT2,R_MAT2,R_LMAT2,RR2>),
                                        f_dptr,mid,prob).get(); break;
          case LSR: pred = exrpc_async(fm_node,(parallel_lnrm_predict<DT2,R_MAT2,R_LMAT2,LSR2>),
                                        f_dptr,mid,prob).get(); break;
          case LNRM: pred = exrpc_async(fm_node,(parallel_lnrm_predict<DT2,R_MAT2,R_LMAT2,LNRM2>),
                                       f_dptr,mid,prob).get(); break;
          case DTM:  pred = exrpc_async(fm_node,(parallel_dtm_predict<DT2,R_MAT2,R_LMAT2>),
                                        f_dptr,mid,prob).get(); break;
          case FMM:  REPORT_ERROR(USER_ERROR, "currently Frovedis doesn't support dense test data for FM!\n");
          case NBM:  pred = exrpc_async(fm_node,(parallel_generic_predict<DT2,R_MAT2,R_LMAT2,NBM2>),
                                        f_dptr,mid,prob).get(); break;
          case RFM:  pred = exrpc_async(fm_node,(parallel_rfm_predict<DT2,R_MAT2,R_LMAT2>),
                                        f_dptr,mid,prob).get(); break;
          case GBT:  pred = exrpc_async(fm_node,(parallel_gbt_predict<DT2,R_MAT2,R_LMAT2>),
                                        f_dptr,mid,prob).get(); break;
          default:   REPORT_ERROR(USER_ERROR, "Unknown model kind is encountered!\n");
        }
      }
      else {
        switch(mkind) {
          case LR: { 
            if(itype == INT)
              pred = exrpc_async(fm_node,(parallel_lrm_predict<DT2,S_MAT24,LR2>),
                                 f_dptr,mid,prob).get(); 
            else if(itype == LONG)
              pred = exrpc_async(fm_node,(parallel_lrm_predict<DT2,S_MAT25,LR2>),
                                 f_dptr,mid,prob).get(); 
            else REPORT_ERROR(USER_ERROR, "Unsupported itype for sparse data!\n");
            break;
          }
          case SVM: {
            if(itype == INT)
              pred = exrpc_async(fm_node,(parallel_svm_predict<DT2,S_MAT24,S_LMAT24,SVM2>),
                                 f_dptr,mid,prob).get(); 
            else if(itype == LONG)
              pred = exrpc_async(fm_node,(parallel_svm_predict<DT2,S_MAT25,S_LMAT25,SVM2>),
                                 f_dptr,mid,prob).get();
            else REPORT_ERROR(USER_ERROR, "Unsupported itype for sparse data!\n");
            break;
          }
          case SVR: {
            if(itype == INT)
              pred = exrpc_async(fm_node,(parallel_lnrm_predict<DT2,S_MAT24,S_LMAT24,SVR2>),
                                 f_dptr,mid,prob).get();
            else if(itype == LONG)
              pred = exrpc_async(fm_node,(parallel_lnrm_predict<DT2,S_MAT25,S_LMAT25,SVR2>),
                                 f_dptr,mid,prob).get();
            else REPORT_ERROR(USER_ERROR, "Unsupported itype for sparse data!\n");
            break;
          }
          case RR: {
            if(itype == INT)
               pred = exrpc_async(fm_node,(parallel_lnrm_predict<DT2,S_MAT24,S_LMAT24,RR2>),
                                  f_dptr,mid,prob).get(); 
            else if(itype == LONG)
               pred = exrpc_async(fm_node,(parallel_lnrm_predict<DT2,S_MAT25,S_LMAT25,RR2>),
                                  f_dptr,mid,prob).get(); 
            else REPORT_ERROR(USER_ERROR, "Unsupported itype for sparse data!\n");
            break;
          }
          case LSR: {
            if(itype == INT)
               pred = exrpc_async(fm_node,(parallel_lnrm_predict<DT2,S_MAT24,S_LMAT24,LSR2>),
                                  f_dptr,mid,prob).get(); 
            else if(itype == LONG)
               pred = exrpc_async(fm_node,(parallel_lnrm_predict<DT2,S_MAT25,S_LMAT25,LSR2>),
                                  f_dptr,mid,prob).get(); 
            else REPORT_ERROR(USER_ERROR, "Unsupported itype for sparse data!\n");
            break;
          }
          case LNRM: {
            if(itype == INT)
               pred = exrpc_async(fm_node,(parallel_lnrm_predict<DT2,S_MAT24,S_LMAT24,LNRM2>),
                                  f_dptr,mid,prob).get(); 
            else if(itype == LONG)
               pred = exrpc_async(fm_node,(parallel_lnrm_predict<DT2,S_MAT25,S_LMAT25,LNRM2>),
                                  f_dptr,mid,prob).get(); 
            else REPORT_ERROR(USER_ERROR, "Unsupported itype for sparse data!\n");
            break;
          }
          case DTM: {
            if(itype == INT)
               pred = exrpc_async(fm_node,(parallel_dtm_predict<DT2,S_MAT24,S_LMAT24>),
                                  f_dptr,mid,prob).get();  
            else if(itype == LONG)
               pred = exrpc_async(fm_node,(parallel_dtm_predict<DT2,S_MAT25,S_LMAT25>),
                                  f_dptr,mid,prob).get(); 
            else REPORT_ERROR(USER_ERROR, "Unsupported itype for sparse data!\n");
            break;
          }
          case FMM: {
            if(itype == INT)
              pred = exrpc_async(fm_node,(parallel_fmm_predict<DT2,S_MAT24,S_LMAT24>),
                                 f_dptr,mid,prob).get();
            else if(itype == LONG)
              pred = exrpc_async(fm_node,(parallel_fmm_predict<DT2,S_MAT25,S_LMAT25>),
                                 f_dptr,mid,prob).get();
            else REPORT_ERROR(USER_ERROR, "Unsupported itype for sparse data!\n");
            break;
          }
          case NBM: {
            if(itype == INT)
              pred = exrpc_async(fm_node,(parallel_generic_predict<DT2,S_MAT24,S_LMAT24,NBM2>),
                                 f_dptr,mid,prob).get(); 
            else if(itype == LONG)
              pred = exrpc_async(fm_node,(parallel_generic_predict<DT2,S_MAT25,S_LMAT25,NBM2>),
                                 f_dptr,mid,prob).get(); 
           else REPORT_ERROR(USER_ERROR, "Unsupported itype for sparse data!\n");
           break;
          }
          case RFM:  REPORT_ERROR(USER_ERROR, "currently Frovedis doesn't support sparse test data for Random Forest prediction!\n");
          case GBT:  REPORT_ERROR(USER_ERROR, "currently Frovedis doesn't support sparse test data for GBT prediction!\n");
          case KSVC:  REPORT_ERROR(USER_ERROR, "currently Frovedis doesn't support sparse test data for SVM Kernel prediction!\n");
          default:   REPORT_ERROR(USER_ERROR, "Unknown model kind is encountered!\n");
        }
      }
      auto sz = pred.size();
      checkAssumption(len == sz);
      for(size_t i = 0; i < sz; ++i) ret[i] = pred[i];
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  void parallel_double_glm_predict(const char* host, int port,
                                   int mid, short mkind, long dptr, 
                                   double* ret, ulong len, bool prob, 
                                   short itype , bool isDense) {
    if(!host) REPORT_ERROR(USER_ERROR, "Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    std::vector<double> pred;
    try {
      if(isDense) {
        switch(mkind) {
          case LR:  pred = exrpc_async(fm_node,(parallel_lrm_predict<DT1,R_MAT1,LR1>),
                                        f_dptr,mid,prob).get(); break;
          case SVM:  pred = exrpc_async(fm_node,(parallel_svm_predict<DT1,R_MAT1,R_LMAT1,SVM1>),
                                        f_dptr,mid,prob).get(); break;
          case SVR:  pred = exrpc_async(fm_node,(parallel_lnrm_predict<DT1,R_MAT1,R_LMAT1,SVR1>),
                                        f_dptr,mid,prob).get(); break;
          case KSVC:  pred = exrpc_async(fm_node,(ksvm_predict<DT1,R_MAT1,KSVC1>),
                                        f_dptr,mid,prob).get(); break;
          case RR: pred = exrpc_async(fm_node,(parallel_lnrm_predict<DT1,R_MAT1,R_LMAT1,RR1>),
                                       f_dptr,mid,prob).get(); break;
          case LSR: pred = exrpc_async(fm_node,(parallel_lnrm_predict<DT1,R_MAT1,R_LMAT1,LSR1>),
                                       f_dptr,mid,prob).get(); break;
          case LNRM: pred = exrpc_async(fm_node,(parallel_lnrm_predict<DT1,R_MAT1,R_LMAT1,LNRM1>),
                                        f_dptr,mid,prob).get(); break;
          case DTM:  pred = exrpc_async(fm_node,(parallel_dtm_predict<DT1,R_MAT1,R_LMAT1>),
                                        f_dptr,mid,prob).get(); break;
          case FMM:  REPORT_ERROR(USER_ERROR, "currently Frovedis doesn't support dense test data for FM!\n");
          case NBM:  pred = exrpc_async(fm_node,(parallel_generic_predict<DT1,R_MAT1,R_LMAT1,NBM1>),
                                        f_dptr,mid,prob).get(); break;
          case RFM:  pred = exrpc_async(fm_node,(parallel_rfm_predict<DT1,R_MAT1,R_LMAT1>),
                                        f_dptr,mid,prob).get(); break;
          case GBT:  pred = exrpc_async(fm_node,(parallel_gbt_predict<DT1,R_MAT1,R_LMAT1>),
                                        f_dptr,mid,prob).get(); break;
          default:   REPORT_ERROR(USER_ERROR, "Unknown model kind is encountered!\n");
        }
      }
      else {
        switch(mkind) {
          case LR: {
            if(itype == INT)
              pred = exrpc_async(fm_node,(parallel_lrm_predict<DT1,S_MAT14,LR1>),
                                 f_dptr,mid,prob).get();
            else if(itype == LONG)
              pred = exrpc_async(fm_node,(parallel_lrm_predict<DT1,S_MAT15,LR1>),
                                 f_dptr,mid,prob).get();
            else REPORT_ERROR(USER_ERROR, "Unsupported itype for sparse data!\n");
            break;
          }
          case SVM: {
            if(itype == INT)
              pred = exrpc_async(fm_node,(parallel_svm_predict<DT1,S_MAT14,S_LMAT14,SVM1>),
                                 f_dptr,mid,prob).get();
            else if(itype == LONG)
              pred = exrpc_async(fm_node,(parallel_svm_predict<DT1,S_MAT15,S_LMAT15,SVM1>),
                                 f_dptr,mid,prob).get();
            else REPORT_ERROR(USER_ERROR, "Unsupported itype for sparse data!\n");
            break;
          }
          case SVR: {
            if(itype == INT)
              pred = exrpc_async(fm_node,(parallel_lnrm_predict<DT1,S_MAT14,S_LMAT14,SVR1>),
                                 f_dptr,mid,prob).get();
            else if(itype == LONG)
              pred = exrpc_async(fm_node,(parallel_lnrm_predict<DT1,S_MAT15,S_LMAT15,SVR1>),
                                 f_dptr,mid,prob).get();
            else REPORT_ERROR(USER_ERROR, "Unsupported itype for sparse data!\n");
            break;
          }
          case RR: {
            if(itype == INT)
               pred = exrpc_async(fm_node,(parallel_lnrm_predict<DT1,S_MAT14,S_LMAT14,RR1>),
                                  f_dptr,mid,prob).get();
            else if(itype == LONG)
               pred = exrpc_async(fm_node,(parallel_lnrm_predict<DT1,S_MAT15,S_LMAT15,RR1>),
                                  f_dptr,mid,prob).get();
            else REPORT_ERROR(USER_ERROR, "Unsupported itype for sparse data!\n");
            break;
          }
          case LSR: {
            if(itype == INT)
               pred = exrpc_async(fm_node,(parallel_lnrm_predict<DT1,S_MAT14,S_LMAT14,LSR1>),
                                  f_dptr,mid,prob).get();
            else if(itype == LONG)
               pred = exrpc_async(fm_node,(parallel_lnrm_predict<DT1,S_MAT15,S_LMAT15,LSR1>),
                                  f_dptr,mid,prob).get();
            else REPORT_ERROR(USER_ERROR, "Unsupported itype for sparse data!\n");
            break;
          }
          case LNRM: {
            if(itype == INT)
               pred = exrpc_async(fm_node,(parallel_lnrm_predict<DT1,S_MAT14,S_LMAT14,LNRM1>),
                                  f_dptr,mid,prob).get();
            else if(itype == LONG)
               pred = exrpc_async(fm_node,(parallel_lnrm_predict<DT1,S_MAT15,S_LMAT15,LNRM1>),
                                  f_dptr,mid,prob).get();
            else REPORT_ERROR(USER_ERROR, "Unsupported itype for sparse data!\n");
            break;
          }
          case DTM: {
            if(itype == INT)
               pred = exrpc_async(fm_node,(parallel_dtm_predict<DT1,S_MAT14,S_LMAT14>),
                                  f_dptr,mid,prob).get();
             else if(itype == LONG)
               pred = exrpc_async(fm_node,(parallel_dtm_predict<DT1,S_MAT15,S_LMAT15>),
                                  f_dptr,mid,prob).get();
             else REPORT_ERROR(USER_ERROR, "Unsupported itype for sparse data!\n");
             break;
          }
          case FMM: {
            if(itype == INT)
              pred = exrpc_async(fm_node,(parallel_fmm_predict<DT1,S_MAT14,S_LMAT14>),
                                 f_dptr,mid,prob).get();
            else if(itype == LONG)
              pred = exrpc_async(fm_node,(parallel_fmm_predict<DT1,S_MAT15,S_LMAT15>),
                                 f_dptr,mid,prob).get();
            else REPORT_ERROR(USER_ERROR, "Unsupported itype for sparse data!\n");
            break;
          }
          case NBM: {
            if(itype == INT)
               pred = exrpc_async(fm_node,(parallel_generic_predict<DT1,S_MAT14,S_LMAT14,NBM1>),
                                  f_dptr,mid,prob).get();
            else if(itype == LONG)
               pred = exrpc_async(fm_node,(parallel_generic_predict<DT1,S_MAT15,S_LMAT15,NBM1>),
                                  f_dptr,mid,prob).get();
            else REPORT_ERROR(USER_ERROR, "Unsupported itype for sparse data!\n");
            break;
          }
          case RFM:  REPORT_ERROR(USER_ERROR, 
                     "currently Frovedis doesn't support sparse test data for Random Forest prediction!\n");
          case GBT:  REPORT_ERROR(USER_ERROR, 
                     "currently Frovedis doesn't support sparse test data for GBT prediction!\n");
          case KSVC: REPORT_ERROR(USER_ERROR, 
                     "currently Frovedis doesn't support sparse test data for SVM Kernel prediction!\n");
          default:   REPORT_ERROR(USER_ERROR, "Unknown model kind is encountered!\n");
        }
      }
      auto sz = pred.size();
      checkAssumption(len == sz);
      for(size_t i = 0; i < sz; ++i) ret[i] = pred[i];
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  void parallel_kmeans_predict(const char* host, int port,
                               int mid, short mdtype, 
                               long dptr, int* ret, ulong len, 
                               short itype , bool isDense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    std::vector<int> pred;
    try {
      if(isDense){
        switch(mdtype) {
          case FLOAT:
            pred = exrpc_async(fm_node,(frovedis_kmeans_predict<R_MAT2,KMM2>),
                               f_dptr,mid).get();
            break;
          case DOUBLE:
            pred = exrpc_async(fm_node,(frovedis_kmeans_predict<R_MAT1,KMM1>),
                               f_dptr,mid).get();
            break;
          default: REPORT_ERROR(USER_ERROR, "model dtype can be either float or double!\n");
         }
      }
      else {
        if(mdtype == FLOAT){
          switch(itype) {
            case INT:  
              pred = exrpc_async(fm_node,(frovedis_kmeans_predict<S_MAT24,KMM2>),
                                 f_dptr,mid).get();
              break;
            case LONG: 
              pred = exrpc_async(fm_node,(frovedis_kmeans_predict<S_MAT25,KMM2>),
                                 f_dptr,mid).get();
              break;
            default: REPORT_ERROR(USER_ERROR, "model itype can be either int or long!\n");
          }
        }
        else if(mdtype == DOUBLE){
          switch(itype) {
            case INT:
               pred = exrpc_async(fm_node,(frovedis_kmeans_predict<S_MAT14,KMM1>),
                                  f_dptr,mid).get();
               break;
            case LONG:
               pred = exrpc_async(fm_node,(frovedis_kmeans_predict<S_MAT15,KMM1>),
                                  f_dptr,mid).get();
               break;
            default: REPORT_ERROR(USER_ERROR, "model itype can be either int or long!\n");
           }
         } 
        else REPORT_ERROR(USER_ERROR, "model dtype can be either float or double!\n");
      }
      auto sz = pred.size();
      checkAssumption(len == sz);
      for(size_t i = 0; i < sz; ++i) ret[i] = pred[i];
    }  
    catch (std::exception& e) {
      set_status(true, e.what());
    } 
  }

  float kmeans_score(const char* host, int port,
                     int mid, short mdtype,
                     long dptr, short itype, bool isDense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host, port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    float score = 0.0;
    try {
      if(isDense){
        switch(mdtype) {
          case FLOAT:
            score = exrpc_async(fm_node,(frovedis_kmeans_score<R_MAT2,KMM2>),
                                f_dptr,mid).get();
            break;
          case DOUBLE:
            score = exrpc_async(fm_node,(frovedis_kmeans_score<R_MAT1,KMM1>),
                                f_dptr,mid).get();
            break;
          default: REPORT_ERROR(USER_ERROR, "model dtype can be either float or double!\n");
         }
      }
      else {
        if(mdtype == FLOAT) {
          switch(itype) {
            case INT:
              score = exrpc_async(fm_node,(frovedis_kmeans_score<S_MAT24,KMM2>),
                                  f_dptr,mid).get();
              break;
            case LONG:
              score = exrpc_async(fm_node,(frovedis_kmeans_score<S_MAT25,KMM2>),
                                  f_dptr,mid).get();
              break;
            default: REPORT_ERROR(USER_ERROR, "model itype can be either int or long!\n");
          }
        }
        else if(mdtype == DOUBLE){
          switch(itype) {
            case INT:
               score = exrpc_async(fm_node,(frovedis_kmeans_score<S_MAT14,KMM1>),
                                   f_dptr,mid).get();
               break;
            case LONG:
               score = exrpc_async(fm_node,(frovedis_kmeans_score<S_MAT15,KMM1>),
                                   f_dptr,mid).get();
               break;
            default: REPORT_ERROR(USER_ERROR, "model itype can be either int or long!\n");
           }
         }
        else REPORT_ERROR(USER_ERROR, "model dtype can be either float or double!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return score;
  }

  PyObject* kmeans_transform(const char* host, int port,
                             int mid, short mdtype,
                             long dptr, short itype, bool isDense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host, port);
    auto f_dptr = (exrpc_ptr_t) dptr;
    dummy_matrix dmat;
    try {
      if(isDense){
        switch(mdtype) {
          case FLOAT:
            dmat = exrpc_async(fm_node,(frovedis_kmeans_transform<DT2,R_MAT2,KMM2>),
                               f_dptr,mid).get();
            break;
          case DOUBLE:
            dmat = exrpc_async(fm_node,(frovedis_kmeans_transform<DT1,R_MAT1,KMM1>),
                               f_dptr,mid).get();
            break;
          default: REPORT_ERROR(USER_ERROR, "model dtype can be either float or double!\n");
         }
      }
      else {
        if(mdtype == FLOAT){
          switch(itype) {
            case INT:
              dmat = exrpc_async(fm_node,(frovedis_kmeans_transform<DT2,S_MAT24,KMM2>),
                                 f_dptr,mid).get();
              break;
            case LONG:
              dmat = exrpc_async(fm_node,(frovedis_kmeans_transform<DT2,S_MAT25,KMM2>),
                                 f_dptr,mid).get();
              break;
            default: REPORT_ERROR(USER_ERROR, "model itype can be either int or long!\n");
          }
        }
        else if(mdtype == DOUBLE){
          switch(itype) {
            case INT:
               dmat = exrpc_async(fm_node,(frovedis_kmeans_transform<DT1,S_MAT14,KMM1>),
                                  f_dptr,mid).get();
               break;
            case LONG:
               dmat = exrpc_async(fm_node,(frovedis_kmeans_transform<DT1,S_MAT15,KMM1>),
                                  f_dptr,mid).get();
               break;
            default: REPORT_ERROR(USER_ERROR, "model itype can be either int or long!\n");
           }
         }
        else REPORT_ERROR(USER_ERROR, "model dtype can be either float or double!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_matrix(dmat);
  }

  PyObject* get_kmeans_centroid(const char* host, int port,
                                int mid, short mdtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host, port);
    PyObject* ret = NULL;
    try {
      switch(mdtype) {
        case FLOAT: {
          auto center = exrpc_async(fm_node,(frovedis_kmeans_centroid<DT2,KMM2>),
                                    mid).get();
          ret = to_python_float_list(center);
          break;
        }
        case DOUBLE: {
          auto center = exrpc_async(fm_node,(frovedis_kmeans_centroid<DT1,KMM1>),
                                    mid).get();
          ret = to_python_double_list(center);
          break;
        }
        default: REPORT_ERROR(USER_ERROR, "model dtype can be either float or double!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return ret;
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
                           bool need_distance, short dtype, 
                           short itype, bool dense,
                           short modelitype, bool modeldense) {
      if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
      exrpc_node fm_node(host,port);
      auto test_dptr = (exrpc_ptr_t) tptr;
      knn_result res;
      try {
        if(dense && modeldense) {
          switch(dtype) {
            case FLOAT:
              res = exrpc_async(fm_node, (frovedis_kneighbors<DT2,DT5,R_MAT2,KNNR2>), test_dptr, 
                                mid, k, need_distance ).get();
              break;
            case DOUBLE:
              res = exrpc_async(fm_node, (frovedis_kneighbors<DT1,DT5,R_MAT1,KNNR1>), test_dptr, 
                                mid, k, need_distance ).get();
              break;
            default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input dense matrix!\n");
          }
        }
        else if(dense && !modeldense) {
          switch(dtype) {
            case FLOAT:
              if(modelitype == INT)
                res = exrpc_async(fm_node, (frovedis_kneighbors<DT2,DT5,R_MAT2,KNNS24>), test_dptr,
                                   mid, k, need_distance).get();
              else if(modelitype == LONG)
                res = exrpc_async(fm_node, (frovedis_kneighbors<DT2,DT5,R_MAT2,KNNS25>), test_dptr,
                                   mid, k, need_distance).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            case DOUBLE:
              if(modelitype == INT)
                res = exrpc_async(fm_node, (frovedis_kneighbors<DT1,DT5,R_MAT1,KNNS14>), test_dptr,
                                   mid, k, need_distance).get();
              else if(modelitype == LONG)
                res = exrpc_async(fm_node, (frovedis_kneighbors<DT1,DT5,R_MAT1,KNNS15>), test_dptr,
                                   mid, k, need_distance).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input sparse matrix!\n");
          }
        }

        else if(!dense && modeldense) {
          switch(dtype) {
            case FLOAT:
              if(itype == INT)
                res = exrpc_async(fm_node, (frovedis_kneighbors<DT2,DT5,S_MAT24,KNNR2>), test_dptr,
                                   mid, k, need_distance).get();
              else if(itype == LONG)
                res = exrpc_async(fm_node, (frovedis_kneighbors<DT2,DT5,S_MAT25,KNNR2>), test_dptr,
                                   mid, k, need_distance).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            case DOUBLE:
              if(itype == INT)
                res = exrpc_async(fm_node, (frovedis_kneighbors<DT1,DT5,S_MAT14,KNNR1>), test_dptr,
                                   mid, k, need_distance).get();
              else if(itype == LONG)
                res = exrpc_async(fm_node, (frovedis_kneighbors<DT1,DT5,S_MAT15,KNNR1>), test_dptr,
                                   mid, k, need_distance).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input dense matrix!\n");
          }
        }
        else {
           switch(dtype) {
            case FLOAT:
              if(itype == INT)
                res = exrpc_async(fm_node, (frovedis_kneighbors<DT2,DT5,S_MAT24,KNNS24>), test_dptr, 
                                mid, k, need_distance ).get();
              else if(itype == LONG)
                res = exrpc_async(fm_node, (frovedis_kneighbors<DT2,DT5,S_MAT25,KNNS25>), test_dptr, 
                                mid, k, need_distance ).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            case DOUBLE:
              if(itype == INT)
                res = exrpc_async(fm_node, (frovedis_kneighbors<DT1,DT5,S_MAT14,KNNS14>), test_dptr, 
                                mid, k, need_distance ).get();
              else if(itype == LONG)
                res = exrpc_async(fm_node, (frovedis_kneighbors<DT1,DT5,S_MAT15,KNNS15>), test_dptr, 
                                mid, k, need_distance ).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input sparse matrix!\n");
          }
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
                           bool need_distance, short dtype, 
                           short itype, bool dense,
                           short modelitype, bool modeldense) { 
      if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
      exrpc_node fm_node(host,port);
      auto test_dptr = (exrpc_ptr_t) tptr;
      knn_result res;
      try {
        if(dense && modeldense) {
          switch(dtype) {
            case FLOAT:
              res = exrpc_async(fm_node, (frovedis_kneighbors<DT2,DT5,R_MAT2,KNCR2>), test_dptr, 
                                mid, k, need_distance ).get();
              break;
            case DOUBLE:
              res = exrpc_async(fm_node, (frovedis_kneighbors<DT1,DT5,R_MAT1,KNCR1>), test_dptr, 
                                mid, k, need_distance ).get();
              break;
            default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input dense matrix!\n");
          }
        }
        else if(dense && !modeldense) {
          switch(dtype) {
            case FLOAT:
              if(modelitype == INT)
                res = exrpc_async(fm_node, (frovedis_kneighbors<DT2,DT5,R_MAT2,KNCS24>), test_dptr, 
                                  mid, k, need_distance ).get();
              else if(modelitype == LONG)
                res = exrpc_async(fm_node, (frovedis_kneighbors<DT2,DT5,R_MAT2,KNCS25>), test_dptr, 
                                  mid, k, need_distance ).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            case DOUBLE:
              if(modelitype == INT)
                res = exrpc_async(fm_node, (frovedis_kneighbors<DT1,DT5,R_MAT1,KNCS14>), test_dptr, 
                                  mid, k, need_distance ).get();
              else if(modelitype == LONG)
                res = exrpc_async(fm_node, (frovedis_kneighbors<DT1,DT5,R_MAT1,KNCS15>), test_dptr, 
                                  mid, k, need_distance ).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input dense matrix!\n");
          }
        }
        else if(!dense && modeldense) {
          switch(dtype) {
            case FLOAT:
              if(itype == INT)
                res = exrpc_async(fm_node, (frovedis_kneighbors<DT2,DT5,S_MAT24,KNCR2>), test_dptr, 
                                  mid, k, need_distance ).get();
              else if(itype == LONG)
                res = exrpc_async(fm_node, (frovedis_kneighbors<DT2,DT5,S_MAT25,KNCR2>), test_dptr, 
                                  mid, k, need_distance ).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            case DOUBLE:
              if(itype == INT)
                res = exrpc_async(fm_node, (frovedis_kneighbors<DT1,DT5,S_MAT14,KNCR1>), test_dptr, 
                                  mid, k, need_distance ).get();
              else if(itype == LONG)
                res = exrpc_async(fm_node, (frovedis_kneighbors<DT1,DT5,S_MAT15,KNCR1>), test_dptr, 
                                  mid, k, need_distance ).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input sparse matrix!\n");
          }
        }
        else {
          switch(dtype) {
            case FLOAT:
              if(itype == INT)
                res = exrpc_async(fm_node, (frovedis_kneighbors<DT2,DT5,S_MAT24,KNCS24>), test_dptr, 
                                  mid, k, need_distance ).get();
              else if(itype == LONG)
                res = exrpc_async(fm_node, (frovedis_kneighbors<DT2,DT5,S_MAT25,KNCS25>), test_dptr, 
                                  mid, k, need_distance ).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            case DOUBLE:
              if(itype == INT)
                res = exrpc_async(fm_node, (frovedis_kneighbors<DT1,DT5,S_MAT14,KNCS14>), test_dptr, 
                                  mid, k, need_distance ).get();
              else if(itype == LONG)
                res = exrpc_async(fm_node, (frovedis_kneighbors<DT1,DT5,S_MAT15,KNCS15>), test_dptr, 
                                  mid, k, need_distance ).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input sparse matrix!\n");
          }
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
                           bool need_distance, short dtype, 
                           short itype, bool dense,
                           short modelitype, bool modeldense) { 
      if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
      exrpc_node fm_node(host,port);
      auto test_dptr = (exrpc_ptr_t) tptr;
      knn_result res;
      try {
        if(dense && modeldense) {
          switch(dtype) {
            case FLOAT:
              res = exrpc_async(fm_node, (frovedis_kneighbors<DT2,DT5,R_MAT2,KNRR2>), test_dptr, 
                                mid, k, need_distance ).get();
              break;
            case DOUBLE:
              res = exrpc_async(fm_node, (frovedis_kneighbors<DT1,DT5,R_MAT1,KNRR1>), test_dptr, 
                                mid, k, need_distance ).get();
              break;
            default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input dense matrix!\n");
          }
        }
        else if(dense && !modeldense) {
          switch(dtype) {
            case FLOAT:
              if(modelitype == INT)
                res = exrpc_async(fm_node, (frovedis_kneighbors<DT2,DT5,R_MAT2,KNRS24>), test_dptr, 
                                  mid, k, need_distance ).get();
              else if(modelitype == LONG)
                res = exrpc_async(fm_node, (frovedis_kneighbors<DT2,DT5,R_MAT2,KNRS25>), test_dptr, 
                                  mid, k, need_distance ).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            case DOUBLE:
              if(modelitype == INT)
                res = exrpc_async(fm_node, (frovedis_kneighbors<DT1,DT5,R_MAT1,KNRS14>), test_dptr, 
                                  mid, k, need_distance ).get();
              else if(modelitype == LONG)
                res = exrpc_async(fm_node, (frovedis_kneighbors<DT1,DT5,R_MAT1,KNRS15>), test_dptr, 
                                  mid, k, need_distance ).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input dense matrix!\n");
          }
        }
        else if(!dense && modeldense) {
          switch(dtype) {
            case FLOAT:
              if(itype == INT)
                res = exrpc_async(fm_node, (frovedis_kneighbors<DT2,DT5,S_MAT24,KNRR2>), test_dptr, 
                                  mid, k, need_distance ).get();
              else if(itype == LONG)
                res = exrpc_async(fm_node, (frovedis_kneighbors<DT2,DT5,S_MAT25,KNRR2>), test_dptr, 
                                  mid, k, need_distance ).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            case DOUBLE:
              if(itype == INT)
                res = exrpc_async(fm_node, (frovedis_kneighbors<DT1,DT5,S_MAT14,KNRR1>), test_dptr, 
                                  mid, k, need_distance ).get();
              else if(itype == LONG)
                res = exrpc_async(fm_node, (frovedis_kneighbors<DT1,DT5,S_MAT15,KNRR1>), test_dptr, 
                                  mid, k, need_distance ).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input sparse matrix!\n");
          }
        }
        else {
          switch(dtype) {
            case FLOAT:
              if(itype == INT)
                res = exrpc_async(fm_node, (frovedis_kneighbors<DT2,DT5,S_MAT24,KNRS24>), test_dptr, 
                                  mid, k, need_distance ).get();
              else if(itype == LONG)
                res = exrpc_async(fm_node, (frovedis_kneighbors<DT2,DT5,S_MAT25,KNRS25>), test_dptr, 
                                  mid, k, need_distance ).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            case DOUBLE:
              if(itype == INT)
                res = exrpc_async(fm_node, (frovedis_kneighbors<DT1,DT5,S_MAT14,KNRS14>), test_dptr, 
                                  mid, k, need_distance ).get();
              else if(itype == LONG)
                res = exrpc_async(fm_node, (frovedis_kneighbors<DT1,DT5,S_MAT15,KNRS15>), test_dptr, 
                                  mid, k, need_distance ).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input sparse matrix!\n");
          }
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
                                 const char* mode, short dtype, 
                                 short itype, bool dense,
                                 short modelitype, bool modeldense) { 
      if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
      exrpc_node fm_node(host,port);
      auto test_dptr = (exrpc_ptr_t) tptr;
      std::string mode_ = mode;
      dummy_matrix dmat;
      try {
        if(dense && modeldense) {
          switch(dtype) {
            case FLOAT:
              dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,R_MAT2,KNNR2,S_MAT25,S_LMAT25>), test_dptr, 
                                 mid, k, mode_).get();
              break;
            case DOUBLE:
              dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,R_MAT1,KNNR1,S_MAT15,S_LMAT15>), test_dptr, 
                                 mid, k, mode_).get();
              break;
            default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input dense matrix!\n");
          }
        }
        else if(dense && !modeldense) {
          switch(dtype) {
            case FLOAT:
              if(modelitype == INT)
                dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,R_MAT2,KNNS24,S_MAT25,S_LMAT25>), test_dptr, 
                                   mid, k, mode_).get();
              else if(modelitype == LONG)
                dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,R_MAT2,KNNS25,S_MAT25,S_LMAT25>), test_dptr, 
                                   mid, k, mode_).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            case DOUBLE:
              if(modelitype == INT)
                dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,R_MAT1,KNNS14,S_MAT15,S_LMAT15>), test_dptr, 
                                   mid, k, mode_).get();
              else if(modelitype == LONG)
                dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,R_MAT1,KNNS15,S_MAT15,S_LMAT15>), test_dptr, 
                                   mid, k, mode_).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input dense matrix!\n");
          }
        }
        else if(!dense && modeldense) {
          switch(dtype) {
            case FLOAT:
              if(itype == INT)
                dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,S_MAT24,KNNR2,S_MAT25,S_LMAT25>), test_dptr, 
                                   mid, k, mode_).get();
              else if(itype == LONG)
                dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,S_MAT25,KNNR2,S_MAT25,S_LMAT25>), test_dptr, 
                                   mid, k, mode_).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            case DOUBLE:
              if(itype == INT)
                dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,S_MAT14,KNNR1,S_MAT15,S_LMAT15>), test_dptr, 
                                   mid, k, mode_).get();
              else if(itype == LONG)
                dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,S_MAT15,KNNR1,S_MAT15,S_LMAT15>), test_dptr, 
                                   mid, k, mode_).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input sparse matrix!\n");
          }
        }
        else {
          switch(dtype) {
            case FLOAT:
              if(itype == INT)
                dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,S_MAT24,KNNS24,S_MAT25,S_LMAT25>), test_dptr, 
                                   mid, k, mode_).get();
              else if(itype == LONG)
                dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,S_MAT25,KNNS25,S_MAT25,S_LMAT25>), test_dptr, 
                                   mid, k, mode_).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            case DOUBLE:
              if(itype == INT)
                dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,S_MAT14,KNNS14,S_MAT15,S_LMAT15>), test_dptr, 
                                   mid, k, mode_).get();
              else if(itype == LONG)
                dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,S_MAT15,KNNS15,S_MAT15,S_LMAT15>), test_dptr, 
                                   mid, k, mode_).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input sparse matrix!\n");
          }
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
                                 const char* mode, short dtype, 
                                 short itype, bool dense,
                                 short modelitype, bool modeldense) { 
      if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
      exrpc_node fm_node(host,port);
      auto test_dptr = (exrpc_ptr_t) tptr;
      std::string mode_ = mode;
      dummy_matrix dmat;
      try {
        if(dense && modeldense) {
          switch(dtype) {
            case FLOAT:
              dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,R_MAT2,KNCR2,S_MAT25,S_LMAT25>), test_dptr, 
                                 mid, k, mode_).get();
              break;
            case DOUBLE:
              dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,R_MAT1,KNCR1,S_MAT15,S_LMAT15>), test_dptr, 
                                 mid, k, mode_).get();
              break;
            default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input dense matrix!\n");
          }
        }
        else if(dense && !modeldense) {
          switch(dtype) {
            case FLOAT:
              if(modelitype == INT)
                dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,R_MAT2,KNCS24,S_MAT25,S_LMAT25>), test_dptr, 
                                   mid, k, mode_).get();
              else if(modelitype == LONG)
                dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,R_MAT2,KNCS25,S_MAT25,S_LMAT25>), test_dptr, 
                                   mid, k, mode_).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            case DOUBLE:
              if(modelitype == INT)
                dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,R_MAT1,KNCS14,S_MAT15,S_LMAT15>), test_dptr, 
                                   mid, k, mode_).get();
              else if(modelitype == LONG)
                dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,R_MAT1,KNCS15,S_MAT15,S_LMAT15>), test_dptr, 
                                   mid, k, mode_).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input dense matrix!\n");
          }
        }
        else if(!dense && modeldense) {
          switch(dtype) {
            case FLOAT:
              if(itype == INT)
                dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,S_MAT24,KNCR2,S_MAT25,S_LMAT25>), test_dptr, 
                                   mid, k, mode_).get();
              else if(itype == LONG)
                dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,S_MAT25,KNCR2,S_MAT25,S_LMAT25>), test_dptr, 
                                   mid, k, mode_).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            case DOUBLE:
              if(itype == INT)
                dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,S_MAT14,KNCR1,S_MAT15,S_LMAT15>), test_dptr, 
                                   mid, k, mode_).get();
              else if(itype == LONG)
                dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,S_MAT15,KNCR1,S_MAT15,S_LMAT15>), test_dptr, 
                                   mid, k, mode_).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input sparse matrix!\n");
          }
        }
        else {
          switch(dtype) {
            case FLOAT:
              if(itype == INT)
                dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,S_MAT24,KNCS24,S_MAT25,S_LMAT25>), test_dptr, 
                                   mid, k, mode_).get();
              else if(itype == LONG)
                dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,S_MAT25,KNCS25,S_MAT25,S_LMAT25>), test_dptr, 
                                   mid, k, mode_).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            case DOUBLE:
              if(itype == INT)
                dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,S_MAT14,KNCS14,S_MAT15,S_LMAT15>), test_dptr, 
                                   mid, k, mode_).get();
              else if(itype == LONG)
                dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,S_MAT15,KNCS15,S_MAT15,S_LMAT15>), test_dptr, 
                                   mid, k, mode_).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input sparse matrix!\n");
          }
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
                                 const char* mode, short dtype, 
                                 short itype, bool dense,
                                 short modelitype, bool modeldense) {
      if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
      exrpc_node fm_node(host,port);
      auto test_dptr = (exrpc_ptr_t) tptr;
      std::string mode_ = mode;
      dummy_matrix dmat;
      try {
        if(dense && modeldense) {
          switch(dtype) {
            case FLOAT:
              dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,R_MAT2,KNRR2,S_MAT25,S_LMAT25>), test_dptr, 
                                 mid, k, mode_).get();
              break;
            case DOUBLE:
              dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,R_MAT1,KNRR1,S_MAT15,S_LMAT15>), test_dptr, 
                                 mid, k, mode_).get();
              break;
            default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input dense matrix!\n");
          }
        }
        else if(dense && !modeldense) {
          switch(dtype) {
            case FLOAT:
              if(modelitype == INT)
                dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,R_MAT2,KNRS24,S_MAT25,S_LMAT25>), test_dptr, 
                                   mid, k, mode_).get();
              else if(modelitype == LONG)
                dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,R_MAT2,KNRS25,S_MAT25,S_LMAT25>), test_dptr, 
                                   mid, k, mode_).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            case DOUBLE:
              if(modelitype == INT)
                dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,R_MAT1,KNRS14,S_MAT15,S_LMAT15>), test_dptr, 
                                   mid, k, mode_).get();
              else if(modelitype == LONG)
                dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,R_MAT1,KNRS15,S_MAT15,S_LMAT15>), test_dptr, 
                                   mid, k, mode_).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input dense matrix!\n");
          }
        }
        else if(!dense && modeldense) {
          switch(dtype) {
            case FLOAT:
              if(itype == INT)
                dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,S_MAT24,KNRR2,S_MAT25,S_LMAT25>), test_dptr, 
                                   mid, k, mode_).get();
              else if(itype == LONG)
                dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,S_MAT25,KNRR2,S_MAT25,S_LMAT25>), test_dptr, 
                                   mid, k, mode_).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            case DOUBLE:
              if(itype == INT)
                dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,S_MAT14,KNRR1,S_MAT15,S_LMAT15>), test_dptr, 
                                   mid, k, mode_).get();
              else if(itype == LONG)
                dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,S_MAT15,KNRR1,S_MAT15,S_LMAT15>), test_dptr, 
                                   mid, k, mode_).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input sparse matrix!\n");
          }
        }
        else {
          switch(dtype) {
            case FLOAT:
              if(itype == INT)
                dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,S_MAT24,KNRS24,S_MAT25,S_LMAT25>), test_dptr, 
                                   mid, k, mode_).get();
              else if(itype == LONG)
                dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,S_MAT25,KNRS25,S_MAT25,S_LMAT25>), test_dptr, 
                                   mid, k, mode_).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            case DOUBLE:
              if(itype == INT)
                dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,S_MAT14,KNRS14,S_MAT15,S_LMAT15>), test_dptr, 
                                   mid, k, mode_).get();
              else if(itype == LONG)
                dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,S_MAT15,KNRS15,S_MAT15,S_LMAT15>), test_dptr, 
                                   mid, k, mode_).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input sparse matrix!\n");
          }
        }
      }
      catch (std::exception& e) {
        set_status(true, e.what());
      }
      return to_py_dummy_matrix(dmat);
    }

  PyObject* knn_radius_neighbors(const char* host, int port,
                                 long tptr, float radius, int mid,
                                 short dtype, 
                                 short itype, bool dense,
                                 short modelitype, bool modeldense) {
      if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
      exrpc_node fm_node(host,port);
      auto test_dptr = (exrpc_ptr_t) tptr;
      dummy_matrix dmat;
      try {
        if(dense && modeldense) {
          switch(dtype) {
            case FLOAT:
              dmat = exrpc_async(fm_node, (frovedis_radius_neighbors<DT5,R_MAT2,KNNR2,S_MAT25,S_LMAT25>), test_dptr, 
                                 mid, radius).get();
              break;
            case DOUBLE:
              dmat = exrpc_async(fm_node, (frovedis_radius_neighbors<DT5,R_MAT1,KNNR1,S_MAT15,S_LMAT15>), test_dptr, 
                                 mid, radius).get();
              break;
            default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input dense matrix!\n");
          }
        }
        else if(dense && !modeldense) {
          switch(dtype) {
            case FLOAT:
              if(modelitype == INT)
                dmat = exrpc_async(fm_node, (frovedis_radius_neighbors<DT5,R_MAT2,KNNS24,S_MAT25,S_LMAT25>), test_dptr,
                                 mid, radius).get();                
              else if(modelitype == LONG)
                dmat = exrpc_async(fm_node, (frovedis_radius_neighbors<DT5,R_MAT2,KNNS25,S_MAT25,S_LMAT25>), test_dptr,
                                 mid, radius).get();                
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            case DOUBLE:
              if(modelitype == INT)
                dmat = exrpc_async(fm_node, (frovedis_radius_neighbors<DT5,R_MAT1,KNNS14,S_MAT15,S_LMAT15>), test_dptr,
                                 mid, radius).get();                
              else if(modelitype == LONG)
                dmat = exrpc_async(fm_node, (frovedis_radius_neighbors<DT5,R_MAT1,KNNS15,S_MAT15,S_LMAT15>), test_dptr,
                                 mid, radius).get();                
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input dense matrix!\n");
          }
        }
        else if(!dense && modeldense) {
          switch(dtype) {
            case FLOAT:
              if(itype == INT)
                dmat = exrpc_async(fm_node, (frovedis_radius_neighbors<DT5,S_MAT24,KNNR2,S_MAT25,S_LMAT25>), test_dptr,
                                 mid, radius).get();                
              else if(itype == LONG)
                dmat = exrpc_async(fm_node, (frovedis_radius_neighbors<DT5,S_MAT25,KNNR2,S_MAT25,S_LMAT25>), test_dptr,
                                 mid, radius).get();                
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            case DOUBLE:
              if(itype == INT)
                dmat = exrpc_async(fm_node, (frovedis_radius_neighbors<DT5,S_MAT14,KNNR1,S_MAT15,S_LMAT15>), test_dptr,
                                 mid, radius).get();                
              else if(itype == LONG)
                dmat = exrpc_async(fm_node, (frovedis_radius_neighbors<DT5,S_MAT15,KNNR1,S_MAT15,S_LMAT15>), test_dptr,
                                 mid, radius).get();                
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input sparse matrix!\n");
          }
        }
        else {
           switch(dtype) {
            case FLOAT:
              if(itype == INT)
                dmat = exrpc_async(fm_node, (frovedis_radius_neighbors<DT5,S_MAT24,KNNS24,S_MAT25,S_LMAT25>), test_dptr, 
                                   mid, radius).get();
              else if(itype == LONG)
                dmat = exrpc_async(fm_node, (frovedis_radius_neighbors<DT5,S_MAT25,KNNS25,S_MAT25,S_LMAT25>), test_dptr, 
                                   mid, radius).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            case DOUBLE:
              if(itype == INT)
                dmat = exrpc_async(fm_node, (frovedis_radius_neighbors<DT5,S_MAT14,KNNS14,S_MAT15,S_LMAT15>), test_dptr, 
                                   mid, radius).get();
              else if(itype == LONG)
                dmat = exrpc_async(fm_node, (frovedis_radius_neighbors<DT5,S_MAT15,KNNS15,S_MAT15,S_LMAT15>), test_dptr, 
                                   mid, radius).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input sparse matrix!\n");
          }

        }
      }
      catch (std::exception& e) {
        set_status(true, e.what());
      }
      return to_py_dummy_matrix(dmat);
    }

  PyObject* knn_radius_neighbors_graph(const char* host, int port,
                                       long tptr, float radius, int mid,
                                       const char* mode, short dtype, 
                                       short itype, bool dense,
                                       short modelitype, bool modeldense) {
      if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
      exrpc_node fm_node(host,port);
      auto test_dptr = (exrpc_ptr_t) tptr;
      std::string mode_ = mode;
      dummy_matrix dmat;
      try {
        if(dense && modeldense) {
          switch(dtype) {
            case FLOAT:
              dmat = exrpc_async(fm_node, (frovedis_radius_neighbors_graph<DT5,R_MAT2,KNNR2,S_MAT25,S_LMAT25>),
                                 test_dptr, mid, radius, mode_).get();
              break;
            case DOUBLE:
              dmat = exrpc_async(fm_node, (frovedis_radius_neighbors_graph<DT5,R_MAT1,KNNR1,S_MAT15,S_LMAT15>), 
                                 test_dptr, mid, radius, mode_).get();
              break;
            default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input dense matrix!\n");
          }
        }
        else if(dense && !modeldense) {
          switch(dtype) {
            case FLOAT:
              if(modelitype == INT)
                dmat = exrpc_async(fm_node, (frovedis_radius_neighbors_graph<DT5,R_MAT2,KNNS24,S_MAT25,S_LMAT25>),
                                   test_dptr, mid, radius, mode_).get();
              else if(modelitype == LONG)
                dmat = exrpc_async(fm_node, (frovedis_radius_neighbors_graph<DT5,R_MAT2,KNNS25,S_MAT25,S_LMAT25>),
                                   test_dptr, mid, radius, mode_).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            case DOUBLE:
              if(modelitype == INT)
                dmat = exrpc_async(fm_node, (frovedis_radius_neighbors_graph<DT5,R_MAT1,KNNS14,S_MAT15,S_LMAT15>),
                                   test_dptr, mid, radius, mode_).get();
              else if(modelitype == LONG)
                dmat = exrpc_async(fm_node, (frovedis_radius_neighbors_graph<DT5,R_MAT1,KNNS15,S_MAT15,S_LMAT15>),
                                   test_dptr, mid, radius, mode_).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input dense matrix!\n");
          }
        }
        else if(!dense && modeldense) {
          switch(dtype) {
            case FLOAT:
              if(itype == INT)
                dmat = exrpc_async(fm_node, (frovedis_radius_neighbors_graph<DT5,S_MAT24,KNNR2,S_MAT25,S_LMAT25>),
                                   test_dptr, mid, radius, mode_).get();
              else if(itype == LONG)
                dmat = exrpc_async(fm_node, (frovedis_radius_neighbors_graph<DT5,S_MAT25,KNNR2,S_MAT25,S_LMAT25>),
                                   test_dptr, mid, radius, mode_).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            case DOUBLE:
              if(itype == INT)
                dmat = exrpc_async(fm_node, (frovedis_radius_neighbors_graph<DT5,S_MAT14,KNNR1,S_MAT15,S_LMAT15>),
                                   test_dptr, mid, radius, mode_).get();
              else if(itype == LONG)
                dmat = exrpc_async(fm_node, (frovedis_radius_neighbors_graph<DT5,S_MAT15,KNNR1,S_MAT15,S_LMAT15>),
                                   test_dptr, mid, radius, mode_).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input sparse matrix!\n");
          }
        }
        else {
           switch(dtype) {
            case FLOAT:
              if(itype == INT)
                dmat = exrpc_async(fm_node, (frovedis_radius_neighbors_graph<DT5,S_MAT24,KNNS24,S_MAT25,S_LMAT25>),
                                   test_dptr, mid, radius, mode_).get();
              else if(itype == LONG)
                dmat = exrpc_async(fm_node, (frovedis_radius_neighbors_graph<DT5,S_MAT25,KNNS25,S_MAT25,S_LMAT25>),
                                   test_dptr, mid, radius, mode_).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            case DOUBLE:
              if(itype == INT)
                dmat = exrpc_async(fm_node, (frovedis_radius_neighbors_graph<DT5,S_MAT14,KNNS14,S_MAT15,S_LMAT15>),
                                   test_dptr, mid, radius, mode_).get();
              else if(itype == LONG)
                dmat = exrpc_async(fm_node, (frovedis_radius_neighbors_graph<DT5,S_MAT15,KNNS15,S_MAT15,S_LMAT15>),
                                   test_dptr, mid, radius, mode_).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input sparse matrix!\n");
          }

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
                          double* ret, long ret_len, 
                          short itype, bool dense,
                          short modelitype, bool modeldense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto test_dptr = (exrpc_ptr_t) tptr;
    std::vector<double> label;
    try {
      if(dense && modeldense) {
        label = exrpc_async(fm_node, (frovedis_knc_predict<DT1,DT5,R_MAT1,KNCR1>), test_dptr, mid, save_proba).get(); 
      }
      else if(dense && !modeldense){
        if(modelitype == INT)
          label = exrpc_async(fm_node, (frovedis_knc_predict<DT1,DT5,R_MAT1,KNCS14>),
                  test_dptr, mid, save_proba).get(); 
        else if(modelitype == LONG)
          label = exrpc_async(fm_node, (frovedis_knc_predict<DT1,DT5,R_MAT1,KNCS15>),
                  test_dptr, mid, save_proba).get(); 
        else REPORT_ERROR(USER_ERROR,
             "Unsupported itype of input dense matrix!\n");
      }
      else if(!dense && modeldense){
        if(itype == INT)
          label = exrpc_async(fm_node, (frovedis_knc_predict<DT1,DT5,S_MAT14,KNCR1>),
                  test_dptr, mid, save_proba).get(); 
        else if(itype == LONG)
          label = exrpc_async(fm_node, (frovedis_knc_predict<DT1,DT5,S_MAT15,KNCR1>),
                  test_dptr, mid, save_proba).get(); 
        else REPORT_ERROR(USER_ERROR,
             "Unsupported itype of input sparse matrix!\n");
      }
      else {
        if(itype == INT)
          label = exrpc_async(fm_node, (frovedis_knc_predict<DT1,DT5,S_MAT14,KNCS14>),
                  test_dptr, mid, save_proba).get(); 
        else if(itype == LONG)
          label = exrpc_async(fm_node, (frovedis_knc_predict<DT1,DT5,S_MAT15,KNCS15>),
                  test_dptr, mid, save_proba).get(); 
        else REPORT_ERROR(USER_ERROR,
             "Unsupported itype of input sparse matrix!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    checkAssumption(label.size() == ret_len);
    for(size_t i = 0; i < ret_len; ++i) ret[i] = label[i];
  }

  void knc_float_predict(const char* host, int port, long tptr,
                         int mid, bool save_proba, 
                         float* ret, long ret_len, 
                         short itype, bool dense,
                         short modelitype, bool modeldense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto test_dptr = (exrpc_ptr_t) tptr;
    std::vector<float> label;
    try {
      if(dense && modeldense) {
        label = exrpc_async(fm_node, (frovedis_knc_predict<DT2,DT5,R_MAT2,KNCR2>), test_dptr, mid, save_proba).get(); 
      }
      else if(!dense && modeldense){
        if(itype == INT)
          label = exrpc_async(fm_node, (frovedis_knc_predict<DT2,DT5,S_MAT24,KNCR2>), test_dptr, mid, save_proba).get(); 
        else if(itype == LONG)
          label = exrpc_async(fm_node, (frovedis_knc_predict<DT2,DT5,S_MAT25,KNCR2>), test_dptr, mid, save_proba).get(); 
        else REPORT_ERROR(USER_ERROR,
             "Unsupported itype of input dense matrix!\n");
      }
      else if(dense && !modeldense){
        if(modelitype == INT)
          label = exrpc_async(fm_node, (frovedis_knc_predict<DT2,DT5,R_MAT2,KNCS24>), test_dptr, mid, save_proba).get(); 
        else if(modelitype == LONG)
          label = exrpc_async(fm_node, (frovedis_knc_predict<DT2,DT5,R_MAT2,KNCS25>), test_dptr, mid, save_proba).get(); 
        else REPORT_ERROR(USER_ERROR,
             "Unsupported itype of input sparse matrix!\n");
      }
      else {
        if(itype == INT)
          label = exrpc_async(fm_node, (frovedis_knc_predict<DT2,DT5,S_MAT24,KNCS24>), test_dptr, mid, save_proba).get(); 
        else if(itype == LONG)
          label = exrpc_async(fm_node, (frovedis_knc_predict<DT2,DT5,S_MAT25,KNCS25>), test_dptr, mid, save_proba).get(); 
        else REPORT_ERROR(USER_ERROR,
             "Unsupported itype of input sparse matrix!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    checkAssumption(label.size() == ret_len);
    for(size_t i = 0; i < ret_len; ++i) ret[i] = label[i];
  }

  void knr_double_predict(const char* host, int port, long tptr,
                          int mid, double* ret, long ret_len, 
                          short itype, bool dense,
                          short modelitype, bool modeldense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto test_dptr = (exrpc_ptr_t) tptr;
    std::vector<double> label;
    try {
      if(dense && modeldense) {
        label = exrpc_async(fm_node, (frovedis_knr_predict<DT1,DT5,R_MAT1,KNRR1>), test_dptr, mid).get(); 
      }
      else if(!dense && modeldense){
        if(itype == INT)
          label = exrpc_async(fm_node, (frovedis_knr_predict<DT1,DT5,S_MAT14,KNRR1>), test_dptr, mid).get(); 
        else if(itype == LONG)
          label = exrpc_async(fm_node, (frovedis_knr_predict<DT1,DT5,S_MAT15,KNRR1>), test_dptr, mid).get(); 
        else REPORT_ERROR(USER_ERROR,
             "Unsupported itype of input sparse matrix!\n");
      }
      else if(dense && !modeldense){
        if(modelitype == INT)
          label = exrpc_async(fm_node, (frovedis_knr_predict<DT1,DT5,R_MAT1,KNRS14>), test_dptr, mid).get(); 
        else if(modelitype == LONG)
          label = exrpc_async(fm_node, (frovedis_knr_predict<DT1,DT5,R_MAT1,KNRS15>), test_dptr, mid).get(); 
        else REPORT_ERROR(USER_ERROR,
             "Unsupported itype of input sparse matrix!\n");
      }
      else {
        if(itype == INT)
          label = exrpc_async(fm_node, (frovedis_knr_predict<DT1,DT5,S_MAT14,KNRS14>), test_dptr, mid).get(); 
        else if(itype == LONG)
          label = exrpc_async(fm_node, (frovedis_knr_predict<DT1,DT5,S_MAT15,KNRS15>), test_dptr, mid).get(); 
        else REPORT_ERROR(USER_ERROR,
             "Unsupported itype of input sparse matrix!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    checkAssumption(label.size() == ret_len);
    for(size_t i = 0; i < ret_len; ++i) ret[i] = label[i];
  }

  void knr_float_predict(const char* host, int port, long tptr,
                         int mid, float* ret, long ret_len, 
                         short itype, bool dense,
                         short modelitype, bool modeldense) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto test_dptr = (exrpc_ptr_t) tptr;
    std::vector<float> label;
    try {
      if(dense && modeldense) {
        label = exrpc_async(fm_node, (frovedis_knr_predict<DT2,DT5,R_MAT2,KNRR2>), test_dptr, mid).get(); 
      }
      else if(!dense && modeldense){
        if(itype == INT)
          label = exrpc_async(fm_node, (frovedis_knr_predict<DT2,DT5,S_MAT24,KNRR2>), test_dptr, mid).get(); 
        else if(itype == LONG)
          label = exrpc_async(fm_node, (frovedis_knr_predict<DT2,DT5,S_MAT25,KNRR2>), test_dptr, mid).get(); 
        else REPORT_ERROR(USER_ERROR,
             "Unsupported itype of input sparse matrix!\n");
      }
      else if(dense && !modeldense){
        if(modelitype == INT)
          label = exrpc_async(fm_node, (frovedis_knr_predict<DT2,DT5,R_MAT2,KNRS24>), test_dptr, mid).get(); 
        else if(modelitype == LONG)
          label = exrpc_async(fm_node, (frovedis_knr_predict<DT2,DT5,R_MAT2,KNRS25>), test_dptr, mid).get(); 
        else REPORT_ERROR(USER_ERROR,
             "Unsupported itype of input sparse matrix!\n");
      }
      else if(!dense && modeldense){
        if(itype == INT)
          label = exrpc_async(fm_node, (frovedis_knr_predict<DT2,DT5,S_MAT24,KNRS24>), test_dptr, mid).get(); 
        else if(itype == LONG)
          label = exrpc_async(fm_node, (frovedis_knr_predict<DT2,DT5,S_MAT25,KNRS25>), test_dptr, mid).get(); 
        else REPORT_ERROR(USER_ERROR,
             "Unsupported itype of input sparse matrix!\n");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    checkAssumption(label.size() == ret_len);
    for(size_t i = 0; i < ret_len; ++i) ret[i] = label[i];
  }

  // knc predict proba
  PyObject* knc_predict_proba(const char* host, int port,
                              long tptr, int mid, short dtype, 
                              short itype, bool dense,
                              short modelitype, bool modeldense) {
      if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
      exrpc_node fm_node(host,port);
      auto test_dptr = (exrpc_ptr_t) tptr;
      dummy_matrix dmat;
      try {
        if(dense && modeldense) {
          switch(dtype) {
            case FLOAT:
              dmat = exrpc_async(fm_node, (frovedis_knc_predict_proba<DT5,R_MAT2,KNCR2,R_MAT2,R_LMAT2>), 
                                 test_dptr, mid).get();
              break;
            case DOUBLE:
              dmat = exrpc_async(fm_node, (frovedis_knc_predict_proba<DT5,R_MAT1,KNCR1,R_MAT1,R_LMAT1>), 
                                 test_dptr, mid).get();
              break;
            default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input dense matrix!\n");
          }
        }
        else if(dense && !modeldense) {
          switch(dtype) {
            case FLOAT:
              if(modelitype == INT)
                dmat = exrpc_async(fm_node, (frovedis_knc_predict_proba<DT5,R_MAT2,KNCS24,R_MAT2,R_LMAT2>), 
                                   test_dptr, mid).get();
              else if(modelitype == LONG)
                dmat = exrpc_async(fm_node, (frovedis_knc_predict_proba<DT5,R_MAT2,KNCS25,R_MAT2,R_LMAT2>), 
                                   test_dptr, mid).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            case DOUBLE:
              if(modelitype == INT)
                dmat = exrpc_async(fm_node, (frovedis_knc_predict_proba<DT5,R_MAT1,KNCS14,R_MAT1,R_LMAT1>), 
                                   test_dptr, mid).get();
              else if(modelitype == LONG)
                dmat = exrpc_async(fm_node, (frovedis_knc_predict_proba<DT5,R_MAT1,KNCS15,R_MAT1,R_LMAT1>), 
                                   test_dptr, mid).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input dense matrix!\n");
          }
        }
        else if(!dense && modeldense) {
          switch(dtype) {
            case FLOAT:
              if(itype == INT)
                dmat = exrpc_async(fm_node, (frovedis_knc_predict_proba<DT5,S_MAT24,KNCR2,R_MAT2,R_LMAT2>), 
                                   test_dptr, mid).get();
              else if(itype == LONG)
                dmat = exrpc_async(fm_node, (frovedis_knc_predict_proba<DT5,S_MAT25,KNCR2,R_MAT2,R_LMAT2>), 
                                   test_dptr, mid).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            case DOUBLE:
              if(itype == INT)
                dmat = exrpc_async(fm_node, (frovedis_knc_predict_proba<DT5,S_MAT14,KNCR1,R_MAT1,R_LMAT1>), 
                                   test_dptr, mid).get();
              else if(itype == LONG)
                dmat = exrpc_async(fm_node, (frovedis_knc_predict_proba<DT5,S_MAT15,KNCR1,R_MAT1,R_LMAT1>), 
                                   test_dptr, mid).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input sparse matrix!\n");
          }
        }
        else {
          switch(dtype) {
            case FLOAT:
              if(itype == INT)
              dmat = exrpc_async(fm_node, (frovedis_knc_predict_proba<DT5,S_MAT24,KNCS24,R_MAT2,R_LMAT2>), 
                                 test_dptr, mid).get();
              else if(itype == LONG)
              dmat = exrpc_async(fm_node, (frovedis_knc_predict_proba<DT5,S_MAT25,KNCS25,R_MAT2,R_LMAT2>), 
                                 test_dptr, mid).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            case DOUBLE:
              if(itype == INT)
              dmat = exrpc_async(fm_node, (frovedis_knc_predict_proba<DT5,S_MAT14,KNCS14,R_MAT1,R_LMAT1>), 
                                 test_dptr, mid).get();
              else if(itype == LONG)
              dmat = exrpc_async(fm_node, (frovedis_knc_predict_proba<DT5,S_MAT15,KNCS15,R_MAT1,R_LMAT1>), 
                                 test_dptr, mid).get();
              else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
              break;
            default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input sparse matrix!\n");
          }
        }
      }
      catch (std::exception& e) {
        set_status(true, e.what());
      }
      return to_py_dummy_matrix(dmat);
    }

  float knr_model_score(const char* host, int port, long xptr,
                        long yptr, int mid, short dtype, 
                        short itype, bool dense,
                        short modelitype, bool modeldense){
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto mptr = (exrpc_ptr_t) xptr;
    auto lblptr = (exrpc_ptr_t) yptr;
    float res = 0.0;
    try {
      if(dense && modeldense) {
        switch(dtype) {
          case FLOAT:
            res = exrpc_async(fm_node, (frovedis_model_score<DT2,DT5,R_MAT2,KNRR2>), mptr, lblptr, mid).get();
            break;
          case DOUBLE:
            res = exrpc_async(fm_node, (frovedis_model_score<DT1,DT5,R_MAT1,KNRR1>), mptr, lblptr, mid).get();
            break;
          default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input dense matrix!\n");
        }
      }
      else  if(dense && ! modeldense){
        switch(dtype) {
          case FLOAT:
            if(modelitype == INT)
            res = exrpc_async(fm_node, (frovedis_model_score<DT2,DT5,R_MAT2,KNRS24>), mptr, lblptr, mid).get();
            else if(modelitype == LONG)
            res = exrpc_async(fm_node, (frovedis_model_score<DT2,DT5,R_MAT2,KNRS25>), mptr, lblptr, mid).get();
            else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
            break;
          case DOUBLE:
            if(modelitype == INT)
            res = exrpc_async(fm_node, (frovedis_model_score<DT1,DT5,R_MAT1,KNRS14>), mptr, lblptr, mid).get();
            else if(modelitype == LONG)
            res = exrpc_async(fm_node, (frovedis_model_score<DT1,DT5,R_MAT1,KNRS15>), mptr, lblptr, mid).get();
            else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
            break;
          default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input sparse matrix!\n");
        }
      }
      else if(!dense && modeldense){
        switch(dtype) {
          case FLOAT:
            if(itype == INT)
            res = exrpc_async(fm_node, (frovedis_model_score<DT2,DT5,S_MAT24,KNRR2>), mptr, lblptr, mid).get();
            else if(itype == LONG)
            res = exrpc_async(fm_node, (frovedis_model_score<DT2,DT5,S_MAT25,KNRR2>), mptr, lblptr, mid).get();
            else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
            break;
          case DOUBLE:
            if(itype == INT)
            res = exrpc_async(fm_node, (frovedis_model_score<DT1,DT5,S_MAT14,KNRR1>), mptr, lblptr, mid).get();
            else if(itype == LONG)
            res = exrpc_async(fm_node, (frovedis_model_score<DT1,DT5,S_MAT15,KNRR1>), mptr, lblptr, mid).get();
            else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
            break;
          default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input sparse matrix!\n");
        }
      }
      else {
        switch(dtype) {
          case FLOAT:
            if(itype == INT)
            res = exrpc_async(fm_node, (frovedis_model_score<DT2,DT5,S_MAT24,KNRS24>), mptr, lblptr, mid).get();
            else if(itype == LONG)
            res = exrpc_async(fm_node, (frovedis_model_score<DT2,DT5,S_MAT25,KNRS25>), mptr, lblptr, mid).get();
            else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
            break;
          case DOUBLE:
            if(itype == INT)
            res = exrpc_async(fm_node, (frovedis_model_score<DT1,DT5,S_MAT14,KNRS14>), mptr, lblptr, mid).get();
            else if(itype == LONG)
            res = exrpc_async(fm_node, (frovedis_model_score<DT1,DT5,S_MAT15,KNRS15>), mptr, lblptr, mid).get();
            else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
            break;
          default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input sparse matrix!\n");
        }
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return res;
  }

  float knc_model_score(const char* host, int port, long xptr,
                    long yptr, int mid, short dtype, 
                    short itype, bool dense,
                    short modelitype, bool modeldense){
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    auto mptr = (exrpc_ptr_t) xptr;
    auto lblptr = (exrpc_ptr_t) yptr;
    float res = 0 ;
    try {
      if(dense && modeldense) {
        switch(dtype) {
          case FLOAT:
            res = exrpc_async(fm_node, (frovedis_model_score<DT2,DT5,R_MAT2,KNCR2>), mptr, lblptr, mid).get();
            break;
          case DOUBLE:
            res = exrpc_async(fm_node, (frovedis_model_score<DT1,DT5,R_MAT1,KNCR1>), mptr, lblptr, mid).get();
            break;
          default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input dense matrix!\n");
        }
      }
      else if(dense && !modeldense){
        switch(dtype) {
          case FLOAT:
            if(modelitype == INT)
              res = exrpc_async(fm_node, (frovedis_model_score<DT2,DT5,R_MAT2,KNCS24>), mptr, lblptr, mid).get();
            else if(modelitype == LONG)
              res = exrpc_async(fm_node, (frovedis_model_score<DT2,DT5,R_MAT2,KNCS25>), mptr, lblptr, mid).get();
            else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
            break;
          case DOUBLE:
            if(modelitype == INT)
              res = exrpc_async(fm_node, (frovedis_model_score<DT1,DT5,R_MAT1,KNCS14>), mptr, lblptr, mid).get();
            else if(modelitype == LONG)
              res = exrpc_async(fm_node, (frovedis_model_score<DT1,DT5,R_MAT1,KNCS14>), mptr, lblptr, mid).get();
            else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
            break;
          default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input sparse matrix!\n");
        }
      }
      else if(!dense && modeldense){
        switch(dtype) {
          case FLOAT:
            if(itype == INT)
              res = exrpc_async(fm_node, (frovedis_model_score<DT2,DT5,S_MAT24,KNCR2>), mptr, lblptr, mid).get();
            else if(itype == LONG)
              res = exrpc_async(fm_node, (frovedis_model_score<DT2,DT5,S_MAT25,KNCR2>), mptr, lblptr, mid).get();
            else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
            break;
          case DOUBLE:
            if(itype == INT)
              res = exrpc_async(fm_node, (frovedis_model_score<DT1,DT5,S_MAT14,KNCR1>), mptr, lblptr, mid).get();
            else if(itype == LONG)
              res = exrpc_async(fm_node, (frovedis_model_score<DT1,DT5,S_MAT15,KNCR1>), mptr, lblptr, mid).get();
            else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
            break;
          default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input sparse matrix!\n");
        }
      }
      else {
        switch(dtype) {
          case FLOAT:
            if(itype == INT)
            res = exrpc_async(fm_node, (frovedis_model_score<DT2,DT5,S_MAT24,KNCS24>), mptr, lblptr, mid).get();
            else if(itype == LONG)
            res = exrpc_async(fm_node, (frovedis_model_score<DT2,DT5,S_MAT25,KNCS25>), mptr, lblptr, mid).get();
            else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
            break;
          case DOUBLE:
            if(itype == INT)
            res = exrpc_async(fm_node, (frovedis_model_score<DT1,DT5,S_MAT14,KNCS14>), mptr, lblptr, mid).get();
            else if(itype == LONG)
            res = exrpc_async(fm_node, (frovedis_model_score<DT1,DT5,S_MAT15,KNCS15>), mptr, lblptr, mid).get();
            else REPORT_ERROR(USER_ERROR,
                 "Unsupported itype of input sparse data!\n");
            break;
          default: REPORT_ERROR(USER_ERROR,"Unsupported dtype for input sparse matrix!\n");
        }
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
          ret = exrpc_async(fm_node,get_lda_component<LDA4>, mid).get();
          break;
        case LONG:
          ret = exrpc_async(fm_node,get_lda_component<LDA3>, mid).get();
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

  PyObject* get_frovedis_support_vector(const char* host, int port,
                                        int mid, short mkind,
                                        short mdtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host, port);
    PyObject* ret_ptr = NULL;
    try {
      if (mdtype == FLOAT) {
      std::vector<float> ret;
        switch(mkind) {
          case KSVC: ret = exrpc_async(fm_node, (get_support_vector<DT2,KSVC2>), mid).get(); break;
          default: REPORT_ERROR(USER_ERROR, "Unknown model for support vector extraction!\n");
        }
        ret_ptr = to_python_float_list(ret);
      }
      else if (mdtype == DOUBLE) {
      std::vector<double> ret;
        switch(mkind) {
          case KSVC: ret = exrpc_async(fm_node, (get_support_vector<DT1,KSVC1>), mid).get(); break;
          default: REPORT_ERROR(USER_ERROR, "Unknown model for support index extraction!\n");
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

  PyObject* get_frovedis_support_idx(const char* host, int port,
                                     int mid, short mkind,
                                     short mdtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host, port);
    std::vector<size_t> ret;
    try {
      if (mdtype == FLOAT) {
        switch(mkind) {
          case KSVC: ret = exrpc_async(fm_node, (get_support_idx<KSVC2>), mid).get(); break;
          default: REPORT_ERROR(USER_ERROR, "Unknown model for support index extraction!\n");
        }
      }
      else if (mdtype == DOUBLE) {
        switch(mkind) {
          case KSVC: ret = exrpc_async(fm_node, (get_support_idx<KSVC1>), mid).get(); break;
          default: REPORT_ERROR(USER_ERROR, "Unknown model for support index extraction!\n");
        }
      }
      else REPORT_ERROR(USER_ERROR,"model dtype can either be float or double!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_python_llong_list(ret);
  }

  PyObject* get_frovedis_dbscan_core_sample_indices(const char* host, int port,
                                     int mid, short mkind,
                                     short mdtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host, port);
    std::vector<size_t> ret;
    try {
      if (mdtype == FLOAT) {
        ret = exrpc_async(fm_node, (get_dbscan_core_sample_indices<DBSCAN2>), mid).get();        
      }
      else if (mdtype == DOUBLE) {
        ret = exrpc_async(fm_node, (get_dbscan_core_sample_indices<DBSCAN1>), mid).get();
      }
      else REPORT_ERROR(USER_ERROR,"model dtype can either be float or double!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_python_llong_list(ret);
  }

  PyObject* get_frovedis_dbscan_components(const char* host, int port,
                                     int mid, short mkind,
                                     short mdtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host, port);
    dummy_matrix ret;
    try {
      if (mdtype == FLOAT)
        ret = exrpc_async(fm_node, (get_dbscan_components<DBSCAN2, DT2>), mid).get();        
      else if (mdtype == DOUBLE)
        ret = exrpc_async(fm_node, (get_dbscan_components<DBSCAN1, DT1>), mid).get();
      else REPORT_ERROR(USER_ERROR,"model dtype can either be float or double!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_matrix(ret);
  }
    
// scaler mean
  PyObject* get_scaler_mean_vector(const char* host, int port,
                                    int mid, short mdtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    PyObject* ret_ptr = NULL;
    try {
      if(mdtype == FLOAT) {
        std::vector<float> mean_vector;
        mean_vector = exrpc_async(fm_node,get_scaler_mean<DT2>,mid).get(); 
        ret_ptr = to_python_float_list(mean_vector);
      } 
      else if(mdtype == DOUBLE) {
        std::vector<double> mean_vector;
        mean_vector = exrpc_async(fm_node,get_scaler_mean<DT1>,mid).get(); 
        ret_ptr =  to_python_double_list(mean_vector); 
      } 
      else REPORT_ERROR(USER_ERROR,"model dtype can either be float or double!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
     
    return ret_ptr;     
  }

  // scaler var
  PyObject* get_scaler_var_vector(const char* host, int port,
                                    int mid, short mdtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    PyObject* ret_ptr = NULL;
    try {
      if(mdtype == FLOAT) {
        std::vector<float> var_vector;
        var_vector = exrpc_async(fm_node,get_scaler_var<DT2>,mid).get();
        ret_ptr = to_python_float_list(var_vector);
      }
      else if(mdtype == DOUBLE) {
        std::vector<double> var_vector;
        var_vector = exrpc_async(fm_node,get_scaler_var<DT1>,mid).get();
        ret_ptr =  to_python_double_list(var_vector);
      }
      else REPORT_ERROR(USER_ERROR,"model dtype can either be float or double!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return ret_ptr;
  }

  // scaler std
  PyObject* get_scaler_std_vector(const char* host, int port,
                                    int mid, short mdtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    PyObject* ret_ptr = NULL;
    try {
      if(mdtype == FLOAT) {
        std::vector<float> std_vector;
        std_vector = exrpc_async(fm_node,get_scaler_std<DT2>,mid).get();
        ret_ptr = to_python_float_list(std_vector);
      }
      else if(mdtype == DOUBLE) {
        std::vector<double> std_vector;
        std_vector = exrpc_async(fm_node,get_scaler_std<DT1>,mid).get();
        ret_ptr =  to_python_double_list(std_vector);
      }
      else REPORT_ERROR(USER_ERROR,"model dtype can either be float or double!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return ret_ptr;
  }

  //ARIMA    
  void get_fitted_vector_float(const char* host, int port,
                               int mid, short mkind,
                               short mdtype, float* ret) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host, port);
    std::vector<double> fitted_vec;
    try {
      if (mkind != ARM) REPORT_ERROR(USER_ERROR, 
         "Unknown model for fitted_values vector extraction!\n");
      if (mdtype == FLOAT) {
        std::vector<float> fitted_vec;
        fitted_vec = exrpc_async(fm_node, (get_fitted_vector<DT2>), mid).get();
        auto sz = fitted_vec.size();
        for(size_t i = 0; i < sz; ++i) ret[i] = fitted_vec[i];
      }
      else REPORT_ERROR(USER_ERROR,
                        "model dtype can either be float or double!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  void get_fitted_vector_double(const char* host, int port,
                                int mid, short mkind,
                                short mdtype, double* ret) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host, port);
    std::vector<double> fitted_vec;
    try {
      if (mkind != ARM) REPORT_ERROR(USER_ERROR, 
         "Unknown model for fitted_values vector extraction!\n");
      if (mdtype == DOUBLE) {
        std::vector<double> fitted_vec;
        fitted_vec = exrpc_async(fm_node, (get_fitted_vector<DT1>), mid).get();
        auto sz = fitted_vec.size();
        for(size_t i = 0; i < sz; ++i) ret[i] = fitted_vec[i];
      }
      else REPORT_ERROR(USER_ERROR,
                        "model dtype can either be float or double!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  PyObject* arima_predict(const char* host, int port, ulong start, 
                          ulong stop, int mid, short mdtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    PyObject* ret_ptr = NULL;
    try {
      if (mdtype == FLOAT) {
        std::vector<float> pred;
        pred = exrpc_async(fm_node, frovedis_arima_predict<DT2>, 
                           mid, start, stop).get();
        ret_ptr = to_python_float_list(pred);
      }
      else if (mdtype == DOUBLE) {
        std::vector<double> pred;
        pred = exrpc_async(fm_node, frovedis_arima_predict<DT1>, 
                           mid, start, stop).get();
        ret_ptr = to_python_double_list(pred);
      }
      else REPORT_ERROR(USER_ERROR,
                        "model dtype can either be float or double!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return ret_ptr;
  }

  PyObject* arima_forecast(const char* host, int port, ulong steps, 
                           int mid, short mdtype) {
    if(!host) REPORT_ERROR(USER_ERROR,"Invalid hostname!!");
    exrpc_node fm_node(host,port);
    PyObject* ret_ptr = NULL;
    try {
      if (mdtype == FLOAT) {
        std::vector<float> fc;
        fc = exrpc_async(fm_node, frovedis_arima_forecast<DT2>, 
                         mid, steps).get();
        ret_ptr = to_python_float_list(fc);
      }
      else if (mdtype == DOUBLE) {
        std::vector<double> fc;
        fc = exrpc_async(fm_node, frovedis_arima_forecast<DT1>, 
                         mid, steps).get();
        ret_ptr = to_python_double_list(fc);
      }
      else REPORT_ERROR(USER_ERROR,
                        "model dtype can either be float or double!\n");
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return ret_ptr;
  }    
}
