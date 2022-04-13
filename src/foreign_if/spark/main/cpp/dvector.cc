#include "spark_client_headers.hpp"
#include "short_hand_dtype.hpp"

using namespace frovedis;

extern "C" {

// Typed Dvectors
JNIEXPORT jlongArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_allocateLocalVector
  (JNIEnv *env, jclass thisCls, jobject master_node, 
   jlongArray block_sizes, jint nproc, jshort dtype) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto blocksz = to_sizet_vector(env, block_sizes, nproc);
  std::vector<exrpc_ptr_t> proxies;
  try {
    switch(dtype) {
      case BOOL:
      case INT:    proxies = exrpc_async(fm_node, (allocate_local_vector<std::vector<int>>), 
                                         blocksz).get(); break;
      case LONG:   proxies = exrpc_async(fm_node, (allocate_local_vector<std::vector<long>>), 
                                         blocksz).get(); break;
      case FLOAT:  proxies = exrpc_async(fm_node, (allocate_local_vector<std::vector<float>>), 
                                         blocksz).get(); break;
      case DOUBLE: proxies = exrpc_async(fm_node, (allocate_local_vector<std::vector<double>>), 
                                         blocksz).get(); break;
      case STRING: proxies = exrpc_async(fm_node, (allocate_local_vector<std::vector<std::string>>), 
                                         blocksz).get(); break;
      default:   REPORT_ERROR(USER_ERROR, "Unsupported datatype is encountered in dvector creation!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jlongArray(env, proxies);
}

// for WordsNodeLocal (char, int)
JNIEXPORT jobjectArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_allocateLocalVectorPair
  (JNIEnv *env, jclass thisCls, jobject master_node,
   jlongArray block_sizes, jint nproc) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto blocksz = to_sizet_vector(env, block_sizes, nproc);
  std::vector<frovedis_mem_pair> proxies;
  try {
    proxies = exrpc_async(fm_node, (allocate_local_vector_pair<std::vector<char>, std::vector<int>>),
                          blocksz).get(); 
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jMemPairArray(env, proxies);
}

// for dataframe column vectors
JNIEXPORT jlongArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_allocateLocalVectors
  (JNIEnv *env, jclass thisCls, jobject master_node,
   jlongArray block_sizes, jint nproc,
   jshortArray dtypes, jlong ncol) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto blocksz = to_sizet_vector(env, block_sizes, nproc);
  auto dtps = to_short_vector(env, dtypes, ncol);
  std::vector<exrpc_ptr_t> proxies; // (ncol + no-of-words) x nproc
  try {
    proxies = exrpc_async(fm_node, allocate_local_vectors, blocksz, dtps).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jlongArray(env, proxies);
}

// --- mainly for debugging purpose ---
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadVectorData
  (JNIEnv *env, jclass thisCls, jobject target_node,
   jlong address, jlong size_, jshort dtype) {

  frovedis_mem_pair mempair;
  size_t size = size_;
  auto node = java_node_to_frovedis_node(env, target_node);
  auto sendbufp = reinterpret_cast<char*>(address);

  try {
    switch(dtype) {
      case BYTE:   mempair = exrpc_async(node, allocate_vector<char>, size).get();
                   exrpc_rawsend(node, sendbufp, mempair.second(), sizeof(char) * size);
                   exrpc_oneway(node, show_vector<char>, mempair.second(), size);
                   break;
      case BOOL:
      case INT:    mempair = exrpc_async(node, allocate_vector<int>, size).get();
                   exrpc_rawsend(node, sendbufp, mempair.second(), sizeof(int) * size);
                   exrpc_oneway(node, show_vector<int>, mempair.second(), size);
                   break;
      case LONG:   mempair = exrpc_async(node, allocate_vector<long>, size).get();
                   exrpc_rawsend(node, sendbufp, mempair.second(), sizeof(long) * size);
                   exrpc_oneway(node, show_vector<long>, mempair.second(), size);
                   break;
      case FLOAT:  mempair = exrpc_async(node, allocate_vector<float>, size).get();
                   exrpc_rawsend(node, sendbufp, mempair.second(), sizeof(float) * size);
                   exrpc_oneway(node, show_vector<float>, mempair.second(), size);
                   break;
      case DOUBLE: mempair = exrpc_async(node, allocate_vector<double>, size).get();
                   exrpc_rawsend(node, sendbufp, mempair.second(), sizeof(double) * size);
                   exrpc_oneway(node, show_vector<double>, mempair.second(), size);
                   break;
      default:     REPORT_ERROR(USER_ERROR, "Unsupported type encountered!");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

void send_raw_local_vector(char* sendbufp, size_t size, short dtype,
                           exrpc_ptr_t vp, size_t idx,
                           exrpc_node& tnode) {
  exrpc_ptr_t recvbufp = 0;
  try {
    switch (dtype) {
      case BYTE: {
        recvbufp = exrpc_async(tnode, allocate_vector_partition<char>, 
                               vp, idx, size).get();
        exrpc_rawsend(tnode, sendbufp, recvbufp, sizeof(char) * size);
        break;
      }
      case BOOL:
      case INT: {
        recvbufp = exrpc_async(tnode, allocate_vector_partition<int>, 
                               vp, idx, size).get();
        exrpc_rawsend(tnode, sendbufp, recvbufp, sizeof(int) * size);
        break;
      }
      case LONG: {
        recvbufp = exrpc_async(tnode, allocate_vector_partition<long>, 
                               vp, idx, size).get();
        exrpc_rawsend(tnode, sendbufp, recvbufp, sizeof(long) * size);
        break;
      }
      case FLOAT: {
        recvbufp = exrpc_async(tnode, allocate_vector_partition<float>, 
                               vp, idx, size).get();
        exrpc_rawsend(tnode, sendbufp, recvbufp, sizeof(float) * size);
        break;
      }
      case DOUBLE: {
        recvbufp = exrpc_async(tnode, allocate_vector_partition<double>, 
                               vp, idx, size).get();
        exrpc_rawsend(tnode, sendbufp, recvbufp, sizeof(double) * size);
        break;
      }
      default:  REPORT_ERROR(USER_ERROR, 
                "send_raw_local_vector: unsupported type encountered!");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

void send_local_vector(long datap, size_t size, short dtype,
                       exrpc_ptr_t vp, size_t idx,
                       exrpc_node& tnode) {
  try {
    switch (dtype) {
      case BYTE: {
        std::vector<char> vec(size);
        auto vecp = vec.data();
        auto sendbufp = reinterpret_cast<char*>(datap);
        for (size_t i = 0; i < size; ++i) vecp[i] = sendbufp[i];
        exrpc_oneway(tnode, (load_local_vector<std::vector<char>>), vp, idx, vec); // vp[idx] = vec
        break;
      }
      case BOOL:
      case INT: {
        std::vector<int> vec(size);
        auto vecp = vec.data();
        auto sendbufp = reinterpret_cast<int*>(datap);
        for (size_t i = 0; i < size; ++i) vecp[i] = sendbufp[i];
        exrpc_oneway(tnode, (load_local_vector<std::vector<int>>), vp, idx, vec); // vp[idx] = vec
        break;
      }
      case LONG: {
        std::vector<long> vec(size);
        auto vecp = vec.data();
        auto sendbufp = reinterpret_cast<long*>(datap);
        for (size_t i = 0; i < size; ++i) vecp[i] = sendbufp[i];
        exrpc_oneway(tnode, (load_local_vector<std::vector<long>>), vp, idx, vec); // vp[idx] = vec
        break;
      }
      case FLOAT: {
        std::vector<float> vec(size);
        auto vecp = vec.data();
        auto sendbufp = reinterpret_cast<float*>(datap);
        for (size_t i = 0; i < size; ++i) vecp[i] = sendbufp[i];
        exrpc_oneway(tnode, (load_local_vector<std::vector<float>>), vp, idx, vec); // vp[idx] = vec
        break;
      }
      case DOUBLE: {
        std::vector<double> vec(size);
        auto vecp = vec.data();
        auto sendbufp = reinterpret_cast<double*>(datap);
        for (size_t i = 0; i < size; ++i) vecp[i] = sendbufp[i];
        exrpc_oneway(tnode, (load_local_vector<std::vector<double>>), vp, idx, vec); // vp[idx] = vec
        break;
      }
      default:  REPORT_ERROR(USER_ERROR,
                "send_local_vector: unsupported type encountered!");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisWorkerTypedVector
  (JNIEnv *env, jclass thisCls, jobject worker_node, jlong vptr,
   jlong index, jlong datap, jlong size, jshort dtype, jboolean rawsend) {

  auto fw_node = java_node_to_frovedis_node(env, worker_node);
  auto vp = (exrpc_ptr_t) vptr;
  if (rawsend) {
    auto sendbufp = reinterpret_cast<char*>(datap);
    send_raw_local_vector(sendbufp, size, dtype, vp, index, fw_node); 
  }
  else {
    send_local_vector(datap, size, dtype, vp, index, fw_node);
  }
}

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisWorkerIntVector
  (JNIEnv *env, jclass thisCls, jobject worker_node, jlong vptr, 
   jlong index, jintArray data, jlong size) {

  auto fw_node = java_node_to_frovedis_node(env, worker_node);
  auto p_vec = to_int_vector(env, data, size);
  auto vp = (exrpc_ptr_t) vptr;
  size_t idx = index;
  try {
    exrpc_oneway(fw_node, (load_local_vector<std::vector<int>>), vp, idx, p_vec); // vp[idx] = p_vec
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisWorkerLongVector
  (JNIEnv *env, jclass thisCls, jobject worker_node, jlong vptr, 
   jlong index, jlongArray data, jlong size) {

  auto fw_node = java_node_to_frovedis_node(env, worker_node);
  auto p_vec = to_long_vector(env, data, size);
  auto vp = (exrpc_ptr_t) vptr;
  size_t idx = index;
  try{
    exrpc_oneway(fw_node, (load_local_vector<std::vector<long>>), vp, idx, p_vec); // vp[idx] = p_vec
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisWorkerFloatVector
  (JNIEnv *env, jclass thisCls, jobject worker_node, jlong vptr, 
   jlong index, jfloatArray data, jlong size) {

  auto fw_node = java_node_to_frovedis_node(env, worker_node);
  auto p_vec = to_float_vector(env, data, size);
  auto vp = (exrpc_ptr_t) vptr;
  size_t idx = index;
  try{
    exrpc_oneway(fw_node, (load_local_vector<std::vector<float>>), vp, idx, p_vec); // vp[idx] = p_vec
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisWorkerDoubleVector
  (JNIEnv *env, jclass thisCls, jobject worker_node, jlong vptr, 
   jlong index, jdoubleArray data, jlong size) {

  auto fw_node = java_node_to_frovedis_node(env, worker_node);
  auto p_vec = to_double_vector(env, data, size);
  auto vp = (exrpc_ptr_t) vptr;
  size_t idx = index;
  try{
    exrpc_oneway(fw_node, (load_local_vector<std::vector<double>>), vp, idx, p_vec); // vp[idx] = p_vec
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisWorkerBoolVector
  (JNIEnv *env, jclass thisCls, jobject worker_node, jlong vptr, 
   jlong index, jbooleanArray data, jlong size) {

  auto fw_node = java_node_to_frovedis_node(env, worker_node);
  auto p_vec = to_bool_vector(env, data, size);
  auto vp = (exrpc_ptr_t) vptr;
  size_t idx = index;
  try{
    exrpc_oneway(fw_node, (load_local_vector<std::vector<int>>), vp, idx, p_vec); // vp[idx] = p_vec
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisWorkerStringVector
  (JNIEnv *env, jclass thisCls, jobject worker_node, jlong vptr, 
   jlong index, jobjectArray data, jlong size) {

  auto fw_node = java_node_to_frovedis_node(env, worker_node);
  auto p_vec = to_string_vector(env, data, size);
  auto vp = (exrpc_ptr_t) vptr;
  size_t idx = index;
  try{
    exrpc_oneway(fw_node, (load_local_vector<std::vector<std::string>>), vp, idx, p_vec); // vp[idx] = p_vec
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisWorkerCharArrayVector
  (JNIEnv *env, jclass thisCls, jobject worker_node, jlong vptr,
   jlong index, jobjectArray data, jlong size) {

  auto fw_node = java_node_to_frovedis_node(env, worker_node);
  auto p_vec = charArray_to_string_vector(env, data, size);
  auto vp = (exrpc_ptr_t) vptr;
  size_t idx = index;
  try{
    exrpc_oneway(fw_node, (load_local_vector<std::vector<std::string>>), vp, idx, p_vec); // vp[idx] = p_vec
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisWorkerCharArray
  (JNIEnv *env, jclass thisCls, jobject worker_node, jlong vptr,
   jlong index, jcharArray data, jintArray sizes, 
   jlong flat_size, jlong actual_size) {

  auto fw_node = java_node_to_frovedis_node(env, worker_node);
  auto p_vec = flat_charArray_to_string_vector(env, data, sizes, 
                                        flat_size, actual_size);
  auto vp = (exrpc_ptr_t) vptr;
  size_t idx = index;
  try{
    exrpc_oneway(fw_node, (load_local_vector<std::vector<std::string>>), vp, idx, p_vec); // vp[idx] = p_vec
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisWorkerCharSizePair
  (JNIEnv *env, jclass thisCls, jobject worker_node, jlong dptr, jlong sptr,
   jlong index, jcharArray data, jintArray sizes,
   jlong flat_size, jlong actual_size) {

  auto fw_node = java_node_to_frovedis_node(env, worker_node);
  auto p_dvec = to_char_vector(env, data, flat_size);
  auto p_svec = to_int_vector(env, sizes, actual_size);
  auto dp = (exrpc_ptr_t) dptr;
  auto sp = (exrpc_ptr_t) sptr;
  size_t idx = index;
  try{
    exrpc_oneway(fw_node, (load_local_vector_pair<std::vector<char>, std::vector<int>>), 
                 idx, dp, p_dvec, sp, p_svec); // dp[idx] = p_dvec; sp[idx] = p_svec;
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisWorkerByteSizePair
  (JNIEnv *env, jclass thisCls, jobject worker_node, jlong dptr, jlong sptr,
   jlong index, jbyteArray data, jintArray sizes,
   jlong flat_size, jlong actual_size) {

  auto fw_node = java_node_to_frovedis_node(env, worker_node);
  auto p_dvec = bytes_to_char_vector(env, data, flat_size);
  auto p_svec = to_int_vector(env, sizes, actual_size);
  auto dp = (exrpc_ptr_t) dptr;
  auto sp = (exrpc_ptr_t) sptr;
  size_t idx = index;
  try{
    exrpc_oneway(fw_node, (load_local_vector_pair<std::vector<char>, std::vector<int>>),
                 idx, dp, p_dvec, sp, p_svec); // dp[idx] = p_dvec; sp[idx] = p_svec;
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisWorkerByteSizePair2
  (JNIEnv *env, jclass thisCls, jobject worker_node, jlong dptr, jlong sptr,
   jlong index, jlong datap, jlong sizesp, jlong flat_size, jlong actual_size,
   jboolean rawsend) {

  auto fw_node = java_node_to_frovedis_node(env, worker_node);
  auto send_data_bufp = reinterpret_cast<char*>(datap);
  auto send_size_bufp = reinterpret_cast<char*>(sizesp);
  auto dp = (exrpc_ptr_t) dptr;
  auto sp = (exrpc_ptr_t) sptr;
  try {
    if (rawsend) {
      send_raw_local_vector(send_data_bufp, flat_size, BYTE, dp, index, fw_node);  // dp[idx] = *datap
      send_raw_local_vector(send_size_bufp, actual_size, INT, sp, index, fw_node); // sp[idx] = *sizesp
    } else {
      size_t idx = index;
      std::vector<char> p_dvec(flat_size);
      auto dvecp = p_dvec.data();
      auto d_sendbufp = reinterpret_cast<char*>(datap);
      for (size_t i = 0; i < flat_size; ++i) dvecp[i] = d_sendbufp[i];
      std::vector<int> p_svec(actual_size);
      auto svecp = p_svec.data();
      auto s_sendbufp = reinterpret_cast<int*>(sizesp);
      for (size_t i = 0; i < actual_size; ++i) svecp[i] = s_sendbufp[i];
      exrpc_oneway(fw_node, (load_local_vector_pair<std::vector<char>, std::vector<int>>),
                   idx, dp, p_dvec, sp, p_svec); // dp[idx] = p_dvec; sp[idx] = p_svec;
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

JNIEXPORT jlong JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_createNodeLocalOfWords
  (JNIEnv *env, jclass thisCls, jobject master_node, 
   jlongArray dptrs, jlongArray sptrs, jint nproc, jboolean align) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto dptrs_ = to_exrpc_vector(env, dptrs, nproc);
  auto sptrs_ = to_exrpc_vector(env, sptrs, nproc);
  bool do_align = (bool) align; // whether to align dvector created from spark-side partitioned data
  exrpc_ptr_t proxy = 0;
  try {
    // merges local chunks stored in dptrs_ and sptrs_ to create dvectors,
    // then creates node_local<words> using the created dvectors.
    proxy = exrpc_async(fm_node, make_node_local_words, dptrs_, sptrs_, do_align).get(); 
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return (jlong) proxy;
}

JNIEXPORT jlong JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_createFrovedisDvector
  (JNIEnv *env, jclass thisCls, jobject master_node,
   jlongArray proxies, jint nproc,
   jshort dtype, jboolean align) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto p_vec = to_exrpc_vector(env, proxies, nproc);
  std::vector<size_t> s_vec; bool verify_sizes = false;
  bool do_align = (bool) align; // whether to align dvector created from spark-side partitioned data
  exrpc_ptr_t dvecp = 0;
  try{
    switch(dtype) {
       case BOOL:   
       case INT:    dvecp = exrpc_async(fm_node,merge_and_get_dvector<int>,
                                      p_vec, s_vec, verify_sizes, do_align).get(); break;
       case LONG:   dvecp = exrpc_async(fm_node,merge_and_get_dvector<long>,
                                      p_vec, s_vec, verify_sizes, do_align).get(); break;
       case FLOAT:  dvecp = exrpc_async(fm_node,merge_and_get_dvector<float>,
                                      p_vec, s_vec, verify_sizes, do_align).get(); break;
       case DOUBLE: dvecp = exrpc_async(fm_node,merge_and_get_dvector<double>,
                                      p_vec, s_vec, verify_sizes, do_align).get(); break;
       case STRING: dvecp = exrpc_async(fm_node,merge_and_get_dvector<std::string>,
                                      p_vec, s_vec, verify_sizes, do_align).get(); break;
       default:     REPORT_ERROR(USER_ERROR, 
                    "Unsupported datatype is encountered in dvector creation!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return (jlong) dvecp;
}

JNIEXPORT jlong JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_createFrovedisDvectorWithSizesVerification
  (JNIEnv *env, jclass thisCls, jobject master_node,
   jlongArray proxies, jlongArray sizes, jint nproc, 
   jshort dtype, jboolean align) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto p_vec = to_exrpc_vector(env, proxies, nproc);
  auto s_vec = to_sizet_vector(env, sizes, nproc); bool verify_sizes = true;
  bool do_align = (bool) align; // whether to align dvector created from spark-side partitioned data
  exrpc_ptr_t dvecp = 0;
  try{
    switch(dtype) {
       case BOOL:
       case INT:    dvecp = exrpc_async(fm_node,merge_and_get_dvector<int>,
                                      p_vec, s_vec, verify_sizes, do_align).get(); break;
       case LONG:   dvecp = exrpc_async(fm_node,merge_and_get_dvector<long>,
                                      p_vec, s_vec, verify_sizes, do_align).get(); break;
       case FLOAT:  dvecp = exrpc_async(fm_node,merge_and_get_dvector<float>,
                                      p_vec, s_vec, verify_sizes, do_align).get(); break;
       case DOUBLE: dvecp = exrpc_async(fm_node,merge_and_get_dvector<double>,
                                      p_vec, s_vec, verify_sizes, do_align).get(); break;
       case STRING: dvecp = exrpc_async(fm_node,merge_and_get_dvector<std::string>,
                                      p_vec, s_vec, verify_sizes, do_align).get(); break;
       default:     REPORT_ERROR(USER_ERROR, 
                    "Unsupported datatype is encountered in dvector creation!\n");
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

JNIEXPORT jlongArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getLocalVectorPointers
  (JNIEnv *env, jclass thisCls, jobject master_node, 
   jlong dptr, jshort dtype) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = (exrpc_ptr_t) dptr;
  std::vector<exrpc_ptr_t> eps;
  try {
    switch(dtype) {
      case INT:    eps = exrpc_async(fm_node, get_dvector_local_pointers<int>, f_dptr).get(); break;
      case LONG:   eps = exrpc_async(fm_node, get_dvector_local_pointers<long>, f_dptr).get(); break;
      case FLOAT:  eps = exrpc_async(fm_node, get_dvector_local_pointers<float>, f_dptr).get(); break;
      case DOUBLE: eps = exrpc_async(fm_node, get_dvector_local_pointers<double>, f_dptr).get(); break;
      case STRING: eps = exrpc_async(fm_node, get_dvector_local_pointers<std::string>, f_dptr).get(); break;
      case WORDS:  eps = exrpc_async(fm_node, get_node_local_word_pointers, f_dptr).get(); break;
      case BOOL:   eps = exrpc_async(fm_node, get_dvector_local_pointers<int>, f_dptr).get(); break;
      default:     REPORT_ERROR(USER_ERROR, 
                   "Unsupported datatype is encountered in dvector to RDD conversion!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jlongArray(env, eps);
}

JNIEXPORT jintArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getFrovedisWorkerIntVector
  (JNIEnv *env, jclass thisCls, jobject worker_node, 
   jlong dptr) {
  auto fw_node = java_node_to_frovedis_node(env, worker_node);
  auto f_dptr = (exrpc_ptr_t) dptr;
  std::vector<int> ret;
  try {
    ret = exrpc_async(fw_node, get_local_vector<int>, f_dptr).get();
  }
  catch(std::exception& e) { set_status(true, e.what()); }
  return to_jintArray(env, ret);
}

JNIEXPORT jlongArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getFrovedisWorkerLongVector
  (JNIEnv *env, jclass thisCls, jobject worker_node, 
   jlong dptr) {
  auto fw_node = java_node_to_frovedis_node(env, worker_node);
  auto f_dptr = (exrpc_ptr_t) dptr;
  std::vector<long> ret;
  try {
    ret = exrpc_async(fw_node, get_local_vector<long>, f_dptr).get();
  }
  catch(std::exception& e) { set_status(true, e.what()); }
  return to_jlongArray2(env, ret);
}

JNIEXPORT jfloatArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getFrovedisWorkerFloatVector
  (JNIEnv *env, jclass thisCls, jobject worker_node, 
   jlong dptr) {
  auto fw_node = java_node_to_frovedis_node(env, worker_node);
  auto f_dptr = (exrpc_ptr_t) dptr;
  std::vector<float> ret;
  try {
    ret = exrpc_async(fw_node, get_local_vector<float>, f_dptr).get();
  }
  catch(std::exception& e) { set_status(true, e.what()); }
  return to_jfloatArray(env, ret);
}

JNIEXPORT jdoubleArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getFrovedisWorkerDoubleVector
  (JNIEnv *env, jclass thisCls, jobject worker_node, 
   jlong dptr) {
  auto fw_node = java_node_to_frovedis_node(env, worker_node);
  auto f_dptr = (exrpc_ptr_t) dptr;
  std::vector<double> ret;
  try {
    ret = exrpc_async(fw_node, get_local_vector<double>, f_dptr).get();
  }
  catch(std::exception& e) { set_status(true, e.what()); }
  return to_jdoubleArray(env, ret);
}

JNIEXPORT jobjectArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getFrovedisWorkerWordsAsStringVector
  (JNIEnv *env, jclass thisCls, jobject worker_node, 
   jlong dptr) {
  auto fw_node = java_node_to_frovedis_node(env, worker_node);
  auto f_dptr = (exrpc_ptr_t) dptr;
  std::vector<std::string> ret;
  try {
    ret = exrpc_async(fw_node, get_string_vector_from_words, f_dptr).get();
  }
  catch(std::exception& e) { set_status(true, e.what()); }
  return to_jStringArray(env, ret);
}

JNIEXPORT jobjectArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getFrovedisWorkerStringVector
  (JNIEnv *env, jclass thisCls, jobject worker_node,
   jlong dptr) {
  auto fw_node = java_node_to_frovedis_node(env, worker_node);
  auto f_dptr = (exrpc_ptr_t) dptr;
  std::vector<std::string> ret;
  try {
    ret = exrpc_async(fw_node, get_local_vector<std::string>, f_dptr).get();
  }
  catch(std::exception& e) { set_status(true, e.what()); }
  return to_jStringArray(env, ret);
}

// actually int array containing 0s and 1s
JNIEXPORT jintArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getFrovedisWorkerBoolVector
  (JNIEnv *env, jclass thisCls, jobject worker_node, 
   jlong dptr) {
  auto fw_node = java_node_to_frovedis_node(env, worker_node);
  auto f_dptr = (exrpc_ptr_t) dptr;
  std::vector<int> ret;
  try {
    ret = exrpc_async(fw_node, get_local_vector<int>, f_dptr).get();
  }
  catch(std::exception& e) { set_status(true, e.what()); }
  return to_jintArray(env, ret);
}

}
