#include "exrpc_dataframe.hpp"

using namespace frovedis;

exrpc_ptr_t create_dataframe (std::vector<short>& types,
                              std::vector<std::string>& cols,
                              std::vector<exrpc_ptr_t>& dvec_proxies) {
  //for(auto t: types) std::cout << t << " "; std::cout << std::endl;
  auto dftblp = new dftable();
  if (!dftblp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  for(size_t i=0; i<cols.size(); ++i) {
    switch(types[i]) {
      case INT:    { auto v1 = reinterpret_cast<dvector<int>*>(dvec_proxies[i]);
                     dftblp->append_column(cols[i],std::move(*v1));
                     delete v1; break; }
      case LONG:   { auto v2 = reinterpret_cast<dvector<long>*>(dvec_proxies[i]);
                     dftblp->append_column(cols[i],std::move(*v2));
                     delete v2; break; }
      case FLOAT:  { auto v3 = reinterpret_cast<dvector<float>*>(dvec_proxies[i]);
                     dftblp->append_column(cols[i],std::move(*v3));
                     delete v3; break; }
      case DOUBLE: { auto v4 = reinterpret_cast<dvector<double>*>(dvec_proxies[i]);
                     dftblp->append_column(cols[i],std::move(*v4));
                     delete v4; break; }
      case STRING: { auto v5 = reinterpret_cast<dvector<std::string>*>(dvec_proxies[i]);
                     dftblp->append_column(cols[i],std::move(*v5));
                     delete v5; break; }
      case BOOL:   { auto v6 = reinterpret_cast<dvector<int>*>(dvec_proxies[i]);
                     dftblp->append_column(cols[i],std::move(*v6));
                     delete v6; break; }
      default:     auto msg = "Unsupported datatype in dataframe creation: " + std::to_string(types[i]);
                   REPORT_ERROR(USER_ERROR,msg);
    }
  }
  return reinterpret_cast<exrpc_ptr_t>(dftblp);
}

void show_dataframe(exrpc_ptr_t& df_proxy) {
  auto dftblp = reinterpret_cast<dftable_base*>(df_proxy);
  dftblp->show();
}

exrpc_ptr_t get_str_dfoperator(std::string& op1, std::string& op2,
                               short& op_id, bool& isImmed) {
  std::shared_ptr<dfoperator> *opt = NULL;
  if(isImmed) {
    switch(op_id) {
      case EQ: opt = new std::shared_ptr<dfoperator>(eq_im<std::string>(op1,op2)); break;
      case NE: opt = new std::shared_ptr<dfoperator>(neq_im<std::string>(op1,op2)); break;
      default: REPORT_ERROR(USER_ERROR,
               "Unsupported filter operation on string type column is encountered!\n");
    }
  }
  else {
    switch(op_id) {
      case EQ: opt = new std::shared_ptr<dfoperator>(eq(op1,op2)); break;
      case NE: opt = new std::shared_ptr<dfoperator>(neq(op1,op2)); break;
      default: REPORT_ERROR(USER_ERROR,
               "Unsupported filter operation on string type column is encountered!\n");
    }
  }
  if (!opt) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  return reinterpret_cast<exrpc_ptr_t> (opt);
}

exrpc_ptr_t get_dfANDoperator(exrpc_ptr_t& lopt_proxy,
                              exrpc_ptr_t& ropt_proxy) {
  auto lopt = reinterpret_cast<std::shared_ptr<dfoperator>*>(lopt_proxy);
  auto ropt = reinterpret_cast<std::shared_ptr<dfoperator>*>(ropt_proxy);
  auto and_opt_ptr = new std::shared_ptr<dfoperator>(and_op(*lopt,*ropt));
  if (!and_opt_ptr) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  delete lopt; delete ropt;
  return reinterpret_cast<exrpc_ptr_t> (and_opt_ptr);
}

exrpc_ptr_t get_dfORoperator(exrpc_ptr_t& lopt_proxy,
                             exrpc_ptr_t& ropt_proxy) {
  auto lopt = reinterpret_cast<std::shared_ptr<dfoperator>*>(lopt_proxy);
  auto ropt = reinterpret_cast<std::shared_ptr<dfoperator>*>(ropt_proxy);
  auto or_opt_ptr = new std::shared_ptr<dfoperator>(or_op(*lopt,*ropt));
  if (!or_opt_ptr) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  delete lopt; delete ropt;
  return reinterpret_cast<exrpc_ptr_t> (or_opt_ptr);
}

exrpc_ptr_t filter_df(exrpc_ptr_t& df_proxy,
                      exrpc_ptr_t& opt_proxy) {
  auto& dftbl = *reinterpret_cast<dftable_base*>(df_proxy);
  auto& dfopt = *reinterpret_cast<std::shared_ptr<dfoperator>*>(opt_proxy);
  auto f_df_ptr = new filtered_dftable(dftbl.filter(dfopt));
  if (!f_df_ptr) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  return reinterpret_cast<exrpc_ptr_t> (f_df_ptr);
}

exrpc_ptr_t select_df(exrpc_ptr_t& df_proxy,
                      std::vector<std::string>& cols) {
  auto& dftbl = *reinterpret_cast<dftable_base*>(df_proxy);
  auto s_df_ptr = new dftable(dftbl.select(cols));
  if (!s_df_ptr) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  return reinterpret_cast<exrpc_ptr_t> (s_df_ptr);
}

exrpc_ptr_t sort_df(exrpc_ptr_t& df_proxy,
                    std::vector<std::string>& cols,
                    bool& isDesc) {
  checkAssumption(cols.size() >= 1);
  auto& dftbl = *reinterpret_cast<dftable_base*>(df_proxy);
  sorted_dftable *s_df_ptr = NULL;
  auto tsize = cols.size();
  if(isDesc) {
    auto ret = dftbl.sort_desc(cols[tsize-1]); // sorting based on last column
    for(int i=tsize-2; i>=0; --i) ret = ret.sort_desc(cols[i]);
    s_df_ptr = new sorted_dftable(std::move(ret));
    if (!s_df_ptr) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  }
  else {
    auto ret = dftbl.sort(cols[tsize-1]); // sorting based on last column
    for(int i=tsize-2; i>=0; --i) ret = ret.sort(cols[i]);
    s_df_ptr = new sorted_dftable(std::move(ret));
    if (!s_df_ptr) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  }
  return reinterpret_cast<exrpc_ptr_t> (s_df_ptr);
}

exrpc_ptr_t join_df(exrpc_ptr_t& left_proxy, exrpc_ptr_t& right_proxy,
                    exrpc_ptr_t& opt_proxy, 
                    std::string& how, std::string& join_type) {
  auto& left = *reinterpret_cast<dftable_base*>(left_proxy);
  auto& right = *reinterpret_cast<dftable_base*>(right_proxy);
  auto& dfopt = *reinterpret_cast<std::shared_ptr<dfoperator>*>(opt_proxy);
  dftable_base *bptr = NULL;
  if (join_type == "bcast") {
    if (how == "inner") 
       bptr = new bcast_joined_dftable(left.bcast_join(right,dfopt));
    else if (how == "outer") 
       bptr = new bcast_joined_dftable(left.outer_bcast_join(right,dfopt));
    else REPORT_ERROR(USER_ERROR, "Unknown join operation!\n");
  }
  else if (join_type == "hash") {
    if (how == "inner") 
       bptr = new hash_joined_dftable(left.hash_join(right,dfopt));
    else if (how == "outer") 
       bptr = new hash_joined_dftable(left.outer_hash_join(right,dfopt));
    else REPORT_ERROR(USER_ERROR, "Unknown join operation!\n");
  }
  else REPORT_ERROR(USER_ERROR, "Unknown join kind!\n");

  if (!bptr) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  return reinterpret_cast<exrpc_ptr_t> (bptr);
}

exrpc_ptr_t group_by_df(exrpc_ptr_t& df_proxy, std::vector<std::string>& cols) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  grouped_dftable *g_df_ptr = new grouped_dftable(df.group_by(cols));
  if (!g_df_ptr) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  return reinterpret_cast<exrpc_ptr_t> (g_df_ptr);
}

