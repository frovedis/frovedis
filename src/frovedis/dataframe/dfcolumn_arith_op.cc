#include "dfcolumn.hpp"

#include <limits>
#include <climits>

#include "dfcolumn_helper.hpp"
#include "../text/parsefloat.hpp"

using namespace std;

namespace frovedis {

template <class T>
std::vector<int> 
do_boolean_cast(const std::vector<T>& v,
                const std::string& from_cast_name,
                bool check_bool_like) { 
  auto size = v.size();
  std::vector<int> ret(size);
  auto vptr = v.data();
  auto rptr = ret.data();
  size_t count_non_bool_like = 0;
  auto null_like = std::numeric_limits<T>::max();
  for (size_t i = 0; i < size; ++i) {
    rptr[i] = (vptr[i] != 0);
    count_non_bool_like += (vptr[i] != 0) && (vptr[i] != 1) && 
                           (vptr[i] != null_like);
  }
  if (check_bool_like && count_non_bool_like) {
    auto tt = get_type_name<T>();
    REPORT_ERROR(USER_ERROR, 
    "some parsed " + from_cast_name + " cannot be treated as true/false");
  }
  return ret;
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::type_cast(const std::string& to_type,
                             bool check_bool_like) {
  std::shared_ptr<dfcolumn> ret;
  auto newnulls = nulls; // copy to move
  if(to_type == "boolean") {
    auto b_ctype = broadcast(dtype());
    auto newval = val.map(do_boolean_cast<T>, 
                          b_ctype, broadcast(check_bool_like))
                     .template moveto_dvector<int>();
    // nulls would also be treated as true, hence no nulls in casted column
    ret = std::make_shared<typed_dfcolumn<int>>(std::move(newval));
  } else if(to_type == "int") {
    auto newval = val.map(do_static_cast<T,int>, nulls);
    ret = std::make_shared<typed_dfcolumn<int>>
      (std::move(newval), std::move(newnulls));
  } else if(to_type == "unsigned int") {
    auto newval = val.map(do_static_cast<T,unsigned int>, nulls);
    ret = std::make_shared<typed_dfcolumn<unsigned int>>
      (std::move(newval), std::move(newnulls));
  } else if(to_type == "long") {
    auto newval = val.map(do_static_cast<T,long>, nulls);
    ret = std::make_shared<typed_dfcolumn<long>>
      (std::move(newval), std::move(newnulls));
  } else if(to_type == "unsigned long") {
    auto newval = val.map(do_static_cast<T,unsigned long>, nulls);
    ret = std::make_shared<typed_dfcolumn<unsigned long>>
      (std::move(newval), std::move(newnulls));
  } else if(to_type == "float") {
    auto newval = val.map(do_static_cast<T,float>, nulls);
    ret = std::make_shared<typed_dfcolumn<float>>
      (std::move(newval), std::move(newnulls));
  } else if(to_type == "double") {
    auto newval = val.map(do_static_cast<T,double>, nulls);
    ret = std::make_shared<typed_dfcolumn<double>>
      (std::move(newval), std::move(newnulls));
  } else if(to_type == "string") {
    auto newval = as_words().map(words_to_vector_string); 
    ret = std::make_shared<typed_dfcolumn<std::string>>
      (std::move(newval), std::move(newnulls));
  } else if(to_type == "dic_string") {
    auto words = as_words();
    ret = std::make_shared<typed_dfcolumn<dic_string>>
      (std::move(words), std::move(newnulls));
  } else if(to_type == "raw_string") {
    auto words = as_words();
    ret = std::make_shared<typed_dfcolumn<raw_string>>
      (std::move(words), std::move(newnulls));
  } else if(to_type == "datetime") {
    auto newval = val.map(do_static_cast<T,datetime_t>, nulls);
    ret = std::make_shared<typed_dfcolumn<datetime>>
      (std::move(newval), std::move(newnulls));
  } else {
    throw std::runtime_error("type_cast: unsupported type: " + to_type);
  }
  return ret;
}

template <class T>
void clear_null(std::vector<T>& val, 
                const std::vector<size_t>& nulls) {
  auto valp = val.data();
  auto nullsp = nulls.data();
  auto size = nulls.size();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < size; i++) valp[nullsp[i]] = static_cast<T>(0);
} 

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::add(const std::shared_ptr<dfcolumn>& right) {
  auto right_type = right->dtype();
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_add(right2);
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_add(right2);
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_add(right2);
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_add(right2);
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_add(right2);
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_add(right2);
  } else throw std::runtime_error("add: unsupported type: " + right_type);
}

template <class T>
template <class U>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::typed_add(const std::shared_ptr<typed_dfcolumn<U>>& right) {
  T a;
  U b;
  typedef decltype(a+b) V;
  auto newval =
    val.map(+[](const std::vector<T>& left, 
                const std::vector<U>& right) {
        return left + right;
      }, right->val);
  if(contain_nulls || right->contain_nulls) {
    auto newnulls = nulls.map(set_union<size_t>, right->nulls);
    newval.mapv(reset_null<V>, newnulls);
    return std::make_shared<typed_dfcolumn<V>>
      (std::move(newval), std::move(newnulls));
  } else {
    auto dvval = newval.template moveto_dvector<V>();
    return std::make_shared<typed_dfcolumn<V>>(std::move(dvval));
  }
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::add_im(const std::shared_ptr<dfscalar>& right) {
  auto right_type = right->dtype();
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_add_im(right2);
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_add_im(right2);
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_add_im(right2);
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_add_im(right2);
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_add_im(right2);
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_add_im(right2);
  } else throw std::runtime_error("add_im: unsupported type: " + right_type);
}

