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
      case LNRM:   exrpc_oneway(fm_node, show_model<LNRM1>, mid); break;
      case MFM:    exrpc_oneway(fm_node, show_model<MFM1>, mid); break;
      case KMEANS: exrpc_oneway(fm_node, show_model<KMM1>, mid); break;
      case DTM:    exrpc_oneway(fm_node, show_model<DTM1>, mid); break;
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
      case LNRM:   exrpc_oneway(fm_node, release_model<LNRM1>, mid); break;
      case MFM:    exrpc_oneway(fm_node, release_model<MFM1>, mid); break;
      case KMEANS: exrpc_oneway(fm_node, release_model<KMM1>, mid); break;
      case DTM:    exrpc_oneway(fm_node, release_model<DTM1>, mid); break;
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
      case LDA:    exrpc_oneway(fm_node, release_model<LDA3>, mid); break; // long for lda_model
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
      case LNRM:   exrpc_oneway(fm_node,save_model<LNRM1>,mid,fs_path); break;
      case MFM:    exrpc_oneway(fm_node,save_model<MFM1>,mid,fs_path); break;
      case KMEANS: exrpc_oneway(fm_node,save_model<KMM1>,mid,fs_path); break;
      case DTM:    exrpc_oneway(fm_node,save_model<DTM1>,mid,fs_path); break;
      case FPM:    exrpc_oneway(fm_node,save_model<FPM1>,mid,fs_path); break;
      case FPR:    exrpc_oneway(fm_node,save_model<FPR1>,mid,fs_path); break;
      case SEM:    exrpc_oneway(fm_node,save_model<SEM1>,mid,fs_path); break;
      case SCM:    exrpc_oneway(fm_node,save_model<SCM1>,mid,fs_path); break;
      case ACM:    exrpc_oneway(fm_node,save_model<ACM1>,mid,fs_path); break;
      case FMM:    exrpc_oneway(fm_node,save_fmm<DT1>,mid,fs_path); break;
      case NBM:    exrpc_oneway(fm_node,save_model<NBM1>,mid,fs_path); break;
      case LDA:    exrpc_oneway(fm_node,save_model<LDA3>,mid,fs_path); break; // long for lda_model
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
      case LRM:    eps = exrpc_async(fm_node,(bcast_model_to_workers<DT1,LRM1>),mid).get(); break;
      case MLR:    eps = exrpc_async(fm_node,(bcast_model_to_workers<DT1,MLR1>),mid).get(); break;
      case SVM:    eps = exrpc_async(fm_node,(bcast_model_to_workers<DT1,SVM1>),mid).get(); break;
      case LNRM:   eps = exrpc_async(fm_node,(bcast_model_to_workers<DT1,LNRM1>),mid).get(); break;
      case MFM:    eps = exrpc_async(fm_node,(bcast_model_to_workers<DT1,MFM1>),mid).get(); break;
      case KMEANS: eps = exrpc_async(fm_node,(bcast_model_to_workers<DT1,KMM1>),mid).get(); break;
      case DTM:    eps = exrpc_async(fm_node,(bcast_model_to_workers<DT1,DTM1>),mid).get(); break;
      case FMM:    eps = exrpc_async(fm_node,bcast_fmm_to_workers<DT1>,mid).get(); break;
      case NBM:    eps = exrpc_async(fm_node,(bcast_model_to_workers<DT1,NBM1>),mid).get(); break;
      default:   REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
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
      case SEM:  exrpc_oneway(fm_node,load_model<SEM1>,mid,SEM,fs_path); break;
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
      case LNRM: model = exrpc_async(fm_node,load_lnrm<DT1>,mid,LNRM,fs_path).get(); break;
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

