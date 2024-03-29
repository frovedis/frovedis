#include "exrpc_dataframe.hpp"
#include <boost/algorithm/string.hpp>
#include "exrpc_dffunction.hpp"

using namespace frovedis;

dftable
fillna_with_min(dftable& df, std::vector<std::string>& cols, 
                std::vector<short>& types) {
  dftable ret;
  auto ncol = df.num_col();
  for (size_t i = 0; i < ncol; ++i) {
    use_dfcolumn use_col(df.raw_column(cols[i]));
    auto dfcol = df.column(cols[i]);
    fillna_column_min_typed_helper(dfcol, types[i]);
    ret.append_column(cols[i], dfcol);
  }
  return ret;
}

void fillna_column_min_typed_helper(
                         std::shared_ptr<frovedis::dfcolumn>& col,
                         short type) {
  switch(type) {
    case BOOL:
    case INT:    col = fillna_column<int>(col, 
                         std::numeric_limits<int>::min());break;
    case DATETIME:
    case TIMEDELTA:
    case LONG:   col = fillna_column<long>(col, 
                         std::numeric_limits<long>::min());break;
    case FLOAT:  col = fillna_column<float>(col, 
                         std::numeric_limits<float>::min());break;
    case DOUBLE: col = fillna_column<double>(col, 
                         std::numeric_limits<double>::min());break;
    case ULONG:  col = fillna_column<unsigned long>(col, 
                         std::numeric_limits<unsigned long>::min());break;
    default:     REPORT_ERROR(USER_ERROR,
                              "Not supported for non-numeric columns!\n");
  }
}

void fillna_column_max_typed_helper(
                         std::shared_ptr<frovedis::dfcolumn>& col, 
                         short type) {
  switch(type) {
    case BOOL:
    case INT:    col = fillna_column<int>(col, 
                              std::numeric_limits<int>::max());break;
    case DATETIME:
    case TIMEDELTA:
    case LONG:   col = fillna_column<long>(col,
                              std::numeric_limits<long>::max());break;
    case FLOAT:  col = fillna_column<float>(col,
                              std::numeric_limits<float>::max());break;
    case DOUBLE: col = fillna_column<double>(col,
                              std::numeric_limits<double>::max());break;
    case ULONG:  col = fillna_column<unsigned long>(col,
                              std::numeric_limits<unsigned long>::max());break;
    default:     REPORT_ERROR(USER_ERROR, 
                              "Not supported for non-numeric columns!\n");
  }
}

dftable
fillna_with_max(dftable& df, std::vector<std::string>& cols, 
                std::vector<short>& types) {
  dftable ret;
  auto ncol = df.num_col();
  for (size_t i = 0; i < ncol; ++i) {
    use_dfcolumn use_col(df.raw_column(cols[i]));
    auto dfcol = df.column(cols[i]);
    fillna_column_max_typed_helper(dfcol, types[i]);
    ret.append_column(cols[i], dfcol);
  }
  return ret;
}

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

// TODO: improve vectorization
void treat_str_nan_as_null(std::vector<std::string>& vec) {
  auto vsz = vec.size();
  auto vptr = vec.data();
  std::string nullstr = "NULL";
  for(size_t i = 0; i < vsz; ++i) {
    if (vptr[i] == "nan" || vptr[i] == "None" || 
        vptr[i] == "<NA>") vptr[i] = nullstr;
  }
}

void treat_words_nan_as_null(words& w) {
  std::string nullstr = "NULL";
  w.replace("nan", nullstr);
  w.replace("None", nullstr);
  w.replace("<NA>", nullstr);
}

std::vector<std::shared_ptr<dffunction>>
to_dffunction(std::vector<exrpc_ptr_t>& funcp) {
  auto size = funcp.size();
  std::vector<std::shared_ptr<dffunction>> func(size);
  for(size_t i = 0; i < size; ++i) func[i] = *reinterpret_cast<std::shared_ptr<dffunction>*>(funcp[i]);
  return func;
}

std::vector<std::shared_ptr<dfaggregator>>
to_dfaggregator(std::vector<exrpc_ptr_t>& aggp) {
  auto size = aggp.size();
  std::vector<std::shared_ptr<dfaggregator>> agg(size);
  for(size_t i = 0; i < size; ++i) agg[i] = *reinterpret_cast<std::shared_ptr<dfaggregator>*>(aggp[i]);
  return agg;
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
      case TIMEDELTA:
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
      case WORDS:  { auto v7 = reinterpret_cast<node_local<words>*>(dvec_proxies[i]);
                     if (nan_as_null) v7->mapv(treat_words_nan_as_null);
                     dftblp->append_dic_string_column(cols[i],(*v7),true);
                     delete v7; break; }
      case TIMESTAMP:
      case DATETIME: { auto v8 = reinterpret_cast<dvector<datetime_t>*>(dvec_proxies[i]);
                     dftblp->append_datetime_column(cols[i],std::move(*v8),true);
                     delete v8; break; }
      default:     auto msg = "Unsupported datatype in dataframe creation: " + std::to_string(types[i]);
                   REPORT_ERROR(USER_ERROR,msg);
    }
  }
  return reinterpret_cast<exrpc_ptr_t>(dftblp);
}

