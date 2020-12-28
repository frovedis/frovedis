#include "exrpc_data_storage.hpp"
#include "short_hand_dense_type.hpp"
#include "short_hand_sparse_type.hpp"
#include "spark_client_headers.hpp"

using namespace frovedis;

extern "C" {

// it returns a mem_pair containing the heads of crs_matrix_local and vector (for labels)
JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisWorkerGLMData
  (JNIEnv *env, jclass thisCls, jobject target_node, jlong nrows, jlong ncols, 
   jdoubleArray lbl, jintArray off, jintArray idx, jdoubleArray val) {

  auto l_mat = get_frovedis_double_crs_matrix_local(env,nrows,ncols,off,idx,val);
  jsize v_len = env->GetArrayLength(lbl);
  jdouble *lblp = env->GetDoubleArrayElements(lbl, 0); double* lblp_ = lblp;
  std::vector<double> label(v_len);
  for(size_t i=0; i<v_len; ++i) label[i] = lblp_[i];
  env->ReleaseDoubleArrayElements(lbl,lblp,JNI_ABORT); 

  auto fw_node = java_node_to_frovedis_node(env,target_node);
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to worker node (" 
            << fw_node.hostname << "," << fw_node.rpcport 
            << ") to load local data.\n";
#endif
  frovedis_mem_pair l_dptr;
  try {
    l_dptr = exrpc_async(fw_node,(load_local_glm_data<DT1,S_LMAT1>),l_mat,label).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
 
  return frovedis_mempair_to_java_mempair(env, l_dptr);
}

// it returns a memptr pointing to the heads of crs_matrix_local (sparse data)
JNIEXPORT jlong JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisWorkerData
  (JNIEnv *env, jclass thisCls, jobject target_node, jlong nrows, jlong ncols,
   jintArray off, jintArray idx, jdoubleArray val) {

  auto l_mat = get_frovedis_double_crs_matrix_local(env,nrows,ncols,off,idx,val);
  auto fw_node = java_node_to_frovedis_node(env,target_node);
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to worker node ("
            << fw_node.hostname << "," << fw_node.rpcport
            << ") to load local data.\n";
#endif
 
  exrpc_ptr_t l_dptr = 0;
  try{
    l_dptr = exrpc_async(fw_node,load_local_data<S_LMAT1>,l_mat).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return (jlong) l_dptr;
}

// it returns a memptr pointing to the heads of local vectors (string coo data)
JNIEXPORT jlong JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisWorkerVectorStringData
  (JNIEnv *env, jclass thisCls, jobject target_node, jobjectArray val, jint size) {

  auto fw_node = java_node_to_frovedis_node(env,target_node);
  auto l_vec = to_string_vector(env,val,size);
  exrpc_ptr_t l_dptr = 0;
  try { 
    l_dptr = exrpc_async(fw_node,load_local_data<std::vector<std::string>>,l_vec).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return (jlong) l_dptr;
}

// it returns a memptr pointing to the heads of created rowmajor_matrix_local 
JNIEXPORT jlong JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisWorkerRmajorData
  (JNIEnv *env, jclass thisCls, jobject target_node, 
   jlong nrows, jlong ncols, jdoubleArray val) {

  jdouble *valp = env->GetDoubleArrayElements(val, 0); double* valp_ = valp;
  rowmajor_matrix_local<double> rmat(nrows,ncols,valp_);
  env->ReleaseDoubleArrayElements(val,valp,JNI_ABORT); 
  auto fw_node = java_node_to_frovedis_node(env,target_node);
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to worker node ("
            << fw_node.hostname << "," << fw_node.rpcport
            << ") to load local rowmajor data.\n";
#endif
  exrpc_ptr_t l_dptr = 0;
  try{
     l_dptr = exrpc_async(fw_node,load_local_data<R_LMAT1>,rmat).get();
   }
  catch(std::exception& e) { set_status(true,e.what()); }
  return (jlong) l_dptr;
}

// it returns a memptr pointing to the heads of created rowmajor_matrix_local 
JNIEXPORT jlong JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisWorkerRmajorMatData
  (JNIEnv *env, jclass thisCls, jobject target_node, 
   jlong nrows, jlong ncols, jobjectArray mat_data) {

  jsize d_len = env->GetArrayLength(mat_data);
  if(d_len != nrows) REPORT_ERROR(INTERNAL_ERROR, "Error in data extraction from JRE");
  rowmajor_matrix_local<double> rmat(nrows,ncols);
  auto rmjr_valp = rmat.val.data();
  // flattening input 2D double jarray to create 1D double std::vector
  for(size_t i = 0; i < nrows; ++i) {
    jdoubleArray jdarr = (jdoubleArray) (env->GetObjectArrayElement(mat_data,i));
    jdouble *valp = env->GetDoubleArrayElements(jdarr, 0); double* valp_ = valp;
    for (size_t j = 0; j < ncols; ++j) rmjr_valp[i*ncols+j] = valp_[j];
    env->ReleaseDoubleArrayElements(jdarr,valp,JNI_ABORT); 
  }
  auto fw_node = java_node_to_frovedis_node(env,target_node);
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to worker node ("
            << fw_node.hostname << "," << fw_node.rpcport
            << ") to load local rowmajor data.\n";
#endif
  exrpc_ptr_t l_dptr = 0;
  try{
     l_dptr = exrpc_async(fw_node,load_local_data<R_LMAT1>,rmat).get();
   }
  catch(std::exception& e) { set_status(true,e.what()); }
  return (jlong) l_dptr;
}

// converts 'mtype' to rowmajor_matrix type and return its memory head
JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getFrovedisRowmajorMatrix
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata, jshort mtype) {

  auto fm_node = java_node_to_frovedis_node(env,master_node);
  auto f_dptr = (exrpc_ptr_t) fdata;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to convert mtype to rowmajor_matrix type.\n";
#endif
  dummy_matrix ret;
  try {
     switch(mtype) {
       case CMJR: ret = exrpc_async(fm_node,(to_rowmajor_matrix<DT1,C_MAT1>),f_dptr).get(); break;
       case BCLC: ret = exrpc_async(fm_node,(to_rowmajor_matrix<DT1,B_MAT1>),f_dptr).get(); break;
       default:   REPORT_ERROR(USER_ERROR,"Unknown dense matrix kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jDummyMatrix(env,ret,RMJR); //returned object is rowmajor matrix
}

// converts crs to rowmajor_matrix type and return its memory head
JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_crsToFrovedisRowmajorMatrix
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata) {

  auto fm_node = java_node_to_frovedis_node(env,master_node);
  auto f_dptr = (exrpc_ptr_t) fdata;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to convert crs to rowmajor_matrix type.\n";
#endif
  dummy_matrix ret;
  try {
    ret = exrpc_async(fm_node,(to_rowmajor_matrix<DT1,S_MAT15>),f_dptr).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jDummyMatrix(env,ret,RMJR); //returned object is rowmajor matrix
}

// returns an array containing memory heads of the romajwor_matrix_locals
JNIEXPORT jlongArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getAllLocalPointers
  (JNIEnv *env, jclass thisCls, jobject master_node, 
   jlong r_dptr, jshort mtype) {

  auto fm_node = java_node_to_frovedis_node(env,master_node);
  auto f_dptr = (exrpc_ptr_t) r_dptr;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to get each local data heads.\n";
#endif
  std::vector<exrpc_ptr_t> eps;
  try {
    switch(mtype) {
      case RMJR: eps = exrpc_async(fm_node,(get_all_local_pointers<R_MAT1,R_LMAT1>),f_dptr).get(); break;
      case CMJR: eps = exrpc_async(fm_node,(get_all_local_pointers<C_MAT1,C_LMAT1>),f_dptr).get(); break;
      case BCLC: eps = exrpc_async(fm_node,(get_all_local_pointers<B_MAT1,B_LMAT1>),f_dptr).get(); break;
      default: REPORT_ERROR(USER_ERROR,"Unknown dense matrix kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jlongArray(env,eps); 
}

// returns each local array from specified frovedis worker nodes
JNIEXPORT jdoubleArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getLocalArray
  (JNIEnv *env, jclass thisCls, jobject target_node, 
   jlong fdata, jshort mtype) {

  auto fw_node = java_node_to_frovedis_node(env,target_node); // from worker node
  auto f_dptr = (exrpc_ptr_t) fdata;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to worker node ("
            << fw_node.hostname << "," << fw_node.rpcport
            << ") to get each local array.\n";
#endif
  std::vector<double> ret;
  try {
    switch(mtype) {
      case RMJR_L: ret = exrpc_async(fw_node,(get_local_array<DT1,R_LMAT1>),f_dptr).get(); break;
      case CMJR_L: ret = exrpc_async(fw_node,(get_local_array<DT1,C_LMAT1>),f_dptr).get(); break;
      case BCLC_L: ret = exrpc_async(fw_node,(get_local_array<DT1,B_LMAT1>),f_dptr).get(); break;
      default: REPORT_ERROR(USER_ERROR,"Unknown local dense matrix kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jdoubleArray(env,ret);
}

// converts input exrpc::matrix to colmajor array
JNIEXPORT jdoubleArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getColmajorArray
  (JNIEnv *env, jclass thisCls, jobject master_node, 
   jlong fdata, jshort mtype) {

  auto fm_node = java_node_to_frovedis_node(env,master_node); 
  auto f_dptr = (exrpc_ptr_t) fdata;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to get colmajor array from input exrpc matrix.\n";
#endif
  std::vector<double> ret;
  try {
    switch(mtype) {
      case RMJR: ret = exrpc_async(fm_node,rowmajor_to_colmajor_array<DT1>,f_dptr).get(); break;
      case CMJR: ret = exrpc_async(fm_node,colmajor_to_colmajor_array<DT1>,f_dptr).get(); break;
      case BCLC: ret = exrpc_async(fm_node,blockcyclic_to_colmajor_array<DT1>,f_dptr).get(); break;
      default: REPORT_ERROR(USER_ERROR,"local dense matrix kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jdoubleArray(env,ret);
}

// converts input exrpc::matrix to rowmajor array
JNIEXPORT jdoubleArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getRowmajorArray
  (JNIEnv *env, jclass thisCls, jobject master_node, 
   jlong fdata, jshort mtype) {

  auto fm_node = java_node_to_frovedis_node(env,master_node); 
  auto f_dptr = (exrpc_ptr_t) fdata;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to get rowmajor array from input exrpc matrix.\n";
#endif
  std::vector<double> ret;
  try{
    switch(mtype) {
      case RMJR: ret = exrpc_async(fm_node,(matrix_to_array<DT1,R_MAT1,R_LMAT1>),f_dptr).get(); break;
      case CMJR: ret = exrpc_async(fm_node,(to_rowmajor_array<DT1,C_MAT1>),f_dptr).get(); break;
      case BCLC: ret = exrpc_async(fm_node,(to_rowmajor_array<DT1,B_MAT1>),f_dptr).get(); break;
      default: REPORT_ERROR(USER_ERROR,"Unknown dense matrix kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jdoubleArray(env,ret);
}

// returns java double array from frovedis side std::vector<doubel>
JNIEXPORT jdoubleArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getDoubleArray
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata) {

  auto fm_node = java_node_to_frovedis_node(env,master_node);
  auto f_dptr = (exrpc_ptr_t) fdata;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to get double array from server side vector.\n";
#endif
  std::vector<double> ret;
  try{
    ret = exrpc_async(fm_node,vec_to_array<DT1>,f_dptr).get(); 
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jdoubleArray(env,ret);
}

// creates the global Frovedis data at master node and returns 
// a pair of created crs_matrix and dvector heads
JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_createFrovedisLabeledPoint
  (JNIEnv *env, jclass thisCls, jobject master_node, 
   jobjectArray eps, jlong nrows, jlong ncols) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  jsize len = env->GetArrayLength(eps); // total worker nodes
  std::vector<exrpc_ptr_t> lmat(len), l_vec(len);
  for(size_t i=0; i<len; ++i) {
    auto jmptr = env->GetObjectArrayElement(eps, i);
    auto fmptr = java_mempair_to_frovedis_mempair(env,jmptr);
    lmat[i] = fmptr.first();
    l_vec[i] = fmptr.second();
  }
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node (" 
            << fm_node.hostname << "," << fm_node.rpcport 
            << ") to create and set data.\n";
#endif
  auto r = static_cast<size_t>(nrows);
  auto c = static_cast<size_t>(ncols);
  frovedis_mem_pair g_dptr;
  try{
    g_dptr = exrpc_async(fm_node,(create_and_set_glm_data<DT1,S_MAT1,S_LMAT1>),lmat,l_vec,r,c).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return frovedis_mempair_to_java_mempair(env, g_dptr);
}

// creates Frovedis::crs_matrix<double> at master node and returns its data pointer
JNIEXPORT jlong JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_createFrovedisSparseData
  (JNIEnv *env, jclass thisCls, jobject master_node,
   jlongArray eps, jlong nrows, jlong ncols) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  jlong *eps_p = env->GetLongArrayElements(eps, 0); 
  jsize len = env->GetArrayLength(eps); // total worker nodes
  std::vector<exrpc_ptr_t> lmat(len);
  for(size_t i=0; i<len; ++i) lmat[i] = (exrpc_ptr_t) eps_p[i];
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to create and set data.\n";
#endif
  auto r = static_cast<size_t>(nrows);
  auto c = static_cast<size_t>(ncols);
  exrpc_ptr_t g_dptr = 0;
  try {
    g_dptr = exrpc_async(fm_node,(create_and_set_data<S_MAT1,S_LMAT1>),lmat,r,c).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return (jlong) g_dptr;
}

// creates Frovedis::crs_matrix<double> at master node from local coo string vectors 
JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_createFrovedisSparseMatrix
  (JNIEnv *env, jclass thisCls, jobject master_node, jlongArray eps, jshort mtype) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  jlong *eps_p = env->GetLongArrayElements(eps, 0); 
  jsize len = env->GetArrayLength(eps); // total worker nodes
  std::vector<exrpc_ptr_t> lmat(len);
  for(size_t i=0; i<len; ++i) lmat[i] = (exrpc_ptr_t) eps_p[i];
  dummy_matrix ret;
  try {
     ret = exrpc_async(fm_node,create_crs_from_local_coo_string_vectors<DT1>,lmat).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jDummyMatrix(env,ret,mtype);
}

// creates frovedis dense data (rmjr, cmjr, bclc etc.) at master node and returns its data pointer
JNIEXPORT jlong JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_createFrovedisDenseData
  (JNIEnv *env, jclass thisCls, jobject master_node,
   jlongArray eps, jlong nrows, jlong ncols, jshort mtype) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  jlong *eps_p = env->GetLongArrayElements(eps, 0);
  jsize len = env->GetArrayLength(eps); // total worker nodes
  std::vector<exrpc_ptr_t> lmat(len);
  for(size_t i=0; i<len; ++i) lmat[i] = (exrpc_ptr_t) eps_p[i];
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to create and set frovedis dense data.\n";
#endif
  auto r = static_cast<size_t>(nrows);
  auto c = static_cast<size_t>(ncols);
  exrpc_ptr_t g_dptr = 0;
  try {
    switch(mtype) {
      case RMJR: g_dptr = exrpc_async(fm_node,create_rmm_data<DT1>,lmat,r,c).get(); break;
      case CMJR: g_dptr = exrpc_async(fm_node,create_cmm_data<DT1>,lmat,r,c).get(); break;
      case BCLC: g_dptr = exrpc_async(fm_node,create_bcm_data<DT1>,lmat,r,c).get(); break;
      default:   REPORT_ERROR(USER_ERROR,"Unknown dense matrix kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return (jlong) g_dptr;
}

// releases the dynamically allocated frovedis glm data from Frovedis nodes
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_releaseFrovedisLabeledPoint
  (JNIEnv *env, jclass thisCls, jobject master_node, jobject fdata, 
   jboolean dense, jshort mtype) {
  
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  bool isDense = (bool) dense;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node (" 
            << fm_node.hostname << "," << fm_node.rpcport 
            << ") to release frovedis data.\n";
#endif
  try{
     if(isDense) {
       switch(mtype) {     
         case RMJR: exrpc_oneway(fm_node,(release_glm_data<DT1,R_MAT1>),f_dptr); break;
         case CMJR: exrpc_oneway(fm_node,(release_glm_data<DT1,C_MAT1>),f_dptr); break;
         default:   REPORT_ERROR(USER_ERROR,"Unsupported dense matrix kind is encountered!\n");
       }
     }
     else exrpc_oneway(fm_node,(release_glm_data<DT1,S_MAT1>),f_dptr);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// releases the dynamically allocated frovedis data from Frovedis nodes
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_releaseFrovedisSparseData
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = (exrpc_ptr_t) fdata;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to release frovedis data.\n";
#endif
  try {
    exrpc_oneway(fm_node,release_data<S_MAT1>,f_dptr);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// releases the dynamically allocated frovedis dense data from Frovedis nodes
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_releaseFrovedisDenseData
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata, jshort mtype) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = (exrpc_ptr_t) fdata;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to release frovedis dense data.\n";
#endif
  try{
    switch(mtype) {
      case RMJR: exrpc_oneway(fm_node,release_data<R_MAT1>,f_dptr); break;
      case CMJR: exrpc_oneway(fm_node,release_data<C_MAT1>,f_dptr); break;
      case BCLC: exrpc_oneway(fm_node,release_data<B_MAT1>,f_dptr); break;
      default:   REPORT_ERROR(USER_ERROR,"Unknown dense matrix kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// prints the created frovedis glm data for debugging purpose
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_showFrovedisLabeledPoint
  (JNIEnv *env, jclass thisCls, jobject master_node, jobject fdata,
   jboolean dense, jshort mtype) {
  
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  bool isDense = (bool) dense;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node (" 
            << fm_node.hostname << "," << fm_node.rpcport 
            << ") to print frovedis data.\n";
#endif
  try{
     if(isDense) { 
       switch(mtype) {     
         case RMJR: exrpc_oneway(fm_node,(show_glm_data<DT1,R_MAT1>),f_dptr); break;
         case CMJR: exrpc_oneway(fm_node,(show_glm_data<DT1,C_MAT1>),f_dptr); break;
         default:   REPORT_ERROR(USER_ERROR,"Unsupported dense matrix kind is encountered!\n");
       }
     }
     else exrpc_oneway(fm_node,(show_glm_data<DT1,S_MAT1>),f_dptr);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// prints the created frovedis data for debugging purpose
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_showFrovedisSparseData
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = (exrpc_ptr_t) fdata;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to print frovedis data.\n";
#endif
  try {
    exrpc_oneway(fm_node,show_data<S_MAT1>,f_dptr);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// prints the created frovedis dense data for debugging purpose
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_showFrovedisDenseData
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata, jshort mtype) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = (exrpc_ptr_t) fdata;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to print frovedis dense data.\n";
#endif
  try{
    switch(mtype) {
      case RMJR: exrpc_oneway(fm_node,show_data<R_MAT1>,f_dptr); break;
      case CMJR: exrpc_oneway(fm_node,show_data<C_MAT1>,f_dptr); break;
      case BCLC: exrpc_oneway(fm_node,show_data<B_MAT1>,f_dptr); break;
      default:   REPORT_ERROR(USER_ERROR,"Unknown dense matrix kind is encountered!\n");
    } 
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// saves frovedis dense data in specified file/dir
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_saveFrovedisDenseData
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata, jshort mtype,
   jstring path, jboolean isbinary) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = (exrpc_ptr_t) fdata;
  auto fs_path = to_cstring(env,path);
  auto bin = (bool) isbinary;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to save frovedis dense data.\n";
#endif
  try{
    switch(mtype) {
      case RMJR: exrpc_oneway(fm_node,save_matrix<R_MAT1>,f_dptr,fs_path,bin); break;
      case CMJR: exrpc_oneway(fm_node,save_matrix<C_MAT1>,f_dptr,fs_path,bin); break;
      case BCLC: exrpc_oneway(fm_node,save_matrix<B_MAT1>,f_dptr,fs_path,bin); break;
      default:   REPORT_ERROR(USER_ERROR,"Unknown dense matrix kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// transposes given frovedis dense data
JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_transposeFrovedisDenseData
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata, jshort mtype) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = (exrpc_ptr_t) fdata;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to transpose frovedis dense data.\n";
#endif
  dummy_matrix ret;
  try{
    switch(mtype) {
      case RMJR: ret = exrpc_async(fm_node,(transpose_matrix<R_MAT1,R_LMAT1>),f_dptr).get(); break;
      case CMJR: REPORT_ERROR(USER_ERROR,"Frovedis doesn't support this transpose currently!\n");
      case BCLC: ret = exrpc_async(fm_node,(transpose_matrix<B_MAT1,B_LMAT1>),f_dptr).get(); break;
      default:   REPORT_ERROR(USER_ERROR,"Unknown dense matrix kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jDummyMatrix(env,ret,mtype);
}

// saves std::vector<T> as diag_matrix_local<T> in specified file/dir
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_saveAsFrovedisDiagMatrixLocal
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata,
   jstring path, jboolean isbinary) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = (exrpc_ptr_t) fdata;
  auto fs_path = to_cstring(env,path);
  auto bin = (bool) isbinary;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to save vector as diag_matrix_local data.\n";
#endif
  try{
    exrpc_oneway(fm_node,save_as_diag_matrix_local<DT1>,f_dptr,fs_path,bin);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// loads frovedis dense data from specified file/dir
JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisDenseData
  (JNIEnv *env, jclass thisCls, jobject master_node, jshort mtype,
   jstring path, jboolean isbinary) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto fs_path = to_cstring(env,path);
  auto bin = (bool) isbinary;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to load frovedis dense data.\n";
#endif
  dummy_matrix ret;
  try {
    switch(mtype) {
      case RMJR: ret = exrpc_async(fm_node,load_rmm_matrix<DT1>,fs_path,bin).get(); break;
      case CMJR: ret = exrpc_async(fm_node,load_cmm_matrix<DT1>,fs_path,bin).get(); break;
      case BCLC: ret = exrpc_async(fm_node,load_bcm_matrix<DT1>,fs_path,bin).get(); break;
      default:   REPORT_ERROR(USER_ERROR,"Unknown dense matrix kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jDummyMatrix(env,ret,mtype);
}

// returns an array containing memory heads of the crs_matrix_local
JNIEXPORT jlongArray 
JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getAllSparseMatrixLocalPointers
  (JNIEnv *env, jclass thisCls, jobject master_node, 
   jlong dptr, jshort mtype) {

  auto fm_node = java_node_to_frovedis_node(env,master_node);
  auto f_dptr = (exrpc_ptr_t) dptr;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to get each local data heads.\n";
#endif
  std::vector<exrpc_ptr_t> eps;
  try {
    switch(mtype) {
      case SCRS: eps = exrpc_async(fm_node,(get_all_crs_matrix_local_pointers<DT1,DT5,DT5>),f_dptr).get(); break;
      default: REPORT_ERROR(USER_ERROR,"Unknown local sparse matrix kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jlongArray(env,eps); 
}

// returns each local nnz from specified frovedis worker nodes
JNIEXPORT jintArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getAllSparseMatrixLocalNNZ
  (JNIEnv *env, jclass thisCls, jobject target_node, 
   jlong fdata, jshort mtype) {

  auto fw_node = java_node_to_frovedis_node(env,target_node); // from worker node
  auto f_dptr = (exrpc_ptr_t) fdata;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to worker node ("
            << fw_node.hostname << "," << fw_node.rpcport
            << ") to get each local array.\n";
#endif
  std::vector<size_t> ret_nnz;
  try {
    switch(mtype) {
      case SCRS: ret_nnz = exrpc_async(fw_node,(get_all_nnz<DT1,DT5,DT5>),f_dptr).get(); break;
      default: REPORT_ERROR(USER_ERROR,"Unknown local sparse matrix kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  std::vector<int> ret(ret_nnz.size());
  auto retn_p = ret_nnz.data();
  auto ret_p = ret.data();
  for(size_t i = 0; i < ret_nnz.size(); ++i) {
    ret_p[i] = static_cast<int>(retn_p[i]);
  }
  return to_jintArray(env,ret);
}

// returns each local nnz from specified frovedis worker nodes
JNIEXPORT jintArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getAllSparseMatrixLocalRows
  (JNIEnv *env, jclass thisCls, jobject target_node, 
   jlong fdata, jshort mtype) {

  auto fw_node = java_node_to_frovedis_node(env,target_node); // from worker node
  auto f_dptr = (exrpc_ptr_t) fdata;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to worker node ("
            << fw_node.hostname << "," << fw_node.rpcport
            << ") to get each local array.\n";
#endif
  std::vector<size_t> ret_nrow;
  try {
    switch(mtype) {
      case SCRS: ret_nrow = exrpc_async(fw_node,(get_all_nrow<DT1,DT5,DT5>),f_dptr).get(); break;
      default: REPORT_ERROR(USER_ERROR,"Unknown local sparse matrix kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  std::vector<int> ret(ret_nrow.size());
  auto retn_p = ret_nrow.data();
  auto ret_p = ret.data();
  for(size_t i = 0; i < ret_nrow.size(); ++i) {
    ret_p[i] = static_cast<int>(retn_p[i]);
  }
  return to_jintArray(env,ret);
}


// extract local val, index and offset
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getLocalCRSMatrixComponents 
  (JNIEnv *env, jclass thisCls, jobject target_node, jlong fdata,
   jdoubleArray data, jintArray indx, jintArray offset, jint nrow, jint nnz) {

  auto fw_node = java_node_to_frovedis_node(env,target_node); // from worker node
  auto f_dptr = (exrpc_ptr_t) fdata;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to worker node ("
            << fw_node.hostname << "," << fw_node.rpcport
            << ") to get each local array.\n";
#endif
  
  crs_matrix_local<DT1,DT5,DT5> ret_crs;
  try {
    ret_crs = exrpc_async(fw_node,(get_crs_matrix_local<DT1,DT5,DT5>),f_dptr).get();
  } 
  catch (std::exception& e) { set_status(true,e.what()); }
  checkAssumption(ret_crs.val.size() == nnz);
  checkAssumption(ret_crs.idx.size() == nnz);
  checkAssumption(ret_crs.off.size() == (nrow + 1));
  std::vector<int> ret_idx(nnz,0);
  std::vector<int> ret_off(nrow+1,0);
  auto ret_idxp = ret_idx.data();
  auto ret_offp = ret_off.data();
  auto ret_crs_idxp = ret_crs.idx.data();
  auto ret_crs_offp = ret_crs.off.data();
  for(size_t i = 0; i < nnz; ++i) {
    ret_idxp[i] = static_cast<int>(ret_crs_idxp[i]);
  }
  for(size_t i = 0; i < nrow + 1; ++i) {
    ret_offp[i] = static_cast<int>(ret_crs_offp[i]);
  }
  env->SetIntArrayRegion(indx, 0, nnz, ret_idx.data());
  env->SetIntArrayRegion(offset, 0, (nrow + 1), ret_off.data());
  env->SetDoubleArrayRegion(data, 0, nnz, ret_crs.val.data());
}

}
