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
  jobject to_jDummyGraph(JNIEnv *, dummy_graph&);
  jobject to_jBFS_Result(JNIEnv *, bfs_result<size_t>&, long);
  jobject to_jSSSP_Result(JNIEnv *, sssp_result<double,size_t>&, long);
  jobject to_jDummyMatrix(JNIEnv *, dummy_matrix&, short );
  jobject to_jDummyGetrfResult(JNIEnv *, lu_fact_result&, short);
  jobject to_jDummyGesvdResult(JNIEnv *, svd_result&, short, bool, bool);
  jobject to_jDummyEvdResult(JNIEnv *, eigen_result&);
  jobject to_jDummyPCAResult(JNIEnv *, pca_result&, short);
  jobject to_jDummyTSNEResult(JNIEnv *, tsne_result&); //tsne
  jobject to_jDummyKNNResult(JNIEnv *env, knn_result&); //knn
  frovedis_mem_pair java_mempair_to_frovedis_mempair(JNIEnv *, jobject&);
  jobject frovedis_mempair_to_java_mempair(JNIEnv *, frovedis_mem_pair& );
  jobject make_jIntDoublePair (JNIEnv *, int, double);
  jobject to_jDummyLDAResult(JNIEnv *, dummy_lda_result&);
  jobject to_jDummyLDAModel(JNIEnv *, dummy_lda_model&);

  crs_matrix_local<double> 
  get_frovedis_double_crs_matrix_local(JNIEnv *, jlong, jlong ,
                                   jintArray&, jintArray&,
                                   jdoubleArray&); 

  // ml etc.
  jlongArray to_jlongArray(JNIEnv *, std::vector<exrpc_ptr_t>&);
  jlongArray to_jlongArray2(JNIEnv *, std::vector<long>&);
  jlongArray to_jlongArray3(JNIEnv *, std::vector<size_t>&);
  jdoubleArray to_jdoubleArray(JNIEnv *, std::vector<double>&);
  jfloatArray to_jfloatArray(JNIEnv *, std::vector<float>&);
  jintArray to_jintArray(JNIEnv *, std::vector<int>&);
  jobjectArray to_jStringArray(JNIEnv *, std::vector<std::string>&);
  jobjectArray to_jNodeArray(JNIEnv *, std::vector<frovedis::exrpc_node>&);
  jobjectArray to_jIntDoublePairArray(JNIEnv *, std::vector<std::pair<size_t,double>>&);
  jobjectArray to_jDummyEdgeArray(JNIEnv *, crs_matrix_local<double>&);
  jobjectArray to_jMemPairArray(JNIEnv *env, std::vector<frovedis_mem_pair>& pd);

  // dataframe etc.
  std::vector<float> to_float_vector(JNIEnv *, jfloatArray&, size_t);
  std::vector<double> to_double_vector(JNIEnv *, jdoubleArray&, size_t);
  std::vector<int> to_int_vector(JNIEnv *, jintArray&, size_t);
  std::vector<char> to_char_vector(JNIEnv *, jcharArray&, size_t);
  std::vector<int> to_bool_vector(JNIEnv *, jbooleanArray&, size_t);
  std::vector<short> to_short_vector(JNIEnv *, jshortArray&, size_t);
  std::vector<long> to_long_vector(JNIEnv *, jlongArray&, size_t );
  std::vector<size_t> to_sizet_vector(JNIEnv *, jlongArray&, size_t);
  std::vector<std::string> to_string_vector(JNIEnv *, jobjectArray&, size_t);
  std::vector<std::string> charArray_to_string_vector(JNIEnv *, jobjectArray&, size_t);
  std::vector<std::string> flat_charArray_to_string_vector(JNIEnv *, jcharArray&, 
                                                           jintArray&, size_t, size_t);
  std::vector<int> flat_charArray_to_int_vector(JNIEnv *, jcharArray&, size_t);
  std::vector<exrpc_ptr_t> to_exrpc_vector(JNIEnv *, jlongArray&, size_t);
  jobject to_spark_dummy_df (JNIEnv *env, dummy_dftable& obj);
}

#endif
