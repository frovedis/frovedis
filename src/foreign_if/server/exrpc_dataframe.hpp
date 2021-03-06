#ifndef _EXRPC_DF_HPP_
#define _EXRPC_DF_HPP_

#include <frovedis/dataframe.hpp>
#include <frovedis/dataframe/dfcolumn_impl.hpp>
#include <frovedis/dataframe/dfaggregator.hpp>
#include "../exrpc/exrpc_expose.hpp"
#include "model_tracker.hpp" // for OPTYPE, model_table to register sparse_conv_info
#include "frovedis_mem_pair.hpp"
#include "dummy_matrix.hpp"

using namespace frovedis;

bool is_present(const std::vector<std::string>& vec,
                const std::string& val);

dftable* get_dftable_pointer(exrpc_ptr_t& df_proxy);

template <class T>
exrpc_ptr_t get_df_column_pointer(exrpc_ptr_t& df_proxy, 
                                  std::string& cname) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  auto cptr = new dvector<T>(df.as_dvector<T>(cname));
  return reinterpret_cast<exrpc_ptr_t>(cptr);
}

template <class T>
std::vector<T>
fillna_column_helper(std::vector<T>& val,
                     std::vector<size_t>& nulls,
                     T fillv) {
  std::vector<T> ret = val;
  auto rptr = ret.data();
  auto nptr = nulls.data();
  auto nullsz = nulls.size();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < nullsz; ++i) rptr[nptr[i]] = fillv;
  return ret;
}

// TODO: make a function of dfcolumn class
template <class T, class V>
std::shared_ptr<dfcolumn>
fillna_column(std::shared_ptr<dfcolumn>& dfcol, V fill_value) {
  std::shared_ptr<dfcolumn> ret;
  auto c1 = std::dynamic_pointer_cast<typed_dfcolumn<T>>(dfcol);
  if (c1->contain_nulls) {
    auto fillv = static_cast<T>(fill_value);
    auto newval = c1->val.map(fillna_column_helper<T>,
                              c1->nulls, broadcast(fillv));
    auto dvval = newval.template moveto_dvector<T>();
    ret = std::make_shared<typed_dfcolumn<T>>(std::move(dvval));
  }
  else ret = dfcol;
  return ret;
}

exrpc_ptr_t create_dataframe (std::vector<short>& types,
                              std::vector<std::string>& cols,
                              std::vector<exrpc_ptr_t>& dvec_proxies);

void show_dataframe(exrpc_ptr_t& df_proxy); 

template <class T>
exrpc_ptr_t get_dfoperator(std::string& op1, std::string& op2,
                           short& op_id, bool& isImmed) {
  std::shared_ptr<dfoperator> *opt = NULL;
  // op2 would be treated as pattern, instead of immediate value in case of LIKE/NLIKE
  if(op_id == LIKE || op_id == NLIKE) isImmed = false;
  if(isImmed) {
    auto data = do_cast<T>(op2);
    switch(op_id) {
        case EQ: opt = new std::shared_ptr<dfoperator>(eq_im<T>(op1,data)); break;
        case NE: opt = new std::shared_ptr<dfoperator>(neq_im<T>(op1,data)); break;
        case LT: opt = new std::shared_ptr<dfoperator>(lt_im<T>(op1,data)); break;
        case LE: opt = new std::shared_ptr<dfoperator>(le_im<T>(op1,data)); break;
        case GT: opt = new std::shared_ptr<dfoperator>(gt_im<T>(op1,data)); break;
        case GE: opt = new std::shared_ptr<dfoperator>(ge_im<T>(op1,data)); break;
        default: REPORT_ERROR(USER_ERROR,
                 "Unsupported filter operation is encountered!\n");
    }
  }
  else {
    switch(op_id) {
        case EQ: opt = new std::shared_ptr<dfoperator>(eq(op1,op2)); break;
        case NE: opt = new std::shared_ptr<dfoperator>(neq(op1,op2)); break;
        case LT: opt = new std::shared_ptr<dfoperator>(lt(op1,op2)); break;
        case LE: opt = new std::shared_ptr<dfoperator>(le(op1,op2)); break;
        case GT: opt = new std::shared_ptr<dfoperator>(gt(op1,op2)); break;
        case GE: opt = new std::shared_ptr<dfoperator>(ge(op1,op2)); break;
        case LIKE: opt = new std::shared_ptr<dfoperator>(is_like(op1,op2)); break;
        case NLIKE: opt = new std::shared_ptr<dfoperator>(is_not_like(op1,op2)); break;
        default: REPORT_ERROR(USER_ERROR,
                 "Unsupported filter operation is encountered!\n");
    }
  }
  if (!opt) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  return reinterpret_cast<exrpc_ptr_t> (opt);
}

