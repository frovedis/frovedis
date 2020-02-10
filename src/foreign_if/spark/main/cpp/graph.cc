#include "exrpc_data_storage.hpp"
#include "exrpc_graphx.hpp"
#include "spark_client_headers.hpp"
#include "short_hand_sparse_type.hpp"

using namespace frovedis;

extern "C" {

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_showGraph
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = static_cast<exrpc_ptr_t> (fdata);
  try {
    exrpc_oneway(fm_node, show_graph<graph>, f_dptr);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_releaseGraph
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = static_cast<exrpc_ptr_t> (fdata);
  try {
    exrpc_oneway(fm_node, release_graph<graph>, f_dptr);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadGraphFromTextFile
  (JNIEnv *env, jclass thisCls, jobject master_node, jstring fname) {
  auto c_fname = to_cstring(env, fname);
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  dummy_graph ret; 
  try {
    ret = exrpc_async(fm_node, load_graph<graph>, c_fname).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jDummyGraph(env, ret);
}

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_saveGraph
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata, 
   jstring fname) {
  auto c_fname = to_cstring(env, fname);
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = static_cast<exrpc_ptr_t> (fdata);
  try {
    exrpc_oneway(fm_node, save_graph<graph>, f_dptr, c_fname);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

JNIEXPORT jlong JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_copyGraph
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = static_cast<exrpc_ptr_t> (fdata);
  exrpc_ptr_t gptr = -1;
  try {
    gptr = exrpc_async(fm_node, copy_graph<graph>, f_dptr).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return (long) gptr;
}

JNIEXPORT jlong JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_setGraphData
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = static_cast<exrpc_ptr_t> (fdata); // adj matrix pointer
  exrpc_ptr_t gptr = -1;
  try {
    gptr = exrpc_async(fm_node, set_graph_data<S_MAT1>, f_dptr).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return (long) gptr;
}

JNIEXPORT jobjectArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getGraphData
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = static_cast<exrpc_ptr_t> (fdata);
  crs_matrix_local<double> adj_mat;
  try {
    adj_mat = exrpc_async(fm_node, get_graph_data<graph>, f_dptr).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jDummyEdgeArray(env, adj_mat);
}

JNIEXPORT jdoubleArray JNICALL 
Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisPageRank(JNIEnv *env, jclass thisCls, 
  jobject master_node, jlong fdata, jdouble epsilon, jdouble dfactor, 
  jint max_iter) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = static_cast<exrpc_ptr_t> (fdata);
  int vb = 0; // no log (default)
  std::vector<double> result;
  try {
    result = exrpc_async(fm_node, frovedis_pagerank<graph>, f_dptr, 
                         epsilon, dfactor, max_iter, vb).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jdoubleArray(env, result);
}

JNIEXPORT void JNICALL
Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisSSSP(JNIEnv *env, jclass thisCls,
  jobject master_node, jlong fdata,
  jintArray dist_arr, jlongArray pred_arr, jlong sz,
  jlong source_vertex) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = static_cast<exrpc_ptr_t> (fdata);
  int vb = 0; // no log (default)
  sssp_result res;
  try {
    res = exrpc_async(fm_node, frovedis_sssp<graph>, f_dptr,
                      source_vertex, vb).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  checkAssumption(sz == res.nodes_dist.size()); // check in case any size issue
  env->SetIntArrayRegion(dist_arr, 0, sz, res.nodes_dist.data());
  env->SetLongArrayRegion(pred_arr, 0, sz, res.nodes_pred.data());
}

JNIEXPORT jlongArray JNICALL
Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisBFS(JNIEnv *env, jclass thisCls,
  jobject master_node, jlong fdata,
  jlongArray nodes_in_which_cc,
  jintArray nodes_dist, jlong sz) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = static_cast<exrpc_ptr_t> (fdata);
  int vb = 0; // no log (default)
  bfs_result res;
  try {
    res = exrpc_async(fm_node, frovedis_bfs<graph>, f_dptr, vb).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  checkAssumption(sz == res.nodes_dist.size()); // check in case any size issue
  env->SetIntArrayRegion(nodes_dist, 0, sz, res.nodes_dist.data());
  env->SetLongArrayRegion(nodes_in_which_cc, 0, sz, res.nodes_in_which_cc.data());
  return to_jlongArray(env, res.num_nodes_in_each_cc);
}

}
