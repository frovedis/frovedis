#ifndef _EXRPC_DFFUNC_HPP_
#define _EXRPC_DFFUNC_HPP_

#include "exrpc_dataframe.hpp"

using namespace frovedis;

template <class T>
std::shared_ptr<dffunction>
get_immed_function(const std::string& op_id,
                   const std::string& lcol,
                   T val, bool is_reversed) {
  std::shared_ptr<dffunction> ret = NULL;
  if (is_reversed) {
    if (op_id == "add")       ret = add_im(val,  lcol);
    else if (op_id == "sub")  ret = sub_im(val,  lcol);
    else if (op_id == "mul")  ret = mul_im(val,  lcol); 
    else if (op_id == "idiv") ret = idiv_im(val, lcol);
    else if (op_id == "fdiv") ret = fdiv_im(val, lcol);
    else if (op_id == "mod")  ret = mod_im(val,  lcol);
    else if (op_id == "pow")  ret = pow_im(val,  lcol);
    else REPORT_ERROR(USER_ERROR, op_id + ": unsupported binary operator!\n");
  } else {
    if (op_id == "add")       ret = add_im(lcol,  val);
    else if (op_id == "sub")  ret = sub_im(lcol,  val);
    else if (op_id == "mul")  ret = mul_im(lcol,  val);
    else if (op_id == "idiv") ret = idiv_im(lcol, val);
    else if (op_id == "fdiv") ret = fdiv_im(lcol, val);
    else if (op_id == "mod")  ret = mod_im(lcol,  val);
    else if (op_id == "pow")  ret = pow_im(lcol,  val);
    else REPORT_ERROR(USER_ERROR, op_id + ": unsupported binary operator!\n");
  }
  return ret;
}

template <class T>
std::vector<size_t>
treat_nan_as_null_helper(std::vector<T>& val,
                         std::vector<size_t>& nulls) {
  auto nans = vector_find_nan(val);
  auto nansz = nans.size();
  if (nansz) {
    // replacing nan with nulls values
    auto vptr = val.data();
    auto nptr = nans.data();
    auto nullval = std::numeric_limits<T>::max();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < nansz; ++i) vptr[nptr[i]] = nullval;
    // updating nulls position vector
    nulls = set_union(nulls, nans);
  }
  return nans;
}

dummy_dftable
frov_df_binary_operation(exrpc_ptr_t& df1, exrpc_ptr_t& df2,
                         bool& is_series,  
                         std::string& fill_value,
                         std::string& fill_value_dtype, 
                         std::string& op_id, bool& nan_is_null);

dummy_dftable
frov_df_immed_binary_operation(exrpc_ptr_t& df,
                               std::string& value,
                               std::string& vtype,
                               std::string& op_id,
                               //std::string& fill_value,
                               bool& is_reversed, 
                               bool& nan_is_null);

dummy_dftable frov_df_abs(exrpc_ptr_t& df);

#endif
