#include "dfcolumn.hpp"

#include <limits>
#include <climits>
#include <unordered_set>
#include <regex>
#include "../core/set_operations.hpp"
#include "dfcolumn_helper.hpp"

using namespace std;

namespace frovedis {

template <class T>
node_local<std::vector<size_t>>
typed_dfcolumn<T>::filter_is_null() {return nulls;}

template <class T>
node_local<std::vector<size_t>>
typed_dfcolumn<T>::filter_is_not_null() {
  auto local_idx = get_local_index();
  if(contain_nulls)
    return local_idx.map(set_difference<size_t>, nulls);
  else return local_idx;
}

template node_local<std::vector<size_t>>
typed_dfcolumn<int>::filter_is_null();
template node_local<std::vector<size_t>>
typed_dfcolumn<unsigned int>::filter_is_null();
template node_local<std::vector<size_t>>
typed_dfcolumn<long>::filter_is_null();
template node_local<std::vector<size_t>>
typed_dfcolumn<unsigned long>::filter_is_null();
template node_local<std::vector<size_t>>
typed_dfcolumn<float>::filter_is_null();
template node_local<std::vector<size_t>>
typed_dfcolumn<double>::filter_is_null();

template  node_local<std::vector<size_t>>
typed_dfcolumn<int>::filter_is_not_null();
template  node_local<std::vector<size_t>>
typed_dfcolumn<unsigned int>::filter_is_not_null();
template  node_local<std::vector<size_t>>
typed_dfcolumn<long>::filter_is_not_null();
template  node_local<std::vector<size_t>>
typed_dfcolumn<unsigned long>::filter_is_not_null();
template  node_local<std::vector<size_t>>
typed_dfcolumn<float>::filter_is_not_null();
template  node_local<std::vector<size_t>>
typed_dfcolumn<double>::filter_is_not_null();

// ----- dic_string -----
node_local<std::vector<size_t>>
typed_dfcolumn<dic_string>::filter_is_null() {return nulls;}

node_local<std::vector<size_t>>
typed_dfcolumn<dic_string>::filter_is_not_null() {
  auto local_idx = get_local_index();
  if(contain_nulls)
    return local_idx.map(set_difference<size_t>, nulls);
  else return local_idx;
}

vector<size_t> filter_like_helper(const dict& dic,
                                  const std::string& pattern,
                                  int wild_card) {
  auto num_words = dic.num_words();
  std::vector<size_t> order(num_words);
  auto orderp = order.data();
  for(size_t i = 0; i < num_words; i++) orderp[i] = i;
  auto ws = decompress_compressed_words(dic.cwords, dic.lens, dic.lens_num,
                                        order);
  return like(ws, pattern, wild_card);
}

vector<size_t> filter_not_like_helper(const dict& dic,
                                      const std::string& pattern,
                                      int wild_card) {
  auto num_words = dic.num_words();
  std::vector<size_t> order(num_words);
  auto orderp = order.data();
  for(size_t i = 0; i < num_words; i++) orderp[i] = i;
  auto ws = decompress_compressed_words(dic.cwords, dic.lens, dic.lens_num,
                                        order);
  auto hit = like(ws, pattern, wild_card);
  return set_difference(order, hit);
}

#if !(defined(_SX) || defined(__ve__))
vector<size_t> filter_like_join(std::vector<size_t>& left,
                                std::vector<size_t>& left_idx, 
                                std::vector<size_t>& right) {
  std::unordered_set<size_t> right_set;
  for(size_t i = 0; i < right.size(); i++) {
    right_set.insert(right[i]);
  }
  vector<size_t> ret;
  for(size_t i = 0; i < left.size(); i++) {
    auto it = right_set.find(left[i]);
    if(it != right_set.end()) {
      ret.push_back(left_idx[i]);
    }
  }
  return ret;
}
#else
std::vector<size_t> filter_like_join(std::vector<size_t>& left,
                                     std::vector<size_t>& left_idx,
                                     std::vector<size_t>& right) {
  vector<size_t> dummy(right.size());
  unique_hashtable<size_t, size_t> ht(right, dummy);
  std::vector<size_t> missed;
  ht.lookup(left, missed); // ret val not used
  return shrink_missed(left_idx, missed);
}
#endif

// implement as join, since matched strings might be many
node_local<std::vector<size_t>>
typed_dfcolumn<dic_string>::
filter_like(const std::string& pattern, int wild_card) {
  auto right_non_null_val = filter_like_helper(*dic, pattern, wild_card);
  auto left_full_local_idx = get_local_index();
  node_local<std::vector<size_t>> left_non_null_idx;
  node_local<std::vector<size_t>> left_non_null_val;
  if(contain_nulls) {
    left_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
    left_non_null_val = val.map(extract_non_null<size_t>, left_full_local_idx,
                                nulls, left_non_null_idx);
  } else {
    left_non_null_val = val.map(extract_helper2<size_t>, left_full_local_idx);
    left_non_null_idx = std::move(left_full_local_idx);
  }
  auto right_non_null_val_bcast = broadcast(right_non_null_val);
  auto left_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  return left_non_null_val.map(filter_like_join, left_non_null_idx,
                               right_non_null_val_bcast);
}

node_local<std::vector<size_t>>
typed_dfcolumn<dic_string>::
filter_not_like(const std::string& pattern, int wild_card) {
  auto right_non_null_val = filter_not_like_helper(*dic, pattern, wild_card);
  auto left_full_local_idx = get_local_index();
  node_local<std::vector<size_t>> left_non_null_idx;
  node_local<std::vector<size_t>> left_non_null_val;
  if(contain_nulls) {
    left_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
    left_non_null_val = val.map(extract_non_null<size_t>, left_full_local_idx,
                                nulls, left_non_null_idx);
  } else {
    left_non_null_val = val.map(extract_helper2<size_t>, left_full_local_idx);
    left_non_null_idx = std::move(left_full_local_idx);
  }
  auto right_non_null_val_bcast = broadcast(right_non_null_val);
  auto left_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  return left_non_null_val.map(filter_like_join, left_non_null_idx,
                               right_non_null_val_bcast);
}

// ----- raw_string -----
node_local<std::vector<size_t>>
typed_dfcolumn<raw_string>::filter_is_null() {return nulls;}

node_local<std::vector<size_t>>
typed_dfcolumn<raw_string>::filter_is_not_null() {
  auto local_idx = get_local_index();
  if(contain_nulls)
    return local_idx.map(set_difference<size_t>, nulls);
  else return local_idx;
}

node_local<std::vector<size_t>>
typed_dfcolumn<raw_string>::
filter_like(const std::string& pattern, int wild_card) {
  auto hit =
    comp_words.map(+[](const compressed_words& cws, 
                       const std::string& p, int wild_card) {
                     return like(cws.decompress(), p, wild_card);
                   }, broadcast(pattern), broadcast(wild_card));
  if(contain_nulls) {
    return hit.map(set_difference<size_t>, nulls);
  } else {
    return hit;
  }
}

node_local<std::vector<size_t>>
typed_dfcolumn<raw_string>::
filter_not_like(const std::string& pattern, int wild_card) {
  auto hit =
    comp_words.map(+[](const compressed_words& cws, 
                       const std::string& p, int wild_card) {
        auto num_words = cws.num_words();
        auto hit_like = like(cws.decompress(), p, wild_card);
        std::vector<size_t> iota(num_words);
        auto iotap = iota.data();
        for(size_t i = 0; i < num_words; i++) iotap[i] = i;
        return set_difference(iota, hit_like);
      }, broadcast(pattern), broadcast(wild_card));
  if(contain_nulls) {
    return hit.map(set_difference<size_t>, nulls);
  } else {
    return hit;
  }
}

// ----- string -----
node_local<std::vector<size_t>>
typed_dfcolumn<string>::filter_is_null() {return nulls;}

node_local<std::vector<size_t>>
typed_dfcolumn<string>::filter_is_not_null() {
  auto local_idx = get_local_index();
  if(contain_nulls)
    return local_idx.map(set_difference<size_t>, nulls);
  else return local_idx;
}

vector<size_t> filter_regex_helper(vector<string>& dic, std::string& pattern) {
  vector<size_t> ret;
  size_t size = dic.size();
  regex re(pattern);
  size_t selfid = static_cast<size_t>(get_selfid());
  size_t nodeinfo = selfid << DFNODESHIFT;
  for(size_t i = 0; i < size; i++) {
    if(regex_match(dic[i], re)) ret.push_back(i+nodeinfo);
  }
  return ret;
}

vector<size_t> filter_not_regex_helper(vector<string>& dic,
                                       std::string& pattern) {
  vector<size_t> ret;
  size_t size = dic.size();
  regex re(pattern);
  size_t selfid = static_cast<size_t>(get_selfid());
  size_t nodeinfo = selfid << DFNODESHIFT;
  for(size_t i = 0; i < size; i++) {
    if(!regex_match(dic[i], re)) ret.push_back(i+nodeinfo);
  }
  return ret;
}

#if !(defined(_SX) || defined(__ve__))
vector<size_t> filter_regex_join(std::vector<size_t>& left,
                                 std::vector<size_t>& left_idx, 
                                 std::vector<size_t>& right) {
  std::unordered_set<size_t> right_set;
  for(size_t i = 0; i < right.size(); i++) {
    right_set.insert(right[i]);
  }
  vector<size_t> ret;
  for(size_t i = 0; i < left.size(); i++) {
    auto it = right_set.find(left[i]);
    if(it != right_set.end()) {
      ret.push_back(left_idx[i]);
    }
  }
  return ret;
}
#else
std::vector<size_t> filter_regex_join(std::vector<size_t>& left,
                                      std::vector<size_t>& left_idx,
                                      std::vector<size_t>& right) {
  vector<size_t> dummy(right.size());
  unique_hashtable<size_t, size_t> ht(right, dummy);
  std::vector<size_t> missed;
  ht.lookup(left, missed); // ret val not used
  return shrink_missed(left_idx, missed);
}
#endif

// implement as join, since matched strings might be many
node_local<std::vector<size_t>>
typed_dfcolumn<string>::
filter_regex(const std::string& pattern) {
  auto right_non_null_val = dic_idx->map(filter_regex_helper, broadcast(pattern));
  auto left_full_local_idx = get_local_index();
  node_local<std::vector<size_t>> left_non_null_idx;
  node_local<std::vector<size_t>> left_non_null_val;
  if(contain_nulls) {
    left_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
    left_non_null_val = val.map(extract_non_null<size_t>, left_full_local_idx,
                                nulls, left_non_null_idx);
  } else {
    left_non_null_val = val.map(extract_helper2<size_t>, left_full_local_idx);
    left_non_null_idx = std::move(left_full_local_idx);
  }
  auto right_non_null_val_bcast =
    broadcast(right_non_null_val.template viewas_dvector<size_t>().gather());
  auto left_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  return left_non_null_val.map(filter_regex_join, left_non_null_idx,
                               right_non_null_val_bcast);
}

node_local<std::vector<size_t>>
typed_dfcolumn<string>::
filter_not_regex(const std::string& pattern) {
  auto right_non_null_val = dic_idx->map(filter_not_regex_helper, broadcast(pattern));
  auto left_full_local_idx = get_local_index();
  node_local<std::vector<size_t>> left_non_null_idx;
  node_local<std::vector<size_t>> left_non_null_val;
  if(contain_nulls) {
    left_non_null_idx = make_node_local_allocate<std::vector<size_t>>();
    left_non_null_val = val.map(extract_non_null<size_t>, left_full_local_idx,
                                nulls, left_non_null_idx);
  } else {
    left_non_null_val = val.map(extract_helper2<size_t>, left_full_local_idx);
    left_non_null_idx = std::move(left_full_local_idx);
  }
  auto right_non_null_val_bcast =
    broadcast(right_non_null_val.template viewas_dvector<size_t>().gather());
  auto left_idx_ret = make_node_local_allocate<std::vector<size_t>>();
  return left_non_null_val.map(filter_regex_join, left_non_null_idx,
                               right_non_null_val_bcast);
}

}
