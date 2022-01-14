#include "spark_client_headers.hpp"

using namespace frovedis;

extern "C" {

// returns the node information of Frovedis master node
JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getMasterInfo
  (JNIEnv *env, jclass thisCls, jstring cmd) {
  exrpc_node n;
  try {
    n = invoke_frovedis_server(to_cstring(env,cmd)); 
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return frovedis_node_to_java_node(env, n);
}

// returns the active worker node count at Frovedis side
JNIEXPORT jint JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getWorkerSize
  (JNIEnv *env, jclass thisCls, jobject master_node) {
  auto fm_node = java_node_to_frovedis_node(env,master_node);
  int ret = 0;
  try {
    ret = exrpc_async0(fm_node, get_nodesize).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return ret;
}

// connects with Frovedis worker nodes for processing single parallel request 
// and returns their node informations
JNIEXPORT jobjectArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getWorkerInfo
  (JNIEnv *env, jclass thisCls, jobject master_node) {
  auto fm_node = java_node_to_frovedis_node(env,master_node);
  auto info = prepare_parallel_exrpc(fm_node);
  auto nodes = get_parallel_exrpc_nodes(fm_node, info);
  wait_parallel_exrpc(fm_node, info);
  return to_jNodeArray(env,nodes);
}

// connects with Frovedis worker nodes for processing N parallel requests 
// and returns their node informations
JNIEXPORT jobjectArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getWorkerInfoMulti
  (JNIEnv *env, jclass thisCls, jobject master_node, 
   jlongArray block_sizes, jint nproc) {
  auto fm_node = java_node_to_frovedis_node(env,master_node);
  auto info = prepare_parallel_exrpc(fm_node);
  auto nodes = get_parallel_exrpc_nodes(fm_node, info);
  auto blocksz = to_sizet_vector(env, block_sizes, nproc);
  require(nodes.size() == nproc, 
  "size of block vector doesn't match with the no. of mpi processes at FrovedisServer!\n");
  wait_parallel_exrpc_multi(fm_node, info, blocksz);
  return to_jNodeArray(env,nodes);
}

// sends request for clean-up (models, trackers etc.) to Frovedis master node
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_cleanUPFrovedisServer
  (JNIEnv *env, jclass thisCls, jobject master_node) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  try {
    exrpc_oneway0(fm_node,cleanup_frovedis_server);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// sends request for finalization to Frovedis master node
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_finalizeFrovedisServer
  (JNIEnv *env, jclass thisCls, jobject master_node) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  try {
    finalize_frovedis_server(fm_node);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

}
