#include "python_client_headers.hpp"
#include "exrpc_data_storage.hpp"
#include "exrpc_dataframe.hpp"

extern "C" {

  // To create dataframe from dvector proxies
  long create_frovedis_dataframe(const char* host, int port, short* types,
                                 const char** col_name, long* dvec, int size){
    ASSERT_PTR(host); 
    std::vector<short> col_types(size);
    std::vector<exrpc_ptr_t> dvec_arr(size);
    for(size_t i = 0; i < size; ++i) col_types[i] = types[i];
    for(size_t i = 0; i < size; ++i) {
      dvec_arr[i] = static_cast<exrpc_ptr_t>(dvec[i]);
    }
    std::vector<std::string> col_names(size);
    for(int i = 0 ; i < size ; ++i) col_names[i] = col_name[i];
    exrpc_node fm_node(host,port);
    exrpc_ptr_t df_proxy = 0;
    try {
      df_proxy = exrpc_async(fm_node,create_dataframe,col_types,col_names,dvec_arr).get();
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
          ret_proxy = exrpc_async(fm_node,get_dfoperator<int>,opd1,opd2,opt,isImmed).get();
          break;
        case LONG:
          ret_proxy = exrpc_async(fm_node,get_dfoperator<long>,opd1,opd2,opt,isImmed).get();
          break;
        case FLOAT:
          ret_proxy = exrpc_async(fm_node, get_dfoperator<float>,opd1,opd2,opt,isImmed).get();
          break;
        case DOUBLE:
          ret_proxy = exrpc_async(fm_node, get_dfoperator<double>,opd1,opd2,opt,isImmed).get();
          break;
        case STRING:
          ret_proxy = exrpc_async(fm_node, get_str_dfoperator, opd1,opd2,opt,isImmed).get();
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
                               const char **cols, int size){
    ASSERT_PTR(host); 
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    std::vector<std::string> targets(size);
    for(size_t i=0; i<size; ++i) targets[i] = std::string(cols[i]); 
    exrpc_ptr_t ret_proxy = 0;
    try {
      ret_proxy = exrpc_async(fm_node,select_df,df_proxy,targets).get();
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
                                const char** cols, int size){
    ASSERT_PTR(host); 
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    std::vector<std::string> targets(size);
    for(size_t i=0; i<size; ++i) targets[i] = std::string(cols[i]);
    exrpc_ptr_t ret_proxy = 0;
    try {
      ret_proxy = exrpc_async(fm_node,group_by_df,df_proxy,targets).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return (static_cast<long>(ret_proxy));
  }
   
  // To perform aggregation on grouped dftable 
  long agg_grouped_dataframe(const char* host, int port, long proxy, 
                             const char** cols, ulong sz1,
                             const char** agg_func, const char** agg_col,
                             const char** agg_col_as, ulong sz2) {
    ASSERT_PTR(host); 
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    std::vector<std::string> grp_cols(sz1);
    std::vector<std::string> s_agg_func(sz2), s_agg_col(sz2), s_agg_col_as(sz2);
    for(size_t i=0; i<sz1; ++i) grp_cols[i] = std::string(cols[i]);
    for(size_t i=0; i<sz2; ++i) s_agg_func[i] = std::string(agg_func[i]);
    for(size_t i=0; i<sz2; ++i) s_agg_col[i] = std::string(agg_col[i]);
    for(size_t i=0; i<sz2; ++i) s_agg_col_as[i] = std::string(agg_col_as[i]);
    exrpc_ptr_t ret_proxy = 0;
    try {
      ret_proxy = exrpc_async(fm_node,frovedis_gdf_aggr,df_proxy,grp_cols,s_agg_func,s_agg_col,s_agg_col_as).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return (static_cast<long>(ret_proxy));
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
                                 const char** cols, const char ** new_cols, 
                                 int size){
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    std::vector<std::string> targets(size), new_targets(size);
    for(size_t i=0; i<size; ++i) { 
       targets[i] = std::string(cols[i]);
       new_targets[i] = std::string(new_cols[i]);
    }
    exrpc_ptr_t ret_proxy = 0;
    try {
      ret_proxy = exrpc_async(fm_node,frovedis_df_rename,df_proxy,targets,new_targets).get();
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
    return to_python_double_list_from_str_vector(ret);
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
    return to_python_double_list_from_str_vector(ret);
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
    return to_python_double_list_from_str_vector(ret);
  }

  PyObject* avg_frovedis_dataframe(const char* host, int port, long proxy,
                                   const char** cols, int size){
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    std::vector<std::string> cc(size);
    for(size_t i=0; i<size; ++i) {
       cc[i] = std::string(cols[i]);
    }
    std::vector<std::string> ret;
    try {
      ret = exrpc_async(fm_node,frovedis_df_avg,df_proxy,cc).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_python_double_list_from_str_vector(ret);
  }

  PyObject* cnt_frovedis_dataframe(const char* host, int port, long proxy,
                                   const char** cols, int size){
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    std::vector<std::string> cc(size);
    for(size_t i=0; i<size; ++i) {
       cc[i] = std::string(cols[i]);
    }
    std::vector<std::string> ret;
    try {
      ret = exrpc_async(fm_node,frovedis_df_cnt,df_proxy,cc).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_python_double_list_from_str_vector(ret);
  }

  PyObject* std_frovedis_dataframe(const char* host, int port, long proxy,
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
      ret = exrpc_async(fm_node,frovedis_df_std,df_proxy,cc,tt).get();
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }
    return to_python_double_list_from_str_vector(ret);
  }

  PyObject* get_frovedis_col(const char* host, int port, long proxy,
                             const char* col_name, short tid){
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto f_dptr = static_cast<exrpc_ptr_t> (proxy);
    auto cname = std::string(col_name);
    PyObject* ret = NULL;
    try {
      switch (tid) {
        case INT: { 
          auto int_col = exrpc_async(fm_node,get_df_int_col,f_dptr,cname).get();
          ret = to_python_int_list(int_col); 
        } break;
        case LONG: {
          auto long_col = exrpc_async(fm_node,get_df_long_col,f_dptr,cname).get();
          ret = to_python_long_list(long_col); 
        } break;
        case FLOAT: {
          auto float_col = exrpc_async(fm_node,get_df_float_col,f_dptr,cname).get();
          ret = to_python_float_list(float_col); 
        } break;
        case DOUBLE: {
          auto double_col = exrpc_async(fm_node,get_df_double_col,f_dptr,cname).get();
          ret = to_python_double_list(double_col); 
        } break;
        case STRING: {
          auto string_col = exrpc_async(fm_node,get_df_string_col,f_dptr,cname).get();
          ret = to_python_string_list(string_col); 
        } break;
        default:  REPORT_ERROR(USER_ERROR,"Unknown type for frovedis dataframe!");
      }
    }
    catch (std::exception& e) {
      set_status(true, e.what());
    }  
    return ret;
  }

  PyObject* df_to_rowmajor(const char* host, int port, long proxy,
                           const char** cols, int size, 
                           short dtype) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    std::vector<std::string> cc(size);
    for(size_t i = 0; i < size; ++i) cc[i] = std::string(cols[i]);
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
                           const char** cols, int size, 
                           short dtype) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    std::vector<std::string> cc(size);
    for(size_t i = 0; i < size; ++i) cc[i] = std::string(cols[i]);
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
                      const char** cols, int size1,
                      const char** cat_cols, int size2,
                      long info_id, short dtype) {
    ASSERT_PTR(host);
    exrpc_node fm_node(host, port);
    auto df_proxy = static_cast<exrpc_ptr_t> (proxy);
    std::vector<std::string> cc1(size1), cc2(size2);
    for(size_t i = 0; i < size1; ++i) cc1[i] = std::string(cols[i]);
    for(size_t i = 0; i < size2; ++i) cc2[i] = std::string(cat_cols[i]);
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
}