template <class T>
template <class U>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::typed_add_im
(const std::shared_ptr<typed_dfscalar<U>>& right) {
  T a;
  U b;
  typedef decltype(a+b) V;
  auto bcast_right = broadcast(right->val);
  auto newval =
    val.map(+[](const std::vector<T>& left, U right) {
        return left + right;
      }, bcast_right);
  if(contain_nulls) {
    newval.mapv(reset_null<V>, nulls);
    return std::make_shared<typed_dfcolumn<V>>(std::move(newval), nulls);
  } else {
    auto dvval = newval.template moveto_dvector<V>();
    return std::make_shared<typed_dfcolumn<V>>(std::move(dvval));
  }
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::sub(const std::shared_ptr<dfcolumn>& right) {
  auto right_type = right->dtype();
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_sub(right2);
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_sub(right2);
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_sub(right2);
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_sub(right2);
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_sub(right2);
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_sub(right2);
  } else throw std::runtime_error("sub: unsupported type: " + right_type);
}

template <class T>
template <class U>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::typed_sub(const std::shared_ptr<typed_dfcolumn<U>>& right) {
  T a;
  U b;
  typedef decltype(a-b) V;
  auto newval =
    val.map(+[](const std::vector<T>& left, 
                const std::vector<U>& right) {
        return left - right;
      }, right->val);
  if(contain_nulls || right->contain_nulls) {
    auto newnulls = nulls.map(set_union<size_t>, right->nulls);
    newval.mapv(reset_null<V>, newnulls);
    return std::make_shared<typed_dfcolumn<V>>
      (std::move(newval), std::move(newnulls));
  } else {
    auto dvval = newval.template moveto_dvector<V>();
    return std::make_shared<typed_dfcolumn<V>>(std::move(dvval));
  }
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::sub_im(const std::shared_ptr<dfscalar>& right) {
  auto right_type = right->dtype();
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_sub_im(right2);
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_sub_im(right2);
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_sub_im(right2);
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_sub_im(right2);
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_sub_im(right2);
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_sub_im(right2);
  } else throw std::runtime_error("sub_im: unsupported type: " + right_type);
}

template <class T>
template <class U>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::typed_sub_im
(const std::shared_ptr<typed_dfscalar<U>>& right) {
  T a;
  U b;
  typedef decltype(a-b) V;
  auto bcast_right = broadcast(right->val);
  auto newval =
    val.map(+[](const std::vector<T>& left, U right) {
        return left - right;
      }, bcast_right);
  if(contain_nulls) {
    newval.mapv(reset_null<V>, nulls);
    return std::make_shared<typed_dfcolumn<V>>(std::move(newval), nulls);
  } else {
    auto dvval = newval.template moveto_dvector<V>();
    return std::make_shared<typed_dfcolumn<V>>(std::move(dvval));
  }
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::rsub_im(const std::shared_ptr<dfscalar>& right) {
  auto right_type = right->dtype();
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_rsub_im(right2);
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_rsub_im(right2);
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_rsub_im(right2);
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_rsub_im(right2);
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_rsub_im(right2);
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_rsub_im(right2);
  } else throw std::runtime_error("rsub_im: unsupported type: " + right_type);
}

template <class T>
template <class U>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::typed_rsub_im
(const std::shared_ptr<typed_dfscalar<U>>& right) {
  T a;
  U b;
  typedef decltype(a-b) V;
  auto bcast_right = broadcast(right->val);
  auto newval =
    val.map(+[](const std::vector<T>& left, U right) {
        return right - left;
      }, bcast_right);
  if(contain_nulls) {
    newval.mapv(reset_null<V>, nulls);
    return std::make_shared<typed_dfcolumn<V>>(std::move(newval), nulls);
  } else {
    auto dvval = newval.template moveto_dvector<V>();
    return std::make_shared<typed_dfcolumn<V>>(std::move(dvval));
  }
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::mul(const std::shared_ptr<dfcolumn>& right) {
  auto right_type = right->dtype();
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_mul(right2);
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_mul(right2);
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_mul(right2);
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_mul(right2);
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_mul(right2);
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_mul(right2);
  } else throw std::runtime_error("mul: unsupported type: " + right_type);
}

template <class T>
template <class U>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::typed_mul(const std::shared_ptr<typed_dfcolumn<U>>& right) {
  T a;
  U b;
  typedef decltype(a*b) V;
  auto newval =
    val.map(+[](const std::vector<T>& left, 
                const std::vector<U>& right) {
        return left * right;
      }, right->val);
  if(contain_nulls || right->contain_nulls) {
    auto newnulls = nulls.map(set_union<size_t>, right->nulls);
    newval.mapv(reset_null<V>, newnulls);
    return std::make_shared<typed_dfcolumn<V>>
      (std::move(newval), std::move(newnulls));
  } else {
    auto dvval = newval.template moveto_dvector<V>();
    return std::make_shared<typed_dfcolumn<V>>(std::move(dvval));
  }
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::mul_im(const std::shared_ptr<dfscalar>& right) {
  auto right_type = right->dtype();
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_mul_im(right2);
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_mul_im(right2);
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_mul_im(right2);
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_mul_im(right2);
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_mul_im(right2);
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_mul_im(right2);
  } else throw std::runtime_error("mul_im: unsupported type: " + right_type);
}

template <class T>
template <class U>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::typed_mul_im
(const std::shared_ptr<typed_dfscalar<U>>& right) {
  T a;
  U b;
  typedef decltype(a*b) V;
  auto bcast_right = broadcast(right->val);
  auto newval =
    val.map(+[](const std::vector<T>& left, U right) {
        return left * right;
      }, bcast_right);
  if(contain_nulls) {
    newval.mapv(reset_null<V>, nulls);
    return std::make_shared<typed_dfcolumn<V>>(std::move(newval), nulls);
  } else {
    auto dvval = newval.template moveto_dvector<V>();
    return std::make_shared<typed_dfcolumn<V>>(std::move(dvval));
  }
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::fdiv(const std::shared_ptr<dfcolumn>& right) {
  auto right_type = right->dtype();
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_fdiv(right2);
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_fdiv(right2);
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_fdiv(right2);
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_fdiv(right2);
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_fdiv(right2);
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_fdiv(right2);
  } else throw std::runtime_error("fdiv: unsupported type: " + right_type);
}

