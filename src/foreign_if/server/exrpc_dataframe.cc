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

exrpc_ptr_t create_dataframe (std::vector<short>& types,
                              std::vector<std::string>& cols,
                              std::vector<exrpc_ptr_t>& dvec_proxies) {
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
                     dftblp->append_column(cols[i],std::move(*v3),true);
                     delete v3; break; }
      case DOUBLE: { auto v4 = reinterpret_cast<dvector<double>*>(dvec_proxies[i]);
                     dftblp->append_column(cols[i],std::move(*v4),true);
                     delete v4; break; }
      case STRING: { auto v5 = reinterpret_cast<dvector<std::string>*>(dvec_proxies[i]);
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
                      std::vector<std::string>& cols) {
  auto& dftbl = *reinterpret_cast<dftable_base*>(df_proxy);
  auto r_df_ptr = new dftable(dftbl.isnull(cols));
  if (!r_df_ptr) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  return reinterpret_cast<exrpc_ptr_t> (r_df_ptr);
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

dummy_vector get_df_int_col(exrpc_ptr_t& df_proxy, 
                                std::string& cname) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  auto vecp = new dvector<int>(df.as_dvector<int>(cname));
  auto vecp_ = reinterpret_cast<exrpc_ptr_t>(vecp);
  return dummy_vector(vecp_, vecp->size(), INT);
}

dummy_vector get_df_long_col(exrpc_ptr_t& df_proxy, 
                             std::string& cname) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  auto vecp = new dvector<long>(df.as_dvector<long>(cname));
  auto vecp_ = reinterpret_cast<exrpc_ptr_t>(vecp);
  return dummy_vector(vecp_, vecp->size(), LONG);
}

dummy_vector get_df_ulong_col(exrpc_ptr_t& df_proxy, 
                              std::string& cname) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  auto vecp = new dvector<unsigned long>(df.as_dvector<unsigned long>(cname));
  auto vecp_ = reinterpret_cast<exrpc_ptr_t>(vecp);
  return dummy_vector(vecp_, vecp->size(), ULONG);
}

dummy_vector get_df_float_col(exrpc_ptr_t& df_proxy, 
                              std::string& cname) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  auto vecp = new dvector<float>(df.as_dvector<float>(cname));
  auto vecp_ = reinterpret_cast<exrpc_ptr_t>(vecp);
  return dummy_vector(vecp_, vecp->size(), FLOAT);
}

