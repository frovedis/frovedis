#ifndef _EXRPC_DF_HPP_
#define _EXRPC_DF_HPP_

#include <frovedis/dataframe.hpp>
#include <frovedis/matrix/matrix_sort.hpp>
#include <frovedis/dataframe/dfaggregator.hpp>
#include "../exrpc/exrpc_expose.hpp"
#include "model_tracker.hpp" // for OPTYPE, model_table to register sparse_conv_info
#include "frovedis_mem_pair.hpp"
#include "dummy_matrix.hpp"
#include "exrpc_data_storage.hpp"

using namespace frovedis;

bool is_present(const std::vector<std::string>& vec,
                const std::string& val);

dftable* get_dftable_pointer(exrpc_ptr_t& df_proxy);

long calc_memory_size(exrpc_ptr_t& df_proxy);

template <class T>
exrpc_ptr_t get_df_column_pointer(exrpc_ptr_t& df_proxy, 
                                  std::string& cname) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  auto cptr = new dvector<T>(df.as_dvector<T>(cname));
  return reinterpret_cast<exrpc_ptr_t>(cptr);
}

void fillna_helper(dftable& ret, const std::string& cname, 
                   std::shared_ptr<dfcolumn>& dfcol, 
                   double fillv);

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

void treat_str_nan_as_null(std::vector<std::string>& vec); // defined in exrpc_dataframe.cc

template <class T>
void treat_nan_as_null(std::vector<T>& vec) {
  auto nanpos = vector_find_nan(vec);
  auto tmax = std::numeric_limits<T>::max();
  auto vptr = vec.data();
  auto nptr = nanpos.data();
  auto nsz = nanpos.size();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < nsz; ++i) vptr[nptr[i]] = tmax;
}

template <class T> // for T: int, long, ulong etc.
std::vector<double> 
treat_null_as_nan(const std::vector<T>& vec,
                  const std::vector<size_t>& nulls) {
  auto ret = vector_astype<double>(vec); // casting is needed for int-like vectors...
  auto mynan = std::numeric_limits<double>::quiet_NaN();
  auto rptr = ret.data();
  auto nptr = nulls.data();
  auto nsz = nulls.size();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < nsz; ++i) rptr[nptr[i]] = mynan;
  return ret;
}

template <class T> // for T: float, double
void treat_null_as_nan_inplace(std::vector<T>& vec,
                               const std::vector<size_t>& nulls) {
  auto mynan = std::numeric_limits<T>::quiet_NaN();
  auto rptr = vec.data();
  auto nptr = nulls.data();
  auto nsz = nulls.size();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < nsz; ++i) rptr[nptr[i]] = mynan;
}

template <class T> // for T: long
void treat_null_as_nat_inplace(std::vector<T>& vec,
                               const std::vector<size_t>& nulls) {
  auto mynat = std::numeric_limits<T>::min();
  auto rptr = vec.data();
  auto nptr = nulls.data();
  auto nsz = nulls.size();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < nsz; ++i) rptr[nptr[i]] = mynat;
}

exrpc_ptr_t create_dataframe (std::vector<short>& types,
                              std::vector<std::string>& cols,
                              std::vector<exrpc_ptr_t>& dvec_proxies,
                              bool& nan_as_null);

exrpc_ptr_t create_dataframe_from_local_vectors (
            std::vector<short>& types,
            std::vector<std::string>& cols,
            std::vector<exrpc_ptr_t>& local_vec_proxies,
            std::vector<int>& offset);

void show_dataframe(exrpc_ptr_t& df_proxy); 

dummy_dftable 
copy_spark_column(exrpc_ptr_t& self_proxy,
                  std::vector<exrpc_ptr_t>& proxies,
                  std::vector<std::string>& cols);

template <class T>
exrpc_ptr_t get_dffunc_im(std::string& value) {
  std::shared_ptr<dffunction> *retptr = NULL;
  if (value == "NULL") {
    retptr = new std::shared_ptr<dffunction>(null_column<T>());
  } else {
    auto val = do_cast<T>(value);
    retptr = new std::shared_ptr<dffunction>(im(val));
  }
  return reinterpret_cast<exrpc_ptr_t> (retptr);
}

dummy_dftable
frovedis_series_string_methods(exrpc_ptr_t& df_proxy,
                               std::string& cname,
                               std::string& param,
                               short& op_id,
                               bool& with_index);

dummy_dftable
frovedis_series_slice(exrpc_ptr_t& df_proxy,
                      std::string& cname,
                      int& start, int& stop, int& step,
                      bool& with_index);

dummy_dftable
frovedis_series_pad(exrpc_ptr_t& df_proxy,
                    std::string& cname, 
                    std::string& side,
                    std::string& value,
                    int& len, bool& with_index);

void frovedis_df_to_csv(exrpc_ptr_t& df_proxy,
                        std::string& fname,
                        std::string& mode,
                        std::string& sep,
                        std::string& nullstr,
                        std::string& date_format,
                        size_t& precision);

exrpc_ptr_t get_dffunc_string_im(std::string& value);

exrpc_ptr_t get_dffunc_bool_im(std::string& value);

exrpc_ptr_t get_dffunc_id(std::string& cname);

exrpc_ptr_t get_dffunc_opt(exrpc_ptr_t& leftp, exrpc_ptr_t& rightp,
                           short& opt_id, std::string& cname);

exrpc_ptr_t get_immed_substr(exrpc_ptr_t& colp, int& pos, int& num,
                             std::string& cname);