// TODO: rename function (without GLM)
// for multiple test inputs: prediction is carried out in parallel in Frovedis worker nodes
JNIEXPORT jdoubleArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_doParallelGLMPredict
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
  std::vector<double> pd;
  try {
    switch(mkind) {
      case LRM:  pd = exrpc_async(fw_node,(parallel_glm_predict<DT1,S_LMAT1,LRM1>),crs_loc,mptr_).get(); break;
      case MLR:  pd = exrpc_async(fw_node,(parallel_glm_predict<DT1,S_LMAT1,MLR1>),crs_loc,mptr_).get(); break;
      case SVM:  pd = exrpc_async(fw_node,(parallel_glm_predict<DT1,S_LMAT1,SVM1>),crs_loc,mptr_).get(); break;
      case LNRM: pd = exrpc_async(fw_node,(parallel_lnrm_predict<DT1,S_LMAT1>),crs_loc,mptr_).get(); break;
      case DTM:  pd = exrpc_async(fw_node,(parallel_dtm_predict<DT1,S_LMAT1>),crs_loc,mptr_).get(); break;
      case FMM:  pd = exrpc_async(fw_node,(parallel_fmm_predict<DT1,S_LMAT1>),crs_loc,mptr_).get(); break;
      case NBM:  pd = exrpc_async(fw_node,(parallel_nbm_predict<DT1,S_LMAT1>),crs_loc,mptr_).get(); break;
      default:   REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jdoubleArray(env, pd);
}

// TODO: rename function (without GLM)
// for single test input: prediction on trained glm is carried out in master node
JNIEXPORT jdouble JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_doSingleGLMPredict
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid, jshort mkind, 
   jlong nrows, jlong ncols, jintArray off, jintArray idx, jdoubleArray val) {

  auto crs_loc = get_frovedis_double_crs_matrix_local(env,nrows,ncols,off,idx,val);
  auto fm_node = java_node_to_frovedis_node(env, master_node);
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node (" 
            << fm_node.hostname << "," << fm_node.rpcport 
            << ") to perform single prediction.\n";
#endif
  double ret = 0.0;
  try {
    switch(mkind) {
      case LRM:  ret = exrpc_async(fm_node,(single_glm_predict<DT1,S_LMAT1,LRM1>),crs_loc,mid).get(); break;
      case MLR:  ret = exrpc_async(fm_node,(single_glm_predict<DT1,S_LMAT1,MLR1>),crs_loc,mid).get(); break;
      case SVM:  ret = exrpc_async(fm_node,(single_glm_predict<DT1,S_LMAT1,SVM1>),crs_loc,mid).get(); break;
      case LNRM: ret = exrpc_async(fm_node,(single_lnrm_predict<DT1,S_LMAT1>),crs_loc,mid).get(); break;
      case DTM:  ret = exrpc_async(fm_node,(single_dtm_predict<DT1,S_LMAT1>),crs_loc,mid).get(); break;
      case FMM:  ret = exrpc_async(fm_node,(single_fmm_predict<DT1,S_LMAT1>),crs_loc,mid).get(); break;
      case NBM:  ret = exrpc_async(fm_node,(single_nbm_predict<DT1,S_LMAT1>),crs_loc,mid).get(); break;
      default:   REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
    } 
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
JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_knnKneighbors
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong tptr,
  jint k, jint mid, jboolean needDistance) { // TODO: add isDense
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto test_dptr = (exrpc_ptr_t) tptr;
  bool need_distance = (bool) needDistance;
  knn_result res;
  try {
    res = exrpc_async(fm_node, (frovedis_kneighbors_spark<DT1,DT5,R_MAT1,KNN1>), test_dptr, 
                      mid, k, need_distance ).get();
  }
  catch (std::exception& e) {
    set_status(true, e.what());
  }
  return to_jDummyKNNResult(env,res);
}

JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_knnKneighborsGraph
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong tptr,
  jint k, jint mid, jstring mode) { // TODO: add isDense
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto test_dptr = (exrpc_ptr_t) tptr;
  auto mode_ = to_cstring(env,mode);
  dummy_matrix dmat;
  try {
    dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,R_MAT1,KNN1,S_MAT15,S_LMAT15>), test_dptr, 
                              mid, k, mode_).get();
  }
  catch (std::exception& e) {
    set_status(true, e.what());
  }
  return to_jDummyMatrix(env, dmat, SCRS);
}

JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_knnRadiusNeighbors
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong tptr,
  jfloat radius, jint mid, jboolean needDistance) { // TODO: add isDense
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto test_dptr = (exrpc_ptr_t) tptr;
  bool need_distance = (bool) needDistance;
  dummy_matrix dmat;
  try {
    dmat = exrpc_async(fm_node, (frovedis_radius_neighbors<DT5,R_MAT1,KNN1,S_MAT15,S_LMAT15>), test_dptr, 
                              mid, radius, need_distance).get();
  }
  catch (std::exception& e) {
    set_status(true, e.what());
  }
  return to_jDummyMatrix(env, dmat, SCRS);
}

JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_knnRadiusNeighborsGraph
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong tptr,
  jfloat radius, jint mid, jstring mode) { // TODO: add isDense
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto test_dptr = (exrpc_ptr_t) tptr;
  auto mode_ = to_cstring(env,mode);
  dummy_matrix dmat;
  try {
    dmat = exrpc_async(fm_node, (frovedis_radius_neighbors_graph<DT5,R_MAT1,KNN1,S_MAT15,S_LMAT15>), test_dptr, 
                              mid, radius, mode_ ).get();
  }
  catch (std::exception& e) {
    set_status(true, e.what());
  }
  return to_jDummyMatrix(env, dmat, SCRS);
}

