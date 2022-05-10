#include "dfcolumn.hpp"

#include <limits>
#include <climits>
#include "../core/find_condition.hpp"
#include "dfcolumn_helper.hpp"

using namespace std;

namespace frovedis {

template <class T, class U>
struct filter_eq_pred {
  int operator()(T a, U b) const {return a == b;}
};

template <class T, class U>
std::vector<size_t> filter_eq_helper(std::vector<T>& left,
                                     std::vector<U>& right) {
  return find_condition_pair(left, right, filter_eq_pred<T,U>());
}

template <class T, class U>
struct filter_eq_immed_pred {
  filter_eq_immed_pred(U c) : c(c) {}
  int operator()(T a) const {return a == c;}
  U c;
};

template <class T, class U>
std::vector<size_t> filter_eq_immed_helper(std::vector<T>& left,
                                           U right) {
  return find_condition(left, filter_eq_immed_pred<T,U>(right));
}

template <class T, class U>
struct filter_neq_pred {
  int operator()(T a, U b) const {return a != b;}
};

template <class T, class U>
std::vector<size_t> filter_neq_helper(std::vector<T>& left,
                                      std::vector<U>& right) {
  return find_condition_pair(left, right, filter_neq_pred<T,U>());
}

template <class T, class U>
struct filter_neq_immed_pred {
  filter_neq_immed_pred(U c) : c(c) {}
  int operator()(T a) const {return a != c;}
  U c;
};

template <class T, class U>
std::vector<size_t> filter_neq_immed_helper(std::vector<T>& left,
                                            U right) {
  return find_condition(left, filter_neq_immed_pred<T,U>(right));
}

// no need to check right null, since it is checked by left if they are equal
template <class T>
node_local<std::vector<size_t>>
typed_dfcolumn<T>::filter_eq(std::shared_ptr<dfcolumn>& right) {
  auto right_type = right->dtype();
  node_local<std::vector<size_t>> filtered_idx;
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_eq_helper<T,double>, right2->val);
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_eq_helper<T,float>, right2->val);
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_eq_helper<T,long>, right2->val);
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_eq_helper<T,unsigned long>, right2->val);
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_eq_helper<T,int>, right2->val);
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_eq_helper<T,unsigned int>, right2->val);
  } else throw std::runtime_error("unsupported type: " + right_type);
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

template <class T>
node_local<std::vector<size_t>>
typed_dfcolumn<T>::filter_eq_immed(std::shared_ptr<dfscalar>& right) {
  auto right_type = right->dtype();
  node_local<std::vector<size_t>> filtered_idx;
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_eq_immed_helper<T,double>,
                           broadcast(right2->val));
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_eq_immed_helper<T,float>,
                           broadcast(right2->val));
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_eq_immed_helper<T,long>,
                           broadcast(right2->val));
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_eq_immed_helper<T,unsigned long>,
                           broadcast(right2->val));
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_eq_immed_helper<T,int>,
                           broadcast(right2->val));
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_eq_immed_helper<T,unsigned int>,
                           broadcast(right2->val));
  } else throw std::runtime_error("unsupported type: " + right_type);
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

template <class T>
node_local<std::vector<size_t>>
typed_dfcolumn<T>::filter_neq(std::shared_ptr<dfcolumn>& right) {
  auto right_type = right->dtype();
  node_local<std::vector<size_t>> filtered_idx;
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_neq_helper<T,double>, right2->val);
    if(right2->contain_nulls)
      filtered_idx = filtered_idx.map(set_difference<size_t>, right2->nulls);
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_neq_helper<T,float>, right2->val);
    if(right2->contain_nulls)
      filtered_idx = filtered_idx.map(set_difference<size_t>, right2->nulls);
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_neq_helper<T,long>, right2->val);
    if(right2->contain_nulls)
      filtered_idx = filtered_idx.map(set_difference<size_t>, right2->nulls);
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_neq_helper<T,unsigned long>, right2->val);
    if(right2->contain_nulls)
      filtered_idx = filtered_idx.map(set_difference<size_t>, right2->nulls);
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_neq_helper<T,int>, right2->val);
    if(right2->contain_nulls)
      filtered_idx = filtered_idx.map(set_difference<size_t>, right2->nulls);
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_neq_helper<T,unsigned int>, right2->val);
    if(right2->contain_nulls)
      filtered_idx = filtered_idx.map(set_difference<size_t>, right2->nulls);
  } else throw std::runtime_error("unsupported type: " + right_type);
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