exrpc_ptr_t get_immed_substr_index(exrpc_ptr_t& colp, 
                                   std::string& delim, int& num,
                                   std::string& cname);

exrpc_ptr_t get_col_substr(exrpc_ptr_t& colp, 
                           exrpc_ptr_t& posp, exrpc_ptr_t& nump,
                           std::string& cname);

exrpc_ptr_t get_col_concat_multi(std::vector<exrpc_ptr_t>& cols,
                                 std::string& as,
                                 std::string& sep, bool& with_sep);

exrpc_ptr_t get_immed_pad(exrpc_ptr_t& colp,
                          int& len, std::string& value,
                          std::string& cname, bool& is_left);

exrpc_ptr_t get_immed_from_to_opt(exrpc_ptr_t& colp,
                                  std::string& from,
                                  std::string& to,
                                  short& opt_id,
                                  std::string& cname);

exrpc_ptr_t get_immed_locate(exrpc_ptr_t& colp, 
                             std::string& substr, int& pos, 
                             std::string& cname);

exrpc_ptr_t append_when_condition(exrpc_ptr_t& leftp, 
                                  exrpc_ptr_t& rightp,
                                  std::string& cname);

exrpc_ptr_t get_dffunc_agg(exrpc_ptr_t& leftp,
                           short& opt_id, std::string& cname, 
                           bool& ignore_nulls);

dummy_dftable append_scalar(exrpc_ptr_t& dfproxy, std::string& cname, 
                            std::string& value, short& dtype);

dummy_dftable execute_dffunc(exrpc_ptr_t& dfproxy, std::string& cname,
                             exrpc_ptr_t& dffunc); 

dummy_dftable execute_dfagg(exrpc_ptr_t& dfproxy, std::vector<exrpc_ptr_t>& agg); 

void set_dffunc_asCol_name(exrpc_ptr_t& fn, std::string& cname);

void set_dfagg_asCol_name(exrpc_ptr_t& fn, std::string& cname);

exrpc_ptr_t get_immed_string_dffunc_opt(exrpc_ptr_t& leftp,
                                        std::string& right,
                                        short& opt_id,
                                        std::string& cname,
                                        bool& is_rev);

template <class T>
exrpc_ptr_t get_immed_when_dffunc(exrpc_ptr_t& leftp,
                                  T& right,
                                  short& opt_id,
                                  std::string& cname) {
  std::shared_ptr<dffunction> *opt = NULL;
  switch(opt_id) {
    case IF:        
    case ELIF: {
      auto& left = *reinterpret_cast<std::shared_ptr<dfoperator>*>(leftp);
      opt = new std::shared_ptr<dffunction>(when({left >> im(right)})->as(cname));
      // ELIF: must also call append_when_condition() to update if-elif conditions from client side
      break;
    }
    case ELSE: { 
      auto& left = *reinterpret_cast<std::shared_ptr<dffunction_when>*>(leftp);
      auto cond = left->cond;
      auto func = left->func;
      func.push_back(im(right)); // added else function
      opt = new std::shared_ptr<dffunction>(when(cond, func)->as(cname)); break;
    }
    default: REPORT_ERROR(USER_ERROR, "when: Unsupported dffunction is requested!\n");
  }
  return reinterpret_cast<exrpc_ptr_t> (opt);
}

exrpc_ptr_t get_immed_int_dffunc_opt(exrpc_ptr_t& leftp,
                                     int& right,
                                     short& opt_id,
                                     std::string& cname);

