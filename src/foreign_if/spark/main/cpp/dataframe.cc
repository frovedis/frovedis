#include "exrpc_data_storage.hpp"
#include "exrpc_dataframe.hpp"
#include "spark_client_headers.hpp"

using namespace frovedis;

extern "C" {

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

JNIEXPORT jlong JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getDFNotOperator
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong op) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto oproxy = static_cast<exrpc_ptr_t> (op);
  exrpc_ptr_t ret_proxy = 0;
  try {
    ret_proxy = exrpc_async(fm_node,get_dfNOToperator,oproxy).get();
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

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_releaseFrovedisGroupedDF
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong proxy) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
  try {
    exrpc_oneway(fm_node,release_data<grouped_dftable>,df_proxy);
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
   jobjectArray targets, jintArray isDescArr, jlong size) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto df_proxy = static_cast<exrpc_ptr_t> (dftbl);
  auto cols = to_string_vector(env,targets,size);
  auto desc = to_int_vector(env,isDescArr,size);
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
   jobjectArray target, jint size) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto df_proxy = static_cast<exrpc_ptr_t> (dftbl);
  auto cols = to_string_vector(env,target,size);
  std::vector<std::string> ret;
  try {
    ret = exrpc_async(fm_node,frovedis_df_std,df_proxy,cols).get();
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
  bool inplace = false; // spark withColumnRenamed is not inplace operation
  exrpc_ptr_t ret = 0;
  try {
    ret = exrpc_async(fm_node, frovedis_df_rename, df_proxy, 
                      cols, new_cols, inplace).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return (jlong) ret;
}

JNIEXPORT jlong JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_selectFrovedisGroupedData
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong gdftable, 
   jobjectArray cols, jint sz) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto proxy = static_cast<exrpc_ptr_t> (gdftable);
  auto tcols = to_string_vector(env, cols, sz);
  exrpc_ptr_t ret = 0;
  try {
    ret = exrpc_async(fm_node,frovedis_gdf_select,proxy,tcols).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return (jlong) ret;
}

JNIEXPORT jlong JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_aggrFrovedisDataframe
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong gdftable, 
   jobjectArray gCols, jint sz1, jobjectArray aFuncs, 
   jobjectArray aCols, jobjectArray aAsCols, jint sz2) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto proxy = static_cast<exrpc_ptr_t> (gdftable);
  auto groupCols = to_string_vector(env,gCols,sz1);
  auto aggFuncs = to_string_vector(env,aFuncs,sz2);
  auto aggCols = to_string_vector(env,aCols,sz2);
  auto aggAsCols = to_string_vector(env,aAsCols,sz2);
  exrpc_ptr_t ret = 0;
  try {
    ret = exrpc_async(fm_node,frovedis_gdf_aggr,proxy,groupCols,aggFuncs,aggCols,aggAsCols).get();
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return (jlong) ret;
}

JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_DFToRowmajorMatrix
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong dftbl,
   jobjectArray target, jint size) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto df_proxy = static_cast<exrpc_ptr_t> (dftbl);
  auto cols = to_string_vector(env,target,size);
  dummy_matrix ret;
  try {
    ret = exrpc_async(fm_node,df_to_rowmajor_double,df_proxy,cols).get(); // spark supports double data only
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jDummyMatrix(env,ret,RMJR);
}

JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_DFToColmajorMatrix
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong dftbl,
   jobjectArray target, jint size) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto df_proxy = static_cast<exrpc_ptr_t> (dftbl);
  auto cols = to_string_vector(env,target,size);
  dummy_matrix ret;
  try {
    ret = exrpc_async(fm_node,df_to_colmajor_double,df_proxy,cols).get(); // spark supports double data only
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jDummyMatrix(env,ret,CMJR);
}

JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_DFToCRSMatrix
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong dftbl,
   jobjectArray target1, jint size1,
   jobjectArray target2, jint size2,
   jlong info_id) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto df_proxy = static_cast<exrpc_ptr_t> (dftbl);
  auto cols = to_string_vector(env,target1,size1);
  auto cat_cols = to_string_vector(env,target2,size2);
  dummy_matrix ret;
  try {
    ret = exrpc_async(fm_node,df_to_crs_double,df_proxy,cols,cat_cols,info_id).get(); // spark supports double data only
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jDummyMatrix(env,ret,SCRS);
}