std::shared_ptr<dfaggregator> get_aggr(std::string& funcname, 
                                       std::string& col, 
                                       std::string& as_col) {
  std::shared_ptr<dfaggregator> ret;
  if (funcname == "sum")        ret = sum_as(col,as_col);
  else if (funcname == "avg" || 
           funcname == "mean")  ret = avg_as(col,as_col);
  else if (funcname == "min")   ret = min_as(col,as_col);
  else if (funcname == "max")   ret = max_as(col,as_col);
  else if (funcname == "count") ret = count_as(col,as_col);
  else REPORT_ERROR(USER_ERROR,"Unsupported aggregation function is requesred!\n");
  return ret;
}

exrpc_ptr_t frovedis_gdf_aggr(exrpc_ptr_t& df_proxy, 
                              std::vector<std::string>& groupedCols,
                              std::vector<std::string>& aggFuncs,
                              std::vector<std::string>& aggCols,
                              std::vector<std::string>& aggAsCols) {

  auto& df = *reinterpret_cast<grouped_dftable*>(df_proxy);
  auto size = aggFuncs.size();
  std::vector<std::shared_ptr<dfaggregator>> agg(size);
  for(size_t i=0; i<size; ++i) {
    agg[i] = get_aggr(aggFuncs[i],aggCols[i],aggAsCols[i]);
  }
  auto retp = new dftable(df.select(groupedCols,agg));
  if (!retp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  return reinterpret_cast<exrpc_ptr_t> (retp);
}


long frovedis_df_size(exrpc_ptr_t& df_proxy) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  return static_cast<long> (df.num_row());
}

