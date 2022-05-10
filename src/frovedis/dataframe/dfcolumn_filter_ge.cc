#include "dfcolumn.hpp"

#include <limits>
#include <climits>
#include "../core/find_condition.hpp"

namespace frovedis {

template <class T, class U>
struct filter_ge_pred {
  int operator()(T a, U b) const {return a >= b;}
};

template <class T, class U>
std::vector<size_t> filter_ge_helper(std::vector<T>& left,
                                     std::vector<U>& right) {
  return find_condition_pair(left, right, filter_ge_pred<T,U>());
}

template <class T, class U>
struct filter_ge_immed_pred {
  filter_ge_immed_pred(U c) : c(c) {}
  int operator()(T a) const {return a >= c;}
  U c;
};

template <class T, class U>
std::vector<size_t> filter_ge_immed_helper(std::vector<T>& left,
                                           U right) {
  return find_condition(left, filter_ge_immed_pred<T,U>(right));
}

template <class T>
node_local<std::vector<size_t>>
typed_dfcolumn<T>::filter_ge(std::shared_ptr<dfcolumn>& right) {
  auto right_type = right->dtype();
  node_local<std::vector<size_t>> filtered_idx;
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_ge_helper<T,double>, right2->val);
    if(right2->contain_nulls)
      filtered_idx = filtered_idx.map(set_difference<size_t>, right2->nulls);
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_ge_helper<T,float>, right2->val);
    if(right2->contain_nulls)
      filtered_idx = filtered_idx.map(set_difference<size_t>, right2->nulls);
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_ge_helper<T,long>, right2->val);
    if(right2->contain_nulls)
      filtered_idx = filtered_idx.map(set_difference<size_t>, right2->nulls);
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_ge_helper<T,unsigned long>, right2->val);
    if(right2->contain_nulls)
      filtered_idx = filtered_idx.map(set_difference<size_t>, right2->nulls);
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_ge_helper<T,int>, right2->val);
    if(right2->contain_nulls)
      filtered_idx = filtered_idx.map(set_difference<size_t>, right2->nulls);
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_ge_helper<T,unsigned int>, right2->val);
    if(right2->contain_nulls)
      filtered_idx = filtered_idx.map(set_difference<size_t>, right2->nulls);
  } else throw std::runtime_error("unsupported type: " + right_type);
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

template <class T>
node_local<std::vector<size_t>>
typed_dfcolumn<T>::filter_ge_immed(std::shared_ptr<dfscalar>& right) {
  auto right_type = right->dtype();
  node_local<std::vector<size_t>> filtered_idx;
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_ge_immed_helper<T,double>,
                           broadcast(right2->val));
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_ge_immed_helper<T,float>,
                           broadcast(right2->val));
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_ge_immed_helper<T,long>,
                           broadcast(right2->val));
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_ge_immed_helper<T,unsigned long>,
                           broadcast(right2->val));
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_ge_immed_helper<T,int>,
                           broadcast(right2->val));
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_ge_immed_helper<T,unsigned int>,
                           broadcast(right2->val));
  } else throw std::runtime_error("unsupported type: " + right_type);
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

template node_local<std::vector<size_t>>
typed_dfcolumn<int>::filter_ge(std::shared_ptr<dfcolumn>& right);
template node_local<std::vector<size_t>>
typed_dfcolumn<int>::filter_ge_immed(std::shared_ptr<dfscalar>& right);

template node_local<std::vector<size_t>>
typed_dfcolumn<unsigned int>::filter_ge(std::shared_ptr<dfcolumn>& right);
template node_local<std::vector<size_t>>
typed_dfcolumn<unsigned int>::filter_ge_immed(std::shared_ptr<dfscalar>& right);

template node_local<std::vector<size_t>>
typed_dfcolumn<long>::filter_ge(std::shared_ptr<dfcolumn>& right);
template node_local<std::vector<size_t>>
typed_dfcolumn<long>::filter_ge_immed(std::shared_ptr<dfscalar>& right);

