#include "dfcolumn.hpp"

#include <limits>
#include <climits>
#include "../core/find_condition.hpp"

using namespace std;

namespace frovedis {

template <class T, class U>
struct filter_lt_pred {
  int operator()(T a, U b) const {return a < b;}
};

template <class T, class U>
std::vector<size_t> filter_lt_helper(std::vector<T>& left,
                                     std::vector<U>& right) {
  return find_condition_pair(left, right, filter_lt_pred<T,U>());
}

template <class T, class U>
struct filter_lt_immed_pred {
  filter_lt_immed_pred(U c) : c(c) {}
  int operator()(T a) const {return a < c;}
  U c;
};

template <class T, class U>
std::vector<size_t> filter_lt_immed_helper(std::vector<T>& left,
                                           U right) {
  return find_condition(left, filter_lt_immed_pred<T,U>(right));
}

template <class T>
node_local<std::vector<size_t>>
typed_dfcolumn<T>::filter_lt(std::shared_ptr<dfcolumn>& right) {
  auto right_type = right->dtype();
  node_local<std::vector<size_t>> filtered_idx;
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_lt_helper<T,double>, right2->val);
    if(right2->contain_nulls)
      filtered_idx = filtered_idx.map(set_difference<size_t>, right2->nulls);
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_lt_helper<T,float>, right2->val);
    if(right2->contain_nulls)
      filtered_idx = filtered_idx.map(set_difference<size_t>, right2->nulls);
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_lt_helper<T,long>, right2->val);
    if(right2->contain_nulls)
      filtered_idx = filtered_idx.map(set_difference<size_t>, right2->nulls);
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_lt_helper<T,unsigned long>, right2->val);
    if(right2->contain_nulls)
      filtered_idx = filtered_idx.map(set_difference<size_t>, right2->nulls);
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_lt_helper<T,int>, right2->val);
    if(right2->contain_nulls)
      filtered_idx = filtered_idx.map(set_difference<size_t>, right2->nulls);
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_lt_helper<T,unsigned int>, right2->val);
    if(right2->contain_nulls)
      filtered_idx = filtered_idx.map(set_difference<size_t>, right2->nulls);
  } else throw std::runtime_error("unsupported type: " + right_type);
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

template <class T>
node_local<std::vector<size_t>>
typed_dfcolumn<T>::filter_lt_immed(std::shared_ptr<dfscalar>& right) {
  auto right_type = right->dtype();
  node_local<std::vector<size_t>> filtered_idx;
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_lt_immed_helper<T,double>,
                           broadcast(right2->val));
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_lt_immed_helper<T,float>,
                           broadcast(right2->val));
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_lt_immed_helper<T,long>,
                           broadcast(right2->val));
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_lt_immed_helper<T,unsigned long>,
                           broadcast(right2->val));
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_lt_immed_helper<T,int>,
                           broadcast(right2->val));
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_lt_immed_helper<T,unsigned int>,
                           broadcast(right2->val));
  } else throw std::runtime_error("unsupported type: " + right_type);
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

template node_local<std::vector<size_t>>
typed_dfcolumn<int>::filter_lt(std::shared_ptr<dfcolumn>& right);
template node_local<std::vector<size_t>>
typed_dfcolumn<int>::filter_lt_immed(std::shared_ptr<dfscalar>& right);

template node_local<std::vector<size_t>>
typed_dfcolumn<unsigned int>::filter_lt(std::shared_ptr<dfcolumn>& right);
template node_local<std::vector<size_t>>
typed_dfcolumn<unsigned int>::filter_lt_immed(std::shared_ptr<dfscalar>& right);

template node_local<std::vector<size_t>>
typed_dfcolumn<long>::filter_lt(std::shared_ptr<dfcolumn>& right);
template node_local<std::vector<size_t>>
typed_dfcolumn<long>::filter_lt_immed(std::shared_ptr<dfscalar>& right);

