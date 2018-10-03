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
      case SVM:    exrpc_oneway(fm_node, show_model<SVM1>, mid); break;
      case LNRM:   exrpc_oneway(fm_node, show_model<LNRM1>, mid); break;
      case MFM:    exrpc_oneway(fm_node, show_model<MFM1>, mid); break;
      case KMEANS: exrpc_oneway(fm_node, show_model<KMM1>, mid); break;
      case DTM:    exrpc_oneway(fm_node, show_model<DTM1>, mid); break;
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
      case SVM:    exrpc_oneway(fm_node, release_model<SVM1>, mid); break;
      case LNRM:   exrpc_oneway(fm_node, release_model<LNRM1>, mid); break;
      case MFM:    exrpc_oneway(fm_node, release_model<MFM1>, mid); break;
      case KMEANS: exrpc_oneway(fm_node, release_model<KMM1>, mid); break;
      case DTM:    exrpc_oneway(fm_node, release_model<DTM1>, mid); break;
      case FMM:    exrpc_oneway(fm_node, release_model<FMM1>, mid); break;
      case NBM:    exrpc_oneway(fm_node, release_model<NBM1>, mid); break;
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
      case FMM:  REPORT_ERROR(USER_ERROR,"currently Frovedis fm_model can't be loaded!\n"); 
      case DTM:  exrpc_oneway(fm_node,load_model<DTM1>,mid,DTM,fs_path); break;
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

}
