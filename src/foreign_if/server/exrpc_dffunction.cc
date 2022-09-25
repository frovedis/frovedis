#include "exrpc_dffunction.hpp"

template <>
void append_null<std::string>(dftable& df,
                              const std::string& cname,
                              size_t num_row) {
  // for string column it can only be NULL, so ignoring fill-value
  auto vec = vector_full<std::string>(num_row, "NULL");
  df.append_column(cname, make_dvector_scatter(vec), true);
}

void append_null_datetime(dftable& df,
                        const std::string& cname,
                        size_t num_row) {
  auto null_val = std::numeric_limits<datetime_t>::max();
  auto vec = vector_full<datetime_t>(num_row, null_val);
  df.append_datetime_column(cname, make_dvector_scatter(vec), true);
}

dftable& append_missing_column(dftable& df,
                               const std::string& cname,
                               const std::string& col_dtype,
                               size_t size,
                               std::string& fill_value,
                               std::string& fill_value_dtype) {
  if (fill_value == "NULL") { // append nulls as per colum type
    switch(get_numeric_dtype(col_dtype)) {
      case INT:    append_null<int>(df, cname, size); break;
      case LONG:   append_null<long>(df, cname, size); break;
      case ULONG:  append_null<unsigned long>(df, cname, size); break;
      case FLOAT:  append_null<float>(df, cname, size); break;
      case DOUBLE: append_null<double>(df, cname, size); break;
      case STRING: append_null<std::string>(df, cname, size); break;
      case DATETIME:   append_null_datetime(df, cname, size); break;
      default:     REPORT_ERROR(USER_ERROR, col_dtype +
                   ": unsupported type detected for appending nulls!\n");
    }
  } else { // append values based on fill_value_dtype
    switch(get_numeric_dtype(fill_value_dtype)) {
      case INT:    append_value<int>(df, cname, size, fill_value); break;
      case LONG:   append_value<long>(df, cname, size, fill_value); break;
      case ULONG:  append_value<unsigned long>(df, cname, size, fill_value); break;
      case FLOAT:  append_value<float>(df, cname, size, fill_value); break;
      case DOUBLE: append_value<double>(df, cname, size, fill_value); break;
      case STRING: append_value<std::string>(df, cname, size, fill_value); break;
      default:     REPORT_ERROR(USER_ERROR, fill_value_dtype +
                   ": unsupported type detected for appending values!\n");
    }
  }
  return df;
}

std::shared_ptr<dffunction>
get_function(const std::string& op_id, 
             const std::string& lcol,
             const std::string& rcol) {
  std::shared_ptr<dffunction> ret = NULL;
  if (op_id == "add")       ret = add_col(lcol,  rcol);
  else if (op_id == "sub")  ret = sub_col(lcol,  rcol);
  else if (op_id == "mul")  ret = mul_col(lcol,  rcol);
  else if (op_id == "idiv") ret = idiv_col(lcol, rcol);
  else if (op_id == "fdiv") ret = fdiv_col(lcol, rcol);
  else if (op_id == "mod")  ret = mod_col(lcol,  rcol);
  else if (op_id == "pow")  ret = pow_col(lcol,  rcol);
  else REPORT_ERROR(USER_ERROR, op_id + ": unsupported binary operator!\n");
  return ret;
}

// inplace operation
void treat_nan_as_null(dftable& df) {
  auto cols = df.columns();
  for (size_t i = 1; i < cols.size(); ++i) { // excluding index column: 0th column
    auto dfcol = df.column(cols[i]);
    use_dfcolumn use(dfcol);
    // 'contain_nulls' may already be 'true' due to presence of existing nulls, 
    // but in case it is 'false', setting it to 'true' when nan is detected 
    if (dfcol->dtype() == "int") {
      auto c1 = std::dynamic_pointer_cast<typed_dfcolumn<int>>(dfcol);
      auto nans = c1->val.map(treat_nan_as_null_helper<int>, c1->nulls);
      auto nansz = nans.template viewas_dvector<size_t>().size();
      if (nansz) c1->contain_nulls = true; 
    } else if (dfcol->dtype() == "unsigned int") {
      auto c1 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned int>>(dfcol);
      auto nans = c1->val.map(treat_nan_as_null_helper<unsigned int>, c1->nulls);
      auto nansz = nans.template viewas_dvector<size_t>().size();
      if (nansz) c1->contain_nulls = true; 
    } else if (dfcol->dtype() == "long") {
      auto c1 = std::dynamic_pointer_cast<typed_dfcolumn<long>>(dfcol);
      auto nans = c1->val.map(treat_nan_as_null_helper<long>, c1->nulls);
      auto nansz = nans.template viewas_dvector<size_t>().size();
      if (nansz) c1->contain_nulls = true; 
    } else if (dfcol->dtype() == "unsigned long") {
      auto c1 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned long>>(dfcol);
      auto nans = c1->val.map(treat_nan_as_null_helper<unsigned long>, c1->nulls);
      auto nansz = nans.template viewas_dvector<size_t>().size();
      if (nansz) c1->contain_nulls = true; 
    } else if (dfcol->dtype() == "float") {
      auto c1 = std::dynamic_pointer_cast<typed_dfcolumn<float>>(dfcol);
      auto nans = c1->val.map(treat_nan_as_null_helper<float>, c1->nulls);
      auto nansz = nans.template viewas_dvector<size_t>().size();
      if (nansz) c1->contain_nulls = true; 
    } else if (dfcol->dtype() == "double") {
      auto c1 = std::dynamic_pointer_cast<typed_dfcolumn<double>>(dfcol);
      auto nans = c1->val.map(treat_nan_as_null_helper<double>, c1->nulls);
      auto nansz = nans.template viewas_dvector<size_t>().size();
      if (nansz) c1->contain_nulls = true; 
    } else {
      REPORT_ERROR(USER_ERROR, 
      "treat_nan_as_null: unsupported column type: " + dfcol->dtype());
    }
  }
}