template node_local<std::vector<size_t>>
typed_dfcolumn<unsigned long>::filter_lt(std::shared_ptr<dfcolumn>& right);
template node_local<std::vector<size_t>>
typed_dfcolumn<unsigned long>::filter_lt_immed(std::shared_ptr<dfscalar>& right);

template node_local<std::vector<size_t>>
typed_dfcolumn<float>::filter_lt(std::shared_ptr<dfcolumn>& right);
template node_local<std::vector<size_t>>
typed_dfcolumn<float>::filter_lt_immed(std::shared_ptr<dfscalar>& right);

template node_local<std::vector<size_t>>
typed_dfcolumn<double>::filter_lt(std::shared_ptr<dfcolumn>& right);
template node_local<std::vector<size_t>>
typed_dfcolumn<double>::filter_lt_immed(std::shared_ptr<dfscalar>& right);

// ----- dic_string -----
std::vector<size_t>
dic_string_compare_prepare_helper(const std::vector<size_t>& val,
                                  const std::vector<size_t>& trans,
                                  const std::vector<size_t>& sort_order) {
  auto valp = val.data();
  auto transp = trans.data();
  auto sort_orderp = sort_order.data();
  auto size = val.size();
  std::vector<size_t> ret(size);
  auto retp = ret.data();
  auto max = std::numeric_limits<size_t>::max();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < size; i++) {
    if(valp[i] == max) retp[i] = max;
    else retp[i] = sort_orderp[transp[valp[i]]];
  }
  return ret;
}

std::vector<size_t>
dic_string_compare_prepare_helper2(const std::vector<size_t>& val,
                                   const std::vector<size_t>& sort_order) {
  auto valp = val.data();
  auto sort_orderp = sort_order.data();
  auto size = val.size();
  std::vector<size_t> ret(size);
  auto retp = ret.data();
  auto max = std::numeric_limits<size_t>::max();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < size; i++) {
    if(valp[i] == max) retp[i] = max;
    else retp[i] = sort_orderp[valp[i]];
  }
  return ret;
}

void
typed_dfcolumn<dic_string>::
compare_prepare(shared_ptr<typed_dfcolumn<dic_string>>& right,
                node_local<std::vector<size_t>>& leftval,
                node_local<std::vector<size_t>>& rightval) {
  if(dic == right->dic) {
    auto& new_dic = *dic;
    auto new_num_words = new_dic.num_words();
    std::vector<size_t> new_order(new_num_words);
    auto new_orderp = new_order.data();
    for(size_t i = 0; i < new_num_words; i++) new_orderp[i] = i;
    lexical_sort_compressed_words(new_dic.cwords, new_dic.lens,
                                  new_dic.lens_num, new_order);
    auto bnew_order = broadcast(new_order);
    leftval = val.map(dic_string_compare_prepare_helper2, bnew_order);
    rightval = right->val.map(dic_string_compare_prepare_helper2, bnew_order);
  } else {
    auto& left_dic = *dic;
    auto& right_dic = *(right->dic);
    auto new_dic = merge_dict(left_dic, right_dic);

    compressed_words to_lookup_left;
    auto left_num_words = left_dic.num_words();
    to_lookup_left.cwords.swap(left_dic.cwords);
    to_lookup_left.lens.swap(left_dic.lens);
    to_lookup_left.lens_num.swap(left_dic.lens_num);
    to_lookup_left.order.resize(left_num_words);
    auto left_orderp = to_lookup_left.order.data();
    for(size_t i = 0; i < left_num_words; i++) left_orderp[i] = i;
    auto left_trans_table = new_dic.lookup(to_lookup_left);
    to_lookup_left.cwords.swap(left_dic.cwords);
    to_lookup_left.lens.swap(left_dic.lens);
    to_lookup_left.lens_num.swap(left_dic.lens_num);

    compressed_words to_lookup_right;
    auto right_num_words = right_dic.num_words();
    to_lookup_right.cwords.swap(right_dic.cwords);
    to_lookup_right.lens.swap(right_dic.lens);
    to_lookup_right.lens_num.swap(right_dic.lens_num);
    to_lookup_right.order.resize(right_num_words);
    auto right_orderp = to_lookup_right.order.data();
    for(size_t i = 0; i < right_num_words; i++) right_orderp[i] = i;
    auto right_trans_table = new_dic.lookup(to_lookup_right);
    to_lookup_right.cwords.swap(right_dic.cwords);
    to_lookup_right.lens.swap(right_dic.lens);
    to_lookup_right.lens_num.swap(right_dic.lens_num);

    auto new_num_words = new_dic.num_words();
    std::vector<size_t> new_order(new_num_words);
    auto new_orderp = new_order.data();
    for(size_t i = 0; i < new_num_words; i++) new_orderp[i] = i;
    lexical_sort_compressed_words(new_dic.cwords, new_dic.lens,
                                  new_dic.lens_num, new_order);
    auto bleft_trans_table = broadcast(left_trans_table);
    auto bright_trans_table = broadcast(right_trans_table);
    auto bnew_order = broadcast(new_order);

    leftval = val.map(dic_string_compare_prepare_helper,
                      bleft_trans_table, bnew_order);
    rightval = right->val.map(dic_string_compare_prepare_helper,
                              bright_trans_table, bnew_order);
  }
}