// KNC
JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_kncKneighbors
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong tptr,
  jint k, jint mid, jboolean needDistance) { // TODO: add isDense
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto test_dptr = (exrpc_ptr_t) tptr;
  bool need_distance = (bool) needDistance;
  knn_result res;
  try {
    res = exrpc_async(fm_node, (frovedis_kneighbors<DT1,DT5,R_MAT1,KNC1>), test_dptr, 
                              mid, k, need_distance ).get();
  }
  catch (std::exception& e) {
    set_status(true, e.what());
  }
  return to_jDummyKNNResult(env,res);
}

JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_kncKneighborsGraph
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong tptr,
  jint k, jint mid, jstring mode) { // TODO: add isDense
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto test_dptr = (exrpc_ptr_t) tptr;
  auto mode_ = to_cstring(env,mode);
  dummy_matrix dmat;
  try {
    dmat = exrpc_async(fm_node, (frovedis_kneighbors_graph<DT5,R_MAT1,KNC1,S_MAT15,S_LMAT15>), test_dptr, 
                              mid, k, mode_).get();
  }
  catch (std::exception& e) {
    set_status(true, e.what());
  }
  return to_jDummyMatrix(env, dmat, SCRS);
}

JNIEXPORT jdoubleArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_kncDoublePredict
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong tptr, jint mid,
   jboolean saveProba) { // TODO: add isDense
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto test_dptr = (exrpc_ptr_t) tptr;
  bool save_proba = (bool) saveProba;
  std::vector<double> label;
  try {
    label = exrpc_async(fm_node, (frovedis_knc_predict<DT1,DT5,R_MAT1,KNC1>), test_dptr, mid, save_proba).get(); 
  }
  catch (std::exception& e) {
      set_status(true, e.what());
  }
  return to_jdoubleArray(env, label);
}

JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_kncPredictProba
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong tptr, jint mid) { // TODO: add isDense
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto test_dptr = (exrpc_ptr_t) tptr;
  dummy_matrix dmat;
  try {
    dmat = exrpc_async(fm_node, (frovedis_knc_predict_proba<DT5,R_MAT1,KNC1,R_MAT1,R_LMAT1>), 
                               test_dptr, mid).get();
  }
  catch (std::exception& e) {
      set_status(true, e.what());
  }
  return to_jDummyMatrix(env, dmat, RMJR);
}

JNIEXPORT jfloat JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_kncModelScore
(JNIEnv *env, jclass thisCls, jobject master_node, jlong xptr, jlong yptr, jint mid) { // TODO: add isDense
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto mptr = (exrpc_ptr_t) xptr;
  auto lblptr = (exrpc_ptr_t) yptr;
  float res = 0.0;
  try {
    res = exrpc_async(fm_node, (frovedis_model_score<DT1,DT5,R_MAT1,KNC1>), mptr, lblptr, mid).get();
  }
  catch (std::exception& e) {
      set_status(true, e.what());
  }  
  return res;
}

// LDA
JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisLDATransform
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata,
   jint mid, jint num_topics, jint num_iter, jdouble alpha, jdouble beta,
   jint num_explore_iter, jstring algo) { 
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = (exrpc_ptr_t) fdata;
  dummy_lda_result ret;
  auto algo_ = to_cstring(env,algo);
  try{
    ret = exrpc_async(fm_node,(frovedis_lda_transform_for_spark<DT3,S_MAT15,LDA3>),
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
    ret = exrpc_async(fm_node,(get_topics_matrix<DT3,LDA3>),mid).get();
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
    ret = exrpc_async(fm_node,get_vocabulary_size<LDA3>,mid).get();
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
    ret = exrpc_async(fm_node,get_num_topics<LDA3>,mid).get();
  }
  catch(std::exception& e) { 
    set_status(true,e.what()); 
  }
  return ret;
}

JNIEXPORT void 
JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getDescribeMatrix
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid, jint nr,
   jint maxTermsPerTopic, jintArray word_id, jdoubleArray word_topic_dist) { 
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto sz = nr * maxTermsPerTopic;
  describeMatrix result;
  try {
    result = exrpc_async(fm_node, get_describe_matrix<LDA3>,mid,maxTermsPerTopic).get();
  }
  catch(std::exception& e) { set_status(true,e.what());}
  env->SetIntArrayRegion(word_id, 0, sz, result.wid_vec.data());
  env->SetDoubleArrayRegion(word_topic_dist, 0, sz, result.dist_vec.data());
}

// loads the lda model from the specified file
JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisLDAModel
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid, jstring path) {

  auto fs_path = to_cstring(env,path);
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  dummy_lda_model ret;
  try {
    ret = exrpc_async(fm_node,load_lda_model<LDA3>,mid,fs_path).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jDummyLDAModel(env, ret);
}
}
