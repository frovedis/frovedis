#include "exrpc_dataframe.hpp"
#include <boost/algorithm/string.hpp>

using namespace frovedis;

bool is_present(const std::vector<std::string>& vec,
                const std::string& val) {
  return std::find(vec.begin(), vec.end(), val) != vec.end();
}

dftable* get_dftable_pointer(exrpc_ptr_t& df_proxy) {
  dftable* dftblp = NULL;
  auto base_dftblp = reinterpret_cast<dftable_base*>(df_proxy);
  if(base_dftblp-> need_materialize()) {
    dftblp = new dftable(base_dftblp->materialize());
    if (!dftblp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  }
  else dftblp = reinterpret_cast<dftable*>(df_proxy);
  return dftblp;
}

void treat_str_nan_as_null(std::vector<std::string>& vec) {
  auto vsz = vec.size();
  auto vptr = vec.data();
  std::string nullstr = "NULL";
  for(size_t i = 0; i < vsz; ++i) {
    if (vptr[i] == "nan" || vptr[i] == "None") vptr[i] = nullstr;
  }
}

exrpc_ptr_t create_dataframe (std::vector<short>& types,
                              std::vector<std::string>& cols,
                              std::vector<exrpc_ptr_t>& dvec_proxies,
                              bool& nan_as_null) {
  //for(auto t: types) std::cout << t << " "; std::cout << std::endl;
  auto dftblp = new dftable();
  if (!dftblp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  for(size_t i=0; i<cols.size(); ++i) {
    switch(types[i]) {
      case INT:    { auto v1 = reinterpret_cast<dvector<int>*>(dvec_proxies[i]);
                     dftblp->append_column(cols[i],std::move(*v1),true);
                     delete v1; break; }
      case LONG:   { auto v2 = reinterpret_cast<dvector<long>*>(dvec_proxies[i]);
                     dftblp->append_column(cols[i],std::move(*v2),true);
                     delete v2; break; }
      case ULONG:  { auto v2 = reinterpret_cast<dvector<unsigned long>*>(dvec_proxies[i]);
                     dftblp->append_column(cols[i],std::move(*v2),true);
                     delete v2; break; }
      case FLOAT:  { auto v3 = reinterpret_cast<dvector<float>*>(dvec_proxies[i]);
                     if (nan_as_null) v3->mapv_partitions(treat_nan_as_null<float>);
                     dftblp->append_column(cols[i],std::move(*v3),true);
                     delete v3; break; }
      case DOUBLE: { auto v4 = reinterpret_cast<dvector<double>*>(dvec_proxies[i]);
                     if (nan_as_null) v4->mapv_partitions(treat_nan_as_null<double>);
                     dftblp->append_column(cols[i],std::move(*v4),true);
                     delete v4; break; }
      case STRING: { auto v5 = reinterpret_cast<dvector<std::string>*>(dvec_proxies[i]);
                     if (nan_as_null) v5->mapv_partitions(treat_str_nan_as_null);
                     dftblp->append_column(cols[i],std::move(*v5),true);
                     delete v5; break; }
      case BOOL:   { auto v6 = reinterpret_cast<dvector<int>*>(dvec_proxies[i]);
                     dftblp->append_column(cols[i],std::move(*v6),true);
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
  //for (auto& dt: dftblp->dtypes()) std::cout << dt.first << ": " << dt.second << "\n";
}

exrpc_ptr_t get_str_dfoperator(std::string& op1, std::string& op2,
                               short& op_id, bool& isImmed) {
  std::shared_ptr<dfoperator> *opt = NULL;
  // op2 would be treated as pattern, instead of immediate value in case of LIKE/NLIKE
  if(op_id == LIKE || op_id == NLIKE) isImmed = false;
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
      case LIKE: opt = new std::shared_ptr<dfoperator>(is_like(op1,op2)); break;
      case NLIKE: opt = new std::shared_ptr<dfoperator>(is_not_like(op1,op2)); break;
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

exrpc_ptr_t get_dfNOToperator(exrpc_ptr_t& opt_proxy) {
  auto opt = reinterpret_cast<std::shared_ptr<dfoperator>*>(opt_proxy);
  auto not_opt_ptr = new std::shared_ptr<dfoperator>(not_op(*opt));
  if (!not_opt_ptr) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  delete opt;
  return reinterpret_cast<exrpc_ptr_t> (not_opt_ptr);
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

exrpc_ptr_t isnull_df(exrpc_ptr_t& df_proxy,
                      std::vector<std::string>& cols,
                      bool& with_index) {
  auto& dftbl = *reinterpret_cast<dftable_base*>(df_proxy);
  auto ret = dftbl.isnull(cols);
  if (with_index) {
    auto icol = dftbl.columns()[0];
    use_dfcolumn use(dftbl.raw_column(icol));
    ret.append_column(icol, dftbl.column(icol)).change_col_position(icol, 0);
  }
  auto retp = new dftable(std::move(ret));
  if (!retp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  return reinterpret_cast<exrpc_ptr_t> (retp);
}

void drop_df_cols(exrpc_ptr_t& df_proxy,
                  std::vector<std::string>& cols) {
  auto& dftbl = *reinterpret_cast<dftable_base*>(df_proxy);
  dftbl.drop_cols(cols);
}

exrpc_ptr_t sort_df(exrpc_ptr_t& df_proxy,
                    std::vector<std::string>& cols,
                    std::vector<int>& isDesc) {
  auto size = cols.size();
  checkAssumption(size > 0 && isDesc.size() == size);
  auto& dftbl = *reinterpret_cast<dftable_base*>(df_proxy);
  sorted_dftable *ret = NULL;
  auto cname = cols[size - 1];
  auto desc = isDesc[size - 1];
  if(desc == 1) ret = new sorted_dftable(dftbl.sort_desc(cname));
  else          ret = new sorted_dftable(dftbl.sort(cname));
  for (int i = size - 2; i >= 0; --i) {
    cname = cols[i];
    desc = isDesc[i];
    if(desc) *ret = ret->sort_desc(cname);
    else     *ret = ret->sort(cname);
  }
  return reinterpret_cast<exrpc_ptr_t> (ret);
}

exrpc_ptr_t join_df(exrpc_ptr_t& left_proxy, 
                    exrpc_ptr_t& right_proxy,
                    exrpc_ptr_t& opt_proxy, 
                    std::string& how, 
                    std::string& join_type) {
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

std::shared_ptr<dfaggregator> 
get_aggr(std::string& funcname, 
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

exrpc_ptr_t 
frovedis_gdf_select(exrpc_ptr_t& df_proxy, 
                    std::vector<std::string>& tcols) {
  auto& df = *reinterpret_cast<grouped_dftable*>(df_proxy);
  auto retp = new dftable(df.select(tcols));
  if (!retp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  return reinterpret_cast<exrpc_ptr_t> (retp);
}

exrpc_ptr_t 
frovedis_gdf_aggr(exrpc_ptr_t& df_proxy, 
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
      case BOOL:
      case INT:  ret[i] = std::to_string(df.sum<int>(cols[i])); break;
      case LONG:  ret[i] = std::to_string(df.sum<long>(cols[i])); break;
      case FLOAT:  ret[i] = std::to_string(df.sum<float>(cols[i])); break;
      case DOUBLE:  ret[i] = std::to_string(df.sum<double>(cols[i])); break;
      case ULONG:  ret[i] = std::to_string(df.sum<unsigned long>(cols[i])); break;
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
      case BOOL:
      case INT:  ret[i] = std::to_string(df.min<int>(cols[i])); break;
      case LONG:  ret[i] = std::to_string(df.min<long>(cols[i])); break;
      case FLOAT:  ret[i] = std::to_string(df.min<float>(cols[i])); break;
      case DOUBLE:  ret[i] = std::to_string(df.min<double>(cols[i])); break;
      case ULONG:  ret[i] = std::to_string(df.min<unsigned long>(cols[i])); break;
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
      case BOOL:
      case INT:  ret[i] = std::to_string(df.max<int>(cols[i])); break;
      case LONG:  ret[i] = std::to_string(df.max<long>(cols[i])); break;
      case FLOAT:  ret[i] = std::to_string(df.max<float>(cols[i])); break;
      case DOUBLE:  ret[i] = std::to_string(df.max<double>(cols[i])); break;
      case ULONG:  ret[i] = std::to_string(df.max<unsigned long>(cols[i])); break;
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

std::vector<std::string>
frovedis_df_std(exrpc_ptr_t& df_proxy,
                 std::vector<std::string>& cols) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  std::vector<std::string> ret(cols.size());
  for (size_t i=0; i<cols.size(); ++i) {
    ret[i] = std::to_string(df.std(cols[i]));
  }
  return ret;
}

std::vector<std::string>
frovedis_df_var(exrpc_ptr_t& df_proxy,
                 std::vector<std::string>& cols) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  std::vector<std::string> ret(cols.size());
  for (size_t i=0; i<cols.size(); ++i) {
    ret[i] = std::to_string(df.var(cols[i]));
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
                               std::vector<std::string>& new_cols,
                               bool& inplace) {
  checkAssumption(cols.size() == new_cols.size());
  dftable_base* ret = reinterpret_cast<dftable_base*>(df_proxy);
  if (!inplace) ret = ret->clone();
  for(size_t i = 0; i < cols.size(); ++i) {
    ret->rename_cols(cols[i], new_cols[i]);
  }
  return reinterpret_cast<exrpc_ptr_t> (ret);
}

dummy_vector get_df_string_col(exrpc_ptr_t& df_proxy, 
                               std::string& cname) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  auto vecp = new dvector<std::string>(df.as_dvector<std::string>(cname));
  auto vecp_ = reinterpret_cast<exrpc_ptr_t>(vecp);
  return dummy_vector(vecp_, vecp->size(), STRING);
}

dummy_matrix df_to_colmajor_float(exrpc_ptr_t& df_proxy,
                                 std::vector<std::string>& t_cols) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  auto mat = df.to_colmajor_matrix_float(t_cols);
  auto retp = new colmajor_matrix<float>(std::move(mat));
  if (!retp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  return to_dummy_matrix<colmajor_matrix<float>, 
                         colmajor_matrix_local<float>>(retp);
}

dummy_matrix df_to_colmajor_double(exrpc_ptr_t& df_proxy,
                                 std::vector<std::string>& t_cols) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  auto mat = df.to_colmajor_matrix_double(t_cols);
  auto retp = new colmajor_matrix<double>(std::move(mat));
  if (!retp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  return to_dummy_matrix<colmajor_matrix<double>, 
                         colmajor_matrix_local<double>>(retp);
}

dummy_matrix df_to_rowmajor_float(exrpc_ptr_t& df_proxy,
                                 std::vector<std::string>& t_cols) { 
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  auto mat = df.to_rowmajor_matrix_float(t_cols);
  auto retp = new rowmajor_matrix<float>(std::move(mat));
  if (!retp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  return to_dummy_matrix<rowmajor_matrix<float>, 
                         rowmajor_matrix_local<float>>(retp);
}

dummy_matrix df_to_rowmajor_double(exrpc_ptr_t& df_proxy,
                                 std::vector<std::string>& t_cols) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  auto mat = df.to_rowmajor_matrix_double(t_cols);
  auto retp = new rowmajor_matrix<double>(std::move(mat));
  if (!retp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  return to_dummy_matrix<rowmajor_matrix<double>, 
                         rowmajor_matrix_local<double>>(retp);
}

dummy_matrix df_to_crs_float(exrpc_ptr_t& df_proxy,
                             std::vector<std::string>& t_cols,
                             std::vector<std::string>& cat_cols,
                             long& info_id) {
  dftable_to_sparse_info info;
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  auto mat = df.to_crs_matrix_float(t_cols, cat_cols, info);
  auto matp = new crs_matrix<float>(std::move(mat));
  if (!matp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed for matrix.\n");
  auto dmat = to_dummy_matrix<crs_matrix<float>, crs_matrix_local<float>>(matp);
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
  auto matp = new crs_matrix<double>(std::move(mat));
  if (!matp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed for matrix.\n");
  auto dmat = to_dummy_matrix<crs_matrix<double>, crs_matrix_local<double>>(matp);
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
  auto matp = new crs_matrix<float>(std::move(mat));
  if (!matp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed for matrix.\n");
  return to_dummy_matrix<crs_matrix<float>, crs_matrix_local<float>>(matp);
}

dummy_matrix df_to_crs_double_using_info(exrpc_ptr_t& df_proxy,
                                         long& info_id) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  auto& info = *get_model_ptr<dftable_to_sparse_info>(info_id);
  auto mat = df.to_crs_matrix_double(info);
  auto matp = new crs_matrix<double>(std::move(mat));
  if (!matp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed for matrix.\n");
  return to_dummy_matrix<crs_matrix<double>, crs_matrix_local<double>>(matp);
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

// multi-eq dfopt
exrpc_ptr_t frov_multi_eq_dfopt(std::vector<std::string>& left_cols, 
                                std::vector<std::string>& right_cols) {
  std::shared_ptr<dfoperator> *opt = NULL;
  checkAssumption(left_cols.size() == right_cols.size())
  // single key join
  if ( left_cols.size() == 1) {
    opt = new std::shared_ptr<dfoperator>(eq(left_cols[0], right_cols[0]));
  }
  else opt = new std::shared_ptr<dfoperator>(multi_eq(left_cols, right_cols));
  if (!opt) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  return reinterpret_cast<exrpc_ptr_t> (opt);
}

exrpc_ptr_t frov_cross_join_dfopt() {
  std::shared_ptr<dfoperator> *opt = \
   new std::shared_ptr<dfoperator>(frovedis::cross());
  if (!opt) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  return reinterpret_cast<exrpc_ptr_t> (opt);
}

dummy_dftable 
frov_load_dataframe_from_csv(std::string& filename,
                             std::vector<std::string>& types,
                             std::vector<std::string>& names,
                             bool& partial_type_info, 
                             std::map<std::string, std::string>& type_map,
                             std::vector<int>& usecols,
                             std::vector<std::string>& bool_cols,
                             csv_config& config,
                             bool& is_all_bools) {
  //config.debug_print();
  auto separator = config.separator;
  auto nullstr = config.nullstr;
  auto comment = config.comment;
  auto rows_to_see = config.rows_to_see;
  auto separate_mb = config.separate_mb;
  auto to_separate = config.to_separate;
  auto add_index = config.add_index;
  auto verbose_level = config.verbose_level;
  auto mangle_dupe_cols = config.mangle_dupe_cols;
  auto index_col = config.index_col;

  bool is_crlf = false;
  bool to_keep_order = true;
  set_verbose_level(verbose_level); 
  dftable *dftblp = NULL;

  if (partial_type_info){
    if ( names.empty() ) {
      dftblp = new dftable(make_dftable_loadtext_infertype(filename, type_map,
                                                           separator, nullstr,
                                                           rows_to_see, is_crlf,
                                                           to_separate, to_keep_order, 
                                                           separate_mb, usecols,
                                                           mangle_dupe_cols));
    }
    else {
      dftblp = new dftable(make_dftable_loadtext_infertype(filename, names, type_map,
                                                           separator, nullstr,
                                                           rows_to_see, is_crlf,
                                                           to_separate, to_keep_order, 
                                                           separate_mb, usecols,
                                                           mangle_dupe_cols));

    }
  }
  else { 
    if ( !types.empty() ) {
      if ( !names.empty() ) {
        dftblp = new dftable(make_dftable_loadtext(filename, types, names,
                                                  separator, nullstr,
                                                  is_crlf, to_separate, 
                                                  to_keep_order,
                                                  separate_mb, usecols, 
                                                  mangle_dupe_cols));
      }
      else {
        dftblp = new dftable(make_dftable_loadtext(filename, types,
                                                  separator, nullstr,
                                                  is_crlf, to_separate, 
                                                  to_keep_order,
                                                  separate_mb, usecols, 
                                                  mangle_dupe_cols));
      }
    }
    else {
      if ( !names.empty() ) {
        dftblp = new dftable(make_dftable_loadtext_infertype(filename, names,
                                                            separator, nullstr,
                                                            rows_to_see, is_crlf,
                                                            to_separate, to_keep_order,
                                                            separate_mb, usecols,
                                                            mangle_dupe_cols));

      }
      else {
        dftblp = new dftable(make_dftable_loadtext_infertype(filename,
                                                            separator, nullstr,
                                                            rows_to_see, is_crlf,
                                                            to_separate, to_keep_order,
                                                            separate_mb, usecols,
                                                            mangle_dupe_cols));
      }
    }
  }

  if (!dftblp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");

  auto cols = dftblp->columns();
  auto check_bool_like = true;
  if (is_all_bools) {
    for(auto& e: cols) {
      auto tmp_e = e + "_tmp";
      dftblp->rename(e, tmp_e);
      dftblp->type_cast(tmp_e, e, "boolean", check_bool_like);
      dftblp->drop(tmp_e);
    }
    dftblp->set_col_order(cols); // to retain initial order
  }
  else if (!bool_cols.empty()) {
    for (auto& e: bool_cols) {
      if (is_present(cols, e)) {
        auto tmp_e = e + "_tmp";
        dftblp->rename(e, tmp_e);
        dftblp->type_cast(tmp_e, e, "boolean", check_bool_like);
        dftblp->drop(tmp_e);
      }
    }
    dftblp->set_col_order(cols); // to retain initial order
  }

  if (index_col >= 0) dftblp->change_col_position(cols[index_col], 0); // set_index
  else if (add_index) dftblp->prepend_rowid<long>("index");
  reset_verbose_level();
  return to_dummy_dftable(dftblp);
}

size_t get_dataframe_length(exrpc_ptr_t& df_proxy) {
  auto dftblp = reinterpret_cast<dftable_base*>(df_proxy);
  return dftblp->num_row();
}

dummy_dftable
frov_df_append_column(exrpc_ptr_t& df_proxy, 
                      std::string& col_name, short& type, 
                      exrpc_ptr_t& dvec_proxy, int& position,
                      bool& drop_old, bool& nan_as_null) {
  dftable* dftblp = NULL;
  if (df_proxy == -1) dftblp = new dftable(); // empty dataframe
  else dftblp = get_dftable_pointer(df_proxy);

  auto df_col_order = dftblp->columns(); // original col_order
  auto sz = df_col_order.size();

  if (drop_old) dftblp->drop(col_name);
  switch(type) {
    case INT:    { auto v1 = reinterpret_cast<dvector<int>*>(dvec_proxy);
                   dftblp->append_column(col_name,std::move(*v1),true);
                   delete v1; break; }
    case LONG:   { auto v2 = reinterpret_cast<dvector<long>*>(dvec_proxy);
                   dftblp->append_column(col_name,std::move(*v2),true);
                   delete v2; break; }
    case ULONG:  { auto v2 = reinterpret_cast<dvector<unsigned long>*>(dvec_proxy);
                   dftblp->append_column(col_name,std::move(*v2),true);
                   delete v2; break; }
    case FLOAT:  { auto v3 = reinterpret_cast<dvector<float>*>(dvec_proxy);
                   if (nan_as_null) v3->mapv_partitions(treat_nan_as_null<float>);
                   dftblp->append_column(col_name,std::move(*v3),true);
                   delete v3; break; }
    case DOUBLE: { auto v4 = reinterpret_cast<dvector<double>*>(dvec_proxy);
                   if (nan_as_null) v4->mapv_partitions(treat_nan_as_null<double>);
                   dftblp->append_column(col_name,std::move(*v4),true);
                   delete v4; break; }
    case STRING: { auto v5 = reinterpret_cast<dvector<std::string>*>(dvec_proxy);
                   if (nan_as_null) v5->mapv_partitions(treat_str_nan_as_null);
                   dftblp->append_column(col_name,std::move(*v5),true);
                   delete v5; break; }
    case BOOL:   { auto v6 = reinterpret_cast<dvector<int>*>(dvec_proxy);
                   dftblp->append_column(col_name,std::move(*v6),true);
                   delete v6; break; }
    default:     auto msg = "frov_df_append_column: Unsupported datatype for append_column: "
                            + std::to_string(type);
                 REPORT_ERROR(USER_ERROR,msg);
  }
  if (drop_old) dftblp->set_col_order(df_col_order);
  if (df_proxy == -1) dftblp->prepend_rowid<long>("index");
  if (position != -1 and position != sz) {
    // position can be zero if index column is not present
    checkAssumption(position >= 0 && position < sz); 
    df_col_order.insert(df_col_order.begin() + position, 1, col_name);
    dftblp->set_col_order(df_col_order);
  }
  return to_dummy_dftable(dftblp);
}

dummy_dftable
frov_df_add_index(exrpc_ptr_t& df_proxy, 
                  std::string& name) {
  auto dftblp = get_dftable_pointer(df_proxy);
  dftblp->prepend_rowid<long>(name);
  return to_dummy_dftable(dftblp);
}

dummy_dftable
frov_df_reset_index(exrpc_ptr_t& df_proxy, 
                    bool& drop) { 
  auto dftblp = get_dftable_pointer(df_proxy);
  auto cols = dftblp->columns();
  checkAssumption(cols.size() > 0);

  if (drop) {
    dftblp->drop(cols[0]); // dropping existing index-column
    dftblp->prepend_rowid<long>("index");
  }
  else {
    if (cols[0] == "label_0")
      REPORT_ERROR(USER_ERROR, 
      "reset_index: exceeds maximum no. of index resetting!\n");
    std::string new_index_col;
    if (cols[0] == "index") {
      // renaming it as "index_col", since having column name as "index"
      // would cause issues in python wrapper -> self.index would be ambiguous
      dftblp->rename("index", "index_col");
      new_index_col = "label_0"; // as in pandas
    }
    else if (cols[0] == "index_col") new_index_col = "label_0"; // as in pandas
    else                             new_index_col = "index_col"; 
    dftblp->prepend_rowid<long>(new_index_col);
  }
  return to_dummy_dftable(dftblp);
}

dummy_dftable
frov_df_drop_duplicates(exrpc_ptr_t& df_proxy, 
                        std::vector<std::string>& cols,
                        std::string& keep) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  auto retp = new dftable(df.drop_duplicates(cols, keep));
  return to_dummy_dftable(retp);
}

dummy_dftable
frov_df_set_index(exrpc_ptr_t& df_proxy, 
                  std::string& cur_index_name, // existing index column
                  std::string& new_index_name, // existing column to be set as index
                  bool& verify_integrity) {
  auto dftblp = get_dftable_pointer(df_proxy);
  auto new_index_name_column = dftblp->column(new_index_name);
  use_dfcolumn use(new_index_name_column);
  if(verify_integrity and !(new_index_name_column->is_unique()))
    REPORT_ERROR(USER_ERROR, 
    "set_index: given column '" + new_index_name + 
    "' does not contain unique values!");
  if (cur_index_name != "") dftblp->drop(cur_index_name); //if self.has_index() is True
  dftblp->change_col_position(new_index_name, 0); //set_index
  return to_dummy_dftable(dftblp);
}

dummy_dftable
frov_df_union(exrpc_ptr_t& df_proxy, 
              std::vector<exrpc_ptr_t>& proxies,
              std::vector<std::string>& names, 
              bool& verify_integrity) {
  auto dftblp = get_dftable_pointer(df_proxy);
  auto sz = proxies.size();
  std::vector<dftable*> other_dfs(sz);
  for(size_t i = 0; i < sz; ++i) other_dfs[i] = get_dftable_pointer(proxies[i]);

  bool keep_order = true; // pandas keeps the original order
  auto union_df = new dftable(dftblp->union_tables(other_dfs, keep_order));
  auto index_column = union_df->column("index");
  use_dfcolumn use(index_column);
  if (verify_integrity && 
      (!index_column->is_unique())) // assumes "index" is present
    REPORT_ERROR(USER_ERROR, "append(): indices have overlapping values!\n");
  union_df->set_col_order(names); // sets desired column order
  return to_dummy_dftable(union_df);
}

dummy_dftable
frov_df_set_col_order(exrpc_ptr_t& df_proxy,
                      std::vector<std::string>& new_cols) {
  auto dftblp = get_dftable_pointer(df_proxy);
  dftblp->set_col_order(new_cols);
  return to_dummy_dftable(dftblp);
}

dummy_dftable
frov_df_astype(exrpc_ptr_t& df_proxy,
               std::vector<std::string>& cols,
               std::vector<short>& types,
               bool& check_bool_like_string) {
  checkAssumption(cols.size() == types.size());
  auto dftblp = get_dftable_pointer(df_proxy);
  auto org_col_order = dftblp->columns();
  for (size_t i = 0; i < cols.size(); ++i) {
    auto c = cols[i];
    auto cc = dftblp->column(c);
    use_dfcolumn use(cc);
    auto t = get_string_dtype(types[i]);
    if (cc->dtype() != t) {
      //std::cout << "col: " << c 
      //          << "; type: " << dftblp->column(c)->dtype() 
      //          << "; to-type: " << t << "\n";
      if (cc->dtype() == "dic_string") { // FUTURE TODO: might need to remove this check
        require(t == "boolean", 
        "type_cast: casting a string-typed column is supported only for bool!\n");
      }
      dftblp->rename(c, c + "__temp");
      dftblp->type_cast(c + "__temp", c, t, check_bool_like_string);
      dftblp->drop(c + "__temp"); 
    }
  }
  // during type-cast, original col-order changes due to drop and append
  dftblp->set_col_order(org_col_order); 
  return to_dummy_dftable(dftblp);
}

void copy_column_helper(dftable& to_df,
                        dftable_base& from_df,
                        std::string& cname,
                        std::string& cname_as,
                        short& dtype) {
  // if cname already exists in 'to_df', it would be replaced
  bool exist = false;
  auto col_order = to_df.columns();
  if (is_present(col_order, cname_as)) { 
    exist = true;
    to_df.drop(cname_as);
  }
  switch(dtype) {
    case INT:    to_df.append_column(cname_as, from_df.as_dvector<int>(cname), 
                                     true); break;
    case LONG:   to_df.append_column(cname_as, from_df.as_dvector<long>(cname),
                                     true); break;
    case ULONG:  to_df.append_column(cname_as, from_df.as_dvector<unsigned long>(cname),
                                     true); break;
    case FLOAT:  to_df.append_column(cname_as, from_df.as_dvector<float>(cname),
                                     true); break;
    case DOUBLE: to_df.append_column(cname_as, from_df.as_dvector<double>(cname),
                                     true); break;
    case STRING: to_df.append_column(cname_as, from_df.as_dvector<std::string>(cname),
                                     true); break;
    default: REPORT_ERROR(USER_ERROR, 
             std::string("copy: unsupported dtype, '") + STR(dtype) + 
             std::string("' encountered!"));
  }
  // retaining the position after copy, in case of existing column
  if (exist) to_df.set_col_order(col_order);
}

dummy_dftable 
frov_df_copy_index(exrpc_ptr_t& to_df, 
                   exrpc_ptr_t& from_df,
                   std::string& index_name,
                   short& dtype) { 
  auto to_df_p = get_dftable_pointer(to_df);
  auto from_df_p = reinterpret_cast<dftable_base*>(from_df);
  copy_column_helper(*to_df_p, *from_df_p, index_name, index_name, dtype);
  to_df_p->change_col_position(index_name, 0); //set_index
  return to_dummy_dftable(to_df_p);
}

dummy_dftable 
frov_df_copy_column(exrpc_ptr_t& to_df, 
                    exrpc_ptr_t& from_df,
                    std::vector<std::string>& names,
                    std::vector<std::string>& names_as,
                    std::vector<short>& dtypes) { 
  dftable* to_df_p = NULL;
  if (to_df == -1) to_df_p = new dftable(); // empty dataframe 
  else to_df_p = get_dftable_pointer(to_df);
  auto from_df_p = reinterpret_cast<dftable_base*>(from_df);
  auto size = names.size();
  checkAssumption(size == dtypes.size());
  for(size_t i = 0; i < size; ++i) {
    copy_column_helper(*to_df_p, *from_df_p, names[i], names_as[i], dtypes[i]);
  }
  if (to_df == -1) to_df_p->prepend_rowid<long>("index");
  return to_dummy_dftable(to_df_p);
}

void fillna_helper(dftable& ret, const std::string& cname, 
                   std::shared_ptr<dfcolumn>& dfcol, 
                   double fillv) {
  if (dfcol->dtype() == "int")
    ret.append_column(cname, fillna_column<int>(dfcol, fillv));
  else if (dfcol->dtype() == "unsigned int")
    ret.append_column(cname, fillna_column<unsigned int>(dfcol, fillv));
  else if (dfcol->dtype() == "long")
    ret.append_column(cname, fillna_column<long>(dfcol, fillv));
  else if (dfcol->dtype() == "unsigned long")
    ret.append_column(cname, fillna_column<unsigned long>(dfcol, fillv));
  else if (dfcol->dtype() == "float")
    ret.append_column(cname, fillna_column<float>(dfcol, fillv));
  else if (dfcol->dtype() == "double")
    ret.append_column(cname, fillna_column<double>(dfcol, fillv));
  else {
    REPORT_ERROR(USER_ERROR,
    "fillna: unsupported column type: " + dfcol->dtype());
  }
}

// TODO: make a function of dftable class
dftable fillna(dftable& df, 
               std::string& fill_value, 
               bool has_index) {
  dftable ret;
  size_t i = 0;
  auto cols = df.columns();
  if (has_index) { // fillna is not applied on index column
    i = 1;
    use_dfcolumn use(df.raw_column(cols[0]));
    ret.append_column(cols[0], df.column(cols[0]));
  }
  auto fillv = do_cast<double>(fill_value); // might raise exception
  for (; i < cols.size(); ++i) {
    use_dfcolumn use(df.raw_column(cols[i]));
    auto dfcol = df.column(cols[i]);
    fillna_helper(ret, cols[i], dfcol, fillv);
  }
  return ret;
}

dummy_dftable 
frov_df_fillna(exrpc_ptr_t& df_proxy, 
               std::string& fill_value, 
               bool& has_index) {
  auto& df = *get_dftable_pointer(df_proxy);
  auto ret = new dftable(fillna(df, fill_value, has_index));
  return to_dummy_dftable(ret);
}

std::string frov_df_to_string(exrpc_ptr_t& df_proxy, bool& has_index) {
  auto df = reinterpret_cast<dftable_base*>(df_proxy);
  return df->to_string(!has_index);
}

dummy_dftable
frov_df_dropna_by_rows(exrpc_ptr_t& df_proxy,
                       std::vector<std::string>& targets,
                       std::string& how,
                       size_t& threshold) {
  auto df = reinterpret_cast<dftable_base*>(df_proxy);
  dftable ret;
  if (threshold == std::numeric_limits<size_t>::max()) {
    ret = df->drop_nulls_by_rows(how, targets);
  } else {
    ret = df->drop_nulls_by_rows(threshold, targets);
  }
  auto retp = new dftable(std::move(ret));
  return to_dummy_dftable(retp);
}

dummy_dftable frov_df_head(exrpc_ptr_t& df_proxy,
                           size_t& limit) {
  auto dftblp = reinterpret_cast<dftable_base*>(df_proxy);
  auto ret = new dftable(dftblp->head(limit));
  return to_dummy_dftable(ret);
}

dummy_dftable frov_df_tail(exrpc_ptr_t& df_proxy,
                           size_t& limit) {
  auto dftblp = reinterpret_cast<dftable_base*>(df_proxy);
  auto ret = new dftable(dftblp->tail(limit));
  return to_dummy_dftable(ret);
}

dummy_dftable frov_df_slice_range(exrpc_ptr_t& df_proxy, 
                                  size_t& a, size_t& b,
                                  size_t& c) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  auto ret = new dftable(make_sliced_dftable(df, a, b, c));
  return to_dummy_dftable(ret);
}

std::vector<size_t> 
frov_df_get_index_loc(exrpc_ptr_t& df_proxy, std::string& column,
                      std::string& value, short& dtype) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  std::vector<size_t> res;
  switch(dtype) {
    case INT:    res = df.get_loc(column, do_cast<int>(value)); break;
    case BOOL:   res = df.get_loc(column, (int) do_cast<bool>(value)); break;
    case LONG:   res = df.get_loc(column, do_cast<long>(value)); break;
    case ULONG:  res = df.get_loc(column, do_cast<unsigned long>(value)); break;
    case FLOAT:  res = df.get_loc(column, do_cast<float>(value)); break;
    case DOUBLE: res = df.get_loc(column, do_cast<double>(value)); break;
    case STRING: res = df.get_loc(column, value); break;
    default:     auto msg = "get_loc: Unsupported datatype: " + 
                            std::to_string(dtype) + "! \n";
                 REPORT_ERROR(USER_ERROR, msg);
  }
  return res;
}

dummy_dftable 
frov_df_countna(exrpc_ptr_t& df_proxy, int& axis, bool& with_index) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  auto ret = new dftable(df.count_nulls(axis, with_index));
  return to_dummy_dftable(ret);
}

dummy_dftable frov_df_ksort(exrpc_ptr_t& df_proxy, int& k,
                            std::vector<std::string>& targets,
                            std::string& keep,
                            bool& is_desc) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  auto ret = is_desc ? df.nlargest(k, targets, keep)
                     : df.nsmallest(k, targets, keep);
  auto retp = new dftable(std::move(ret));
  return to_dummy_dftable(retp);
}

// TODO: move the implementation in dataframe library
dftable frov_df_mean_impl(exrpc_ptr_t& df_proxy, 
                          std::vector<std::string>& cols,
                          int& axis, bool& skip_na, 
                          bool& with_index) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  std::string index_nm = "index";
  if (with_index) index_nm = df.columns()[0]; // 0th column is always treated as index

  dftable ret;
  auto ncol = cols.size();
  if (axis == 0) { // simply calculates the avg of each column one-by-one
    std::vector<double> mean_res(ncol);
    if (skip_na) {
      for (size_t i = 0; i < ncol; ++i) mean_res[i] = df.avg(cols[i]);
    } else {
      auto tmax = std::numeric_limits<double>::max();
      for (size_t i = 0; i < ncol; ++i) {
        auto cname = cols[i];
        use_dfcolumn use(df.raw_column(cname));
        if (df.column(cname)->if_contain_nulls()) mean_res[i] = tmax;
        else mean_res[i] = df.avg(cname);
      }
    }
    ret.append_column("index", make_dvector_scatter(cols)); 
    ret.append_column("mean", make_dvector_scatter(mean_res), true); // may contain null 
  } 
  else if (axis == 1) {
    if (ncol == 0 ) append_null<double>(ret, "mean", df.num_row());
    else {
      dftable tmp;
      double fillv = 0;
      // --- calculate sum(axis = 1) ---
      std::string old_sum = "old_sum";
      use_dfcolumn use(df.raw_column(cols[0]));
      auto dfcol = df.column(cols[0]);
      if (skip_na) fillna_helper(tmp, old_sum, dfcol, fillv); // replaces nulls with fillv (if any) and append in 'tmp'
      else tmp.append_column(old_sum, dfcol); // appends as it is in 'tmp' (no copy)
      for (size_t i = 1; i < ncol; ++i) {
        use_dfcolumn use_col(df.raw_column(cols[i]));
        dfcol = df.column(cols[i]);
        if (skip_na) fillna_helper(tmp, cols[i], dfcol, fillv); // replaces nulls with fillv (if any) and append in 'tmp'
        else tmp.append_column(cols[i], dfcol); // appends as it is in 'tmp' (no copy)
        auto func = frovedis::add_col(old_sum, cols[i]);
        use_dfcolumn use_func(func->columns_to_use(tmp)); 
        auto rescol = func->execute(tmp);
        // replace old_sum with new sum: old_sum += col_i
        tmp.drop(cols[i]);
        tmp.drop(old_sum);
        tmp.append_column(old_sum, rescol); // old_sum becomes new_sum (rescol)
      }
      if (skip_na) {
        // --- calculate row-wise non-na values ---
        auto countna_df = df.select(cols).count_nulls(axis);
        auto subt_fn = frovedis::sub_im(ncol, "count"); 
        use_dfcolumn use_sub(subt_fn->columns_to_use(countna_df));
        countna_df.append_column("count_non_na", subt_fn->execute(countna_df)); // count_non_na = ncol - count
        // --- calculate mean --- 
        auto mean_fn = frovedis::fdiv_col(old_sum, "count_non_na"); // mean = sum / count_non_na
        use_dfcolumn use_mean(mean_fn->columns_to_use(tmp, countna_df));
        ret.append_column("mean", mean_fn->execute(tmp, countna_df));
      } else {
        // --- calculate mean --- 
        auto mean_fn = frovedis::fdiv_im(old_sum, ncol); // mean = sum / ncol
        use_dfcolumn use_mean(mean_fn->columns_to_use(tmp));
        ret.append_column("mean", mean_fn->execute(tmp));
      }
    } 
    // use index as it is in input dataframe, if any. otherwise add index.
    if (with_index) {
      use_dfcolumn use(df.raw_column(index_nm));
      ret.append_column(index_nm, df.column(index_nm)).change_col_position(index_nm, 0);
    } else {
      ret.prepend_rowid<long>(index_nm);
    }
  } 
  else REPORT_ERROR(USER_ERROR, "mean: supported axis: 0 and 1 only!\n");
  return ret;
}

dummy_dftable frov_df_mean(exrpc_ptr_t& df_proxy, 
                           std::vector<std::string>& cols,
                           int& axis, bool& skip_na, 
                           bool& with_index) {
  auto ret = frov_df_mean_impl(df_proxy, cols, axis, skip_na, with_index);
  auto retp = new dftable(std::move(ret));
  return to_dummy_dftable(retp);
}

// TODO: move the implementation in dataframe library
dftable frov_df_var_impl(exrpc_ptr_t& df_proxy, 
                         std::vector<std::string>& cols,
                         int& axis, bool& skip_na, 
                         bool& with_index) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  std::string index_nm = "index";
  if (with_index) index_nm = df.columns()[0]; // 0th column is always treated as index

  dftable ret;
  std::string mean_str = "mean", sum_str = "sum", sub_str = "sub", sq_str = "square";
  auto ncol = cols.size();
  if (axis == 0) {
    std::vector<double> var_res(ncol);
    if (skip_na) {
      for (size_t i = 0; i < ncol; ++i) var_res[i] = df.var(cols[i]);
    } else {
      auto tmax = std::numeric_limits<double>::max();
      for (size_t i = 0; i < ncol; ++i) {
        auto cname = cols[i];
        use_dfcolumn use(df.raw_column(cname));
        if (df.column(cname)->if_contain_nulls()) var_res[i] = tmax;
        else var_res[i] = df.var(cname);
      }
    }
    ret.append_column("index", make_dvector_scatter(cols)); 
    ret.append_column("var", make_dvector_scatter(var_res), true); // may contain null 
  }
  else if (axis == 1) {
    if (ncol == 0 ) append_null<double>(ret, "var", df.num_row());
    else {
      double fillv = 0;
      auto tmp = frov_df_mean_impl(df_proxy, cols, axis, skip_na, with_index);
      //auto tmp = *reinterpret_cast<dftable*>(dummy_df.dfptr);
      use_dfcolumn use_(df.raw_column(cols[0]));
      auto dfcol = df.column(cols[0]);
      if (skip_na) fillna_helper(tmp, sum_str, dfcol, fillv); // replaces nulls with fillv (if any) and append in 'tmp'
      else tmp.append_column(sum_str, dfcol); //tmp:mean,sum
      //set sum_str to zeros
      auto func = frovedis::mul_im(sum_str, 0);
      use_dfcolumn use_func_mul(func->columns_to_use(tmp)); 
      auto sum_0 = func->execute(tmp);
      tmp.drop(sum_str); //tmp:mean
      tmp.append_column(sum_str, sum_0); //tmp:mean,sum
      for (size_t i = 0; i < ncol; ++i) {
        use_dfcolumn use_col(df.raw_column(cols[i]));
        dfcol = df.column(cols[i]);
        tmp.append_column(cols[i], dfcol);//tmp:mean,sum,cols[i]
        auto func = frovedis::sub_col(mean_str, cols[i]);
        use_dfcolumn use_func_sub(func->columns_to_use(tmp)); 
        auto rescol = func->execute(tmp);
        tmp.drop(cols[i]);//tmp:mean,sum
        if (skip_na) fillna_helper(tmp, sub_str, rescol, fillv); // replaces nulls with fillv (if any) and append in 'tmp'
        else tmp.append_column(sub_str, rescol); //tmp:mean,sum,sub
        func = frovedis::pow_im(sub_str, 2);
        use_dfcolumn use_func_pow(func->columns_to_use(tmp)); 
        rescol = func->execute(tmp);
        tmp.drop(sub_str);//tmp:mean,sum
        tmp.append_column(sq_str, rescol);//tmp:mean,sum,square
        func = frovedis::add_col(sum_str, sq_str);
        use_dfcolumn use_func_add(func->columns_to_use(tmp)); 
        rescol = func->execute(tmp);
        tmp.drop(sum_str);//tmp:mean,square
        tmp.drop(sq_str);//tmp:mean
        tmp.append_column(sum_str, rescol);//tmp:mean,sum
      }
      tmp.drop(mean_str);//tmp:sum
      if (skip_na) {
        // --- calculate row-wise non-na values ---
        auto countna_df = df.select(cols).count_nulls(axis);
        auto subt_fn = frovedis::sub_im(ncol - 1, "count"); //using 'ncol = ncol - 1' for variance
        use_dfcolumn use_sub(subt_fn->columns_to_use(countna_df));
        countna_df.append_column("count_non_na", subt_fn->execute(countna_df)); // count_non_na = ncol - count
        // --- calculate variance --- 
        auto var_fn = frovedis::fdiv_col(sum_str, "count_non_na"); // variance = sum / count_non_na
        use_dfcolumn use_var(var_fn->columns_to_use(tmp, countna_df));
        ret.append_column("var", var_fn->execute(tmp, countna_df));
      } else {
        // --- calculate variance --- 
        auto var_fn = frovedis::fdiv_im(sum_str, ncol - 1); // variance = sum / ncol - 1
        use_dfcolumn use_var(var_fn->columns_to_use(tmp));
        ret.append_column("var", var_fn->execute(tmp));
      }
    }
    // use index as it is in input dataframe, if any. otherwise add index.
    if (with_index) {
      use_dfcolumn use(df.raw_column(index_nm));
      ret.append_column(index_nm, df.column(index_nm)).change_col_position(index_nm, 0);
    } else {
      ret.prepend_rowid<long>(index_nm);
    }
  }
  else REPORT_ERROR(USER_ERROR, "variance: supported axis: 0 and 1 only!\n");
  return ret;
}

dummy_dftable frov_df_var(exrpc_ptr_t& df_proxy, 
                           std::vector<std::string>& cols,
                           int& axis, bool& skip_na, 
                           bool& with_index) {
  auto ret = frov_df_var_impl(df_proxy, cols, axis, skip_na, with_index);
  auto retp = new dftable(std::move(ret));
  return to_dummy_dftable(retp);
}

dummy_dftable frov_df_std(exrpc_ptr_t& df_proxy, 
                           std::vector<std::string>& cols,
                           int& axis, bool& skip_na, 
                           bool& with_index) {
  dftable ret;
  std::string var_str = "var", std_str = "std";
  auto ncol = cols.size();
  
  if ((axis == 0) || (axis == 1)) {
    ret = frov_df_var_impl(df_proxy, cols, axis, skip_na, with_index);
    if (ncol > 0 ) {
      use_dfcolumn use_var_str(ret.raw_column(var_str));
      auto func = frovedis::pow_im(var_str, 0.5); //square root
      use_dfcolumn use_func_sub(func->columns_to_use(ret)); 
      auto rescol = func->execute(ret);
      ret.drop(var_str);//ret:<>
      ret.append_column(std_str, rescol); //ret:std
    }
    else {
      ret.rename_cols(var_str, std_str);
    }
  }
  else REPORT_ERROR(USER_ERROR, "standard deviation: supported axis: 0 and 1 only!\n");
  auto retp = new dftable(std::move(ret));
  return to_dummy_dftable(retp);
}
