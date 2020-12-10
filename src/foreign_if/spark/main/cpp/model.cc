#include "exrpc_model.hpp"
#include "short_hand_dense_type.hpp"
#include "short_hand_sparse_type.hpp"
#include "short_hand_model_type.hpp"
#include "spark_client_headers.hpp"

using namespace frovedis;

extern "C" {

// prints the requested (registered) trained LR model information for debugging purpose
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_showFrovedisModel
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid, jshort mkind) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  try {
    switch(mkind) {
      case LRM:    exrpc_oneway(fm_node, show_model<LRM1>, mid); break;
      case MLR:    exrpc_oneway(fm_node, show_model<MLR1>, mid); break;
      case SVM:    exrpc_oneway(fm_node, show_model<SVM1>, mid); break;
      case SVR:    exrpc_oneway(fm_node, show_model<SVR1>, mid); break;
      case KSVC:   exrpc_oneway(fm_node, show_model<KSVC1>, mid); break;
      case LNRM:   exrpc_oneway(fm_node, show_model<LNRM1>, mid); break;
      case MFM:    exrpc_oneway(fm_node, show_model<MFM1>, mid); break;
      case KMEANS: exrpc_oneway(fm_node, show_model<KMM1>, mid); break;
      case DTM:    exrpc_oneway(fm_node, show_model<DTM1>, mid); break;
      case GBT:    exrpc_oneway(fm_node, show_model<GBT1>, mid); break;
      case RFM:    exrpc_oneway(fm_node, show_model<RFM1>, mid); break;
      case FPM:    exrpc_oneway(fm_node, show_model<FPM1>, mid); break;
      case FPR:    exrpc_oneway(fm_node, show_model<FPR1>, mid); break;
      case SEM:    exrpc_oneway(fm_node, show_model<SEM1>, mid); break;
      case SCM:    exrpc_oneway(fm_node, show_model<SCM1>, mid); break;
      case ACM:    exrpc_oneway(fm_node, show_model<ACM1>, mid); break;
      case FMM:    REPORT_ERROR(USER_ERROR, "currently Frovedis fm_model cant be displayed!");
      case NBM:    exrpc_oneway(fm_node, show_model<NBM1>, mid); break;      
      default:     REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
   }
 }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// releases the requested (registered) trained LR model from the heap
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_releaseFrovedisModel
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid, jshort mkind) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  try{
    switch(mkind) {
      case LRM:    exrpc_oneway(fm_node, release_model<LRM1>, mid); break;
      case MLR:    exrpc_oneway(fm_node, release_model<MLR1>, mid); break;
      case SVM:    exrpc_oneway(fm_node, release_model<SVM1>, mid); break;
      case SVR:    exrpc_oneway(fm_node, release_model<SVR1>, mid); break;
      case KSVC:   exrpc_oneway(fm_node, release_model<KSVC1>, mid); break;
      case LNRM:   exrpc_oneway(fm_node, release_model<LNRM1>, mid); break;
      case MFM:    exrpc_oneway(fm_node, release_model<MFM1>, mid); break;
      case KMEANS: exrpc_oneway(fm_node, release_model<KMM1>, mid); break;
      case DTM:    exrpc_oneway(fm_node, release_model<DTM1>, mid); break;
      case GBT:    exrpc_oneway(fm_node, release_model<GBT1>, mid); break;
      case RFM:    exrpc_oneway(fm_node, release_model<RFM1>, mid); break;
      case FPM:    exrpc_oneway(fm_node, release_model<FPM1>, mid); break;
      case FPR:    exrpc_oneway(fm_node, release_model<FPR1>, mid); break;
      case SEM:    exrpc_oneway(fm_node, release_model<SEM1>, mid); break;
      case SCM:    exrpc_oneway(fm_node, release_model<SCM1>, mid); break;
      case ACM:    exrpc_oneway(fm_node, release_model<ACM1>, mid); break;
      case FMM:    exrpc_oneway(fm_node, release_model<FMM1>, mid); break;
      case NBM:    exrpc_oneway(fm_node, release_model<NBM1>, mid); break;
      case W2V:    exrpc_oneway(fm_node, release_model<W2V1>, mid); break;
      case DBSCAN: exrpc_oneway(fm_node, release_model<DBSCAN1>, mid); break;
      case KNN:    exrpc_oneway(fm_node, release_model<KNN1>, mid); break;
      case KNC:    exrpc_oneway(fm_node, release_model<KNC1>, mid); break;
      case KNR:    exrpc_oneway(fm_node, release_model<KNR1>, mid); break;
      case LDASP:  exrpc_oneway(fm_node, release_model<LDASP3>, mid); break;
      default:     REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
} 

// saves the model to the specified file 
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_saveFrovedisModel
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid, jshort mkind, jstring path) {

  auto fs_path = to_cstring(env,path);
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  try {
    switch(mkind) {
      case LRM:    exrpc_oneway(fm_node,save_model<LRM1>,mid,fs_path); break;
      case MLR:    exrpc_oneway(fm_node,save_model<MLR1>,mid,fs_path); break;
      case SVM:    exrpc_oneway(fm_node,save_model<SVM1>,mid,fs_path); break;
      case SVR:    exrpc_oneway(fm_node,save_model<SVR1>,mid,fs_path); break;
      case KSVC:   exrpc_oneway(fm_node,save_model<KSVC1>,mid,fs_path); break;
      case LNRM:   exrpc_oneway(fm_node,save_model<LNRM1>,mid,fs_path); break;
      case MFM:    exrpc_oneway(fm_node,save_model<MFM1>,mid,fs_path); break;
      case KMEANS: exrpc_oneway(fm_node,save_model<KMM1>,mid,fs_path); break;
      case DTM:    exrpc_oneway(fm_node,save_model<DTM1>,mid,fs_path); break;
      case GBT:    exrpc_oneway(fm_node,save_model<GBT1>,mid,fs_path); break;
      case RFM:    exrpc_oneway(fm_node,save_model<RFM1>,mid,fs_path); break;
      case FPM:    exrpc_oneway(fm_node,save_model<FPM1>,mid,fs_path); break;
      case FPR:    exrpc_oneway(fm_node,save_model<FPR1>,mid,fs_path); break;
      case SEM:    exrpc_oneway(fm_node,save_model<SEM1>,mid,fs_path); break;
      case SCM:    exrpc_oneway(fm_node,save_model<SCM1>,mid,fs_path); break;
      case ACM:    exrpc_oneway(fm_node,save_model<ACM1>,mid,fs_path); break;
      case FMM:    exrpc_oneway(fm_node,save_fmm<DT1>,mid,fs_path); break;
      case NBM:    exrpc_oneway(fm_node,save_model<NBM1>,mid,fs_path); break;
      case LDASP:  exrpc_oneway(fm_node,save_model<LDASP3>,mid,fs_path); break;
      default:     REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// broadcasts the registered model from Frovedis master node to worker nodes
// and returns a vector containing model heads at worker nodes
JNIEXPORT jlongArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_broadcast2AllWorkers
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid, jshort mkind) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node (" 
            << fm_node.hostname << "," << fm_node.rpcport 
            << ") to broadcast model[" << mid << "] among worker nodes.\n";
#endif

  std::vector<exrpc_ptr_t> eps;
  jlongArray ret = NULL;
  try {
    switch(mkind) {
      case MFM:    eps = exrpc_async(fm_node,(bcast_model_to_workers<DT1,MFM1>),mid).get(); break;
      case KMEANS: eps = exrpc_async(fm_node,(bcast_model_to_workers<DT1,KMM1>),mid).get(); break;
      default:     REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
    }
    // converting eps to jlongArray
    size_t sz = eps.size();
    jlong arr[sz];
    for(size_t i= 0; i<sz; ++i) arr[i] = (jlong) eps[i];
    ret = env->NewLongArray(sz);
    if(ret == NULL) REPORT_ERROR(INTERNAL_ERROR, "New jlongArray allocation failed.\n");
    env->SetLongArrayRegion(ret, 0, sz, arr);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return ret;
}

// resets threshold value in trained Frovedis glm 
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_setFrovedisGLMThreshold
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid, jshort mkind, jdouble thr) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  try {
     switch(mkind) {
       case LRM:    exrpc_oneway(fm_node,(set_glm_threshold<DT1,LRM1>),mid,thr); break;
       case MLR:    exrpc_oneway(fm_node,(set_glm_threshold<DT1,MLR1>),mid,thr); break;
       case SVM:    exrpc_oneway(fm_node,(set_glm_threshold<DT1,SVM1>),mid,thr); break;
       case SVR:
       case LNRM:
       case MFM:
       case KMEANS: REPORT_ERROR(USER_ERROR,"This model doesn't have any threshold parameter!\n");
       default:     REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}
 

// loads frovedis generic models
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisModel
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid, jshort mkind, jstring path) {
  auto fs_path = to_cstring(env,path);
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  try {
    switch(mkind) {
      case FPM:  exrpc_oneway(fm_node,load_model<FPM1>,mid,FPM,fs_path); break;
      case FPR:  exrpc_oneway(fm_node,load_model<FPR1>,mid,FPM,fs_path); break;
      case FMM:  REPORT_ERROR(USER_ERROR,"currently Frovedis fm_model can't be loaded!\n"); 
      case DTM:  exrpc_oneway(fm_node,load_model<DTM1>,mid,DTM,fs_path); break;
      case GBT:  exrpc_oneway(fm_node,load_model<GBT1>,mid,DTM,fs_path); break;
      case RFM:  exrpc_oneway(fm_node,load_model<RFM1>,mid,RFM,fs_path); break;
      case SEM:  exrpc_oneway(fm_node,load_model<SEM1>,mid,SEM,fs_path); break;
      //case KSVC: exrpc_oneway(fm_node,load_model<KSVC1>,mid,KSVC,fs_path); break;
      default:   REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// loads the nbm from the specified file
JNIEXPORT jstring JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisNBM
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid, jshort mkind, jstring path) {

  auto fs_path = to_cstring(env,path);
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  std::string str = "";
  try {
    str = exrpc_async(fm_node,load_nbm<DT1>,mid,NBM,fs_path).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  jstring js = env->NewStringUTF(str.c_str());
  return js;
}

// loads the scm from the specified file
JNIEXPORT jintArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisSCM
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid, jstring path) {
  auto fs_path = to_cstring(env,path);
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  std::vector<int> ret;
  try {
    ret = exrpc_async(fm_node,load_scm<DT1>,mid,fs_path).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jintArray(env,ret);
}

JNIEXPORT jintArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_FrovedisACMPredict
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid, jint ncluster) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  std::vector<int> ret;
  try {
    ret = exrpc_async(fm_node,frovedis_acm_pred<DT1>,mid,ncluster).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jintArray(env,ret);
}

// loads the acm from the specified file
JNIEXPORT int JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisACM
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid, jstring path) {

  auto fs_path = to_cstring(env,path);
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  int nsamples = 0;
  try {
    nsamples = exrpc_async(fm_node,load_acm<DT1>,mid,fs_path).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return nsamples;
}

// loads the glm from the specified file 
JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisGLM
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid, jshort mkind, jstring path) {

  auto fs_path = to_cstring(env,path);
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  dummy_glm model;
  try {
    switch(mkind) {
      case LRM:  model = exrpc_async(fm_node,load_glm<LRM1>,mid,LRM,fs_path).get(); break;
      case SVM:  model = exrpc_async(fm_node,load_glm<SVM1>,mid,SVM,fs_path).get(); break;
      case SVR:  model = exrpc_async(fm_node,load_lnrm<DT1>,mid,SVR,fs_path).get(); break;
      case LNRM: model = exrpc_async(fm_node,load_lnrm<DT1>,mid,LNRM,fs_path).get(); break;
      case KSVC: model = exrpc_async(fm_node,load_glm<KSVC1>,mid,KSVC,fs_path).get(); break;
      default:   REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
#ifdef _EXRPC_DEBUG_
  std::cout << "model loading completed...printing the loaded glm info: \n";
  model.debug_print();
#endif
  return frovedis_dummyGLM_to_java_dummyGLM(env, model);
}

// loads the mfm from the specified file 
JNIEXPORT jint JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisMFM
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid, jshort mkind, jstring path) {

  auto fs_path = to_cstring(env,path);
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  dummy_mfm ret;
  try {
    ret = exrpc_async(fm_node,load_mfm<DT1>,mid,MFM,fs_path).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return ret.rank;
}

// loads the kmm from the specified file 
JNIEXPORT jint JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisKMM
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid, jshort mkind, jstring path) {

  auto fs_path = to_cstring(env,path);
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  int ret = 0; 
  try {
    ret = exrpc_async(fm_node,load_kmm<DT1>,mid,KMEANS,fs_path).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return ret;  
}

// Array[uid], Array[pid]: prediction carried out in parallel in worker nodes
JNIEXPORT jdoubleArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_doParallelALSPredict
  (JNIEnv *env, jclass thisCls, jobject worker_node, jlong mptr, jshort mkind, 
   jintArray uids, jintArray pids) {

  auto fw_node = java_node_to_frovedis_node(env, worker_node);
  auto mptr_ = (exrpc_ptr_t) mptr;
  jsize total = env->GetArrayLength(uids);
  jint *uidsp = env->GetIntArrayElements(uids, 0);  
  jint *pidsp = env->GetIntArrayElements(pids, 0); 
  std::vector<std::pair<size_t,size_t>> ids(total);
  for(int i=0; i<total; ++i) ids[i] = std::make_pair(uidsp[i], pidsp[i]);
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to worker node (" 
            << fw_node.hostname << "," << fw_node.rpcport 
            << ") to perform multiple prediction in parallel.\n";
#endif
  std::vector<double> pd;
  try {
    pd = exrpc_async(fw_node,parallel_mfm_predict<DT1>,mptr_,ids).get(); 
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jdoubleArray(env, pd);
}

// (uid, pid): prediction on trained mfm is carried out in master node
JNIEXPORT jdouble JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_doSingleALSPredict
  (JNIEnv *env, jclass thisCls, jobject master_node, 
   jint mid, jshort mkind, jint uid, jint pid) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node (" 
            << fm_node.hostname << "," << fm_node.rpcport 
            << ") to perform single prediction.\n";
#endif
  double ret = 0.0;
  try {
    ret= exrpc_async(fm_node,single_mfm_predict<DT1>,mid,uid,pid).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return ret; 
}

// to recommend products with rating for a given user based on trained mfm
JNIEXPORT jobjectArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_recommendProducts
  (JNIEnv *env, jclass thisCls, jobject master_node, 
   jint mid, jshort mkind, jint uid, jint num) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node (" 
            << fm_node.hostname << "," << fm_node.rpcport 
            << ") to recommend products with rating for user: " << uid << "\n";
#endif
  std::vector<std::pair<size_t,double>> ret;
  try {
    ret = exrpc_async(fm_node,recommend_products<DT1>,mid,uid,num).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jIntDoublePairArray(env, ret); 
}

// to recommend users with rating for a given product based on trained mfm
JNIEXPORT jobjectArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_recommendUsers
  (JNIEnv *env, jclass thisCls, jobject master_node, 
   jint mid, jshort mkind, jint pid, jint num) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node (" 
            << fm_node.hostname << "," << fm_node.rpcport 
            << ") to recommend users with rating for product: " << pid << "\n";
#endif
  std::vector<std::pair<size_t,double>> ret;
  try {
    ret = exrpc_async(fm_node,recommend_users<DT1>,mid,pid,num).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jIntDoublePairArray(env, ret); 
}

// for multiple test inputs: prediction is carried out in parallel in Frovedis worker nodes
JNIEXPORT jintArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_doParallelKMMPredict
  (JNIEnv *env, jclass thisCls, jobject worker_node, jlong mptr, jshort mkind, 
   jlong nrows, jlong ncols, jintArray off, jintArray idx, jdoubleArray val) {

  auto crs_loc = get_frovedis_double_crs_matrix_local(env,nrows,ncols,off,idx,val);
  auto fw_node = java_node_to_frovedis_node(env, worker_node);
  auto mptr_ = (exrpc_ptr_t) mptr;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to worker node (" 
            << fw_node.hostname << "," << fw_node.rpcport 
            << ") to perform multiple prediction in parallel.\n";
#endif
  std::vector<int> pd;
  try {
    pd = exrpc_async(fw_node,(parallel_kmm_predict<S_LMAT1,KMM1>),crs_loc,mptr_).get(); 
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jintArray(env, pd);
}

//Spectral Clustering - get affinity matrix
JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getSCMAffinityMatrix
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  dummy_matrix ret;
  try{
    ret = exrpc_async(fm_node,get_scm_affinity_matrix<DT1>,mid).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jDummyMatrix(env,ret,RMJR);
}

//Spectral Embedding - get affinity matrix
JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getSEMAffinityMatrix
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  dummy_matrix ret;
  try{
    ret = exrpc_async(fm_node,get_sem_affinity_matrix<DT1>,mid).get(); 
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jDummyMatrix(env,ret,RMJR);
}

//Spectral Embedding - get affinity matrix
JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getSEMEmbeddingMatrix
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  dummy_matrix ret;
  try{
    ret = exrpc_async(fm_node,get_sem_embedding_matrix<DT1>,mid).get(); 
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jDummyMatrix(env,ret,RMJR);
}

// for single test input: prediction on trained kmm is carried out in master node
JNIEXPORT jint JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_doSingleKMMPredict
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid, jshort mkind, 
   jlong nrows, jlong ncols, jintArray off, jintArray idx, jdoubleArray val) {

  auto crs_loc = get_frovedis_double_crs_matrix_local(env,nrows,ncols,off,idx,val);
  auto fm_node = java_node_to_frovedis_node(env, master_node);
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node (" 
            << fm_node.hostname << "," << fm_node.rpcport 
            << ") to perform single prediction.\n";
#endif
  int ret = 0;
  try {
    ret = exrpc_async(fm_node,(single_kmm_predict<S_LMAT1,KMM1>),crs_loc,mid).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return ret;
}

JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getW2VWeightPointer
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  dummy_matrix ret;
  try{
    ret = exrpc_async(fm_node,get_w2v_weight_ptr<DT2>,mid).get(); // type float
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jDummyMatrix(env,ret,RMJR);
}

JNIEXPORT jfloatArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getW2VWeight
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  std::vector<float> ret;
  try{
    ret = exrpc_async(fm_node,get_w2v_weight_vector<DT2>,mid).get(); // type float
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jfloatArray(env,ret);
}

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_showW2VWeight
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  try{
    exrpc_oneway(fm_node,show_w2v_weight<DT2>,mid); // type float
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_saveW2VModel
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid,
   jobjectArray vocab_words, jint size, jstring path) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto vocab = to_string_vector(env,vocab_words,size);
  auto fs_path = to_cstring(env,path);
  try{
    exrpc_oneway(fm_node,save_w2v_model<DT2>,mid,vocab,fs_path); // type float
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

//KNN 
JNIEXPORT jobject JNICALL
Java_com_nec_frovedis_Jexrpc_JNISupport_knnKneighbors
(JNIEnv *env, jclass thisCls, jobject master_node, jlong tptr,
  jint k, jint mid, jboolean needDistance, jboolean dense) { 
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto test_dptr = (exrpc_ptr_t) tptr;
  bool need_distance = (bool) needDistance;
  bool isDense = (bool) dense;
  knn_result res;
  try {
    if (isDense){
      res = exrpc_async(fm_node, (frovedis_kneighbors_spark<DT1,DT5,R_MAT1,KNN1>),
                        test_dptr, mid, k, need_distance ).get();
    }
    else REPORT_ERROR(USER_ERROR, 
         "Frovedis K-Nearest Neighbor doesn't support sparse input at "
         "this moment.\n");
  }
  catch (std::exception& e) {
    set_status(true, e.what());
  }
  return to_jDummyKNNResult(env,res);
}

JNIEXPORT jobject JNICALL
Java_com_nec_frovedis_Jexrpc_JNISupport_knnKneighborsGraph
(JNIEnv *env, jclass thisCls, jobject master_node, jlong tptr,
  jint k, jint mid, jstring mode, jboolean dense) { 
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto test_dptr = (exrpc_ptr_t) tptr;
  auto mode_ = to_cstring(env,mode);
  bool isDense = (bool) dense;
  dummy_matrix dmat;
  try {
    if (isDense){
      dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,R_MAT1,KNN1,S_MAT15,S_LMAT15>),
                         test_dptr, mid, k, mode_).get();  
    }
    else REPORT_ERROR(USER_ERROR, 
         "Frovedis K-Nearest Neighbor doesn't support sparse input at "
         "this moment.\n");
  }
  catch (std::exception& e) {
    set_status(true, e.what());
  }
  return to_jDummyMatrix(env, dmat, SCRS);
}

JNIEXPORT jobject JNICALL
Java_com_nec_frovedis_Jexrpc_JNISupport_knnRadiusNeighbors
(JNIEnv *env, jclass thisCls, jobject master_node, jlong tptr,
  jfloat radius, jint mid, jboolean needDistance, jboolean dense) { 
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto test_dptr = (exrpc_ptr_t) tptr;
  bool need_distance = (bool) needDistance;
  bool isDense = (bool) dense;
  dummy_matrix dmat;
  try {
    if (isDense){
      dmat = exrpc_async(fm_node, (frovedis_radius_neighbors<DT5,R_MAT1,KNN1,S_MAT15,S_LMAT15>),
                                  test_dptr, mid, radius, need_distance).get();  
    }
    else REPORT_ERROR(USER_ERROR, 
         "Frovedis K-Nearest Neighbor doesn't support sparse input at "
         "this moment.\n");
  }
  catch (std::exception& e) {
    set_status(true, e.what());
  }
  return to_jDummyMatrix(env, dmat, SCRS);
}

JNIEXPORT jobject JNICALL
Java_com_nec_frovedis_Jexrpc_JNISupport_knnRadiusNeighborsGraph
(JNIEnv *env, jclass thisCls, jobject master_node, jlong tptr,
  jfloat radius, jint mid, jstring mode, jboolean dense) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto test_dptr = (exrpc_ptr_t) tptr;
  auto mode_ = to_cstring(env,mode);
  bool isDense = (bool) dense;
  dummy_matrix dmat;
  try {
    if (isDense){
      dmat = exrpc_async(fm_node, (frovedis_radius_neighbors_graph<DT5,R_MAT1,KNN1,S_MAT15,S_LMAT15>),
                        test_dptr, mid, radius, mode_ ).get();  
    }
    else REPORT_ERROR(USER_ERROR, 
         "Frovedis K-Nearest Neighbor doesn't support sparse input at "
         "this moment.\n");
  }
  catch (std::exception& e) {
    set_status(true, e.what());
  }
  return to_jDummyMatrix(env, dmat, SCRS);
}

// KNC
JNIEXPORT jobject JNICALL
Java_com_nec_frovedis_Jexrpc_JNISupport_kncKneighbors
(JNIEnv *env, jclass thisCls, jobject master_node, jlong tptr,
  jint k, jint mid, jboolean needDistance, jboolean dense) { 
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto test_dptr = (exrpc_ptr_t) tptr;
  bool need_distance = (bool) needDistance;
  bool isDense = (bool) dense;
  knn_result res;
  try {
    if(isDense){
      res = exrpc_async(fm_node, (frovedis_kneighbors_spark<DT1,DT5,R_MAT1,KNC1>),
                        test_dptr, mid, k, need_distance ).get();
    }
    else REPORT_ERROR(USER_ERROR, 
         "Frovedis K-neighbor classifier doesn't support sparse input at "
         "this moment.\n");
  }
  catch (std::exception& e) {
    set_status(true, e.what());
  }
  return to_jDummyKNNResult(env,res);
}

JNIEXPORT jobject JNICALL
Java_com_nec_frovedis_Jexrpc_JNISupport_kncKneighborsGraph
(JNIEnv *env, jclass thisCls, jobject master_node, jlong tptr,
  jint k, jint mid, jstring mode, jboolean dense) { 
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto test_dptr = (exrpc_ptr_t) tptr;
  auto mode_ = to_cstring(env,mode);
  bool isDense = (bool) dense;
  dummy_matrix dmat;
  try {
    if (isDense){
      dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,R_MAT1,KNC1,S_MAT15,S_LMAT15>),
                        test_dptr, mid, k, mode_).get();  
    }
    else REPORT_ERROR(USER_ERROR, 
         "Frovedis K-neighbor classifier doesn't support sparse input at "
         "this moment.\n");
  }
  catch (std::exception& e) {
    set_status(true, e.what());
  }
  return to_jDummyMatrix(env, dmat, SCRS);
}

JNIEXPORT jdoubleArray JNICALL
Java_com_nec_frovedis_Jexrpc_JNISupport_kncDoublePredict
(JNIEnv *env, jclass thisCls, jobject master_node, jlong tptr, jint mid,
   jboolean saveProba, jboolean dense) { 
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto test_dptr = (exrpc_ptr_t) tptr;
  bool save_proba = (bool) saveProba;
  bool isDense = (bool) dense;
  std::vector<double> label;
  try {
    if (isDense){
      label = exrpc_async(fm_node, (frovedis_knc_predict<DT1,DT5,R_MAT1,KNC1>),
                          test_dptr, mid, save_proba).get();   
    }
    else REPORT_ERROR(USER_ERROR, 
         "Frovedis K-neighbor classifier doesn't support sparse input at "
         "this moment.\n"); 
  }
  catch (std::exception& e) {
      set_status(true, e.what());
  }
  return to_jdoubleArray(env, label);
}

JNIEXPORT jobject JNICALL
Java_com_nec_frovedis_Jexrpc_JNISupport_kncPredictProba
(JNIEnv *env, jclass thisCls, jobject master_node, jlong tptr, jint mid,
jboolean dense) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto test_dptr = (exrpc_ptr_t) tptr;
  bool isDense = (bool) dense;
  dummy_matrix dmat;
  try {
    if (isDense){
      dmat = exrpc_async(fm_node, (frovedis_knc_predict_proba<DT5,R_MAT1,KNC1,R_MAT1,R_LMAT1>), 
                               test_dptr, mid).get();  
    }
    else REPORT_ERROR(USER_ERROR, 
         "Frovedis K-neighbor classifier doesn't support sparse input at "
         "this moment.\n");
  }
  catch (std::exception& e) {
      set_status(true, e.what());
  }
  return to_jDummyMatrix(env, dmat, RMJR);
}

JNIEXPORT jfloat JNICALL
Java_com_nec_frovedis_Jexrpc_JNISupport_kncModelScore
(JNIEnv *env, jclass thisCls, jobject master_node, jlong xptr, jlong yptr,
jint mid, jboolean dense) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto mptr = (exrpc_ptr_t) xptr;
  auto lblptr = (exrpc_ptr_t) yptr;
  float res = 0.0;
  bool isDense = (bool) dense;
  try {
    if (isDense){
      res = exrpc_async(fm_node, (frovedis_model_score<DT1,DT5,R_MAT1,KNC1>),
                        mptr, lblptr, mid).get();  
    }
    else REPORT_ERROR(USER_ERROR, 
         "Frovedis K-neighbor classifier doesn't support sparse input at "
         "this moment.\n");
  }
  catch (std::exception& e) {
      set_status(true, e.what());
  }  
  return res;
}

// KNR
JNIEXPORT jobject JNICALL
Java_com_nec_frovedis_Jexrpc_JNISupport_knrKneighbors
(JNIEnv *env, jclass thisCls, jobject master_node, jlong tptr,
  jint k, jint mid, jboolean needDistance, jboolean dense) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto test_dptr = (exrpc_ptr_t) tptr;
  bool need_distance = (bool) needDistance;
  bool isDense = (bool) dense;
  knn_result res;
  try {
    if(isDense){
      res = exrpc_async(fm_node, (frovedis_kneighbors_spark<DT1,DT5,R_MAT1,KNR1>),
                        test_dptr, mid, k, need_distance ).get();
    }
    else REPORT_ERROR(USER_ERROR, 
         "Frovedis K-neighbor regressor doesn't support sparse input at "
         "this moment.\n");
  }
  catch (std::exception& e) {
    set_status(true, e.what());
  }
  return to_jDummyKNNResult(env,res);
}

JNIEXPORT jobject JNICALL
Java_com_nec_frovedis_Jexrpc_JNISupport_knrKneighborsGraph
(JNIEnv *env, jclass thisCls, jobject master_node, jlong tptr,
  jint k, jint mid, jstring mode ,jboolean dense) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto test_dptr = (exrpc_ptr_t) tptr;
  auto mode_ = to_cstring(env,mode);
  bool isDense = (bool) dense;
  dummy_matrix dmat;
  try {
    if(isDense) {
      dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,R_MAT1,KNR1,S_MAT15,S_LMAT15>),
                        test_dptr, mid, k, mode_).get();  
    }
    else REPORT_ERROR(USER_ERROR, 
         "Frovedis K-neighbor regressor doesn't support sparse input at "
         "this moment.\n");
  }
  catch (std::exception& e) {
    set_status(true, e.what());
  }
  return to_jDummyMatrix(env, dmat, SCRS);
}

JNIEXPORT jdoubleArray JNICALL
Java_com_nec_frovedis_Jexrpc_JNISupport_knrDoublePredict
(JNIEnv *env, jclass thisCls, jobject master_node, jlong tptr, jint mid, jboolean dense) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto test_dptr = (exrpc_ptr_t) tptr;
  std::vector<double> label;
  bool isDense = (bool) dense;
  try {
    if(isDense){
      label = exrpc_async(fm_node, (frovedis_knr_predict<DT1,DT5,R_MAT1,KNR1>),
                          test_dptr, mid).get();
    }
    else REPORT_ERROR(USER_ERROR, 
         "Frovedis K-neighbor regressor doesn't support sparse input at "
         "this moment.\n");
  }
  catch (std::exception& e) {
      set_status(true, e.what());
  }
  return to_jdoubleArray(env, label);
}

JNIEXPORT jfloat JNICALL
Java_com_nec_frovedis_Jexrpc_JNISupport_knrModelScore
(JNIEnv *env, jclass thisCls, jobject master_node, jlong xptr, jlong yptr, jint mid, jboolean dense) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto mptr = (exrpc_ptr_t) xptr;
  auto lblptr = (exrpc_ptr_t) yptr;
  float res = 0.0;
  bool isDense = (bool) dense;
  try {
    if(isDense){
      res = exrpc_async(fm_node, (frovedis_model_score<DT1,DT5,R_MAT1,KNR1>),
                        mptr, lblptr, mid).get();  
    }
    else REPORT_ERROR(USER_ERROR, 
         "Frovedis K-neighbor regressor doesn't support sparse input at "
         "this moment.\n");   
  }
  catch (std::exception& e) {
      set_status(true, e.what());
  }  
  return res;
}

// LDA
JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisLDATransform
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata,
   jint mid, jint num_iter, jdouble alpha, jdouble beta,
   jint num_explore_iter, jstring algo) { 
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = (exrpc_ptr_t) fdata;
  dummy_lda_result ret;
  auto algo_ = to_cstring(env,algo);
  try{
    ret = exrpc_async(fm_node,(frovedis_lda_transform_for_spark<DT3,S_MAT15,LDASP3>),
                      f_dptr, alpha, beta, num_iter, algo_, 
                      num_explore_iter, mid).get();
  }
  catch(std::exception& e) { 
    set_status(true,e.what()); 
  }
  return to_jDummyLDAResult(env,ret);
}

JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getTopicsMatrix
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid) { 
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  dummy_matrix ret;
  try{
    ret = exrpc_async(fm_node,(get_topics_matrix<DT3,LDASP3>),mid).get();
  }
  catch(std::exception& e) { 
    set_status(true,e.what()); 
  }
  return to_jDummyMatrix(env,ret,RMJR);
}

JNIEXPORT jint JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getVocabSize
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid) { 
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  int ret = -1;
  try{
    ret = exrpc_async(fm_node,get_vocabulary_size<LDASP3>,mid).get();
  }
  catch(std::exception& e) { 
    set_status(true,e.what()); 
  }
  return ret;
}

JNIEXPORT jint JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getNumTopics
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid) { 
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  int ret = -1;
  try{
    ret = exrpc_async(fm_node,get_num_topics<LDASP3>,mid).get();
  }
  catch(std::exception& e) { 
    set_status(true,e.what()); 
  }
  return ret;
}

JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getTopicWordDistribution
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  dummy_matrix dmat;
  try {
    dmat = exrpc_async(fm_node,get_topic_word_distribution<LDASP3>,mid).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jDummyMatrix(env,dmat,RMJR);
}

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_extractTopWordsPerTopic
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata,
   jint num_topics, jint maxTermsPerTopic,
   jintArray word_id, jdoubleArray topic_word_dist) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = exrpc_ptr_t (fdata);
  auto sz = num_topics * maxTermsPerTopic;
  distMatrix<DT4,DT1> result;
  try {
    result = exrpc_async(fm_node, (extract_sorted_topic_word_distribution<DT4,DT1>),
                         f_dptr,maxTermsPerTopic).get();
  }
  catch(std::exception& e) { set_status(true,e.what());}
  checkAssumption(result.indices.val.size() == sz);
  env->SetIntArrayRegion(word_id, 0, sz, result.indices.val.data());
  env->SetDoubleArrayRegion(topic_word_dist, 0, sz, result.distances.val.data());
}

JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getTopicDocDistribution
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid) { 
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  dummy_matrix dmat;
  try {
    dmat = exrpc_async(fm_node,get_topic_doc_distribution<LDASP3>,mid).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jDummyMatrix(env,dmat,RMJR);
}

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_extractTopDocsPerTopic
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid, jlong fdata, 
   jint num_topics, jint maxDocumentsPerTopic, 
   jlongArray doc_id, jdoubleArray topic_doc_dist) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = exrpc_ptr_t (fdata);
  auto sz = num_topics * maxDocumentsPerTopic;
  distMatrix<DT3,DT1> result;
  try {
    result = exrpc_async(fm_node, (extract_sorted_topic_doc_distribution<LDASP3,DT3,DT1>),
                         f_dptr,mid,maxDocumentsPerTopic).get();
  }
  catch(std::exception& e) { set_status(true,e.what());}
  checkAssumption(result.indices.val.size() == sz);
  env->SetLongArrayRegion(doc_id, 0, sz, result.indices.val.data());
  env->SetDoubleArrayRegion(topic_doc_dist, 0, sz, result.distances.val.data());
}

JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getDocTopicDistribution
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid) { 
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  dummy_matrix dmat;
  try {
    dmat = exrpc_async(fm_node,get_doc_topic_distribution<LDASP3>,mid).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jDummyMatrix(env,dmat,RMJR);
}

JNIEXPORT jlongArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getLDAModelDocIds
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  std::vector<long> doc_id;
  try {
    doc_id = exrpc_async(fm_node,get_doc_id<LDASP3>,mid).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jlongArray2(env, doc_id);
}

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_extractTopTopicsPerDoc
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata, 
   jint num_docs, jint max_topics, 
   jintArray topic_id, jdoubleArray doc_topic_dist) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = exrpc_ptr_t (fdata);
  auto sz = num_docs * max_topics;
  distMatrix<DT4,DT1> result;
  try {
    result = exrpc_async(fm_node, (extract_sorted_doc_topic_distribution<DT4,DT1>),
                         f_dptr,max_topics).get();
  }
  catch(std::exception& e) { set_status(true,e.what());}
  checkAssumption(result.indices.val.size() == sz);
  env->SetIntArrayRegion(topic_id, 0, sz, result.indices.val.data());
  env->SetDoubleArrayRegion(doc_topic_dist, 0, sz, result.distances.val.data());
}

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_transformAndExtractTopTopicsPerDoc
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata,
   jint mid, jint num_iter, 
   jdouble alpha, jdouble beta,
   jint num_explore_iter, jstring algo,
   jint num_docs, jint max_topics,
   jintArray topic_id, jdoubleArray doc_topic_dist) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = (exrpc_ptr_t) fdata;
  auto algo_ = to_cstring(env,algo);
  auto sz = num_docs * max_topics;
  distMatrix<DT4,DT1> result;
  try {
    result = exrpc_async(fm_node, (get_top_topics_per_document<DT3,S_MAT15,LDASP3,DT4,DT1>),
                         f_dptr, alpha, beta, num_iter, algo_,
                         num_explore_iter, mid, max_topics).get();
  }
  catch(std::exception& e) { set_status(true,e.what());}
  checkAssumption(result.indices.val.size() == sz);
  env->SetIntArrayRegion(topic_id, 0, sz, result.indices.val.data());
  env->SetDoubleArrayRegion(doc_topic_dist, 0, sz, result.distances.val.data());
}

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_transformAndExtractTopDocsPerTopic
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata,
   jlongArray test_doc_id, jlong num_docs,
   jint mid, jint num_iter, 
   jdouble alpha, jdouble beta,
   jint num_explore_iter, jstring algo,
   jint num_topics, jint maxDocumentsPerTopic,
   jlongArray doc_id, jdoubleArray topic_doc_dist) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = (exrpc_ptr_t) fdata;
  auto algo_ = to_cstring(env,algo);
  auto sz = num_topics * maxDocumentsPerTopic;
  auto test_doc_id_vec = to_long_vector(env, test_doc_id, (long)num_docs);
  distMatrix<DT3,DT1> result;
  try {
    result = exrpc_async(fm_node, (get_top_documents_per_topic<DT3,S_MAT15,LDASP3,DT3,DT1>),
                         f_dptr, test_doc_id_vec, alpha, beta, num_iter, algo_,
                         num_explore_iter, mid, maxDocumentsPerTopic).get();
  }
  catch(std::exception& e) { set_status(true,e.what());}
  checkAssumption(result.indices.val.size() == sz);
  env->SetLongArrayRegion(doc_id, 0, sz, result.indices.val.data());
  env->SetDoubleArrayRegion(topic_doc_dist, 0, sz, result.distances.val.data());
}

