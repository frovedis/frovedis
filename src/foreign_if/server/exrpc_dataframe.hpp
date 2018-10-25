#ifndef _EXRPC_DF_HPP_
#define _EXRPC_DF_HPP_

#include <frovedis/dataframe.hpp>
#include <frovedis/dataframe/dfaggregator.hpp>
#include "../exrpc/exrpc_expose.hpp"
#include "model_tracker.hpp" // for OPTYPE

using namespace frovedis;

exrpc_ptr_t create_dataframe (std::vector<short>& types,
                              std::vector<std::string>& cols,
                              std::vector<exrpc_ptr_t>& dvec_proxies);

void show_dataframe(exrpc_ptr_t& df_proxy); 

// convert a numeric string to number
template <class T>
T cast (std::string& data) {
  T c_data = 0;
  try {
    c_data = boost::lexical_cast<T>(data);
  }
  catch (const boost::bad_lexical_cast &excpt) {
    REPORT_ERROR(USER_ERROR, "Invalid operands in filter operation: " + data);
  }
  return c_data;
}

template <class T>
exrpc_ptr_t get_dfoperator(std::string& op1, std::string& op2,
                           short& op_id, bool& isImmed) {
  std::shared_ptr<dfoperator> *opt = NULL;
  if(isImmed) {
    auto data = cast<T>(op2);
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

exrpc_ptr_t filter_df(exrpc_ptr_t& df_proxy, exrpc_ptr_t& opt_proxy);

exrpc_ptr_t select_df(exrpc_ptr_t& df_proxy, std::vector<std::string>& cols);

exrpc_ptr_t sort_df(exrpc_ptr_t& df_proxy, 
                    std::vector<std::string>& cols, bool& isDesc);

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
                                         std::vector<std::string>& cols,
                                         std::vector<short>& types);

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
                               std::vector<std::string>& new_cols);

std::vector<int> get_df_int_col(exrpc_ptr_t& df_proxy,
                                std::string& cname);

std::vector<long> get_df_long_col(exrpc_ptr_t& df_proxy,
                                  std::string& cname);

std::vector<double> get_df_double_col(exrpc_ptr_t& df_proxy,
                                      std::string& cname);

std::vector<float> get_df_float_col(exrpc_ptr_t& df_proxy,
                                    std::string& cname);

std::vector<std::string> 
get_df_string_col(exrpc_ptr_t& df_proxy, std::string& cname);

exrpc_ptr_t frovedis_gdf_aggr(exrpc_ptr_t& df_proxy, 
                              std::vector<std::string>& groupedCols,
                              std::vector<std::string>& aggFuncs,
                              std::vector<std::string>& aggCols,
                              std::vector<std::string>& aggAsCols);

#endif
