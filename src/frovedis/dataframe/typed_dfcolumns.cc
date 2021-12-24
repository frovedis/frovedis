#include "typed_dfcolumn_int.cc"
#include "typed_dfcolumn_uint.cc"
#include "typed_dfcolumn_long.cc"
#include "typed_dfcolumn_ulong.cc"
#include "typed_dfcolumn_float.cc"
#include "typed_dfcolumn_double.cc"
#include "typed_dfcolumn_string.cc"
#include "typed_dfcolumn_raw_string.cc"
#include "typed_dfcolumn_dic_string.cc"
#include "typed_dfcolumn_datetime.cc"

// below is used in dffunction_when
// moved here in order to gather typed_dfcolumn<T> instantiation
#include "dfutil.hpp"

namespace frovedis {

template <class T, class U>
std::string merge_type_helper() {
  T a;
  U b;
  typedef decltype(a+b) V;
  return get_dftype_name<V>();
}

std::string merge_type(const std::string& left, const std::string& right) {
  if(left == "double") {
    if(right == "double") return merge_type_helper<double,double>();
    else if(right == "float") return merge_type_helper<double,float>();
    else if(right == "long") return merge_type_helper<double,long>();
    else if(right == "unsigned long")
      return merge_type_helper<double,unsigned long>();
    else if(right == "int") return merge_type_helper<double,int>();
    else if(right == "unsigned int")
      return merge_type_helper<double,unsigned int>();
  } else if(left == "float") {
    if(right == "double") return merge_type_helper<float,double>();
    else if(right == "float") return merge_type_helper<float,float>();
    else if(right == "long") return merge_type_helper<float,long>();
    else if(right == "unsigned long")
      return merge_type_helper<float,unsigned long>();
    else if(right == "int") return merge_type_helper<float,int>();
    else if(right == "unsigned int")
      return merge_type_helper<float,unsigned int>();
  } else if(left == "long") {
    if(right == "double") return merge_type_helper<long,double>();
    else if(right == "float") return merge_type_helper<long,float>();
    else if(right == "long") return merge_type_helper<long,long>();
    else if(right == "unsigned long")
      return merge_type_helper<long,unsigned long>();
    else if(right == "int") return merge_type_helper<long,int>();
    else if(right == "unsigned int")
      return merge_type_helper<long,unsigned int>();
  } else if(left == "unsigned long") {
    if(right == "double") return merge_type_helper<unsigned long,double>();
    else if(right == "float") return merge_type_helper<unsigned long,float>();
    else if(right == "long") return merge_type_helper<unsigned long,long>();
    else if(right == "unsigned long")
      return merge_type_helper<unsigned long,unsigned long>();
    else if(right == "int") return merge_type_helper<unsigned long,int>();
    else if(right == "unsigned int")
      return merge_type_helper<unsigned long,unsigned int>();
  } else if(left == "int") {
    if(right == "double") return merge_type_helper<int,double>();
    else if(right == "float") return merge_type_helper<int,float>();
    else if(right == "long") return merge_type_helper<int,long>();
    else if(right == "unsigned long")
      return merge_type_helper<int,unsigned long>();
    else if(right == "int") return merge_type_helper<int,int>();
    else if(right == "unsigned int")
      return merge_type_helper<int,unsigned int>();
  } else if(left == "unsigned int") {
    if(right == "double") return merge_type_helper<unsigned int,double>();
    else if(right == "float") return merge_type_helper<unsigned int,float>();
    else if(right == "long") return merge_type_helper<unsigned int,long>();
    else if(right == "unsigned long")
      return merge_type_helper<unsigned int,unsigned long>();
    else if(right == "int") return merge_type_helper<unsigned int,int>();
    else if(right == "unsigned int")
      return merge_type_helper<unsigned int,unsigned int>();
  } else if(left == right) {
    return left;
  } else {
    throw std::runtime_error("unsupported merge type: " + left + ", " + right);
  }
  // to avoid warning
  throw std::runtime_error("unsupported merge type: " + left + ", " + right);
}

std::string merge_type(const std::vector<std::shared_ptr<dfcolumn>>& columns) {
  auto columns_size = columns.size();
  size_t i = 0;
  std::string type = "int"; // if all columns are all null, type is int
  for(; i < columns_size; i++) {
    if(!columns[i]->is_all_null()) {
      type = columns[i]->dtype();
      break;
    }
  }
  for(; i < columns_size; i++) {
    if(!columns[i]->is_all_null())
      type = merge_type(type, columns[i]->dtype());
  }
  return type;
}

template <class T>
std::shared_ptr<dfcolumn>
merge_column_helper(std::vector<std::shared_ptr<dfcolumn>>& columns,
                    std::vector<node_local<std::vector<size_t>>>& idx,
                    node_local<std::vector<size_t>>& table_idx,
                    node_local<std::vector<size_t>>& null_idx,
                    bool use_null_idx) {
  auto retval = table_idx.map(+[](const std::vector<size_t>& table_idx) {
      return std::vector<T>(table_idx.size());
    });
  auto retnull = make_node_local_allocate<std::vector<size_t>>();
  auto conv_table = table_idx.map(+[](const std::vector<size_t>& table_idx) {
      auto table_idx_size = table_idx.size();
      if(table_idx_size == 0) return std::vector<size_t>();
      auto max_size = table_idx[table_idx_size - 1];
      std::vector<size_t> ret(max_size+1);
      auto retp = ret.data();
      auto table_idxp = table_idx.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
      for(size_t i = 0; i < table_idx_size; i++) {
        retp[table_idxp[i]] = i;
      }
      return ret;
    });
  for(size_t i = 0; i < columns.size(); i++) {
    std::shared_ptr<typed_dfcolumn<T>> tcol;
    if(get_dftype_name<T>() == columns[i]->dtype()) {
      tcol = std::dynamic_pointer_cast<typed_dfcolumn<T>>(columns[i]);
    } else if(columns[i]->is_all_null()) {
      tcol = std::dynamic_pointer_cast<typed_dfcolumn<T>>
        (create_null_column<T>(columns[i]->sizes()));
    } else {
      auto ccol = columns[i]->type_cast(get_dftype_name<T>());
      tcol = std::dynamic_pointer_cast<typed_dfcolumn<T>>(ccol);
    }
    if(!tcol) throw std::runtime_error("merge_column: internal type error");
    retval.mapv
      (+[](std::vector<T>& retval,
           const std::vector<T>& colval,
           std::vector<size_t>& retnull,
           const std::vector<size_t>& colnull,
           const std::vector<size_t>& idx,
           const std::vector<size_t>& conv_table) {
        auto retvalp = retval.data();
        auto colvalp = colval.data();
        auto colval_size = colval.size();
        auto idxp = idx.data();
        auto conv_tablep = conv_table.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
        for(size_t i = 0; i < colval_size; i++) {
          retvalp[conv_tablep[idxp[i]]] = colvalp[i];
        }
        auto colnull_size = colnull.size();
        if(colnull_size > 0) {
          auto retnullp = retnull.data();
          auto colnullp = colnull.data();
          auto crnt_retnull_size = retnull.size();
          auto new_retnull_size = crnt_retnull_size + colnull_size;
          std::vector<size_t> new_retnull(new_retnull_size);
          auto new_retnullp = new_retnull.data();
          for(size_t i = 0; i < crnt_retnull_size; i++) {
            new_retnullp[i] = retnullp[i];
          }
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
          for(size_t i = 0; i < colnull_size; i++) {
            new_retnullp[crnt_retnull_size + i] =
              conv_tablep[idxp[colnullp[i]]];
          }
          retnull.swap(new_retnull);
          // sort will be done afterwards
        }
      }, tcol->val, retnull, tcol->nulls, idx[i], conv_table);
  }
  if(use_null_idx) {
    null_idx.mapv
      (+[](const std::vector<size_t>& null_idx,
           std::vector<size_t>& retnull,
           std::vector<T>& retval,
           const std::vector<size_t>& conv_table) {
        auto null_idx_size = null_idx.size();
        auto null_idxp = null_idx.data();
        auto conv_tablep = conv_table.data();
        auto retnullp = retnull.data();
        auto crnt_retnull_size = retnull.size();
        auto new_retnull_size = crnt_retnull_size + null_idx_size;
        std::vector<size_t> new_retnull(new_retnull_size);
        auto new_retnullp = new_retnull.data();
        for(size_t i = 0; i < crnt_retnull_size; i++) {
          new_retnullp[i] = retnullp[i];
        }
        auto retvalp = retval.data();
        auto max = std::numeric_limits<T>::max();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
        for(size_t i = 0; i < null_idx_size; i++) {
          auto pos = conv_tablep[null_idxp[i]];
          new_retnullp[crnt_retnull_size + i] = pos;
          retvalp[pos] = max;
        }
        radix_sort(new_retnull);
        retnull.swap(new_retnull);
      }, retnull, retval, conv_table);
    return std::make_shared<typed_dfcolumn<T>>(std::move(retval),
                                               std::move(retnull));
  } else {
    retnull.mapv(+[](std::vector<size_t>& nulls){radix_sort(nulls);});
    return std::make_shared<typed_dfcolumn<T>>(std::move(retval),
                                               std::move(retnull));
  }
}

std::shared_ptr<dfcolumn>
merge_column_datetime_helper(std::vector<std::shared_ptr<dfcolumn>>& columns,
                             std::vector<node_local<std::vector<size_t>>>& idx,
                             node_local<std::vector<size_t>>& table_idx,
                             node_local<std::vector<size_t>>& null_idx,
                             bool use_null_idx) {
  auto ret = merge_column_helper<datetime_t>(columns, idx, table_idx, null_idx,
                                             use_null_idx);
  auto ret2 = std::dynamic_pointer_cast<typed_dfcolumn<datetime_t>>(ret);
  return std::make_shared<typed_dfcolumn<datetime>>
    (std::move(ret2->val), std::move(ret2->nulls));
}

std::shared_ptr<dfcolumn>
merge_column_dic_string_helper
(std::vector<std::shared_ptr<dfcolumn>>& columns,
 std::vector<node_local<std::vector<size_t>>>& idx,
 node_local<std::vector<size_t>>& table_idx,
 node_local<std::vector<size_t>>& null_idx,
 bool use_null_idx) {
  auto columns_size = columns.size();
  std::vector<std::shared_ptr<typed_dfcolumn<dic_string>>>
    columns2(columns_size);
  std::vector<dict*> dic_colsp(columns_size);
  for(size_t i = 0; i < columns_size; i++) {
    if(columns[i]->is_all_null()) {
      columns2[i] = std::dynamic_pointer_cast<typed_dfcolumn<dic_string>>
        (create_null_column<dic_string>(columns[i]->sizes()));
    } else {
      columns2[i] =
        dynamic_pointer_cast<typed_dfcolumn<dic_string>>(columns[i]);
    }
    if(!columns2[i])
      throw std::runtime_error
        ("merge_column_dic_string_helper: internal dynamic_pointer_cast error");
    dic_colsp[i] = &(*columns2[i]->dic);
  }
  auto newdic =
    std::make_shared<dict>(union_columns_dic_string_create_dic(dic_colsp));

  std::vector<std::shared_ptr<dfcolumn>> tmp_columns(columns_size);
  for(size_t i = 0; i < columns_size; i++) {
    auto newval = union_columns_dic_string_prepare
      (*newdic, *columns2[i]->dic, columns2[i]->val);
    tmp_columns[i] = std::make_shared<typed_dfcolumn<size_t>>
      (std::move(newval), std::move(columns2[i]->nulls));
  }
  auto tmpret_column = merge_column_helper<size_t>(tmp_columns, idx, table_idx,
                                                   null_idx, use_null_idx);
  auto tmpret_column2 =
    dynamic_pointer_cast<typed_dfcolumn<size_t>>(tmpret_column);
  return std::make_shared<typed_dfcolumn<dic_string>>
    (std::move(newdic), std::move(tmpret_column2->val),
     std::move(tmpret_column2->nulls));
}

std::shared_ptr<dfcolumn>
merge_column(std::vector<std::shared_ptr<dfcolumn>>& columns,
             std::vector<node_local<std::vector<size_t>>>& idx,
             node_local<std::vector<size_t>>& table_idx,
             node_local<std::vector<size_t>>& null_idx,
             bool use_null_idx) {
  auto merged_type = merge_type(columns);
  if(merged_type == "double") {
    return merge_column_helper<double>(columns, idx, table_idx, null_idx,
                                       use_null_idx);
  } else if (merged_type == "float") {
    return merge_column_helper<float>(columns, idx, table_idx, null_idx,
                                      use_null_idx);
  } else if(merged_type == "long") {
    return merge_column_helper<long>(columns, idx, table_idx, null_idx,
                                     use_null_idx);
  } else if(merged_type == "unsigned long") {
    return
      merge_column_helper<unsigned long>(columns, idx, table_idx, null_idx,
                                         use_null_idx);
  } else if(merged_type == "int") {
    return merge_column_helper<int>(columns, idx, table_idx, null_idx,
                                    use_null_idx);
  } else if(merged_type == "unsigned int") {
    return merge_column_helper<unsigned int>(columns, idx, table_idx, null_idx,
                                             use_null_idx);
  } else if(merged_type == "dic_string") {
    return merge_column_dic_string_helper(columns, idx, table_idx, null_idx,
                                          use_null_idx);
  } else if(merged_type == "datetime") {
    return merge_column_datetime_helper(columns, idx, table_idx, null_idx,
                                        use_null_idx);
  } else throw std::runtime_error("dffunction_when: incompatible types");
  // to avoid warning
  throw std::runtime_error("dffunction_when: incompatible types");
}

}