// for immediate value (right) of non-string type, T
// where left is a "dffunction", used in spark wrapper
template <class T>
exrpc_ptr_t get_immed_dffunc_opt(exrpc_ptr_t& leftp,
                                 std::string& right_str,
                                 short& opt_id,
                                 std::string& cname,
                                 bool& is_rev) {
  auto right = do_cast<T>(right_str);
  if (opt_id == IF || opt_id == ELIF || opt_id == ELSE)
    return get_immed_when_dffunc(leftp, right, opt_id, cname);

  auto& left = *reinterpret_cast<std::shared_ptr<dffunction>*>(leftp);
  std::shared_ptr<dffunction> *opt = NULL;
  if (!is_rev) {
    switch(opt_id) {
      // --- conditional ---
      case EQ:   opt = new std::shared_ptr<dffunction>((left == right)->as(cname)); break;
      case NE:   opt = new std::shared_ptr<dffunction>((left != right)->as(cname)); break;
      case LT:   opt = new std::shared_ptr<dffunction>((left < right)->as(cname)); break;
      case LE:   opt = new std::shared_ptr<dffunction>((left <= right)->as(cname)); break;
      case GT:   opt = new std::shared_ptr<dffunction>((left > right)->as(cname)); break;
      case GE:   opt = new std::shared_ptr<dffunction>((left >= right)->as(cname)); break;
      // --- mathematical ---
      case ADD:  opt = new std::shared_ptr<dffunction>(add_im_as (left, right, cname)); break;
      case SUB:  opt = new std::shared_ptr<dffunction>(sub_im_as (left, right, cname)); break;
      case MUL:  opt = new std::shared_ptr<dffunction>(mul_im_as (left, right, cname)); break;
      case IDIV: opt = new std::shared_ptr<dffunction>(idiv_im_as(left, right, cname)); break;
      case FDIV: opt = new std::shared_ptr<dffunction>(fdiv_im_as(left, right, cname)); break;
      case MOD:  opt = new std::shared_ptr<dffunction>(mod_im_as (left, right, cname)); break;
      case POW:  opt = new std::shared_ptr<dffunction>(pow_im_as (left, right, cname)); break;
      // --- date ---
      case DATEDIFF:  REPORT_ERROR(USER_ERROR, 
                        "date_diff: supported only for string as immediate value!\n");
      case MONTHSBETWEEN: REPORT_ERROR(USER_ERROR, 
                         "months_between: supported only for string as immediate value!\n");
      default:   REPORT_ERROR(USER_ERROR, "Unsupported dffunction is requested!\n");
    }
  } else {
    switch(opt_id) {
      // --- conditional ---
      case EQ:   opt = new std::shared_ptr<dffunction>((right == left)->as(cname)); break;
      case NE:   opt = new std::shared_ptr<dffunction>((right != left)->as(cname)); break;
      case LT:   opt = new std::shared_ptr<dffunction>((right < left)->as(cname)); break;
      case LE:   opt = new std::shared_ptr<dffunction>((right <= left)->as(cname)); break;
      case GT:   opt = new std::shared_ptr<dffunction>((right > left)->as(cname)); break;
      case GE:   opt = new std::shared_ptr<dffunction>((right >= left)->as(cname)); break;
      // --- mathematical ---
      case ADD:  opt = new std::shared_ptr<dffunction>(add_im_as (right, left, cname)); break;
      case SUB:  opt = new std::shared_ptr<dffunction>(sub_im_as (right, left, cname)); break;
      case MUL:  opt = new std::shared_ptr<dffunction>(mul_im_as (right, left, cname)); break;
      case IDIV: opt = new std::shared_ptr<dffunction>(idiv_im_as(right, left, cname)); break;
      case FDIV: opt = new std::shared_ptr<dffunction>(fdiv_im_as(right, left, cname)); break;
      case MOD:  opt = new std::shared_ptr<dffunction>(mod_im_as (right, left, cname)); break;
      case POW:  opt = new std::shared_ptr<dffunction>(pow_im_as (right, left, cname)); break;
      // --- date ---
      case DATEDIFF:  REPORT_ERROR(USER_ERROR, 
                         "date_diff: reversed operation is supported only for string as immediate value!\n");
      case MONTHSBETWEEN: REPORT_ERROR(USER_ERROR, 
                         "months_between: reversed operation is supported only for string as immediate value!\n");
      default:   REPORT_ERROR(USER_ERROR, "Unsupported dffunction is requested!\n");
    }
  }
  return reinterpret_cast<exrpc_ptr_t> (opt);
}

// where left is a "String", used in python wrapper
template <class T>
exrpc_ptr_t get_dfoperator(std::string& op1, std::string& op2,
                           short& op_id, bool& isImmed) {
  std::shared_ptr<dfoperator> *opt = NULL;
  // op2 would be treated as pattern, instead of immediate value in case of LIKE/NLIKE
  if(op_id == LIKE || op_id == NLIKE) isImmed = false;
  if(isImmed) {
    auto data = do_cast<T>(op2);
    switch(op_id) {
        case EQ: opt = new std::shared_ptr<dfoperator>(eq_im(op1,data)); break;
        case NE: opt = new std::shared_ptr<dfoperator>(neq_im(op1,data)); break;
        case LT: opt = new std::shared_ptr<dfoperator>(lt_im(op1,data)); break;
        case LE: opt = new std::shared_ptr<dfoperator>(le_im(op1,data)); break;
        case GT: opt = new std::shared_ptr<dfoperator>(gt_im(op1,data)); break;
        case GE: opt = new std::shared_ptr<dfoperator>(ge_im(op1,data)); break;
        default: REPORT_ERROR(USER_ERROR,
                 "Unsupported conditional operation is encountered!\n");
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
                 "Unsupported conditional operation is encountered!\n");
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
dummy_dftable fselect_df(exrpc_ptr_t& df_proxy, std::vector<exrpc_ptr_t>& funcp);
exrpc_ptr_t isnull_df(exrpc_ptr_t& df_proxy, std::vector<std::string>& cols, 
                      bool& with_index);
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
                    std::string& how, std::string& join_type,
                    bool& check_opt_proxy, std::string& rsuf);

exrpc_ptr_t group_by_df(exrpc_ptr_t& df_proxy, std::vector<std::string>& cols);
exrpc_ptr_t fgroup_by_df(exrpc_ptr_t& df_proxy, std::vector<exrpc_ptr_t>& funcp);

long frovedis_df_size(exrpc_ptr_t& df_proxy);

std::vector<std::string> frovedis_df_sum(exrpc_ptr_t& df_proxy,
                                         std::vector<std::string>& cols,
                                         std::vector<short>& types);

std::vector<std::string> frovedis_df_avg(exrpc_ptr_t& df_proxy,
                                         std::vector<std::string>& cols);

std::vector<std::string> frovedis_df_std(exrpc_ptr_t& df_proxy,
                                         std::vector<std::string>& cols);

std::vector<std::string> frovedis_df_sem(exrpc_ptr_t& df_proxy,
                                         std::vector<std::string>& cols);

std::vector<std::string> frovedis_df_var(exrpc_ptr_t& df_proxy,
                                         std::vector<std::string>& cols);

std::vector<std::string> frovedis_df_mad(exrpc_ptr_t& df_proxy,
                                         std::vector<std::string>& cols);

std::vector<std::string> frovedis_df_median(exrpc_ptr_t& df_proxy,
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
                               std::vector<std::string>& new_cols,
                               bool& inplace);

dummy_vector get_df_string_col(exrpc_ptr_t& df_proxy, std::string& cname);

// for pandas wrapper...
template <class T>
dummy_vector get_df_col(exrpc_ptr_t& df_proxy, 
                        std::string& cname, 
                        short& ctype, 
                        bool& need_nat_treatment) { // for TimeDelta and DateTime
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  auto col = df.column(cname);
  use_dfcolumn use(col);
  dummy_vector dvec;
  if (col->if_contain_nulls()) {
    auto nullpos = col->get_nulls();
    auto vec = df.as_dvector<T>(cname);
    if (col->dtype() == "float" || col->dtype() == "double") { // no need for casting, null-treatment can be in-place
      auto retp = new dvector<T>(vec.mapv_partitions(
                    treat_null_as_nan_inplace<T>, nullpos));
      auto retp_ = reinterpret_cast<exrpc_ptr_t>(retp);
      dvec = dummy_vector(retp_, retp->size(), ctype);
    } else if (need_nat_treatment || col->dtype() == "datetime") { // long dvector
      auto retp = new dvector<T>(vec.mapv_partitions(
                    treat_null_as_nat_inplace<T>, nullpos));
      auto retp_ = reinterpret_cast<exrpc_ptr_t>(retp);
      dvec = dummy_vector(retp_, retp->size(), ctype);
    } else {
      auto retp = new dvector<double>(vec.map_partitions(
                    treat_null_as_nan<T>, nullpos));
      auto retp_ = reinterpret_cast<exrpc_ptr_t>(retp);
      dvec = dummy_vector(retp_, retp->size(), DOUBLE);
    }
  } else {
    auto retp = new dvector<T>(df.as_dvector<T>(cname));
    auto retp_ = reinterpret_cast<exrpc_ptr_t>(retp);
    dvec = dummy_vector(retp_, retp->size(), ctype);
  }
  return dvec;
}

exrpc_ptr_t frovedis_gdf_select(exrpc_ptr_t& df_proxy, 
                                std::vector<std::string>& tcols);

dummy_dftable frovedis_gdf_fselect(exrpc_ptr_t& df_proxy, 
                                   std::vector<exrpc_ptr_t>& funcp);

dummy_dftable 
frovedis_gdf_agg_select(exrpc_ptr_t& df_proxy,
                        std::vector<std::string>& cols,
                        std::vector<exrpc_ptr_t>& aggp);

dummy_dftable 
frovedis_gdf_agg_fselect(exrpc_ptr_t& df_proxy,
                        std::vector<exrpc_ptr_t>& funcp,
                        std::vector<exrpc_ptr_t>& aggp);

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
                                          csv_config& config, bool& is_all_bools);