template <class T>
node_local<std::vector<size_t>>
typed_dfcolumn<T>::filter_neq_immed(std::shared_ptr<dfscalar>& right) {
  auto right_type = right->dtype();
  node_local<std::vector<size_t>> filtered_idx;
  if(right_type == "double") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<double>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_neq_immed_helper<T,double>,
                           broadcast(right2->val));
  } else if (right_type == "float") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<float>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_neq_immed_helper<T,float>,
                           broadcast(right2->val));
  } else if(right_type == "long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<long>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_neq_immed_helper<T,long>,
                           broadcast(right2->val));
  } else if(right_type == "unsigned long") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned long>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_neq_immed_helper<T,unsigned long>,
                           broadcast(right2->val));
  } else if(right_type == "int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<int>>(right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_neq_immed_helper<T,int>,
                           broadcast(right2->val));
  } else if(right_type == "unsigned int") {
    auto right2 = std::dynamic_pointer_cast<typed_dfscalar<unsigned int>>
      (right);
    if(!static_cast<bool>(right2))
      throw std::runtime_error("internal type error");
    filtered_idx = val.map(filter_neq_immed_helper<T,unsigned int>,
                           broadcast(right2->val));
  } else throw std::runtime_error("unsupported type: " + right_type);
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

template node_local<std::vector<size_t>>
typed_dfcolumn<int>::filter_eq(std::shared_ptr<dfcolumn>& right);
template node_local<std::vector<size_t>>
typed_dfcolumn<int>::filter_eq_immed(std::shared_ptr<dfscalar>& right);
template node_local<std::vector<size_t>>
typed_dfcolumn<int>::filter_neq(std::shared_ptr<dfcolumn>& right);
template node_local<std::vector<size_t>>
typed_dfcolumn<int>::filter_neq_immed(std::shared_ptr<dfscalar>& right);

template node_local<std::vector<size_t>>
typed_dfcolumn<unsigned int>::filter_eq(std::shared_ptr<dfcolumn>& right);
template node_local<std::vector<size_t>>
typed_dfcolumn<unsigned int>::filter_eq_immed(std::shared_ptr<dfscalar>& right);
template node_local<std::vector<size_t>>
typed_dfcolumn<unsigned int>::filter_neq(std::shared_ptr<dfcolumn>& right);
template node_local<std::vector<size_t>>
typed_dfcolumn<unsigned int>::filter_neq_immed(std::shared_ptr<dfscalar>& right);

template node_local<std::vector<size_t>>
typed_dfcolumn<long>::filter_eq(std::shared_ptr<dfcolumn>& right);
template node_local<std::vector<size_t>>
typed_dfcolumn<long>::filter_eq_immed(std::shared_ptr<dfscalar>& right);
template node_local<std::vector<size_t>>
typed_dfcolumn<long>::filter_neq(std::shared_ptr<dfcolumn>& right);
template node_local<std::vector<size_t>>
typed_dfcolumn<long>::filter_neq_immed(std::shared_ptr<dfscalar>& right);

template node_local<std::vector<size_t>>
typed_dfcolumn<unsigned long>::filter_eq(std::shared_ptr<dfcolumn>& right);
template node_local<std::vector<size_t>>
typed_dfcolumn<unsigned long>::filter_eq_immed(std::shared_ptr<dfscalar>& right);
template node_local<std::vector<size_t>>
typed_dfcolumn<unsigned long>::filter_neq(std::shared_ptr<dfcolumn>& right);
template node_local<std::vector<size_t>>
typed_dfcolumn<unsigned long>::filter_neq_immed(std::shared_ptr<dfscalar>& right);

template node_local<std::vector<size_t>>
typed_dfcolumn<float>::filter_eq(std::shared_ptr<dfcolumn>& right);
template node_local<std::vector<size_t>>
typed_dfcolumn<float>::filter_eq_immed(std::shared_ptr<dfscalar>& right);
template node_local<std::vector<size_t>>
typed_dfcolumn<float>::filter_neq(std::shared_ptr<dfcolumn>& right);
template node_local<std::vector<size_t>>
typed_dfcolumn<float>::filter_neq_immed(std::shared_ptr<dfscalar>& right);

template node_local<std::vector<size_t>>
typed_dfcolumn<double>::filter_eq(std::shared_ptr<dfcolumn>& right);
template node_local<std::vector<size_t>>
typed_dfcolumn<double>::filter_eq_immed(std::shared_ptr<dfscalar>& right);
template node_local<std::vector<size_t>>
typed_dfcolumn<double>::filter_neq(std::shared_ptr<dfcolumn>& right);
template node_local<std::vector<size_t>>
typed_dfcolumn<double>::filter_neq_immed(std::shared_ptr<dfscalar>& right);

