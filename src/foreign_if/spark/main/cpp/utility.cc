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
  auto ret = std::string(cStr);
  env->ReleaseStringUTFChars(s,cStr);
  return ret;
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
  jmethodID dglmConst = env->GetMethodID(dglmCls, "<init>", "(ISJID)V");
  if (dglmConst == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyGLM::DummyGLM(I,S,J,I,D) not found in JRE\n");
  long nftr = static_cast<long>(obj.numFeatures);
  auto newDummyGLM = env->NewObject(dglmCls, dglmConst,
                                    obj.mid, obj.mkind, 
                                    nftr, obj.numClasses,
                                    obj.threshold);
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

jobject to_jDummyGraph(JNIEnv *env, dummy_graph& obj) {
  jclass dgrCls = env->FindClass(JRE_PATH_DummyGraph);
  if (dgrCls == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyGraph class not found in JRE\n");
  jmethodID dgrConst = env->GetMethodID(dgrCls, "<init>", "(JJJ)V");
  if (dgrConst == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyGraph(J,J,J) not found in JRE\n");
  long dptr = static_cast<long>(obj.dptr);
  long nEdges = static_cast<long>(obj.num_edges);
  long nNodes = static_cast<long>(obj.num_nodes);
  auto newDummyGr = env->NewObject(dgrCls,dgrConst,dptr,nEdges,nNodes);
  if (newDummyGr == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyGraph object creation failed\n");
  return newDummyGr;
}


jobject to_jDummyGetrfResult(JNIEnv *env, lu_fact_result& obj, short mtype) {
  jclass rfCls = env->FindClass(JRE_PATH_DummyGetrfResult);
  if (rfCls == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyGetrfResult class not found in JRE\n");
  jmethodID rfConst = env->GetMethodID(rfCls, "<init>", "(JIS)V");
  if (rfConst == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyGetrfResult(J,I,S) not found in JRE\n");
  long ipivp = static_cast<long>(obj.ipiv_ptr);
  auto newRF = env->NewObject(rfCls,rfConst,ipivp,obj.info,mtype);
  if (newRF == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyGetrfResult object creation failed\n");
  return newRF;
}

jobject to_jDummyGesvdResult(JNIEnv *env, svd_result& obj, short mtype,
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

jobject to_jDummyEvdResult(JNIEnv *env, eigen_result& obj) {
  jclass evdCls = env->FindClass(JRE_PATH_DummyEvdResult);
  if (evdCls == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyEvdResult class not found in JRE\n");
  jmethodID evdConst = env->GetMethodID(evdCls, "<init>", "(JJIII)V");
  if (evdConst == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyEvdResult(JJIII) not found in JRE\n");
  long svecp = static_cast<long>(obj.svec_ptr);
  long vmatp = static_cast<long>(obj.umat_ptr);
  auto newEVD = env->NewObject(evdCls, evdConst, svecp, vmatp,
                               obj.m, obj.n, obj.k);
  if (newEVD == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyEvdResult object creation failed\n");
  return newEVD;
}

jobject to_jDummyPCAResult(JNIEnv *env, pca_result& obj, short mtype) {
  jclass pcaCls = env->FindClass(JRE_PATH_DummyPCAResult);
  if (pcaCls == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyPCAResult class not found in JRE\n");
  jmethodID pcaConst = env->GetMethodID(pcaCls, "<init>", "(JIISJI)V");
  if (pcaConst == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyPCAResult(JIISJI) not found in JRE\n");
  long pcmatp = static_cast<long>(obj.comp_ptr);
  long varp = static_cast<long>(obj.var_ratio_ptr);
  auto newPCA = env->NewObject(pcaCls, pcaConst, 
                               pcmatp, obj.n_features, obj.n_components, 
                               mtype,  varp, obj.n_components);
  if (newPCA == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyPCAResult object creation failed\n");
  return newPCA;
}

jobject to_jDummyTSNEResult(JNIEnv *env, tsne_result& obj) {
  jclass tsneCls = env->FindClass(JRE_PATH_DummyTSNEResult);
  if (tsneCls == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyTSNEResult class not found in JRE\n");
  jmethodID tsneConst = env->GetMethodID(tsneCls, "<init>", "(JIIID)V");
  if (tsneConst == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyTSNEResult(JIIID) not found in JRE\n");
  long embedmatp = static_cast<long>(obj.embedding_ptr);
  auto newTSNE = env->NewObject(tsneCls, tsneConst,
                                embedmatp, obj.n_samples, obj.n_comps,
                                obj.n_iter_, obj.kl_divergence_);
  if (newTSNE == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyTSNEResult object creation failed\n");
  return newTSNE;
}

jobject to_jDummyKNNResult(JNIEnv *env, knn_result& obj) {
  jclass knnCls = env->FindClass(JRE_PATH_DummyKNNResult);
  if (knnCls == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyKNNResult class not found in JRE\n");
  jmethodID knnConst = env->GetMethodID(knnCls, "<init>", "(JJIIIII)V");
  if (knnConst == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyKNNResult(JJIIIII) not found in JRE\n");
  long indices_ptr = static_cast<long>(obj.indices_ptr); // rowmajor_matrix
  long distances_ptr = static_cast<long>(obj.distances_ptr); // rowmajor_matrix
  auto newKNN = env->NewObject(knnCls, knnConst,
                               indices_ptr, distances_ptr, 
                               obj.nrow_ind, obj.ncol_ind,
                               obj.nrow_dist, obj.ncol_dist, obj.k);
  if (newKNN == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyKNNResult object creation failed\n");
  return newKNN;
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

// conversion std::vector<frovedis_mem_pair> => jobjectArray(MemPair)
jobjectArray to_jMemPairArray(JNIEnv *env,
                              std::vector<frovedis_mem_pair>& pd) {
  jclass mpCls = env->FindClass(JRE_PATH_MemPair);
  if (mpCls == NULL) REPORT_ERROR(INTERNAL_ERROR, "MemPair class not found in JRE\n");
  jmethodID mpConst = env->GetMethodID(mpCls, "<init>", "(JJ)V");
  if (mpConst == NULL) REPORT_ERROR(INTERNAL_ERROR, "MemPair::MemPair(J,J) not found in JRE\n");
  jobjectArray ret = env->NewObjectArray(pd.size(), mpCls, NULL);
  if(ret == NULL) REPORT_ERROR(INTERNAL_ERROR, "New jMemPairArray allocation failed.\n");
  for(size_t i = 0; i < pd.size(); ++i) {
    auto mp = pd[i];
    jlong f = (jlong) mp.first();
    jlong s = (jlong) mp.second();
    auto newmempair = env->NewObject(mpCls, mpConst, f, s);
    if (newmempair == NULL) REPORT_ERROR(INTERNAL_ERROR, "MemPair object creation failed\n");
    env->SetObjectArrayElement(ret, i, newmempair);
  }
  return ret;
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
  env->ReleaseIntArrayElements(off,offp,JNI_ABORT); 
  env->ReleaseIntArrayElements(idx,idxp,JNI_ABORT); 
  env->ReleaseDoubleArrayElements(val,valp,JNI_ABORT); 
  return l_mat;
}

// conversion std::vector<exrpc_ptr_t> => jlongArray
jlongArray to_jlongArray(JNIEnv *env, std::vector<exrpc_ptr_t>& eps) {
  size_t sz = eps.size();
  std::vector<long> l_eps(sz);
  for(size_t i=0; i<sz; ++i) l_eps[i] = static_cast<long>(eps[i]);
  jlong* arr = &l_eps[0];
  jlongArray ret = env->NewLongArray(sz);
  if(ret == NULL) REPORT_ERROR(INTERNAL_ERROR, "New jlongArray allocation failed.\n");
  env->SetLongArrayRegion(ret, 0, sz, arr);
  return ret;
}

// conversion std::vector<long> => jlongArray
jlongArray to_jlongArray2(JNIEnv *env, std::vector<long>& eps) {
  size_t sz = eps.size();
  jlong* arr = &eps[0];
  jlongArray ret = env->NewLongArray(sz);
  if(ret == NULL) REPORT_ERROR(INTERNAL_ERROR, "New jlongArray allocation failed.\n");
  env->SetLongArrayRegion(ret, 0, sz, arr);
  return ret;
}

// conversion std::vector<size_t> => jlongArray
jlongArray to_jlongArray3(JNIEnv *env, std::vector<size_t>& eps) {
  size_t sz = eps.size();
  std::vector<long> eps_l(sz);
  for(size_t i=0; i<sz; ++i) eps_l[i] = static_cast<long>(eps[i]);
  jlong* arr = &eps_l[0];
  jlongArray ret = env->NewLongArray(sz);
  if(ret == NULL) REPORT_ERROR(INTERNAL_ERROR, "New jlongArray allocation failed.\n");
  env->SetLongArrayRegion(ret, 0, sz, arr);
  return ret;
}

// conversion std::vector<int> => jshortArray
jshortArray to_jshortArray2(JNIEnv *env, std::vector<int>& eps) {
  size_t sz = eps.size();
  std::vector<short> eps_s(sz);
  for(size_t i=0; i<sz; ++i) eps_s[i] = static_cast<short>(eps[i]);
  jshort* arr = &eps_s[0];
  jshortArray ret = env->NewShortArray(sz);
  if(ret == NULL) REPORT_ERROR(INTERNAL_ERROR, "New jshortArray allocation failed.\n");
  env->SetShortArrayRegion(ret, 0, sz, arr);
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

// conversion std::vector<float> => jfloatArray
jfloatArray to_jfloatArray(JNIEnv *env, std::vector<float>& pd) {
  jfloat* arr = &pd[0];
  size_t sz = pd.size();
  jfloatArray ret = env->NewFloatArray(sz);
  if(ret == NULL) REPORT_ERROR(INTERNAL_ERROR, "New jfloatArray allocation failed.\n");
  env->SetFloatArrayRegion(ret, 0, sz, arr);
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
  for(size_t i = 0; i < d_len; ++i) { 
    jstring js = (jstring) (env->GetObjectArrayElement(data,i));
    const char *rawstr = env->GetStringUTFChars(js, 0);
    std::string str(rawstr);
    data_vec[i] = str;
    env->ReleaseStringUTFChars(js,rawstr);
  }
  return data_vec;
}

std::vector<std::string>
charArray_to_string_vector(JNIEnv *env, jobjectArray& data, size_t size) {
  jsize d_len = env->GetArrayLength(data);
  if(d_len != size) REPORT_ERROR(INTERNAL_ERROR, "Error in data extraction from JRE");
  std::vector<std::string> data_vec(d_len);
  for(size_t i = 0; i < d_len; ++i) {
    jcharArray jcarr = (jcharArray) (env->GetObjectArrayElement(data,i));
    jsize str_size = env->GetArrayLength(jcarr);
    jchar *valp = env->GetCharArrayElements(jcarr, 0); 
    char rawstr[str_size]; 
    for(size_t j = 0; j < str_size; ++j) rawstr[j] = valp[j];
    data_vec[i] = std::string(rawstr, str_size);
    env->ReleaseCharArrayElements(jcarr, valp, JNI_ABORT); 
  }
  return data_vec;
}

std::vector<std::string>
flat_charArray_to_string_vector(JNIEnv *env, jcharArray& data, jintArray& sizes, 
                                size_t flat_size, size_t actual_size) {
  jsize d_len = env->GetArrayLength(data);
  if(d_len != flat_size) REPORT_ERROR(INTERNAL_ERROR, "Error in data extraction from JRE");
  jchar *datap = env->GetCharArrayElements(data, 0);  
  jint *sizesp = env->GetIntArrayElements(sizes, 0); int* sizesp_ = sizesp;
  std::vector<std::string> data_vec(actual_size);
  size_t k = 0;
  for(size_t i = 0; i < actual_size; ++i) {
    auto str_size = sizesp_[i];
    char rawstr[str_size]; 
    for(size_t j = 0; j < str_size; ++j) rawstr[j] = datap[k + j];
    data_vec[i] = std::string(rawstr, str_size); // TODO: send chars, size to server and create strings there
    k += str_size;
  }
  env->ReleaseCharArrayElements(data, datap, JNI_ABORT);
  env->ReleaseIntArrayElements(sizes, sizesp, JNI_ABORT);
  return data_vec;
}

std::vector<int> flat_charArray_to_int_vector(JNIEnv *env, jcharArray& data, size_t size) {
  jsize d_len = env->GetArrayLength(data);
  if(d_len != size) REPORT_ERROR(INTERNAL_ERROR, "Error in data extraction from JRE");
  jchar *datap = env->GetCharArrayElements(data, 0);
  std::vector<int> ret(size); auto retp = ret.data();
  for(size_t i = 0; i < size; ++i) retp[i] = datap[i]; 
  env->ReleaseCharArrayElements(data, datap, JNI_ABORT);
  return ret;
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

// conversion jcharArray => std::vector<char>
std::vector<char> to_char_vector(JNIEnv *env, jcharArray& data, size_t size) {
  jsize d_len = env->GetArrayLength(data);
  if(d_len != size) REPORT_ERROR(INTERNAL_ERROR, "Error in data extraction from JRE");
  jchar *datap = env->GetCharArrayElements(data, 0);
  std::vector<char> data_vec(d_len);
  for(size_t i=0; i<d_len; ++i) data_vec[i] = datap[i];
  env->ReleaseCharArrayElements(data,datap,JNI_ABORT);
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
  for(size_t i = 0; i < d_len; ++i) data_vec[i] = (datap[i] == 1);
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

jobject to_java_DummyEdge (JNIEnv *env, size_t srcID, size_t dstID, double attr) {
  jclass edgeCls = env->FindClass(JRE_PATH_DummyEdge);
  if (edgeCls == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyEdge class not found in JRE\n");
  jmethodID dummyEdgeConst = env->GetMethodID(edgeCls, "<init>", "(JJD)V");
  if (dummyEdgeConst == NULL) 
    REPORT_ERROR(INTERNAL_ERROR, "DummyEdge::DummyEdge(J,J,D) not found in JRE\n");
  auto new_edge = env->NewObject(edgeCls, dummyEdgeConst, (long)srcID, (long)dstID, attr);
  if (new_edge == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyEdge object creation failed\n");
  return new_edge;
}

// conversion of graph adjacency matrix => jobjectArray(DummyEdge)
jobjectArray to_jDummyEdgeArray(JNIEnv *env, crs_matrix_local<double>& mat) {
  auto nrow = mat.local_num_row;
  jclass edgeCls = env->FindClass(JRE_PATH_DummyEdge);
  if (edgeCls == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyEdge class not found in JRE\n");
  jobjectArray ret = env->NewObjectArray(mat.val.size(), edgeCls, NULL);
  if (ret == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyEdge Array creation failed!\n");
  for(size_t i = 0; i < nrow; ++i) {
    for(size_t j = mat.off[i]; j < mat.off[i+1]; ++j) {
      auto srcID = mat.idx[j] + 1;
      auto dstID = i + 1;
      auto attr = mat.val[j];
      auto edge = to_java_DummyEdge(env,srcID,dstID,attr);
      env->SetObjectArrayElement(ret, j, edge);
    }
  }
  return ret;
}

jobject to_jDummyLDAResult(JNIEnv *env, dummy_lda_result& obj) {
  jclass ldaCls = env->FindClass(JRE_PATH_DummyLDAResult);
  if (ldaCls == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyLDAResult class not found in JRE\n");
  jmethodID ldaConst = env->GetMethodID(ldaCls, "<init>", "(JJJDD)V");
  if (ldaConst == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyLDAResult(J,J,J,D,D) not found in JRE\n");
  long mptr = static_cast<long>(obj.dist_mat.mptr);
  long nrow = static_cast<long>(obj.dist_mat.nrow);
  long ncol = static_cast<long>(obj.dist_mat.ncol);
  double perplexity = static_cast<double>(obj.perplexity);
  double likelihood = static_cast<double>(obj.likelihood);
  auto newDummyMat = env->NewObject(ldaCls,ldaConst,mptr,nrow,ncol,perplexity,likelihood);
  if (newDummyMat == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyLDAResult object creation failed\n");
  return newDummyMat;
}

jobject to_jDummyLDAModel(JNIEnv *env, dummy_lda_model& obj) {
  jclass ldaCls = env->FindClass(JRE_PATH_DummyLDAModel);
  if (ldaCls == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyLDAModel class not found in JRE\n");
  jmethodID ldaConst = env->GetMethodID(ldaCls, "<init>", "(III)V");
  if (ldaConst == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyLDAModel(I,I,I) not found in JRE\n");
  int num_topics = static_cast<int>(obj.num_topics);
  int vocabsz = static_cast<int>(obj.vocabsz);
  int ndocs = static_cast<int>(obj.num_docs);
  auto newDummyModel = env->NewObject(ldaCls,ldaConst,num_topics,vocabsz,ndocs);
  if (newDummyModel == NULL) REPORT_ERROR(INTERNAL_ERROR, "DummyLDAModel object creation failed\n");
  return newDummyModel;
}

jobject to_jBFS_Result (JNIEnv *env, 
  bfs_result<size_t>& result, long source_vertex) {
  jclass bfsCls = env->FindClass(JRE_PATH_bfs_result);
  if (bfsCls == NULL) REPORT_ERROR(INTERNAL_ERROR, "bfs_result class not found in JRE\n");
  jmethodID bfsConst = env->GetMethodID(bfsCls, "<init>", "([J[J[JJ)V");
  if (bfsConst == NULL) REPORT_ERROR(INTERNAL_ERROR, "bfs_result constructor not found in JRE\n");
  jlongArray destids = to_jlongArray3(env, result.destids);
  jlongArray distances = to_jlongArray3(env, result.distances);
  jlongArray predecessors = to_jlongArray3(env, result.predecessors);
  auto ret = env->NewObject(bfsCls, bfsConst, destids, distances, predecessors, source_vertex);
  if (ret == NULL) REPORT_ERROR(INTERNAL_ERROR, "bfs_result object creation failed\n");
  return ret;
}

jobject to_jSSSP_Result (JNIEnv *env,
  sssp_result<double,size_t>& result, long source_vertex) {
  jclass ssspCls = env->FindClass(JRE_PATH_sssp_result);
  if (ssspCls == NULL) REPORT_ERROR(INTERNAL_ERROR, "sssp_result class not found in JRE\n");
  jmethodID ssspConst = env->GetMethodID(ssspCls, "<init>", "([J[D[JJ)V");
  if (ssspConst == NULL) REPORT_ERROR(INTERNAL_ERROR, "sssp_result constructor not found in JRE\n");
  jlongArray destids = to_jlongArray3(env, result.destids);
  jdoubleArray distances = to_jdoubleArray(env, result.distances);
  jlongArray predecessors = to_jlongArray3(env, result.predecessors);
  auto ret = env->NewObject(ssspCls, ssspConst, destids, distances, predecessors, source_vertex);
  if (ret == NULL) REPORT_ERROR(INTERNAL_ERROR, "sssp_result object creation failed\n");
  return ret;
}

jobject to_spark_dummy_df (JNIEnv *env, dummy_dftable& obj) {
  jclass dummy_df = env->FindClass(JRE_PATH_DummyDftable);
  if (dummy_df == NULL) REPORT_ERROR(INTERNAL_ERROR, "dummy_dftable class not found in JRE\n");
  jmethodID dfConst = env->GetMethodID(dummy_df, "<init>", "(J[Ljava/lang/String;[S)V");
  if (dfConst == NULL) REPORT_ERROR(INTERNAL_ERROR, "dummy_df constructor not found in JRE\n");
  jlong dfptr = obj.dfptr;
  jobjectArray names = to_jStringArray(env, obj.names);
  jshortArray types = to_jshortArray2(env, obj.types);
  auto ret = env->NewObject(dummy_df, dfConst, dfptr, names, types);
  if (ret == NULL) REPORT_ERROR(INTERNAL_ERROR, "sssp_result object creation failed\n");
  return ret;
}

}