JNIEXPORT jobject JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_DFToCRSMatrixUsingInfo
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong dftbl,
   jlong info_id) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto df_proxy = static_cast<exrpc_ptr_t> (dftbl);
  dummy_matrix ret;
  try {
    ret = exrpc_async(fm_node,df_to_crs_double_using_info,df_proxy,info_id).get(); // spark supports double data only
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return to_jDummyMatrix(env,ret,SCRS);
}

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_loadSparseConversionInfo
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong info_id, 
   jstring dirname) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto fs_path = to_cstring(env,dirname);
  try {
    exrpc_oneway(fm_node,load_sparse_conversion_info,info_id,fs_path);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_saveSparseConversionInfo
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong info_id, 
   jstring dirname) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto fs_path = to_cstring(env,dirname);
  try {
    exrpc_oneway(fm_node,save_sparse_conversion_info,info_id,fs_path);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_releaseSparseConversionInfo
  (JNIEnv *env, jclass thisCls, jobject master_node, jlong info_id) {

  auto fm_node = java_node_to_frovedis_node(env, master_node);
  try {
    exrpc_oneway(fm_node,release_sparse_conversion_info,info_id);
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

JNIEXPORT jlong JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getCrossDfopt
                     (JNIEnv *env, jclass thisCls,
                      jobject master_node) {
                       
  auto fm_node = java_node_to_frovedis_node(env, master_node);
 
  exrpc_ptr_t ret_proxy = 0;
  try {
    ret_proxy = exrpc_async0(fm_node,frov_cross_join_dfopt).get();
  }
  catch (std::exception& e) {
    set_status(true, e.what());
  }
  return (jlong) ret_proxy;
}

JNIEXPORT jlong JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_getDFColumnPointer
  (JNIEnv *env, jclass thisCls, jobject master_node, 
   jlong df_proxy, jstring col_name, jshort tid) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto df = (exrpc_ptr_t) df_proxy;
  auto cname = to_cstring(env, col_name);
  exrpc_ptr_t ret = 0;
  try {
    switch(tid) {
      case INT:    ret = exrpc_async(fm_node, get_df_column_pointer<int>, df, cname).get(); break;
      case LONG:   ret = exrpc_async(fm_node, get_df_column_pointer<long>, df, cname).get(); break;
      case FLOAT:  ret = exrpc_async(fm_node, get_df_column_pointer<float>, df, cname).get(); break;
      case DOUBLE: ret = exrpc_async(fm_node, get_df_column_pointer<double>, df, cname).get(); break;
      case STRING: ret = exrpc_async(fm_node, get_df_column_pointer<std::string>, df, cname).get(); break;
      case BOOL:   ret = exrpc_async(fm_node, get_df_column_pointer<int>, df, cname).get(); break;
      default:     REPORT_ERROR(USER_ERROR, 
                   "Unsupported datatype is encountered in df column extraction!\n");
    }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
  return (jlong) ret;
}

JNIEXPORT void JNICALL Java_com_nec_frovedis_Jexrpc_JNISupport_releaseDFColumnPointer
  (JNIEnv *env, jclass thisCls, jobject master_node, 
   jlong dptr, jshort tid) {
  auto fm_node = java_node_to_frovedis_node(env, master_node);
  auto f_dptr = (exrpc_ptr_t) dptr;
  try {
     switch(tid) {
       case INT:    exrpc_oneway(fm_node, release_dvector<int>, f_dptr); break;
       case LONG:   exrpc_oneway(fm_node, release_dvector<long>, f_dptr); break;
       case FLOAT:  exrpc_oneway(fm_node, release_dvector<float>, f_dptr); break;
       case DOUBLE: exrpc_oneway(fm_node, release_dvector<double>, f_dptr); break;
       case STRING: exrpc_oneway(fm_node, release_dvector<std::string>, f_dptr); break;
       case BOOL:   exrpc_oneway(fm_node, release_dvector<int>, f_dptr); break;
       default:     REPORT_ERROR(USER_ERROR, 
                    "Unsupported datatype is encountered in df column destruction!\n");
     }
  }
  catch(std::exception& e) { set_status(true,e.what()); }
}

}