template <class T>
template <class U>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::typed_fdiv(const std::shared_ptr<typed_dfcolumn<U>>& right) {
  typedef double V;
  auto newval =
    val.map(+[](const std::vector<T>& left, 
                const std::vector<U>& right) {
        return vector_fdiv(left, right);
      }, right->val);
  if(contain_nulls || right->contain_nulls) {
    auto newnulls = nulls.map(set_union<size_t>, right->nulls);
    newval.mapv(reset_null<V>, newnulls);
    return std::make_shared<typed_dfcolumn<V>>
      (std::move(newval), std::move(newnulls));
  } else {
    auto dvval = newval.template moveto_dvector<V>();
    return std::make_shared<typed_dfcolumn<V>>(std::move(dvval));
  }
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::fdiv_im(const std::shared_ptr<dfscalar>& right) {
  auto right_type = right->dtype();
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_fdiv_im(right2);
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_fdiv_im(right2);
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_fdiv_im(right2);
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_fdiv_im(right2);
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_fdiv_im(right2);
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_fdiv_im(right2);
  } else throw std::runtime_error("fdiv_im: unsupported type: " + right_type);
}

template <class T>
template <class U>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::typed_fdiv_im
(const std::shared_ptr<typed_dfscalar<U>>& right) {
  typedef double V;
  auto bcast_right = broadcast(right->val);
  auto newval =
    val.map(+[](const std::vector<T>& left, U right) {
        return vector_fdiv(left, right);
      }, bcast_right);
  if(contain_nulls) {
    newval.mapv(reset_null<V>, nulls);
    return std::make_shared<typed_dfcolumn<V>>(std::move(newval), nulls);
  } else {
    auto dvval = newval.template moveto_dvector<V>();
    return std::make_shared<typed_dfcolumn<V>>(std::move(dvval));
  }
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::rfdiv_im(const std::shared_ptr<dfscalar>& right) {
  auto right_type = right->dtype();
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_rfdiv_im(right2);
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_rfdiv_im(right2);
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_rfdiv_im(right2);
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_rfdiv_im(right2);
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_rfdiv_im(right2);
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_rfdiv_im(right2);
  } else throw std::runtime_error("rfdiv_im: unsupported type: " + right_type);
}

template <class T>
template <class U>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::typed_rfdiv_im
(const std::shared_ptr<typed_dfscalar<U>>& right) {
  typedef double V;
  auto bcast_right = broadcast(right->val);
  auto newval =
    val.map(+[](const std::vector<T>& left, U right) {
        return vector_fdiv(right, left);
      }, bcast_right);
  if(contain_nulls) {
    newval.mapv(reset_null<V>, nulls);
    return std::make_shared<typed_dfcolumn<V>>(std::move(newval), nulls);
  } else {
    auto dvval = newval.template moveto_dvector<V>();
    return std::make_shared<typed_dfcolumn<V>>(std::move(dvval));
  }
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::idiv(const std::shared_ptr<dfcolumn>& right) {
  auto right_type = right->dtype();
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_idiv(right2);
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_idiv(right2);
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_idiv(right2);
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_idiv(right2);
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_idiv(right2);
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_idiv(right2);
  } else throw std::runtime_error("idiv: unsupported type: " + right_type);
}

template <class T>
template <class U>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::typed_idiv(const std::shared_ptr<typed_dfcolumn<U>>& right) {
  typedef long V;
  auto newval =
    val.map(+[](const std::vector<T>& left, 
                const std::vector<U>& right) {
        return vector_idiv(left, right);
      }, right->val);
  if(contain_nulls || right->contain_nulls) {
    auto newnulls = nulls.map(set_union<size_t>, right->nulls);
    newval.mapv(reset_null<V>, newnulls);
    return std::make_shared<typed_dfcolumn<V>>
      (std::move(newval), std::move(newnulls));
  } else {
    auto dvval = newval.template moveto_dvector<V>();
    return std::make_shared<typed_dfcolumn<V>>(std::move(dvval));
  }
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::idiv_im(const std::shared_ptr<dfscalar>& right) {
  auto right_type = right->dtype();
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_idiv_im(right2);
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_idiv_im(right2);
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_idiv_im(right2);
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_idiv_im(right2);
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_idiv_im(right2);
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_idiv_im(right2);
  } else throw std::runtime_error("idiv_im: unsupported type: " + right_type);
}

template <class T>
template <class U>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::typed_idiv_im
(const std::shared_ptr<typed_dfscalar<U>>& right) {
  typedef long V;
  auto bcast_right = broadcast(right->val);
  auto newval =
    val.map(+[](const std::vector<T>& left, U right) {
        return vector_idiv(left, right);
      }, bcast_right);
  if(contain_nulls) {
    newval.mapv(reset_null<V>, nulls);
    return std::make_shared<typed_dfcolumn<V>>(std::move(newval), nulls);
  } else {
    auto dvval = newval.template moveto_dvector<V>();
    return std::make_shared<typed_dfcolumn<V>>(std::move(dvval));
  }
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::ridiv_im(const std::shared_ptr<dfscalar>& right) {
  auto right_type = right->dtype();
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_ridiv_im(right2);
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_ridiv_im(right2);
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_ridiv_im(right2);
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_ridiv_im(right2);
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_ridiv_im(right2);
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_ridiv_im(right2);
  } else throw std::runtime_error("ridiv_im: unsupported type: " + right_type);
}

