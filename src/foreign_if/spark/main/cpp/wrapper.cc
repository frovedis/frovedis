#include "exrpc_svd.hpp"
#include "exrpc_pca.hpp"
#include "exrpc_pblas.hpp"
#include "exrpc_scalapack.hpp"
#include "short_hand_dense_type.hpp"
#include "short_hand_sparse_type.hpp"
#include "spark_client_headers.hpp"

using namespace frovedis;

extern "C" {

// to compute SVD of a given sparse matrix 
JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_computeSVD
  (JNIEnv *env, jclass thisCls, jobject master_node, 
   jlong fdata, jint k, jboolean isDense, jboolean movable) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = (exrpc_ptr_t) fdata;
  bool mvbl = (bool) movable;
  bool dense = (bool) isDense;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to compute truncated svd.\n";
#endif
  gesvd_result res;
  bool rearrange_out = true;
  try{
    if(dense){
      res = exrpc_async(fm_node,(frovedis_dense_truncated_svd<R_MAT1,DT1>),f_dptr,k,mvbl,rearrange_out).get();
    }
    else {
      res = exrpc_async(fm_node,(frovedis_sparse_truncated_svd<S_MAT1,DT1>),f_dptr,k,mvbl,rearrange_out).get();
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jDummyGesvdResult(env,res,CMJR,true,true);
}

// to compute PCA of a given dense matrix (rowmajor)
JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_computePCA
  (JNIEnv *env, jclass thisCls, jobject master_node,
   jlong fdata, jint k, jboolean movable) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = (exrpc_ptr_t) fdata;
  bool mvbl = (bool) movable;
#ifdef _EXRPC_DEBUG_
  std::cout << "Connecting to master node ("
            << fm_node.hostname << "," << fm_node.rpcport
            << ") to compute PCA.\n";
#endif
  pca_result res;
  bool rearrange_out = true;
  try{
    res = exrpc_async(fm_node,(frovedis_pca<R_MAT1,DT1>),f_dptr,k,mvbl,rearrange_out).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jDummyPCAResult(env,res,CMJR);
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
    exrpc_oneway(fm_node,(release_data<std::vector<double>>),f_dptr); // SVAL
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

} 