// assumes both df1 and df2 have same indices
dummy_dftable
frov_df_binary_operation(exrpc_ptr_t& df1,
                         exrpc_ptr_t& df2,
                         bool& is_series,
                         std::string& fill_value,
                         std::string& fill_value_dtype,
                         std::string& op_id,
                         bool& nan_is_null) {
  dftable left  = *get_dftable_pointer(df1); // copy
  dftable right = *get_dftable_pointer(df2); // copy
  auto lcol = left.columns();
  auto rcol = right.columns();
  auto lindex = lcol[0];
  auto rindex = rcol[0];

  // TODO: fix it by aligning index with missing values
  // index values might be scattered
  left  = left.sort(lindex).materialize();
  right = right.sort(rindex).materialize();
  std::vector<size_t> left_sizes;
  if(!check_distribution(left, right, left_sizes)) 
    right = right.align_as(left_sizes); 

  if(!verify_column_identicality(left, lindex, right, rindex))
    REPORT_ERROR(USER_ERROR, op_id +
    std::string(": can be performed on columns having identical index, ") +
    std::string("detected one or more missing indices or ") +
    std::string("index column of different types!\n"));

  // TODO: treating missing entries in input tables like in pandas 
  // in datframe library itself.
  //   if column in both tables and null is not in same location,
  //   then replace nulls with fill_value
  if (fill_value != "NULL") {
    auto has_index = true; // python wrapper has index
    left = fillna(left, fill_value, has_index);
    right = fillna(right, fill_value, has_index);
  }

  auto num_row = left.num_row();
  checkAssumption(num_row == right.num_row());

  dftable ret;
  ret.append_column(lindex, left.column(lindex));

  if (is_series) {
    checkAssumption(lcol.size() == 2 && rcol.size() == 2);
    auto func = get_function(op_id, lcol[1], rcol[1]);
    use_dfcolumn use(func->columns_to_use(left, right));
    ret.append_column(func->get_as(), func->execute(left, right));
  }
  else {
    // aligning columns in left table by appending missing columns with fill_value
    for (size_t i = 1; i < rcol.size(); ++i) {
      auto c = rcol[i];
      if (!is_present(lcol, c)) {
        append_missing_column(left, c, right.column(c)->dtype(), num_row, 
                              fill_value, fill_value_dtype);
      }
    }
    // aligning columns in right table by appending missing columns with fill_value
    for (size_t i = 1; i < lcol.size(); ++i) {
      auto c = lcol[i];
      if (!is_present(rcol, c)) {
        append_missing_column(right, c, left.column(c)->dtype(), num_row, 
                              fill_value, fill_value_dtype);
      }
    }
    // both tables have same columns -> ready for applying binary operation
    lcol = left.columns(); // right.columns() // both should be same now
    for (size_t i = 1; i < lcol.size(); ++i) {
      auto c = lcol[i];
      auto func = get_function(op_id, c, c);
      use_dfcolumn use(func->columns_to_use(left, right));
      ret.append_column(c, func->execute(left, right));
    }
  }
  if (nan_is_null) treat_nan_as_null(ret); // treating nan values as null (for pandas etc.)
  return to_dummy_dftable(new dftable(std::move(ret)));
}