template <class T>
template <class U>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::typed_ridiv_im
(const std::shared_ptr<typed_dfscalar<U>>& right) {
  typedef long V;
  auto bcast_right = broadcast(right->val);
  auto newval =
    val.map(+[](const std::vector<T>& left, U right) {
        return vector_idiv(right, left);
      }, bcast_right);
  if(contain_nulls) {
    newval.mapv(reset_null<V>, nulls);
    return std::make_shared<typed_dfcolumn<V>>(std::move(newval), nulls);
  } else {
    auto dvval = newval.template moveto_dvector<V>();
    return std::make_shared<typed_dfcolumn<V>>(std::move(dvval));
  }
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::mod(const std::shared_ptr<dfcolumn>& right) {
  auto right_type = right->dtype();
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_mod(right2);
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_mod(right2);
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_mod(right2);
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_mod(right2);
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_mod(right2);
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_mod(right2);
  } else throw std::runtime_error("mod: unsupported type: " + right_type);
}

template <class T>
template <class U>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::typed_mod(const std::shared_ptr<typed_dfcolumn<U>>& right) {
  T a;
  U b;
  long c;
  typedef decltype(a - b * c) V;
  auto newval =
    val.map(+[](const std::vector<T>& left, 
                const std::vector<U>& right) {
        return vector_mod(left, right);
      }, right->val);
  if(contain_nulls || right->contain_nulls) {
    auto newnulls = nulls.map(set_union<size_t>, right->nulls);
    newval.mapv(reset_null<V>, newnulls);
    return std::make_shared<typed_dfcolumn<V>>
      (std::move(newval), std::move(newnulls));
  } else {
    auto dvval = newval.template moveto_dvector<V>();
    return std::make_shared<typed_dfcolumn<V>>(std::move(dvval));
  }
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::mod_im(const std::shared_ptr<dfscalar>& right) {
  auto right_type = right->dtype();
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_mod_im(right2);
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_mod_im(right2);
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_mod_im(right2);
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_mod_im(right2);
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_mod_im(right2);
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_mod_im(right2);
  } else throw std::runtime_error("mod_im: unsupported type: " + right_type);
}

template <class T>
template <class U>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::typed_mod_im
(const std::shared_ptr<typed_dfscalar<U>>& right) {
  T a;
  U b;
  long c;
  typedef decltype(a - b * c) V;
  auto bcast_right = broadcast(right->val);
  auto newval =
    val.map(+[](const std::vector<T>& left, U right) {
        return vector_mod(left, right);
      }, bcast_right);
  if(contain_nulls) {
    newval.mapv(reset_null<V>, nulls);
    return std::make_shared<typed_dfcolumn<V>>(std::move(newval), nulls);
  } else {
    auto dvval = newval.template moveto_dvector<V>();
    return std::make_shared<typed_dfcolumn<V>>(std::move(dvval));
  }
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::rmod_im(const std::shared_ptr<dfscalar>& right) {
  auto right_type = right->dtype();
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_rmod_im(right2);
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_rmod_im(right2);
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_rmod_im(right2);
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_rmod_im(right2);
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_rmod_im(right2);
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_rmod_im(right2);
  } else throw std::runtime_error("rmod_im: unsupported type: " + right_type);
}

template <class T>
template <class U>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::typed_rmod_im
(const std::shared_ptr<typed_dfscalar<U>>& right) {
  T a;
  U b;
  long c;
  typedef decltype(a - b * c) V;
  auto bcast_right = broadcast(right->val);
  auto newval =
    val.map(+[](const std::vector<T>& left, U right) {
        return vector_mod(right, left);
      }, bcast_right);
  if(contain_nulls) {
    newval.mapv(reset_null<V>, nulls);
    return std::make_shared<typed_dfcolumn<V>>(std::move(newval), nulls);
  } else {
    auto dvval = newval.template moveto_dvector<V>();
    return std::make_shared<typed_dfcolumn<V>>(std::move(dvval));
  }
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::pow(const std::shared_ptr<dfcolumn>& right) {
  auto right_type = right->dtype();
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_pow(right2);
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_pow(right2);
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_pow(right2);
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_pow(right2);
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_pow(right2);
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_pow(right2);
  } else throw std::runtime_error("pow: unsupported type: " + right_type);
}

template <class T>
template <class U>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::typed_pow(const std::shared_ptr<typed_dfcolumn<U>>& right) {
  T a;
  U b;
  typedef decltype(std::pow(a,b)) V;
  if(contain_nulls) val.mapv(clear_null<T>, nulls);
  if(right->contain_nulls) right->val.mapv(clear_null<U>, right->nulls);
  auto newval =
    val.map(+[](const std::vector<T>& left, 
                const std::vector<U>& right) {
        return vector_pow(left, right);
      }, right->val);
  if(contain_nulls) val.mapv(reset_null<T>, nulls);
  if(right->contain_nulls) right->val.mapv(reset_null<U>, right->nulls);
  if (contain_nulls || right->contain_nulls) {
    auto newnulls = nulls.map(set_union<size_t>, right->nulls);
    newval.mapv(reset_null<V>, newnulls);
    return std::make_shared<typed_dfcolumn<V>>(std::move(newval), 
                                               std::move(newnulls));
  } else {
    auto dvval = newval.template moveto_dvector<V>();
    return std::make_shared<typed_dfcolumn<V>>(std::move(dvval));
  }
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::pow_im(const std::shared_ptr<dfscalar>& right) {
  auto right_type = right->dtype();
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_pow_im(right2);
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_pow_im(right2);
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_pow_im(right2);
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_pow_im(right2);
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_pow_im(right2);
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_pow_im(right2);
  } else throw std::runtime_error("pow_im: unsupported type: " + right_type);
}

