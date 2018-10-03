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

JNIEXPORT jlong JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisWorkerBooleanVector
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

}