size_t get_dataframe_length(exrpc_ptr_t& df_proxy);

dummy_dftable
frov_df_append_column(exrpc_ptr_t& df_proxy, 
                      std::string& col_name,
                      short& type, 
                      exrpc_ptr_t& dvec_proxy, int& position,
                      bool& drop_old,
                      bool& nan_as_null);

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
               std::vector<short>& types,
               bool& check_bool_like_string);

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

std::string frov_df_to_string(exrpc_ptr_t& df_proxy, bool& has_index);

dummy_dftable
frov_df_dropna_by_rows(exrpc_ptr_t& df_proxy,
                       std::vector<std::string>& targets,
                       std::string& how,
                       size_t& threshold);

template <class T>
dummy_dftable
frov_df_dropna_by_cols(exrpc_ptr_t& df_proxy,
                       std::string& tcol,
                       std::vector<T>& targets,
                       std::string& how,
                       size_t& threshold) {
  auto df = reinterpret_cast<dftable_base*>(df_proxy);
  dftable ret;
  if (threshold == std::numeric_limits<size_t>::max()) {
    ret = df->drop_nulls_by_cols(how, tcol, targets);
  } else {
    ret = df->drop_nulls_by_cols(threshold, tcol, targets);
  }
  auto retp = new dftable(std::move(ret));
  return to_dummy_dftable(retp);
}

template <class T>
void append_null(dftable& df, 
		 const std::string& cname, 
		 size_t num_row) {
  std::vector<size_t> sizes;
  if (df.num_row() == 0) sizes = get_block_sizes(num_row);
  else                   sizes = df.num_rows();
  auto mysz = make_node_local_scatter(sizes);
  auto vals = make_node_local_allocate<std::vector<T>>();
  auto nulls = make_node_local_allocate<std::vector<size_t>>();
  vals.mapv(+[](std::vector<T>& vals, size_t mysz) {
                 vals = vector_full<T>(mysz, std::numeric_limits<T>::max());
             }, mysz);
  nulls.mapv(+[](std::vector<size_t>& nulls, size_t mysz) {
                 nulls = vector_arrange<size_t>(mysz);
             }, mysz);
  auto null_col = std::make_shared<typed_dfcolumn<T>>(std::move(vals), std::move(nulls));
  df.append_column(cname, null_col);
}

template <class T>
void append_value(dftable& df, 
		  const std::string& cname, 
		  size_t num_row,
                  const std::string& fill_value) {
  std::vector<size_t> sizes;
  if (df.num_row() == 0) sizes = get_block_sizes(num_row);
  else                   sizes = df.num_rows();
  auto mysz = make_node_local_scatter(sizes);
  auto vals = make_node_local_allocate<std::vector<T>>();
  auto fillv = do_cast<T>(fill_value); // might raise exception
  vals.mapv(+[](std::vector<T>& vals, size_t mysz, T fillv) {
                 vals = vector_full<T>(mysz, fillv);
             }, mysz, broadcast(fillv));
  df.append_column(cname, vals.template moveto_dvector<T>()); 
}