template <class T>
template <class U>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::typed_pow_im
(const std::shared_ptr<typed_dfscalar<U>>& right) {
  T a;
  U b;
  typedef decltype(std::pow(a,b)) V;
  if(contain_nulls) val.mapv(clear_null<T>, nulls);
  auto bcast_right = broadcast(right->val);
  auto newval =
    val.map(+[](const std::vector<T>& left, U right) {
        return vector_pow(left, right);
      }, bcast_right);
  if (contain_nulls) {
    newval.mapv(reset_null<V>, nulls);
    return std::make_shared<typed_dfcolumn<V>>(std::move(newval), nulls);
  } else {
    auto dvval = newval.template moveto_dvector<V>();
    return std::make_shared<typed_dfcolumn<V>>(std::move(dvval));
  }
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::rpow_im(const std::shared_ptr<dfscalar>& right) {
  auto right_type = right->dtype();
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_rpow_im(right2);
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_rpow_im(right2);
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_rpow_im(right2);
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_rpow_im(right2);
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_rpow_im(right2);
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    return typed_rpow_im(right2);
  } else throw std::runtime_error("rpow_im: unsupported type: " + right_type);
}

template <class T>
template <class U>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::typed_rpow_im
(const std::shared_ptr<typed_dfscalar<U>>& right) {
  T a;
  U b;
  typedef decltype(std::pow(a,b)) V;
  if(contain_nulls) val.mapv(clear_null<T>, nulls);
  auto bcast_right = broadcast(right->val);
  auto newval =
    val.map(+[](const std::vector<T>& left, U right) {
        return vector_pow(right, left);
      }, bcast_right);
  if (contain_nulls) {
    newval.mapv(reset_null<V>, nulls);
    return std::make_shared<typed_dfcolumn<V>>(std::move(newval), nulls);
  } else {
    auto dvval = newval.template moveto_dvector<V>();
    return std::make_shared<typed_dfcolumn<V>>(std::move(dvval));
  }
}

template <class T>
struct abs_helper_signed_struct {
  void operator()(const T* srcp, T* retp, size_t size) {
    for(size_t i = 0; i < size; i++) {
      retp[i] = srcp[i] >= 0 ? srcp[i] : -srcp[i];
    }
  }
};

template <class T>
struct abs_helper_unsigned_struct {
  void operator()(const T* srcp, T* retp, size_t size) {
    for(size_t i = 0; i < size; i++) {
      retp[i] = srcp[i];
    }
  }
};