std::vector<std::string> 
frovedis_df_sum(exrpc_ptr_t& df_proxy, 
                std::vector<std::string>& cols,
                std::vector<short>& types) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  std::vector<std::string> ret(cols.size());
  for (size_t i=0; i<cols.size(); ++i) { 
    switch(types[i]) {
      case INT:  ret[i] = std::to_string(df.sum<int>(cols[i])); break;
      case LONG:  ret[i] = std::to_string(df.sum<long>(cols[i])); break;
      case FLOAT:  ret[i] = std::to_string(df.sum<float>(cols[i])); break;
      case DOUBLE:  ret[i] = std::to_string(df.sum<double>(cols[i])); break;
      default: REPORT_ERROR(USER_ERROR, "sum on non-numeric column!\n");
    }
  }
  return ret;
}

std::vector<std::string> 
frovedis_df_min(exrpc_ptr_t& df_proxy, 
                std::vector<std::string>& cols,
                std::vector<short>& types) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  std::vector<std::string> ret(cols.size());
  for (size_t i=0; i<cols.size(); ++i) { 
    switch(types[i]) {
      case INT:  ret[i] = std::to_string(df.min<int>(cols[i])); break;
      case LONG:  ret[i] = std::to_string(df.min<long>(cols[i])); break;
      case FLOAT:  ret[i] = std::to_string(df.min<float>(cols[i])); break;
      case DOUBLE:  ret[i] = std::to_string(df.min<double>(cols[i])); break;
      default: REPORT_ERROR(USER_ERROR, "min on non-numeric column!\n");
    }
  }
  return ret;
}

std::vector<std::string> 
frovedis_df_max(exrpc_ptr_t& df_proxy, 
                 std::vector<std::string>& cols,
                 std::vector<short>& types) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  std::vector<std::string> ret(cols.size());
  for (size_t i=0; i<cols.size(); ++i) { 
    switch(types[i]) {
      case INT:  ret[i] = std::to_string(df.max<int>(cols[i])); break;
      case LONG:  ret[i] = std::to_string(df.max<long>(cols[i])); break;
      case FLOAT:  ret[i] = std::to_string(df.max<float>(cols[i])); break;
      case DOUBLE:  ret[i] = std::to_string(df.max<double>(cols[i])); break;
      default: REPORT_ERROR(USER_ERROR, "max on non-numeric column!\n");
    }
  }
  return ret;
}

std::vector<std::string> 
frovedis_df_avg(exrpc_ptr_t& df_proxy, 
                std::vector<std::string>& cols) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  std::vector<std::string> ret(cols.size());
  for (size_t i=0; i<cols.size(); ++i) { 
    ret[i] = std::to_string(df.avg(cols[i]));
  }
  return ret;
}

template <class T>
double calc_part_std(std::vector<T>& data, double mean) {
  double sum = 0;
  for(size_t i=0; i<data.size(); ++i) {
    auto diff = data[i] - mean;
    auto sqr = diff * diff;
    sum += sqr;
  }
  //std::cout << "[" << get_selfid() << "]: " << sum << std::endl;
  return sum;
}

double sum(double a, double b) { return a + b; }

template <class T>
double get_std(dftable_base& df, const std::string& name) {
  auto tc1 = std::dynamic_pointer_cast<typed_dfcolumn<T>>(df.column(name));
  if(!tc1)
    throw std::runtime_error
      ("get_std: column type is different from specified type");
  auto size = df.count(name);
  double mean = df.avg(name);
  auto&& val1 = tc1->get_val();
  double ret = val1.map(calc_part_std<T>,broadcast(mean)).reduce(sum);
  //std::cout << ret << std::endl;
  return std::sqrt(ret/(size-1));
}