dummy_dftable frov_df_head(exrpc_ptr_t& df_proxy, size_t& limit);

dummy_dftable frov_df_tail(exrpc_ptr_t& df_proxy, size_t& limit);

dummy_dftable frov_df_slice_range(exrpc_ptr_t& df_proxy, 
                                  size_t& a, size_t& b,
                                  size_t& c);
std::vector<size_t> 
frov_df_get_index_loc(exrpc_ptr_t& df_proxy,
                      std::string& column,
                      std::string& value,
                      short& dtype);

dummy_dftable frov_df_countna(exrpc_ptr_t& df_proxy, 
                              int& axis, bool& with_index);

dummy_dftable frov_df_ksort(exrpc_ptr_t& df_proxy, int& k,
                            std::vector<std::string>& targets,
                            std::string& keep,
                            bool& is_desc);

dummy_dftable frov_df_mean(exrpc_ptr_t& df_proxy, 
                           std::vector<std::string>& cols,
                           int& axis, bool& skip_na, bool& with_index);

dummy_dftable frov_df_var(exrpc_ptr_t& df_proxy, 
                           std::vector<std::string>& cols,
                           int& axis, bool& skip_na, double& ddof, bool& with_index);

dummy_dftable frov_df_mad(exrpc_ptr_t& df_proxy, 
                           std::vector<std::string>& cols,
                           int& axis, bool& skip_na, bool& with_index);

dummy_dftable frov_df_std(exrpc_ptr_t& df_proxy, 
                           std::vector<std::string>& cols,
                           int& axis, bool& skip_na, double& ddof, bool& with_index);
dummy_dftable frov_df_sem(exrpc_ptr_t& df_proxy, 
                          std::vector<std::string>& cols,
                          int& axis, bool& skip_na, double& ddof, bool& with_index);
dummy_dftable frov_df_median(exrpc_ptr_t& df_proxy, 
                           std::vector<std::string>& cols,
                           std::vector<short>& types,
                           int& axis, bool& skip_na, bool& with_index);
dummy_dftable frov_df_cov(exrpc_ptr_t& df_proxy,
                          std::vector<std::string>& cols,
                          int& min_periods, double& ddof,
                          bool& low_memory, bool& with_index);
// TODO: remove (if unused)
double frov_col_cov(exrpc_ptr_t& df_proxy,
                   std::string& col1,
                   int& min_periods, double& ddof,
                   bool& with_index);
// TODO: remove (if unused)
double frov_col2_cov(exrpc_ptr_t& df_proxy,
                   std::string& col1, std::string& col2,
                   int& min_periods, double& ddof,
                   bool& with_index);
double frov_series_cov(exrpc_ptr_t& self_proxy, std::string& col1,
                       exrpc_ptr_t& other_proxy, std::string& col2,
                       int& min_periods, double& ddof);
dummy_dftable 
frovedis_gdf_aggr_with_ddof(exrpc_ptr_t& df_proxy,
                          std::vector<std::string>& groupedCols,
                          std::string& aggFunc,
                          std::vector<std::string>& aggCols,
                          std::vector<std::string>& aggAsCols,
                          double& ddof);

dummy_dftable
frovedis_gdf_aggr_with_mincount(exrpc_ptr_t& df_proxy,
                                std::vector<std::string>& groupedCols,
                                std::string& aggFunc,
                                std::vector<std::string>& aggCols,
                                std::vector<std::string>& aggAsCols,
                                int& mincount);

dummy_dftable
frov_df_mode_cols(exrpc_ptr_t& df_proxy, 
                  std::vector<std::string>& cols,
                  bool& dropna);
// TODO: improve vectorization in intialize_cnts_rows, update_counts
template <class T>
std::vector<std::map<T, size_t>>
intialize_cnts_rows(std::vector<T>& val, std::vector<T>& most_freq, T null_val, bool dropna){
    
    auto sz = val.size();
    std::vector<T> res_vec(sz);
    std::vector<T> initial_most_freq(sz);

    std::vector<std::map<T, size_t>> cnts_rows(sz);
    for(size_t i=0; i<sz; i++){
        if (dropna && val[i]==null_val) continue;
        cnts_rows[i][val[i]]++;
        initial_most_freq[i] = val[i];
    }

    most_freq.swap(initial_most_freq);
    return cnts_rows;
}

template <class T>
void update_counts( std::vector<std::map<T, size_t> >& cnts_rows,  std::vector<T>& val,
                std::vector<T>& most_freq, T null_val, bool dropna) {
    auto sz = val.size();
    for(size_t i=0; i<sz; i++){
        if (dropna && val[i]==null_val) continue;
        cnts_rows[i][val[i]]++;

        if (cnts_rows[i][val[i]] > cnts_rows[i][most_freq[i]] ) most_freq[i] = val[i];
    }
}

dftable mode_rows_numeric(dftable& df, std::vector<std::string>& columns,
                          bool include_index, bool dropna = true);

dftable mode_rows_str(dftable& df, std::vector<std::string>& columns,
                      bool include_index, bool dropna = true);

dummy_dftable
frov_df_mode_rows(exrpc_ptr_t& df_proxy, 
                  std::vector<std::string>& col_names,
                  bool& is_string,
                  bool& dropna);