// to disable compier warning of comparing unsigned < 0
template <class T>
void abs_helper(const T* srcp, T* retp, size_t size) {
  typename std::conditional
    <std::numeric_limits<T>::is_signed,
     abs_helper_signed_struct<T>,
     abs_helper_unsigned_struct<T>>::type abs_helper_func;
  return abs_helper_func(srcp, retp, size);
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::abs() {
  auto newval =
    val.map(+[](const std::vector<T>& left) {
        auto leftp = left.data();
        auto size = left.size();
        std::vector<T> ret(size);
        auto retp = ret.data();
        abs_helper(leftp, retp, size);
        return ret;
      });
  if(contain_nulls) {
/* // abs does not need to update null value
    newval.mapv(+[](std::vector<T>& val, const std::vector<size_t>& nulls) {
        auto valp = val.data();
        auto nullsp = nulls.data();
        auto size = nulls.size();
        auto max = std::numeric_limits<T>::max();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
        for(size_t i = 0; i < size; i++) {
          valp[nullsp[i]] = max;
        }
      }, nulls);
*/
    return std::make_shared<typed_dfcolumn<T>>(std::move(newval), nulls);
  } else {
    auto dvval = newval.template moveto_dvector<T>();
    return std::make_shared<typed_dfcolumn<T>>(std::move(dvval));
  }
}


template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::type_cast(const std::string& to_type,
                               bool check_bool_like);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::type_cast(const std::string& to_type,
                                        bool check_bool_like);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::type_cast(const std::string& to_type,
                                bool check_bool_like);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::type_cast(const std::string& to_type,
                                         bool check_bool_like);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::type_cast(const std::string& to_type,
                                 bool check_bool_like);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::type_cast(const std::string& to_type,
                                  bool check_bool_like);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::add(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::add(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::add(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::add(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::add(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::add(const std::shared_ptr<dfcolumn>& right);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::add_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::add_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::add_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::add_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::add_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::add_im(const std::shared_ptr<dfscalar>& right);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::sub(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::sub(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::sub(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::sub(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::sub(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::sub(const std::shared_ptr<dfcolumn>& right);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::sub_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::sub_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::sub_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::sub_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::sub_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::sub_im(const std::shared_ptr<dfscalar>& right);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::rsub_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::rsub_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::rsub_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::rsub_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::rsub_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::rsub_im(const std::shared_ptr<dfscalar>& right);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::mul(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::mul(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::mul(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::mul(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::mul(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::mul(const std::shared_ptr<dfcolumn>& right);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::mul_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::mul_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::mul_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::mul_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::mul_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::mul_im(const std::shared_ptr<dfscalar>& right);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::fdiv(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::fdiv(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::fdiv(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::fdiv(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::fdiv(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::fdiv(const std::shared_ptr<dfcolumn>& right);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::fdiv_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::fdiv_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::fdiv_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::fdiv_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::fdiv_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::fdiv_im(const std::shared_ptr<dfscalar>& right);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::rfdiv_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::rfdiv_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::rfdiv_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::rfdiv_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::rfdiv_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::rfdiv_im(const std::shared_ptr<dfscalar>& right);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::idiv(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::idiv(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::idiv(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::idiv(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::idiv(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::idiv(const std::shared_ptr<dfcolumn>& right);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::idiv_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::idiv_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::idiv_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::idiv_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::idiv_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::idiv_im(const std::shared_ptr<dfscalar>& right);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::ridiv_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::ridiv_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::ridiv_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::ridiv_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::ridiv_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::ridiv_im(const std::shared_ptr<dfscalar>& right);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::mod(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::mod(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::mod(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::mod(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::mod(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::mod(const std::shared_ptr<dfcolumn>& right);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::mod_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::mod_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::mod_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::mod_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::mod_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::mod_im(const std::shared_ptr<dfscalar>& right);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::rmod_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::rmod_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::rmod_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::rmod_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::rmod_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::rmod_im(const std::shared_ptr<dfscalar>& right);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::pow(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::pow(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::pow(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::pow(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::pow(const std::shared_ptr<dfcolumn>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::pow(const std::shared_ptr<dfcolumn>& right);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::pow_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::pow_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::pow_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::pow_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::pow_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::pow_im(const std::shared_ptr<dfscalar>& right);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::rpow_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::rpow_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::rpow_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::rpow_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::rpow_im(const std::shared_ptr<dfscalar>& right);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::rpow_im(const std::shared_ptr<dfscalar>& right);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::abs();
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::abs();
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::abs();
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::abs();
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::abs();
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::abs();


// ----- dic_string -----

std::shared_ptr<dfcolumn>
typed_dfcolumn<dic_string>::type_cast(const std::string& to_type,
                                      bool check_bool_like) {
  std::shared_ptr<dfcolumn> ret;
  auto newnulls = nulls; // copy to move
  if(to_type == "boolean") {
    if (check_bool_like) {
      auto ddic = dic->decompress();
      auto b_words_to_bool_map = broadcast(words_to_bool(ddic));
      auto newcol = b_words_to_bool_map.map(vector_take<int,int>, val);
      // TODO: should treat nulls as true?
      ret = std::make_shared<typed_dfcolumn<int>>
        (std::move(newcol), std::move(newnulls));
    } else {
      // True if non-empty string
      auto newcol = as_words().map(+[](const words& ws) {
          auto lensp = ws.lens.data();
          auto lens_size = ws.lens.size();
          std::vector<int> ret(lens_size);
          auto retp = ret.data();
          for(size_t i = 0; i < lens_size; i++) retp[i] = (lensp[i] != 0);
          return ret;
        }).template moveto_dvector<int>();
      // nulls would also be treated as true, hence no nulls in casted column
      ret = std::make_shared<typed_dfcolumn<int>>(std::move(newcol));
    }
  } else if(to_type == "int") {
    auto newval = as_words(6,"%Y-%m-%d",false,"0").
      map(+[](const words& ws){return parsenumber<int>(ws);});
    ret = std::make_shared<typed_dfcolumn<int>>
      (std::move(newval), std::move(newnulls));
  } else if(to_type == "unsigned int") {
    auto newval = as_words(6,"%Y-%m-%d",false,"0").
      map(+[](const words& ws){return parsenumber<unsigned int>(ws);});
    ret = std::make_shared<typed_dfcolumn<unsigned int>>
      (std::move(newval), std::move(newnulls));
  } else if(to_type == "long") {
    auto newval = as_words(6,"%Y-%m-%d",false,"0").
      map(+[](const words& ws){return parsenumber<long>(ws);});
    ret = std::make_shared<typed_dfcolumn<long>>
      (std::move(newval), std::move(newnulls));
  } else if(to_type == "unsigned long") {
    auto newval = as_words(6,"%Y-%m-%d",false,"0").
      map(+[](const words& ws){return parsenumber<unsigned long>(ws);});
    ret = std::make_shared<typed_dfcolumn<unsigned long>>
      (std::move(newval), std::move(newnulls));
  } else if(to_type == "float") {
    auto newval = as_words(6,"%Y-%m-%d",false,"0").
      map(+[](const words& ws){return parsenumber<float>(ws);});
    ret = std::make_shared<typed_dfcolumn<float>>
      (std::move(newval), std::move(newnulls));
  } else if(to_type == "double") {
    auto newval = as_words(6,"%Y-%m-%d",false,"0").
      map(+[](const words& ws){return parsenumber<double>(ws);});
    ret = std::make_shared<typed_dfcolumn<double>>
      (std::move(newval), std::move(newnulls));
  } else if(to_type == "string") {
    auto newval = as_words().map(+[](const words& ws)
                                 {return words_to_vector_string(ws);});
    ret = std::make_shared<typed_dfcolumn<std::string>>
      (std::move(newval), std::move(newnulls));
  } else if(to_type == "dic_string") {
    ret = std::make_shared<typed_dfcolumn<dic_string>>(*this);
  } else if(to_type == "raw_string") {
    auto newval = as_words();
    ret = std::make_shared<typed_dfcolumn<raw_string>>
      (std::move(newval), std::move(newnulls));
  } else if(to_type == "datetime") { 
    auto clen = first(true).length();
    std::string fmt;
    if (clen == 10) fmt = "%Y-%m-%d";
    else if (clen == 11) fmt = "%Y-%b-%d";
    else if (clen == 19) fmt = "%Y-%m-%d %H-%M-%S";
    else if (clen == 20) fmt = "%Y-%b-%d %H-%M-%S";
    else throw std::runtime_error(
    "unknown format for string -> datetime conversion!");
    // create safe NULL string
    auto nullstr_size = fmt.size();
    if(fmt.find("%Y") != std::string::npos) nullstr_size += 2;
    if(fmt.find("%b") != std::string::npos) nullstr_size += 1;
    std::string nullstr(nullstr_size, '0');
    auto newval = as_words(6,"%Y-%m-%d",false,nullstr).
      map(+[](const words& ws, const std::string& fmt)
          {return parsedatetime(ws, fmt);}, broadcast(fmt));
    ret = std::make_shared<typed_dfcolumn<datetime>>
      (std::move(newval), std::move(newnulls));
  } else if(to_type.find("datetime:") == 0) {
    auto fmt = to_type.substr(9);
    // create safe NULL string
    auto nullstr_size = fmt.size();
    if(fmt.find("%Y") != std::string::npos) nullstr_size += 2;
    if(fmt.find("%b") != std::string::npos) nullstr_size += 1;
    std::string nullstr(nullstr_size, '0');
    auto newval = as_words(6,"%Y-%m-%d",false,nullstr).
      map(+[](const words& ws, const std::string& fmt)
          {return parsedatetime(ws, fmt);}, broadcast(fmt));
    ret = std::make_shared<typed_dfcolumn<datetime>>
      (std::move(newval), std::move(newnulls));
  } else {
    throw std::runtime_error
      ("dic_string column doesn't support casting to: " + to_type);
  }
  return ret;
}


// ----- raw_string -----
std::shared_ptr<dfcolumn>
typed_dfcolumn<raw_string>::type_cast(const std::string& to_type,
                                      bool check_bool_like) {
  std::shared_ptr<dfcolumn> ret;
  auto newnulls = nulls; // copy to move
  if(to_type == "boolean") {
    if (check_bool_like) {
      auto ws = as_words();
      auto newcol = ws.map(+[](words& ws){return words_to_bool(ws);});
      // TODO: should treat nulls as true?
      ret = std::make_shared<typed_dfcolumn<int>>
        (std::move(newcol), std::move(newnulls));
    } else {
      // True if non-empty string
      auto newcol = as_words().map(+[](const words& ws) {
          auto lensp = ws.lens.data();
          auto lens_size = ws.lens.size();
          std::vector<int> ret(lens_size);
          auto retp = ret.data();
          for(size_t i = 0; i < lens_size; i++) retp[i] = (lensp[i] != 0);
          return ret;
        }).template moveto_dvector<int>();
      // nulls would also be treated as true, hence no nulls in casted column
      ret = std::make_shared<typed_dfcolumn<int>>(std::move(newcol));
    }
  } else if(to_type == "int") {
    auto newval = as_words(6,"%Y-%m-%d",false,"0").
      map(+[](const words& ws){return parsenumber<int>(ws);});
    ret = std::make_shared<typed_dfcolumn<int>>
      (std::move(newval), std::move(newnulls));
  } else if(to_type == "unsigned int") {
    auto newval = as_words(6,"%Y-%m-%d",false,"0").
      map(+[](const words& ws){return parsenumber<unsigned int>(ws);});
    ret = std::make_shared<typed_dfcolumn<unsigned int>>
      (std::move(newval), std::move(newnulls));
  } else if(to_type == "long") {
    auto newval = as_words(6,"%Y-%m-%d",false,"0").
      map(+[](const words& ws){return parsenumber<long>(ws);});
    ret = std::make_shared<typed_dfcolumn<long>>
      (std::move(newval), std::move(newnulls));
  } else if(to_type == "unsigned long") {
    auto newval = as_words(6,"%Y-%m-%d",false,"0").
      map(+[](const words& ws){return parsenumber<unsigned long>(ws);});
    ret = std::make_shared<typed_dfcolumn<unsigned long>>
      (std::move(newval), std::move(newnulls));
  } else if(to_type == "float") {
    auto newval = as_words(6,"%Y-%m-%d",false,"0").
      map(+[](const words& ws){return parsenumber<float>(ws);});
    ret = std::make_shared<typed_dfcolumn<float>>
      (std::move(newval), std::move(newnulls));
  } else if(to_type == "double") {
    auto newval = as_words(6,"%Y-%m-%d",false,"0").
      map(+[](const words& ws){return parsenumber<double>(ws);});
    ret = std::make_shared<typed_dfcolumn<double>>
      (std::move(newval), std::move(newnulls));
  } else if(to_type == "string") {
    auto newval = as_words().map(+[](const words& ws)
                                 {return words_to_vector_string(ws);});
    ret = std::make_shared<typed_dfcolumn<std::string>>
      (std::move(newval), std::move(newnulls));
  } else if(to_type == "dic_string") {
    auto newval = as_words();
    ret = std::make_shared<typed_dfcolumn<dic_string>>
      (std::move(newval), std::move(newnulls));
  } else if(to_type == "raw_string") {
    ret = std::make_shared<typed_dfcolumn<raw_string>>(*this);
  } else if(to_type == "datetime") { 
    auto clen = first(true).length();
    std::string fmt;
    if (clen == 10) fmt = "%Y-%m-%d";
    else if (clen == 11) fmt = "%Y-%b-%d";
    else if (clen == 19) fmt = "%Y-%m-%d %H-%M-%S";
    else if (clen == 20) fmt = "%Y-%b-%d %H-%M-%S";
    else throw std::runtime_error(
    "unknown format for string -> datetime conversion!");
    // create safe NULL string
    auto nullstr_size = fmt.size();
    if(fmt.find("%Y") != std::string::npos) nullstr_size += 2;
    if(fmt.find("%b") != std::string::npos) nullstr_size += 1;
    std::string nullstr(nullstr_size, '0');
    auto newval = as_words(6,"%Y-%m-%d",false,nullstr).
      map(+[](const words& ws, const std::string& fmt)
          {return parsedatetime(ws, fmt);}, broadcast(fmt));
    ret = std::make_shared<typed_dfcolumn<datetime>>
      (std::move(newval), std::move(newnulls));
  } else if(to_type.find("datetime:") == 0) {
    auto fmt = to_type.substr(9);
    // create safe NULL string
    auto nullstr_size = fmt.size();
    if(fmt.find("%Y") != std::string::npos) nullstr_size += 2;
    if(fmt.find("%b") != std::string::npos) nullstr_size += 1;
    std::string nullstr(nullstr_size, '0');
    auto newval = as_words(6,"%Y-%m-%d",false,nullstr).
      map(+[](const words& ws, const std::string& fmt)
          {return parsedatetime(ws, fmt);}, broadcast(fmt));
    ret = std::make_shared<typed_dfcolumn<datetime>>
      (std::move(newval), std::move(newnulls));
  } else {
    throw std::runtime_error
      ("raw_string column doesn't support casting to: " + to_type);
  }
  return ret;
}

// ----- string -----

vector<size_t> to_contiguous_idx(vector<size_t>& idx, vector<size_t>& sizes);

std::shared_ptr<dfcolumn>
typed_dfcolumn<std::string>::type_cast(const std::string& to_type,
                                       bool check_bool_like) {
  std::shared_ptr<dfcolumn> ret;
  auto newnulls = nulls; // copy to move
  if(to_type == "boolean") {
    if (check_bool_like) {
      auto dic_dv = dic_idx->viewas_dvector<std::string>();
      auto dicsizes = dic_dv.sizes();
      auto sdic = dic_dv.gather();
      auto tmp_dic = vector_string_to_words(sdic);
      auto idx = val.map(to_contiguous_idx, broadcast(dicsizes));
      auto b_words_to_bool_map = broadcast(words_to_bool(tmp_dic));
      auto newcol = b_words_to_bool_map.map(vector_take<int,int>, idx);
      // TODO: should treat nulls as true?
      ret = std::make_shared<typed_dfcolumn<int>>(std::move(newcol), nulls);
    } else {
      // True if non-empty string
      auto newcol = get_val().map(+[](const std::vector<std::string>& vec) {
                      auto sz = vec.size();
                      std::vector<int> ret(sz); 
                      for(size_t i = 0; i < sz; ++i) ret[i] = vec[i] != "";
                      return ret;
                    }).template moveto_dvector<int>();
      // nulls would also be treated as true, hence no nulls in casted column
      ret = std::make_shared<typed_dfcolumn<int>>(std::move(newcol));
    }
  } else if(to_type == "int") {
    auto newval = as_words(6,"%Y-%m-%d",false,"0").
      map(+[](const words& ws){return parsenumber<int>(ws);});
    ret = std::make_shared<typed_dfcolumn<int>>
      (std::move(newval), std::move(newnulls));
  } else if(to_type == "unsigned int") {
    auto newval = as_words(6,"%Y-%m-%d",false,"0").
      map(+[](const words& ws){return parsenumber<unsigned int>(ws);});
    ret = std::make_shared<typed_dfcolumn<unsigned int>>
      (std::move(newval), std::move(newnulls));
  } else if(to_type == "long") {
    auto newval = as_words(6,"%Y-%m-%d",false,"0").
      map(+[](const words& ws){return parsenumber<long>(ws);});
    ret = std::make_shared<typed_dfcolumn<long>>
      (std::move(newval), std::move(newnulls));
  } else if(to_type == "unsigned long") {
    auto newval = as_words(6,"%Y-%m-%d",false,"0").
      map(+[](const words& ws){return parsenumber<unsigned long>(ws);});
    ret = std::make_shared<typed_dfcolumn<unsigned long>>
      (std::move(newval), std::move(newnulls));
  } else if(to_type == "float") {
    auto newval = as_words(6,"%Y-%m-%d",false,"0").
      map(+[](const words& ws){return parsenumber<float>(ws);});
    ret = std::make_shared<typed_dfcolumn<float>>
      (std::move(newval), std::move(newnulls));
  } else if(to_type == "double") {
    auto newval = as_words(6,"%Y-%m-%d",false,"0").
      map(+[](const words& ws){return parsenumber<double>(ws);});
    ret = std::make_shared<typed_dfcolumn<double>>
      (std::move(newval), std::move(newnulls));
  } else if(to_type == "string") {
    ret = std::make_shared<typed_dfcolumn<std::string>>(*this);
  } else if(to_type == "dic_string") {
    auto newval = as_words();
    ret = std::make_shared<typed_dfcolumn<dic_string>>
      (std::move(newval), std::move(newnulls));
  } else if(to_type == "raw_string") {
    auto newval = as_words();
    ret = std::make_shared<typed_dfcolumn<raw_string>>
      (std::move(newval), std::move(newnulls));
  } else if(to_type == "datetime") { 
    auto clen = first(true).length();
    std::string fmt;
    if (clen == 10) fmt = "%Y-%m-%d";
    else if (clen == 11) fmt = "%Y-%b-%d";
    else if (clen == 19) fmt = "%Y-%m-%d %H-%M-%S";
    else if (clen == 20) fmt = "%Y-%b-%d %H-%M-%S";
    else throw std::runtime_error(
    "unknown format for string -> datetime conversion!");
    // create safe NULL string
    auto nullstr_size = fmt.size();
    if(fmt.find("%Y") != std::string::npos) nullstr_size += 2;
    if(fmt.find("%b") != std::string::npos) nullstr_size += 1;
    std::string nullstr(nullstr_size, '0');
    auto newval = as_words(6,"%Y-%m-%d",false,nullstr).
      map(+[](const words& ws, const std::string& fmt)
          {return parsedatetime(ws, fmt);}, broadcast(fmt));
    ret = std::make_shared<typed_dfcolumn<datetime>>
      (std::move(newval), std::move(newnulls));
  } else if(to_type.find("datetime:") == 0) {
    auto fmt = to_type.substr(9);
    // create safe NULL string
    auto nullstr_size = fmt.size();
    if(fmt.find("%Y") != std::string::npos) nullstr_size += 2;
    if(fmt.find("%b") != std::string::npos) nullstr_size += 1;
    std::string nullstr(nullstr_size, '0');
    auto newval = as_words(6,"%Y-%m-%d",false,nullstr).
      map(+[](const words& ws, const std::string& fmt)
          {return parsedatetime(ws, fmt);}, broadcast(fmt));
    ret = std::make_shared<typed_dfcolumn<datetime>>
      (std::move(newval), std::move(newnulls));
  } else {
    throw std::runtime_error
      ("string column doesn't support casting to: " + to_type);
  }
  return ret;
}

// datetime operations are defined in typed_dfcolumn_datetime.cc

}
