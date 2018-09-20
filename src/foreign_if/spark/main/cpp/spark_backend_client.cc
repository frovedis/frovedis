// ----------------------------------------------------------------------------
// NOTE1: This file uses some JAVA package PATHs: 
// e.g., com/nec/frovedis/Jexrpc/Node, com/nec/frovedis/Jexrpc/MemPair etc.
// If you need any changes in JAVA package PATHs, kindly perform 
// necessary changes in "jre_config.hpp" and rebuild the library.
// ----------------------------------------------------------------------------
// NOTE2: As per current implementation only double(DT1) version is supported.
// To support, float(DT2) version, you just need to replicate the below
// functions specifying "DT2" (instead of "DT1") in the template arguments. 
// Similarly, only SPARSE (crs_matrix[_local]<T>) data is supported
// for the training and prediction purposes. If the DENSE version support is 
// provided in Frovedis side, then the same can also be configurable
// just replicating the below sparse wrappers with dense wrappers by
// explicitly specifying "D_MAT1/D_LMAT1" (instead of S_MAT1/S_LMAT1)
// ----------------------------------------------------------------------------

#include "exrpc_request_headers.hpp"
#include "jre_config.hpp"
#include "JNISupport.hpp"
#include <cstdlib>

using namespace frovedis;

// --- global exception tracking variables ---
bool status = false;
std::string info = "";