node_local<std::vector<size_t>>
typed_dfcolumn<dic_string>::filter_lt(std::shared_ptr<dfcolumn>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<dic_string>>(right);
  if(!static_cast<bool>(right2))
    throw std::runtime_error("filter_lt: column types are different");
  node_local<std::vector<size_t>> leftval, rightval;
  compare_prepare(right2, leftval, rightval);
  auto filtered_idx = leftval.map(filter_lt_helper<size_t, size_t>, rightval);
  if(right2->contain_nulls)
    filtered_idx = filtered_idx.map(set_difference<size_t>, right2->nulls);
  if(contain_nulls)
    filtered_idx = filtered_idx.map(set_difference<size_t>, nulls);
  return filtered_idx;
}

void
typed_dfcolumn<dic_string>::
compare_prepare_immed(const std::string& right,
                      node_local<std::vector<size_t>>& leftval,
                      size_t& rightval) {
  auto& left_dic = *dic;
  auto cws = make_compressed_words(vector_string_to_words({right}));
  auto idx = left_dic.lookup(cws);
  auto NOT_FOUND = numeric_limits<size_t>::max();
  if(idx[0] == NOT_FOUND) {
    // TODO: inefficient because original dict is not utilized
    // modify lookup to support lower_bound/upper_bound?
    auto right_dic = make_dict(cws);
    auto new_dic = merge_dict(left_dic, right_dic);
    compressed_words to_lookup_left;
    auto left_num_words = left_dic.num_words();
    to_lookup_left.cwords.swap(left_dic.cwords);
    to_lookup_left.lens.swap(left_dic.lens);
    to_lookup_left.lens_num.swap(left_dic.lens_num);
    to_lookup_left.order.resize(left_num_words);
    auto left_orderp = to_lookup_left.order.data();
    for(size_t i = 0; i < left_num_words; i++) left_orderp[i] = i;
    auto left_trans_table = new_dic.lookup(to_lookup_left);
    to_lookup_left.cwords.swap(left_dic.cwords);
    to_lookup_left.lens.swap(left_dic.lens);
    to_lookup_left.lens_num.swap(left_dic.lens_num);

    compressed_words& to_lookup_right = cws;
    auto right_trans_table = new_dic.lookup(to_lookup_right);

    auto new_num_words = new_dic.num_words();
    std::vector<size_t> new_order(new_num_words);
    auto new_orderp = new_order.data();
    for(size_t i = 0; i < new_num_words; i++) new_orderp[i] = i;
    lexical_sort_compressed_words(new_dic.cwords, new_dic.lens,
                                  new_dic.lens_num, new_order);
    auto bleft_trans_table = broadcast(left_trans_table);
    auto bnew_order = broadcast(new_order);
    leftval = val.map(dic_string_compare_prepare_helper,
                      bleft_trans_table, bnew_order);
    rightval = new_order[right_trans_table[0]];
  } else {
    auto& new_dic = left_dic;
    auto new_num_words = new_dic.num_words();
    std::vector<size_t> new_order(new_num_words);
    auto new_orderp = new_order.data();
    for(size_t i = 0; i < new_num_words; i++) new_orderp[i] = i;
    lexical_sort_compressed_words(new_dic.cwords, new_dic.lens,
                                  new_dic.lens_num, new_order);
    auto bnew_order = broadcast(new_order);
    leftval = val.map(dic_string_compare_prepare_helper2, bnew_order);
    rightval = new_order[idx[0]];
  }
}

