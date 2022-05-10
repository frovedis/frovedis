#include "dfcolumn.hpp"

#include <limits>
#include <climits>

#include "dfcolumn_helper.hpp"

using namespace std;

namespace frovedis {

template <class T>
template <class U>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::typed_eq(const std::shared_ptr<typed_dfcolumn<U>>& right) {
  auto newval =
    val.map(+[](const std::vector<T>& left, 
                const std::vector<U>& right) {
              auto leftp = left.data();
              auto rightp = right.data();
              auto size = left.size();
              std::vector<int> ret(size);
              auto retp = ret.data();
              for(size_t i = 0; i < size; i++)
                retp[i] = (leftp[i] == rightp[i]);
              return ret;
      }, right->val);
  if(contain_nulls || right->contain_nulls) {
    auto newnulls = nulls.map(set_union<size_t>, right->nulls);
    newval.mapv(reset_null<int>, newnulls);
    return std::make_shared<typed_dfcolumn<int>>
      (std::move(newval), std::move(newnulls));
  } else {
    auto dvval = newval.template moveto_dvector<int>();
    return std::make_shared<typed_dfcolumn<int>>(std::move(dvval));
  }
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::eq(const std::shared_ptr<dfcolumn>& right) {
  auto right_type = right->dtype();
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_eq(right2);
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_eq(right2);
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_eq(right2);
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_eq(right2);
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_eq(right2);
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_eq(right2);
  } else throw std::runtime_error("eq: unsupported type: " + right_type);  
}

template <class T>
template <class U>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::typed_eq_im
(const std::shared_ptr<typed_dfscalar<U>>& right) {
  auto bcast_right = broadcast(right->val);
  auto newval =
    val.map(+[](const std::vector<T>& left, U right) {
              auto leftp = left.data();
              auto size = left.size();
              std::vector<int> ret(size);
              auto retp = ret.data();
              for(size_t i = 0; i < size; i++)
                retp[i] = (leftp[i] == right);
              return ret;
      }, bcast_right);
  if(contain_nulls) {
    newval.mapv(reset_null<int>, nulls);
    return std::make_shared<typed_dfcolumn<int>>(std::move(newval), nulls);
  } else {
    auto dvval = newval.template moveto_dvector<int>();
    return std::make_shared<typed_dfcolumn<int>>(std::move(dvval));
  }
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::eq_im(const std::shared_ptr<dfscalar>& right) {
  auto right_type = right->dtype();
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_eq_im(right2);
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_eq_im(right2);
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_eq_im(right2);
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_eq_im(right2);
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_eq_im(right2);
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_eq_im(right2);
  } else throw std::runtime_error("eq_im: unsupported type: " + right_type);
}

template <class T>
template <class U>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::typed_neq(const std::shared_ptr<typed_dfcolumn<U>>& right) {
  auto newval =
    val.map(+[](const std::vector<T>& left, 
                const std::vector<U>& right) {
              auto leftp = left.data();
              auto rightp = right.data();
              auto size = left.size();
              std::vector<int> ret(size);
              auto retp = ret.data();
              for(size_t i = 0; i < size; i++)
                retp[i] = (leftp[i] != rightp[i]);
              return ret;
      }, right->val);
  if(contain_nulls || right->contain_nulls) {
    auto newnulls = nulls.map(set_union<size_t>, right->nulls);
    newval.mapv(reset_null<int>, newnulls);
    return std::make_shared<typed_dfcolumn<int>>
      (std::move(newval), std::move(newnulls));
  } else {
    auto dvval = newval.template moveto_dvector<int>();
    return std::make_shared<typed_dfcolumn<int>>(std::move(dvval));
  }
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::neq(const std::shared_ptr<dfcolumn>& right) {
  auto right_type = right->dtype();
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_neq(right2);
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_neq(right2);
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_neq(right2);
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_neq(right2);
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_neq(right2);
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_neq(right2);
  } else throw std::runtime_error("neq: unsupported type: " + right_type);  
}

template <class T>
template <class U>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::typed_neq_im
(const std::shared_ptr<typed_dfscalar<U>>& right) {
  auto bcast_right = broadcast(right->val);
  auto newval =
    val.map(+[](const std::vector<T>& left, U right) {
              auto leftp = left.data();
              auto size = left.size();
              std::vector<int> ret(size);
              auto retp = ret.data();
              for(size_t i = 0; i < size; i++)
                retp[i] = (leftp[i] != right);
              return ret;
      }, bcast_right);
  if(contain_nulls) {
    newval.mapv(reset_null<int>, nulls);
    return std::make_shared<typed_dfcolumn<int>>(std::move(newval), nulls);
  } else {
    auto dvval = newval.template moveto_dvector<int>();
    return std::make_shared<typed_dfcolumn<int>>(std::move(dvval));
  }
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::neq_im(const std::shared_ptr<dfscalar>& right) {
  auto right_type = right->dtype();
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_neq_im(right2);
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_neq_im(right2);
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_neq_im(right2);
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_neq_im(right2);
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_neq_im(right2);
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_neq_im(right2);
  } else throw std::runtime_error("neq_im: unsupported type: " + right_type);
}

template <class T>
template <class U>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::typed_lt(const std::shared_ptr<typed_dfcolumn<U>>& right) {
  auto newval =
    val.map(+[](const std::vector<T>& left, 
                const std::vector<U>& right) {
              auto leftp = left.data();
              auto rightp = right.data();
              auto size = left.size();
              std::vector<int> ret(size);
              auto retp = ret.data();
              for(size_t i = 0; i < size; i++)
                retp[i] = (leftp[i] < rightp[i]);
              return ret;
      }, right->val);
  if(contain_nulls || right->contain_nulls) {
    auto newnulls = nulls.map(set_union<size_t>, right->nulls);
    newval.mapv(reset_null<int>, newnulls);
    return std::make_shared<typed_dfcolumn<int>>
      (std::move(newval), std::move(newnulls));
  } else {
    auto dvval = newval.template moveto_dvector<int>();
    return std::make_shared<typed_dfcolumn<int>>(std::move(dvval));
  }
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::lt(const std::shared_ptr<dfcolumn>& right) {
  auto right_type = right->dtype();
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_lt(right2);
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_lt(right2);
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_lt(right2);
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_lt(right2);
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_lt(right2);
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_lt(right2);
  } else throw std::runtime_error("lt: unsupported type: " + right_type);  
}

template <class T>
template <class U>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::typed_lt_im
(const std::shared_ptr<typed_dfscalar<U>>& right) {
  auto bcast_right = broadcast(right->val);
  auto newval =
    val.map(+[](const std::vector<T>& left, U right) {
              auto leftp = left.data();
              auto size = left.size();
              std::vector<int> ret(size);
              auto retp = ret.data();
              for(size_t i = 0; i < size; i++)
                retp[i] = (leftp[i] < right);
              return ret;
      }, bcast_right);
  if(contain_nulls) {
    newval.mapv(reset_null<int>, nulls);
    return std::make_shared<typed_dfcolumn<int>>(std::move(newval), nulls);
  } else {
    auto dvval = newval.template moveto_dvector<int>();
    return std::make_shared<typed_dfcolumn<int>>(std::move(dvval));
  }
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::lt_im(const std::shared_ptr<dfscalar>& right) {
  auto right_type = right->dtype();
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_lt_im(right2);
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_lt_im(right2);
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_lt_im(right2);
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_lt_im(right2);
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_lt_im(right2);
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_lt_im(right2);
  } else throw std::runtime_error("lt_im: unsupported type: " + right_type);
}

template <class T>
template <class U>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::typed_le(const std::shared_ptr<typed_dfcolumn<U>>& right) {
  auto newval =
    val.map(+[](const std::vector<T>& left, 
                const std::vector<U>& right) {
              auto leftp = left.data();
              auto rightp = right.data();
              auto size = left.size();
              std::vector<int> ret(size);
              auto retp = ret.data();
              for(size_t i = 0; i < size; i++)
                retp[i] = (leftp[i] <= rightp[i]);
              return ret;
      }, right->val);
  if(contain_nulls || right->contain_nulls) {
    auto newnulls = nulls.map(set_union<size_t>, right->nulls);
    newval.mapv(reset_null<int>, newnulls);
    return std::make_shared<typed_dfcolumn<int>>
      (std::move(newval), std::move(newnulls));
  } else {
    auto dvval = newval.template moveto_dvector<int>();
    return std::make_shared<typed_dfcolumn<int>>(std::move(dvval));
  }
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::le(const std::shared_ptr<dfcolumn>& right) {
  auto right_type = right->dtype();
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_le(right2);
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_le(right2);
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_le(right2);
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_le(right2);
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_le(right2);
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_le(right2);
  } else throw std::runtime_error("le: unsupported type: " + right_type);  
}

template <class T>
template <class U>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::typed_le_im
(const std::shared_ptr<typed_dfscalar<U>>& right) {
  auto bcast_right = broadcast(right->val);
  auto newval =
    val.map(+[](const std::vector<T>& left, U right) {
              auto leftp = left.data();
              auto size = left.size();
              std::vector<int> ret(size);
              auto retp = ret.data();
              for(size_t i = 0; i < size; i++)
                retp[i] = (leftp[i] <= right);
              return ret;
      }, bcast_right);
  if(contain_nulls) {
    newval.mapv(reset_null<int>, nulls);
    return std::make_shared<typed_dfcolumn<int>>(std::move(newval), nulls);
  } else {
    auto dvval = newval.template moveto_dvector<int>();
    return std::make_shared<typed_dfcolumn<int>>(std::move(dvval));
  }
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::le_im(const std::shared_ptr<dfscalar>& right) {
  auto right_type = right->dtype();
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_le_im(right2);
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_le_im(right2);
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_le_im(right2);
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_le_im(right2);
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_le_im(right2);
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_le_im(right2);
  } else throw std::runtime_error("le_im: unsupported type: " + right_type);
}

template <class T>
template <class U>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::typed_gt(const std::shared_ptr<typed_dfcolumn<U>>& right) {
  auto newval =
    val.map(+[](const std::vector<T>& left, 
                const std::vector<U>& right) {
              auto leftp = left.data();
              auto rightp = right.data();
              auto size = left.size();
              std::vector<int> ret(size);
              auto retp = ret.data();
              for(size_t i = 0; i < size; i++)
                retp[i] = (leftp[i] > rightp[i]);
              return ret;
      }, right->val);
  if(contain_nulls || right->contain_nulls) {
    auto newnulls = nulls.map(set_union<size_t>, right->nulls);
    newval.mapv(reset_null<int>, newnulls);
    return std::make_shared<typed_dfcolumn<int>>
      (std::move(newval), std::move(newnulls));
  } else {
    auto dvval = newval.template moveto_dvector<int>();
    return std::make_shared<typed_dfcolumn<int>>(std::move(dvval));
  }
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::gt(const std::shared_ptr<dfcolumn>& right) {
  auto right_type = right->dtype();
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_gt(right2);
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_gt(right2);
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_gt(right2);
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_gt(right2);
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_gt(right2);
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_gt(right2);
  } else throw std::runtime_error("gt: unsupported type: " + right_type);  
}

template <class T>
template <class U>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::typed_gt_im
(const std::shared_ptr<typed_dfscalar<U>>& right) {
  auto bcast_right = broadcast(right->val);
  auto newval =
    val.map(+[](const std::vector<T>& left, U right) {
              auto leftp = left.data();
              auto size = left.size();
              std::vector<int> ret(size);
              auto retp = ret.data();
              for(size_t i = 0; i < size; i++)
                retp[i] = (leftp[i] > right);
              return ret;
      }, bcast_right);
  if(contain_nulls) {
    newval.mapv(reset_null<int>, nulls);
    return std::make_shared<typed_dfcolumn<int>>(std::move(newval), nulls);
  } else {
    auto dvval = newval.template moveto_dvector<int>();
    return std::make_shared<typed_dfcolumn<int>>(std::move(dvval));
  }
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::gt_im(const std::shared_ptr<dfscalar>& right) {
  auto right_type = right->dtype();
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_gt_im(right2);
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_gt_im(right2);
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_gt_im(right2);
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_gt_im(right2);
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_gt_im(right2);
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_gt_im(right2);
  } else throw std::runtime_error("gt_im: unsupported type: " + right_type);
}

template <class T>
template <class U>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::typed_ge(const std::shared_ptr<typed_dfcolumn<U>>& right) {
  auto newval =
    val.map(+[](const std::vector<T>& left, 
                const std::vector<U>& right) {
              auto leftp = left.data();
              auto rightp = right.data();
              auto size = left.size();
              std::vector<int> ret(size);
              auto retp = ret.data();
              for(size_t i = 0; i < size; i++)
                retp[i] = (leftp[i] >= rightp[i]);
              return ret;
      }, right->val);
  if(contain_nulls || right->contain_nulls) {
    auto newnulls = nulls.map(set_union<size_t>, right->nulls);
    newval.mapv(reset_null<int>, newnulls);
    return std::make_shared<typed_dfcolumn<int>>
      (std::move(newval), std::move(newnulls));
  } else {
    auto dvval = newval.template moveto_dvector<int>();
    return std::make_shared<typed_dfcolumn<int>>(std::move(dvval));
  }
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::ge(const std::shared_ptr<dfcolumn>& right) {
  auto right_type = right->dtype();
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_ge(right2);
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_ge(right2);
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_ge(right2);
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_ge(right2);
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_ge(right2);
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_ge(right2);
  } else throw std::runtime_error("ge: unsupported type: " + right_type);  
}

template <class T>
template <class U>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::typed_ge_im
(const std::shared_ptr<typed_dfscalar<U>>& right) {
  auto bcast_right = broadcast(right->val);
  auto newval =
    val.map(+[](const std::vector<T>& left, U right) {
              auto leftp = left.data();
              auto size = left.size();
              std::vector<int> ret(size);
              auto retp = ret.data();
              for(size_t i = 0; i < size; i++)
                retp[i] = (leftp[i] >= right);
              return ret;
      }, bcast_right);
  if(contain_nulls) {
    newval.mapv(reset_null<int>, nulls);
    return std::make_shared<typed_dfcolumn<int>>(std::move(newval), nulls);
  } else {
    auto dvval = newval.template moveto_dvector<int>();
    return std::make_shared<typed_dfcolumn<int>>(std::move(dvval));
  }
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::ge_im(const std::shared_ptr<dfscalar>& right) {
  auto right_type = right->dtype();
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_ge_im(right2);
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_ge_im(right2);
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_ge_im(right2);
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_ge_im(right2);
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_ge_im(right2);
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_ge_im(right2);
  } else throw std::runtime_error("ge_im: unsupported type: " + right_type);
}

/*
template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::is_null() {
  auto lsizes = make_node_local_scatter(sizes());
  auto ret = nulls.map(+[](const std::vector<size_t>& nulls, size_t size) {
      std::vector<int> ret(size);
      auto retp = ret.data();
      auto nullsp = nulls.data();
      auto nulls_size = nulls.size();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
      for(size_t i = 0; i < nulls_size; i++) retp[nullsp[i]] = 1;
      return ret;
    }, lsizes);
  auto dvval = ret.template moveto_dvector<int>();
  return std::make_shared<typed_dfcolumn<int>>(std::move(dvval));
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::is_not_null() {
  auto lsizes = make_node_local_scatter(sizes());
  auto ret = nulls.map(+[](const std::vector<size_t>& nulls, size_t size) {
      std::vector<int> ret(size, 1);
      auto retp = ret.data();
      auto nullsp = nulls.data();
      auto nulls_size = nulls.size();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
      for(size_t i = 0; i < nulls_size; i++) retp[nullsp[i]] = 0;
      return ret;
    }, lsizes);
  auto dvval = ret.template moveto_dvector<int>();
  return std::make_shared<typed_dfcolumn<int>>(std::move(dvval));
}
*/

template <class T>
template <class U>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::typed_and_op
(const std::shared_ptr<typed_dfcolumn<U>>& right) {
  auto newval =
    val.map(+[](const std::vector<T>& left, 
                const std::vector<U>& right) {
              auto leftp = left.data();
              auto rightp = right.data();
              auto size = left.size();
              std::vector<int> ret(size);
              auto retp = ret.data();
              for(size_t i = 0; i < size; i++) {
                retp[i] = (leftp[i] && rightp[i]);
              }
              return ret;
      }, right->val);
  if(contain_nulls || right->contain_nulls) {
    auto newnulls = nulls.map(set_union<size_t>, right->nulls);
    newval.mapv(reset_null<int>, newnulls);
    return std::make_shared<typed_dfcolumn<int>>
      (std::move(newval), std::move(newnulls));
  } else {
    auto dvval = newval.template moveto_dvector<int>();
    return std::make_shared<typed_dfcolumn<int>>(std::move(dvval));
  }
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::and_op(const std::shared_ptr<dfcolumn>& right) {
  auto right_type = right->dtype();
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_and_op(right2);
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_and_op(right2);
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_and_op(right2);
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_and_op(right2);
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_and_op(right2);
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_and_op(right2);
  } else throw std::runtime_error("and_op: unsupported type: " + right_type);
}

template <class T>
template <class U>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::typed_or_op
(const std::shared_ptr<typed_dfcolumn<U>>& right) {
  auto newval =
    val.map(+[](const std::vector<T>& left, 
                const std::vector<U>& right) {
              auto leftp = left.data();
              auto rightp = right.data();
              auto size = left.size();
              std::vector<int> ret(size);
              auto retp = ret.data();
              for(size_t i = 0; i < size; i++) {
                retp[i] = (leftp[i] || rightp[i]);
              }
              return ret;
      }, right->val);
  if(contain_nulls || right->contain_nulls) {
    auto newnulls = nulls.map(set_union<size_t>, right->nulls);
    newval.mapv(reset_null<int>, newnulls);
    return std::make_shared<typed_dfcolumn<int>>
      (std::move(newval), std::move(newnulls));
  } else {
    auto dvval = newval.template moveto_dvector<int>();
    return std::make_shared<typed_dfcolumn<int>>(std::move(dvval));
  }
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::or_op(const std::shared_ptr<dfcolumn>& right) {
  auto right_type = right->dtype();
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_or_op(right2);
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_or_op(right2);
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_or_op(right2);
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_or_op(right2);
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_or_op(right2);
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_or_op(right2);
  } else throw std::runtime_error("or_op: unsupported type: " + right_type);
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::not_op() {
  auto newval =
    val.map(+[](const std::vector<T>& left) {
              auto leftp = left.data();
              auto size = left.size();
              std::vector<int> ret(size);
              auto retp = ret.data();
              for(size_t i = 0; i < size; i++)
                retp[i] = !leftp[i];
              return ret;
      });
  if(contain_nulls) {
    newval.mapv(reset_null<int>, nulls);
    return std::make_shared<typed_dfcolumn<int>>(std::move(newval), nulls);
  } else {
    auto dvval = newval.template moveto_dvector<int>();
    return std::make_shared<typed_dfcolumn<int>>(std::move(dvval));
  }
}

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::eq(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::eq(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::eq(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::eq(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::eq(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::eq(const std::shared_ptr<dfcolumn>& right);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::eq_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::eq_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::eq_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::eq_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::eq_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::eq_im(const std::shared_ptr<dfscalar>& right);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::neq(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::neq(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::neq(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::neq(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::neq(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::neq(const std::shared_ptr<dfcolumn>& right);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::neq_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::neq_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::neq_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::neq_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::neq_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::neq_im(const std::shared_ptr<dfscalar>& right);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::lt(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::lt(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::lt(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::lt(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::lt(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::lt(const std::shared_ptr<dfcolumn>& right);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::lt_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::lt_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::lt_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::lt_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::lt_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::lt_im(const std::shared_ptr<dfscalar>& right);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::le(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::le(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::le(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::le(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::le(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::le(const std::shared_ptr<dfcolumn>& right);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::le_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::le_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::le_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::le_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::le_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::le_im(const std::shared_ptr<dfscalar>& right);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::gt(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::gt(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::gt(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::gt(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::gt(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::gt(const std::shared_ptr<dfcolumn>& right);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::gt_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::gt_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::gt_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::gt_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::gt_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::gt_im(const std::shared_ptr<dfscalar>& right);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::ge(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::ge(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::ge(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::ge(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::ge(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::ge(const std::shared_ptr<dfcolumn>& right);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::ge_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::ge_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::ge_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::ge_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::ge_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::ge_im(const std::shared_ptr<dfscalar>& right);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::and_op(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::and_op(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::and_op(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::and_op(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::and_op(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::and_op(const std::shared_ptr<dfcolumn>& right);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::or_op(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::or_op(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::or_op(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::or_op(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::or_op(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::or_op(const std::shared_ptr<dfcolumn>& right);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::not_op();
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::not_op();
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::not_op();
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::not_op();
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::not_op();
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::not_op();


// ----- datetime -----

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::typed_eq(
  const std::shared_ptr<typed_dfcolumn<datetime>>& right) {
  auto newval =
    val.map(+[](const std::vector<datetime_t>& left,
                const std::vector<datetime_t>& right) {
              auto leftp = left.data();
              auto rightp = right.data();
              auto size = left.size();
              std::vector<int> ret(size);
              auto retp = ret.data();
              for(size_t i = 0; i < size; i++)
                retp[i] = (leftp[i] == rightp[i]);
              return ret;
      }, right->val);
  if(contain_nulls || right->contain_nulls) {
    auto newnulls = nulls.map(set_union<size_t>, right->nulls);
    newval.mapv(reset_null<int>, newnulls);
    return std::make_shared<typed_dfcolumn<int>>
      (std::move(newval), std::move(newnulls));
  } else {
    auto dvval = newval.template moveto_dvector<int>();
    return std::make_shared<typed_dfcolumn<int>>(std::move(dvval));
  }
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::eq(const std::shared_ptr<dfcolumn>& right) {
  auto right_type = right->dtype();
  std::shared_ptr<typed_dfcolumn<datetime>> right2 = NULL;
  if (right_type == "string" || right_type == "dic_string" ||
      right_type == "raw_string") {
    auto tmp = right->type_cast("datetime");
    right2 = std::dynamic_pointer_cast<typed_dfcolumn<datetime>>(tmp);
  } else if(right_type == "datetime") {
    right2 = std::dynamic_pointer_cast<typed_dfcolumn<datetime>>(right);
  } else throw std::runtime_error(
    "datetime == " + right_type + ": invalid operation!");
  if(!static_cast<bool>(right2)) 
    throw std::runtime_error("internal type error");
  return typed_eq(right2);
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::typed_eq_im
(const std::shared_ptr<typed_dfscalar<datetime>>& right) {
  auto bcast_right = broadcast(right->val);
  auto newval =
    val.map(+[](const std::vector<datetime_t>& left, datetime_t right) {
              auto leftp = left.data();
              auto size = left.size();
              std::vector<int> ret(size);
              auto retp = ret.data();
              for(size_t i = 0; i < size; i++)
                retp[i] = (leftp[i] == right);
              return ret;
      }, bcast_right);
  if(contain_nulls) {
    newval.mapv(reset_null<int>, nulls);
    return std::make_shared<typed_dfcolumn<int>>(std::move(newval), nulls);
  } else {
    auto dvval = newval.template moveto_dvector<int>();
    return std::make_shared<typed_dfcolumn<int>>(std::move(dvval));
  }
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::eq_im(const std::shared_ptr<dfscalar>& right) {
  auto right_type = right->dtype();
  std::shared_ptr<typed_dfscalar<datetime>> right2 = NULL;
  if (right_type == "string" || right_type == "dic_string" ||
      right_type == "raw_string") {
    auto tmp = right->type_cast("datetime");
    right2 = std::dynamic_pointer_cast<typed_dfscalar<datetime>>(tmp);
  } else if(right_type == "datetime") {
    right2 = std::dynamic_pointer_cast<typed_dfscalar<datetime>>(right);
  } else { // must be of datetime_t type
    auto tmp = std::dynamic_pointer_cast<typed_dfscalar<datetime_t>>(right);
    if(!static_cast<bool>(tmp)) throw std::runtime_error(
      "datetime == " + right_type + ": invalid operation!");
    right2 = std::make_shared<typed_dfscalar<datetime>>(tmp->val);
  }
  return typed_eq_im(right2);
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::typed_neq(
  const std::shared_ptr<typed_dfcolumn<datetime>>& right) {
  auto newval =
    val.map(+[](const std::vector<datetime_t>& left,
                const std::vector<datetime_t>& right) {
              auto leftp = left.data();
              auto rightp = right.data();
              auto size = left.size();
              std::vector<int> ret(size);
              auto retp = ret.data();
              for(size_t i = 0; i < size; i++)
                retp[i] = (leftp[i] != rightp[i]);
              return ret;
      }, right->val);
  if(contain_nulls || right->contain_nulls) {
    auto newnulls = nulls.map(set_union<size_t>, right->nulls);
    newval.mapv(reset_null<int>, newnulls);
    return std::make_shared<typed_dfcolumn<int>>
      (std::move(newval), std::move(newnulls));
  } else {
    auto dvval = newval.template moveto_dvector<int>();
    return std::make_shared<typed_dfcolumn<int>>(std::move(dvval));
  }
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::neq(const std::shared_ptr<dfcolumn>& right) {
  auto right_type = right->dtype();
  std::shared_ptr<typed_dfcolumn<datetime>> right2 = NULL;
  if (right_type == "string" || right_type == "dic_string" ||
      right_type == "raw_string") {
    auto tmp = right->type_cast("datetime");
    right2 = std::dynamic_pointer_cast<typed_dfcolumn<datetime>>(tmp);
  } else if(right_type == "datetime") {
    right2 = std::dynamic_pointer_cast<typed_dfcolumn<datetime>>(right);
  } else throw std::runtime_error(
    "datetime != " + right_type + ": invalid operation!");
  if(!static_cast<bool>(right2)) 
    throw std::runtime_error("internal type error");
  return typed_neq(right2);
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::typed_neq_im
(const std::shared_ptr<typed_dfscalar<datetime>>& right) {
  auto bcast_right = broadcast(right->val);
  auto newval =
    val.map(+[](const std::vector<datetime_t>& left, datetime_t right) {
              auto leftp = left.data();
              auto size = left.size();
              std::vector<int> ret(size);
              auto retp = ret.data();
              for(size_t i = 0; i < size; i++)
                retp[i] = (leftp[i] != right);
              return ret;
      }, bcast_right);
  if(contain_nulls) {
    newval.mapv(reset_null<int>, nulls);
    return std::make_shared<typed_dfcolumn<int>>(std::move(newval), nulls);
  } else {
    auto dvval = newval.template moveto_dvector<int>();
    return std::make_shared<typed_dfcolumn<int>>(std::move(dvval));
  }
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::neq_im(const std::shared_ptr<dfscalar>& right) {
  auto right_type = right->dtype();
  std::shared_ptr<typed_dfscalar<datetime>> right2 = NULL;
  if (right_type == "string" || right_type == "dic_string" ||
      right_type == "raw_string") {
    auto tmp = right->type_cast("datetime");
    right2 = std::dynamic_pointer_cast<typed_dfscalar<datetime>>(tmp);
  } else if(right_type == "datetime") {
    right2 = std::dynamic_pointer_cast<typed_dfscalar<datetime>>(right);
  } else { // must be of datetime_t type
    auto tmp = std::dynamic_pointer_cast<typed_dfscalar<datetime_t>>(right);
    if(!static_cast<bool>(tmp)) throw std::runtime_error(
      "datetime != " + right_type + ": invalid operation!");
    right2 = std::make_shared<typed_dfscalar<datetime>>(tmp->val);
  }
  return typed_neq_im(right2);
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::typed_gt(
  const std::shared_ptr<typed_dfcolumn<datetime>>& right) {
  auto newval =
    val.map(+[](const std::vector<datetime_t>& left,
                const std::vector<datetime_t>& right) {
              auto leftp = left.data();
              auto rightp = right.data();
              auto size = left.size();
              std::vector<int> ret(size);
              auto retp = ret.data();
              for(size_t i = 0; i < size; i++)
                retp[i] = (leftp[i] > rightp[i]);
              return ret;
      }, right->val);
  if(contain_nulls || right->contain_nulls) {
    auto newnulls = nulls.map(set_union<size_t>, right->nulls);
    newval.mapv(reset_null<int>, newnulls);
    return std::make_shared<typed_dfcolumn<int>>
      (std::move(newval), std::move(newnulls));
  } else {
    auto dvval = newval.template moveto_dvector<int>();
    return std::make_shared<typed_dfcolumn<int>>(std::move(dvval));
  }
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::gt(const std::shared_ptr<dfcolumn>& right) {
  auto right_type = right->dtype();
  std::shared_ptr<typed_dfcolumn<datetime>> right2 = NULL;
  if (right_type == "string" || right_type == "dic_string" ||
      right_type == "raw_string") {
    auto tmp = right->type_cast("datetime");
    right2 = std::dynamic_pointer_cast<typed_dfcolumn<datetime>>(tmp);
  } else if(right_type == "datetime") {
    right2 = std::dynamic_pointer_cast<typed_dfcolumn<datetime>>(right);
  } else throw std::runtime_error(
    "datetime > " + right_type + ": invalid operation!");
  if(!static_cast<bool>(right2)) 
    throw std::runtime_error("internal type error");
  return typed_gt(right2);
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::typed_gt_im
(const std::shared_ptr<typed_dfscalar<datetime>>& right) {
  auto bcast_right = broadcast(right->val);
  auto newval =
    val.map(+[](const std::vector<datetime_t>& left, datetime_t right) {
              auto leftp = left.data();
              auto size = left.size();
              std::vector<int> ret(size);
              auto retp = ret.data();
              for(size_t i = 0; i < size; i++)
                retp[i] = (leftp[i] > right);
              return ret;
      }, bcast_right);
  if(contain_nulls) {
    newval.mapv(reset_null<int>, nulls);
    return std::make_shared<typed_dfcolumn<int>>(std::move(newval), nulls);
  } else {
    auto dvval = newval.template moveto_dvector<int>();
    return std::make_shared<typed_dfcolumn<int>>(std::move(dvval));
  }
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::gt_im(const std::shared_ptr<dfscalar>& right) {
  auto right_type = right->dtype();
  std::shared_ptr<typed_dfscalar<datetime>> right2 = NULL;
  if (right_type == "string" || right_type == "dic_string" ||
      right_type == "raw_string") {
    auto tmp = right->type_cast("datetime");
    right2 = std::dynamic_pointer_cast<typed_dfscalar<datetime>>(tmp);
  } else if(right_type == "datetime") {
    right2 = std::dynamic_pointer_cast<typed_dfscalar<datetime>>(right);
  } else { // must be of datetime_t type
    auto tmp = std::dynamic_pointer_cast<typed_dfscalar<datetime_t>>(right);
    if(!static_cast<bool>(tmp)) throw std::runtime_error(
      "datetime > " + right_type + ": invalid operation!");
    right2 = std::make_shared<typed_dfscalar<datetime>>(tmp->val);
  }
  return typed_gt_im(right2);
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::typed_ge(
  const std::shared_ptr<typed_dfcolumn<datetime>>& right) {
  auto newval =
    val.map(+[](const std::vector<datetime_t>& left,
                const std::vector<datetime_t>& right) {
              auto leftp = left.data();
              auto rightp = right.data();
              auto size = left.size();
              std::vector<int> ret(size);
              auto retp = ret.data();
              for(size_t i = 0; i < size; i++)
                retp[i] = (leftp[i] >= rightp[i]);
              return ret;
      }, right->val);
  if(contain_nulls || right->contain_nulls) {
    auto newnulls = nulls.map(set_union<size_t>, right->nulls);
    newval.mapv(reset_null<int>, newnulls);
    return std::make_shared<typed_dfcolumn<int>>
      (std::move(newval), std::move(newnulls));
  } else {
    auto dvval = newval.template moveto_dvector<int>();
    return std::make_shared<typed_dfcolumn<int>>(std::move(dvval));
  }
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::ge(const std::shared_ptr<dfcolumn>& right) {
  auto right_type = right->dtype();
  std::shared_ptr<typed_dfcolumn<datetime>> right2 = NULL;
  if (right_type == "string" || right_type == "dic_string" ||
      right_type == "raw_string") {
    auto tmp = right->type_cast("datetime");
    right2 = std::dynamic_pointer_cast<typed_dfcolumn<datetime>>(tmp);
  } else if(right_type == "datetime") {
    right2 = std::dynamic_pointer_cast<typed_dfcolumn<datetime>>(right);
  } else throw std::runtime_error(
    "datetime >= " + right_type + ": invalid operation!");
  if(!static_cast<bool>(right2)) 
    throw std::runtime_error("internal type error");
  return typed_ge(right2);
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::typed_ge_im
(const std::shared_ptr<typed_dfscalar<datetime>>& right) {
  auto bcast_right = broadcast(right->val);
  auto newval =
    val.map(+[](const std::vector<datetime_t>& left, datetime_t right) {
              auto leftp = left.data();
              auto size = left.size();
              std::vector<int> ret(size);
              auto retp = ret.data();
              for(size_t i = 0; i < size; i++)
                retp[i] = (leftp[i] >= right);
              return ret;
      }, bcast_right);
  if(contain_nulls) {
    newval.mapv(reset_null<int>, nulls);
    return std::make_shared<typed_dfcolumn<int>>(std::move(newval), nulls);
  } else {
    auto dvval = newval.template moveto_dvector<int>();
    return std::make_shared<typed_dfcolumn<int>>(std::move(dvval));
  }
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::ge_im(const std::shared_ptr<dfscalar>& right) {
  auto right_type = right->dtype();
  std::shared_ptr<typed_dfscalar<datetime>> right2 = NULL;
  if (right_type == "string" || right_type == "dic_string" ||
      right_type == "raw_string") {
    auto tmp = right->type_cast("datetime");
    right2 = std::dynamic_pointer_cast<typed_dfscalar<datetime>>(tmp);
  } else if(right_type == "datetime") {
    right2 = std::dynamic_pointer_cast<typed_dfscalar<datetime>>(right);
  } else { // must be of datetime_t type
    auto tmp = std::dynamic_pointer_cast<typed_dfscalar<datetime_t>>(right);
    if(!static_cast<bool>(tmp)) throw std::runtime_error(
      "datetime >= " + right_type + ": invalid operation!");
    right2 = std::make_shared<typed_dfscalar<datetime>>(tmp->val);
  }
  return typed_ge_im(right2);
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::typed_lt(
  const std::shared_ptr<typed_dfcolumn<datetime>>& right) {
  auto newval =
    val.map(+[](const std::vector<datetime_t>& left,
                const std::vector<datetime_t>& right) {
              auto leftp = left.data();
              auto rightp = right.data();
              auto size = left.size();
              std::vector<int> ret(size);
              auto retp = ret.data();
              for(size_t i = 0; i < size; i++)
                retp[i] = (leftp[i] < rightp[i]);
              return ret;
      }, right->val);
  if(contain_nulls || right->contain_nulls) {
    auto newnulls = nulls.map(set_union<size_t>, right->nulls);
    newval.mapv(reset_null<int>, newnulls);
    return std::make_shared<typed_dfcolumn<int>>
      (std::move(newval), std::move(newnulls));
  } else {
    auto dvval = newval.template moveto_dvector<int>();
    return std::make_shared<typed_dfcolumn<int>>(std::move(dvval));
  }
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::lt(const std::shared_ptr<dfcolumn>& right) {
  auto right_type = right->dtype();
  std::shared_ptr<typed_dfcolumn<datetime>> right2 = NULL;
  if (right_type == "string" || right_type == "dic_string" ||
      right_type == "raw_string") {
    auto tmp = right->type_cast("datetime");
    right2 = std::dynamic_pointer_cast<typed_dfcolumn<datetime>>(tmp);
  } else if(right_type == "datetime") {
    right2 = std::dynamic_pointer_cast<typed_dfcolumn<datetime>>(right);
  } else throw std::runtime_error(
    "datetime < " + right_type + ": invalid operation!");
  if(!static_cast<bool>(right2)) 
    throw std::runtime_error("internal type error");
  return typed_lt(right2);
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::typed_lt_im
(const std::shared_ptr<typed_dfscalar<datetime>>& right) {
  auto bcast_right = broadcast(right->val);
  auto newval =
    val.map(+[](const std::vector<datetime_t>& left, datetime_t right) {
              auto leftp = left.data();
              auto size = left.size();
              std::vector<int> ret(size);
              auto retp = ret.data();
              for(size_t i = 0; i < size; i++)
                retp[i] = (leftp[i] < right);
              return ret;
      }, bcast_right);
  if(contain_nulls) {
    newval.mapv(reset_null<int>, nulls);
    return std::make_shared<typed_dfcolumn<int>>(std::move(newval), nulls);
  } else {
    auto dvval = newval.template moveto_dvector<int>();
    return std::make_shared<typed_dfcolumn<int>>(std::move(dvval));
  }
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::lt_im(const std::shared_ptr<dfscalar>& right) {
  auto right_type = right->dtype();
  std::shared_ptr<typed_dfscalar<datetime>> right2 = NULL;
  if (right_type == "string" || right_type == "dic_string" ||
      right_type == "raw_string") {
    auto tmp = right->type_cast("datetime");
    right2 = std::dynamic_pointer_cast<typed_dfscalar<datetime>>(tmp);
  } else if(right_type == "datetime") {
    right2 = std::dynamic_pointer_cast<typed_dfscalar<datetime>>(right);
  } else { // must be of datetime_t type
    auto tmp = std::dynamic_pointer_cast<typed_dfscalar<datetime_t>>(right);
    if(!static_cast<bool>(tmp)) throw std::runtime_error(
      "datetime < " + right_type + ": invalid operation!");
    right2 = std::make_shared<typed_dfscalar<datetime>>(tmp->val);
  }
  return typed_lt_im(right2);
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::typed_le(
  const std::shared_ptr<typed_dfcolumn<datetime>>& right) {
  auto newval =
    val.map(+[](const std::vector<datetime_t>& left,
                const std::vector<datetime_t>& right) {
              auto leftp = left.data();
              auto rightp = right.data();
              auto size = left.size();
              std::vector<int> ret(size);
              auto retp = ret.data();
              for(size_t i = 0; i < size; i++)
                retp[i] = (leftp[i] <= rightp[i]);
              return ret;
      }, right->val);
  if(contain_nulls || right->contain_nulls) {
    auto newnulls = nulls.map(set_union<size_t>, right->nulls);
    newval.mapv(reset_null<int>, newnulls);
    return std::make_shared<typed_dfcolumn<int>>
      (std::move(newval), std::move(newnulls));
  } else {
    auto dvval = newval.template moveto_dvector<int>();
    return std::make_shared<typed_dfcolumn<int>>(std::move(dvval));
  }
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::le(const std::shared_ptr<dfcolumn>& right) {
  auto right_type = right->dtype();
  std::shared_ptr<typed_dfcolumn<datetime>> right2 = NULL;
  if (right_type == "string" || right_type == "dic_string" ||
      right_type == "raw_string") {
    auto tmp = right->type_cast("datetime");
    right2 = std::dynamic_pointer_cast<typed_dfcolumn<datetime>>(tmp);
  } else if(right_type == "datetime") {
    right2 = std::dynamic_pointer_cast<typed_dfcolumn<datetime>>(right);
  } else throw std::runtime_error(
    "datetime <= " + right_type + ": invalid operation!");
  if(!static_cast<bool>(right2)) 
    throw std::runtime_error("internal type error");
  return typed_le(right2);
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::typed_le_im
(const std::shared_ptr<typed_dfscalar<datetime>>& right) {
  auto bcast_right = broadcast(right->val);
  auto newval =
    val.map(+[](const std::vector<datetime_t>& left, datetime_t right) {
              auto leftp = left.data();
              auto size = left.size();
              std::vector<int> ret(size);
              auto retp = ret.data();
              for(size_t i = 0; i < size; i++)
                retp[i] = (leftp[i] <= right);
              return ret;
      }, bcast_right);
  if(contain_nulls) {
    newval.mapv(reset_null<int>, nulls);
    return std::make_shared<typed_dfcolumn<int>>(std::move(newval), nulls);
  } else {
    auto dvval = newval.template moveto_dvector<int>();
    return std::make_shared<typed_dfcolumn<int>>(std::move(dvval));
  }
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::le_im(const std::shared_ptr<dfscalar>& right) {
  auto right_type = right->dtype();
  std::shared_ptr<typed_dfscalar<datetime>> right2 = NULL;
  if (right_type == "string" || right_type == "dic_string" ||
      right_type == "raw_string") {
    auto tmp = right->type_cast("datetime");
    right2 = std::dynamic_pointer_cast<typed_dfscalar<datetime>>(tmp);
  } else if(right_type == "datetime") {
    right2 = std::dynamic_pointer_cast<typed_dfscalar<datetime>>(right);
  } else { // must be of datetime_t type
    auto tmp = std::dynamic_pointer_cast<typed_dfscalar<datetime_t>>(right);
    if(!static_cast<bool>(tmp)) throw std::runtime_error(
      "datetime <= " + right_type + ": invalid operation!");
    right2 = std::make_shared<typed_dfscalar<datetime>>(tmp->val);
  }
  return typed_le_im(right2);
}

}