std::vector<std::string>
frovedis_df_std(exrpc_ptr_t& df_proxy,
                 std::vector<std::string>& cols,
                 std::vector<short>& types) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  std::vector<std::string> ret(cols.size());
  for (size_t i=0; i<cols.size(); ++i) {
    switch(types[i]) {
      case INT:     ret[i] = std::to_string(get_std<int>(df,cols[i])); break;
      case LONG:    ret[i] = std::to_string(get_std<long>(df,cols[i])); break;
      case FLOAT:   ret[i] = std::to_string(get_std<float>(df,cols[i])); break;
      case DOUBLE:  ret[i] = std::to_string(get_std<double>(df,cols[i])); break;
      default: REPORT_ERROR(USER_ERROR, "std on non-numeric column!\n");
    }
  }
  return ret;
}

std::vector<std::string> 
frovedis_df_cnt(exrpc_ptr_t& df_proxy, 
                std::vector<std::string>& cols) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  std::vector<std::string> ret(cols.size());
  for (size_t i=0; i<cols.size(); ++i) { 
    ret[i] = std::to_string(df.count(cols[i]));
  }
  return ret;
}

exrpc_ptr_t frovedis_df_rename(exrpc_ptr_t& df_proxy,
                               std::vector<std::string>& cols,
                               std::vector<std::string>& new_cols) {
  checkAssumption(cols.size() == new_cols.size());
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  auto ret = new dftable(df); // copying dataframe
  if (!ret) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  for(size_t i=0; i<cols.size(); ++i) {
    ret->rename(cols[i], new_cols[i]);
  }
  return reinterpret_cast<exrpc_ptr_t> (ret);
}

std::vector<int> get_df_int_col(exrpc_ptr_t& df_proxy, 
                                std::string& cname) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  return df.as_dvector<int>(cname).gather();
}

std::vector<long> get_df_long_col(exrpc_ptr_t& df_proxy, 
                                  std::string& cname) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  return df.as_dvector<long>(cname).gather();
}

std::vector<float> get_df_float_col(exrpc_ptr_t& df_proxy, 
                                    std::string& cname) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  return df.as_dvector<float>(cname).gather();
}

std::vector<double> get_df_double_col(exrpc_ptr_t& df_proxy, 
                                      std::string& cname) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  return df.as_dvector<double>(cname).gather();
}

std::vector<std::string> 
get_df_string_col(exrpc_ptr_t& df_proxy, std::string& cname) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  return df.as_dvector<std::string>(cname).gather();
}

dummy_matrix df_to_colmajor_float(exrpc_ptr_t& df_proxy,
                                 std::vector<std::string>& t_cols) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  auto mat = df.to_colmajor_matrix_float(t_cols);
  auto nrow = mat.num_row;
  auto ncol = mat.num_col;
  auto retp = new colmajor_matrix<float>(std::move(mat));
  if (!retp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  auto retp_ = reinterpret_cast<exrpc_ptr_t>(retp);
  return dummy_matrix(retp_,nrow,ncol);
}

dummy_matrix df_to_colmajor_double(exrpc_ptr_t& df_proxy,
                                 std::vector<std::string>& t_cols) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  auto mat = df.to_colmajor_matrix_double(t_cols);
  auto nrow = mat.num_row;
  auto ncol = mat.num_col;
  auto retp = new colmajor_matrix<double>(std::move(mat));
  if (!retp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  auto retp_ = reinterpret_cast<exrpc_ptr_t>(retp);
  return dummy_matrix(retp_,nrow,ncol);
}