exrpc_ptr_t get_str_dfoperator(std::string& op1, std::string& op2,
                               short& op_id, bool& isImmed); 

exrpc_ptr_t get_dfANDoperator(exrpc_ptr_t& lopt_proxy, exrpc_ptr_t& ropt_proxy);
exrpc_ptr_t get_dfORoperator(exrpc_ptr_t& lopt_proxy, exrpc_ptr_t& ropt_proxy);
exrpc_ptr_t get_dfNOToperator(exrpc_ptr_t& opt_proxy);
exrpc_ptr_t filter_df(exrpc_ptr_t& df_proxy, exrpc_ptr_t& opt_proxy);
exrpc_ptr_t select_df(exrpc_ptr_t& df_proxy, std::vector<std::string>& cols);
exrpc_ptr_t isnull_df(exrpc_ptr_t& df_proxy, std::vector<std::string>& cols);
void drop_df_cols(exrpc_ptr_t& df_proxy, std::vector<std::string>& cols);

template <class T>
dummy_dftable 
drop_df_rows(exrpc_ptr_t& df_proxy,
             std::string& cname,
             std::vector<T>& rowids) {
  auto& dftbl = *reinterpret_cast<dftable_base*>(df_proxy);
  auto retp = new dftable(dftbl.drop_rows(cname, rowids));
  if (!retp) REPORT_ERROR(INTERNAL_ERROR, "memory allocation failed.\n");
  return to_dummy_dftable(retp);
}

exrpc_ptr_t sort_df(exrpc_ptr_t& df_proxy, 
                    std::vector<std::string>& cols, 
                    std::vector<int>& isDesc);

exrpc_ptr_t join_df(exrpc_ptr_t& left_proxy, exrpc_ptr_t& right_proxy,
                    exrpc_ptr_t& opt_proxy, 
                    std::string& how, std::string& join_type);

exrpc_ptr_t group_by_df(exrpc_ptr_t& df_proxy, std::vector<std::string>& cols);

long frovedis_df_size(exrpc_ptr_t& df_proxy);

std::vector<std::string> frovedis_df_sum(exrpc_ptr_t& df_proxy,
                                         std::vector<std::string>& cols,
                                         std::vector<short>& types);

std::vector<std::string> frovedis_df_avg(exrpc_ptr_t& df_proxy,
                                         std::vector<std::string>& cols);

std::vector<std::string> frovedis_df_std(exrpc_ptr_t& df_proxy,
                                         std::vector<std::string>& cols);

std::vector<std::string> frovedis_df_cnt(exrpc_ptr_t& df_proxy,
                                         std::vector<std::string>& cols);

std::vector<std::string> frovedis_df_min(exrpc_ptr_t& df_proxy,
                                         std::vector<std::string>& cols,
                                         std::vector<short>& types);

std::vector<std::string> frovedis_df_max(exrpc_ptr_t& df_proxy,
                                         std::vector<std::string>& cols,
                                         std::vector<short>& types);

exrpc_ptr_t frovedis_df_rename(exrpc_ptr_t& df_proxy,
                               std::vector<std::string>& cols,
                               std::vector<std::string>& new_cols,
                               bool& inplace);

dummy_vector get_df_int_col(exrpc_ptr_t& df_proxy, std::string& cname);
dummy_vector get_df_long_col(exrpc_ptr_t& df_proxy, std::string& cname);
dummy_vector get_df_ulong_col(exrpc_ptr_t& df_proxy, std::string& cname);
dummy_vector get_df_float_col(exrpc_ptr_t& df_proxy, std::string& cname);
dummy_vector get_df_double_col(exrpc_ptr_t& df_proxy, std::string& cname);
dummy_vector get_df_string_col(exrpc_ptr_t& df_proxy, std::string& cname);

exrpc_ptr_t frovedis_gdf_select(exrpc_ptr_t& df_proxy, 
                                std::vector<std::string>& tcols);

exrpc_ptr_t frovedis_gdf_aggr(exrpc_ptr_t& df_proxy, 
                              std::vector<std::string>& groupedCols,
                              std::vector<std::string>& aggFuncs,
                              std::vector<std::string>& aggCols,
                              std::vector<std::string>& aggAsCols);

