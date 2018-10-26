// ----------------------------------------------------------------------------
// NOTE1: This file uses some JAVA package PATHs: 
// e.g., com/nec/frovedis/Jexrpc/Node, com/nec/frovedis/Jexrpc/MemPair etc.
// If you need any changes in JAVA package PATHs, kindly perform 
// necessary changes in "jre_config.hpp" and rebuild the library.
// ----------------------------------------------------------------------------

#include "spark_client_headers.hpp"

using namespace frovedis;

extern "C" {

// converting jstring to std::string
std::string to_cstring(JNIEnv *env, jstring s) {
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

jobject to_jDummyPCAResult(JNIEnv *env, pca_result& obj, short mtype) {
  jclass pcaCls = env->FindClass(JRE_PATH_DummyPCAResult);
  if (pcaCls == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyPCAResult class not found in JRE\n");
  jmethodID pcaConst = env->GetMethodID(pcaCls, "<init>", "(JIISJI)V");
  if (pcaConst == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyPCAResult(JIISJI) not found in JRE\n");
  long pcmatp = static_cast<long>(obj.pc_ptr);
  long varp = static_cast<long>(obj.var_ptr);
  auto newPCA = env->NewObject(pcaCls, pcaConst, 
                               pcmatp, obj.nrows, obj.ncols, mtype,
                               varp, obj.k);
  if (newPCA == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyPCAResult object creation failed\n");
  return newPCA;
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

}