dummy_vector frov_get_bool_mask(exrpc_ptr_t& df_opt_proxy, 
                                exrpc_ptr_t& df_proxy,
                                bool& ignore_null);

exrpc_ptr_t frov_df_filter_using_mask(exrpc_ptr_t& df_proxy,
                                      exrpc_ptr_t& mask_dvec_proxy);

exrpc_ptr_t frov_df_distinct(exrpc_ptr_t& df_proxy);

void append_na_count(dftable& df,
                     int axis, const std::string& cname);
template <class T>
dftable fillna_and_append_non_na_count(dftable_base& df, T fillv,
                                       int axis, const std::string& cname) { // assumes no index
  dftable ret;
  auto cols = df.columns();
  auto ncol = cols.size();
  for (auto c: cols) {
    use_dfcolumn use(df.raw_column(c));
    auto dfcol = df.column(c);
    fillna_helper(ret, c, dfcol, fillv);
  }

  auto count_na = df.count_nulls(axis); 
  auto subt_fn = sub_im(ncol, "count");
  use_dfcolumn use(subt_fn->columns_to_use(count_na));
  ret.append_column(cname, subt_fn->execute(count_na));
  return ret;
}

template <class T>
dftable sum_axis0_helper(dftable_base& df,         
                         std::vector<std::string>& cols,
                         std::vector<short>& types,
                         bool skip_na, int min_count) {
  auto ncol = cols.size();
  auto nrow = df.num_row();
  checkAssumption (ncol == types.size());
  std::vector<T> sum_res(ncol); auto resp = sum_res.data();
  auto tmax = std::numeric_limits<T>::max();
  
  if (nrow < min_count) { // quick check: applicable for both with_na and skip_na
    for(size_t i = 0; i < ncol; ++i) resp[i] = tmax;
  } else {
    for(size_t i = 0; i < ncol; ++i) {
      auto col = cols[i];
      auto is_invalid = skip_na ? df.count(col) < min_count  // non-nulls are less than threshold
                                : df.count(col) != nrow;     // one or more nulls
      if (is_invalid) resp[i] = tmax;
      else {
        switch(types[i]) {
          case BOOL:
          case INT:    resp[i] = df.sum<int>(col);           break;
          case LONG:   resp[i] = df.sum<long>(col);          break;
          case ULONG:  resp[i] = df.sum<unsigned long>(col); break;
          case FLOAT:  resp[i] = df.sum<float>(col);         break;
          case DOUBLE: resp[i] = df.sum<double>(col);        break;
          default:     REPORT_ERROR(USER_ERROR, "sum on non-numeric column!\n");
        }
      }
    }
  }
  dftable ret;
  ret.append_column("index", make_dvector_scatter(cols));
  ret.append_column("sum", make_dvector_scatter(sum_res), true);
  return ret;
}

template <class T>
dftable sum_axis1_helper(dftable_base& df,         
                         std::vector<std::string>& cols,
                         std::vector<short>& types,
                         bool skip_na, int min_count,
                         bool with_index) {
  dftable ret;
  auto ncol = cols.size();
  if ((ncol == 0 ) || (ncol < min_count)){
    append_null<T>(ret, "sum", df.num_row());
  } 
  else {
    auto target_df = df.select(cols);  // no index in selected columns
    auto func = (ncol == 1) ? ~cols[0] + 0 : ~cols[0] + ~cols[1];
    for(size_t i = 2; i < ncol; ++i) func = func + ~cols[i];
    if (skip_na) {
      std::string n_valid = "n_valid";
      target_df = fillna_and_append_non_na_count(target_df, 0, 1, n_valid);
      // if non-na in each row >= min_count, apply "func"; else put NULL
      auto cond = when({(~n_valid >= min_count) >> func});
      use_dfcolumn use(cond->columns_to_use(target_df));
      ret.append_column("sum", cond->execute(target_df));
    } else {
      use_dfcolumn use(func->columns_to_use(target_df));
      ret.append_column("sum", func->execute(target_df));
    }
  }
  // use index as it is in input dataframe, if any. otherwise add index.
  if (with_index) {
    auto index_nm = df.columns()[0]; 
    use_dfcolumn use(df.raw_column(index_nm));
    ret.append_column(index_nm, df.column(index_nm))
       .change_col_position(index_nm, 0);
  } else {
    ret.prepend_rowid<long>("index");
  }
  return ret;
}

template <class T>
dftable frov_df_sum_impl(exrpc_ptr_t& df_proxy,
                          std::vector<std::string>& cols,
                          std::vector<short>& types,
                          int& axis, bool& skip_na, 
                          int& min_count,
                          bool& with_index) {
  dftable ret;
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  if (axis == 0) 
    ret = sum_axis0_helper<T>(df, cols, types, skip_na, min_count);
  else if (axis == 1)
    ret = sum_axis1_helper<T>(df, cols, types, skip_na, min_count, with_index);
  else REPORT_ERROR(USER_ERROR, "sum: supported axis: 0 and 1 only!\n");
  return ret;
}

template <class T>
dummy_dftable 
frov_df_sum(exrpc_ptr_t& df_proxy,
             std::vector<std::string>& cols,
             std::vector<short>& types,
             int& axis, bool& skip_na, int& min_count,
             bool& with_index) {
  auto ret = frov_df_sum_impl<T>(df_proxy, cols, types, axis, skip_na, 
                                  min_count, with_index);
  auto retp = new dftable(std::move(ret));
  return to_dummy_dftable(retp);
}