// loads the lda model from the specified file
JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisLDAModel
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid, jstring path) {

  auto fs_path = to_cstring(env,path);
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  dummy_lda_model ret;
  try {
    ret = exrpc_async(fm_node,load_lda_model<LDASP3>,mid,fs_path).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jDummyLDAModel(env, ret);
}

JNIEXPORT jdoubleArray JNICALL
Java_com_nec_frovedis_Jexrpc_JNISupport_genericPredict
(JNIEnv *env, jclass thisCls, jobject master_node, jlong tptr, jint mid,
   jshort mkind, jboolean dense, jboolean need_prob) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = (exrpc_ptr_t) tptr;
  bool isDense = (bool) dense;
  std::vector<double> pred;
  bool prob = (bool) need_prob;
  
  try {
    if (isDense) {
      switch(mkind) {
        case LRM:  pred = exrpc_async(fm_node,(parallel_lrm_predict<DT1,R_MAT1,R_LMAT1,LRM1>),
                                      f_dptr,mid,prob).get(); break;
        case MLR:  pred = exrpc_async(fm_node,(parallel_lrm_predict<DT1,R_MAT1,R_LMAT1,MLR1>),
                                      f_dptr,mid,prob).get(); break;
        case SVM:  pred = exrpc_async(fm_node,(parallel_svm_predict<DT1,R_MAT1,R_LMAT1>),
                                      f_dptr,mid,prob).get(); break;
        case SVR:  pred = exrpc_async(fm_node,(parallel_lnrm_predict<DT1,R_MAT1,R_LMAT1>),
                                      f_dptr,mid,prob).get(); break;
        case KSVC: pred = exrpc_async(fm_node,(ksvm_predict<DT1,R_MAT1,KSVC1>),
                                      f_dptr,mid,prob).get(); break;
        case LNRM: pred = exrpc_async(fm_node,(parallel_lnrm_predict<DT1,R_MAT1,R_LMAT1>),
                                      f_dptr,mid,prob).get(); break;
        case FMM:  REPORT_ERROR(USER_ERROR, "currently Frovedis doesn't support dense test data for FM!\n");
        case NBM:  pred = exrpc_async(fm_node,(parallel_generic_predict<DT1,R_MAT1,R_LMAT1,NBM1>),
                                      f_dptr,mid,prob).get(); break;
        case DTM:  pred = exrpc_async(fm_node,(parallel_dtm_predict<DT1,R_MAT1,R_LMAT1>),
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
        case LRM:  pred = exrpc_async(fm_node,(parallel_lrm_predict<DT1,S_MAT15,S_LMAT15,LRM1>),
                                      f_dptr,mid,prob).get(); break;
        case MLR:  pred = exrpc_async(fm_node,(parallel_lrm_predict<DT1,S_MAT15,S_LMAT15,MLR1>),
                                      f_dptr,mid,prob).get(); break;
        case SVM:  pred = exrpc_async(fm_node,(parallel_svm_predict<DT1,S_MAT15,S_LMAT15>),
                                      f_dptr,mid,prob).get(); break;
        case SVR:  pred = exrpc_async(fm_node,(parallel_lnrm_predict<DT1,S_MAT15,S_LMAT15>),
                                      f_dptr,mid,prob).get(); break;
        case LNRM: pred = exrpc_async(fm_node,(parallel_lnrm_predict<DT1,S_MAT15,S_LMAT15>),
                                      f_dptr,mid,prob).get(); break;
        case FMM:  pred = exrpc_async(fm_node,(parallel_fmm_predict<DT1,S_MAT15,S_LMAT15>),
                                      f_dptr,mid,prob).get(); break;
        case NBM:  pred = exrpc_async(fm_node,(parallel_generic_predict<DT1,S_MAT15,S_LMAT15,NBM1>),
                                      f_dptr,mid,prob).get(); break;
        case DTM:  pred = exrpc_async(fm_node,(parallel_dtm_predict<DT1,S_MAT15,S_LMAT15>),
                                      f_dptr,mid,prob).get(); break;
        case RFM:  REPORT_ERROR(USER_ERROR, "currently Frovedis doesn't support sparse test data for Random Forest prediction!\n");
        case GBT:  REPORT_ERROR(USER_ERROR, "currently Frovedis doesn't support sparse test data for GBT prediction!\n");
        case KSVC: REPORT_ERROR(USER_ERROR, "currently Frovedis doesn't support sparse test data for Kernel SVM prediction!\n");
        default:  REPORT_ERROR(USER_ERROR, "Unknown model kind is encountered!\n");
      }
    }
  }
  catch (std::exception& e) {
      set_status(true, e.what());
  }
  return to_jdoubleArray(env, pred);
}

JNIEXPORT jdouble JNICALL
Java_com_nec_frovedis_Jexrpc_JNISupport_genericSinglePredict
(JNIEnv *env, jclass thisCls, jobject master_node, jlong tptr, jint mid,
   jshort mkind, jboolean dense) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = (exrpc_ptr_t) tptr;
  bool isDense = (bool) dense;
  double pred = 0.0;

  try {
    if (isDense) {
      switch(mkind) {
        case LRM:  pred = exrpc_async(fm_node,(single_glm_predict<DT1,R_LMAT1,LRM1>),
                                      f_dptr,mid).get(); break;
        case MLR:  pred = exrpc_async(fm_node,(single_glm_predict<DT1,R_LMAT1,MLR1>),
                                      f_dptr,mid).get(); break;
        case SVM:  pred = exrpc_async(fm_node,(single_glm_predict<DT1,R_LMAT1,SVM1>),
                                      f_dptr,mid).get(); break;
        case SVR:  pred = exrpc_async(fm_node,(single_generic_predict<DT1,R_LMAT1,SVR1>),
                                      f_dptr,mid).get(); break;
        case KSVC: pred = exrpc_async(fm_node,(single_generic_predict<DT1,R_LMAT1,KSVC1>),
                                      f_dptr,mid).get(); break;
                                      //f_dptr,mid,false).get(); break;
        case LNRM: pred = exrpc_async(fm_node,(single_generic_predict<DT1,R_LMAT1,LNRM1>),
                                      f_dptr,mid).get(); break;
        case FMM:  REPORT_ERROR(USER_ERROR, "currently Frovedis doesn't support dense test data for FM!\n");
        case NBM:  pred = exrpc_async(fm_node,(single_generic_predict<DT1,R_LMAT1,NBM1>),
                                      f_dptr,mid).get(); break;
        case DTM:  pred = exrpc_async(fm_node,(single_generic_predict<DT1,R_LMAT1,DTM1>),
                                      f_dptr,mid).get(); break;
        case RFM:  pred = exrpc_async(fm_node,(single_generic_predict<DT1,R_LMAT1,RFM1>),
                                      f_dptr,mid).get(); break;
        case GBT:  pred = exrpc_async(fm_node,(single_generic_predict<DT1,R_LMAT1,GBT1>),
                                      f_dptr,mid).get(); break;
        default:   REPORT_ERROR(USER_ERROR, "Unknown model kind is encountered!\n");
      }
    }
    else {
      switch(mkind) {
        case LRM:  pred = exrpc_async(fm_node,(single_glm_predict<DT1,S_LMAT1,LRM1>),
                                      f_dptr,mid).get(); break;
        case MLR:  pred = exrpc_async(fm_node,(single_glm_predict<DT1,S_LMAT1,MLR1>),
                                      f_dptr,mid).get(); break;
        case SVM:  pred = exrpc_async(fm_node,(single_glm_predict<DT1,S_LMAT1,SVM1>),
                                      f_dptr,mid).get(); break;
        case SVR:  pred = exrpc_async(fm_node,(single_generic_predict<DT1,S_LMAT1,SVR1>),
                                      f_dptr,mid).get(); break;
        case LNRM: pred = exrpc_async(fm_node,(single_generic_predict<DT1,S_LMAT1,LNRM1>),
                                      f_dptr,mid).get(); break;
        case FMM:  pred = exrpc_async(fm_node,(single_generic_predict<DT1,S_LMAT1,FMM1>),
                                      f_dptr,mid).get(); break;
        case NBM:  pred = exrpc_async(fm_node,(single_generic_predict<DT1,S_LMAT1,NBM1>),
                                      f_dptr,mid).get(); break;
        case DTM:  pred = exrpc_async(fm_node,(single_generic_predict<DT1,S_LMAT1,DTM1>),
                                      f_dptr,mid).get(); break;
        case RFM:  REPORT_ERROR(USER_ERROR, "currently Frovedis doesn't support sparse test data for Random Forest prediction!\n");
        case GBT:  REPORT_ERROR(USER_ERROR, "currently Frovedis doesn't support sparse test data for GBT prediction!\n");
        case KSVC: REPORT_ERROR(USER_ERROR, "currently Frovedis doesn't support sparse test data for Kernel SVM prediction!\n");
        default:   REPORT_ERROR(USER_ERROR, "Unknown model kind is encountered!\n");
      }
    }
  }
  catch (std::exception& e) {
      set_status(true, e.what());
  }
  return pred;
}