// for spark -> frovedis dataframe load
exrpc_ptr_t create_dataframe_from_local_vectors (
            std::vector<short>& types,
            std::vector<std::string>& cols,
            std::vector<exrpc_ptr_t>& local_vec_proxies,
            std::vector<int>& offset) {
  auto nproc = get_nodesize();
  std::vector<exrpc_ptr_t> proxies(nproc); auto pp = proxies.data();
  std::vector<exrpc_ptr_t> word_sizep(nproc); auto sp = word_sizep.data();
  auto proxy_matptr = local_vec_proxies.data();
  bool do_align = true; // seems to be required as of now for performance
  //bool do_align = false; // not required during dvector creation, would be taken care by append_column()

  auto dftblp = new dftable();
  if (!dftblp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  short wid = WORDS;

  for(size_t i = 0; i < cols.size(); ++i) {
    auto row_offset = offset[i];
    //std::cout << "merging partitions for " << i << "th column with offset: " << row_offset << std::endl;

    if (types[i] == wid) { // special handling
      auto vp1 = proxy_matptr + row_offset * nproc;       // proxy of char-data-vectors
      auto vp2 = proxy_matptr + (row_offset + 1) * nproc; // proxy of int-size-vectors
      for (size_t j = 0; j < nproc; ++j) { 
        pp[j] = vp1[j];
        sp[j] = vp2[j];
      }
      auto dv = make_node_local_words_impl(proxies, word_sizep, do_align);
      dftblp->append_dic_string_column(cols[i], dv, true);
    } else {
      auto vp = proxy_matptr + row_offset * nproc;
      for (size_t j = 0; j < nproc; ++j) pp[j] = vp[j];
      switch(types[i]) {
        case BOOL:
        case INT:    { auto dv = merge_and_get_dvector_impl<int>(proxies, do_align);
                       dftblp->append_column(cols[i], std::move(dv), true);
                       break; }
        case TIMEDELTA:
        case LONG:   { auto dv = merge_and_get_dvector_impl<long>(proxies, do_align);
                       dftblp->append_column(cols[i], std::move(dv), true);
                       break; }
        case FLOAT:  { auto dv = merge_and_get_dvector_impl<float>(proxies, do_align);
                       dftblp->append_column(cols[i], std::move(dv), true);
                       break; }
        case DOUBLE: { auto dv = merge_and_get_dvector_impl<double>(proxies, do_align);
                       dftblp->append_column(cols[i], std::move(dv), true);
                       break; }
        case STRING: { auto dv = merge_and_get_dvector_impl<std::string>(proxies, do_align);
                       dftblp->append_column(cols[i], std::move(dv), true);
                       break; }
        case TIMESTAMP:
        case DATETIME: {auto dv = merge_and_get_dvector_impl<datetime_t>(proxies, do_align);
                        dftblp->append_datetime_column(cols[i], std::move(dv), true);
                        break; }
        default:  auto msg = "Unsupported datatype in dataframe creation: " + std::to_string(types[i]);
                  REPORT_ERROR(USER_ERROR,msg);
      }
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
      case EQ: opt = new std::shared_ptr<dfoperator>(eq_im(op1,op2)); break;
      case NE: opt = new std::shared_ptr<dfoperator>(neq_im(op1,op2)); break;
      case GT: opt = new std::shared_ptr<dfoperator>(gt_im(op1,op2)); break;
      case GE: opt = new std::shared_ptr<dfoperator>(ge_im(op1,op2)); break;
      case LT: opt = new std::shared_ptr<dfoperator>(lt_im(op1,op2)); break;
      case LE: opt = new std::shared_ptr<dfoperator>(le_im(op1,op2)); break;
      default: REPORT_ERROR(USER_ERROR,
               "Unsupported operation on string type column is encountered!\n");
    }
  }
  else {
    switch(op_id) {
      case EQ: opt = new std::shared_ptr<dfoperator>(eq(op1,op2)); break;
      case NE: opt = new std::shared_ptr<dfoperator>(neq(op1,op2)); break;
      case GT: opt = new std::shared_ptr<dfoperator>(gt(op1,op2)); break;
      case GE: opt = new std::shared_ptr<dfoperator>(ge(op1,op2)); break;
      case LT: opt = new std::shared_ptr<dfoperator>(lt(op1,op2)); break;
      case LE: opt = new std::shared_ptr<dfoperator>(le(op1,op2)); break;
      case LIKE: opt = new std::shared_ptr<dfoperator>(is_like(op1,op2)); break;
      case NLIKE: opt = new std::shared_ptr<dfoperator>(is_not_like(op1,op2)); break;
      default: REPORT_ERROR(USER_ERROR,
               "Unsupported operation on string type column is encountered!\n");
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
  return reinterpret_cast<exrpc_ptr_t> (and_opt_ptr);
}

exrpc_ptr_t get_dfORoperator(exrpc_ptr_t& lopt_proxy,
                             exrpc_ptr_t& ropt_proxy) {
  auto lopt = reinterpret_cast<std::shared_ptr<dfoperator>*>(lopt_proxy);
  auto ropt = reinterpret_cast<std::shared_ptr<dfoperator>*>(ropt_proxy);
  auto or_opt_ptr = new std::shared_ptr<dfoperator>(or_op(*lopt,*ropt));
  if (!or_opt_ptr) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  return reinterpret_cast<exrpc_ptr_t> (or_opt_ptr);
}

exrpc_ptr_t get_dfNOToperator(exrpc_ptr_t& opt_proxy) {
  auto opt = reinterpret_cast<std::shared_ptr<dfoperator>*>(opt_proxy);
  auto not_opt_ptr = new std::shared_ptr<dfoperator>(not_op(*opt));
  if (!not_opt_ptr) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
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

dummy_dftable fselect_df(exrpc_ptr_t& df_proxy,
                         std::vector<exrpc_ptr_t>& funcp) {
  auto& dftbl = *reinterpret_cast<dftable_base*>(df_proxy);
  auto func = to_dffunction(funcp);
  auto s_df_ptr = new dftable(dftbl.fselect(func));
  if (!s_df_ptr) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  return to_dummy_dftable(s_df_ptr);
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
                    std::string& join_type,
                    bool& check_opt_proxy,
                    std::string& rsuf) {
  auto& left = *reinterpret_cast<dftable_base*>(left_proxy);
  auto& right = *reinterpret_cast<dftable_base*>(right_proxy);
  auto dfopt = *reinterpret_cast<std::shared_ptr<dfoperator>*>(opt_proxy);
  if (check_opt_proxy) dfopt = dfopt->modify_right(rsuf)->rectify_order(left, right);

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

exrpc_ptr_t fgroup_by_df(exrpc_ptr_t& df_proxy, 
                         std::vector<exrpc_ptr_t>& funcp) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  auto func = to_dffunction(funcp);
  grouped_dftable *g_df_ptr = new grouped_dftable(df.fgroup_by(func));
  if (!g_df_ptr) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  return reinterpret_cast<exrpc_ptr_t> (g_df_ptr);
}

std::shared_ptr<dfaggregator> 
get_aggr(const std::string& funcname, 
         const std::string& col, 
         const std::string& as_col) {
  std::shared_ptr<dfaggregator> ret;
  if (funcname == "sum")        ret = sum_as(col,as_col);
  else if (funcname == "avg" || 
           funcname == "mean")  ret = avg_as(col,as_col);
  else if (funcname == "min")   ret = min_as(col,as_col);
  else if (funcname == "max")   ret = max_as(col,as_col);
  else if (funcname == "count") ret = count_as(col,as_col);
  else if (funcname == "nunique") ret = count_distinct_as(col,as_col);
  else if (funcname == "size")  ret = size_as(col,as_col);
  else if (funcname == "var")   ret = var_as(col,as_col);
  else if (funcname == "sem")   ret = sem_as(col,as_col);
  else if (funcname == "std")   ret = std_as(col,as_col);
  else if (funcname == "mad")   ret = mad_as(col,as_col); 
  else if (funcname == "first")   ret = first_as(col,as_col); 
  else if (funcname == "last")   ret = last_as(col,as_col); 
  else REPORT_ERROR(USER_ERROR,"Unsupported aggregation function is requested!\n");
  return ret;
}

std::shared_ptr<dfaggregator> 
get_aggr(const std::string& funcname, 
         const std::string& col, 
         const std::string& as_col,
         double& ddof) {
  std::shared_ptr<dfaggregator> ret;
  if (funcname == "var")        ret = var_as(col, as_col, ddof);
  else if (funcname == "sem")   ret = sem_as(col, as_col, ddof);
  else if (funcname == "std")   ret = std_as(col, as_col, ddof);
  else REPORT_ERROR(USER_ERROR,"Unsupported aggregation function is requested!\n");
  return ret;
}

exrpc_ptr_t 
frovedis_gdf_select(exrpc_ptr_t& df_proxy, 
                    std::vector<std::string>& tcols) {
  auto& gdf = *reinterpret_cast<grouped_dftable*>(df_proxy);
  auto retp = new dftable(gdf.select(tcols));
  if (!retp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  return reinterpret_cast<exrpc_ptr_t> (retp);
}

dummy_dftable
frovedis_gdf_fselect(exrpc_ptr_t& df_proxy,
                     std::vector<exrpc_ptr_t>& funcp) {
  auto& gdf = *reinterpret_cast<grouped_dftable*>(df_proxy);
  auto func = to_dffunction(funcp);
  auto s_df_ptr = new dftable(gdf.fselect(func));
  if (!s_df_ptr) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  return to_dummy_dftable(s_df_ptr);
}

dummy_dftable 
frovedis_gdf_agg_select(exrpc_ptr_t& df_proxy,
                        std::vector<std::string>& cols,
                        std::vector<exrpc_ptr_t>& aggp) {
  auto& gdf = *reinterpret_cast<grouped_dftable*>(df_proxy);
  auto agg = to_dfaggregator(aggp);
  auto retp = new dftable(gdf.select(cols, agg));
  if (!retp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  return to_dummy_dftable(retp);
}

dummy_dftable 
frovedis_gdf_agg_fselect(exrpc_ptr_t& df_proxy,
                         std::vector<exrpc_ptr_t>& funcp,
                         std::vector<exrpc_ptr_t>& aggp) {
  auto& gdf = *reinterpret_cast<grouped_dftable*>(df_proxy);
  auto func = to_dffunction(funcp);
  auto agg = to_dfaggregator(aggp);
  auto retp = new dftable(gdf.fselect(func, agg));
  if (!retp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  return to_dummy_dftable(retp);
}

void convert_float_nan_to_null(dftable& df) {
  auto cols = df.columns();
  for (size_t i = 1; i < cols.size(); ++i) { // excluding index column: 0th column
    auto dfcol = df.column(cols[i]);
    use_dfcolumn use(dfcol);
    if (dfcol->dtype() == "float") {
      auto c1 = std::dynamic_pointer_cast<typed_dfcolumn<float>>(dfcol);
      auto nans = c1->val.map(treat_nan_as_null_helper<float>, c1->nulls);
      auto nansz = nans.template viewas_dvector<size_t>().size();
      if (nansz) c1->contain_nulls = true; 
    }
    else if (dfcol->dtype() == "double") {
      auto c1 = std::dynamic_pointer_cast<typed_dfcolumn<double>>(dfcol);
      auto nans = c1->val.map(treat_nan_as_null_helper<double>, c1->nulls);
      auto nansz = nans.template viewas_dvector<size_t>().size();
      if (nansz) c1->contain_nulls = true; 
    }
  }
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
  convert_float_nan_to_null(*retp);
  if (!retp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  return reinterpret_cast<exrpc_ptr_t> (retp);
}

dummy_dftable 
frovedis_gdf_aggr_with_ddof(exrpc_ptr_t& df_proxy,
                          std::vector<std::string>& groupedCols,
                          std::string& aggFunc,
                          std::vector<std::string>& aggCols,
                          std::vector<std::string>& aggAsCols,
                          double& ddof) {
  auto& gdf = *reinterpret_cast<grouped_dftable*>(df_proxy);
  auto size = aggCols.size();
  std::vector<std::shared_ptr<dfaggregator>> agg(size);
  for(size_t i=0; i<size; ++i) {
    agg[i] = get_aggr(aggFunc, aggCols[i], aggAsCols[i], ddof);
  }
  auto retp = new dftable(gdf.select(groupedCols, agg));
  convert_float_nan_to_null(*retp);
  if (!retp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  return to_dummy_dftable(retp);
}

dummy_dftable
frovedis_gdf_aggr_with_mincount(exrpc_ptr_t& df_proxy,
                                std::vector<std::string>& groupedCols,
                                std::string& aggFunc,
                                std::vector<std::string>& aggCols,
                                std::vector<std::string>& aggAsCols,
                                int& mincount) {
  auto& gdf = *reinterpret_cast<grouped_dftable*>(df_proxy);
  auto size = aggCols.size();

  dftable ret;
  std::vector<std::shared_ptr<dfaggregator>> agg;
  if (mincount <= 0) {
    agg.resize(size);
    for(size_t i = 0; i < size; ++i) {
      agg[i] = get_aggr(aggFunc, aggCols[i], aggAsCols[i]);
    }
    ret = gdf.select(groupedCols, agg);
  } else {
    agg.resize(size * 2);
    std::string tmp = "__temp__", cnt = "__count__";
    for(size_t i = 0; i < size; ++i) {
      agg[i * 2 + 0] = get_aggr(aggFunc, aggCols[i], aggAsCols[i] + tmp);
      agg[i * 2 + 1] = get_aggr("count", aggCols[i], aggCols[i] + cnt);
    }
    ret = gdf.select(groupedCols, agg);
    auto gcol_size = groupedCols.size();
    std::vector<std::shared_ptr<dffunction>> func(gcol_size + size);
    for(size_t i = 0; i < gcol_size; ++i) func[i] = ~groupedCols[i];
    for(size_t i = 0; i < size; ++i) {
      auto count_col = aggCols[i] + cnt;
      auto tcol = aggAsCols[i] + tmp;
      func[gcol_size + i] = when({(~count_col >= mincount) >> ~tcol})->as(aggAsCols[i]);
    }
    ret = ret.fselect(func);
  }
  
  auto retp = new dftable(std::move(ret));
  convert_float_nan_to_null(*retp);
  if (!retp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  return to_dummy_dftable(retp);
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
frovedis_df_sem(exrpc_ptr_t& df_proxy,
                 std::vector<std::string>& cols) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  std::vector<std::string> ret(cols.size());
  for (size_t i=0; i<cols.size(); ++i) {
    ret[i] = std::to_string(df.sem(cols[i]));
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
frovedis_df_mad(exrpc_ptr_t& df_proxy,
                std::vector<std::string>& cols) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  std::vector<std::string> ret(cols.size());
  for (size_t i=0; i<cols.size(); ++i) {
    ret[i] = std::to_string(df.mad(cols[i]));
  }
  return ret;
}

std::vector<std::string>
frovedis_df_median(exrpc_ptr_t& df_proxy,
                   std::vector<std::string>& cols,
                   std::vector<short>& types) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  std::vector<std::string> ret(cols.size());
  for (size_t i=0; i<cols.size(); ++i) {
    switch(types[i]) {
      case BOOL:   
      case INT:    ret[i] = std::to_string(df.median<int>(cols[i]));break;
      case LONG:   ret[i] = std::to_string(df.median<long>(cols[i]));break;
      case FLOAT:  ret[i] = std::to_string(df.median<float>(cols[i]));break;
      case DOUBLE: ret[i] = std::to_string(df.median<double>(cols[i]));break;
      case ULONG:  ret[i] = std::to_string(df.median<unsigned long>(cols[i]));break;
      default:     REPORT_ERROR(USER_ERROR, "median on non-numeric column!\n");
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
    case TIMEDELTA:
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
    case WORDS:  { auto v7 = reinterpret_cast<node_local<words>*>(dvec_proxy);
                   if (nan_as_null) v7->mapv(treat_words_nan_as_null);
                   dftblp->append_dic_string_column(col_name,(*v7),true);
                   delete v7; break; }
    case TIMESTAMP:
    case DATETIME: { auto v8 = reinterpret_cast<dvector<datetime_t>*>(dvec_proxy);
                     dftblp->append_datetime_column(col_name,std::move(*v8),true);
                     delete v8; break; }
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
               std::vector<std::string>& fmt,
               bool& check_bool_like_string) {
  checkAssumption(cols.size() == types.size());
  auto dftblp = get_dftable_pointer(df_proxy);
  auto org_col_order = dftblp->columns();
  for (size_t i = 0; i < cols.size(); ++i) {
    auto c = cols[i];
    auto cc = dftblp->column(c);
    use_dfcolumn use(cc);
    auto t = get_string_dtype(types[i]);
    if (fmt[i] != "" && t == "datetime")
        t = t + ":" + fmt[i];
    if (cc->dtype() != t) {
      //std::cout << "col: " << c 
      //          << "; type: " << dftblp->column(c)->dtype() 
      //          << "; to-type: " << t << "\n";
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
    case BOOL:
    case INT:    to_df.append_column(cname_as, from_df.as_dvector<int>(cname), 
                                     true); break;
    case DATETIME:   to_df.append_datetime_column(
                                    cname_as, 
                                    from_df.as_dvector<datetime_t>(cname),
                                    true); break;
    case TIMEDELTA:
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
  else if (dfcol->dtype() == "datetime")
    ret.append_column(cname, fillna_column<datetime_t>(dfcol, fillv));
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
  auto& df = *get_dftable_pointer(df_proxy);
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
    case DATETIME:
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

dftable calc_non_na_vals(dftable_base& df, 
                         std::vector<std::string>& cols, 
                         int& axis, bool& with_index, 
                         size_t& ncol, std::string col_name = "count_non_na") {
  dftable countna_df;
  auto sz = cols.size();
  if (with_index) {
    std::vector<std::string> cols2(sz + 1);
    cols2[0] = df.columns()[0];
    for (size_t i = 1; i <= sz; ++i) cols2[i] = cols[i - 1];
    //for (size_t i = 0; i <= sz; ++i) std::cout << cols2[i] << " ";
    //std::cout << std::endl;
    countna_df = df.select(cols2).count_nulls(axis, with_index);
  }
  else countna_df = df.select(cols).count_nulls(axis, with_index);
  auto subt_fn = frovedis::sub_im(ncol, "count");
  use_dfcolumn use_sub(subt_fn->columns_to_use(countna_df));
  // count_non_na = ncol - count
  countna_df.append_column(col_name, subt_fn->execute(countna_df));
  return countna_df;
}

// TODO: move the implementation in dataframe library
dftable frov_df_mean_impl(exrpc_ptr_t& df_proxy, 
                          std::vector<std::string>& cols,
                          int& axis, bool& skip_na, 
                          bool& with_index) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  std::string index_nm = "index";
  // 0th column is always treated as index
  if (with_index) index_nm = df.columns()[0];

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
    ret.append_column("mean", 
                      make_dvector_scatter(mean_res), true); // may contain null 
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
      // replaces nulls with fillv (if any) and append in 'tmp'
      if (skip_na) fillna_helper(tmp, old_sum, dfcol, fillv);
      // appends as it is in 'tmp' (no copy)
      else tmp.append_column(old_sum, dfcol);
      for (size_t i = 1; i < ncol; ++i) {
        use_dfcolumn use_col(df.raw_column(cols[i]));
        dfcol = df.column(cols[i]);
        // replaces nulls with fillv (if any) and append in 'tmp'
        if (skip_na) fillna_helper(tmp, cols[i], dfcol, fillv);
        // appends as it is in 'tmp' (no copy)
        else tmp.append_column(cols[i], dfcol);
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
        auto countna_df = calc_non_na_vals(df, cols, axis, with_index, 
                                           ncol, "count_non_na");
        // --- calculate mean --- 
        auto mean_fn = frovedis::fdiv_col(old_sum, 
                                  "count_non_na"); // mean = sum / count_non_na
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
      ret.append_column(index_nm, df.column(index_nm))
         .change_col_position(index_nm, 0);
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

void replace_le_zero(std::vector<double>& res_col) {
  auto pos = vector_find_le(res_col, 0.);
  auto pos_sz = pos.size();
  auto posp = pos.data();
  auto resp = res_col.data();
  auto dmax = std::numeric_limits<double>::max();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for (size_t i = 0; i < pos_sz; ++i) resp[posp[i]] = dmax;
}

// TODO: move the implementation in dataframe library
dftable frov_df_var_impl(exrpc_ptr_t& df_proxy, 
                         std::vector<std::string>& cols,
                         int& axis, bool& skip_na, double& ddof,
                         bool& with_index) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  std::string index_nm = "index";
  if (with_index) index_nm = df.columns()[0]; 

  dftable ret;
  std::string mean_str = "mean", sum_str = "sum";
  std::string sub_str = "sub", sq_str = "square";
  auto ncol = cols.size();
  if (axis == 0) {
    std::vector<double> var_res(ncol);
    if (skip_na) {
      for (size_t i = 0; i < ncol; ++i) var_res[i] = df.var(cols[i], ddof);
    } else {
      auto tmax = std::numeric_limits<double>::max();
      for (size_t i = 0; i < ncol; ++i) {
        auto cname = cols[i];
        use_dfcolumn use(df.raw_column(cname));
        if (df.column(cname)->if_contain_nulls()) var_res[i] = tmax;
        else var_res[i] = df.var(cname, ddof);
      }
    }
    ret.append_column("index", make_dvector_scatter(cols)); 
    ret.append_column("var", 
                      make_dvector_scatter(var_res), true); // may contain null 
  }
  else if (axis == 1) {
    if (ncol == 0 ) append_null<double>(ret, "var", df.num_row());
    else {
      double fillv = 0;
      auto tmp = frov_df_mean_impl(df_proxy, cols, axis, skip_na, with_index);
      auto sum_col = tmp.as_dvector<double>(mean_str);
      sum_col.mapv(+[](double& x){ //sum col with zeros
                   x = 0;});
      tmp.append_column(sum_str, sum_col); //tmp:mean,sum
      for (size_t i = 0; i < ncol; ++i) {
        use_dfcolumn use_col(df.raw_column(cols[i]));
        auto dfcol = df.column(cols[i]);
        tmp.append_column(cols[i], dfcol);//tmp:mean,sum,cols[i]
        auto func = frovedis::sub_col(mean_str, cols[i]);
        use_dfcolumn use_func_sub(func->columns_to_use(tmp)); 
        auto rescol = func->execute(tmp);
        tmp.drop(cols[i]);//tmp:mean,sum
        // replaces nulls with fillv (if any) and append in 'tmp'
        if (skip_na) fillna_helper(tmp, sub_str, rescol, fillv);
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
      double n_ddof = ncol - ddof;
      if (n_ddof > 0) {
        if (skip_na) {
          // --- calculate row-wise non-na values ---
          auto countna_df = calc_non_na_vals(df, cols, 
                                             axis, with_index, 
                                             ncol, "count_non_na");
          auto subt_fn = frovedis::sub_im("count_non_na", ddof);
          use_dfcolumn use_sub(subt_fn->columns_to_use(countna_df));
          auto res_col = subt_fn->execute(countna_df); // ncol - count -ddof
          countna_df.append_column("count_non_na_ddof", res_col);
          countna_df.drop("count_non_na");
          auto col_ddof_dv = countna_df.as_dvector<double>("count_non_na_ddof");
          auto col_ddof = col_ddof_dv.viewas_node_local();
          col_ddof.mapv(replace_le_zero);
          countna_df.append_column("count_non_na", col_ddof_dv, true);
          countna_df.drop("count_non_na_ddof");
          auto var_fn = frovedis::fdiv_col(sum_str, 
                               "count_non_na"); // variance = sum / count_non_na
          use_dfcolumn use_var(var_fn->columns_to_use(tmp, countna_df));
          ret.append_column("var", var_fn->execute(tmp, countna_df));
        } else {
          // --- calculate variance --- 
          auto var_fn = frovedis::fdiv_im(sum_str, 
                                       n_ddof); // variance = sum / ncol - ddof
          use_dfcolumn use_var(var_fn->columns_to_use(tmp));
          ret.append_column("var", var_fn->execute(tmp));
        }
      }
      else {
        std::vector<double> var_data(df.num_row());
        for (size_t i =0; i < df.num_row(); ++i) {
          var_data[i] = std::numeric_limits<double>::quiet_NaN();
        }
        ret.append_column("var", make_dvector_scatter(var_data), true);
      }
    }
    // use index as it is in input dataframe, if any. otherwise add index.
    if (with_index) {
      use_dfcolumn use(df.raw_column(index_nm));
      ret.append_column(index_nm, df.column(index_nm))
         .change_col_position(index_nm, 0);
    } else {
      ret.prepend_rowid<long>(index_nm);
    }
  }
  else REPORT_ERROR(USER_ERROR, "variance: supported axis: 0 and 1 only!\n");
  return ret;
}

dummy_dftable frov_df_var(exrpc_ptr_t& df_proxy, 
                          std::vector<std::string>& cols,
                          int& axis, bool& skip_na, double& ddof,
                          bool& with_index) {
  auto ret = frov_df_var_impl(df_proxy, cols, axis, skip_na, ddof, with_index);
  auto retp = new dftable(std::move(ret));
  return to_dummy_dftable(retp);
}


dftable frov_df_std_impl(exrpc_ptr_t& df_proxy, 
                               std::vector<std::string>& cols,
                               int& axis, bool& skip_na, double& ddof,
                               bool& with_index) {
  dftable ret;
  std::string var_str = "var", std_str = "std";
  auto ncol = cols.size();
  
  if ((axis == 0) || (axis == 1)) {
    ret = frov_df_var_impl(df_proxy, cols, axis, skip_na, ddof, with_index);
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
  else REPORT_ERROR(USER_ERROR, 
                    "standard deviation: supported axis: 0 and 1 only!\n");  
  return ret;
}

dummy_dftable frov_df_std(exrpc_ptr_t& df_proxy, 
                          std::vector<std::string>& cols,
                          int& axis, bool& skip_na, double& ddof,
                          bool& with_index) {
  auto ret = frov_df_std_impl(df_proxy, cols, axis, skip_na, ddof, with_index);
  auto retp = new dftable(std::move(ret));
  return to_dummy_dftable(retp);
}

dftable frov_df_sem_impl(exrpc_ptr_t& df_proxy, 
                           std::vector<std::string>& cols,
                           int& axis, bool& skip_na, double& ddof,
                           bool& with_index) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  dftable ret;
  std::string std_str = "std", sem_str = "sem";
  auto ncol = cols.size();
  size_t count;
  ret = frov_df_std_impl(df_proxy, cols, axis, skip_na, ddof, with_index);
  if (ncol > 0 ) {
    if (axis == 0) {
      use_dfcolumn use_count_non_na(ret.raw_column(std_str));
      if (skip_na) {
        std::vector<double> count(ncol);
        for (size_t i = 0; i < ncol; ++i) {
          auto cname = cols[i];
          use_dfcolumn use(df.raw_column(cname));
          count[i] = std::sqrt(df.column(cname)->count());
        }
        ret.append_column("count_non_na_root", 
                          make_dvector_scatter(count), true);
        auto func = frovedis::fdiv_col(std_str, "count_non_na_root");
        use_dfcolumn use_func_sub(func->columns_to_use(ret)); 
        ret.append_column(sem_str, func->execute(ret));
        ret.drop(std_str);
        ret.drop("count_non_na_root");
      }
      else {
        count = df.column(cols[0])->size();
        // --- calculate sem --- 
        auto sem_fn = frovedis::fdiv_im(std_str, 
                              std::sqrt(count)); // variance = sum / (count)^0.5
        use_dfcolumn use_sem(sem_fn->columns_to_use(ret));
        ret.append_column(sem_str, sem_fn->execute(ret));
        ret.drop(std_str);
      }
    }
    else if (axis == 1) {
      if (skip_na) {
        // --- calculate row-wise non-na values ---
        auto countna_df = calc_non_na_vals(df, cols, axis, with_index, 
                                           ncol, "count_non_na");
        use_dfcolumn use_count_non_na(countna_df.raw_column("count_non_na"));
        auto func = frovedis::pow_im("count_non_na", 0.5); //square root
        use_dfcolumn use_func_sqrt(func->columns_to_use(countna_df)); 
        auto rescol = func->execute(countna_df);
        countna_df.drop("count_non_na");
        countna_df.append_column("count_non_na", rescol); 
        // --- calculate sem --- 
        auto sem_fn = frovedis::fdiv_col(std_str, 
                              "count_non_na"); // sem = std / (count_non_na)^0.5
        use_dfcolumn use_sem(sem_fn->columns_to_use(ret, countna_df));
        ret.append_column(sem_str, sem_fn->execute(ret, countna_df));
        ret.drop(std_str);
      } else {
        // --- calculate sem --- 
        auto sem_fn = frovedis::fdiv_im(std_str, 
                                std::sqrt(ncol)); // variance = sum / (ncol)^0.5
        use_dfcolumn use_sem(sem_fn->columns_to_use(ret));
        ret.append_column(sem_str, sem_fn->execute(ret));
        ret.drop(std_str);
      }
    }
    else REPORT_ERROR(USER_ERROR, 
                     "standard error of mean: supported axis: 0 and 1 only!\n");
  }
  else {
    ret.rename_cols(std_str, sem_str);
  }
  return ret;
}

dummy_dftable frov_df_sem(exrpc_ptr_t& df_proxy, 
                          std::vector<std::string>& cols,
                          int& axis, bool& skip_na, double& ddof,
                          bool& with_index) {
  auto ret = frov_df_sem_impl(df_proxy, cols, axis, skip_na, ddof, with_index);
  auto retp = new dftable(std::move(ret));
  return to_dummy_dftable(retp);
}



dftable frov_df_median_impl(exrpc_ptr_t& df_proxy, 
                            std::vector<std::string>& cols,
                            std::vector<short>& types,
                            int& axis, bool& skip_na, 
                            bool& with_index) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  std::string index_nm = "index";
  // 0th column is always treated as index
  if (with_index) index_nm = df.columns()[0];

  dftable ret;
  std::string median_str = "median";
  auto ncol = cols.size();
  if (axis == 0) {
    std::vector<double> median_res(ncol);
    if (skip_na) {
      for (size_t i = 0; i < ncol; ++i) {
        switch(types[i]) {
          case BOOL:   
          case INT:    median_res[i] = df.median<int>(cols[i]);break;
          case DATETIME:   
          case TIMEDELTA:   
          case LONG:   median_res[i] = df.median<long>(cols[i]); break;
          case FLOAT:  median_res[i] = df.median<float>(cols[i]); break;
          case DOUBLE: median_res[i] = df.median<double>(cols[i]); break;
          case ULONG:  median_res[i] = df.median<unsigned long>(cols[i]); break;
          default:     REPORT_ERROR(USER_ERROR, 
                                    "median on non-numeric column!\n");
        }
      }
    } else {
      auto tmax = std::numeric_limits<double>::max();
      for (size_t i = 0; i < ncol; ++i) {
        auto cname = cols[i];
        use_dfcolumn use(df.raw_column(cname));
        if (df.column(cname)->if_contain_nulls()) median_res[i] = tmax;
        else {
          switch(types[i]) {
            case BOOL:   
            case INT:    median_res[i] = df.median<int>(cname);break;
            case DATETIME:   
            case TIMEDELTA:   
            case LONG:   median_res[i] = df.median<long>(cname); break;
            case FLOAT:  median_res[i] = df.median<float>(cname); break;
            case DOUBLE: median_res[i] = df.median<double>(cname); break;
            case ULONG:  median_res[i] = df.median<unsigned long>(cname); break;
            default:     REPORT_ERROR(USER_ERROR, 
                                      "median on non-numeric column!\n");
          }
        }
      }
    }
    ret.append_column("index", make_dvector_scatter(cols)); 
    ret.append_column(median_str, 
                      make_dvector_scatter(median_res), 
                      true); // may contain null 
  }
  else if (axis == 1) {
    if (ncol == 0 ) append_null<double>(ret, "median", df.num_row());
    else {
      auto rmat = df.to_rowmajor_matrix_double(cols);
      std::vector<double> median;
      auto nan_count_dv = df.select(cols).count_nulls(axis, with_index)
                            .as_dvector<size_t>("count");
      median = matrix_median_by_rows(rmat, nan_count_dv, skip_na, true);
      ret.append_column(median_str, make_dvector_scatter(median), true);
    }
    if (with_index) {
      use_dfcolumn use(df.raw_column(index_nm));
      ret.append_column(index_nm, 
                        df.column(index_nm)).change_col_position(index_nm, 0);
    } else {
      ret.prepend_rowid<long>(index_nm);
    }
  }
  else REPORT_ERROR(USER_ERROR, "variance: supported axis: 0 and 1 only!\n");
  return ret;
}

dummy_dftable frov_df_median(exrpc_ptr_t& df_proxy, 
                           std::vector<std::string>& cols,
                           std::vector<short>& types,
                           int& axis, bool& skip_na, 
                           bool& with_index) {
  auto ret = frov_df_median_impl(df_proxy, cols, types, 
                                 axis, skip_na, with_index);
  auto retp = new dftable(std::move(ret));
  return to_dummy_dftable(retp);
}

//Mean Absolute Deviation
//summation(|Xi - m(x)|)/n
dftable frov_df_mad_impl(exrpc_ptr_t& df_proxy, 
                         std::vector<std::string>& cols,
                         int& axis, bool& skip_na,
                         bool& with_index) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  std::string index_nm = "index";
  if (with_index) index_nm = df.columns()[0]; 

  dftable ret;
  std::string mean_str = "mean", sum_str = "sum", sub_str = "sub";
  auto ncol = cols.size();
  if (axis == 0) {
    std::vector<double> mad_res(ncol);
    if (skip_na) {
      for (size_t i = 0; i < ncol; ++i) mad_res[i] = df.mad(cols[i]);
    } else {
      auto tmax = std::numeric_limits<double>::max();
      for (size_t i = 0; i < ncol; ++i) {
        auto cname = cols[i];
        use_dfcolumn use(df.raw_column(cname));
        if (df.column(cname)->if_contain_nulls()) mad_res[i] = tmax;
        else mad_res[i] = df.mad(cname);
      }
    }
    ret.append_column("index", make_dvector_scatter(cols)); 
    ret.append_column("mad", 
                      make_dvector_scatter(mad_res), true); // may contain null 
  }
  else if (axis == 1) {
    if (ncol == 0 ) append_null<double>(ret, "mad", df.num_row());
    else {
      double fillv = 0;
      auto tmp = frov_df_mean_impl(df_proxy, cols, axis, skip_na, with_index);
      auto sum_col = tmp.as_dvector<double>(mean_str);
      sum_col.mapv(+[](double& x){ //TODO: sum col zeroos
                   x = 0;});
      tmp.append_column(sum_str, sum_col); // may contain null 
      for (size_t i = 0; i < ncol; ++i) {
        use_dfcolumn use_col(df.raw_column(cols[i]));
        auto func = frovedis::sub_col(mean_str, cols[i]);
        use_dfcolumn use_func_sub(func->columns_to_use(tmp, df)); 
        auto rescol = func->execute(tmp, df);
        // replaces nulls with fillv (if any) and append in 'tmp'
        if (skip_na) fillna_helper(tmp, sub_str, rescol, fillv);
        else tmp.append_column(sub_str, rescol); //tmp:mean,sum,sub
        use_dfcolumn use_sub(tmp.raw_column(sub_str));
        func = frovedis::abs_col(sub_str);
        use_dfcolumn use_func_abs(func->columns_to_use(tmp)); 
        rescol = func->execute(tmp);
        tmp.drop(sub_str);
        tmp.append_column(sub_str, rescol);
        func = frovedis::add_col(sum_str, sub_str);
        use_dfcolumn use_func_add(func->columns_to_use(tmp)); 
        rescol = func->execute(tmp);
        tmp.drop(sum_str);//tmp:mean,sub
        tmp.drop(sub_str);//tmp:mean
        tmp.append_column(sum_str, rescol);//tmp:mean,sum
      }
      tmp.drop(mean_str);//tmp:sum
      if (skip_na) {
        // --- calculate row-wise non-na values ---
        auto countna_df = calc_non_na_vals(df, cols, axis, with_index, 
                                           ncol, "count_non_na");
        // variance = sum / count_non_na
        auto var_fn = frovedis::fdiv_col(sum_str, "count_non_na");
        use_dfcolumn use_var(var_fn->columns_to_use(tmp, countna_df));
        ret.append_column("mad", var_fn->execute(tmp, countna_df));
      } else {
        // --- calculate mad --- 
        auto var_fn = frovedis::fdiv_im(sum_str, ncol); // variance = sum / ncol
        use_dfcolumn use_var(var_fn->columns_to_use(tmp));
        ret.append_column("mad", var_fn->execute(tmp));
      }
    }
    // use index as it is in input dataframe, if any. otherwise add index.
    if (with_index) {
      use_dfcolumn use(df.raw_column(index_nm));
      ret.append_column(index_nm, 
                        df.column(index_nm)).change_col_position(index_nm, 0);
    } else {
      ret.prepend_rowid<long>(index_nm);
    }
  }
  else REPORT_ERROR(USER_ERROR, "mad: supported axis: 0 and 1 only!\n");
  return ret;
}

dummy_dftable frov_df_mad(exrpc_ptr_t& df_proxy, 
                          std::vector<std::string>& cols,
                          int& axis, bool& skip_na,
                          bool& with_index) {
  auto ret = frov_df_mad_impl(df_proxy, cols, axis, skip_na, with_index);
  auto retp = new dftable(std::move(ret));
  return to_dummy_dftable(retp);
}

dummy_dftable frov_df_cov(exrpc_ptr_t& df_proxy, 
                          std::vector<std::string>& cols,
                          int& min_periods, double& ddof,
                          bool& low_memory, bool& with_index) {
  std::string index_nm = "index";
  auto& df = *get_dftable_pointer(df_proxy);
  auto df1 = df.select(cols);
  auto ret = df1.covariance(min_periods, ddof, low_memory)
                .append_column(index_nm, make_dvector_scatter(cols))
                .change_col_position(index_nm, 0);
  auto retp = new dftable(std::move(ret));
  return to_dummy_dftable(retp);
}

// TODO: remove (if unused)
double frov_col_cov(exrpc_ptr_t& df_proxy, 
                    std::string& col1,
                    int& min_periods, double& ddof,
                     bool& with_index) {
  auto& df = *get_dftable_pointer(df_proxy);
  return df.covariance(col1, min_periods, ddof);
}

// TODO: remove (if unused)
double frov_col2_cov(exrpc_ptr_t& df_proxy, 
                     std::string& col1, std::string& col2,
                     int& min_periods, double& ddof,
                     bool& with_index) {
  auto& df = *get_dftable_pointer(df_proxy);
  return df.covariance(col1, col2, min_periods, ddof);
}

dummy_dftable
frov_df_mode_cols(exrpc_ptr_t& df_proxy, 
                  std::vector<std::string>& col_names,
                  bool& dropna) {
  dftable* dftblp = get_dftable_pointer(df_proxy);
  auto ncols = col_names.size();

  if (ncols == 0)
    REPORT_ERROR(USER_ERROR, "mode: no column names provided!\n");

  dftable res;

  res = mode_helper(*dftblp, col_names[0], dropna);
  res.append_rowid("ind1");

  for( size_t i=1; i < ncols; i++ ) {
    auto col = col_names[i];
    dftable tmp_df = mode_helper(*dftblp, col, dropna);
    tmp_df.append_rowid("ind2"); 

    if ( res.num_row() >= tmp_df.num_row() ) {
        res = res.outer_bcast_join(tmp_df, eq("ind1", "ind2")).materialize();
        res.drop_cols({"ind2"});
    }
    else {
        res = tmp_df.outer_bcast_join(res, eq("ind2", "ind1")).materialize();
        res.drop_cols({"ind1"});
        res.rename_cols("ind2", "ind1");
    }
  }

  res.drop_cols({"ind1"});
  res.prepend_rowid<long>("index");
  auto retp = new dftable(std::move(res));
  return to_dummy_dftable(retp);
}

dftable mode_rows_numeric(dftable& df, std::vector<std::string>& columns,
                          bool include_index, bool dropna){
  auto ncols = columns.size();
  auto nl_cnts_numeric = make_node_local_allocate<std::vector<std::map<double, size_t> >>();
  auto nl_most_freq_numeric = make_node_local_allocate<std::vector<double>>();
  bool numeric_cnts_initialized = false;

  auto null_val = std::numeric_limits<double>::max();
  dftable res;

  if (include_index) {
    res = df.select({df.columns()[0]});
  }

  for(size_t i = 0; i < ncols; i++) {
    auto curr_col = columns[i];
    auto val = df.column(curr_col)->as_dvector_double().as_node_local();
    if (!numeric_cnts_initialized) {
      nl_cnts_numeric = val.map(intialize_cnts_rows<double>, nl_most_freq_numeric,
                                broadcast(null_val), broadcast(dropna));
      numeric_cnts_initialized = true;   
    }
    else nl_cnts_numeric.mapv(update_counts<double>, val, nl_most_freq_numeric,
                              broadcast(null_val), broadcast(dropna));
  }

  auto cnts_dvec_numeric = nl_most_freq_numeric.as_dvector<double>();    
  if (numeric_cnts_initialized) res.append_column("0", cnts_dvec_numeric, true);
  return res;
}

dftable mode_rows_str(dftable& df, std::vector<std::string>& columns,
                      bool include_index, bool dropna){
  auto ncols = columns.size();
  auto nl_cnts_str = make_node_local_allocate<std::vector<std::map<std::string, size_t> >>();
  auto nl_most_freq_str = make_node_local_allocate<std::vector<std::string>>();
  bool str_cnts_initialized = false;

  std::string null_val = "NULL";
  dftable res;
  
  if (include_index) {
    res = df.select({df.columns()[0]});
  }

  for(size_t i=0; i<ncols; i++) {
      auto curr_col = columns[i];
      auto val = df.column(curr_col)->as_dvector<std::string>().as_node_local();
      if (!str_cnts_initialized) {
        nl_cnts_str = val.map(intialize_cnts_rows<std::string>, nl_most_freq_str,
                              broadcast(null_val), broadcast(dropna) );
        str_cnts_initialized = true;
      }
      else nl_cnts_str.mapv(update_counts<std::string>, val, nl_most_freq_str,
                            broadcast(null_val), broadcast(dropna));
  }

  auto cnts_dvec_str = nl_most_freq_str.as_dvector<std::string>();
  if (str_cnts_initialized) res.append_column("0", cnts_dvec_str, true);
  return res;
}

dummy_dftable
frov_df_mode_rows(exrpc_ptr_t& df_proxy, 
                std::vector<std::string>& col_names,
                bool& is_string,
                bool& dropna) {
  dftable* dftblp = get_dftable_pointer(df_proxy);
  auto ncols = col_names.size();

  if (ncols == 0)
    REPORT_ERROR(USER_ERROR, "mode: no column names provided!\n");

  bool include_index = true;
  dftable res;
  if (is_string) res = mode_rows_str(*dftblp, col_names, include_index, dropna);
  else res = mode_rows_numeric(*dftblp, col_names, include_index, dropna);
  
  auto retp = new dftable(std::move(res));
  return to_dummy_dftable(retp);
}

// --- dffunc for spark wrapper ---
exrpc_ptr_t get_dffunc_id(std::string& cname) {
  auto retptr = new std::shared_ptr<dffunction>(id_col(cname));
  return reinterpret_cast<exrpc_ptr_t> (retptr);
}

exrpc_ptr_t get_dffunc_string_im(std::string& value) {
  std::shared_ptr<dffunction> *retptr = NULL;
  if (value == "NULL") {
    retptr = new std::shared_ptr<dffunction>(null_dic_string_column());
  } else {
    retptr = new std::shared_ptr<dffunction>(dic_string_im(value));
  }
  return reinterpret_cast<exrpc_ptr_t> (retptr);
}

int named_bool_toInt(std::string& arg) {
  std::string str;
  str.resize(arg.length());
  std::transform(arg.cbegin(), arg.cend(), str.begin(), ::tolower);
  return str == "true"; 
}

exrpc_ptr_t get_dffunc_bool_im(std::string& value) {
  std::shared_ptr<dffunction> *retptr = NULL;
  if (value == "NULL") {
    retptr = new std::shared_ptr<dffunction>(null_column<int>());
  } else {
    retptr = new std::shared_ptr<dffunction>(im(named_bool_toInt(value)));
  }
  return reinterpret_cast<exrpc_ptr_t> (retptr);
}

exrpc_ptr_t get_when_dffunc(exrpc_ptr_t& leftp,
                            exrpc_ptr_t& rightp,
                            short& opt_id,
                            std::string& cname) {
  auto& right = *reinterpret_cast<std::shared_ptr<dffunction>*>(rightp);
  std::shared_ptr<dffunction> *opt = NULL;
  switch(opt_id) {
    case IF:
    case ELIF: {
      auto& left = *reinterpret_cast<std::shared_ptr<dfoperator>*>(leftp);
      opt = new std::shared_ptr<dffunction>(when({left >> right})->as(cname));
      // ELIF: must also call append_when_condition() to update if-elif conditions from client side
      break;
    }
    case ELSE: {
      auto& left = *reinterpret_cast<std::shared_ptr<dffunction_when>*>(leftp);
      auto cond = left->cond;
      auto func = left->func;
      func.push_back(right); // added else function
      opt = new std::shared_ptr<dffunction>(when(cond, func)->as(cname)); break;
    }
    default: REPORT_ERROR(USER_ERROR, "when: Unsupported dffunction is requested!\n");
  }
  return reinterpret_cast<exrpc_ptr_t> (opt);
}

exrpc_ptr_t get_dffunc_opt(exrpc_ptr_t& leftp,
                           exrpc_ptr_t& rightp,
                           short& opt_id,
                           std::string& cname) {
  if (opt_id == IF || opt_id == ELIF || opt_id == ELSE)
    return get_when_dffunc(leftp, rightp, opt_id, cname);

  auto& left = *reinterpret_cast<std::shared_ptr<dffunction>*>(leftp);
  auto& right = *reinterpret_cast<std::shared_ptr<dffunction>*>(rightp);
  std::shared_ptr<dffunction> *opt = NULL;
  switch(opt_id) { 
    // --- conditional ---
    case EQ:        opt = new std::shared_ptr<dffunction>(eq(left, right)->as(cname)); break;
    case NE:        opt = new std::shared_ptr<dffunction>(neq(left, right)->as(cname)); break;
    case GT:        opt = new std::shared_ptr<dffunction>(gt(left, right)->as(cname)); break;
    case GE:        opt = new std::shared_ptr<dffunction>(ge(left, right)->as(cname)); break;
    case LT:        opt = new std::shared_ptr<dffunction>(lt(left, right)->as(cname)); break;
    case LE:        opt = new std::shared_ptr<dffunction>(le(left, right)->as(cname)); break;
    // --- special conditional --- (LIKE/NLIKE: immed operation)
    case AND:       opt = new std::shared_ptr<dffunction>(and_op(left,right)->as(cname)); break;
    case OR:        opt = new std::shared_ptr<dffunction>(or_op(left,right)->as(cname)); break;
    case NOT:       opt = new std::shared_ptr<dffunction>(not_op(left)->as(cname)); break;
    case ISNULL:    opt = new std::shared_ptr<dffunction>(is_null(left)->as(cname)); break;
    case ISNOTNULL: opt = new std::shared_ptr<dffunction>(is_not_null(left)->as(cname)); break;
    //case ISNAN:     opt = new std::shared_ptr<dffunction>(is_nan(left)->as(cname)); break;
    case ISNAN:     REPORT_ERROR(USER_ERROR, "is_nan: currently unsupported in frovedis dataframe!\n");
    // --- mathematical ---
    case ADD:       opt = new std::shared_ptr<dffunction>(add_col_as(left, right, cname)); break;
    case SUB:       opt = new std::shared_ptr<dffunction>(sub_col_as(left, right, cname)); break;
    case MUL:       opt = new std::shared_ptr<dffunction>(mul_col_as(left, right, cname)); break;
    case IDIV:      opt = new std::shared_ptr<dffunction>(idiv_col_as(left, right, cname)); break;
    case FDIV:      opt = new std::shared_ptr<dffunction>(fdiv_col_as(left, right, cname)); break;
    case MOD:       opt = new std::shared_ptr<dffunction>(mod_col_as(left, right, cname)); break;
    case POW:       opt = new std::shared_ptr<dffunction>(pow_col_as(left, right, cname)); break;
    // --- string ---
    case UPPER:     opt = new std::shared_ptr<dffunction>(upper_col_as(left, cname)); break;
    case LOWER:     opt = new std::shared_ptr<dffunction>(lower_col_as(left, cname)); break;
    case LEN:       opt = new std::shared_ptr<dffunction>(length_col_as(left, cname)); break;
    case CHARLEN:   opt = new std::shared_ptr<dffunction>(char_length_col_as(left, cname)); break;
    case REV:       opt = new std::shared_ptr<dffunction>(reverse_col_as(left, cname)); break;
    case TRIM:      opt = new std::shared_ptr<dffunction>(trim_im_as(left, cname)); break;
    case LTRIM:     opt = new std::shared_ptr<dffunction>(ltrim_im_as(left, cname)); break;
    case RTRIM:     opt = new std::shared_ptr<dffunction>(rtrim_im_as(left, cname)); break;
    case ASCII:     opt = new std::shared_ptr<dffunction>(ascii_col_as(left, cname)); break;
    case INITCAP:   opt = new std::shared_ptr<dffunction>(initcap_col_as(left, cname)); break;
    case HAMMINGDIST: opt = new std::shared_ptr<dffunction>(hamming_col_as(left, right, cname)); break;
    // -- date --
    case GETYEAR:   opt = new std::shared_ptr<dffunction>(datetime_extract_col_as(left, datetime_type::year, cname)); break;
    case GETMONTH:   opt = new std::shared_ptr<dffunction>(datetime_extract_col_as(left, datetime_type::month, cname)); break;
    case GETDAYOFMONTH:   opt = new std::shared_ptr<dffunction>(datetime_extract_col_as(left, datetime_type::day, cname)); break;
    case GETHOUR:   opt = new std::shared_ptr<dffunction>(datetime_extract_col_as(left, datetime_type::hour, cname)); break;
    case GETMINUTE:   opt = new std::shared_ptr<dffunction>(datetime_extract_col_as(left, datetime_type::minute, cname)); break;
    case GETSECOND:   opt = new std::shared_ptr<dffunction>(datetime_extract_col_as(left, datetime_type::second, cname)); break;
    case GETQUARTER:   opt = new std::shared_ptr<dffunction>(datetime_extract_col_as(left, datetime_type::quarter, cname)); break;
    case GETDAYOFWEEK:   opt = new std::shared_ptr<dffunction>(datetime_extract_col_as(left, datetime_type::dayofweek, cname)); break;
    case GETDAYOFYEAR:   opt = new std::shared_ptr<dffunction>(datetime_extract_col_as(left, datetime_type::dayofyear, cname)); break;
    case GETWEEKOFYEAR:   opt = new std::shared_ptr<dffunction>(datetime_extract_col_as(left, datetime_type::weekofyear, cname)); break;
    case ADDDATE: opt = new std::shared_ptr<dffunction>(datetime_add_col_as(left, right, datetime_type::day, cname)); break;
    case ADDMONTHS: opt = new std::shared_ptr<dffunction>(datetime_add_col_as(left, right, datetime_type::month, cname)); break;
    case SUBDATE: opt = new std::shared_ptr<dffunction>(datetime_sub_col_as(left, right, datetime_type::day, cname)); break;
    case DATEDIFF: opt = new std::shared_ptr<dffunction>(datetime_diff_col_as(left, right, datetime_type::day, cname)); break;
    case MONTHSBETWEEN: opt = new std::shared_ptr<dffunction>(datetime_months_between_col_as(left, right, cname)); break;
    case NEXTDAY: opt = new std::shared_ptr<dffunction>(datetime_next_day_col_as(left, right, cname)); break;
    case TRUNCMONTH: opt = new std::shared_ptr<dffunction>(datetime_truncate_col_as(left, datetime_type::month, cname)); break;
    case TRUNCYEAR: opt = new std::shared_ptr<dffunction>(datetime_truncate_col_as(left, datetime_type::year, cname)); break;
    case TRUNCWEEK: opt = new std::shared_ptr<dffunction>(datetime_truncate_col_as(left, datetime_type::weekofyear, cname)); break;
    case TRUNCQUARTER: opt = new std::shared_ptr<dffunction>(datetime_truncate_col_as(left, datetime_type::quarter, cname)); break;
    case TRUNCHOUR: opt = new std::shared_ptr<dffunction>(datetime_truncate_col_as(left, datetime_type::hour, cname)); break;
    case TRUNCMINUTE: opt = new std::shared_ptr<dffunction>(datetime_truncate_col_as(left, datetime_type::minute, cname)); break;
    case TRUNCSECOND: opt = new std::shared_ptr<dffunction>(datetime_truncate_col_as(left, datetime_type::second, cname)); break;

    default:   REPORT_ERROR(USER_ERROR, "Unsupported dffunction/dfoperator is requested!\n");
  }
  return reinterpret_cast<exrpc_ptr_t> (opt);
}

exrpc_ptr_t get_string_immed_when_dffunc(exrpc_ptr_t& leftp,
                                         std::string& right,
                                         short& opt_id,
                                         std::string& cname) {
  std::shared_ptr<dffunction> *opt = NULL;
  switch(opt_id) {
    case IF:
    case ELIF: {
      auto& left = *reinterpret_cast<std::shared_ptr<dfoperator>*>(leftp);
      opt = new std::shared_ptr<dffunction>(when({left >> dic_string_im(right)})->as(cname));
      // ELIF: must also call append_when_condition() to update if-elif conditions from client side
      break;
    }
    case ELSE: {
      auto& left = *reinterpret_cast<std::shared_ptr<dffunction_when>*>(leftp);
      auto cond = left->cond;
      auto func = left->func;
      func.push_back(dic_string_im(right)); // added else function
      opt = new std::shared_ptr<dffunction>(when(cond, func)->as(cname)); break;
    }
    default: REPORT_ERROR(USER_ERROR, "when: Unsupported dffunction is requested!\n");
  }
  return reinterpret_cast<exrpc_ptr_t> (opt);
}

exrpc_ptr_t get_immed_int_dffunc_opt(exrpc_ptr_t& leftp,
                                     int& right,
                                     short& opt_id,
                                     std::string& cname) {
  auto& left = *reinterpret_cast<std::shared_ptr<dffunction>*>(leftp);
  std::shared_ptr<dffunction> *opt = NULL;
  switch(opt_id) {
    // --- date ---
    case ADDDATE:   opt = new std::shared_ptr<dffunction>(datetime_add_im_as(
                            left, right, datetime_type::day, cname)); break;
    case ADDMONTHS: opt = new std::shared_ptr<dffunction>(datetime_add_im_as(
                            left, right, datetime_type::month, cname)); break;
    case SUBDATE:   opt = new std::shared_ptr<dffunction>(datetime_sub_im_as(
                            left, right, datetime_type::day, cname)); break;
    case NEXTDAY:   opt = new std::shared_ptr<dffunction>(
                            datetime_next_day_im_as(left, right, cname)); break;
    // --- string ---
    case REPEAT:    opt = new std::shared_ptr<dffunction>(
                            repeat_im_as(left, right, cname)); break;
    default:   REPORT_ERROR(USER_ERROR, 
               "Unsupported dffunction with integer as immed value is requested!\n");
  }
  return reinterpret_cast<exrpc_ptr_t> (opt);
}

exrpc_ptr_t get_immed_string_dffunc_opt(exrpc_ptr_t& leftp,
                                        std::string& right,
                                        short& opt_id,
                                        std::string& cname,
                                        bool& is_rev) {
  if (opt_id == IF || opt_id == ELIF || opt_id == ELSE)
    return get_string_immed_when_dffunc(leftp, right, opt_id, cname);

  auto& left = *reinterpret_cast<std::shared_ptr<dffunction>*>(leftp);
  std::shared_ptr<dffunction> *opt = NULL;
  if (!is_rev) {
    switch(opt_id) {
      case EQ:    opt = new std::shared_ptr<dffunction>((left == right)->as(cname)); break;
      case NE:    opt = new std::shared_ptr<dffunction>((left != right)->as(cname)); break;
      case GT:    opt = new std::shared_ptr<dffunction>((left > right)->as(cname)); break;
      case GE:    opt = new std::shared_ptr<dffunction>((left >= right)->as(cname)); break;
      case LT:    opt = new std::shared_ptr<dffunction>((left < right)->as(cname)); break;
      case LE:    opt = new std::shared_ptr<dffunction>((left <= right)->as(cname)); break;
      case LIKE:  opt = new std::shared_ptr<dffunction>(is_like(left, right)->as(cname)); break;
      case NLIKE: opt = new std::shared_ptr<dffunction>(is_not_like(left, right)->as(cname)); break;
      case CAST:  opt = new std::shared_ptr<dffunction>(cast_col_as(left, right, cname)); break;
      case TRIMWS:  opt = new std::shared_ptr<dffunction>(trim_im_as(left, right, cname)); break;
      case LTRIMWS: opt = new std::shared_ptr<dffunction>(ltrim_im_as(left, right, cname)); break;
      case RTRIMWS: opt = new std::shared_ptr<dffunction>(rtrim_im_as(left, right, cname)); break;
      // currently supported operation for datetime/string/raw_string/dic_string column only: 
      // SUB: date_column - immediate_date_as_string => id_col("date") - "2020-02-01"
      case DATEDIFF:
      case SUB:   opt = new std::shared_ptr<dffunction>(sub_im_as<std::string>(
                                            left, right, cname)); break;
      // datetime_months_between_*: doesn't support immediate value
      case MONTHSBETWEEN: opt = new std::shared_ptr<dffunction>(
                                  datetime_months_between_col_as(left, 
                                    cast_col(dic_string_im(right), "datetime"), 
                                    cname)); break;
      case DATEFORMAT:  opt = new std::shared_ptr<dffunction>(
                                  datetime_format_im_as(left, right,
                                                       "dic_string",cname)
                                  ); break;
      default: REPORT_ERROR(USER_ERROR,
               "Unsupported dffunction with string as immed value is requested!\n");
    }
  } else {
    switch(opt_id) {
      case EQ:    opt = new std::shared_ptr<dffunction>((right == left)->as(cname)); break;
      case NE:    opt = new std::shared_ptr<dffunction>((right != left)->as(cname)); break;
      case GT:    opt = new std::shared_ptr<dffunction>((right > left)->as(cname)); break;
      case GE:    opt = new std::shared_ptr<dffunction>((right >= left)->as(cname)); break;
      case LT:    opt = new std::shared_ptr<dffunction>((right < left)->as(cname)); break;
      case LE:    opt = new std::shared_ptr<dffunction>((right <= left)->as(cname)); break;
      case LIKE:  REPORT_ERROR(USER_ERROR, "like: reversed operation is not allowed!\n");
      case NLIKE: REPORT_ERROR(USER_ERROR, "not_like: reversed operation is not allowed!\n");
      case CAST:  REPORT_ERROR(USER_ERROR, "cast: reversed operation is not allowed!\n");
      case TRIMWS:  REPORT_ERROR(USER_ERROR, "trim: reversed operation is not allowed!\n");
      case LTRIMWS: REPORT_ERROR(USER_ERROR, "ltrim: reversed operation is not allowed!\n");
      case RTRIMWS: REPORT_ERROR(USER_ERROR, "rtrim: reversed operation is not allowed!\n");
      // currently supported operation for datetime/string/raw_string/dic_string column only: 
      // immediate_date_as_string - date_column => "2020-02-01" - id_col("date")
      case DATEDIFF:
      case SUB:   opt = new std::shared_ptr<dffunction>(sub_im_as<std::string>(
                                            right, left, cname)); break;
      // datetime_months_between_*: doesn't support immediate value
      case MONTHSBETWEEN: opt = new std::shared_ptr<dffunction>(
                                  datetime_months_between_col_as(
                                    cast_col(dic_string_im(right), "datetime"), 
                                    left, cname)); break;
      default: REPORT_ERROR(USER_ERROR,
               "Unsupported dffunction with string as immed value is requested!\n");
    }
  }
  return reinterpret_cast<exrpc_ptr_t> (opt);
}

exrpc_ptr_t 
append_when_condition(exrpc_ptr_t& leftp, exrpc_ptr_t& rightp,
                      std::string& cname) {
  auto& left  = *reinterpret_cast<std::shared_ptr<dffunction_when>*>(leftp);
  auto& right = *reinterpret_cast<std::shared_ptr<dffunction_when>*>(rightp);
  auto cond = vector_concat(left->cond, right->cond);
  auto func = vector_concat(left->func, right->func);
  auto opt = new std::shared_ptr<dffunction>(when(cond, func)->as(cname));
  return reinterpret_cast<exrpc_ptr_t> (opt);
}

exrpc_ptr_t 
get_immed_substr(exrpc_ptr_t& colp, int& pos, int& num,
                 std::string& cname) {
  auto& col = *reinterpret_cast<std::shared_ptr<dffunction>*>(colp);
  auto retp = new std::shared_ptr<dffunction>(
                substr_posim_numim_as(col, pos, num, cname));
  return reinterpret_cast<exrpc_ptr_t> (retp);
}

exrpc_ptr_t 
get_immed_substr_index(exrpc_ptr_t& colp, 
                       std::string& delim, int& num,
                       std::string& cname) {
  auto& col = *reinterpret_cast<std::shared_ptr<dffunction>*>(colp);
  auto retp = new std::shared_ptr<dffunction>(
                substring_index_im_as(col, delim, num, cname));
  return reinterpret_cast<exrpc_ptr_t> (retp);
}

exrpc_ptr_t 
get_col_substr(exrpc_ptr_t& colp, 
               exrpc_ptr_t& posp, exrpc_ptr_t& nump,
               std::string& cname) {
  auto& col = *reinterpret_cast<std::shared_ptr<dffunction>*>(colp);
  auto& pos = *reinterpret_cast<std::shared_ptr<dffunction>*>(posp);
  auto& num = *reinterpret_cast<std::shared_ptr<dffunction>*>(nump);
  auto retp = new std::shared_ptr<dffunction>(
                substr_poscol_numcol_as(col, pos, num, cname));
  return reinterpret_cast<exrpc_ptr_t> (retp);
}

exrpc_ptr_t get_col_concat_multi(std::vector<exrpc_ptr_t>& cols,
                                 std::string& as,
                                 std::string& sep, bool& with_sep) {
  auto funcs = to_dffunction(cols);
  auto retp = new std::shared_ptr<dffunction>(
                with_sep ? concat_multi_ws_as(sep, funcs, as)
                         : concat_multi_col_as(funcs, as)
              );
  return reinterpret_cast<exrpc_ptr_t> (retp);
}

exrpc_ptr_t get_immed_locate(exrpc_ptr_t& colp, 
                             std::string& substr, int& pos, 
                             std::string& cname) {
  auto& col = *reinterpret_cast<std::shared_ptr<dffunction>*>(colp);
  auto retp = new std::shared_ptr<dffunction>(
                locate_im_as(col, substr, pos, cname));
  return reinterpret_cast<exrpc_ptr_t> (retp);
}

exrpc_ptr_t get_immed_pad(exrpc_ptr_t& colp,
                          int& len, std::string& value,
                          std::string& cname, bool& is_left) {
  auto& col = *reinterpret_cast<std::shared_ptr<dffunction>*>(colp);
  auto retp = new std::shared_ptr<dffunction>(
                is_left ? lpad_im_as(col, len, value, cname)
                        : rpad_im_as(col, len, value, cname)
              );
  return reinterpret_cast<exrpc_ptr_t> (retp);
}

exrpc_ptr_t get_immed_from_to_opt(exrpc_ptr_t& colp,
                                  std::string& from,
                                  std::string& to,
                                  short& opt_id,
                                  std::string& cname) {
  auto& col = *reinterpret_cast<std::shared_ptr<dffunction>*>(colp);
  std::shared_ptr<dffunction> *opt = NULL;
  switch(opt_id) {
    case REPLACE:   opt = new std::shared_ptr<dffunction>(
                            replace_im_as(col, from, to, cname)); break;
    case TRANSLATE: opt = new std::shared_ptr<dffunction>(
                            translate_im_as(col, from, to, cname)); break;
    default: REPORT_ERROR(USER_ERROR,
             "Unsupported dffunction with from-and-to is requested!\n");
  }
  return reinterpret_cast<exrpc_ptr_t> (opt);
}

exrpc_ptr_t get_dffunc_agg(exrpc_ptr_t& leftp,
                           short& opt_id,
                           std::string& cname,
                           bool& ignore_nulls) { // applicable for first/last
  auto& left = *reinterpret_cast<std::shared_ptr<dffunction>*>(leftp);
  std::shared_ptr<dfaggregator> *opt = NULL;
  switch(opt_id) {
    // --- aggregator --- 
    case aMAX:  opt = new std::shared_ptr<dfaggregator>(max_as(left, cname)); break;
    case aMIN:  opt = new std::shared_ptr<dfaggregator>(min_as(left, cname)); break;
    case aSUM:  opt = new std::shared_ptr<dfaggregator>(sum_as(left, cname)); break;
    case aAVG:  opt = new std::shared_ptr<dfaggregator>(avg_as(left, cname)); break;
    case aVAR:  opt = new std::shared_ptr<dfaggregator>(var_as(left, cname)); break;
    case aSEM:  opt = new std::shared_ptr<dfaggregator>(sem_as(left, cname)); break;
    case aSTD:  opt = new std::shared_ptr<dfaggregator>(std_as(left, cname)); break;
    case aMAD:  opt = new std::shared_ptr<dfaggregator>(mad_as(left, cname)); break; // TODO: implement in library for grouped_table
    case aCNT:  opt = new std::shared_ptr<dfaggregator>(count_as(left, cname)); break;
    case aSIZE: opt = new std::shared_ptr<dfaggregator>(size_as(left, cname)); break;
    case aDSUM: opt = new std::shared_ptr<dfaggregator>(sum_distinct_as(left, cname)); break;
    case aDCNT: opt = new std::shared_ptr<dfaggregator>(count_distinct_as(left, cname)); break;
    case aFST:  opt = new std::shared_ptr<dfaggregator>(first_as(left, cname, ignore_nulls)); break;
    case aLST:  opt = new std::shared_ptr<dfaggregator>(last_as(left, cname, ignore_nulls)); break;
    default:    REPORT_ERROR(USER_ERROR, "Unsupported dfaggregator is requested!\n");
  }
  return reinterpret_cast<exrpc_ptr_t> (opt);
}

dummy_dftable append_scalar(exrpc_ptr_t& dfproxy, std::string& cname, 
                            std::string& value, short& dtype) {
  auto& df = *reinterpret_cast<dftable_base*>(dfproxy);
  auto ret = df.materialize(); // new dftable always
  auto size = ret.num_row();
  require(size > 0, "append_scalar: is not allowed for empty dataframe!\n");
  if (value == "NULL") {
    switch (dtype) {
      case BOOL:
      case INT:    append_null<int>(ret, cname, size); break;
      case LONG:   append_null<long>(ret, cname, size); break;
      case ULONG:  append_null<unsigned long>(ret, cname, size); break;
      case FLOAT:  append_null<float>(ret, cname, size); break;
      case DOUBLE: append_null<double>(ret, cname, size); break;
      case STRING: append_null<std::string>(ret, cname, size); break;
      default:     REPORT_ERROR(USER_ERROR, 
                   "append_scalar: unsupported type detected for appending nulls!\n");
    }
  } else {
    switch (dtype) {
      case INT:    append_value<int>(ret, cname, size, value); break;
      case BOOL:   append_value<int>(ret, cname, size, std::to_string(named_bool_toInt(value))); break;
      case LONG:   append_value<long>(ret, cname, size, value); break;
      case ULONG:  append_value<unsigned long>(ret, cname, size, value); break;
      case FLOAT:  append_value<float>(ret, cname, size, value); break;
      case DOUBLE: append_value<double>(ret, cname, size, value); break;
      case STRING: append_value<std::string>(ret, cname, size, value); break;
      default:     REPORT_ERROR(USER_ERROR,
                   "append_scalar: unsupported type detected for appending values!\n");
    }
  }
  auto retp = new dftable(std::move(ret));
  if (!retp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  return to_dummy_dftable(retp);
}

dummy_dftable execute_dffunc(exrpc_ptr_t& dfproxy, std::string& cname, 
                             exrpc_ptr_t& dffunc) {
  auto& df = *reinterpret_cast<dftable_base*>(dfproxy);
  auto func = *reinterpret_cast<std::shared_ptr<dffunction>*>(dffunc);
  func->as(cname);
  auto ret = df.materialize().call_function(func); // new dftable always
  auto retp = new dftable(std::move(ret));
  if (!retp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  return to_dummy_dftable(retp);
}


dummy_dftable execute_dfagg(exrpc_ptr_t& dfproxy, 
                            std::vector<exrpc_ptr_t>& aggp) {
  auto& df = *reinterpret_cast<dftable_base*>(dfproxy);
  auto agg = to_dfaggregator(aggp);
  std::vector<std::shared_ptr<dffunction>> aggfun(agg.size());
  for(size_t i = 0; i < agg.size(); i++) {aggfun[i] = agg[i];}
  auto retp = new dftable(df.aggregate(aggfun));
  if (!retp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  return to_dummy_dftable(retp);
}

void set_dffunc_asCol_name(exrpc_ptr_t& fn, std::string& cname) {
  auto& func = *reinterpret_cast<std::shared_ptr<dffunction>*>(fn);
  func->as(cname);
}

void set_dfagg_asCol_name(exrpc_ptr_t& fn, std::string& cname) {
  auto& agg = *reinterpret_cast<std::shared_ptr<dfaggregator>*>(fn);
  agg->as(cname);
}

dummy_vector
frov_get_bool_mask(exrpc_ptr_t& df_opt_proxy, 
                   exrpc_ptr_t& df_proxy,
                   bool& ignore_nulls) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  auto dfopt = *reinterpret_cast<std::shared_ptr<dfoperator>*>(df_opt_proxy);
  dummy_vector ret;
  if (ignore_nulls) { // pandas default behavior
    auto filter_idx = dfopt->filter(df);
    auto bool_mask = filter_idx.map(get_bool_mask_helper, 
                     make_node_local_scatter(df.num_rows())); // treats null as false, like pandas
    auto retp = new dvector<int>(bool_mask.moveto_dvector<int>());
    auto retp_ = reinterpret_cast<exrpc_ptr_t>(retp);
    ret = dummy_vector(retp_, retp->size(), INT);
  } else {
    auto retp = new dvector<int>(dfopt->execute(df)->as_dvector<int>()); // keeps nulls, like sql/spark
    auto retp_ = reinterpret_cast<exrpc_ptr_t>(retp);
    ret = dummy_vector(retp_, retp->size(), INT);
  }
  return ret;
}

exrpc_ptr_t 
frov_df_filter_using_mask(exrpc_ptr_t& df_proxy,
                          exrpc_ptr_t& mask_dvec_proxy) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  auto& dvec = *reinterpret_cast<dvector<int>*>(mask_dvec_proxy);
  if (df.num_row() != dvec.size()) {
    auto msg = "Item wrong length " + std::to_string(dvec.size()) +
               " instead of " + std::to_string(df.num_row()) + "\n";
    REPORT_ERROR(USER_ERROR, msg);
  } 
  dvec.align_as(df.num_rows()); // realign mask dvector as per dataframe column alignments
  auto filter_idx = dvec.viewas_node_local()
                        .map(+[] (const std::vector<int>& vec) 
                                 { return vector_find_one(vec); });
  auto retp = new filtered_dftable(filtered_dftable(df, filter_idx));
  return reinterpret_cast<exrpc_ptr_t>(retp);
}

exrpc_ptr_t frov_df_distinct(exrpc_ptr_t& df_proxy) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  auto res = new dftable(df.distinct());
  if (!res) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  return reinterpret_cast<exrpc_ptr_t> (res);
}

double frov_series_cov(exrpc_ptr_t& self_proxy, std::string& col1,
                       exrpc_ptr_t& other_proxy, std::string& col2,
                       int& min_periods, double& ddof) {
  auto& self  = *reinterpret_cast<dftable_base*>(self_proxy);
  auto& other = *reinterpret_cast<dftable_base*>(other_proxy);
  use_dfcolumn use({self.raw_column(col1), other.raw_column(col2)});
  auto c1 = self.column(col1);
  auto c2 = other.column(col2);
  double ret = 0;
  auto sizes = self.num_rows();
  auto lv1 = c1->as_dvector_double().moveto_node_local();
  auto lv2 = c2->as_dvector_double().align_as(sizes) // realign if not same
                                    .moveto_node_local(); 
  if (c1->if_contain_nulls() || c2->if_contain_nulls()) {
    auto lnv1 = c1->get_nulls().map(get_bool_mask_helper,
                                    make_node_local_scatter(sizes));
    auto lnv2 = c2->get_nulls().map(get_bool_mask_helper,
                                    make_node_local_scatter(sizes))
                               .moveto_dvector<int>()
                               .align_as(sizes) // realign if not same
                               .moveto_node_local();
    bool ignore_ddof = false; // since series case
    ret = cov_impl(lv1, lv2, lnv1, lnv2, min_periods, ddof, ignore_ddof);
  } else {
    ret = cov_impl(lv1, lv2, min_periods, ddof);
  }
  return ret;
}

dummy_dftable 
copy_spark_column(exrpc_ptr_t& self_proxy,
                  std::vector<exrpc_ptr_t>& proxies,
                  std::vector<std::string>& cols) {
  dftable* selfp;
  if (self_proxy == -1) selfp = new dftable(); // empty case
  else                  selfp = reinterpret_cast<dftable*>(self_proxy);
  for(size_t i = 0; i < proxies.size(); ++i) {
    auto& other  = *reinterpret_cast<dftable*>(proxies[i]);
    selfp->append_column(cols[i], other.column(cols[i])); // distribution should be same
  }
  return to_dummy_dftable(selfp);
}

long calc_memory_size(exrpc_ptr_t& df_proxy) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  auto cols = df.columns();
  long size = 0;
  long denom = 1024;
  for (size_t i = 0; i < cols.size(); ++i) size += df.column(cols[i])->calc_spill_size();
  size = ceil_div(ceil_div(size, denom), denom); // size in MB
  return size;
}

dummy_dftable
frov_df_first_element(exrpc_ptr_t& df_proxy, std::string& col,
                      bool& skipna) {
  auto dftblp = reinterpret_cast<dftable_base*>(df_proxy);
  auto dfp = new dftable(dftblp->aggregate({first(col, skipna)}));
  return to_dummy_dftable(dfp);
}

dummy_dftable
frov_df_last_element(exrpc_ptr_t& df_proxy, std::string& col,
                     bool& skipna) {
  auto dftblp = reinterpret_cast<dftable_base*>(df_proxy);
  auto dfp = new dftable(dftblp->aggregate({last(col, skipna)}));
  return to_dummy_dftable(dfp);
}

dummy_dftable 
frov_df_clip(exrpc_ptr_t& df_proxy,
            std::string& lower_limit_col,
            std::string& upper_limit_col,
            bool& with_index) {
  auto dftblp = reinterpret_cast<dftable_base*>(df_proxy);
  auto lower_limit = ~lower_limit_col;
  auto upper_limit = ~upper_limit_col;

  auto columns = dftblp->columns();
  auto sz = columns.size();
  std::vector<std::shared_ptr<frovedis::dffunction>> res_cols(sz);

  size_t start = 0;
  if (with_index) {
    res_cols[0] = ~columns[0];
    start = 1;
  }
 
  for(size_t i = start; i < sz; i++){
    auto curr_col = ~columns[i];
    auto res_col = when({curr_col < lower_limit, curr_col > upper_limit},
                        {lower_limit, upper_limit, curr_col} );
    res_cols[i] = res_col->as(columns[i]);
  }

  auto retp = new dftable(dftblp->fselect(res_cols));
  return to_dummy_dftable(retp);
}

std::shared_ptr<dffunction>
get_clipped_column(std::shared_ptr<dffunction>& curr_col, 
                  std::string& lower_limit, short lower_dtype,
                  std::string& upper_limit, short upper_dtype) {
  std::shared_ptr<dffunction> res_col;
  std::string msg = "get_clipped_column: Unsupported datatype for ";
  auto u_msg = msg + "upper: " + std::to_string(upper_dtype) + "!\n";
  auto l_msg = msg + "lower: " + std::to_string(lower_dtype) + "!\n";

  switch (lower_dtype) {
    case STRING: 
      if (upper_dtype == STRING) {
        res_col = get_clipped_column_helper<std::string, std::string>(curr_col, 
                                                                  lower_limit, 
                                                                  upper_limit);
      } else REPORT_ERROR(USER_ERROR, u_msg);
      break;
    case DATETIME: 
      if (upper_dtype == DATETIME) {
        res_col = get_clipped_column_helper<datetime_t, datetime_t>(curr_col, 
                                                      lower_limit, 
                                                      upper_limit);
      } else REPORT_ERROR(USER_ERROR, u_msg);
      break;
    case INT: 
      if (upper_dtype == INT) {
        res_col = get_clipped_column_helper<int, int>(curr_col, 
                                                      lower_limit, 
                                                      upper_limit);
      } else if (upper_dtype == LONG) {
        res_col = get_clipped_column_helper<int, long>(curr_col, 
                                                       lower_limit, 
                                                       upper_limit);
      } else if (upper_dtype == FLOAT) {
        res_col = get_clipped_column_helper<int, float>(curr_col, 
                                                       lower_limit, 
                                                       upper_limit);
      } else if (upper_dtype == DOUBLE) {
        res_col = get_clipped_column_helper<int, double>(curr_col, 
                                                       lower_limit, 
                                                       upper_limit);
      } else REPORT_ERROR(USER_ERROR, u_msg);
      break;
    case LONG: 
      if (upper_dtype == INT) {
        res_col = get_clipped_column_helper<long, int>(curr_col, 
                                                      lower_limit, 
                                                      upper_limit);
      } else if (upper_dtype == LONG) {
        res_col = get_clipped_column_helper<long, long>(curr_col, 
                                                       lower_limit, 
                                                       upper_limit);
      } else if (upper_dtype == FLOAT) {
        res_col = get_clipped_column_helper<long, float>(curr_col, 
                                                       lower_limit, 
                                                       upper_limit);
      } else if (upper_dtype == DOUBLE) {
        res_col = get_clipped_column_helper<long, double>(curr_col, 
                                                       lower_limit, 
                                                       upper_limit);
      } else REPORT_ERROR(USER_ERROR, u_msg);
      break;
    case FLOAT: 
      if (upper_dtype == INT) {
        res_col = get_clipped_column_helper<float, int>(curr_col, 
                                                      lower_limit, 
                                                      upper_limit);
      } else if (upper_dtype == LONG) {
        res_col = get_clipped_column_helper<float, long>(curr_col, 
                                                       lower_limit, 
                                                       upper_limit);
      } else if (upper_dtype == FLOAT) {
        res_col = get_clipped_column_helper<float, float>(curr_col, 
                                                       lower_limit, 
                                                       upper_limit);
      } else if (upper_dtype == DOUBLE) {
        res_col = get_clipped_column_helper<float, double>(curr_col, 
                                                       lower_limit, 
                                                       upper_limit);
      } else REPORT_ERROR(USER_ERROR, u_msg);
      break;
    case DOUBLE: 
      if (upper_dtype == INT) {
        res_col = get_clipped_column_helper<double, int>(curr_col, 
                                                      lower_limit, 
                                                      upper_limit);
      } else if (upper_dtype == LONG) {
        res_col = get_clipped_column_helper<double, long>(curr_col, 
                                                       lower_limit, 
                                                       upper_limit);
      } else if (upper_dtype == FLOAT) {
        res_col = get_clipped_column_helper<double, float>(curr_col, 
                                                       lower_limit, 
                                                       upper_limit);
      } else if (upper_dtype == DOUBLE) {
        res_col = get_clipped_column_helper<double, double>(curr_col, 
                                                       lower_limit, 
                                                       upper_limit);
      } else REPORT_ERROR(USER_ERROR, u_msg);
      break;
    default:  REPORT_ERROR(USER_ERROR, l_msg);
  }
  return res_col;
}

dummy_dftable 
frov_df_clip_axis1(exrpc_ptr_t& df_proxy,
                  std::vector<std::string>& lower_limit,
                  std::vector<short>& lower_dtypes,
                  std::vector<std::string>& upper_limit,
                  std::vector<short>& upper_dtypes,
                  bool& with_index) {
  auto dftblp = reinterpret_cast<dftable_base*>(df_proxy);
  auto columns = dftblp->columns();
  auto sz = columns.size();
  std::vector<std::shared_ptr<frovedis::dffunction>> res_cols(sz);
  size_t start = 0;
  if (with_index) {
    res_cols[0] = ~columns[0];
    start = 1;
  }
  for(size_t i = start; i < sz; i++){
    auto curr_col = ~columns[i];
    auto res_col = get_clipped_column(curr_col,
                                      lower_limit[i-start], 
                                      lower_dtypes[i-start],  
                                      upper_limit[i-start], 
                                      upper_dtypes[i-start]);
    res_cols[i] = res_col->as(columns[i]);
  }
  auto retp = new dftable(dftblp->fselect(res_cols));
  return to_dummy_dftable(retp);
}

dummy_dftable
frov_df_sel_rows_by_indices(exrpc_ptr_t& df_proxy,
                            std::vector<int>& indices) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  auto ret = df.select_rows_by_indices(indices);
  auto retp = new dftable(std::move(ret));
  return to_dummy_dftable(retp);
}

dummy_dftable frov_df_datetime_operation(exrpc_ptr_t& df_proxy,
                                      std::string& left_col,
                                      std::string& right_col,
                                      std::string& as_name,
                                      short& op_id,
                                      bool& with_index) {
  auto dftblp = reinterpret_cast<dftable_base*>(df_proxy);
  std::shared_ptr<dffunction> dffunc;
  switch(op_id) {
      case GETDAYOFMONTH:   dffunc = datetime_extract_col_as(left_col, datetime_type::day, as_name); break;
      case GETDAYOFWEEK:    dffunc = datetime_extract_col_as(left_col, datetime_type::dayofweek, as_name); break;
      case GETHOUR:      dffunc = datetime_extract_col_as(left_col, datetime_type::hour, as_name); break;
      case GETMINUTE:    dffunc = datetime_extract_col_as(left_col, datetime_type::minute, as_name); break;
      case GETSECOND:    dffunc = datetime_extract_col_as(left_col, datetime_type::second, as_name); break;
      case GETNANOSECOND:    dffunc = datetime_extract_col_as(left_col, datetime_type::nanosecond, as_name); break;
      case GETYEAR:   dffunc = datetime_extract_col_as(left_col, datetime_type::year, as_name); break;
      case GETMONTH:   dffunc = datetime_extract_col_as(left_col, datetime_type::month, as_name); break;
      case GETQUARTER:   dffunc = datetime_extract_col_as(left_col, datetime_type::quarter, as_name); break;
      case GETDAYOFYEAR:   dffunc = datetime_extract_col_as(left_col, datetime_type::dayofyear, as_name); break;
      case GETWEEKOFYEAR:   dffunc = datetime_extract_col_as(left_col, datetime_type::weekofyear, as_name); break;
      default: REPORT_ERROR(USER_ERROR,
               "Unsupported conditional operation is encountered!\n");
  }
  
  std::vector<std::shared_ptr<frovedis::dffunction>> res_cols;

  if (with_index){
    res_cols = { ~dftblp->columns()[0], dffunc };
  }
  else res_cols = {dffunc};

  auto retp = new dftable(dftblp->fselect(res_cols));
  return to_dummy_dftable(retp);
}

dummy_dftable frov_df_concat_columns(exrpc_ptr_t& df_proxy,
                                    std::string& sep,
                                    std::vector<std::string>& cols,
                                    std::string& as_name,
                                    bool& cast_as_datetime,
                                    std::string& fmt,
                                    bool& with_index) {
  auto dftblp = reinterpret_cast<dftable_base*>(df_proxy);
  auto sz = cols.size();
  std::vector<std::shared_ptr<dffunction>> cols_dffuncs(sz);
  std::vector<std::shared_ptr<frovedis::dffunction>> res_cols;

  for(size_t i=0; i<sz; i++) cols_dffuncs[i] = ~cols[i];

  if (cast_as_datetime) {
    size_t pad_length = 2;
    for(size_t i=0; i<sz; i++)
      cols_dffuncs[i] = lpad_im_as(cols_dffuncs[i], pad_length, "0", cols[i]);
  }  
  
  auto dffunc = concat_multi_ws_as(sep, cols_dffuncs, as_name);
  if (cast_as_datetime) dffunc = cast_col_as(dffunc, "datetime:"+fmt, as_name);

  if (with_index){
    res_cols = { ~dftblp->columns()[0], dffunc };
  }
  else res_cols = {dffunc};

  auto retp = new dftable(dftblp->fselect(res_cols));
  return to_dummy_dftable(retp);
}

dummy_dftable
frovedis_series_string_methods(exrpc_ptr_t& df_proxy,
                               std::string& cname,
                               std::string& param,
                               short& op_id,
                               bool& with_index) {
  auto& dftbl = *reinterpret_cast<dftable_base*>(df_proxy);
  auto cols = dftbl.columns();
  std::shared_ptr<dffunction> opt = NULL;
  switch(op_id) { 
    case UPPER:     opt = upper_col(cname); break;
    case LOWER:     opt = lower_col(cname); break;
    case LEN:       opt = length_col(cname); break;
    case REV:       opt = reverse_col(cname); break;
    case TRIM:      opt = trim_im(cname); break;
    case LTRIM:     opt = ltrim_im(cname); break;
    case RTRIM:     opt = rtrim_im(cname); break;
    case TRIMWS:    opt = trim_im(cname, param); break;
    case LTRIMWS:   opt = ltrim_im(cname, param); break;
    case RTRIMWS:   opt = rtrim_im(cname, param); break;
    case ASCII:     opt = ascii_col(cname); break;
    case INITCAP:   opt = initcap_col(cname); break;
    case HAMMINGDIST: opt = hamming_col(cname, param); break;
    default:   REPORT_ERROR(USER_ERROR, "Unsupported string method is encountered!\n");
  }
  
  auto retp = new dftable();
  if (with_index) {
    require(cols.size() >= 2, "string_methods: expected at least two columns!");
    auto index_name = cols[0];
    use_dfcolumn use(dftbl.raw_column(index_name));
    retp->append_column(index_name, dftbl.column(index_name));
  }
  use_dfcolumn use(opt->columns_to_use(dftbl));
  retp->append_column(cname, opt->execute(dftbl));
  return to_dummy_dftable(retp);
}

dummy_dftable
frovedis_series_slice(exrpc_ptr_t& df_proxy,
                      std::string& cname,
                      int& start, int& stop, int& step,
                      bool& with_index) {
  auto& dftbl = *reinterpret_cast<dftable_base*>(df_proxy);
  auto cols = dftbl.columns();
  require(step == 1, 
  "Currently frovedis substr() supports only 1 as for step value!");
  auto opt = substr_posim_numim(cname, start, stop - start);

  auto retp = new dftable();
  if (with_index) {
    require(cols.size() >= 2, "slice: expected at least two columns!");
    auto index_name = cols[0];
    use_dfcolumn use(dftbl.raw_column(index_name));
    retp->append_column(index_name, dftbl.column(index_name));
  }
  use_dfcolumn use(opt->columns_to_use(dftbl));
  retp->append_column(cname, opt->execute(dftbl));
  return to_dummy_dftable(retp);
}

dummy_dftable
frovedis_series_pad(exrpc_ptr_t& df_proxy,
                    std::string& cname, 
                    std::string& side, 
                    std::string& value,
                    int& len, bool& with_index) {
  auto& dftbl = *reinterpret_cast<dftable_base*>(df_proxy);
  auto cols = dftbl.columns();
  std::shared_ptr<dffunction> opt = NULL;
  if (side == "left") opt = lpad_im(cname, len, value);
  else if (side == "right") opt = rpad_im(cname, len, value);
  else REPORT_ERROR(USER_ERROR, std::string("pad: Unsupported side '") +
                    side + std::string("' is encountered!\n"));

  auto retp = new dftable();
  if (with_index) {
    require(cols.size() >= 2, "pad: expected at least two columns!");
    auto index_name = cols[0];
    use_dfcolumn use(dftbl.raw_column(index_name));
    retp->append_column(index_name, dftbl.column(index_name));
  }
  use_dfcolumn use(opt->columns_to_use(dftbl));
  retp->append_column(cname, opt->execute(dftbl));
  return to_dummy_dftable(retp);
}

void frovedis_df_to_csv(exrpc_ptr_t& df_proxy,
                        std::string& fname,
                        std::string& mode,
                        std::string& sep,
                        std::string& nullstr,
                        std::string& date_format,
                        size_t& precision) { 
  auto& dftbl = *reinterpret_cast<dftable_base*>(df_proxy);
  auto quote_and_escape = false;
  if (mode == "w") dftbl.savetext(fname, precision, date_format, 
                                  sep, quote_and_escape, nullstr);
  else if (mode == "wb") dftbl.save(fname);
  else REPORT_ERROR(USER_ERROR, "to_csv: supported modes are w and wb only!\n");
}

void frovedis_set_datetime_type_for_add_sub_op(std::string& name) {
  datetime_type type = datetime_type::day;
  if (name == "year") type = datetime_type::year;
  else if (name == "month") type = datetime_type::month;
  else if (name == "day") type = datetime_type::day;
  else if (name == "hour") type = datetime_type::hour;
  else if (name == "minute") type = datetime_type::minute;
  else if (name == "second") type = datetime_type::second;
  else if (name == "nanosecond") type = datetime_type::nanosecond;
  else REPORT_ERROR(USER_ERROR, "unsupported datetime_type for add_sub operation.");
  frovedis::set_datetime_type_for_add_sub_op(type);
}

int is_bool_column(exrpc_ptr_t& df_proxy, std::string& cname) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  use_dfcolumn use_col(df.raw_column(cname));
  auto dfcol = df.column(cname);
  // a BOOL column is implemented as a int32 column having 1s and 0s only.
  int ret = (dfcol->dtype() == "int") ? is_bool_column_helper<int>(dfcol) : 0; 
  return ret;
}