node_local<std::vector<size_t>>
typed_dfcolumn<dic_string>::filter_lt_immed(std::shared_ptr<dfscalar>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfscalar<std::string>>(right);
  if(!static_cast<bool>(right2))
    throw std::runtime_error("filter_lt_immed: column types are different");
  node_local<std::vector<size_t>> leftval;
  size_t rightval;
  compare_prepare_immed(right2->val, leftval, rightval);
  auto filtered_idx = leftval.map(filter_lt_immed_helper<size_t, size_t>,
                                  broadcast(rightval));
  if(contain_nulls)
    filtered_idx = filtered_idx.map(set_difference<size_t>, nulls);
  return filtered_idx;
}

// ----- datetime -----
node_local<std::vector<size_t>>
typed_dfcolumn<datetime>::filter_lt(std::shared_ptr<dfcolumn>& right) {
  std::shared_ptr<typed_dfcolumn<datetime>> right2 = NULL;
  auto dt = right->dtype();
  if(dt == "datetime") {
    right2 = std::dynamic_pointer_cast<typed_dfcolumn<datetime>>(right);
  } else if(dt == "string" || dt == "dic_string" || dt == "raw_string") {
    auto tmp = right->type_cast("datetime"); // using default format: %Y-%m-%d
    right2 = std::dynamic_pointer_cast<typed_dfcolumn<datetime>>(tmp);
  }
  else throw std::runtime_error("datetime < " + dt + ": invalid operation!");
  auto filtered_idx = val.map(filter_lt_helper<datetime_t, datetime_t>,
                              right2->val);
  if(right2->contain_nulls)
    filtered_idx = filtered_idx.map(set_difference<size_t>, right2->nulls);
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

node_local<std::vector<size_t>>
typed_dfcolumn<datetime>::filter_lt_immed(std::shared_ptr<dfscalar>& right) {
  std::shared_ptr<typed_dfscalar<datetime>> right2 = NULL;
  auto dt = right->dtype();
  if(dt == "datetime") {
    right2 = std::dynamic_pointer_cast<typed_dfscalar<datetime>>(right);
  } else if(dt == "string") {
    auto tmp = right->type_cast("datetime"); // using default format: %Y-%m-%d
    right2 = std::dynamic_pointer_cast<typed_dfscalar<datetime>>(tmp);
  }
  else { // must be of datetime_t type
    auto tmp = std::dynamic_pointer_cast<typed_dfscalar<datetime_t>>(right);
    if(!static_cast<bool>(tmp)) 
      throw std::runtime_error("datetime < " + dt + ": invalid operation!");
    right2 = std::make_shared<typed_dfscalar<datetime>>(tmp->val);
  }
  auto filtered_idx = val.map(filter_lt_immed_helper<datetime_t, datetime_t>,
                              broadcast(right2->val));
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

}