extern "C" {

void set_status(bool stat, const std::string& what) {
  // if previous exception is not cleared
  if (status) info += "\n -> " + what;
  else {
    status = stat;
    info = "Exception occured at Frovedis Server: \n -> " + what;
  }
}

void reset_status() {
  status = false;
  info = "";
}

std::string get_info() { 
    auto ret = info;
    reset_status();
    return ret;
}

JNIEXPORT jstring JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_checkServerException
  (JNIEnv *env, jclass thisClass) {
  auto info = get_info();
  return env->NewStringUTF(info.c_str());
}

// converting jstring to std::string
inline std::string to_cstring(JNIEnv *env, jstring s) {
  const char *cStr = env->GetStringUTFChars(s, NULL);
  return std::string(cStr);
}

exrpc_node java_node_to_frovedis_node(JNIEnv *env, jobject& n) {
  jclass nodeCls = env->FindClass(JRE_PATH_Node);
  if (nodeCls == NULL) REPORT_ERROR(INTERNAL_ERROR, "Node class not found in JRE\n");
  jfieldID fid = env->GetFieldID(nodeCls, "hostname", "Ljava/lang/String;");
  if(fid == NULL) REPORT_ERROR(INTERNAL_ERROR, "Node::hostname not found in JRE\n");
  jstring hname = (jstring) env->GetObjectField(n, fid);
  fid = env->GetFieldID(nodeCls, "rpcport", "I");
  if(fid == NULL) REPORT_ERROR(INTERNAL_ERROR, "Node::rpcport not found in JRE\n");
  jint pid = env->GetIntField(n, fid);
  return exrpc_node(to_cstring(env,hname),pid);
}

jobject frovedis_node_to_java_node (JNIEnv *env, exrpc_node& n) {
  jclass nodeCls = env->FindClass(JRE_PATH_Node);
  if (nodeCls == NULL) REPORT_ERROR(INTERNAL_ERROR, "Node class not found in JRE\n");
  jmethodID nodeConst = env->GetMethodID(nodeCls, "<init>", "(Ljava/lang/String;I)V");
  if (nodeConst == NULL) REPORT_ERROR(INTERNAL_ERROR, "Node::Node(S,I) not found in JRE\n");
  jstring hname = env->NewStringUTF(n.hostname.c_str());
  jint pid = n.rpcport;
  auto newexrpc_node = env->NewObject(nodeCls,nodeConst,hname,pid);
  if (newexrpc_node == NULL) REPORT_ERROR(INTERNAL_ERROR, "exrpc_node object creation failed\n");
  return newexrpc_node;
}

jobject frovedis_dummyGLM_to_java_dummyGLM(JNIEnv *env, dummy_glm& obj) { 
  jclass dglmCls = env->FindClass(JRE_PATH_DummyGLM);
  if (dglmCls == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyGLM class not found in JRE\n");
  jmethodID dglmConst = env->GetMethodID(dglmCls, "<init>", "(ISJIDD)V");
  if (dglmConst == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyGLM::DummyGLM(I,S,J,I,D,D) not found in JRE\n");
  long nftr = static_cast<long>(obj.numFeatures);
  auto newDummyGLM = env->NewObject(dglmCls, dglmConst,
                                    obj.mid, obj.mkind, 
                                    nftr, obj.numClasses,
                                    obj.intercept, obj.threshold);
  if (newDummyGLM == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyGLM object creation failed\n");
  return newDummyGLM;
}

jobject to_jDummyMatrix(JNIEnv *env, dummy_matrix& obj, short mtype) {
  jclass dmatCls = env->FindClass(JRE_PATH_DummyMatrix);
  if (dmatCls == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyMatrix class not found in JRE\n");
  jmethodID dmatConst = env->GetMethodID(dmatCls, "<init>", "(JJJS)V");
  if (dmatConst == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyMatrix(J,J,J,S) not found in JRE\n");
  long mptr = static_cast<long>(obj.mptr);
  long nrow = static_cast<long>(obj.nrow);
  long ncol = static_cast<long>(obj.ncol);
  auto newDummyMat = env->NewObject(dmatCls,dmatConst,mptr,nrow,ncol,mtype);
  if (newDummyMat == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyMatrix object creation failed\n");
  return newDummyMat;
}

jobject to_jDummyGetrfResult(JNIEnv *env, getrf_result& obj, short mtype) {
  jclass rfCls = env->FindClass(JRE_PATH_DummyGetrfResult);
  if (rfCls == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyGetrfResult class not found in JRE\n");
  jmethodID rfConst = env->GetMethodID(rfCls, "<init>", "(JIS)V");
  if (rfConst == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyGetrfResult(J,I,S) not found in JRE\n");
  long ipivp = static_cast<long>(obj.ipiv_ptr);
  auto newRF = env->NewObject(rfCls,rfConst,ipivp,obj.info,mtype);
  if (newRF == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyGetrfResult object creation failed\n");
  return newRF;
}

jobject to_jDummyGesvdResult(JNIEnv *env, gesvd_result& obj, short mtype,
                             bool wantU, bool wantV) {
  jclass svdCls = env->FindClass(JRE_PATH_DummyGesvdResult);
  if (svdCls == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyGesvdResult class not found in JRE\n");
  jmethodID svdConst = env->GetMethodID(svdCls, "<init>", "(JJJIIIIS)V");
  if (svdConst == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyGesvdResult(JJJIIIIS) not found in JRE\n");
  long svecp = static_cast<long>(obj.svec_ptr);
  long umatp = wantU ? static_cast<long>(obj.umat_ptr) : -1;
  long vmatp = wantV ? static_cast<long>(obj.vmat_ptr) : -1;
  auto newSVD = env->NewObject(svdCls, svdConst, svecp, umatp, vmatp,
                               obj.m, obj.n, obj.k, obj.info, mtype);
  if (newSVD == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyGesvdResult object creation failed\n");
  return newSVD;
}

frovedis_mem_pair java_mempair_to_frovedis_mempair(JNIEnv *env, jobject& mp) {
  jclass mpCls = env->FindClass(JRE_PATH_MemPair);
  if (mpCls == NULL) REPORT_ERROR(INTERNAL_ERROR, "MemPair class not found in JRE\n");
  jfieldID fid = env->GetFieldID(mpCls, "mx", "J");
  if(fid == NULL) REPORT_ERROR(INTERNAL_ERROR, "MemPair::mx not found in JRE\n");
  exrpc_ptr_t first = (exrpc_ptr_t) env->GetLongField(mp, fid);
  fid = env->GetFieldID(mpCls, "my", "J");
  if(fid == NULL) REPORT_ERROR(INTERNAL_ERROR, "MemPair::my not found in JRE\n");
  exrpc_ptr_t second = (exrpc_ptr_t) env->GetLongField(mp, fid);
  return frovedis_mem_pair(first,second);
}

jobject frovedis_mempair_to_java_mempair(JNIEnv *env, frovedis_mem_pair& mp) {
  jclass mpCls = env->FindClass(JRE_PATH_MemPair);
  if (mpCls == NULL) REPORT_ERROR(INTERNAL_ERROR, "MemPair class not found in JRE\n");
  jmethodID mpConst = env->GetMethodID(mpCls, "<init>", "(JJ)V");
  if (mpConst == NULL) REPORT_ERROR(INTERNAL_ERROR, "MemPair::MemPair(J,J) not found in JRE\n");
  jlong f = (jlong) mp.first();
  jlong s = (jlong) mp.second();
  auto newmempair = env->NewObject(mpCls,mpConst,f,s);
  if (newmempair == NULL) REPORT_ERROR(INTERNAL_ERROR, "MemPair object creation failed\n");
  return newmempair;
}

jobject make_jIntDoublePair (JNIEnv *env, int key, double value) {
  jclass cls = env->FindClass(JRE_PATH_IntDoublePair);
  if (cls == NULL) REPORT_ERROR(INTERNAL_ERROR, "IntDoublePair class not found in JRE\n");
  jmethodID Const = env->GetMethodID(cls, "<init>", "(ID)V");
  if (Const == NULL) REPORT_ERROR(INTERNAL_ERROR, "IntDoublePair(I,D) not found in JRE\n");
  auto newPair = env->NewObject(cls,Const,key,value);
  if (newPair == NULL) REPORT_ERROR(INTERNAL_ERROR, "IntDoublePair object creation failed\n");
  return newPair;
}

crs_matrix_local<double> 
get_frovedis_double_crs_matrix_local(JNIEnv *env, jlong nrows, jlong ncols,
                                   jintArray& off, jintArray& idx, 
                                   jdoubleArray& val) {
  jsize d_len = env->GetArrayLength(idx);
  jint *offp = env->GetIntArrayElements(off, 0);       int* offp_ = offp;
  jint *idxp = env->GetIntArrayElements(idx, 0);       int* idxp_ = idxp;
  jdouble *valp = env->GetDoubleArrayElements(val, 0); double* valp_ = valp;
  auto r = static_cast<size_t>(nrows);
  auto c = static_cast<size_t>(ncols);
  crs_matrix_local<double> l_mat(r,c);
  l_mat.copy_from_jarray(offp_,idxp_,valp_,d_len);
  return l_mat;
}

// conversion std::vector<exrpc_ptr_t> => jlongArray
jlongArray to_jlongArray(JNIEnv *env, std::vector<exrpc_ptr_t>& eps) {
  size_t sz = eps.size();
  std::vector<long> l_eps(sz);
  for(size_t i=0; i<sz; ++i) l_eps[i] = (long) eps[i];
  jlong* arr = &l_eps[0];
  jlongArray ret = env->NewLongArray(sz);
  if(ret == NULL) REPORT_ERROR(INTERNAL_ERROR, "New jlongArray allocation failed.\n");
  env->SetLongArrayRegion(ret, 0, sz, arr);
  return ret;
}
  
// conversion std::vector<double> => jdoubleArray
jdoubleArray to_jdoubleArray(JNIEnv *env, std::vector<double>& pd) {
  jdouble* arr = &pd[0];
  size_t sz = pd.size();
  jdoubleArray ret = env->NewDoubleArray(sz);
  if(ret == NULL) REPORT_ERROR(INTERNAL_ERROR, "New jdoubleArray allocation failed.\n");
  env->SetDoubleArrayRegion(ret, 0, sz, arr);
  return ret;
}
  
// conversion std::vector<int> => jintArray
jintArray to_jintArray(JNIEnv *env, std::vector<int>& pd) {
  jint* arr = &pd[0];
  size_t sz = pd.size();
  jintArray ret = env->NewIntArray(sz);
  if(ret == NULL) REPORT_ERROR(INTERNAL_ERROR, "New jintArray allocation failed.\n");
  env->SetIntArrayRegion(ret, 0, sz, arr);
  return ret;
}

jobjectArray to_jStringArray(JNIEnv *env, std::vector<std::string>& data) {
  auto size = data.size();
  jclass str_cls = env->FindClass(JRE_PATH_STRING);
  if(str_cls == NULL) REPORT_ERROR(INTERNAL_ERROR, "String class not found in JRE.\n");
  jobjectArray sarray = env->NewObjectArray(size, str_cls, NULL);
  if(sarray == NULL) REPORT_ERROR(INTERNAL_ERROR, "New java string array allocation failed.\n");
  for (int i=0; i<size; i++) {
    jstring js = env->NewStringUTF(data[i].c_str());
    env->SetObjectArrayElement(sarray, i, js);
  }
  return sarray;
}

// --- typed vector conversion ---  
// conversion jobjectArray(string type) => std::vector<std::string>
std::vector<std::string> 
to_string_vector(JNIEnv *env, jobjectArray& data, size_t size) {
  jsize d_len = env->GetArrayLength(data);
  if(d_len != size) REPORT_ERROR(INTERNAL_ERROR, "Error in data extraction from JRE");
  std::vector<std::string> data_vec(d_len);
  for(size_t i=0; i<d_len; ++i) { 
    jstring js = (jstring) (env->GetObjectArrayElement(data,i));
    const char *rawstr = env->GetStringUTFChars(js, 0);
    std::string str(rawstr);
    data_vec[i] = str;
    env->ReleaseStringUTFChars(js,rawstr);
  }
  return data_vec;
}

// conversion jfloatArray => std::vector<float>
std::vector<float> to_float_vector(JNIEnv *env, jfloatArray& data, size_t size) {
  jsize d_len = env->GetArrayLength(data);
  if(d_len != size) REPORT_ERROR(INTERNAL_ERROR, "Error in data extraction from JRE");
  jfloat *datap = env->GetFloatArrayElements(data, 0);  float* datap_ = datap;
  std::vector<float> data_vec(d_len);
  for(size_t i=0; i<d_len; ++i) data_vec[i] = datap_[i];
  env->ReleaseFloatArrayElements(data,datap,JNI_ABORT);
  return data_vec;
}

// conversion jdoubleArray => std::vector<double>
std::vector<double> to_double_vector(JNIEnv *env, jdoubleArray& data, size_t size) {
  jsize d_len = env->GetArrayLength(data);
  if(d_len != size) REPORT_ERROR(INTERNAL_ERROR, "Error in data extraction from JRE");
  jdouble *datap = env->GetDoubleArrayElements(data, 0);  double* datap_ = datap;
  std::vector<double> data_vec(d_len);
  for(size_t i=0; i<d_len; ++i) data_vec[i] = datap_[i];
  env->ReleaseDoubleArrayElements(data,datap,JNI_ABORT);
  return data_vec;
}

// conversion jintArray => std::vector<int>
std::vector<int> to_int_vector(JNIEnv *env, jintArray& data, size_t size) {
  jsize d_len = env->GetArrayLength(data);
  if(d_len != size) REPORT_ERROR(INTERNAL_ERROR, "Error in data extraction from JRE");
  jint *datap = env->GetIntArrayElements(data, 0);  int* datap_ = datap;
  std::vector<int> data_vec(d_len);
  for(size_t i=0; i<d_len; ++i) data_vec[i] = datap_[i];
  env->ReleaseIntArrayElements(data,datap,JNI_ABORT);
  return data_vec;
}

// conversion jshortArray => std::vector<short>
std::vector<short> to_short_vector(JNIEnv *env, jshortArray& data, size_t size) {
  jsize d_len = env->GetArrayLength(data);
  if(d_len != size) REPORT_ERROR(INTERNAL_ERROR, "Error in data extraction from JRE");
  jshort *datap = env->GetShortArrayElements(data, 0);  short* datap_ = datap;
  std::vector<short> data_vec(d_len);
  for(size_t i=0; i<d_len; ++i) data_vec[i] = datap_[i];
  env->ReleaseShortArrayElements(data,datap,JNI_ABORT);
  return data_vec;
}

// conversion jlongArray => std::vector<long>
std::vector<long> to_long_vector(JNIEnv *env, jlongArray& data, size_t size) {
  jsize d_len = env->GetArrayLength(data);
  if(d_len != size) REPORT_ERROR(INTERNAL_ERROR, "Error in data extraction from JRE");
  jlong *datap = env->GetLongArrayElements(data, 0);  long* datap_ = datap;
  std::vector<long> data_vec(d_len);
  for(size_t i=0; i<d_len; ++i) data_vec[i] = datap_[i];
  env->ReleaseLongArrayElements(data,datap,JNI_ABORT);
  return data_vec;
}

// conversion jlongArray => std::vector<size_t>
std::vector<size_t>
to_sizet_vector(JNIEnv *env, jlongArray& data, size_t size) {
  jsize d_len = env->GetArrayLength(data);
  if(d_len != size) REPORT_ERROR(INTERNAL_ERROR, "Error in data extraction from JRE");
  jlong *datap = env->GetLongArrayElements(data, 0);  long* datap_ = datap;
  std::vector<size_t> data_vec(d_len);
  for(size_t i=0; i<d_len; ++i) data_vec[i] = static_cast<size_t>(datap_[i]);
  env->ReleaseLongArrayElements(data,datap,JNI_ABORT);
  return data_vec;
}

// conversion jlongArray => std::vector<exrpc_ptr_t>
std::vector<exrpc_ptr_t>
to_exrpc_vector(JNIEnv *env, jlongArray& data, size_t size) {
  jsize d_len = env->GetArrayLength(data);
  if(d_len != size) REPORT_ERROR(INTERNAL_ERROR, "Error in data extraction from JRE");
  jlong *datap = env->GetLongArrayElements(data, 0);  long* datap_ = datap;
  std::vector<exrpc_ptr_t> data_vec(d_len);
  for(size_t i=0; i<d_len; ++i) data_vec[i] = static_cast<exrpc_ptr_t>(datap_[i]);
  env->ReleaseLongArrayElements(data,datap,JNI_ABORT);
  return data_vec;
}

// conversion jbooleanArray => std::vector<int>
// bool (true/false) will be stored as integer (1/0)
// since C++ std::vector<bool> handles "bool" objects differently
std::vector<int> to_bool_vector(JNIEnv *env, jbooleanArray& data, size_t size) {
  jsize d_len = env->GetArrayLength(data);
  if(d_len != size) REPORT_ERROR(INTERNAL_ERROR, "Error in data extraction from JRE");
  jboolean *datap = env->GetBooleanArrayElements(data, 0);  // bool* datap_ = datap;
  std::vector<int> data_vec(d_len);
  for(size_t i=0; i<d_len; ++i) data_vec[i] = datap[i] ? 1 : 0;
  env->ReleaseBooleanArrayElements(data,datap,JNI_ABORT);
  return data_vec;
}

  
// conversion std::vector<frovedis::exrpc_node> => jobjectArray(java::Node)
jobjectArray to_jNodeArray(JNIEnv *env, std::vector<frovedis::exrpc_node>& nodes) {
  jclass nodeCls = env->FindClass(JRE_PATH_Node);
  if (nodeCls == NULL) REPORT_ERROR(INTERNAL_ERROR, "Node class not found in JRE\n");
  jobjectArray ret = env->NewObjectArray(nodes.size(), nodeCls, NULL);
  if (ret == NULL) REPORT_ERROR(INTERNAL_ERROR, "Node jNodeArray creation failed!\n");
  for(size_t i=0; i<nodes.size(); ++i){
    auto n = frovedis_node_to_java_node(env,nodes[i]);
    env->SetObjectArrayElement(ret, i, n);
  }
  return ret;
}

// conversion std::vector<std::pair<size_t,double>> => jobjectArray(IntDoublePair)
jobjectArray to_jIntDoublePairArray(JNIEnv *env, 
                                   std::vector<std::pair<size_t,double>>& pd) {

  jclass cls = env->FindClass(JRE_PATH_IntDoublePair);
  if (cls == NULL) REPORT_ERROR(INTERNAL_ERROR, "IntDoublePair class not found in JRE\n");  
  jobjectArray ret = env->NewObjectArray(pd.size(), cls, NULL);
  if(ret == NULL) REPORT_ERROR(INTERNAL_ERROR, "New jIntDoublePairArray allocation failed.\n");
  for(size_t i=0; i<pd.size(); ++i){
    auto p = make_jIntDoublePair(env, pd[i].first, pd[i].second);
    env->SetObjectArrayElement(ret, i, p);
  }  
  return ret;
}
  
// returns the node information of Frovedis master node
JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getMasterInfo
  (JNIEnv *env, jclass thisCls, jstring cmd) {
  auto n = invoke_frovedis_server(to_cstring(env,cmd)); 
  return frovedis_node_to_java_node(env, n);
}

// returns the active worker node count at Frovedis side
JNIEXPORT jint JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getWorkerSize
  (JNIEnv *env, jclass thisCls, jobject master_node) {
  auto fm_node = java_node_to_frovedis_node(env,master_node);
  return exrpc_async0(fm_node, get_nodesize).get();
}

// connects with Frovedis worker nodes and returns their node informations
JNIEXPORT jobjectArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getWorkerInfo
  (JNIEnv *env, jclass thisCls, jobject master_node) {
  auto fm_node = java_node_to_frovedis_node(env,master_node);
  auto info = prepare_parallel_exrpc(fm_node);
  auto nodes = get_parallel_exrpc_nodes(fm_node, info);
  wait_parallel_exrpc(fm_node, info);
  return to_jNodeArray(env,nodes);
}

// it returns a mem_pair containing the heads of crs_matrix_local and vector (for labels)
JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisWorkerGLMData
  (JNIEnv *env, jclass thisCls, jobject target_node, jlong nrows, jlong ncols, 
   jdoubleArray lbl, jintArray off, jintArray idx, jdoubleArray val) {

  auto l_mat = get_frovedis_double_crs_matrix_local(env,nrows,ncols,off,idx,val);
  jsize v_len = env->GetArrayLength(lbl);
  jdouble *lblp = env->GetDoubleArrayElements(lbl, 0); double* lblp_ = lblp;
  std::vector<double> label(v_len);
  for(size_t i=0; i<v_len; ++i) label[i] = lblp_[i];

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
   jboolean dense) {
  
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  bool isDense = (bool) dense;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node (" 
            << fm_node.hostname << "," << fm_node.rpcport 
            << ") to release frovedis data.\n";
#endif
  try{
     if(isDense) exrpc_oneway(fm_node,(release_glm_data<DT1,D_MAT1>),f_dptr);
     else        exrpc_oneway(fm_node,(release_glm_data<DT1,S_MAT1>),f_dptr);
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
   jboolean dense) {
  
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  bool isDense = (bool) dense;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node (" 
            << fm_node.hostname << "," << fm_node.rpcport 
            << ") to print frovedis data.\n";
#endif
  try{
     if(isDense) exrpc_oneway(fm_node,(show_glm_data<DT1,D_MAT1>),f_dptr);
     else        exrpc_oneway(fm_node,(show_glm_data<DT1,S_MAT1>),f_dptr);
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
      case RMJR: ret = exrpc_async(fm_node,transpose_matrix<R_MAT1>,f_dptr).get(); break;
      case CMJR: REPORT_ERROR(USER_ERROR,"Frovedis doesn't support this transpose currently!\n");
      case BCLC: ret = exrpc_async(fm_node,transpose_matrix<B_MAT1>,f_dptr).get(); break;
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
  finalize_frovedis_server(fm_node);
}

// initiates the training call at Frovedis master node for LogisticRegressionWithSGD
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisLRSGD
  (JNIEnv *env, jclass thisCls, jobject master_node, jobject fdata, 
   jint numIter, jdouble stepSize, jdouble mbf, 
   jdouble regParam, jint mid, jboolean movable, jboolean dense) {
  
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  bool mvbl = (bool) movable;
  bool isDense = (bool) dense;
  int rtype = 0; // ZERO (default)
  bool icpt = false; // default
  double tol = 0.001; // default
  int vb = 0; // no log (default)
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node (" 
            << fm_node.hostname << "," << fm_node.rpcport 
            << ") to train frovedis LR_SGD.\n";
#endif
  try{
    if(isDense) 
      exrpc_oneway(fm_node,(frovedis_lr_sgd<DT1,D_MAT1>),f_dptr,numIter,stepSize,mbf,rtype,regParam,icpt,tol,vb,mid,mvbl);
    else
      exrpc_oneway(fm_node,(frovedis_lr_sgd<DT1,S_MAT1>),f_dptr,numIter,stepSize,mbf,rtype,regParam,icpt,tol,vb,mid,mvbl);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}
  

// initiates the training call at Frovedis master node for LogisticRegressionWithLBFGS
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisLRLBFGS
  (JNIEnv *env, jclass thisCls, jobject master_node, jobject fdata,
   jint numIter, jdouble stepSize, jint histSize,
   jdouble regParam, jint mid, jboolean movable, jboolean dense) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  bool mvbl = (bool) movable;
  bool isDense = (bool) dense;
  int rtype = 0; // ZERO (default)
  bool icpt = false; // default
  double tol = 0.001; // default
  int vb = 0; // no log (default)
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to train frovedis LR_LBFGS.\n";
#endif
  try {
    if (isDense)
      exrpc_oneway(fm_node,(frovedis_lr_lbfgs<DT1,D_MAT1>),f_dptr,numIter,stepSize,histSize,rtype,regParam,icpt,tol,vb,mid,mvbl);
    else
      exrpc_oneway(fm_node,(frovedis_lr_lbfgs<DT1,S_MAT1>),f_dptr,numIter,stepSize,histSize,rtype,regParam,icpt,tol,vb,mid,mvbl);
  } 
  catch(std::exception& e) { set_status(true,e.what()); }
}

// initiates the training call at Frovedis master node for SVMWithSGD
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisSVMSGD
  (JNIEnv *env, jclass thisCls, jobject master_node, jobject fdata, 
   jint numIter, jdouble stepSize, jdouble mbf, 
   jdouble regParam, jint mid, jboolean movable, jboolean dense) {
  
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  bool mvbl = (bool) movable;
  int rtype = 0; // ZERO (default)
  bool icpt = false; // default
  double tol = 0.001; // default
  int vb = 0; // no log (default)
  bool isDense = (bool) dense;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node (" 
            << fm_node.hostname << "," << fm_node.rpcport 
            << ") to train frovedis SVM_SGD.\n";
#endif
  try {
    if(isDense)
      exrpc_oneway(fm_node,(frovedis_svm_sgd<DT1,D_MAT1>),f_dptr,numIter,stepSize,mbf,rtype,regParam,icpt,tol,vb,mid,mvbl);
    else
      exrpc_oneway(fm_node,(frovedis_svm_sgd<DT1,S_MAT1>),f_dptr,numIter,stepSize,mbf,rtype,regParam,icpt,tol,vb,mid,mvbl);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// initiates the training call at Frovedis master node for SVMWithLBFGS
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisSVMLBFGS
  (JNIEnv *env, jclass thisCls, jobject master_node, jobject fdata,
   jint numIter, jdouble stepSize, jint histSize,
   jdouble regParam, jint mid, jboolean movable, jboolean dense) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  bool mvbl = (bool) movable;
  int rtype = 0; // ZERO (default)
  bool icpt = false; // default
  double tol = 0.001; // default
  int vb = 0; // no log (default)
  bool isDense = (bool) dense;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to train frovedis SVM_LBFGS.\n";
#endif
  try {
    if(isDense)
      exrpc_oneway(fm_node,(frovedis_svm_lbfgs<DT1,D_MAT1>),f_dptr,numIter,stepSize,histSize,rtype,regParam,icpt,tol,vb,mid,mvbl);
    else
      exrpc_oneway(fm_node,(frovedis_svm_lbfgs<DT1,S_MAT1>),f_dptr,numIter,stepSize,histSize,rtype,regParam,icpt,tol,vb,mid,mvbl);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// initiates the training call at Frovedis master node for LinearRegressionWithSGD
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisLNRSGD
  (JNIEnv *env, jclass thisCls, jobject master_node, jobject fdata, 
   jint numIter, jdouble stepSize, jdouble mbf, 
   jint mid, jboolean movable, jboolean dense) {
  
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  bool mvbl = (bool) movable;
  bool icpt = false; // default
  int vb = 0; // no log (default)
  bool isDense = (bool) dense;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node (" 
            << fm_node.hostname << "," << fm_node.rpcport 
            << ") to train frovedis LNR_SGD.\n";
#endif
  try {
    if(isDense)
      exrpc_oneway(fm_node,(frovedis_lnr_sgd<DT1,D_MAT1>),f_dptr,numIter,stepSize,mbf,icpt,vb,mid,mvbl);
    else
      exrpc_oneway(fm_node,(frovedis_lnr_sgd<DT1,S_MAT1>),f_dptr,numIter,stepSize,mbf,icpt,vb,mid,mvbl);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// initiates the training call at Frovedis master node for LinearRegressionWithLBFGS
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisLNRLBFGS
  (JNIEnv *env, jclass thisCls, jobject master_node, jobject fdata,
   jint numIter, jdouble stepSize, jint histSize,
   jint mid, jboolean movable, jboolean dense) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  bool mvbl = (bool) movable;
  bool icpt = false; // default
  int vb = 0; // no log (default)
  bool isDense = (bool) dense;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to train frovedis LNR_LBFGS.\n";
#endif 
  try {
    if(isDense)
      exrpc_oneway(fm_node,(frovedis_lnr_lbfgs<DT1,D_MAT1>),f_dptr,numIter,stepSize,histSize,icpt,vb,mid,mvbl);
    else
      exrpc_oneway(fm_node,(frovedis_lnr_lbfgs<DT1,S_MAT1>),f_dptr,numIter,stepSize,histSize,icpt,vb,mid,mvbl);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// initiates the training call at Frovedis master node for LassoWithSGD
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisLassoSGD
  (JNIEnv *env, jclass thisCls, jobject master_node, jobject fdata, 
   jint numIter, jdouble stepSize, jdouble mbf, 
   jdouble regParam, jint mid, jboolean movable, jboolean dense) {
  
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  bool mvbl = (bool) movable;
  bool icpt = false; // default
  double tol = 0.001; // default
  int vb = 0; // no log (default)
  bool isDense = (bool) dense;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node (" 
            << fm_node.hostname << "," << fm_node.rpcport 
            << ") to train frovedis Lasso_SGD.\n";
#endif
  try {
    if(isDense)
      exrpc_oneway(fm_node,(frovedis_lasso_sgd<DT1,D_MAT1>),f_dptr,numIter,stepSize,mbf,regParam,icpt,tol,vb,mid,mvbl);
    else
      exrpc_oneway(fm_node,(frovedis_lasso_sgd<DT1,S_MAT1>),f_dptr,numIter,stepSize,mbf,regParam,icpt,tol,vb,mid,mvbl);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// initiates the training call at Frovedis master node for LassoWithLBFGS
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisLassoLBFGS
  (JNIEnv *env, jclass thisCls, jobject master_node, jobject fdata,
   jint numIter, jdouble stepSize, jint histSize,
   jdouble regParam, jint mid, jboolean movable, jboolean dense) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  bool mvbl = (bool) movable;
  bool icpt = false; // default
  double tol = 0.001; // default
  int vb = 0; // no log (default)
  bool isDense = (bool) dense;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to train frovedis Lasso_LBFGS.\n";
#endif
  try {
    if(isDense)
      exrpc_oneway(fm_node,(frovedis_lasso_lbfgs<DT1,D_MAT1>),f_dptr,numIter,stepSize,histSize,regParam,icpt,tol,vb,mid,mvbl);
    else
      exrpc_oneway(fm_node,(frovedis_lasso_lbfgs<DT1,S_MAT1>),f_dptr,numIter,stepSize,histSize,regParam,icpt,tol,vb,mid,mvbl);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// initiates the training call at Frovedis master node for RidgeRegressionWithSGD
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisRidgeSGD
  (JNIEnv *env, jclass thisCls, jobject master_node, jobject fdata, 
   jint numIter, jdouble stepSize, jdouble mbf, 
   jdouble regParam, jint mid, jboolean movable, jboolean dense) {
  
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  bool mvbl = (bool) movable;
  bool icpt = false; // default
  double tol = 0.001; // default
  int vb = 0; // no log (default)
  bool isDense = (bool) dense;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node (" 
            << fm_node.hostname << "," << fm_node.rpcport 
            << ") to train frovedis Ridge_SGD.\n";
#endif
  try {
    if(isDense)
      exrpc_oneway(fm_node,(frovedis_ridge_sgd<DT1,D_MAT1>),f_dptr,numIter,stepSize,mbf,regParam,icpt,tol,vb,mid,mvbl);
    else
      exrpc_oneway(fm_node,(frovedis_ridge_sgd<DT1,S_MAT1>),f_dptr,numIter,stepSize,mbf,regParam,icpt,tol,vb,mid,mvbl);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// initiates the training call at Frovedis master node for RidgeRegressionWithLBFGS
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisRidgeLBFGS
  (JNIEnv *env, jclass thisCls, jobject master_node, jobject fdata,
   jint numIter, jdouble stepSize, jint histSize,
   jdouble regParam, jint mid, jboolean movable, jboolean dense) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  bool mvbl = (bool) movable;
  bool icpt = false; // default
  double tol = 0.001; // default
  int vb = 0; // no log (default)
  bool isDense = (bool) dense;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to train frovedis Ridge_LBFGS.\n";
#endif
  try {
    if(isDense)
      exrpc_oneway(fm_node,(frovedis_ridge_lbfgs<DT1,D_MAT1>),f_dptr,numIter,stepSize,histSize,regParam,icpt,tol,vb,mid,mvbl);
    else
      exrpc_oneway(fm_node,(frovedis_ridge_lbfgs<DT1,S_MAT1>),f_dptr,numIter,stepSize,histSize,regParam,icpt,tol,vb,mid,mvbl);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// initiates the training call at Frovedis master node for MatrixFactorizationUsingALS 
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisMFUsingALS
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata,
   jint rank, jint numIter, jdouble alpha, jdouble regParam,
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
    exrpc_oneway(fm_node,(frovedis_mf_als<DT1,S_MAT1>),f_dptr,rank,numIter,alpha,regParam,seed,vb,mid,mvbl);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// initiates the training call at Frovedis master node for KMeans 
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisKMeans
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong fdata,
   jint k, jint numIter, jlong seed, jdouble epsilon,
   jint mid, jboolean movable, jboolean dense) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = (exrpc_ptr_t) fdata;
  bool mvbl = (bool) movable;
  int vb = 0; // no log (default)
  bool isDense = (bool) dense;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to train frovedis KMeans.\n";
#endif
  try {
    if(isDense) // kmeans accepts rowmajor matrix as for dense data
      exrpc_oneway(fm_node,(frovedis_kmeans<DT1,R_MAT1>),f_dptr,k,numIter,seed,epsilon,vb,mid,mvbl);
    else
      exrpc_oneway(fm_node,(frovedis_kmeans<DT1,S_MAT1>),f_dptr,k,numIter,seed,epsilon,vb,mid,mvbl);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

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

  try{
    if(isDense) 
      exrpc_oneway(fm_node,(frovedis_dt<DT1,D_MAT1>),f_dptr,str,vb,mid,mvbl);
    else
      REPORT_ERROR(USER_ERROR, "currently Frovedis doesn't support decision tree train with sparse data!\n");
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// calling frovedis server side Naive Bayes trainer
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_callFrovedisNBM
  (JNIEnv *env, jclass thisCls, jobject master_node, jobject fdata,
   jdouble lambda, jint model_id, jstring modelType, 
   jboolean movable, jboolean dense) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = java_mempair_to_frovedis_mempair(env, fdata);
  bool mvbl = (bool) movable;
  bool isDense = (bool) dense;
  int vb = 0; // no log (default)
  auto mtype = to_cstring(env,modelType);
  try {
    if(isDense)
      exrpc_oneway(fm_node,(frovedis_nb<DT1,D_MAT1,D_LMAT1>),f_dptr,mtype,lambda,vb,model_id,mvbl);
    else
      exrpc_oneway(fm_node,(frovedis_nb<DT1,S_MAT1,S_LMAT1>),f_dptr,mtype,lambda,vb,model_id,mvbl);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// prints the requested (registered) trained LR model information for debugging purpose
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_showFrovedisModel
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid, jshort mkind) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  try {
    switch(mkind) {
      case LRM:    exrpc_oneway(fm_node, show_model<LRM1>, mid); break;
      case SVM:    exrpc_oneway(fm_node, show_model<SVM1>, mid); break;
      case LNRM:   exrpc_oneway(fm_node, show_model<LNRM1>, mid); break;
      case MFM:    exrpc_oneway(fm_node, show_model<MFM1>, mid); break;
      case KMEANS: exrpc_oneway(fm_node, show_model<KMM1>, mid); break;
      case DTM:    exrpc_oneway(fm_node, show_model<DTM1>, mid); break;
      case FMM:    REPORT_ERROR(USER_ERROR, "currently Frovedis fm_model cant be displayed!");
      case NBM:    exrpc_oneway(fm_node, show_model<NBM1>, mid); break;
      default:     REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
   }
 }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// releases the requested (registered) trained LR model from the heap
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_releaseFrovedisModel
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid, jshort mkind) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  try{
    switch(mkind) {
      case LRM:    exrpc_oneway(fm_node, release_model<LRM1>, mid); break;
      case SVM:    exrpc_oneway(fm_node, release_model<SVM1>, mid); break;
      case LNRM:   exrpc_oneway(fm_node, release_model<LNRM1>, mid); break;
      case MFM:    exrpc_oneway(fm_node, release_model<MFM1>, mid); break;
      case KMEANS: exrpc_oneway(fm_node, release_model<KMM1>, mid); break;
      case DTM:    exrpc_oneway(fm_node, release_model<DTM1>, mid); break;
      case FMM:    exrpc_oneway(fm_node, release_model<FMM1>, mid); break;
      case NBM:    exrpc_oneway(fm_node, release_model<NBM1>, mid); break;
      default:     REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
} 

// saves the model to the specified file 
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_saveFrovedisModel
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid, jshort mkind, jstring path) {

  auto fs_path = to_cstring(env,path);
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  try {
    switch(mkind) {
      case LRM:    exrpc_oneway(fm_node,save_model<LRM1>,mid,fs_path); break;
      case SVM:    exrpc_oneway(fm_node,save_model<SVM1>,mid,fs_path); break;
      case LNRM:   exrpc_oneway(fm_node,save_model<LNRM1>,mid,fs_path); break;
      case MFM:    exrpc_oneway(fm_node,save_model<MFM1>,mid,fs_path); break;
      case KMEANS: exrpc_oneway(fm_node,save_model<KMM1>,mid,fs_path); break;
      case DTM:    exrpc_oneway(fm_node,save_model<DTM1>,mid,fs_path); break;
      case FMM:    exrpc_oneway(fm_node,save_fmm<DT1>,mid,fs_path); break;
      case NBM:    exrpc_oneway(fm_node,save_model<NBM1>,mid,fs_path); break;
      default:     REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// broadcasts the registered model from Frovedis master node to worker nodes
// and returns a vector containing model heads at worker nodes
JNIEXPORT jlongArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_broadcast2AllWorkers
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid, jshort mkind) {
  
  auto fm_node = java_node_to_frovedis_node(env, master_node);
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node (" 
            << fm_node.hostname << "," << fm_node.rpcport 
            << ") to broadcast model[" << mid << "] among worker nodes.\n";
#endif

  std::vector<exrpc_ptr_t> eps;
  jlongArray ret = NULL;
  try {
    switch(mkind) {
      case LRM:    eps = exrpc_async(fm_node,(bcast_model_to_workers<DT1,LRM1>),mid).get(); break;
      case SVM:    eps = exrpc_async(fm_node,(bcast_model_to_workers<DT1,SVM1>),mid).get(); break;
      case LNRM:   eps = exrpc_async(fm_node,(bcast_model_to_workers<DT1,LNRM1>),mid).get(); break;
      case MFM:    eps = exrpc_async(fm_node,(bcast_model_to_workers<DT1,MFM1>),mid).get(); break;
      case KMEANS: eps = exrpc_async(fm_node,(bcast_model_to_workers<DT1,KMM1>),mid).get(); break;
      case DTM:    eps = exrpc_async(fm_node,(bcast_model_to_workers<DT1,DTM1>),mid).get(); break;
      case FMM:    eps = exrpc_async(fm_node,bcast_fmm_to_workers<DT1>,mid).get(); break;
      case NBM:    eps = exrpc_async(fm_node,(bcast_model_to_workers<DT1,NBM1>),mid).get(); break;
      default:   REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
    }
    // converting eps to jlongArray
    size_t sz = eps.size();
    jlong arr[sz];
    for(size_t i= 0; i<sz; ++i) arr[i] = (jlong) eps[i];
    ret = env->NewLongArray(sz);
    if(ret == NULL) REPORT_ERROR(INTERNAL_ERROR, "New jlongArray allocation failed.\n");
    env->SetLongArrayRegion(ret, 0, sz, arr);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return ret;
}

// resets threshold value in trained Frovedis glm 
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_setFrovedisGLMThreshold
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid, jshort mkind, jdouble thr) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  try {
     switch(mkind) {
       case LRM:    exrpc_oneway(fm_node,(set_glm_threshold<DT1,LRM1>),mid,thr); break;
       case SVM:    exrpc_oneway(fm_node,(set_glm_threshold<DT1,SVM1>),mid,thr); break;
       case LNRM:
       case MFM:
       case KMEANS: REPORT_ERROR(USER_ERROR,"This model doesn't have any threshold parameter!\n");
       default:     REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}
 

// loads frovedis generic models
JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisModel
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid, jshort mkind, jstring path) {

  auto fs_path = to_cstring(env,path);
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  try {
    switch(mkind) {
      case FMM:  REPORT_ERROR(USER_ERROR,"currently Frovedis fm_model can't be loaded!\n"); 
      case DTM:  exrpc_oneway(fm_node,load_model<DTM1>,mid,DTM,fs_path); break;
      default:   REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

// loads the nbm from the specified file
JNIEXPORT jstring JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisNBM
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid, jshort mkind, jstring path) {

  auto fs_path = to_cstring(env,path);
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  std::string str = "";
  try {
    str = exrpc_async(fm_node,load_nbm<DT1>,mid,NBM,fs_path).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  jstring js = env->NewStringUTF(str.c_str());
  return js;
}

// loads the glm from the specified file 
JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisGLM
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid, jshort mkind, jstring path) {

  auto fs_path = to_cstring(env,path);
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  dummy_glm model;
  try {
    switch(mkind) {
      case LRM:  model = exrpc_async(fm_node,load_glm<LRM1>,mid,LRM,fs_path).get(); break;
      case SVM:  model = exrpc_async(fm_node,load_glm<SVM1>,mid,SVM,fs_path).get(); break;
      case LNRM: model = exrpc_async(fm_node,load_lnrm<DT1>,mid,LNRM,fs_path).get(); break;
      default:   REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
#ifdef _EXRPC_DEBUG_
  std::cout << "model loading completed...printing the loaded glm info: \n";
  model.debug_print();
#endif
  return frovedis_dummyGLM_to_java_dummyGLM(env, model);
}

// loads the mfm from the specified file 
JNIEXPORT jint JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisMFM
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid, jshort mkind, jstring path) {

  auto fs_path = to_cstring(env,path);
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  dummy_mfm ret;
  try {
    ret = exrpc_async(fm_node,load_mfm<DT1>,mid,MFM,fs_path).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return ret.rank;
}

// loads the kmm from the specified file 
JNIEXPORT jint JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadFrovedisKMM
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid, jshort mkind, jstring path) {

  auto fs_path = to_cstring(env,path);
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  int ret = 0; 
  try {
    ret = exrpc_async(fm_node,load_kmm<DT1>,mid,KMEANS,fs_path).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return ret;  
}

// TODO: rename function (without GLM)
// for multiple test inputs: prediction is carried out in parallel in Frovedis worker nodes
JNIEXPORT jdoubleArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_doParallelGLMPredict
  (JNIEnv *env, jclass thisCls, jobject worker_node, jlong mptr, jshort mkind, 
   jlong nrows, jlong ncols, jintArray off, jintArray idx, jdoubleArray val) {

  auto crs_loc = get_frovedis_double_crs_matrix_local(env,nrows,ncols,off,idx,val);
  auto fw_node = java_node_to_frovedis_node(env, worker_node);
  auto mptr_ = (exrpc_ptr_t) mptr;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to worker node (" 
            << fw_node.hostname << "," << fw_node.rpcport 
            << ") to perform multiple prediction in parallel.\n";
#endif
  std::vector<double> pd;
  try {
    switch(mkind) {
      case LRM:  pd = exrpc_async(fw_node,(parallel_glm_predict<DT1,S_LMAT1,LRM1>),crs_loc,mptr_).get(); break;
      case SVM:  pd = exrpc_async(fw_node,(parallel_glm_predict<DT1,S_LMAT1,SVM1>),crs_loc,mptr_).get(); break;
      case LNRM: pd = exrpc_async(fw_node,(parallel_lnrm_predict<DT1,S_LMAT1>),crs_loc,mptr_).get(); break;
      case DTM:  pd = exrpc_async(fw_node,(parallel_dtm_predict<DT1,S_LMAT1>),crs_loc,mptr_).get(); break;
      case FMM:  pd = exrpc_async(fw_node,(parallel_fmm_predict<DT1,S_LMAT1>),crs_loc,mptr_).get(); break;
      case NBM:  pd = exrpc_async(fw_node,(parallel_nbm_predict<DT1,S_LMAT1>),crs_loc,mptr_).get(); break;
      default:   REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jdoubleArray(env, pd);
}

// TODO: rename function (without GLM)
// for single test input: prediction on trained glm is carried out in master node
JNIEXPORT jdouble JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_doSingleGLMPredict
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid, jshort mkind, 
   jlong nrows, jlong ncols, jintArray off, jintArray idx, jdoubleArray val) {

  auto crs_loc = get_frovedis_double_crs_matrix_local(env,nrows,ncols,off,idx,val);
  auto fm_node = java_node_to_frovedis_node(env, master_node);
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node (" 
            << fm_node.hostname << "," << fm_node.rpcport 
            << ") to perform single prediction.\n";
#endif
  double ret = 0.0;
  try {
    switch(mkind) {
      case LRM:  ret = exrpc_async(fm_node,(single_glm_predict<DT1,S_LMAT1,LRM1>),crs_loc,mid).get(); break;
      case SVM:  ret = exrpc_async(fm_node,(single_glm_predict<DT1,S_LMAT1,SVM1>),crs_loc,mid).get(); break;
      case LNRM: ret = exrpc_async(fm_node,(single_lnrm_predict<DT1,S_LMAT1>),crs_loc,mid).get(); break;
      case DTM:  ret = exrpc_async(fm_node,(single_dtm_predict<DT1,S_LMAT1>),crs_loc,mid).get(); break;
      case FMM:  ret = exrpc_async(fm_node,(single_fmm_predict<DT1,S_LMAT1>),crs_loc,mid).get(); break;
      case NBM:  ret = exrpc_async(fm_node,(single_nbm_predict<DT1,S_LMAT1>),crs_loc,mid).get(); break;
      default:   REPORT_ERROR(USER_ERROR,"Unknown Model Kind is encountered!\n");
    } 
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return ret;
}

// Array[uid], Array[pid]: prediction carried out in parallel in worker nodes
JNIEXPORT jdoubleArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_doParallelALSPredict
  (JNIEnv *env, jclass thisCls, jobject worker_node, jlong mptr, jshort mkind, 
   jintArray uids, jintArray pids) {

  auto fw_node = java_node_to_frovedis_node(env, worker_node);
  auto mptr_ = (exrpc_ptr_t) mptr;
  jsize total = env->GetArrayLength(uids);
  jint *uidsp = env->GetIntArrayElements(uids, 0);  
  jint *pidsp = env->GetIntArrayElements(pids, 0); 
  std::vector<std::pair<size_t,size_t>> ids(total);
  for(int i=0; i<total; ++i) ids[i] = std::make_pair(uidsp[i], pidsp[i]);
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to worker node (" 
            << fw_node.hostname << "," << fw_node.rpcport 
            << ") to perform multiple prediction in parallel.\n";
#endif
  std::vector<double> pd;
  try {
    pd = exrpc_async(fw_node,parallel_mfm_predict<DT1>,mptr_,ids).get(); 
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jdoubleArray(env, pd);
}

// (uid, pid): prediction on trained mfm is carried out in master node
JNIEXPORT jdouble JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_doSingleALSPredict
  (JNIEnv *env, jclass thisCls, jobject master_node, 
   jint mid, jshort mkind, jint uid, jint pid) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node (" 
            << fm_node.hostname << "," << fm_node.rpcport 
            << ") to perform single prediction.\n";
#endif
  double ret = 0.0;
  try {
    ret= exrpc_async(fm_node,single_mfm_predict<DT1>,mid,uid,pid).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return ret; 
}

// to recommend products with rating for a given user based on trained mfm
JNIEXPORT jobjectArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_recommendProducts
  (JNIEnv *env, jclass thisCls, jobject master_node, 
   jint mid, jshort mkind, jint uid, jint num) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node (" 
            << fm_node.hostname << "," << fm_node.rpcport 
            << ") to recommend products with rating for user: " << uid << "\n";
#endif
  std::vector<std::pair<size_t,double>> ret;
  try {
    ret = exrpc_async(fm_node,recommend_products<DT1>,mid,uid,num).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jIntDoublePairArray(env, ret); 
}

// to recommend users with rating for a given product based on trained mfm
JNIEXPORT jobjectArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_recommendUsers
  (JNIEnv *env, jclass thisCls, jobject master_node, 
   jint mid, jshort mkind, jint pid, jint num) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node (" 
            << fm_node.hostname << "," << fm_node.rpcport 
            << ") to recommend users with rating for product: " << pid << "\n";
#endif
  std::vector<std::pair<size_t,double>> ret;
  try {
    ret = exrpc_async(fm_node,recommend_users<DT1>,mid,pid,num).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jIntDoublePairArray(env, ret); 
}

// for multiple test inputs: prediction is carried out in parallel in Frovedis worker nodes
JNIEXPORT jintArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_doParallelKMMPredict
  (JNIEnv *env, jclass thisCls, jobject worker_node, jlong mptr, jshort mkind, 
   jlong nrows, jlong ncols, jintArray off, jintArray idx, jdoubleArray val) {

  auto crs_loc = get_frovedis_double_crs_matrix_local(env,nrows,ncols,off,idx,val);
  auto fw_node = java_node_to_frovedis_node(env, worker_node);
  auto mptr_ = (exrpc_ptr_t) mptr;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to worker node (" 
            << fw_node.hostname << "," << fw_node.rpcport 
            << ") to perform multiple prediction in parallel.\n";
#endif
  std::vector<int> pd;
  try {
    pd = exrpc_async(fw_node,(parallel_kmm_predict<S_LMAT1,KMM1>),crs_loc,mptr_).get(); 
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jintArray(env, pd);
}

// for single test input: prediction on trained kmm is carried out in master node
JNIEXPORT jint JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_doSingleKMMPredict
  (JNIEnv *env, jclass thisCls, jobject master_node, jint mid, jshort mkind, 
   jlong nrows, jlong ncols, jintArray off, jintArray idx, jdoubleArray val) {

  auto crs_loc = get_frovedis_double_crs_matrix_local(env,nrows,ncols,off,idx,val);
  auto fm_node = java_node_to_frovedis_node(env, master_node);
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node (" 
            << fm_node.hostname << "," << fm_node.rpcport 
            << ") to perform single prediction.\n";
#endif
  int ret = 0;
  try {
    ret = exrpc_async(fm_node,(single_kmm_predict<S_LMAT1,KMM1>),crs_loc,mid).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return ret;
}

// to compute SVD of a given sparse matrix 
JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_computeSVD
  (JNIEnv *env, jclass thisCls, jobject master_node, 
   jlong fdata, jint k, jboolean movable) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = (exrpc_ptr_t) fdata;
  bool mvbl = (bool) movable;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to compute sparse_svd.\n";
#endif
  gesvd_result res;
  try{
    res = exrpc_async(fm_node,(frovedis_sparse_svd<S_MAT1,DT1>),f_dptr,k,mvbl).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jDummyGesvdResult(env,res,CMJR,true,true);
}

JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getSVDResultFromFiles
  (JNIEnv *env, jclass thisCls, jobject master_node, jshort mtype,
   jstring s_file, jstring u_file, jstring v_file, 
   jboolean wantU, jboolean wantV, jboolean isbinary) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto isU = (bool) wantU;
  auto isV = (bool) wantV;
  auto bin = (bool) isbinary;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to load saved svd results.\n";
#endif
  auto sfl = to_cstring(env,s_file);
  auto ufl = isU ? to_cstring(env,u_file) : "";
  auto vfl = isV ? to_cstring(env,v_file) : "";
  gesvd_result ret;
  try {
    switch(mtype) {
      case CMJR: ret = exrpc_async(fm_node,load_cmm_svd_results<DT1>,sfl,ufl,vfl,isU,isV,bin).get(); break;
      case BCLC: ret = exrpc_async(fm_node,load_bcm_svd_results<DT1>,sfl,ufl,vfl,isU,isV,bin).get(); break;
      default:   REPORT_ERROR(USER_ERROR,"Unknown dense matrix kind is encountered!\n");
    } 
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jDummyGesvdResult(env,ret,mtype,isU,isV); 
}

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_swap
  (JNIEnv *env, jclass thisCls, jobject master_node, jshort mtype, 
   jlong vptr1, jlong vptr2) { 

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr1 = (exrpc_ptr_t) vptr1;
  auto f_dptr2 = (exrpc_ptr_t) vptr2;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to invoke frovedis::swap().\n";
#endif
  try {
    switch(mtype) {
      case CMJR: exrpc_oneway(fm_node,(frovedis_swap<DT1,C_LMAT1>),f_dptr1,f_dptr2); break;
      case BCLC: exrpc_oneway(fm_node,(frovedis_swap<DT1,B_MAT1>),f_dptr1,f_dptr2); break;
      default:   REPORT_ERROR(USER_ERROR,"Unknown input matrix kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_copy
  (JNIEnv *env, jclass thisCls, jobject master_node, jshort mtype,
   jlong vptr1, jlong vptr2) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr1 = (exrpc_ptr_t) vptr1;
  auto f_dptr2 = (exrpc_ptr_t) vptr2;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to invoke frovedis::copy().\n";
#endif
  try {
    switch(mtype) {
      case CMJR: exrpc_oneway(fm_node,(frovedis_copy<DT1,C_LMAT1>),f_dptr1,f_dptr2); break;
      case BCLC: exrpc_oneway(fm_node,(frovedis_copy<DT1,B_MAT1>),f_dptr1,f_dptr2); break;
      default:   REPORT_ERROR(USER_ERROR,"Unknown input matrix kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_scal
  (JNIEnv *env, jclass thisCls, jobject master_node, jshort mtype,
   jlong vptr, jdouble alpha) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = (exrpc_ptr_t) vptr;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to invoke frovedis::scal().\n";
#endif
  try {
    switch(mtype) {
      case CMJR: exrpc_oneway(fm_node,(frovedis_scal<DT1,C_LMAT1>),f_dptr,alpha); break;
      case BCLC: exrpc_oneway(fm_node,(frovedis_scal<DT1,B_MAT1>),f_dptr,alpha); break;
      default:   REPORT_ERROR(USER_ERROR,"Unknown input matrix kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_axpy
  (JNIEnv *env, jclass thisCls, jobject master_node, jshort mtype,
   jlong vptr1, jlong vptr2, jdouble alpha) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr1 = (exrpc_ptr_t) vptr1;
  auto f_dptr2 = (exrpc_ptr_t) vptr2;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to invoke frovedis::axpy().\n";
#endif
  try {
    switch(mtype) {
      case CMJR: exrpc_oneway(fm_node,(frovedis_axpy<DT1,C_LMAT1>),f_dptr1,f_dptr2,alpha); break;
      case BCLC: exrpc_oneway(fm_node,(frovedis_axpy<DT1,B_MAT1>),f_dptr1,f_dptr2,alpha); break;
      default:   REPORT_ERROR(USER_ERROR,"Unknown input matrix kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

JNIEXPORT jdouble JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_dot
  (JNIEnv *env, jclass thisCls, jobject master_node, jshort mtype,
   jlong vptr1, jlong vptr2) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr1 = (exrpc_ptr_t) vptr1;
  auto f_dptr2 = (exrpc_ptr_t) vptr2;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to invoke frovedis::dot().\n";
#endif
  double ret = 0.0;
  try {
    switch(mtype) {
      case CMJR: ret = exrpc_async(fm_node,(frovedis_dot<DT1,C_LMAT1>),f_dptr1,f_dptr2).get(); break;
      case BCLC: ret = exrpc_async(fm_node,(frovedis_dot<DT1,B_MAT1>),f_dptr1,f_dptr2).get(); break;
      default:   REPORT_ERROR(USER_ERROR,"Unknown input matrix kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return ret;
}

JNIEXPORT jdouble JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_nrm2
  (JNIEnv *env, jclass thisCls, jobject master_node, jshort mtype,
   jlong vptr) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = (exrpc_ptr_t) vptr;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to invoke frovedis::nrm2().\n";
#endif
  double ret = 0.0;
  try {
    switch(mtype) {
      case CMJR: ret = exrpc_async(fm_node,(frovedis_nrm2<DT1,C_LMAT1>),f_dptr).get(); break;
      case BCLC: ret = exrpc_async(fm_node,(frovedis_nrm2<DT1,B_MAT1>),f_dptr).get(); break;
      default:   REPORT_ERROR(USER_ERROR,"Unknown input matrix kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return ret;
}

JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_gemv
  (JNIEnv *env, jclass thisCls, jobject master_node, jshort mtype, 
   jlong mptr, jlong vptr, jboolean isTrans, 
   jdouble al, jdouble be) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_mptr = (exrpc_ptr_t) mptr;
  auto f_vptr = (exrpc_ptr_t) vptr;
  bool trans = (bool) isTrans;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to invoke frovedis::gemv().\n";
#endif
  dummy_matrix ret;
  try {
    switch(mtype) {
      case CMJR: ret = exrpc_async(fm_node,(frovedis_gemv<DT1,C_LMAT1>),f_mptr,f_vptr,trans,al,be).get(); break;
      case BCLC: ret = exrpc_async(fm_node,(frovedis_gemv<DT1,B_MAT1>),f_mptr,f_vptr,trans,al,be).get(); break;
      default:   REPORT_ERROR(USER_ERROR,"Unknown input matrix kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jDummyMatrix(env, ret, mtype);
}

JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_ger
  (JNIEnv *env, jclass thisCls, jobject master_node, jshort mtype,
   jlong vptr1, jlong vptr2, jdouble al) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_vptr1 = (exrpc_ptr_t) vptr1;
  auto f_vptr2 = (exrpc_ptr_t) vptr2;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to invoke frovedis::ger().\n";
#endif
  dummy_matrix ret;
  try {
    switch(mtype) {
      case CMJR: ret = exrpc_async(fm_node,(frovedis_ger<DT1,C_LMAT1>),f_vptr1,f_vptr2,al).get(); break;
      case BCLC: ret = exrpc_async(fm_node,(frovedis_ger<DT1,B_MAT1>),f_vptr1,f_vptr2,al).get(); break;
      default:   REPORT_ERROR(USER_ERROR,"Unknown input matrix kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jDummyMatrix(env, ret, mtype);
}

JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_gemm
  (JNIEnv *env, jclass thisCls, jobject master_node, jshort mtype,
   jlong mptr1, jlong mptr2, jboolean trans_m1, jboolean trans_m2, 
   jdouble al, jdouble be) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_mptr1 = (exrpc_ptr_t) mptr1;
  auto f_mptr2 = (exrpc_ptr_t) mptr2;
  auto tm1 = (bool) trans_m1;
  auto tm2 = (bool) trans_m2;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to invoke frovedis::gemm().\n";
#endif
  dummy_matrix ret;
  try {
    switch(mtype) {
      case CMJR: ret = exrpc_async(fm_node,(frovedis_gemm<DT1,C_LMAT1>),f_mptr1,f_mptr2,tm1,tm2,al,be).get(); break;
      case BCLC: ret = exrpc_async(fm_node,(frovedis_gemm<DT1,B_MAT1>),f_mptr1,f_mptr2,tm1,tm2,al,be).get(); break;
      default:   REPORT_ERROR(USER_ERROR,"Unknown input matrix kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jDummyMatrix(env, ret, mtype);
}

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_geadd
  (JNIEnv *env, jclass thisCls, jobject master_node, jshort mtype,
   jlong mptr1, jlong mptr2, jboolean isTrans,
   jdouble al, jdouble be) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_mptr1 = (exrpc_ptr_t) mptr1;
  auto f_mptr2 = (exrpc_ptr_t) mptr2;
  auto trans = (bool) isTrans;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to invoke frovedis::geadd().\n";
#endif
  try {
    exrpc_oneway(fm_node,(frovedis_geadd<DT1,B_MAT1>),f_mptr1,f_mptr2,trans,al,be);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
 }

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_releaseIPIV
  (JNIEnv *env, jclass thisCls, jobject master_node, jshort mtype, 
   jlong dptr) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = (exrpc_ptr_t) dptr;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to release IPIV data pointer.\n";
#endif
  try {
    switch(mtype) {
      case CMJR: exrpc_oneway(fm_node,(release_data<std::vector<int>>),f_dptr); break;
      case BCLC: exrpc_oneway(fm_node,(release_data<lvec<int>>),f_dptr); break;
      default:   REPORT_ERROR(USER_ERROR,"Unknown input matrix kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_releaseDoubleArray
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong dptr) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = (exrpc_ptr_t) dptr;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to release double array pointer.\n";
#endif 
  try {
    exrpc_oneway(fm_node,(release_data<std::vector<double>>),f_dptr);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getrf
  (JNIEnv *env, jclass thisCls, jobject master_node, jshort mtype, 
   jlong mptr) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_mptr = (exrpc_ptr_t) mptr;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to invoke frovedis::getrf().\n";
#endif
  getrf_result ret;
  try {
    switch(mtype) {
      case CMJR: ret = exrpc_async(fm_node,(frovedis_getrf<DT1,C_LMAT1,std::vector<int>>),f_mptr).get(); break;
      case BCLC: ret = exrpc_async(fm_node,(frovedis_getrf<DT1,B_MAT1,lvec<int>>),f_mptr).get(); break;
      default:   REPORT_ERROR(USER_ERROR,"Unknown input matrix kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jDummyGetrfResult(env,ret,mtype);
}

JNIEXPORT jint JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getri
  (JNIEnv *env, jclass thisCls, jobject master_node, jshort mtype,
   jlong mptr, jlong ipivp) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_mptr = (exrpc_ptr_t) mptr;
  auto f_ipivp = (exrpc_ptr_t) ipivp;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to invoke frovedis::getri().\n";
#endif
  int stat = 0;
  try {
    switch(mtype) {
      case CMJR: stat = exrpc_async(fm_node,(frovedis_getri<DT1,C_LMAT1,std::vector<int>>),f_mptr,f_ipivp).get(); break;
      case BCLC: stat = exrpc_async(fm_node,(frovedis_getri<DT1,B_MAT1,lvec<int>>),f_mptr,f_ipivp).get(); break;
      default:   REPORT_ERROR(USER_ERROR,"Unknown input matrix kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return stat;
}

JNIEXPORT jint JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getrs
  (JNIEnv *env, jclass thisCls, jobject master_node, jshort mtype,
   jlong mptrA, jlong mptrB, jlong ipivp, jboolean isTrans) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_mptrA = (exrpc_ptr_t) mptrA;
  auto f_mptrB = (exrpc_ptr_t) mptrB;
  auto f_ipivp = (exrpc_ptr_t) ipivp;
  auto trans = (bool) isTrans;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to invoke frovedis::getrs().\n";
#endif
  int stat = 0;
  try {
    switch(mtype) {
      case CMJR: 
        stat = exrpc_async(fm_node,(frovedis_getrs<DT1,C_LMAT1,std::vector<int>>),f_mptrA,f_mptrB,f_ipivp,trans).get(); 
        break;
      case BCLC: 
        stat = exrpc_async(fm_node,(frovedis_getrs<DT1,B_MAT1,lvec<int>>),f_mptrA,f_mptrB,f_ipivp,trans).get(); 
        break;
      default:   REPORT_ERROR(USER_ERROR,"Unknown input matrix kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return stat;
}

JNIEXPORT jint JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_gesv
  (JNIEnv *env, jclass thisCls, jobject master_node, jshort mtype,
   jlong mptrA, jlong mptrB) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_mptrA = (exrpc_ptr_t) mptrA;
  auto f_mptrB = (exrpc_ptr_t) mptrB;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to invoke frovedis::gesv().\n";
#endif
  int stat = 0;
  try {
    switch(mtype) {
      case CMJR: stat = exrpc_async(fm_node,(frovedis_gesv<DT1,C_LMAT1,std::vector<int>>),f_mptrA,f_mptrB).get(); break;
      case BCLC: stat = exrpc_async(fm_node,(frovedis_gesv<DT1,B_MAT1,lvec<int>>),f_mptrA,f_mptrB).get(); break;
      default:   REPORT_ERROR(USER_ERROR,"Unknown input matrix kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return stat;
}

JNIEXPORT jint JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_gels
  (JNIEnv *env, jclass thisCls, jobject master_node, jshort mtype,
   jlong mptrA, jlong mptrB, jboolean isTrans) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_mptrA = (exrpc_ptr_t) mptrA;
  auto f_mptrB = (exrpc_ptr_t) mptrB;
  auto trans = (bool) isTrans;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to invoke frovedis::gels().\n";
#endif
  int stat = 0;
  try {
    switch(mtype) {
      case CMJR: stat = exrpc_async(fm_node,(frovedis_gels<DT1,C_LMAT1>),f_mptrA,f_mptrB,trans).get(); break;
      case BCLC: stat = exrpc_async(fm_node,(frovedis_gels<DT1,B_MAT1>),f_mptrA,f_mptrB,trans).get(); break;
      default:   REPORT_ERROR(USER_ERROR,"Unknown input matrix kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return stat;
}

JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_gesvd
  (JNIEnv *env, jclass thisCls, jobject master_node, jshort mtype,
   jlong mptr, jboolean wantU, jboolean wantV) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_mptr = (exrpc_ptr_t) mptr;
  auto isU = (bool) wantU;
  auto isV = (bool) wantV;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to invoke frovedis::gesvd().\n";
#endif
  gesvd_result ret;
  try {
    switch(mtype) {
      case CMJR: ret = exrpc_async(fm_node,(frovedis_gesvd<DT1,C_LMAT1>),f_mptr,isU,isV).get(); break;
      case BCLC: ret = exrpc_async(fm_node,(frovedis_gesvd<DT1,B_MAT1>),f_mptr,isU,isV).get(); break;
      default:   REPORT_ERROR(USER_ERROR,"Unknown input matrix kind is encountered!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jDummyGesvdResult(env,ret,mtype,isU,isV); 
}

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

JNIEXPORT jlong JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_createFrovedisDataframe
  (JNIEnv *env, jclass thisCls, jobject master_node, jshortArray dtps,
   jobjectArray cols, jlongArray dvec_proxies, jlong size) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto dtypes = to_short_vector(env, dtps, size);
  auto col_names = to_string_vector(env, cols, size);
  auto dvecps = to_exrpc_vector(env, dvec_proxies, size);
  exrpc_ptr_t df_proxy = 0;
  try {
    df_proxy = exrpc_async(fm_node,create_dataframe,dtypes,col_names,dvecps).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return (jlong) df_proxy;
}

JNIEXPORT jlong JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getDFOperator
  (JNIEnv *env, jclass thisCls, jobject master_node,
   jstring col1, jstring col2, jshort tid, jshort optid, jboolean isImmed) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto t1 = to_cstring(env, col1);
  auto t2 = to_cstring(env, col2);
  auto immed = (bool) isImmed;
  exrpc_ptr_t opt_proxy = 0;
  try {
    switch(tid) {
       case INT:    opt_proxy = exrpc_async(fm_node,get_dfoperator<int>,t1,t2,optid,immed).get(); break;
       case LONG:   opt_proxy = exrpc_async(fm_node,get_dfoperator<long>,t1,t2,optid,immed).get(); break;
       case FLOAT:  opt_proxy = exrpc_async(fm_node,get_dfoperator<float>,t1,t2,optid,immed).get(); break;
       case DOUBLE: opt_proxy = exrpc_async(fm_node,get_dfoperator<double>,t1,t2,optid,immed).get(); break;
       case STRING: opt_proxy = exrpc_async(fm_node,get_str_dfoperator,t1,t2,optid,immed).get(); break;
       case BOOL:   opt_proxy = exrpc_async(fm_node,get_dfoperator<int>,t1,t2,optid,immed).get(); break;
       default:     REPORT_ERROR(USER_ERROR,
                  "Unsupported datatype is encountered in dfoperator creation!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return (jlong) opt_proxy;
}

JNIEXPORT jlong JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getDFAndOperator
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong op1, jlong op2) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto oproxy1 = static_cast<exrpc_ptr_t> (op1);
  auto oproxy2 = static_cast<exrpc_ptr_t> (op2);
  exrpc_ptr_t ret_proxy = 0;
  try {
    ret_proxy = exrpc_async(fm_node,get_dfANDoperator,oproxy1,oproxy2).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return (jlong) ret_proxy;
}

JNIEXPORT jlong JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getDFOrOperator
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong op1, jlong op2) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto oproxy1 = static_cast<exrpc_ptr_t> (op1);
  auto oproxy2 = static_cast<exrpc_ptr_t> (op2);
  exrpc_ptr_t ret_proxy = 0;
  try {
    ret_proxy = exrpc_async(fm_node,get_dfORoperator,oproxy1,oproxy2).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return (jlong) ret_proxy;
}

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_releaseFrovedisDFOperator
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong proxy) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto opt_proxy = static_cast<exrpc_ptr_t> (proxy);
  try {
    exrpc_oneway(fm_node,(release_data<std::shared_ptr<dfoperator>>),opt_proxy);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_releaseFrovedisDataframe
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong proxy) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
  try {
    exrpc_oneway(fm_node,release_data<dftable>,df_proxy);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_showFrovedisDataframe
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong proxy) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
  try {
    exrpc_oneway(fm_node,show_dataframe,df_proxy);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}


JNIEXPORT jlong JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_filterFrovedisDataframe
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong dftbl, jlong opt) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto df_proxy = static_cast<exrpc_ptr_t> (dftbl);
  auto opt_proxy = static_cast<exrpc_ptr_t> (opt);
  exrpc_ptr_t ret_proxy = 0;
  try {
    ret_proxy = exrpc_async(fm_node,filter_df,df_proxy,opt_proxy).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return (jlong) ret_proxy;
}

JNIEXPORT jlong JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_joinFrovedisDataframes
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong dftbl1, jlong dftbl2,
   jlong opt, jstring type, jstring algo) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto df_proxy1 = static_cast<exrpc_ptr_t> (dftbl1);
  auto df_proxy2 = static_cast<exrpc_ptr_t> (dftbl2);
  auto opt_proxy = static_cast<exrpc_ptr_t> (opt);
  auto jtype = to_cstring(env, type);
  auto jalgo = to_cstring(env, algo);
  exrpc_ptr_t ret_proxy = 0;
  try {
    ret_proxy = exrpc_async(fm_node,join_df,df_proxy1,df_proxy2,opt_proxy,jtype,jalgo).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return (jlong) ret_proxy;
}

JNIEXPORT jlong JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_selectFrovedisDataframe
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong dftbl,
   jobjectArray target, jlong size) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto df_proxy = static_cast<exrpc_ptr_t> (dftbl);
  auto cols = to_string_vector(env,target,size);
  exrpc_ptr_t ret_proxy = 0;
  try {
    ret_proxy = exrpc_async(fm_node,select_df,df_proxy,cols).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return (jlong) ret_proxy;
}

JNIEXPORT jlong JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_groupFrovedisDataframe
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong dftbl,
   jobjectArray target, jlong size) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto df_proxy = static_cast<exrpc_ptr_t> (dftbl);
  auto cols = to_string_vector(env,target,size);
  exrpc_ptr_t ret_proxy = 0;
  try {
    ret_proxy = exrpc_async(fm_node,group_by_df,df_proxy,cols).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return (jlong) ret_proxy;
}

JNIEXPORT jlong JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_sortFrovedisDataframe
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong dftbl,
   jobjectArray target, jlong size, jboolean isDesc) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto df_proxy = static_cast<exrpc_ptr_t> (dftbl);
  auto cols = to_string_vector(env,target,size);
  auto desc = (bool) isDesc; 
  exrpc_ptr_t ret_proxy = 0;
  try {
    ret_proxy = exrpc_async(fm_node,sort_df,df_proxy,cols,desc).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return (jlong) ret_proxy;
}

JNIEXPORT jlong JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getFrovedisDFSize
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong dftbl) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto df_proxy = static_cast<exrpc_ptr_t> (dftbl);
  exrpc_ptr_t ret_proxy = 0;
  try {
    ret_proxy = exrpc_async(fm_node,frovedis_df_size,df_proxy).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return (jlong) ret_proxy;
}

JNIEXPORT jobjectArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getFrovedisDFCounts
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong dftbl,
   jobjectArray target, jint size) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto df_proxy = static_cast<exrpc_ptr_t> (dftbl);
  auto cols = to_string_vector(env,target,size);
  std::vector<std::string> ret;
  try {
    ret = exrpc_async(fm_node,frovedis_df_cnt,df_proxy,cols).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jStringArray(env,ret);
}

JNIEXPORT jobjectArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getFrovedisDFMeans
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong dftbl,
   jobjectArray target, jint size) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto df_proxy = static_cast<exrpc_ptr_t> (dftbl);
  auto cols = to_string_vector(env,target,size);
  std::vector<std::string> ret;
  try {
    ret = exrpc_async(fm_node,frovedis_df_avg,df_proxy,cols).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jStringArray(env,ret);
}

JNIEXPORT jobjectArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getFrovedisDFTotals
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong dftbl,
   jobjectArray target, jshortArray tid, jint size) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto df_proxy = static_cast<exrpc_ptr_t> (dftbl);
  auto cols = to_string_vector(env,target,size);
  auto tids = to_short_vector(env,tid,size);
  std::vector<std::string> ret;
  try {
    ret = exrpc_async(fm_node,frovedis_df_sum,df_proxy,cols,tids).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jStringArray(env,ret);
}

JNIEXPORT jobjectArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getFrovedisDFMins
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong dftbl,
   jobjectArray target, jshortArray tid, jint size) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto df_proxy = static_cast<exrpc_ptr_t> (dftbl);
  auto cols = to_string_vector(env,target,size);
  auto tids = to_short_vector(env,tid,size);
  std::vector<std::string> ret;
  try {
    ret = exrpc_async(fm_node,frovedis_df_min,df_proxy,cols,tids).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jStringArray(env,ret);
}

JNIEXPORT jobjectArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getFrovedisDFMaxs
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong dftbl,
   jobjectArray target, jshortArray tid, jint size) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto df_proxy = static_cast<exrpc_ptr_t> (dftbl);
  auto cols = to_string_vector(env,target,size);
  auto tids = to_short_vector(env,tid,size);
  std::vector<std::string> ret;
  try {
     ret = exrpc_async(fm_node,frovedis_df_max,df_proxy,cols,tids).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jStringArray(env,ret);
}

JNIEXPORT jobjectArray JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getFrovedisDFStds
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong dftbl,
   jobjectArray target, jshortArray tid, jint size) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto df_proxy = static_cast<exrpc_ptr_t> (dftbl);
  auto cols = to_string_vector(env,target,size);
  auto tids = to_short_vector(env,tid,size);
  std::vector<std::string> ret;
  try {
    ret = exrpc_async(fm_node,frovedis_df_std,df_proxy,cols,tids).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jStringArray(env,ret);
}

JNIEXPORT jlong JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_renameFrovedisDataframe
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong dftbl,
   jobjectArray names, jobjectArray new_names, jint size) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto df_proxy = static_cast<exrpc_ptr_t> (dftbl);
  auto cols = to_string_vector(env,names,size);
  auto new_cols = to_string_vector(env,new_names,size);
  exrpc_ptr_t ret = 0;
  try {
    ret = exrpc_async(fm_node,frovedis_df_rename,df_proxy,cols,new_cols).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return (jlong) ret;
}

}