dummy_dftable
frov_df_immed_binary_operation(exrpc_ptr_t& df,
                               std::string& value,
                               std::string& vtype,
                               std::string& op_id,
                               //std::string& fill_value,
                               bool& is_reversed,
                               bool& nan_is_null) {
  dftable_base& left  = *reinterpret_cast<dftable_base*>(df);
  /*
   * pandas seems not to support fill_value for immed case...
   *
  dftable left = *get_dftable_pointer(df);
  if (fill_value != "NULL") {
    auto has_index = true; // python wrapper has index
    left = fillna(left, fill_value, has_index);
  }
  */
  auto lcol = left.columns();
  auto lindex = lcol[0];

  dftable ret;
  ret.append_column(lindex, left.column(lindex));
  bool nan_check_req = nan_is_null;

  if (vtype == "int" || vtype == "int32") {
    auto right_val = do_cast<int>(value);
    for(size_t i = 1; i < lcol.size(); ++i) {
      auto func = get_immed_function<int>(op_id, lcol[i], right_val, is_reversed);
      use_dfcolumn use(func->columns_to_use(left));
      ret.append_column(lcol[i], func->execute(left));
    } 
  } else if (vtype == "long" || vtype == "int64") {
    auto right_val = do_cast<long>(value);
    for(size_t i = 1; i < lcol.size(); ++i) {
      auto func = get_immed_function<long>(op_id, lcol[i], right_val, is_reversed);
      use_dfcolumn use(func->columns_to_use(left));
      ret.append_column(lcol[i], func->execute(left));
    } 
  } else if (vtype == "timedelta") {
    auto right_val = do_cast<long>(value);
    auto mynat = std::numeric_limits<long>::min();
    if (right_val == mynat) {
      nan_check_req = false; // since only null would be generated in following loop
      for(size_t i = 1; i < lcol.size(); ++i) {
        auto func = null_column<datetime>();
        ret.append_column(lcol[i], func->execute(left));
      }
    } else {
      for(size_t i = 1; i < lcol.size(); ++i) {
        auto func = get_immed_function<long>(op_id, lcol[i], right_val, is_reversed);
        use_dfcolumn use(func->columns_to_use(left));
        ret.append_column(lcol[i], func->execute(left));
      }
    } 
  } else if (vtype == "float" || vtype == "float32") {
    for(size_t i = 1; i < lcol.size(); ++i) {
      // for python: [float (op) float] => "float"
      if (left.column(lcol[i])->dtype() == "float") {
        auto right_val = do_cast<float>(value);
        auto func = get_immed_function<float>(op_id, lcol[i], right_val, is_reversed);
        use_dfcolumn use(func->columns_to_use(left));
        ret.append_column(lcol[i], func->execute(left));
      } else { // for python: [any (op) float] => "double"
        auto right_val = do_cast<double>(value);
        auto func = get_immed_function<double>(op_id, lcol[i], right_val, is_reversed);
        use_dfcolumn use(func->columns_to_use(left));
        ret.append_column(lcol[i], func->execute(left));
      }
    }
  } else if (vtype == "double" || vtype == "float64") {
    if (value == "nan" && nan_is_null) { // any operation with nan would produce NULL result in pandas
      nan_check_req = false; // since only null would be generated in following loop
      for(size_t i = 1; i < lcol.size(); ++i) {
        auto func = null_column<double>();
        ret.append_column(lcol[i], func->execute(left));
      }
    } else {
      auto right_val = do_cast<double>(value);
      for(size_t i = 1; i < lcol.size(); ++i) {
        auto func = get_immed_function<double>(op_id, lcol[i], right_val, is_reversed);
        use_dfcolumn use(func->columns_to_use(left));
        ret.append_column(lcol[i], func->execute(left));
      }
    }  
  } else if (vtype == "timestamp") {
    auto right_val = do_cast<datetime_t>(value);
    for(size_t i = 1; i < lcol.size(); ++i) {
      auto func = get_immed_function<datetime_t>(op_id, lcol[i], right_val, is_reversed);
      use_dfcolumn use(func->columns_to_use(left));
      ret.append_column(lcol[i], func->execute(left));
    } 
  } else {
      REPORT_ERROR(USER_ERROR, vtype + 
      ": unsupported type is encountered as for immediate value!\n");
  }
  if (nan_check_req) treat_nan_as_null(ret); // treating nan values as null (for pandas etc.)
  return to_dummy_dftable(new dftable(std::move(ret)));
}

dummy_dftable frov_df_abs(exrpc_ptr_t& df) {
  dftable_base& left_base  = *reinterpret_cast<dftable_base*>(df);
  auto lcol = left_base.columns();
  auto lindex = lcol[0];

  dftable ret;
  ret.append_column(lindex, left_base.column(lindex));

  for(size_t i = 1; i < lcol.size(); ++i) {
    auto colname = lcol[i];
    auto col = left_base.column(colname);
    use_dfcolumn use(col);
    auto dtype = col->dtype();
    if (dtype != "string" && dtype != "dic_string") {
      ret.append_column(colname, abs_col(colname)->execute(left_base));
    } else {
      ret.append_column(colname, col);
    }
  }
  return to_dummy_dftable(new dftable(std::move(ret)));
}
