#include "python_client_headers.hpp"
#include "exrpc_data_storage.hpp"
#include "exrpc_dataframe.hpp"
#include "exrpc_dffunction.hpp"

extern "C" {

  // To create dataframe from dvector proxies
  long create_frovedis_dataframe(const char* host, int port, short* types,
                                 const char** col_name, 
                                 long* dvec, ulong size) {
    ASSERT_PTR(host); 
    std::vector<short> col_types(size);
    std::vector<exrpc_ptr_t> dvec_arr(size);
    for(size_t i = 0; i < size; ++i) col_types[i] = types[i];
    for(size_t i = 0; i < size; ++i) {
      dvec_arr[i] = static_cast<exrpc_ptr_t>(dvec[i]);
    }
    auto col_names = to_string_vector(col_name, size);
    exrpc_node fm_node(host,port);
    exrpc_ptr_t df_proxy = 0;
    try {
      df_proxy = exrpc_async(fm_node, create_dataframe, col_types, \
                             col_names, dvec_arr).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return (static_cast<long>(df_proxy));
  }

  // To show frovedis dataframe
  void show_frovedis_dataframe(const char* host, int port, long proxy){
    ASSERT_PTR(host); 
    exrpc_node fm_node(host,port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    try {
      exrpc_oneway(fm_node,show_dataframe,df_proxy);
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  // To support dfoperator
  long get_frovedis_dfoperator(const char* host, int port, const char* op1,
                             const char* op2, short dtype, short opt, 
                             bool isImmed){
    ASSERT_PTR(host); 
    exrpc_node fm_node(host, port);
    std::string opd1(op1);
    std::string opd2(op2);
    exrpc_ptr_t ret_proxy = 0;
    try {
      switch(dtype) {
        case INT:
          ret_proxy = exrpc_async(fm_node, get_dfoperator<int>,
                                  opd1, opd2, opt, isImmed).get();
          break;
        case LONG:
          ret_proxy = exrpc_async(fm_node, get_dfoperator<long>,
                                  opd1, opd2, opt, isImmed).get();
          break;
        case ULONG:
          ret_proxy = exrpc_async(fm_node, get_dfoperator<unsigned long>,
                                  opd1, opd2, opt, isImmed).get();
          break;
        case FLOAT:
          ret_proxy = exrpc_async(fm_node, get_dfoperator<float>,
                                  opd1, opd2, opt, isImmed).get();
          break;
        case DOUBLE:
          ret_proxy = exrpc_async(fm_node, get_dfoperator<double>,
                                  opd1, opd2, opt, isImmed).get();
          break;
        case STRING:
          ret_proxy = exrpc_async(fm_node, get_str_dfoperator, 
                                  opd1, opd2, opt, isImmed).get();
          break;
        default: REPORT_ERROR(USER_ERROR,"Unknown type is encountered!\n");  
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }  
    return (static_cast<long>(ret_proxy));
  }                  

  //To support dfAND operator
  long get_frovedis_dfANDoperator(const char* host,int port,long op1,long op2){
    ASSERT_PTR(host); 
    exrpc_node fm_node(host, port);
    auto lopt_proxy = static_cast<exrpc_ptr_t> (op1); 
    auto ropt_proxy = static_cast<exrpc_ptr_t> (op2);
    exrpc_ptr_t ret_proxy = 0;
    try {
      ret_proxy = exrpc_async(fm_node, get_dfANDoperator,lopt_proxy,ropt_proxy).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }    
    return (static_cast<long>(ret_proxy));
  }
  
  //To support dfOR operator
  long get_frovedis_dfORoperator(const char* host,int port,long op1,long op2){
    ASSERT_PTR(host); 
    exrpc_node fm_node(host, port);
    auto lopt_proxy = static_cast<exrpc_ptr_t> (op1);
    auto ropt_proxy = static_cast<exrpc_ptr_t> (op2);
    exrpc_ptr_t ret_proxy = 0;
    try {
      ret_proxy = exrpc_async(fm_node, get_dfORoperator,lopt_proxy,ropt_proxy).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return (static_cast<long>(ret_proxy));
  }

  //To support dfNOT operator
  long get_frovedis_dfNOToperator(const char* host,int port,long op1){
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto lopt_proxy = static_cast<exrpc_ptr_t> (op1);
    exrpc_ptr_t ret_proxy = 0;
    try {
      ret_proxy = exrpc_async(fm_node, get_dfNOToperator,lopt_proxy).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return (static_cast<long>(ret_proxy));
  }
 
  //To release data frame
  void release_frovedis_dataframe(const char* host,int port,long proxy){
    ASSERT_PTR(host); 
    exrpc_node fm_node(host,port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    try {
      exrpc_oneway(fm_node,release_data<dftable>,df_proxy);
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }
  
  //To release dfoperator
  void release_dfoperator(const char* host,int port,
                          long proxy){
    ASSERT_PTR(host); 
    exrpc_node fm_node(host,port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    try {
      exrpc_oneway(fm_node,(release_data<std::shared_ptr<dfoperator>>),df_proxy);
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }
  
  // To filter rows from the given dataframe based on the given condition
  long filter_frovedis_dataframe(const char* host, int port, long proxy1, 
                                 long proxy2){
    ASSERT_PTR(host); 
    exrpc_node fm_node(host, port);
    auto pro1 = static_cast<exrpc_ptr_t> (proxy1);
    auto pro2 = static_cast<exrpc_ptr_t> (proxy2); 
    exrpc_ptr_t proxy = 0;
    try {
      proxy = exrpc_async(fm_node, filter_df,pro1, pro2).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return (static_cast<long>(proxy));
  } 
 
  // To select requested columns from given dataframe
  long select_frovedis_dataframe(const char* host, int port, long proxy, 
                               const char **cols, ulong size) {
    ASSERT_PTR(host); 
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    auto targets = to_string_vector(cols, size);
    exrpc_ptr_t ret_proxy = 0;
    try {
      ret_proxy = exrpc_async(fm_node, select_df, df_proxy, targets).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return (static_cast<long>(ret_proxy));
  }
   
  // To find missing values in given columns of the input dataframe
  long isnull_frovedis_dataframe(const char* host, int port, long proxy, 
                                 const char **cols, ulong size) {
    ASSERT_PTR(host); 
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    auto targets = to_string_vector(cols, size);
    exrpc_ptr_t ret_proxy = 0;
    try {
      ret_proxy = exrpc_async(fm_node, isnull_df, df_proxy, targets).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return (static_cast<long>(ret_proxy));
  }
   
  // To sort dataframe entries based on given column and requested order
  long sort_frovedis_dataframe(const char* host, int port, long proxy, 
                               const char** cols, 
                               int* asc, int size){
    ASSERT_PTR(host); 
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    std::vector<std::string> targets(size);
    std::vector<int> isdesc(size);
    for(size_t i = 0; i < size; ++i) {
      targets[i] = std::string(cols[i]);
      isdesc[i] = asc[i] == 1 ? 0 : 1;
    }
    exrpc_ptr_t ret_proxy = 0;
    try {
      ret_proxy = exrpc_async(fm_node,sort_df,df_proxy,targets,isdesc).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return (static_cast<long>(ret_proxy));
  }

  // To perform groupBy based on given columns 
  long group_frovedis_dataframe(const char* host, int port, long proxy, 
                                const char** cols, ulong size) {
    ASSERT_PTR(host); 
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    auto targets = to_string_vector(cols, size);
    exrpc_ptr_t ret_proxy = 0;
    try {
      ret_proxy = exrpc_async(fm_node, group_by_df, df_proxy, targets).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return (static_cast<long>(ret_proxy));
  }
   
  // To perform select on grouped dftable 
  long select_grouped_dataframe(const char* host, int port, long proxy, 
                                const char** cols, ulong sz) {
    ASSERT_PTR(host); 
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    auto tcols = to_string_vector(cols, sz);
    exrpc_ptr_t ret = 0;
    try {
      ret = exrpc_async(fm_node,frovedis_gdf_select,df_proxy,tcols).get();
    }
    catch(std::exception& e) { set_status(true,e.what()); }
    return (static_cast<long>(ret));
  }

  // To perform aggregation on grouped dftable 
  long agg_grouped_dataframe(const char* host, int port, long proxy, 
                             const char** cols, ulong sz1,
                             const char** agg_func, const char** agg_col,
                             const char** agg_col_as, ulong sz2) {
    ASSERT_PTR(host); 
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    auto grp_cols = to_string_vector(cols, sz1);
    auto s_agg_func = to_string_vector(agg_func, sz2);
    auto s_agg_col = to_string_vector(agg_col, sz2);
    auto s_agg_col_as = to_string_vector(agg_col_as, sz2);
    exrpc_ptr_t ret = 0;
    try {
      ret = exrpc_async(fm_node,frovedis_gdf_aggr,df_proxy,grp_cols,
                        s_agg_func,s_agg_col,s_agg_col_as).get();
    }
    catch(std::exception& e) { set_status(true,e.what()); }
    return (static_cast<long>(ret));
  }

  // To perform join operation 
  long join_frovedis_dataframe(const char* host, int port,  
                               long proxy1, long proxy2, long proxy3,
                               const char* kind, const char* type){
    ASSERT_PTR(host); 
    exrpc_node fm_node(host, port);
    auto left = static_cast<exrpc_ptr_t> (proxy1);
    auto right = static_cast<exrpc_ptr_t> (proxy2); 
    auto opt = static_cast<exrpc_ptr_t> (proxy3); 
    std::string k(kind), t(type);
    exrpc_ptr_t ret_proxy = 0;
    try {
      ret_proxy = exrpc_async(fm_node, join_df, left, right, opt, k, t).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return (static_cast<long>(ret_proxy));
  } 

  // To rename dataframe columns (returns new dataframe) 
  long rename_frovedis_dataframe(const char* host, int port, long proxy,
                                 const char** cols, const char** new_cols, 
                                 ulong size, bool inplace) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    auto targets = to_string_vector(cols, size);
    auto new_targets = to_string_vector(new_cols, size);
    exrpc_ptr_t ret_proxy = 0;
    try {
      ret_proxy = exrpc_async(fm_node, frovedis_df_rename, df_proxy, 
                              targets, new_targets, inplace).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return (static_cast<long>(ret_proxy));
  }

  PyObject* min_frovedis_dataframe(const char* host, int port, long proxy,
                                   const char** cols, short* types,
                                   int size){
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    std::vector<std::string> cc(size);
    std::vector<short> tt(size);
    for(size_t i=0; i<size; ++i) {
       cc[i] = std::string(cols[i]);
       tt[i] = types[i];
    }
    std::vector<std::string> ret;
    try {
      ret = exrpc_async(fm_node,frovedis_df_min,df_proxy,cc,tt).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_python_string_list(ret);
  }

  PyObject* max_frovedis_dataframe(const char* host, int port, long proxy,
                                   const char** cols, short* types,
                                   int size){
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    std::vector<std::string> cc(size);
    std::vector<short> tt(size);
    for(size_t i=0; i<size; ++i) {
       cc[i] = std::string(cols[i]);
       tt[i] = types[i];
    }
    std::vector<std::string> ret;
    try {
      ret = exrpc_async(fm_node,frovedis_df_max,df_proxy,cc,tt).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_python_string_list(ret);
  }

  PyObject* sum_frovedis_dataframe(const char* host, int port, long proxy,
                                   const char** cols, short* types,
                                   int size){
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    std::vector<std::string> cc(size);
    std::vector<short> tt(size);
    for(size_t i=0; i<size; ++i) {
       cc[i] = std::string(cols[i]);
       tt[i] = types[i];
    }
    std::vector<std::string> ret;
    try {
     ret = exrpc_async(fm_node,frovedis_df_sum,df_proxy,cc,tt).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_python_string_list(ret);
  }

  PyObject* avg_frovedis_dataframe(const char* host, int port, long proxy,
                                   const char** cols, ulong size){
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    auto cc = to_string_vector(cols, size);
    std::vector<std::string> ret;
    try {
      ret = exrpc_async(fm_node,frovedis_df_avg,df_proxy,cc).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_python_string_list(ret);
  }

  PyObject* cnt_frovedis_dataframe(const char* host, int port, long proxy,
                                   const char** cols, ulong size){
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    auto cc = to_string_vector(cols, size);
    std::vector<std::string> ret;
    try {
      ret = exrpc_async(fm_node,frovedis_df_cnt,df_proxy,cc).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_python_string_list(ret);
  }

  PyObject* std_frovedis_dataframe(const char* host, int port, long proxy,
                                   const char** cols, ulong size) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    auto cc = to_string_vector(cols, size);
    std::vector<std::string> ret;
    try {
      ret = exrpc_async(fm_node,frovedis_df_std,df_proxy,cc).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_python_string_list(ret);
  }

  PyObject* get_frovedis_col(const char* host, int port, long proxy,
                             const char* col_name, short tid){
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto f_dptr = static_cast<exrpc_ptr_t> (proxy);
    auto cname = std::string(col_name);
    dummy_vector dvec;
    try {
      switch (tid) {
        case BOOL:
        case INT:    dvec = exrpc_async(fm_node, get_df_int_col, 
                                        f_dptr, cname).get(); break;
        case LONG:   dvec = exrpc_async(fm_node, get_df_long_col, 
                                        f_dptr, cname).get(); break;
        case ULONG:  dvec = exrpc_async(fm_node, get_df_ulong_col, 
                                        f_dptr, cname).get(); break;
        case FLOAT:  dvec = exrpc_async(fm_node, get_df_float_col, 
                                        f_dptr, cname).get(); break;
        case DOUBLE: dvec = exrpc_async(fm_node, get_df_double_col, 
                                        f_dptr, cname).get(); break;
        case STRING: dvec = exrpc_async(fm_node, get_df_string_col,
                                        f_dptr, cname).get(); break;
        default:     REPORT_ERROR(USER_ERROR, 
                     "Unknown type for frovedis dataframe: " + std::to_string(tid));
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }  
    return to_py_dummy_vector(dvec);
  }

  PyObject* df_to_rowmajor(const char* host, int port, long proxy,
                           const char** cols, ulong size, 
                           short dtype) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    auto cc = to_string_vector(cols, size);
    dummy_matrix dmat;
    try {
      switch(dtype) {
        case FLOAT:  dmat = exrpc_async(fm_node,df_to_rowmajor_float,df_proxy,cc).get(); break;
        case DOUBLE: dmat = exrpc_async(fm_node,df_to_rowmajor_double,df_proxy,cc).get(); break;
        default: REPORT_ERROR(USER_ERROR, 
                 "Unsupported dataframe to matrix conversion dtype is encountered!\n");
      };
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_matrix(dmat);
  }

  PyObject* df_to_colmajor(const char* host, int port, long proxy,
                           const char** cols, ulong size, 
                           short dtype) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    auto cc = to_string_vector(cols, size);
    dummy_matrix dmat;
    try {
      switch(dtype) {
        case FLOAT:  dmat = exrpc_async(fm_node,df_to_colmajor_float,df_proxy,cc).get(); break;
        case DOUBLE: dmat = exrpc_async(fm_node,df_to_colmajor_double,df_proxy,cc).get(); break;
        default: REPORT_ERROR(USER_ERROR, 
                 "Unsupported dataframe to matrix conversion dtype is encountered!\n");
      };
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_matrix(dmat);
  }

  PyObject* df_to_crs(const char* host, int port, long proxy,
                      const char** cols, ulong size1,
                      const char** cat_cols, ulong size2,
                      long info_id, short dtype) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    auto cc1 = to_string_vector(cols, size1);
    auto cc2 = to_string_vector(cat_cols, size2);
    dummy_matrix dmat;
    try {
      switch(dtype) {
        case FLOAT:  dmat = exrpc_async(fm_node,df_to_crs_float,df_proxy,cc1,cc2,info_id).get(); break;
        case DOUBLE: dmat = exrpc_async(fm_node,df_to_crs_double,df_proxy,cc1,cc2,info_id).get(); break;
        default: REPORT_ERROR(USER_ERROR,
                 "Unsupported dataframe to matrix conversion dtype is encountered!\n");
      };
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_matrix(dmat);
  }

  PyObject* df_to_crs_using_info(const char* host, int port, long proxy,
                                 long info_id, short dtype) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    dummy_matrix dmat;
    try {
      switch(dtype) {
        case FLOAT:  dmat = exrpc_async(fm_node,df_to_crs_float_using_info,df_proxy,info_id).get(); break;
        case DOUBLE: dmat = exrpc_async(fm_node,df_to_crs_double_using_info,df_proxy,info_id).get(); break;
        default: REPORT_ERROR(USER_ERROR,
                 "Unsupported dataframe to matrix conversion dtype is encountered!\n");
      };
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_matrix(dmat);
  }

  void load_dftable_to_sparse_info(const char* host, int port,
                                   long info_id, const char* dirname) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    try {
      exrpc_oneway(fm_node,load_sparse_conversion_info,info_id,std::string(dirname));
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  void save_dftable_to_sparse_info(const char* host, int port,
                                   long info_id, const char* dirname) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    try {
      exrpc_oneway(fm_node,save_sparse_conversion_info,info_id,std::string(dirname));
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  void release_dftable_to_sparse_info(const char* host, int port, long info_id) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    try {
      exrpc_oneway(fm_node,release_sparse_conversion_info,info_id);
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  // multi-eq join
  long get_multi_eq_dfopt(const char* host, int port, 
                        const char** left_on,
                        const char** right_on,
                        ulong sz) {
    ASSERT_PTR(host); 
    exrpc_node fm_node(host, port);

    auto left_cols = to_string_vector(left_on, sz);
    auto right_cols = to_string_vector(right_on, sz);

    exrpc_ptr_t ret_proxy = 0;
    try {
      ret_proxy = exrpc_async(fm_node, frov_multi_eq_dfopt, left_cols, right_cols).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return (static_cast<long>(ret_proxy));
  }

  PyObject* load_dataframe_from_csv(const char* host, int port,
                                   const char* filename,
                                   const char** types, const char** names,
                                   ulong types_size, ulong names_size,
                                   char sep, const char* nullstr,
                                   const char* comment,
                                   size_t rows_to_see, double separate_mb,
                                   bool partial_type_info,
                                   const char** dtype_keys_arr, 
                                   const char** dtype_vals_arr,
                                   ulong dtypes_dict_size, 
                                   bool to_separate, bool add_index,
                                   int* usecols, ulong usecols_len,
                                   bool verbose, bool mangle_dupe_cols,
                                   int index_col, const char** bool_cols,
                                   ulong bool_cols_size) {
    ASSERT_PTR(host);
 
    std::vector<std::string> col_types, col_names, bool_col_vec;
    if (names_size > 0) col_names = to_string_vector(names, names_size);
    if (types_size > 0) col_types = to_string_vector(types, types_size);
    if (bool_cols_size > 0) bool_col_vec = to_string_vector(bool_cols, bool_cols_size);

    std::map<std::string, std::string> type_map;
    if (partial_type_info) {
      auto dtype_keys = to_string_vector(dtype_keys_arr, dtypes_dict_size);
      auto dtype_vals = to_string_vector(dtype_vals_arr, dtypes_dict_size);
      for (size_t i = 0; i < dtype_keys.size(); i++) type_map[dtype_keys[i]] = dtype_vals[i];
    }

    auto filename_ = std::string(filename);
    auto usecols_vec = to_int_vector(usecols, usecols_len);
    csv_config conf((int)sep, nullstr, comment, rows_to_see, 
                    separate_mb, to_separate, add_index, 
                    verbose, mangle_dupe_cols, index_col);

    dummy_dftable res;
    exrpc_node fm_node(host, port);
    try {
      res = exrpc_async(fm_node, frov_load_dataframe_from_csv, filename_,
                        col_types, col_names, partial_type_info, type_map, 
                        usecols_vec, bool_col_vec, conf). get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_df(res);
  }

  long get_frovedis_dataframe_length(const char* host, int port, 
                                     long fdata) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto fproxy = static_cast<exrpc_ptr_t>(fdata);
    size_t len = 0;
    try {
      len = exrpc_async(fm_node, get_dataframe_length, fproxy).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return (long) len;
  }

  PyObject* df_convert_dicstring_to_bool(const char* host, int port,
                                        long proxy, const char **col_names,
                                        ulong sz, const char* nullstr) {
    ASSERT_PTR(host);
    auto df_proxy = static_cast<exrpc_ptr_t>(proxy);
    auto col_names_ = to_string_vector(col_names, sz);
    auto nullstr_ = std::string(nullstr);
    exrpc_node fm_node(host,port);
    dummy_dftable res;
    try {
      res = exrpc_async(fm_node, frov_df_convert_dicstring_to_bool, df_proxy,
                       col_names_, nullstr_).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_df(res);
  }

  PyObject* df_append_column(const char* host, int port,  long proxy,
                            const char* col_name, short type, long dvec,
                            int position, bool drop_old) {
    ASSERT_PTR(host);
    auto df_proxy = static_cast<exrpc_ptr_t>(proxy);
    auto dvec_proxy = static_cast<exrpc_ptr_t>(dvec);
    auto col_name_ = std::string(col_name);
    exrpc_node fm_node(host,port);
    dummy_dftable res;
    try {
      res = exrpc_async(fm_node, frov_df_append_column, df_proxy, col_name_,
                      type, dvec_proxy, position, drop_old).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_df(res);
  }

  PyObject* df_add_index(const char* host, int port, long proxy,
                         const char* name) {
    ASSERT_PTR(host); 
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    std::string name_(name);
    dummy_dftable res;
    try {
      res = exrpc_async(fm_node, frov_df_add_index, df_proxy, name_).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_df(res);
  }

  PyObject* df_reset_index(const char* host, int port, 
                           long proxy, bool drop) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    dummy_dftable res;
    try {
      res = exrpc_async(fm_node, frov_df_reset_index, df_proxy, drop).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_df(res);
  }

  PyObject* df_set_index(const char* host, int port, long proxy,
                         const char* cur_ind_name, 
                         const char* new_ind_name,
                         bool verify_integrity) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    std::string cur_index_name(cur_ind_name), new_index_name(new_ind_name);
    dummy_dftable res;
    try {
      res = exrpc_async(fm_node, frov_df_set_index,
                        df_proxy, cur_index_name, new_index_name,
                        verify_integrity).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_df(res);
  }

  PyObject* df_copy_index(const char* host, int port, 
                          long to_df, long from_df, 
                          const char* i_col, short dtype) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto to_df_ = static_cast<exrpc_ptr_t> (to_df);
    auto from_df_ = static_cast<exrpc_ptr_t> (from_df);
    std::string index_col(i_col);
    dummy_dftable res;
    try {
      res = exrpc_async(fm_node, frov_df_copy_index, 
                        to_df_, from_df_, index_col, dtype).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_df(res);
  }

  PyObject* df_copy_column(const char* host, int port, 
                           long to_df, long from_df, 
                           const char** names, 
                           const char** names_as, 
                           short* types, 
                           ulong size) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto to_df_ = static_cast<exrpc_ptr_t> (to_df);
    auto from_df_ = static_cast<exrpc_ptr_t> (from_df);
    std::vector<std::string> nm(size), nm_as(size);
    std::vector<short> tt(size);
    for(size_t i = 0; i < size; ++i) {
       nm[i] = std::string(names[i]);
       nm_as[i] = std::string(names_as[i]);
       tt[i] = types[i];
    }
    dummy_dftable res;
    try {
      res = exrpc_async(fm_node, frov_df_copy_column, 
                        to_df_, from_df_, nm, nm_as, tt).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_df(res);
  }

  PyObject* df_astype(const char* host, int port, 
                      long df,  
                      const char** cols, short* types,
                      ulong size) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (df);
    std::vector<std::string> cc(size);
    std::vector<short> tt(size);
    for(size_t i = 0; i < size; ++i) {
       cc[i] = std::string(cols[i]);
       tt[i] = types[i];
    }
    dummy_dftable res;
    try {
      res = exrpc_async(fm_node, frov_df_astype, df_proxy, cc, tt).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_df(res);
  }

  void drop_frovedis_dataframe_columns(const char* host, int port, long proxy,
                                       const char** cols, ulong sz) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    auto tcols = to_string_vector(cols, sz);
    try {
      exrpc_oneway(fm_node, drop_df_cols, df_proxy, tcols);
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
  }

  PyObject* drop_frovedis_duplicate_rows(const char* host, int port, 
                                         long proxy,
                                         const char** cols, ulong sz,
                                         const char* keep) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    auto tcols = to_string_vector(cols, sz);
    std::string keep_str(keep);
    dummy_dftable ret;
    try {
      ret = exrpc_async(fm_node, frov_df_drop_duplicates, 
                        df_proxy, tcols, keep_str).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_df(ret);
  }

  PyObject* drop_frovedis_dataframe_rows_int(const char* host, int port, 
                                             long proxy,
                                             int* cols, ulong sz,
                                             const char* index_cname) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    auto tcols = to_int_vector(cols, sz);
    std::string ind_name(index_cname);
    dummy_dftable ret;
    try {
      ret = exrpc_async(fm_node, drop_df_rows<int>, 
                        df_proxy, ind_name, tcols).get(); 
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_df(ret);
  }

  PyObject* drop_frovedis_dataframe_rows_long(const char* host, int port, 
                                              long proxy,
                                              long* cols, ulong sz,
                                              const char* index_cname) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    auto tcols = to_long_vector(cols, sz);
    std::string ind_name(index_cname);
    dummy_dftable ret;
    try {
      ret = exrpc_async(fm_node, drop_df_rows<long>, 
                        df_proxy, ind_name, tcols).get(); 
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_df(ret);
  }

  PyObject* drop_frovedis_dataframe_rows_ulong(const char* host, int port, 
                                               long proxy,
                                               ulong* cols, ulong sz,
                                               const char* index_cname) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    auto tcols = to_ulong_vector(cols, sz);
    std::string ind_name(index_cname);
    dummy_dftable ret;
    try {
      ret = exrpc_async(fm_node, drop_df_rows<unsigned long>, 
                        df_proxy, ind_name, tcols).get(); 
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_df(ret);
  }

  PyObject* drop_frovedis_dataframe_rows_float(const char* host, int port, 
                                               long proxy,
                                               float* cols, ulong sz,
                                               const char* index_cname) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    auto tcols = to_float_vector(cols, sz);
    std::string ind_name(index_cname);
    dummy_dftable ret;
    try {
      ret = exrpc_async(fm_node, drop_df_rows<float>, 
                        df_proxy, ind_name, tcols).get(); 
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_df(ret);
  }

  PyObject* drop_frovedis_dataframe_rows_double(const char* host, int port, 
                                                long proxy,
                                                double* cols, ulong sz,
                                                const char* index_cname) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    auto tcols = to_double_vector(cols, sz);
    std::string ind_name(index_cname);
    dummy_dftable ret;
    try {
      ret = exrpc_async(fm_node, drop_df_rows<double>, 
                        df_proxy, ind_name, tcols).get(); 
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_df(ret);
  }

  PyObject* drop_frovedis_dataframe_rows_str(const char* host, int port, 
                                             long proxy,
                                             const char** cols, ulong sz,
                                             const char* index_cname) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    auto tcols = to_string_vector(cols, sz);
    std::string ind_name(index_cname);
    dummy_dftable ret;
    try {
      ret = exrpc_async(fm_node, drop_df_rows<std::string>, 
                        df_proxy, ind_name, tcols).get(); 
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_df(ret);
  }

 PyObject* df_union(const char* host, int port, long df_proxy,
                    long* proxies, ulong size, 
                    const char** names, ulong names_size, 
                    bool verify_integrity) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy_ = static_cast<exrpc_ptr_t> (df_proxy);
    auto names_ = to_string_vector(names, names_size);
    std::vector<exrpc_ptr_t> proxies_(size);
    for(size_t i = 0; i < size; ++i) {
      proxies_[i] = static_cast<exrpc_ptr_t>(proxies[i]);
    }
    dummy_dftable res;
    try {
      res = exrpc_async(fm_node, frov_df_union, df_proxy_, proxies_,
                        names_, verify_integrity).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_df(res);
  }

  PyObject* df_set_col_order(const char* host, int port, long df_proxy,
                            const char** new_cols, ulong sz) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy_ = static_cast<exrpc_ptr_t> (df_proxy);
    auto new_cols_ = to_string_vector(new_cols, sz);

    dummy_dftable res;
    try {
      res = exrpc_async(fm_node, frov_df_set_col_order, df_proxy_,
                       new_cols_).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_df(res);
  }

  PyObject* df_binary_operation(const char* host, int port, 
                                long df_proxy1, long df_proxy2,
                                bool is_series,
                                const char* fill_value,
                                const char* fill_value_dtype,
                                const char* op_type,
                                bool nan_is_null) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy_1 = static_cast<exrpc_ptr_t>(df_proxy1);
    auto df_proxy_2 = static_cast<exrpc_ptr_t>(df_proxy2);
    std::string fillv(fill_value), fillv_dt(fill_value_dtype), opt(op_type);
    dummy_dftable res;
    try {
      res = exrpc_async(fm_node, frov_df_binary_operation, 
                        df_proxy_1, df_proxy_2,
                        is_series, fillv, fillv_dt, opt, nan_is_null).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_df(res);
  }

  PyObject* df_immed_binary_operation(const char* host, int port,
                                      long df_proxy1,
                                      const char* immed_value,
                                      const char* value_type,
                                      const char* op_type,
                                      bool is_reversed,
                                      bool nan_is_null) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy_1 = static_cast<exrpc_ptr_t>(df_proxy1);
    std::string val(immed_value), vtype(value_type), opt(op_type);
    dummy_dftable res;
    try {
      res = exrpc_async(fm_node, frov_df_immed_binary_operation,
                        df_proxy_1, val, vtype, opt, 
                        is_reversed, nan_is_null).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_df(res);
  }

  PyObject* df_abs(const char* host, int port, long df_proxy) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy_ = static_cast<exrpc_ptr_t>(df_proxy);
    dummy_dftable res;
    try {
      res = exrpc_async(fm_node, frov_df_abs, df_proxy_).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_df(res);
  }

  PyObject* df_fillna(const char* host, int port, long df_proxy,
                      const char* fill_value, bool has_index) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy_ = static_cast<exrpc_ptr_t>(df_proxy);
    std::string fillv(fill_value);
    dummy_dftable res;
    try {
      res = exrpc_async(fm_node, frov_df_fillna, df_proxy_, fillv, has_index).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_py_dummy_df(res);
  }
}