template <class T>
dftable max_axis0_helper(dftable_base& df,         
                         std::vector<std::string>& cols,
                         std::vector<short>& types,
                         bool skip_na) {
  auto ncol = cols.size();
  auto nrow = df.num_row();
  checkAssumption (ncol == types.size());
  std::vector<T> max_res(ncol); auto resp = max_res.data();
  auto tmax = std::numeric_limits<T>::max();
  
  for(size_t i = 0; i < ncol; ++i) {
    auto col = cols[i];
    auto is_invalid = !skip_na && df.count(col) != nrow;
    if (is_invalid) resp[i] = tmax;
    else {
      switch(types[i]) {
        case BOOL:
        case INT:    resp[i] = df.max<int>(col);           break;
        case LONG:   resp[i] = df.max<long>(col);          break;
        case ULONG:  resp[i] = df.max<unsigned long>(col); break;
        case FLOAT:  resp[i] = df.max<float>(col);         break;
        case DOUBLE: resp[i] = df.max<double>(col);        break;
        default:     REPORT_ERROR(USER_ERROR, "max on non-numeric column!\n");
      }
    }
  }
  
  dftable ret;
  ret.append_column("index", make_dvector_scatter(cols));
  ret.append_column("max", make_dvector_scatter(max_res), true);
  return ret;
}

void fillna_column_min_typed_helper(
                         std::shared_ptr<frovedis::dfcolumn>& col,
                         short type);
dftable
fillna_with_min(dftable& df, std::vector<std::string>& cols, 
                std::vector<short>& types);

template <class T>
dftable max_axis1_helper(dftable_base& df,         
                         std::vector<std::string>& cols,
                         std::vector<short>& types,
                         bool skip_na,
                         bool with_index) {
  dftable ret;
  auto ncol = cols.size();
  if (ncol == 0 ) {
    append_null<T>(ret, "max", df.num_row());
  } 
  else {
    auto target_df = df.select(cols);  // no index in selected columns
    dftable count_na_df;
    count_na_df = target_df.count_nulls(1);
    if (skip_na) target_df = fillna_with_min(target_df, cols, types);
    std::string max = "max";
    if (ncol == 1) {
      use_dfcolumn use(target_df.raw_column(cols[0]));
      ret.append_column(max, target_df.column(cols[0]));
    }
    else {
      auto func = when({(~cols[0] >= ~cols[1]) >> ~cols[0]}, ~cols[1])->as(max);
      target_df.call_function(func); 
      for(size_t i = 2; i < ncol; ++i) {
        target_df.call_function(when({(~max >= ~cols[i]) >> ~max}, ~cols[i])); 
        target_df.drop(max);
        target_df.rename("when", max);
      }
      std::string nan_count = "nan_count";
      use_dfcolumn use(target_df.raw_column(max));
      ret.append_column(max, target_df.column(max));
      ret.append_column(nan_count, count_na_df.column("count"));
      if (skip_na) ret.call_function(when({(~nan_count != ncol) >> ~max}));
      else ret.call_function(when({(~nan_count == 0) >> ~max}));
      ret.drop(max);
      ret.drop(nan_count);
      ret.rename("when", max);
    }
  }
  // use index as it is in input dataframe, if any. otherwise add index.
  if (with_index) {
    auto index_nm = df.columns()[0]; 
    use_dfcolumn use(df.raw_column(index_nm));
    ret.append_column(index_nm, df.column(index_nm))
       .change_col_position(index_nm, 0);
  } else {
    ret.prepend_rowid<long>("index");
  }
  return ret;
}

template <class T>
dftable frov_df_max_impl(exrpc_ptr_t& df_proxy,
                          std::vector<std::string>& cols,
                          std::vector<short>& types,
                          int& axis, bool& skip_na, 
                          bool& with_index) {
  dftable ret;
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  if (axis == 0) 
    ret = max_axis0_helper<T>(df, cols, types, skip_na);
  else if (axis == 1)
    ret = max_axis1_helper<T>(df, cols, types, skip_na, with_index);
  else REPORT_ERROR(USER_ERROR, "max: supported axis: 0 and 1 only!\n");
  return ret;
}

template <class T>
dummy_dftable 
frov_df_max(exrpc_ptr_t& df_proxy,
             std::vector<std::string>& cols,
             std::vector<short>& types,
             int& axis, bool& skip_na,
             bool& with_index) {
  auto ret = frov_df_max_impl<T>(df_proxy, cols, types, axis, skip_na, 
                                  with_index);
  auto retp = new dftable(std::move(ret));
  return to_dummy_dftable(retp);
}

template <class T>
dftable min_axis0_helper(dftable_base& df,         
                         std::vector<std::string>& cols,
                         std::vector<short>& types,
                         bool skip_na) {
  auto ncol = cols.size();
  auto nrow = df.num_row();
  checkAssumption (ncol == types.size());
  std::vector<T> min_res(ncol); auto resp = min_res.data();
  auto tmax = std::numeric_limits<T>::max();
  
  for(size_t i = 0; i < ncol; ++i) {
    auto col = cols[i];
    auto is_invalid = !skip_na && df.count(col) != nrow;
    if (is_invalid) resp[i] = tmax;
    else {
      switch(types[i]) {
        case BOOL:
        case INT:    resp[i] = df.min<int>(col);           break;
        case LONG:   resp[i] = df.min<long>(col);          break;
        case ULONG:  resp[i] = df.min<unsigned long>(col); break;
        case FLOAT:  resp[i] = df.min<float>(col);         break;
        case DOUBLE: resp[i] = df.min<double>(col);        break;
        default:     REPORT_ERROR(USER_ERROR, "min on non-numeric column!\n");
      }
    }
  }
  
  dftable ret;
  ret.append_column("index", make_dvector_scatter(cols));
  ret.append_column("min", make_dvector_scatter(min_res), true);
  return ret;
}