// GBT getters
JNIEXPORT jint JNICALL
Java_com_nec_frovedis_Jexrpc_JNISupport_gbtNumTrees
(JNIEnv *env, jclass thisCls, jobject master_node, jint mid) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  size_t n = 0;
  int res;
  try {
    n = exrpc_async(fm_node,frovedis_ensemble_get_num_trees<GBT1>,mid).get();   
  }
  catch (std::exception& e) {
    set_status(true, e.what());
  }
  res = static_cast<int>(n);
  return res;
}

JNIEXPORT jint JNICALL
Java_com_nec_frovedis_Jexrpc_JNISupport_gbtTotalNumNodes
(JNIEnv *env, jclass thisCls, jobject master_node, jint mid) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  size_t n = 0;
  int res;
  try {
    n = exrpc_async(fm_node,frovedis_ensemble_get_total_num_nodes<GBT1>,mid).get(); 
  }
  catch (std::exception& e) {
    set_status(true, e.what());
  }
  res = static_cast<int>(n);
  return res;
}

JNIEXPORT jdoubleArray JNICALL
Java_com_nec_frovedis_Jexrpc_JNISupport_gbtTreeWeights
(JNIEnv *env, jclass thisCls, jobject master_node, jint mid) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  std::vector<double> res;
  try {
    res = exrpc_async(fm_node,(frovedis_ensemble_get_tree_weights<GBT1,DT1>),mid).get();   
  }
  catch (std::exception& e) {
    set_status(true, e.what());
  }
  return to_jdoubleArray(env, res);
}

