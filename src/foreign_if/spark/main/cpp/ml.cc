#include "exrpc_ml.hpp"
#include "short_hand_dense_type.hpp"
#include "short_hand_sparse_type.hpp"
#include "spark_client_headers.hpp"
#include "exrpc_model.hpp"
#include "short_hand_model_type.hpp"

using namespace frovedis;

extern "C" {

// (1) --- Logistic Regression ---
// initiates the training call at Frovedis master node for LogisticRegression
JNIEXPORT int JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisLR
  (JNIEnv *env, jclass thisCls, jobject master_node, jobject fdata, 
   jint numIter, jdouble stepSize, jint histSize,
   jdouble mbf, jint rtype, 
   jdouble regParam, jboolean isMult, jboolean fit_intercept,
   jdouble tol, jint mid, jboolean movable, jboolean dense, jboolean shrink, 
   jdoubleArray sample_weight_ptr, jlong sample_weight_length,
   jstring solver, jboolean warmStart) {
  
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  bool mult = (bool) isMult;
  bool icpt = (bool) fit_intercept;
  bool mvbl = (bool) movable;
  bool isDense = (bool) dense;
  bool use_shrink = (bool) shrink;
  int vb = 0; // no log (default)
  size_t n_iter = 0;

  glm_config config;
  config.set_max_iter(numIter).
         set_alpha(stepSize).
         set_solver(to_cstring(env, solver)).
         set_intercept(icpt).
         set_mini_batch_fraction(mbf).
         set_tol(tol).
         set_warm_start(warmStart).
         set_reg_param(regParam).
         set_reg_type(get_regularizer_type(rtype)).
         set_is_mult(mult).
         set_hist_size(histSize);

  if(use_shrink) config.set_solver("shrink-sgd");
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node (" 
            << fm_node.hostname << "," << fm_node.rpcport 
            << ") to train frovedis LR.\n";
#endif
  try{
    std::vector<double> sample_weight = to_double_vector(env, 
                                      sample_weight_ptr, sample_weight_length);
    if(isDense) 
      n_iter = exrpc_async(fm_node,(frovedis_lr<DT1,D_MAT1>),f_dptr,
                           config,vb,mid,sample_weight,mvbl).get();
    else {
      n_iter = exrpc_async(fm_node,(frovedis_lr<DT1,S_MAT1>),f_dptr,
                           config,vb,mid,sample_weight,mvbl).get();
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return static_cast<int>(n_iter);
}

//--- SVM Kernel ---
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisKernelSVM
  (JNIEnv *env, jclass thisCls, jobject master_node, jobject fdata,
   jdouble C, jstring kernelType, jint degree, jdouble gamma, jdouble coef0,
   jdouble tol, jint cache_size, jint maxIter, jint mid, 
   jboolean movable, jboolean dense, jint nclasses) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  bool mvbl = (bool) movable;
  auto kernel = to_cstring(env,kernelType);
  bool isDense = (bool) dense;
  int vb = 0; // no log (default)
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to train frovedis SVM kernel.\n";
#endif

  try {
    if (nclasses > 2)
      throw std::invalid_argument("Currently frovedis SVM Kernel supports only binary classification!\n");

    if(isDense) {
      exrpc_oneway(fm_node,(frovedis_svc<DT1,R_MAT1>),f_dptr,tol,C,cache_size,maxIter,
                   kernel,gamma,coef0,degree,vb,mid,mvbl);
    }
    else REPORT_ERROR(USER_ERROR, "Frovedis doesn't support input sparse data for SVM Kernel!\n");
  }
  catch (std::exception& e) {
    set_status(true, e.what());
  }
}

// (2) --- Linear SVM ---
// initiates the training call at Frovedis master node for SVMWithSGD
JNIEXPORT int JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisSVM
  (JNIEnv *env, jclass thisCls, jobject master_node, jobject fdata, 
   jint numIter, jdouble stepSize, jint histSize, jdouble mbf, jdouble regParam,
   jint mid, jboolean movable, jboolean dense, jint nclasses, 
   jdoubleArray sample_weight_ptr, jlong sample_weight_length,
   jstring solver, jboolean warmStart) {
  
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  bool mvbl = (bool) movable;
  int rtype = 0; // ZERO (default)
  bool icpt = false; // default
  double tol = 0.001; // default //TODO: send from spark side
  int vb = 0; // no log (default)
  bool isDense = (bool) dense;
  size_t n_iter = 0;
  glm_config config;
  config.set_max_iter(numIter).
         set_alpha(stepSize).
         set_solver(to_cstring(env, solver)).
         set_intercept(icpt).
         set_mini_batch_fraction(mbf).
         set_tol(tol).
         set_warm_start(warmStart).
         set_reg_type(get_regularizer_type(rtype)).
         set_reg_param(regParam).
         set_hist_size(histSize);
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node (" 
            << fm_node.hostname << "," << fm_node.rpcport 
            << ") to train frovedis SVMD.\n";
#endif
  try {
    std::vector<double> sample_weight = to_double_vector(env, 
                                      sample_weight_ptr, sample_weight_length);
    if (nclasses > 2)
      throw std::invalid_argument("Currently frovedis SVM with SGD supports only binary classification!\n");

    if(isDense)
      n_iter = exrpc_async(fm_node,(frovedis_svm<DT1,D_MAT1>),f_dptr,
                   config,vb,mid,sample_weight,mvbl).get();
    else
      n_iter = exrpc_async(fm_node,(frovedis_svm<DT1,S_MAT1>),f_dptr,
                   config,vb,mid,sample_weight,mvbl).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return static_cast<int>(n_iter);
}

// initiates the training call at Frovedis master node for SVMRegressionWithSGD and LinearSVR
JNIEXPORT int JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisSVR
  (JNIEnv *env, jclass thisCls, jobject master_node, jobject fdata,
   jint numIter, jdouble stepSize, jdouble mbf,
   jdouble regParam, jstring regType, jstring lossName,
   jdouble eps, jboolean isIntercept, jdouble tol,
   jint mid, jboolean movable, jboolean dense, 
   jdoubleArray sample_weight_ptr, jlong sample_weight_length,
   jstring solver, jboolean warm_start) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  bool mvbl = (bool) movable;
  auto regTyp = to_cstring(env,regType);  
  int rtype = 0; // ZERO (default)
  if (regTyp == "ZERO")  rtype = 0;
  else if (regTyp == "L1") rtype = 1;
  else if (regTyp == "L2") rtype = 2;
  else REPORT_ERROR(USER_ERROR, "Unsupported regType is provided!\n");
  bool icpt = (bool) isIntercept;
  int vb = 0; // no log (default)
  bool isDense = (bool) dense;
  auto lossType = to_cstring(env,lossName);
  std::string loss; 
  if (lossType == "epsilon_insensitive")  loss = "EPS";
  else if (lossType == "squared_epsilon_insensitive") loss = "SQEPS";
  else REPORT_ERROR(USER_ERROR, "Unsupported loss type is provided!\n");
  size_t n_iter = 0;
  glm_config config;
  config.set_max_iter(numIter).
         set_alpha(stepSize).
         set_solver(to_cstring(env, solver)).
         set_intercept(icpt).
         set_mini_batch_fraction(mbf).
         set_tol(tol).
         set_warm_start(warm_start).
         set_reg_param(regParam).
         set_reg_type(get_regularizer_type(rtype)).
         set_loss_type(loss).
         set_epsilon(eps);
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to train frovedis Linear SVM Regression.\n";
#endif
  try {
    std::vector<double> sample_weight = to_double_vector(env, 
                                      sample_weight_ptr, sample_weight_length);
    if(isDense)
      n_iter = exrpc_async(fm_node,(frovedis_svm_regressor<DT1,D_MAT1>),f_dptr,
                   config,vb,mid,sample_weight,mvbl).get();
    else
      n_iter = exrpc_async(fm_node,(frovedis_svm_regressor<DT1,S_MAT1>),f_dptr,
                   config,vb,mid,sample_weight,mvbl).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return static_cast<int>(n_iter);
}

// (3) --- Linear Regression ---
// initiates the training call at Frovedis master node for LinearRegression
JNIEXPORT int JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisLNR
  (JNIEnv *env, jclass thisCls, jobject master_node, jobject fdata, 
   jint numIter, jdouble stepSize, jint histSize, jdouble mbf, 
   jint mid, jboolean movable, jboolean dense, 
   jdoubleArray sample_weight_ptr, jlong sample_weight_length,
   jstring solver, jboolean warmStart) {
  
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  bool mvbl = (bool) movable;
  bool icpt = false; // default
  int vb = 0; // no log (default)
  double tol = 0.001; // default (TODO: send from spark client)
  bool isDense = (bool) dense;
  glm_config config;
  config.set_max_iter(numIter).
         set_alpha(stepSize).
         set_solver(to_cstring(env, solver)).
         set_intercept(icpt).
         set_mini_batch_fraction(mbf).
         set_tol(tol).
         set_warm_start(warmStart).
         set_hist_size(histSize);

  lnr_result<double> res;
  std::vector<double> sample_weight = to_double_vector(env, 
                                      sample_weight_ptr, sample_weight_length);
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node (" 
            << fm_node.hostname << "," << fm_node.rpcport 
            << ") to train frovedis LNR.\n";
#endif
  try {
    if(isDense)
      res = exrpc_async(fm_node,(frovedis_lnr<DT1,D_MAT1>),f_dptr,
                   config,vb,mid,sample_weight,mvbl).get();
    else
      res = exrpc_async(fm_node,(frovedis_lnr<DT1,S_MAT1>),f_dptr,
                   config,vb,mid,sample_weight,mvbl).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return res.n_iter;
}

// (4) --- Lasso Regression ---
// initiates the training call at Frovedis master node for Lasso
JNIEXPORT int JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisLasso
  (JNIEnv *env, jclass thisCls, jobject master_node, jobject fdata, 
   jint numIter, jdouble stepSize, jint histSize, jdouble mbf, 
   jdouble regParam, jint mid, jboolean movable, jboolean dense, 
   jdoubleArray sample_weight_ptr, jlong sample_weight_length,
   jstring solver, jboolean warmStart) {
  
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  bool mvbl = (bool) movable;
  bool icpt = false; // default
  double tol = 0.001; // default
  int vb = 0; // no log (default)
  bool isDense = (bool) dense;
  size_t n_iter = 0;
  glm_config config;
  config.set_max_iter(numIter).
         set_alpha(stepSize).
         set_solver(to_cstring(env, solver)).
         set_intercept(icpt).
         set_mini_batch_fraction(mbf).
         set_tol(tol).
         set_warm_start(warmStart).
         set_reg_param(regParam).
         set_hist_size(histSize);

#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node (" 
            << fm_node.hostname << "," << fm_node.rpcport 
            << ") to train frovedis Lasso.\n";
#endif
  try {
    std::vector<double> sample_weight = to_double_vector(env, 
                                      sample_weight_ptr, sample_weight_length);
    if(isDense)
      n_iter = exrpc_async(fm_node,(frovedis_lasso<DT1,D_MAT1>),f_dptr,
                   config,vb,mid,sample_weight,mvbl).get();
    else
      n_iter = exrpc_async(fm_node,(frovedis_lasso<DT1,S_MAT1>),f_dptr,
                   config,vb,mid,sample_weight,mvbl).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return static_cast<int>(n_iter);
}

// (5) --- Ridge Regression ---
// initiates the training call at Frovedis master node for RidgeRegression
JNIEXPORT int JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisRidge
  (JNIEnv *env, jclass thisCls, jobject master_node, jobject fdata, 
   jint numIter, jdouble stepSize, jint histSize, jdouble mbf, 
   jdouble regParam, jint mid, jboolean movable, jboolean dense, 
   jdoubleArray sample_weight_ptr, jlong sample_weight_length,
   jstring solver, jboolean warmStart) {
  
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  bool mvbl = (bool) movable;
  bool icpt = false; // default
  double tol = 0.001; // default
  int vb = 0; // no log (default)
  bool isDense = (bool) dense;
  size_t n_iter = 0;

  glm_config config;
  config.set_max_iter(numIter).
         set_alpha(stepSize).
         set_solver(to_cstring(env, solver)).
         set_intercept(icpt).
         set_mini_batch_fraction(mbf).
         set_tol(tol).
         set_warm_start(warmStart).
         set_reg_param(regParam).
         set_hist_size(histSize);
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node (" 
            << fm_node.hostname << "," << fm_node.rpcport 
            << ") to train frovedis Ridge.\n";
#endif
  try {
    std::vector<double> sample_weight = to_double_vector(env, 
                                      sample_weight_ptr, sample_weight_length);
    if(isDense)
      n_iter = exrpc_async(fm_node,(frovedis_ridge<DT1,D_MAT1>),f_dptr,
                   config,vb,mid,sample_weight,mvbl).get();
    else
      n_iter = exrpc_async(fm_node,(frovedis_ridge<DT1,S_MAT1>),f_dptr,
                   config,vb,mid,sample_weight,mvbl).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return static_cast<int>(n_iter);
}

// (6) --- Matrix Factoriztion using ALS ---
// initiates the training call at Frovedis master node for MatrixFactorizationUsingALS 
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisMFUsingALS
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata,
   jint rank, jint numIter, jdouble alpha, jdouble sim_factor, jdouble regParam,
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
    exrpc_oneway(fm_node,(frovedis_mf_als<DT1,S_MAT1>),f_dptr,rank,
                 numIter,alpha,regParam,sim_factor,seed,vb,mid,mvbl);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// (7) --- Kmeans ---
// initiates the training call at Frovedis master node for KMeans 
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisKMeans
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata,
   jint k, jint numIter, jlong seed, jdouble epsilon,
   jint mid, jboolean movable, jboolean dense, jboolean use_shrink) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = (exrpc_ptr_t) fdata;
  bool mvbl = (bool) movable;
  int vb = 0; // no log (default)
  bool isDense = (bool) dense;
  bool shrink = (bool) use_shrink;
  int n_init = 1; // FIXME: send from spark side
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to train frovedis KMeans.\n";
#endif
  kmeans_result ret;
  try {
    if(isDense) { // kmeans accepts rowmajor matrix as for dense data
      ret = exrpc_async(fm_node,(frovedis_kmeans_fit<DT1,R_MAT1>),f_dptr,k,
                        numIter,n_init,epsilon,seed,vb,mid,shrink,mvbl).get();
    }
    else {
      ret = exrpc_async(fm_node,(frovedis_kmeans_fit<DT1,S_MAT1>),f_dptr,k,
                        numIter,n_init,epsilon,seed,vb,mid,shrink,mvbl).get();
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  std::cout << "frovedis kmeans converged in " << ret.n_iter_ 
            << " steps! inertia: " << ret.inertia_ << "\n";
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
  double thr = 0.0; //(TODO: Send from spark scala side)
  int ncls = 2; // TODO: Send from spark scala side
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to train frovedis agglomerative training.\n";
#endif
  try {
    if(isDense) { // agglomerative clustering accepts rowmajor matrix as for dense data
      auto lbl = exrpc_async(fm_node,(frovedis_aca<DT1,R_MAT1>),f_dptr,mid,link,ncls,thr,vb,mvbl).get();
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

  try {
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

    if(isDense) 
      exrpc_oneway(fm_node,(frovedis_dt<DT1,D_MAT1>),f_dptr,str,vb,mid,mvbl);
    else
      REPORT_ERROR(USER_ERROR, 
      "currently Frovedis doesn't support decision tree train with sparse data!\n");
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// (13) --- Naive Bayes ---
// calling frovedis server side Naive Bayes trainer
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisNBM
  (JNIEnv *env, jclass thisCls, jobject master_node, jobject fdata,
   jdouble lambda, jdouble threshold, jboolean fit_prior,
    jdoubleArray class_prior, jlong class_prior_length, 
   jdoubleArray sample_weight, jlong sample_weight_length, jint model_id, jstring modelType, 
   jboolean movable, jboolean dense) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  bool mvbl = (bool) movable;
  bool isDense = (bool) dense;
  int vb = 0; // no log (default)
  auto mtype = to_cstring(env,modelType);
  std::vector<double> clp_vec = to_double_vector(env,
                                class_prior, class_prior_length);
  std::vector<double> sw_vec = to_double_vector(env,
                               sample_weight, sample_weight_length);
  bool fp = (bool) fit_prior;
  try {
    if(isDense)
      exrpc_oneway(fm_node,(frovedis_nb<DT1,R_MAT1>),f_dptr,
                   mtype,lambda, fp, clp_vec, sw_vec, 
                   threshold,vb,model_id,mvbl);
    else
      exrpc_oneway(fm_node,(frovedis_nb<DT1,S_MAT1>),f_dptr,
                   mtype,lambda, fp, clp_vec, sw_vec,
                   threshold,vb,model_id,mvbl);
   
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// (14) --- FP-Growth ---
jobjectArray to_FrovedisSparkArray(JNIEnv *env, 
             std::vector<std::pair<std::vector<int>,long>>& fs) {
  jclass item_cls = env->FindClass(JRE_PATH_DummyFreqItemset);
  if (item_cls == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyFreqItemset class not found in path!\n");
  jmethodID constructor = env->GetMethodID(item_cls, "<init>", "([IJ)V");
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

JNIEXPORT int JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisFPM
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata,
   jdouble min_support, jint depth, jint c_point,
   jint opt_level, jint mid, jboolean movable) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = static_cast<exrpc_ptr_t> (fdata);
  bool mvbl = (bool) movable;
  int vb = 0; // no log (default)
  int fis_cnt = 0; // output
  try {
    fis_cnt = exrpc_async(fm_node, frovedis_fp_growth<dftable>, 
                          f_dptr, min_support, depth, c_point, opt_level, 
                          vb, mid, mvbl).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return fis_cnt;
}

// generate association rule
JNIEXPORT int JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisFPMR
  (JNIEnv *env, jclass thisCls, jobject master_node ,
   jdouble minConfidence, jint mid , jint midr) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  int count = 0;
  try {
    count = exrpc_async(fm_node, frovedis_fpr<fp_growth_model>, 
                        minConfidence, mid, midr).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return count;
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
   jint min_samples, jint mid, jboolean isDense, jdoubleArray sample_weight,
   jlong sample_weight_length) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = (exrpc_ptr_t) fdata;
  int vb = 0;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to train frovedis DBSCAN.\n";
#endif
  std::vector<int> ret;
  std::vector<double> sw_vec = to_double_vector(env, 
                               sample_weight, sample_weight_length);
  try {
    if(isDense){ // dbscan accepts rowmajor matrix for dense data
      ret = exrpc_async(fm_node,(frovedis_dbscan<DT1,R_MAT1>),f_dptr,sw_vec,eps,min_samples,vb,mid).get();
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
      exrpc_oneway(fm_node,(frovedis_knn<DT1,S_MAT15>), f_xptr, k, radius,
                       algorithm_, metric_, chunk_size, vb, mid);
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
      exrpc_oneway(fm_node,(frovedis_knc<DT1,S_MAT15>), f_dptr, k,
                       algorithm_, metric_, chunk_size, vb, mid);
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
      exrpc_oneway(fm_node,(frovedis_knr<DT1,S_MAT15>), f_dptr, k,
                       algorithm_, metric_, chunk_size, vb, mid);
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

// (21) --- Random Forest ---
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisRF
  (JNIEnv *env, jclass thisClass, jobject master_node, jobject fdata,
   jstring algoname, jint max_depth, jdouble min_info_gain, jint num_classes,
   jint max_bins, jdouble subsampling_rate,
   jstring impurityType, jint num_trees, jstring feature_subset, jlong seed,
   jintArray keys, jintArray values, jint size,
   jint mid, jboolean movable, jboolean dense) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  auto cat_ftr_info = get_kv_pair(env,keys,values,size);
  bool mvbl = (bool) movable;
  bool isDense = (bool) dense;
  int vb = 0; // no log (default)

  auto feature_subset_strategy = to_cstring(env, feature_subset);
  auto impurity_ = to_cstring(env,impurityType);
 
  try {
    auto al = to_cstring(env, algoname);
    tree::algorithm algo = tree::algorithm::Classification; // initializing
    if (al == "Classification")  algo = tree::algorithm::Classification;
    else if (al == "Regression") algo = tree::algorithm::Regression;
    else REPORT_ERROR(USER_ERROR, "Unsupported algorithm type is provided!\n");

    if(isDense) {
      tree::sampling_strategy<double> subsample_strat;
      subsample_strat.set_feature_subset_strategy(feature_subset_strategy)
                    .set_subsampling_rate(subsampling_rate);
      tree::strategy<double> strat;
      strat.set_algorithm(algo)
           .set_impurity_type(impurity_)
           .set_max_depth(max_depth)
           .set_min_info_gain(min_info_gain)
           .set_max_bins(max_bins)
           .set_categorical_features_info(cat_ftr_info)
           .set_num_trees(num_trees)
           .set_sampling_strategy(std::move(subsample_strat));
      if(to_cstring(env, algoname) == "Classification")
        strat.set_num_classes(num_classes);
      if(seed != -1)
        strat.set_seed(seed);
      exrpc_oneway(fm_node,(frovedis_rf<DT1,D_MAT1>),f_dptr,strat,vb,mid,mvbl);
    }
    else REPORT_ERROR(USER_ERROR, 
         "currently Frovedis doesn't support random forest train with sparse data!\n");
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// --- (22) GBT ---
JNIEXPORT void Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisGbtFit
  (JNIEnv *env, jclass thisCls, jobject master_node, jobject fdata,
   jstring algo, jstring loss, jstring impurity, 
   jdouble learning_rate,
   jint max_depth, jdouble min_info_gain,
   jint random_state, jdouble tol, jint max_bins,
   jdouble subsampling_rate, 
   jstring feature_subset_strategy,
   jint n_estimators, jint nclasses,
   jintArray keys, jintArray values, jint size,
   jint mid, jboolean movable, jboolean dense) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);

  auto algorithm = tree::algorithm::Regression;
  if(to_cstring(env, algo) == "Classification") 
    algorithm = tree::algorithm::Classification;
  else algorithm = tree::algorithm::Regression;

  auto impurity_ = to_cstring(env,impurity);
  auto feature_subset_strategy_ = to_cstring(env, feature_subset_strategy);
  auto loss_ = to_cstring(env, loss);
  bool isDense = (bool) dense;
  auto cat_ftr_info = get_kv_pair(env,keys,values,size);
  bool mvbl = (bool) movable;
  int verbose = 0;

  try {
    if (nclasses > 2) 
      REPORT_ERROR(USER_ERROR, 
      "Currently frovedis GBT Classifier supports only binary classification!\n");

    if(isDense) {
      tree::sampling_strategy<double> subsample_strat;
      subsample_strat.set_subsampling_rate(subsampling_rate)
                     .set_feature_subset_strategy(feature_subset_strategy_);
      // tol wont be used right now
      tree::strategy<double> strat;
      strat.set_algorithm(algorithm)
           .set_impurity_type(impurity_)
           .set_loss_type(loss_)
           .set_learning_rate(learning_rate)
           .set_max_depth(max_depth)
           .set_min_info_gain(min_info_gain)
           .set_max_bins(max_bins)
           .set_num_iterations(n_estimators)
           .set_sampling_strategy(std::move(subsample_strat))
           .set_categorical_features_info(cat_ftr_info);
      if(to_cstring(env, algo) == "Classification") 
        strat.set_num_classes(nclasses);
      if(random_state != -1)
        strat.set_seed(random_state);
      exrpc_oneway(fm_node,(frovedis_gbt<DT1,D_MAT1>),f_dptr,strat,verbose,mid,mvbl);
    }
    else REPORT_ERROR(USER_ERROR, 
         "Frovedis doesn't support input sparse data for GBT training!\n");
  }
  catch (std::exception& e) {
    set_status(true, e.what());
  }
}

// (23) --- Gaussian Mixture ---
JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisGMM
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata, 
   jint ncomponent, jstring covtype, jdouble tol, jint max_iter, 
   jstring init_type, jlong seed, jint mid, jboolean dense, jboolean movable) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = (exrpc_ptr_t) fdata;
  bool mvbl = (bool) movable;
  int vb = 0; // no log (default)
  int n_init = 1;
  auto cov_type = to_cstring(env, covtype);
  auto param_type = to_cstring(env, init_type); 
  bool isDense = (bool) dense;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to train frovedis gaussian mixture.\n";
#endif
  gmm_result ret;
  try {   
    if(isDense) { // gaussian mixture accepts rowmajor matrix as for dense data
      ret = exrpc_async(fm_node,(frovedis_gmm<DT1,R_MAT1>),f_dptr,mid,ncomponent,
                           cov_type, tol,max_iter,n_init,param_type,seed,vb,mvbl).get();
    }
    else REPORT_ERROR(USER_ERROR, 
         "Frovedis gaussian mixture doesn't accept sparse input at this moment.\n");
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return make_jIntDoublePair(env, ret.n_iter_, ret.likelihood_);
}    

}