template node_local<std::vector<size_t>>
typed_dfcolumn<unsigned long>::filter_ge(std::shared_ptr<dfcolumn>& right);
template node_local<std::vector<size_t>>
typed_dfcolumn<unsigned long>::filter_ge_immed(std::shared_ptr<dfscalar>& right);

template node_local<std::vector<size_t>>
typed_dfcolumn<float>::filter_ge(std::shared_ptr<dfcolumn>& right);
template node_local<std::vector<size_t>>
typed_dfcolumn<float>::filter_ge_immed(std::shared_ptr<dfscalar>& right);

template node_local<std::vector<size_t>>
typed_dfcolumn<double>::filter_ge(std::shared_ptr<dfcolumn>& right);
template node_local<std::vector<size_t>>
typed_dfcolumn<double>::filter_ge_immed(std::shared_ptr<dfscalar>& right);

// ----- dic_string -----
node_local<std::vector<size_t>>
typed_dfcolumn<dic_string>::filter_ge(std::shared_ptr<dfcolumn>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<dic_string>>(right);
  if(!static_cast<bool>(right2))
    throw std::runtime_error("filter_ge: column types are different");
  node_local<std::vector<size_t>> leftval, rightval;
  compare_prepare(right2, leftval, rightval);
  auto filtered_idx = leftval.map(filter_ge_helper<size_t, size_t>, rightval);
  if(right2->contain_nulls)
    filtered_idx = filtered_idx.map(set_difference<size_t>, right2->nulls);
  if(contain_nulls)
    filtered_idx = filtered_idx.map(set_difference<size_t>, nulls);
  return filtered_idx;
}

node_local<std::vector<size_t>>
typed_dfcolumn<dic_string>::filter_ge_immed(std::shared_ptr<dfscalar>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfscalar<std::string>>(right);
  if(!static_cast<bool>(right2))
    throw std::runtime_error("filter_ge_immed: column types are different");
  node_local<std::vector<size_t>> leftval;
  size_t rightval;
  compare_prepare_immed(right2->val, leftval, rightval);
  auto filtered_idx = leftval.map(filter_ge_immed_helper<size_t, size_t>,
                                  broadcast(rightval));
  if(contain_nulls)
    filtered_idx = filtered_idx.map(set_difference<size_t>, nulls);
  return filtered_idx;
}

// ----- datetime -----
node_local<std::vector<size_t>>
typed_dfcolumn<datetime>::filter_ge(std::shared_ptr<dfcolumn>& right) {
  std::shared_ptr<typed_dfcolumn<datetime>> right2 = NULL;
  auto dt = right->dtype();
  if(dt == "datetime") {
    right2 = std::dynamic_pointer_cast<typed_dfcolumn<datetime>>(right);
  } else if(dt == "string" || dt == "dic_string" || dt == "raw_string") {
    auto tmp = right->type_cast("datetime"); // using default format: %Y-%m-%d
    right2 = std::dynamic_pointer_cast<typed_dfcolumn<datetime>>(tmp);
  }
  else throw std::runtime_error("datetime >= " + dt + ": invalid operation!");
  auto filtered_idx = val.map(filter_ge_helper<datetime_t, datetime_t>,
                              right2->val);
  if(right2->contain_nulls)
    filtered_idx = filtered_idx.map(set_difference<size_t>, right2->nulls);
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

node_local<std::vector<size_t>>
typed_dfcolumn<datetime>::filter_ge_immed(std::shared_ptr<dfscalar>& right) {
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
      throw std::runtime_error("datetime >= " + dt + ": invalid operation!");
    right2 = std::make_shared<typed_dfscalar<datetime>>(tmp->val);
  }
  auto filtered_idx = val.map(filter_ge_immed_helper<datetime_t, datetime_t>,
                              broadcast(right2->val));
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

}