dummy_matrix df_to_colmajor_float(exrpc_ptr_t& df_proxy,
                                  std::vector<std::string>& t_cols);

dummy_matrix df_to_colmajor_double(exrpc_ptr_t& df_proxy,
                                   std::vector<std::string>& t_cols);

dummy_matrix df_to_rowmajor_float(exrpc_ptr_t& df_proxy,
                                  std::vector<std::string>& t_cols);

dummy_matrix df_to_rowmajor_double(exrpc_ptr_t& df_proxy,
                                   std::vector<std::string>& t_cols);

dummy_matrix df_to_crs_float(exrpc_ptr_t& df_proxy,
                             std::vector<std::string>& t_cols,
                             std::vector<std::string>& cat_cols,
                             long& info_id);

dummy_matrix df_to_crs_double(exrpc_ptr_t& df_proxy,
                              std::vector<std::string>& t_cols,
                              std::vector<std::string>& cat_cols,
                              long& info_id);

dummy_matrix df_to_crs_float_using_info(exrpc_ptr_t& df_proxy,
                                        long& info_id);

dummy_matrix df_to_crs_double_using_info(exrpc_ptr_t& df_proxy,
                                         long& info_id);

void load_sparse_conversion_info(long& info_id, std::string&); 
void save_sparse_conversion_info(long& info_id, std::string&); 
void release_sparse_conversion_info(long& info_id);
exrpc_ptr_t frov_multi_eq_dfopt(std::vector<std::string>& left_cols, 
                                std::vector<std::string>& right_cols);
exrpc_ptr_t frov_cross_join_dfopt();

dummy_dftable frov_load_dataframe_from_csv(std::string& filename,
                                          std::vector<std::string>& types,
                                          std::vector<std::string>& names,
                                          bool& partial_type_info, 
                                          std::map<std::string, std::string>& type_map,
                                          std::vector<int>& usecols,
                                          std::vector<std::string>& bool_cols,
                                          csv_config& config);
size_t get_dataframe_length(exrpc_ptr_t& df_proxy);

dummy_dftable
frov_df_convert_dicstring_to_bool(exrpc_ptr_t& df_proxy,
                                 std::vector<std::string>& col_names,
                                 std::string& nullstr);

dummy_dftable
frov_df_append_column(exrpc_ptr_t& df_proxy, 
                      std::string& col_name,
                      short& type, 
                      exrpc_ptr_t& dvec_proxy, int& position,
                      bool& drop_old);

dummy_dftable
frov_df_add_index(exrpc_ptr_t& df_proxy, std::string& name);

dummy_dftable 
frov_df_reset_index(exrpc_ptr_t& df_proxy, bool& drop);

dummy_dftable
frov_df_set_index(exrpc_ptr_t& df_proxy, 
                  std::string& cur_index_name, // existing index column
                  std::string& new_index_name, // existing column to be set as index
                  bool& verify_integrity);

dummy_dftable
frov_df_drop_duplicates(exrpc_ptr_t& df_proxy, 
                        std::vector<std::string>& cols,
                        std::string& keep);

dummy_dftable
frov_df_union(exrpc_ptr_t& df_proxy, 
              std::vector<exrpc_ptr_t>& proxies,
              std::vector<std::string>& names, 
              bool& verify_integrity);

dummy_dftable
frov_df_astype(exrpc_ptr_t& df_proxy,
               std::vector<std::string>& cols,
               std::vector<short>& types);

dummy_dftable
frov_df_set_col_order(exrpc_ptr_t& df_proxy,
                      std::vector<std::string>& new_cols);

void copy_column_helper(dftable& to_df,
                        dftable_base& from_df,
                        std::string& cname,
                        short& dtype);

dummy_dftable 
frov_df_copy_index(exrpc_ptr_t& to_df, 
                   exrpc_ptr_t& from_df,
                   std::string& index_col_name,
                   short& dtype);

dummy_dftable 
frov_df_copy_column(exrpc_ptr_t& to_df, 
                    exrpc_ptr_t& from_df,
                    std::vector<std::string>& names,
                    std::vector<std::string>& names_as,
                    std::vector<short>& dtypes); 

dftable fillna(dftable& df, 
	       std::string& fill_value, 
	       bool has_index);

dummy_dftable 
frov_df_fillna(exrpc_ptr_t& df_proxy, 
               std::string& fill_value, 
               bool& has_index);

#endif
