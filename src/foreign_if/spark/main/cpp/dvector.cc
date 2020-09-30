#include "exrpc_data_storage.hpp"
#include "short_hand_dtype.hpp"
#include "spark_client_headers.hpp"

using namespace frovedis;

extern "C" {

// Typed Dvectors
JNIEXPORT jlong JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisWorkerIntVector
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong size,
   jintArray data) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto data_vec = to_int_vector(env,data,size);
  exrpc_ptr_t proxy = 0;
  try {
    proxy = exrpc_async(fm_node, (load_local_data<std::vector<int>>), data_vec).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return (jlong) proxy;
}

JNIEXPORT jlong JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisWorkerLongVector
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong size,
   jlongArray data) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto data_vec = to_long_vector(env,data,size);
  exrpc_ptr_t proxy = 0;
  try{
    proxy = exrpc_async(fm_node, (load_local_data<std::vector<long>>), data_vec).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return (jlong) proxy;
}

JNIEXPORT jlong JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisWorkerFloatVector
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong size,
   jfloatArray data) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto data_vec = to_float_vector(env,data,size);
  exrpc_ptr_t proxy = 0;
  try {
    proxy = exrpc_async(fm_node, (load_local_data<std::vector<float>>), data_vec).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return (jlong) proxy;
}

JNIEXPORT jlong JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisWorkerDoubleVector
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong size,
   jdoubleArray data) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto data_vec = to_double_vector(env,data,size);
  exrpc_ptr_t proxy = 0;
  try {
    proxy = exrpc_async(fm_node, (load_local_data<std::vector<double>>), data_vec).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return (jlong) proxy;
}

JNIEXPORT jlong JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisWorkerBoolVector
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong size,
   jbooleanArray data) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto data_vec = to_bool_vector(env,data,size); // actually std::vector<int>
  exrpc_ptr_t proxy = 0;
  try {
    proxy = exrpc_async(fm_node, (load_local_data<std::vector<int>>), data_vec).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return (jlong) proxy;
}

JNIEXPORT jlong JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisWorkerStringVector
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong size,
   jobjectArray data) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto data_vec = to_string_vector(env,data,size);
  exrpc_ptr_t proxy = 0;
  try {
    proxy = exrpc_async(fm_node, (load_local_data<std::vector<std::string>>), data_vec).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return (jlong) proxy;
}

JNIEXPORT jlong JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_createFrovedisDvector
  (JNIEnv *env, jclass thisCls, jobject master_node,
   jlongArray proxies, jlongArray sizes, jlong size, jshort dtype) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto p_vec = to_exrpc_vector(env, proxies, size);
  auto s_vec = to_sizet_vector(env, sizes, size); // no need
  exrpc_ptr_t dvecp = 0;
  //std::cout << "[createFrovedisDvector] dtype: " << dtype << std::endl;
  try{
    switch(dtype) {
       case INT:    dvecp = exrpc_async(fm_node,create_and_set_dvector<int>,p_vec).get(); break;
       case LONG:   dvecp = exrpc_async(fm_node,create_and_set_dvector<long>,p_vec).get(); break;
       case FLOAT:  dvecp = exrpc_async(fm_node,create_and_set_dvector<float>,p_vec).get(); break;
       case DOUBLE: dvecp = exrpc_async(fm_node,create_and_set_dvector<double>,p_vec).get(); break;
       case STRING: dvecp = exrpc_async(fm_node,create_and_set_dvector<std::string>,p_vec).get(); break;
       case BOOL:   dvecp = exrpc_async(fm_node,create_and_set_dvector<int>,p_vec).get(); break;
       default:     REPORT_ERROR(USER_ERROR, "Unsupported datatype is encountered in dvector creation!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return (jlong) dvecp;
}

// needed for FrovedisLabeledPoint

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_releaseFrovedisDvector
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong dvec_ptr) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = (exrpc_ptr_t) dvec_ptr;
  try {
    exrpc_oneway(fm_node, release_dvector<DT1>, f_dptr);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

JNIEXPORT jdoubleArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getUniqueDvectorElements
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong dvec_ptr) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = (exrpc_ptr_t) dvec_ptr;
  std::vector<double> uniq_elem;
  try {
    uniq_elem = exrpc_async(fm_node,get_distinct_elements<double>,f_dptr).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jdoubleArray(env, uniq_elem);
}

JNIEXPORT jlong JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getZeroBasedEncodedDvector
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong dvec_ptr) { 
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = (exrpc_ptr_t) dvec_ptr;
  exrpc_ptr_t res_ptr = -1;
  try {
    res_ptr = exrpc_async(fm_node, get_encoded_dvector_zero_based<double>, f_dptr).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return (jlong) res_ptr;
}

JNIEXPORT jlong JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getEncodedDvectorAs
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong dvec_ptr, 
   jdoubleArray src, jdoubleArray enc, jint uniqCnt) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = (exrpc_ptr_t) dvec_ptr;
  auto src_vec = to_double_vector(env, src, uniqCnt);
  auto enc_vec = to_double_vector(env, enc, uniqCnt);
  exrpc_ptr_t res_ptr = -1;
  try {
    res_ptr = exrpc_async(fm_node, get_encoded_dvector<double>, f_dptr, 
                          src_vec, enc_vec).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return (jlong) res_ptr;
}

}