dummy_vector get_df_double_col(exrpc_ptr_t& df_proxy, 
                               std::string& cname) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  auto vecp = new dvector<double>(df.as_dvector<double>(cname));
  auto vecp_ = reinterpret_cast<exrpc_ptr_t>(vecp);
  return dummy_vector(vecp_, vecp->size(), DOUBLE);
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
exrpc_ptr_t frov_multi_eq_dfopt(std::vector<std::string>& left_cols, std::vector<std::string>& right_cols) {
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

std::vector<int>
dicstring_id_to_bool(std::vector<size_t>& vec, std::vector<int>& bool_val_map){
  auto sz = vec.size();
  std::vector<int> ret(sz);
  for(size_t i=0; i<sz; i++) ret[i] = bool_val_map[vec[i]];
  return ret;
}

void convert_bool_column_dictring(dftable& df, const std::string& col_name,
                                  const std::string& nullstr){

  auto dfcol = df.column(col_name);
  use_dfcolumn use(dfcol);
  auto typed_dfcol = std::dynamic_pointer_cast<typed_dfcolumn<dic_string>>(dfcol);

  auto nl_dict = *typed_dfcol->dic;
  auto dict1 = nl_dict.get(0);

  auto nwords = dict1.num_words();
  std::vector<size_t> indices1(nwords);
  for(size_t i=0; i<nwords; i++) indices1[i] = i;
  auto all_words = dict1.index_to_words(indices1);
  auto str_vec = words_to_vector_string(all_words);

  std::vector<int> bool_val_map(nwords);
  auto nullstr_lower = boost::algorithm::to_lower_copy(nullstr);
  auto int_max = std::numeric_limits<int>::max();

  for(size_t i=0; i<nwords; i++) {
    boost::algorithm::to_lower(str_vec[i]);
    if(str_vec[i] == "true") bool_val_map[i] = 1;
    else if (str_vec[i] == "false") bool_val_map[i] = 0;
    else if (str_vec[i] == nullstr_lower) bool_val_map[i] = int_max;
    else {
      std::string msg = "parse-error: invalid value " + str_vec[i] + " for boolean conversion!\n";
      REPORT_ERROR(USER_ERROR, msg);
    }
  }

  auto new_dvec = typed_dfcol->val
                            .map(dicstring_id_to_bool, broadcast(bool_val_map))
                            .as_dvector<int>();
  auto original_col_order = df.columns();
  df.drop(col_name).append_column(col_name, new_dvec, true);
  df.set_col_order(original_col_order);
}

dummy_dftable 
frov_load_dataframe_from_csv(std::string& filename,
                             std::vector<std::string>& types,
                             std::vector<std::string>& names,
                             bool& partial_type_info, 
                             std::map<std::string, std::string>& type_map,
                             std::vector<int>& usecols,
                             std::vector<std::string>& bool_cols,
                             csv_config& config) {
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

  if ( !bool_cols.empty()){
    auto cols = dftblp->columns();
    for (auto& e: bool_cols){
      auto it = std::find (cols.begin(), cols.end(), e);
      if (it != cols.end()) {
        convert_bool_column_dictring(*dftblp, e, nullstr);
      }
    }
  }

  if (index_col >= 0) {
    auto cols = dftblp->columns();
    dftblp->set_index(cols[index_col]);
  }
  else if (add_index) dftblp->add_index("index");
  reset_verbose_level();
  return to_dummy_dftable(dftblp);
}

size_t get_dataframe_length(exrpc_ptr_t& df_proxy) {
  auto dftblp = reinterpret_cast<dftable_base*>(df_proxy);
  return dftblp->num_row();
}

dummy_dftable
frov_df_convert_dicstring_to_bool(exrpc_ptr_t& df_proxy,
                                  std::vector<std::string>& col_names,
                                  std::string& nullstr) { 
  auto dftblp = get_dftable_pointer(df_proxy); 
  auto cols = dftblp->columns();
  for (auto& e: col_names){
    auto it = std::find (cols.begin(), cols.end(), e);
    if (it != cols.end()) {
      convert_bool_column_dictring(*dftblp, e, nullstr);
    }
    else REPORT_ERROR(USER_ERROR, "frov_df_convert_dicstring_to_bool: given"
                                  " column does not exist!\n");
  }
  return to_dummy_dftable(dftblp);
}

dummy_dftable
frov_df_append_column(exrpc_ptr_t& df_proxy, 
                      std::string& col_name, short& type, 
                      exrpc_ptr_t& dvec_proxy, int& position,
                      bool& drop_old) {
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
                   dftblp->append_column(col_name,std::move(*v3),true);
                   delete v3; break; }
    case DOUBLE: { auto v4 = reinterpret_cast<dvector<double>*>(dvec_proxy);
                   dftblp->append_column(col_name,std::move(*v4),true);
                   delete v4; break; }
    case STRING: { auto v5 = reinterpret_cast<dvector<std::string>*>(dvec_proxy);
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
  if (df_proxy == -1) dftblp->add_index("index");
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
  dftblp->add_index(name);
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
    dftblp->add_index("index");
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
    dftblp->add_index(new_index_col);
  }
  return to_dummy_dftable(dftblp);
}

dummy_dftable
frov_df_drop_duplicates(exrpc_ptr_t& df_proxy, 
                        std::vector<std::string>& cols,
                        std::string& keep) {
  auto dftblp = get_dftable_pointer(df_proxy);
  auto retp = new dftable(dftblp->drop_duplicates(cols, keep));
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
  dftblp->set_index(new_index_name);
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
               std::vector<short>& types) {
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
      if (cc->dtype() == "dic_string")
        REPORT_ERROR(USER_ERROR, 
        "type_cast: casting a string-typed column is not supported!\n");
      dftblp->rename(c, c + "__temp");
      dftblp->type_cast(c + "__temp", c, t);
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
  to_df_p->set_index(index_name);
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
  if (to_df == -1) to_df_p->add_index("index");
  return to_dummy_dftable(to_df_p);
}

// TODO: make a function of dftable class
dftable fillna(dftable& df, 
               std::string& fill_value, 
               bool has_index) {
  dftable ret;
  size_t i = 0;
  auto cols = df.columns();
  if (has_index) {
    i = 1;
    ret.append_column(cols[0], df.column(cols[0]));
  }
  auto fillv = do_cast<double>(fill_value); // might raise exception
  for (; i < cols.size(); ++i) {
    auto dfcol = df.column(cols[i]);
    use_dfcolumn use(dfcol);
    if (dfcol->dtype() == "int")
      ret.append_column(cols[i], fillna_column<int>(dfcol, fillv));
    else if (dfcol->dtype() == "unsigned int")
      ret.append_column(cols[i], fillna_column<unsigned int>(dfcol, fillv));
    else if (dfcol->dtype() == "long")
      ret.append_column(cols[i], fillna_column<long>(dfcol, fillv));
    else if (dfcol->dtype() == "unsigned long")
      ret.append_column(cols[i], fillna_column<unsigned long>(dfcol, fillv));
    else if (dfcol->dtype() == "float")
      ret.append_column(cols[i], fillna_column<float>(dfcol, fillv));
    else if (dfcol->dtype() == "double")
      ret.append_column(cols[i], fillna_column<double>(dfcol, fillv));
    else {
      REPORT_ERROR(USER_ERROR,
      "fillna: unsupported column type: " + dfcol->dtype());
    }
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
