#include "spark_client_headers.hpp"
#include "short_hand_sparse_type.hpp"
#include "exrpc_graphx.hpp"

using namespace frovedis;

extern "C" {

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_showGraph
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = static_cast<exrpc_ptr_t> (fdata);
  try {
    exrpc_oneway(fm_node, show_graph<graph<DT1>>, f_dptr);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_releaseGraph
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = static_cast<exrpc_ptr_t> (fdata);
  try {
    exrpc_oneway(fm_node, release_graph<graph<DT1>>, f_dptr);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadGraphFromTextFile
  (JNIEnv *env, jclass thisCls, jobject master_node, jstring fname) {
  auto c_fname = to_cstring(env, fname);
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  dummy_graph ret; 
  try {
    ret = exrpc_async(fm_node, (load_graph<graph<DT1>,DT1>), c_fname).get();
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
    exrpc_oneway(fm_node, save_graph<graph<DT1>>, f_dptr, c_fname);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

JNIEXPORT jlong JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_copyGraph
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = static_cast<exrpc_ptr_t> (fdata);
  exrpc_ptr_t gptr = -1;
  try {
    gptr = exrpc_async(fm_node, copy_graph<graph<DT1>>, f_dptr).get();
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
    gptr = exrpc_async(fm_node, (set_graph_data<S_MAT1,DT1>), f_dptr).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return (long) gptr;
}

JNIEXPORT jobjectArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getGraphEdgeData
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = static_cast<exrpc_ptr_t> (fdata);
  crs_matrix_local<double> adj_mat;
  try {
    adj_mat = exrpc_async(fm_node, (get_graph_edge_data<graph<DT1>,DT1>), f_dptr).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jDummyEdgeArray(env, adj_mat);
}

JNIEXPORT jdoubleArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getGraphVertexData
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = static_cast<exrpc_ptr_t> (fdata);
  std::vector<double> vertices;
  try {
    vertices = exrpc_async(fm_node, get_graph_vertex_data<graph<DT1>>, f_dptr).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jdoubleArray(env, vertices);
}

JNIEXPORT jobject JNICALL 
Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisPageRank(JNIEnv *env, jclass thisCls, 
  jobject master_node, jlong fdata, jdouble epsilon, jdouble dfactor, 
  jint max_iter) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = static_cast<exrpc_ptr_t> (fdata);
  int vb = 0; // no log (default)
  dummy_graph ret; 
  try {
    ret = exrpc_async(fm_node, frovedis_normalized_pagerank<graph<DT1>>, f_dptr, 
                      epsilon, dfactor, max_iter, vb).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jDummyGraph(env, ret);
}

JNIEXPORT jobject JNICALL
Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisSSSP(JNIEnv *env, jclass thisCls,
  jobject master_node, jlong fdata, jlong source_vertex) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = static_cast<exrpc_ptr_t> (fdata);
  int vb = 0; // no log (default)
  sssp_result<DT1, DT5> res;
  try {
    res = exrpc_async(fm_node, (frovedis_sssp<graph<DT1>,DT1,DT5>), f_dptr,
                      source_vertex, vb).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jSSSP_Result(env, res, source_vertex);
}

JNIEXPORT jobject JNICALL
Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisBFS(JNIEnv *env, jclass thisCls,
  jobject master_node, jlong fdata,
  jlong source_vertex, jint opt_level, jdouble hyb_threshold, jlong depth_limit) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = static_cast<exrpc_ptr_t> (fdata);
  int vb = 0; // no log (default)
  bfs_result<DT5> res;
  try {
    res = exrpc_async(fm_node, (frovedis_bfs<graph<DT1>,DT5>), f_dptr,
                      source_vertex, opt_level, hyb_threshold, depth_limit, 
                      vb).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jBFS_Result(env, res, source_vertex);
}

JNIEXPORT jlongArray JNICALL
Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisCC(JNIEnv *env, jclass thisCls,
  jobject master_node, jlong fdata,
  jlongArray nodes_in_which_cc,
  jlongArray nodes_dist, jlong num_vertices, 
  jint opt_level, jdouble hyb_threshold) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = static_cast<exrpc_ptr_t> (fdata);
  int vb = 0; // no log (default)
  cc_result<DT5> result;
  try {
    result = exrpc_async(fm_node, (frovedis_cc<graph<DT1>,DT5>), f_dptr, 
                      opt_level, hyb_threshold, vb).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  checkAssumption(result.distances.size() == num_vertices);
  checkAssumption(result.num_nodes_in_which_cc.size() == num_vertices);
  auto resdistp = result.distances.data();
  auto reswhichp = result.num_nodes_in_which_cc.data();
  auto lmax = std::numeric_limits<long>::max();
  auto uimax = std::numeric_limits<size_t>::max();
  std::vector<long> tmpdist(num_vertices), tmpwhich(num_vertices);
  auto tmpdistp = tmpdist.data();
  auto tmpwhichp = tmpwhich.data();
  for(size_t i = 0; i < num_vertices; ++i){
    tmpdistp[i] = (resdistp[i] == uimax) ? lmax : (long) resdistp[i];
    tmpwhichp[i] = (reswhichp[i] == uimax) ? lmax : (long) reswhichp[i];
  }
  env->SetLongArrayRegion(nodes_dist, 0, num_vertices, tmpdistp);
  env->SetLongArrayRegion(nodes_in_which_cc, 0, num_vertices, tmpwhichp);
  std::vector<long> ret(2 * result.num_cc); auto retp = ret.data();
  for(size_t i = 0; i < result.num_cc; ++i) {
    retp[2 * i] = result.root_in_each_cc[i];
    retp[2 * i + 1] = result.num_nodes_in_each_cc[i];
  }
  return to_jlongArray(env, ret);
}

}
