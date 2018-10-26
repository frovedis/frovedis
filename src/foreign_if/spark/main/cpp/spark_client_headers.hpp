#ifndef _SPARK_CLIENT_HEADERS_
#define _SPARK_CLIENT_HEADERS_

#include "jre_config.hpp"
#include "JNISupport.hpp"
#include "exrpc_util_headers.hpp"
#include "frovedis/matrix/crs_matrix.hpp"

using namespace frovedis;

// --- global exception tracking variables ---
extern bool status;
extern std::string info;

extern "C" { 
  void set_status(bool , const std::string& );
  void reset_status();
  std::string get_info();

  std::string to_cstring(JNIEnv *, jstring );
  exrpc_node java_node_to_frovedis_node(JNIEnv *, jobject&);
  jobject frovedis_node_to_java_node (JNIEnv *, exrpc_node&);

  jobject frovedis_dummyGLM_to_java_dummyGLM(JNIEnv *, dummy_glm&);
  jobject to_jDummyMatrix(JNIEnv *, dummy_matrix&, short );
  jobject to_jDummyGetrfResult(JNIEnv *, getrf_result&, short);
  jobject to_jDummyGesvdResult(JNIEnv *, gesvd_result&, short, bool, bool);
  jobject to_jDummyPCAResult(JNIEnv *, pca_result&, short);
  frovedis_mem_pair java_mempair_to_frovedis_mempair(JNIEnv *, jobject&);
  jobject frovedis_mempair_to_java_mempair(JNIEnv *, frovedis_mem_pair& );
  jobject make_jIntDoublePair (JNIEnv *, int, double);

  crs_matrix_local<double> 
  get_frovedis_double_crs_matrix_local(JNIEnv *, jlong, jlong ,
                                   jintArray&, jintArray&,
                                   jdoubleArray&); 

  // ml etc.
  jlongArray to_jlongArray(JNIEnv *, std::vector<exrpc_ptr_t>&);
  jdoubleArray to_jdoubleArray(JNIEnv *, std::vector<double>&);
  jintArray to_jintArray(JNIEnv *, std::vector<int>&);
  jobjectArray to_jStringArray(JNIEnv *, std::vector<std::string>&);
  jobjectArray to_jNodeArray(JNIEnv *, std::vector<frovedis::exrpc_node>&);
  jobjectArray to_jIntDoublePairArray(JNIEnv *, std::vector<std::pair<size_t,double>>&);

  // dataframe etc.
  std::vector<float> to_float_vector(JNIEnv *, jfloatArray&, size_t);
  std::vector<double> to_double_vector(JNIEnv *, jdoubleArray&, size_t );
  std::vector<int> to_int_vector(JNIEnv *, jintArray&, size_t );
  std::vector<int> to_bool_vector(JNIEnv *, jbooleanArray&, size_t);
  std::vector<short> to_short_vector(JNIEnv *, jshortArray&, size_t);
  std::vector<long> to_long_vector(JNIEnv *, jlongArray&, size_t );
  std::vector<size_t> to_sizet_vector(JNIEnv *, jlongArray&, size_t);
  std::vector<std::string> to_string_vector(JNIEnv *, jobjectArray&, size_t);
  std::vector<exrpc_ptr_t> to_exrpc_vector(JNIEnv *, jlongArray&, size_t);
}

#endif
