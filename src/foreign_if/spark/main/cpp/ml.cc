#include "exrpc_ml.hpp"
#include "short_hand_dense_type.hpp"
#include "short_hand_sparse_type.hpp"
#include "spark_client_headers.hpp"
#include "exrpc_model.hpp"
#include "short_hand_model_type.hpp"


using namespace frovedis;

extern "C" {

// (1) --- Logistic Regression ---
// initiates the training call at Frovedis master node for LogisticRegressionWithSGD
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisLRSGD
  (JNIEnv *env, jclass thisCls, jobject master_node, jobject fdata, 
   jint numIter, jdouble stepSize, jdouble mbf, jint rtype, 
   jdouble regParam, jboolean isMult, jboolean fit_intercept,
   jdouble tol, jint mid, jboolean movable, jboolean dense) {
  
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  bool mult = (bool) isMult;
  bool icpt = (bool) fit_intercept;
  bool mvbl = (bool) movable;
  bool isDense = (bool) dense;
  int vb = 0; // no log (default)
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node (" 
            << fm_node.hostname << "," << fm_node.rpcport 
            << ") to train frovedis LR_SGD.\n";
#endif
  try{
    if(isDense) 
      exrpc_oneway(fm_node,(frovedis_lr_sgd<DT1,D_MAT1>),f_dptr,numIter,stepSize,mbf,rtype,regParam,mult,icpt,tol,vb,mid,mvbl);
    else
      exrpc_oneway(fm_node,(frovedis_lr_sgd<DT1,S_MAT1>),f_dptr,numIter,stepSize,mbf,rtype,regParam,mult,icpt,tol,vb,mid,mvbl);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// initiates the training call at Frovedis master node for LogisticRegressionWithLBFGS
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisLRLBFGS
  (JNIEnv *env, jclass thisCls, jobject master_node, jobject fdata,
   jint numIter, jdouble stepSize, jint histSize, jint rtype,
   jdouble regParam, jboolean isMult, jboolean fit_intercept,
   jdouble tol, jint mid, jboolean movable, jboolean dense) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  bool mvbl = (bool) movable;
  bool mult = (bool) isMult;
  bool icpt = (bool) fit_intercept;
  bool isDense = (bool) dense;
  int vb = 0; // no log (default)
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to train frovedis LR_LBFGS.\n";
#endif
  try {
    if (isDense)
      exrpc_oneway(fm_node,(frovedis_lr_lbfgs<DT1,D_MAT1>),f_dptr,numIter,stepSize,histSize,rtype,regParam,mult,icpt,tol,vb,mid,mvbl);
    else
      exrpc_oneway(fm_node,(frovedis_lr_lbfgs<DT1,S_MAT1>),f_dptr,numIter,stepSize,histSize,rtype,regParam,mult,icpt,tol,vb,mid,mvbl);
  } 
  catch(std::exception& e) { set_status(true,e.what()); }
}

// (2) --- Linear SVM ---
// initiates the training call at Frovedis master node for SVMWithSGD
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisSVMSGD
  (JNIEnv *env, jclass thisCls, jobject master_node, jobject fdata, 
   jint numIter, jdouble stepSize, jdouble mbf, 
   jdouble regParam, jint mid, jboolean movable, jboolean dense) {
  
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  bool mvbl = (bool) movable;
  int rtype = 0; // ZERO (default)
  bool icpt = false; // default
  double tol = 0.001; // default
  int vb = 0; // no log (default)
  bool isDense = (bool) dense;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node (" 
            << fm_node.hostname << "," << fm_node.rpcport 
            << ") to train frovedis SVM_SGD.\n";
#endif
  try {
    if(isDense)
      exrpc_oneway(fm_node,(frovedis_svm_sgd<DT1,D_MAT1>),f_dptr,numIter,stepSize,mbf,rtype,regParam,icpt,tol,vb,mid,mvbl);
    else
      exrpc_oneway(fm_node,(frovedis_svm_sgd<DT1,S_MAT1>),f_dptr,numIter,stepSize,mbf,rtype,regParam,icpt,tol,vb,mid,mvbl);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// initiates the training call at Frovedis master node for SVMWithLBFGS
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisSVMLBFGS
  (JNIEnv *env, jclass thisCls, jobject master_node, jobject fdata,
   jint numIter, jdouble stepSize, jint histSize,
   jdouble regParam, jint mid, jboolean movable, jboolean dense) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  bool mvbl = (bool) movable;
  int rtype = 0; // ZERO (default)
  bool icpt = false; // default
  double tol = 0.001; // default
  int vb = 0; // no log (default)
  bool isDense = (bool) dense;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to train frovedis SVM_LBFGS.\n";
#endif
  try {
    if(isDense)
      exrpc_oneway(fm_node,(frovedis_svm_lbfgs<DT1,D_MAT1>),f_dptr,numIter,stepSize,histSize,rtype,regParam,icpt,tol,vb,mid,mvbl);
    else
      exrpc_oneway(fm_node,(frovedis_svm_lbfgs<DT1,S_MAT1>),f_dptr,numIter,stepSize,histSize,rtype,regParam,icpt,tol,vb,mid,mvbl);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// (3) --- Linear Regression ---
// initiates the training call at Frovedis master node for LinearRegressionWithSGD
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisLNRSGD
  (JNIEnv *env, jclass thisCls, jobject master_node, jobject fdata, 
   jint numIter, jdouble stepSize, jdouble mbf, 
   jint mid, jboolean movable, jboolean dense) {
  
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  bool mvbl = (bool) movable;
  bool icpt = false; // default
  int vb = 0; // no log (default)
  double tol = 0.001; // default (TODO: send from spark client)
  bool isDense = (bool) dense;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node (" 
            << fm_node.hostname << "," << fm_node.rpcport 
            << ") to train frovedis LNR_SGD.\n";
#endif
  try {
    if(isDense)
      exrpc_oneway(fm_node,(frovedis_lnr_sgd<DT1,D_MAT1>),f_dptr,numIter,stepSize,mbf,icpt,tol,vb,mid,mvbl);
    else
      exrpc_oneway(fm_node,(frovedis_lnr_sgd<DT1,S_MAT1>),f_dptr,numIter,stepSize,mbf,icpt,tol,vb,mid,mvbl);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// initiates the training call at Frovedis master node for LinearRegressionWithLBFGS
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisLNRLBFGS
  (JNIEnv *env, jclass thisCls, jobject master_node, jobject fdata,
   jint numIter, jdouble stepSize, jint histSize,
   jint mid, jboolean movable, jboolean dense) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  bool mvbl = (bool) movable;
  bool icpt = false; // default
  int vb = 0; // no log (default)
  double tol = 0.001; // default (TODO: send from spark client)
  bool isDense = (bool) dense;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to train frovedis LNR_LBFGS.\n";
#endif 
  try {
    if(isDense)
      exrpc_oneway(fm_node,(frovedis_lnr_lbfgs<DT1,D_MAT1>),f_dptr,numIter,stepSize,histSize,icpt,tol,vb,mid,mvbl);
    else
      exrpc_oneway(fm_node,(frovedis_lnr_lbfgs<DT1,S_MAT1>),f_dptr,numIter,stepSize,histSize,icpt,tol,vb,mid,mvbl);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// (4) --- Lasso Regression ---
// initiates the training call at Frovedis master node for LassoWithSGD
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisLassoSGD
  (JNIEnv *env, jclass thisCls, jobject master_node, jobject fdata, 
   jint numIter, jdouble stepSize, jdouble mbf, 
   jdouble regParam, jint mid, jboolean movable, jboolean dense) {
  
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  bool mvbl = (bool) movable;
  bool icpt = false; // default
  double tol = 0.001; // default
  int vb = 0; // no log (default)
  bool isDense = (bool) dense;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node (" 
            << fm_node.hostname << "," << fm_node.rpcport 
            << ") to train frovedis Lasso_SGD.\n";
#endif
  try {
    if(isDense)
      exrpc_oneway(fm_node,(frovedis_lasso_sgd<DT1,D_MAT1>),f_dptr,numIter,stepSize,mbf,regParam,icpt,tol,vb,mid,mvbl);
    else
      exrpc_oneway(fm_node,(frovedis_lasso_sgd<DT1,S_MAT1>),f_dptr,numIter,stepSize,mbf,regParam,icpt,tol,vb,mid,mvbl);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// initiates the training call at Frovedis master node for LassoWithLBFGS
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisLassoLBFGS
  (JNIEnv *env, jclass thisCls, jobject master_node, jobject fdata,
   jint numIter, jdouble stepSize, jint histSize,
   jdouble regParam, jint mid, jboolean movable, jboolean dense) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  bool mvbl = (bool) movable;
  bool icpt = false; // default
  double tol = 0.001; // default
  int vb = 0; // no log (default)
  bool isDense = (bool) dense;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to train frovedis Lasso_LBFGS.\n";
#endif
  try {
    if(isDense)
      exrpc_oneway(fm_node,(frovedis_lasso_lbfgs<DT1,D_MAT1>),f_dptr,numIter,stepSize,histSize,regParam,icpt,tol,vb,mid,mvbl);
    else
      exrpc_oneway(fm_node,(frovedis_lasso_lbfgs<DT1,S_MAT1>),f_dptr,numIter,stepSize,histSize,regParam,icpt,tol,vb,mid,mvbl);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// (5) --- Ridge Regression ---
// initiates the training call at Frovedis master node for RidgeRegressionWithSGD
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisRidgeSGD
  (JNIEnv *env, jclass thisCls, jobject master_node, jobject fdata, 
   jint numIter, jdouble stepSize, jdouble mbf, 
   jdouble regParam, jint mid, jboolean movable, jboolean dense) {
  
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  bool mvbl = (bool) movable;
  bool icpt = false; // default
  double tol = 0.001; // default
  int vb = 0; // no log (default)
  bool isDense = (bool) dense;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node (" 
            << fm_node.hostname << "," << fm_node.rpcport 
            << ") to train frovedis Ridge_SGD.\n";
#endif
  try {
    if(isDense)
      exrpc_oneway(fm_node,(frovedis_ridge_sgd<DT1,D_MAT1>),f_dptr,numIter,stepSize,mbf,regParam,icpt,tol,vb,mid,mvbl);
    else
      exrpc_oneway(fm_node,(frovedis_ridge_sgd<DT1,S_MAT1>),f_dptr,numIter,stepSize,mbf,regParam,icpt,tol,vb,mid,mvbl);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// initiates the training call at Frovedis master node for RidgeRegressionWithLBFGS
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisRidgeLBFGS
  (JNIEnv *env, jclass thisCls, jobject master_node, jobject fdata,
   jint numIter, jdouble stepSize, jint histSize,
   jdouble regParam, jint mid, jboolean movable, jboolean dense) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  bool mvbl = (bool) movable;
  bool icpt = false; // default
  double tol = 0.001; // default
  int vb = 0; // no log (default)
  bool isDense = (bool) dense;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to train frovedis Ridge_LBFGS.\n";
#endif
  try {
    if(isDense)
      exrpc_oneway(fm_node,(frovedis_ridge_lbfgs<DT1,D_MAT1>),f_dptr,numIter,stepSize,histSize,regParam,icpt,tol,vb,mid,mvbl);
    else
      exrpc_oneway(fm_node,(frovedis_ridge_lbfgs<DT1,S_MAT1>),f_dptr,numIter,stepSize,histSize,regParam,icpt,tol,vb,mid,mvbl);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// (6) --- Matrix Factoriztion using ALS ---
// initiates the training call at Frovedis master node for MatrixFactorizationUsingALS 
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisMFUsingALS
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata,
   jint rank, jint numIter, jdouble alpha, jdouble regParam,
   jlong seed, jint mid, jboolean movable) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = (exrpc_ptr_t) fdata;
  bool mvbl = (bool) movable;
  int vb = 0; // no log (default)
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to train frovedis MatrixFactorization using ALS.\n";
#endif
  try {
    exrpc_oneway(fm_node,(frovedis_mf_als<DT1,S_MAT1>),f_dptr,rank,numIter,alpha,regParam,seed,vb,mid,mvbl);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// (7) --- Kmeans ---
// initiates the training call at Frovedis master node for KMeans 
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisKMeans
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata,
   jint k, jint numIter, jlong seed, jdouble epsilon,
   jint mid, jboolean movable, jboolean dense) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = (exrpc_ptr_t) fdata;
  bool mvbl = (bool) movable;
  int vb = 0; // no log (default)
  bool isDense = (bool) dense;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to train frovedis KMeans.\n";
#endif
  try {
    if(isDense) // kmeans accepts rowmajor matrix as for dense data
      exrpc_oneway(fm_node,(frovedis_kmeans<DT1,R_MAT1>),f_dptr,k,numIter,seed,epsilon,vb,mid,mvbl);
    else
      exrpc_oneway(fm_node,(frovedis_kmeans<DT1,S_MAT1>),f_dptr,k,numIter,seed,epsilon,vb,mid,mvbl);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// (8) --- Agglomerative Clustering ---
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisACA
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata, 
   jint mid, jstring linkage, jboolean movable, jboolean dense) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = (exrpc_ptr_t) fdata;
  bool mvbl = (bool) movable;
  int vb = 0; // no log (default)
  bool isDense = (bool) dense;
  auto link = to_cstring(env,linkage);
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to train frovedis agglomerative training.\n";
#endif
  try {
    if(isDense) { // agglomerative clustering accepts rowmajor matrix as for dense data
      exrpc_oneway(fm_node,(frovedis_aca2<DT1,R_MAT1>),f_dptr,mid,link,vb,mvbl);
    }
    else REPORT_ERROR(USER_ERROR, 
         "Frovedis agglomerative clustering doesn't accept sparse input at this moment.\n");
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// (9) --- Spectral Clustering ---
JNIEXPORT jintArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisSCA
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata, jint ncluster, 
   jint niteration, jint ncomponent, jdouble eps, jdouble gamma,
   jboolean normlaplacian, jint mid, jboolean precomputed, 
   jint mode, jboolean drop_first,
   jboolean movable, jboolean dense) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = (exrpc_ptr_t) fdata;
  bool mvbl = (bool) movable;
  bool nlap = (bool) normlaplacian;
  bool pre = (bool) precomputed;
  int vb = 0; // no log (default)
  bool isDense = (bool) dense;
  bool drop = (bool) drop_first;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to train frovedis spectral clustering.\n";
#endif
  std::vector<int> ret;
   try {
    if(isDense){ // spectral clustering accepts rowmajor matrix as for dense data
       ret = exrpc_async(fm_node,(frovedis_sca<DT1,R_MAT1>),f_dptr,ncluster,
                         niteration,ncomponent,eps,gamma,nlap,mid,vb,pre,mode,drop,mvbl).get();
    }
    else REPORT_ERROR(USER_ERROR, 
         "Frovedis spectral clustering doesn't accept sparse input at this moment.\n");
  }
  catch(std::exception& e) { set_status(true,e.what()); }

  return to_jintArray(env,ret);
}

// (10) --- Spectral Embedding ---
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisSEA
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata, jint ncomponent,
   jdouble gamma,
   jboolean normlaplacian, jint mid, 
   jboolean precomputed, 
   jint mode, jboolean drop_first,
   jboolean movable, jboolean dense) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = (exrpc_ptr_t) fdata;
  bool nlap = (bool) normlaplacian;
  bool pre = (bool) precomputed;
  bool mvbl = (bool) movable;
  int vb = 0; // no log (default)
  bool isDense = (bool) dense;
  bool drop = (bool) drop_first;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to train frovedis spectral embedding.\n";
#endif
  try {
    if(isDense){ // spectral embedding accepts rowmajor matrix as for dense data
       exrpc_oneway(fm_node,(frovedis_sea<DT1,R_MAT1>),f_dptr,ncomponent,gamma,nlap,mid,vb,pre,mode,drop,mvbl);
    }
    else REPORT_ERROR(USER_ERROR, 
         "Frovedis embedding clustering doesn't accept sparse input at this moment.\n");
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// (11) --- Factorization Machine ---
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisFM
  (JNIEnv *env, jclass thisClass, jobject master_node, jobject fdata, 
   jdouble init_stdev, jdouble learn_rate, jint iteration, jstring optimizer, 
   jboolean is_regression, jint batch_size, 
   jboolean dim_1, jboolean dim_2, jint dim3, 
   jdouble reg1, jdouble reg2, jdouble reg3, 
   jint mid, jboolean movable) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  auto opt = to_cstring(env,optimizer);
  bool regr = (bool) is_regression;
  bool dim1 = (bool) dim_1;
  bool dim2 = (bool) dim_2;
  bool mvbl = (bool) movable;
  int vb = 0; // no log (default)

  fm::fm_config<DT1> conf(dim1, dim2, dim3, init_stdev, iteration, learn_rate,
                             reg1, reg2, reg3, regr, batch_size);
  try {
    exrpc_oneway(fm_node,(frovedis_fm<DT1,S_MAT1>),f_dptr,opt,conf,vb,mid,mvbl);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// (12) --- Decision Tree ---
std::unordered_map<size_t,size_t> 
get_kv_pair(JNIEnv *env, jintArray jkeys, jintArray jvals, size_t size) {
  auto keys = to_int_vector(env, jkeys, size);
  auto vals = to_int_vector(env, jvals, size);
  std::unordered_map<size_t,size_t> ret;
  for (size_t i=0; i<keys.size(); ++i) {
    auto k = static_cast<size_t> (keys[i]);
    auto v = static_cast<size_t> (vals[i]);
    ret[k] = v;
  }
  return ret;
}

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisDT
  (JNIEnv *env, jclass thisClass, jobject master_node, jobject fdata, 
   jstring algoname, jint max_depth, jint num_classes, jint max_bins, 
   jstring quantile_strategy, jdouble min_info_gain, jint min_instance, 
   jstring impurityType, 
   jintArray keys, jintArray values, jint size,
   jint mid, jboolean movable, jboolean dense) { 

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  auto cat_ftr_info = get_kv_pair(env,keys,values,size);
  bool mvbl = (bool) movable;
  bool isDense = (bool) dense;
  int vb = 0; // no log (default)

  auto al = to_cstring(env,algoname);
  tree::algorithm algo = tree::algorithm::Classification; // initializing
  if (al == "Classification")  algo = tree::algorithm::Classification;
  else if (al == "Regression") algo = tree::algorithm::Regression;
  else REPORT_ERROR(USER_ERROR, "Unsupported quantile strategy is provided!\n");

  auto qn = to_cstring(env,quantile_strategy);
  tree::quantile_strategy qns = tree::quantile_strategy::ApproxHist; // initializing
  if (qn == "ApproxHist") qns = tree::quantile_strategy::ApproxHist;
  else REPORT_ERROR(USER_ERROR, "Unsupported quantile strategy is provided!\n");

  auto impt = to_cstring(env,impurityType);
  tree::impurity_type impurity = tree::impurity_type::Default; // initializing
  if (impt == "default")       impurity = tree::impurity_type::Default;
  else if (impt == "gini")     impurity = tree::impurity_type::Gini;
  else if (impt == "entropy")  impurity = tree::impurity_type::Entropy;
  else if (impt == "variance") impurity = tree::impurity_type::Variance;
  else REPORT_ERROR(USER_ERROR, "Unsupported impurity type is provided!\n");

  tree::strategy<DT1> str(
      algo, impurity, 
      max_depth, num_classes, max_bins,
      qns, tree::categorize_strategy::Single,
      std::move(cat_ftr_info),
      min_instance, min_info_gain);

  try{
    if(isDense) 
      exrpc_oneway(fm_node,(frovedis_dt<DT1,D_MAT1>),f_dptr,str,vb,mid,mvbl);
    else
      REPORT_ERROR(USER_ERROR, "currently Frovedis doesn't support decision tree train with sparse data!\n");
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// (13) --- Naive Bayes ---
// calling frovedis server side Naive Bayes trainer
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisNBM
  (JNIEnv *env, jclass thisCls, jobject master_node, jobject fdata,
   jdouble lambda, jint model_id, jstring modelType, 
   jboolean movable, jboolean dense) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  bool mvbl = (bool) movable;
  bool isDense = (bool) dense;
  int vb = 0; // no log (default)
  auto mtype = to_cstring(env,modelType);
  try {
    if(isDense)
      exrpc_oneway(fm_node,(frovedis_nb<DT1,D_MAT1,D_LMAT1>),f_dptr,mtype,lambda,vb,model_id,mvbl);
    else
      exrpc_oneway(fm_node,(frovedis_nb<DT1,S_MAT1,S_LMAT1>),f_dptr,mtype,lambda,vb,model_id,mvbl);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// (14) --- FP-Growth ---
jobjectArray to_FrovedisSparkArray(JNIEnv *env, 
             std::vector<std::pair<std::vector<int>,long>>& fs) {
  jclass item_cls = env->FindClass(JRE_PATH_DummyFreqItemset);
  if (item_cls == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyFreqItemset class not found in path!\n");
  jmethodID constructor = env->GetMethodID(item_cls, "<init>", "([i;J)V");
  jobjectArray ret = env->NewObjectArray(fs.size(), item_cls, NULL);
  if (ret == NULL) REPORT_ERROR(INTERNAL_ERROR, "New DummyFreqItemset Array allocation failed!\n");
  for(int j=0; j<fs.size(); j++) {
    auto each = fs[j];
    auto vec = each.first;
    int* arr = &vec[0];
    int sz = vec.size();
    jintArray iarray = env->NewIntArray(sz);
    if(iarray == NULL) REPORT_ERROR(INTERNAL_ERROR, "New jintArray allocation failed.\n");
    env->SetIntArrayRegion(iarray, 0, sz, arr);
    jobject obj = env->NewObject(item_cls, constructor, iarray, (jlong) each.second); //TODO: SIGSEV 
    if(obj == NULL) REPORT_ERROR(INTERNAL_ERROR, "New object creation failed.\n");
    env->SetObjectArrayElement(ret, j, obj);
  }
  return ret;
}

JNIEXPORT jobjectArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_toSparkFPM
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto vec = exrpc_async(fm_node, get_frovedis_fpm<int>, mid).get();
  return to_FrovedisSparkArray(env,vec);
}

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisFPM
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata,
   jdouble minSupport, jint mid, jboolean movable) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = static_cast<exrpc_ptr_t> (fdata);
  bool mvbl = (bool) movable;
  int vb = 0; // no log (default)
  try {
    exrpc_oneway(fm_node,frovedis_fp_growth<dftable>,f_dptr,minSupport,vb,mid,mvbl);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// generate association rule
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisFPMR
  (JNIEnv *env, jclass thisCls, jobject master_node ,
   jdouble minConfidence, jint mid , jint midr) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  try {
    exrpc_oneway(fm_node,frovedis_fpr<fp_growth_model>,minConfidence,mid,midr);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// (15) --- Word2Vector ---
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisW2V
  (JNIEnv *env, jclass thisCls, jobject master_node, 
   jlong hash_ptr, jintArray vocab_count, jint vocab_size, 
   jint hidden_size, jint window, 
   jfloat sample, jint negative, jint iter, jdouble alpha, 
   jfloat model_sync_period, jint min_sync_words, jint full_sync_times, 
   jint message_size, jint num_threads, jint mid) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto hsh_dptr = static_cast<exrpc_ptr_t> (hash_ptr);
  auto count_vector = to_int_vector(env, vocab_count, vocab_size);

  // --- input alert ---
  int are_supposed_parameters =    \
    hidden_size <= 512 &&          \
    hidden_size % 2 == 0 &&        \
    negative == 5 &&               \
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

  float al = (float) alpha; // spark side alpha is double
  w2v::train_config config = {
    hidden_size, window, sample, negative, iter,
    al, model_sync_period, min_sync_words,
    full_sync_times, message_size, num_threads
  };

  try {
    exrpc_oneway(fm_node, frovedis_w2v<DT2>, hsh_dptr, count_vector, config, mid); //float
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// (16) --- DBSCAN ---
JNIEXPORT jintArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisDBSCAN
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata, jdouble eps,
   jint min_samples, jint mid, jboolean isDense) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = (exrpc_ptr_t) fdata;
  int vb = 0;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to train frovedis DBSCAN.\n";
#endif
  std::vector<int> ret;
  try {
    if(isDense){ // dbscan accepts rowmajor matrix for dense data
      ret = exrpc_async(fm_node,(frovedis_dbscan<DT1,R_MAT1>),f_dptr,eps,min_samples,vb,mid).get();
    }
    else REPORT_ERROR(USER_ERROR,
         "Frovedis DBSCAN doesn't accept sparse input at this moment.\n");
  }
  catch(std::exception& e) { set_status(true,e.what()); }

  return to_jintArray(env,ret);
}

// --- (17) K-Nearest Neighbor (KNN) ---
JNIEXPORT void Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisKnnFit
            (JNIEnv *env, jclass thisCls, jobject master_node, jlong xptr, jint k,
             jfloat radius, jstring algorithm, jstring metric,
             jfloat chunk_size, jint mid, jboolean dense) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_xptr = (exrpc_ptr_t) xptr;
  auto algorithm_ = to_cstring(env,algorithm);
  auto metric_ = to_cstring(env,metric);
  bool isDense= (bool) dense;
  int vb = 0; // no log (default)
  try {
    if(isDense) {
      exrpc_oneway(fm_node,(frovedis_knn<DT1,R_MAT1>), f_xptr, k, radius,
                       algorithm_, metric_, chunk_size, vb, mid);
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
JNIEXPORT void Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisKncFit
          (JNIEnv *env, jclass thisCls, jobject master_node, jobject fdata, jint k,
           jstring algorithm, jstring metric,
           jfloat chunk_size, jint mid, jboolean dense) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  auto algorithm_ = to_cstring(env,algorithm);
  auto metric_ = to_cstring(env,metric);
  bool isDense= (bool) dense;
  int vb = 0; // no log (default)
  try {
    if(isDense) {
      exrpc_oneway(fm_node,(frovedis_knc<DT1,R_MAT1>), f_dptr, k,
                       algorithm_, metric_, chunk_size, vb, mid);
    }
    else{
      REPORT_ERROR(USER_ERROR, "frovedis KNeighbors Classifier currently supports only dense data. \n");
    }
  }
  catch (std::exception& e) {
    set_status(true, e.what());
  }
}

// --- (19) K-Nearest Neighbor Regressor (KNR) ---
JNIEXPORT void Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisKnrFit
            (JNIEnv *env, jclass thisCls, jobject master_node, jobject fdata, jint k,
             jstring algorithm, jstring metric,
             jfloat chunk_size, jint mid, jboolean dense) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  auto algorithm_ = to_cstring(env,algorithm);
  auto metric_ = to_cstring(env,metric);
  bool isDense= (bool) dense;
  int vb = 0; // no log (default)
  try {
    if(isDense) {
      exrpc_oneway(fm_node,(frovedis_knr<DT1,R_MAT1>), f_dptr, k,
                       algorithm_, metric_, chunk_size, vb, mid);
    }
    else{
      REPORT_ERROR(USER_ERROR, "frovedis KNeighbors Regressor currently supports only dense data. \n");
    }
  }
  catch (std::exception& e) {
    set_status(true, e.what());
  }
}

// --- (20) Latent Dirichlet Allocation (LDA) ---
JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisLDA
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata,
   jlongArray orig_doc_id, jlong num_docs, jint mid, 
   jint num_topics, jint num_iter, jdouble alpha, jdouble beta,
   jint num_explore_iter, jint num_eval_cycle, jstring algo) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = (exrpc_ptr_t) fdata;
  std::vector<long> org_doc_id_vector = to_long_vector(env, orig_doc_id, num_docs);
  int vb = 0; // no log (default)
  auto algo_ = to_cstring(env,algo);
  dummy_lda_model ret;
  try {
    ret = exrpc_async(fm_node,(frovedis_lda_train_for_spark<DT3,S_MAT15>), f_dptr, 
                      org_doc_id_vector, alpha, beta, num_topics, 
                      num_iter, algo_, num_explore_iter, num_eval_cycle, vb, mid).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jDummyLDAModel(env, ret);
}

}