// ----- dic_string -----
std::vector<size_t>
dic_string_equal_prepare_helper(const std::vector<size_t>& rval,
                                const std::vector<size_t>& trans) {
  auto rval_size = rval.size();
  std::vector<size_t> ret(rval_size);
  auto retp = ret.data();
  auto rvalp = rval.data();
  auto transp = trans.data();
  auto NOT_FOUND_TRANS = numeric_limits<size_t>::max();
  auto nullvalue = numeric_limits<size_t>::max();
  auto NOT_FOUND = NOT_FOUND_TRANS - 1;
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < rval_size; i++) {
    if(rvalp[i] == nullvalue) retp[i] = nullvalue;
    else {
      auto v = transp[rvalp[i]];
      if(v == NOT_FOUND_TRANS) retp[i] = NOT_FOUND; // to distinguish from NULL
      else retp[i] = v;
    }
  }
  return ret;
}

node_local<vector<size_t>>
typed_dfcolumn<dic_string>::
equal_prepare(shared_ptr<typed_dfcolumn<dic_string>>& right) {
  if(dic == right->dic) {
    return right->val;
  } else {
    auto& left_dic = *dic;
    auto& right_dic = *(right->dic);
    compressed_words to_lookup;
    auto right_num_words = right_dic.num_words();
    to_lookup.cwords.swap(right_dic.cwords);
    to_lookup.lens.swap(right_dic.lens);
    to_lookup.lens_num.swap(right_dic.lens_num);
    to_lookup.order.resize(right_num_words);
    auto orderp = to_lookup.order.data();
    for(size_t i = 0; i < right_num_words; i++) orderp[i] = i;
    auto trans_table = broadcast(left_dic.lookup(to_lookup));
    to_lookup.cwords.swap(right_dic.cwords);
    to_lookup.lens.swap(right_dic.lens);
    to_lookup.lens_num.swap(right_dic.lens_num);
    return right->val.map(dic_string_equal_prepare_helper, trans_table);
  }
}