dummy_matrix df_to_rowmajor_float(exrpc_ptr_t& df_proxy,
                                 std::vector<std::string>& t_cols) { 
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  auto mat = df.to_rowmajor_matrix_float(t_cols);
  auto nrow = mat.num_row;
  auto ncol = mat.num_col;
  auto retp = new rowmajor_matrix<float>(std::move(mat));
  if (!retp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  auto retp_ = reinterpret_cast<exrpc_ptr_t>(retp);
  return dummy_matrix(retp_,nrow,ncol);
}

dummy_matrix df_to_rowmajor_double(exrpc_ptr_t& df_proxy,
                                 std::vector<std::string>& t_cols) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  auto mat = df.to_rowmajor_matrix_double(t_cols);
  auto nrow = mat.num_row;
  auto ncol = mat.num_col;
  auto retp = new rowmajor_matrix<double>(std::move(mat));
  if (!retp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  auto retp_ = reinterpret_cast<exrpc_ptr_t>(retp);
  return dummy_matrix(retp_,nrow,ncol);
}

dummy_matrix df_to_crs_float(exrpc_ptr_t& df_proxy,
                             std::vector<std::string>& t_cols,
                             std::vector<std::string>& cat_cols,
                             long& info_id) {
  dftable_to_sparse_info info;
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  auto mat = df.to_crs_matrix_float(t_cols, cat_cols, info);
  auto nrow = mat.num_row;
  auto ncol = mat.num_col;
  auto matp = new crs_matrix<float>(std::move(mat));
  if (!matp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed for matrix.\n");
  auto matp_ = reinterpret_cast<exrpc_ptr_t>(matp);
  auto dmat = dummy_matrix(matp_,nrow,ncol);

  auto infop = new dftable_to_sparse_info(std::move(info));
  if (!infop) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed for info.\n");
  auto infop_ = reinterpret_cast<exrpc_ptr_t>(infop);
  // registering the sparse conversion info in model_table
  register_model(info_id, SPARSE_CONV_INFO, infop_);
  return dmat;
}

dummy_matrix df_to_crs_double(exrpc_ptr_t& df_proxy,
                              std::vector<std::string>& t_cols,
                              std::vector<std::string>& cat_cols,
                              long& info_id) {
  dftable_to_sparse_info info;
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  auto mat = df.to_crs_matrix_double(t_cols, cat_cols, info);
  auto nrow = mat.num_row;
  auto ncol = mat.num_col;
  auto matp = new crs_matrix<double>(std::move(mat));
  if (!matp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed for matrix.\n");
  auto matp_ = reinterpret_cast<exrpc_ptr_t>(matp);
  auto dmat = dummy_matrix(matp_,nrow,ncol);

  auto infop = new dftable_to_sparse_info(std::move(info));
  if (!infop) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed for info.\n");
  auto infop_ = reinterpret_cast<exrpc_ptr_t>(infop);
  // registering the sparse conversion info in model_table
  register_model(info_id, SPARSE_CONV_INFO, infop_);
  return dmat;
}

dummy_matrix df_to_crs_float_using_info(exrpc_ptr_t& df_proxy,
                                        long& info_id) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  auto& info = *get_model_ptr<dftable_to_sparse_info>(info_id);
  auto mat = df.to_crs_matrix_float(info);
  auto nrow = mat.num_row;
  auto ncol = mat.num_col;
  auto matp = new crs_matrix<float>(std::move(mat));
  if (!matp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed for matrix.\n");
  auto matp_ = reinterpret_cast<exrpc_ptr_t>(matp);
  return dummy_matrix(matp_,nrow,ncol);
}

dummy_matrix df_to_crs_double_using_info(exrpc_ptr_t& df_proxy,
                                         long& info_id) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  auto& info = *get_model_ptr<dftable_to_sparse_info>(info_id);
  auto mat = df.to_crs_matrix_double(info);
  auto nrow = mat.num_row;
  auto ncol = mat.num_col;
  auto matp = new crs_matrix<double>(std::move(mat));
  if (!matp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed for matrix.\n");
  auto matp_ = reinterpret_cast<exrpc_ptr_t>(matp);
  return dummy_matrix(matp_,nrow,ncol);
}

void load_sparse_conversion_info(long& info_id, std::string& dirname) {
  auto infop = new dftable_to_sparse_info();
  if (!infop) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed for info.\n");
  infop->load(dirname);
  auto infop_ = reinterpret_cast<exrpc_ptr_t>(infop);
  // registering the sparse conversion info in model_table
  register_model(info_id, SPARSE_CONV_INFO, infop_);
} 

void save_sparse_conversion_info(long& info_id, std::string& dirname) {
  auto& info = *get_model_ptr<dftable_to_sparse_info>(info_id);
  info.save(dirname);
}

void release_sparse_conversion_info(long& info_id) {
  if(!is_deleted(info_id)) {               // if not yet deleted, then
    deleted_model_tracker.insert(info_id); // mark as 'deleted'
    delete get_model_ptr<dftable_to_sparse_info>(info_id);  // get the model pointer and release it
    model_table.erase(info_id);            // remove it's entry from model_table
  }
  else std::cout << "[warning] Request for already deleted info[" << info_id << "].\n";
}