dftable
fillna_with_max(dftable& df, std::vector<std::string>& cols, 
                std::vector<short>& types);

template <class T>
dftable min_axis1_helper(dftable_base& df,         
                         std::vector<std::string>& cols,
                         std::vector<short>& types,
                         bool skip_na,
                         bool with_index) {
  dftable ret;
  auto ncol = cols.size();
  if (ncol == 0 ) {
    append_null<T>(ret, "min", df.num_row());
  } 
  else {
    auto target_df = df.select(cols);  // no index in selected columns
    dftable count_na_df;
    count_na_df = target_df.count_nulls(1);
    if (skip_na) target_df = fillna_with_max(target_df, cols, types);
    std::string min = "min";
    if (ncol == 1) {
      use_dfcolumn use(target_df.raw_column(cols[0]));
      ret.append_column(min, target_df.column(cols[0]));
    }
    else {
      auto func = when({(~cols[0] <= ~cols[1]) >> ~cols[0]}, ~cols[1])->as(min);
      target_df.call_function(func);
      for(size_t i = 2; i < ncol; ++i) {
        target_df.call_function(when({(~min <= ~cols[i]) >> ~min}, ~cols[i])); 
        target_df.drop(min);
        target_df.rename("when", min);
      }
      std::string nan_count = "nan_count";
      use_dfcolumn use(target_df.raw_column(min));
      ret.append_column(min, target_df.column(min));
      ret.append_column(nan_count, count_na_df.column("count"));
      if (skip_na) ret.call_function(when({(~nan_count != ncol) >> ~min})); 
      else ret.call_function(when({(~nan_count == 0) >> ~min})); 
      ret.drop(min);
      ret.drop(nan_count);
      ret.rename("when", min);
    }
  }
  // use index as it is in input dataframe, if any. otherwise add index.
  if (with_index) {
    auto index_nm = df.columns()[0]; 
    use_dfcolumn use(df.raw_column(index_nm));
    ret.append_column(index_nm, df.column(index_nm))
       .change_col_position(index_nm, 0);
  } else {
    ret.prepend_rowid<long>("index");
  }
  return ret;
}

template <class T>
dftable frov_df_min_impl(exrpc_ptr_t& df_proxy,
                          std::vector<std::string>& cols,
                          std::vector<short>& types,
                          int& axis, bool& skip_na, 
                          bool& with_index) {
  dftable ret;
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  if (axis == 0) 
    ret = min_axis0_helper<T>(df, cols, types, skip_na);
  else if (axis == 1)
    ret = min_axis1_helper<T>(df, cols, types, skip_na, with_index);
  else REPORT_ERROR(USER_ERROR, "min: supported axis: 0 and 1 only!\n");
  return ret;
}

template <class T>
dummy_dftable 
frov_df_min(exrpc_ptr_t& df_proxy,
             std::vector<std::string>& cols,
             std::vector<short>& types,
             int& axis, bool& skip_na,
             bool& with_index) {
  auto ret = frov_df_min_impl<T>(df_proxy, cols, types, axis, skip_na, 
                                  with_index);
  auto retp = new dftable(std::move(ret));
  return to_dummy_dftable(retp);
}

dummy_dftable
frov_df_first_element(exrpc_ptr_t&, std::string&, bool& skipna);
dummy_dftable
frov_df_last_element(exrpc_ptr_t&, std::string&, bool& skipna);

dummy_dftable 
frov_df_clip(exrpc_ptr_t& df_proxy,
            std::string& lower_limit_col,
            std::string& upper_limit_col,
            bool& with_index);

dummy_dftable 
frov_df_clip_axis1_numeric(exrpc_ptr_t& df_proxy,
                          std::vector<double>& lower_limit,
                          std::vector<double>& upper_limit,
                          bool& with_index);
dummy_dftable 
frov_df_clip_axis1_str(exrpc_ptr_t& df_proxy,
                      std::vector<std::string>& lower_limit,
                      std::vector<std::string>& upper_limit,
                      bool& with_index);

template <class T>
dummy_dftable
frov_df_sel_rows_by_val(exrpc_ptr_t& df_proxy,
                        std::string& target_col,
                        std::vector<T>& target_val) {
  auto& df = *reinterpret_cast<dftable_base*>(df_proxy);
  auto ret = df.select_rows_by_values(target_col, target_val);
  auto retp = new dftable(std::move(ret));
  return to_dummy_dftable(retp);
}

dummy_dftable
frov_df_sel_rows_by_indices(exrpc_ptr_t& df_proxy,
                            std::vector<int>& indices);

dummy_dftable frov_df_datetime_operation(exrpc_ptr_t& df_proxy,
                                        std::string& left_col,
                                        std::string& right_col,
                                        std::string& as_name,
                                        short& op_id,
                                        bool& with_index);

dummy_dftable frov_df_concat_columns(exrpc_ptr_t& df_proxy,
                                    std::string& sep,
                                    std::vector<std::string>& cols,
                                    std::string& as_name,
                                    bool& cast_as_datetime,
                                    std::string& fmt,
                                    bool& with_index);

void frovedis_set_datetime_type_for_add_sub_op(std::string& name);

#endif