node_local<std::vector<size_t>>
typed_dfcolumn<dic_string>::filter_eq(std::shared_ptr<dfcolumn>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<dic_string>>(right);
  if(!static_cast<bool>(right2)) throw std::runtime_error("filter_eq: column types are different");
  auto rightval = equal_prepare(right2);
  auto filtered_idx = val.map(filter_eq_helper<size_t, size_t>, rightval);
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

node_local<std::vector<size_t>>
typed_dfcolumn<dic_string>::filter_neq(std::shared_ptr<dfcolumn>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<dic_string>>(right);
  if(!static_cast<bool>(right2)) throw std::runtime_error("filter_eq: column types are different");
  auto rightval = equal_prepare(right2);
  auto filtered_idx = val.map(filter_neq_helper<size_t, size_t>, rightval);
  if(right2->contain_nulls)
    filtered_idx = filtered_idx.map(set_difference<size_t>, right2->nulls);
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

node_local<std::vector<size_t>>
typed_dfcolumn<dic_string>::filter_eq_immed(std::shared_ptr<dfscalar>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfscalar<std::string>>(right);
  if(!static_cast<bool>(right2))
    throw std::runtime_error("filter string column with non string");
  auto rightval = right2->val;
  words right_word;
  right_word.chars = char_to_int(rightval);
  right_word.starts = {0};
  right_word.lens = {rightval.size()};
  auto right_cword = make_compressed_words(right_word);
  auto lookedup = dic->lookup(right_cword);
  if(lookedup[0] != numeric_limits<size_t>::max()) {
    return val.map
      (+[](std::vector<size_t>& val, const std::vector<size_t>& nulls,
           bool contain_nulls, size_t lookedup) {
        auto filtered_idx =
          filter_eq_immed_helper<size_t,size_t>(val, lookedup);
        if(contain_nulls) return set_difference(filtered_idx, nulls);
        else return filtered_idx;
      }, nulls, broadcast(contain_nulls), broadcast(lookedup[0]));
  } else {
    return make_node_local_allocate<std::vector<size_t>>();
  }
}

node_local<std::vector<size_t>>
typed_dfcolumn<dic_string>::filter_neq_immed(std::shared_ptr<dfscalar>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfscalar<std::string>>(right);
  if(!static_cast<bool>(right2))
    throw std::runtime_error("filter string column with non string");
  auto rightval = right2->val;
  words right_word;
  right_word.chars = char_to_int(rightval);
  right_word.starts = {0};
  right_word.lens = {rightval.size()};
  auto right_cword = make_compressed_words(right_word);
  auto lookedup = dic->lookup(right_cword);
  if(lookedup[0] != numeric_limits<size_t>::max()) {
    return val.map
      (+[](std::vector<size_t>& val, const std::vector<size_t>& nulls,
           bool contain_nulls, size_t lookedup) {
        auto filtered_idx =
          filter_neq_immed_helper<size_t,size_t>(val, lookedup);
        if(contain_nulls) return set_difference(filtered_idx, nulls);
        else return filtered_idx;
      }, nulls, broadcast(contain_nulls), broadcast(lookedup[0]));
  } else {
    return make_node_local_allocate<std::vector<size_t>>();
  }
}

// ----- raw_string -----
// TODO: efficient implementation of filter_[n]eq_immed
node_local<std::vector<size_t>>
typed_dfcolumn<raw_string>::filter_eq_immed(std::shared_ptr<dfscalar>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfscalar<std::string>>(right);
  if(!static_cast<bool>(right2))
    throw std::runtime_error("filter raw_string with non string");
  return filter_like(right2->val,0);
}

node_local<std::vector<size_t>>
typed_dfcolumn<raw_string>::filter_neq_immed(std::shared_ptr<dfscalar>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfscalar<std::string>>(right);
  if(!static_cast<bool>(right2))
    throw std::runtime_error("filter raw_string with non string");
  return filter_not_like(right2->val,0);
}


// ----- string -----
void create_string_trans_table(my_map<string,size_t>& leftdic,
                               my_map<string,size_t>& rightdic,
                               vector<size_t>& from,
                               vector<size_t>& to) {
  auto maxsize = rightdic.size();
  from.reserve(maxsize);
  to.reserve(maxsize);
  for(auto it = rightdic.begin(); it != rightdic.end(); ++it) {
    auto f = leftdic.find(it->first);
    if(f != leftdic.end()) {
      from.push_back(it->second);
      to.push_back(f->second);
    }
  }
}

vector<size_t> equal_prepare_helper(vector<size_t>& val,
                                    vector<size_t>& from,
                                    vector<size_t>& to) {
  auto ht = unique_hashtable<size_t, size_t>(from, to);
  vector<size_t> missed;
  auto ret = ht.lookup(val, missed);
  size_t* retp = &ret[0];
  size_t* missedp = &missed[0];
  size_t missedsize = missed.size();
  size_t misseddummy = numeric_limits<size_t>::max() - 1;
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < missedsize; i++) {
    retp[missedp[i]] = misseddummy - i;
  }
  return ret;
}

// assumes implementation of dunordered_map
node_local<vector<size_t>>
typed_dfcolumn<string>::
equal_prepare(shared_ptr<typed_dfcolumn<string>>& right) {
  auto& rightdic = *(right->dic);
  auto rightdicnl = rightdic.viewas_node_local();
  auto nlfrom = make_node_local_allocate<std::vector<size_t>>();
  auto nlto = make_node_local_allocate<std::vector<size_t>>();
  dic->viewas_node_local().mapv(create_string_trans_table, rightdicnl,
                                nlfrom, nlto);
  auto bcastfrom = broadcast(nlfrom.moveto_dvector<size_t>().gather());
  auto bcastto = broadcast(nlto.moveto_dvector<size_t>().gather());
  return right->val.map(equal_prepare_helper, bcastfrom, bcastto);
}

node_local<std::vector<size_t>>
typed_dfcolumn<string>::filter_eq(std::shared_ptr<dfcolumn>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<string>>(right);
  if(!static_cast<bool>(right2))
    throw std::runtime_error("filter_eq: column types are different");
  auto rightval = equal_prepare(right2);
  auto filtered_idx = val.map(filter_eq_helper<size_t,size_t>, rightval);
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

node_local<std::vector<size_t>>
typed_dfcolumn<string>::filter_eq_immed(std::shared_ptr<dfscalar>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfscalar<std::string>>(right);
  if(!static_cast<bool>(right2))
    throw std::runtime_error("filter string column with non string");
  bool found;
  size_t right_val = dic->get(right2->val, found);
  if(found) {
    auto filtered_idx = val.map(filter_eq_immed_helper<size_t,size_t>,
                                broadcast(right_val));
    if(contain_nulls)
      return filtered_idx.map(set_difference<size_t>, nulls);
    else return filtered_idx;
  } else {
    return make_node_local_allocate<std::vector<size_t>>();
  }
}

node_local<std::vector<size_t>>
typed_dfcolumn<string>::filter_neq(std::shared_ptr<dfcolumn>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfcolumn<string>>(right);
  if(!static_cast<bool>(right2))
    throw std::runtime_error("filter_eq: column types are different");
  auto rightval = equal_prepare(right2);
  auto filtered_idx = val.map(filter_neq_helper<size_t,size_t>, rightval);
  if(right2->contain_nulls)
    filtered_idx = filtered_idx.map(set_difference<size_t>, right2->nulls);
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

node_local<std::vector<size_t>>
typed_dfcolumn<string>::filter_neq_immed(std::shared_ptr<dfscalar>& right) {
  auto right2 = std::dynamic_pointer_cast<typed_dfscalar<std::string>>(right);
  if(!static_cast<bool>(right2))
    throw std::runtime_error("filter string column with non string");
  bool found;
  size_t right_val = dic->get(right2->val, found);
  if(found) {
    auto filtered_idx = val.map(filter_neq_immed_helper<size_t,size_t>,
                                broadcast(right_val));
    if(contain_nulls)
      return filtered_idx.map(set_difference<size_t>, nulls);
    else return filtered_idx;
  } else {
    return val.map(get_local_index_helper<size_t>);
  }
}

// ----- datetime -----
node_local<std::vector<size_t>>
typed_dfcolumn<datetime>::filter_eq(std::shared_ptr<dfcolumn>& right) {
  std::shared_ptr<typed_dfcolumn<datetime>> right2 = NULL;
  auto dt = right->dtype();
  if(dt == "datetime") {
    right2 = std::dynamic_pointer_cast<typed_dfcolumn<datetime>>(right);
  } else if(dt == "string" || dt == "dic_string" || dt == "raw_string") {
    auto tmp = right->type_cast("datetime"); // using default format: %Y-%m-%d
    right2 = std::dynamic_pointer_cast<typed_dfcolumn<datetime>>(tmp);
  }
  else throw std::runtime_error("datetime == " + dt + ": invalid operation!");
  auto filtered_idx = val.map(filter_eq_helper<datetime_t, datetime_t>,
                              right2->val);
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

node_local<std::vector<size_t>>
typed_dfcolumn<datetime>::filter_eq_immed(std::shared_ptr<dfscalar>& right) {
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
      throw std::runtime_error("datetime == " + dt + ": invalid operation!");
    right2 = std::make_shared<typed_dfscalar<datetime>>(tmp->val);
  }
  auto filtered_idx = val.map(filter_eq_immed_helper<datetime_t, datetime_t>,
                              broadcast(right2->val));
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

node_local<std::vector<size_t>>
typed_dfcolumn<datetime>::filter_neq(std::shared_ptr<dfcolumn>& right) {
  std::shared_ptr<typed_dfcolumn<datetime>> right2 = NULL;
  auto dt = right->dtype();
  if(dt == "datetime") {
    right2 = std::dynamic_pointer_cast<typed_dfcolumn<datetime>>(right);
  } else if(dt == "string" || dt == "dic_string" || dt == "raw_string") {
    auto tmp = right->type_cast("datetime"); // using default format: %Y-%m-%d
    right2 = std::dynamic_pointer_cast<typed_dfcolumn<datetime>>(tmp);
  }
  else throw std::runtime_error("datetime != " + dt + ": invalid operation!");
  auto filtered_idx = val.map(filter_neq_helper<datetime_t, datetime_t>,
                              right2->val);
  if(right2->contain_nulls)
    filtered_idx = filtered_idx.map(set_difference<size_t>, right2->nulls);
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

node_local<std::vector<size_t>>
typed_dfcolumn<datetime>::filter_neq_immed(std::shared_ptr<dfscalar>& right) {
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
      throw std::runtime_error("datetime != " + dt + ": invalid operation!");
    right2 = std::make_shared<typed_dfscalar<datetime>>(tmp->val);
  }
  auto filtered_idx = val.map(filter_neq_immed_helper<datetime_t, datetime_t>,
                              broadcast(right2->val));
  if(contain_nulls)
    return filtered_idx.map(set_difference<size_t>, nulls);
  else return filtered_idx;
}

}