JNIEXPORT jstring JNICALL
Java_com_nec_frovedis_Jexrpc_JNISupport_gbtToString
(JNIEnv *env, jclass thisCls, jobject master_node, jint mid) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  std::string res;
  try {
    res = exrpc_async(fm_node,frovedis_ensemble_to_string<GBT1>,mid).get();   
  }
  catch (std::exception& e) {
    set_status(true, e.what());
  }
  jstring js = env->NewStringUTF(res.c_str());
  return js;
}

// RFM getters

JNIEXPORT jint JNICALL
Java_com_nec_frovedis_Jexrpc_JNISupport_rfNumTrees
(JNIEnv *env, jclass thisCls, jobject master_node, jint mid) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  size_t rf = 0;
  int res;
  try {
    rf = exrpc_async(fm_node,frovedis_ensemble_get_num_trees<RFM1>,mid).get();
  }
  catch (std::exception& e) {
    set_status(true, e.what());
  }
  res = static_cast<int>(rf);
  return res;
}

JNIEXPORT jint JNICALL
Java_com_nec_frovedis_Jexrpc_JNISupport_rfTotalNumNodes
(JNIEnv *env, jclass thisCls, jobject master_node, jint mid) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  size_t rf = 0;
  int res;
  try {
    rf = exrpc_async(fm_node,frovedis_ensemble_get_total_num_nodes<RFM1>,mid).get();
  }
  catch (std::exception& e) {
    set_status(true, e.what());
  }
  res = static_cast<int>(rf);
  return res;
}

JNIEXPORT jstring JNICALL
Java_com_nec_frovedis_Jexrpc_JNISupport_rfToString
(JNIEnv *env, jclass thisCls, jobject master_node, jint mid) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  std::string res;
  try {
    res = exrpc_async(fm_node,frovedis_ensemble_to_string<RFM1>,mid).get();
  }
  catch (std::exception& e) {
    set_status(true, e.what());
  }
  jstring js = env->NewStringUTF(res.c_str());
  return js;
}

}
