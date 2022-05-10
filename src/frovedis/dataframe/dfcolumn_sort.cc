#include "dfcolumn.hpp"

#include <limits>
#include <climits>
#include "../core/radix_sort.hpp"

using namespace std;

namespace frovedis {

#if defined(_SX) || defined(__ve__)
template <class K, class V>
void sort_pair(std::vector<K>& key_array, std::vector<V>& val_array) {
  radix_sort(key_array, val_array);
}
template <class K, class V>
void sort_pair_desc(std::vector<K>& key_array, std::vector<V>& val_array) {
  radix_sort_desc(key_array, val_array);
}
#else
template <class K, class V>
struct sort_pair_helper {
  bool operator()(const std::pair<K,V>& left,
                  const std::pair<K,V>& right) {
    return left.first < right.first;
  }
};
template <class K, class V>
void sort_pair(std::vector<K>& key_array, std::vector<V>& val_array) {
  size_t size = key_array.size();
  if(val_array.size() != size)
    throw std::runtime_error("sort_pair: different size of arrays");
  std::vector<std::pair<K,V>> kv_pair(size);
  for(size_t i = 0; i < size; i++) {
    kv_pair[i].first = key_array[i];
    kv_pair[i].second = val_array[i];
  }
  std::stable_sort(kv_pair.begin(), kv_pair.end(), sort_pair_helper<K,V>());
  for(size_t i = 0; i < size; i++) {
    key_array[i] = kv_pair[i].first;
    val_array[i] = kv_pair[i].second;
  }
}
template <class K, class V>
struct sort_pair_desc_helper {
  bool operator()(const std::pair<K,V>& left,
                  const std::pair<K,V>& right) {
    return left.first > right.first;
  }
};
template <class K, class V>
void sort_pair_desc(std::vector<K>& key_array, std::vector<V>& val_array) {
  size_t size = key_array.size();
  if(val_array.size() != size)
    throw std::runtime_error("sort_pair: different size of arrays");
  std::vector<std::pair<K,V>> kv_pair(size);
  for(size_t i = 0; i < size; i++) {
    kv_pair[i].first = key_array[i];
    kv_pair[i].second = val_array[i];
  }
  std::stable_sort(kv_pair.begin(), kv_pair.end(),
                   sort_pair_desc_helper<K,V>());
  for(size_t i = 0; i < size; i++) {
    key_array[i] = kv_pair[i].first;
    val_array[i] = kv_pair[i].second;
  }
}
#endif

template <class T>
std::vector<T> dfcolumn_sort_local(std::vector<T>& val,
                                   std::vector<size_t>& idx) {
  size_t size = val.size();
  idx.resize(size);
  size_t selfid = static_cast<size_t>(get_selfid());
  size_t nodeinfo = selfid << DFNODESHIFT;
  size_t* idxp = &idx[0];
  for(size_t i = 0; i < size; i++) {
    idxp[i] = i + nodeinfo;
  }
  sort_pair(val, idx);
  int nodesize = get_nodesize();
  size_t each = ceil_div(size, size_t(nodesize));
  size_t part_size = ceil_div(size, each);
  std::vector<T> part(part_size);
  T* partp = &part[0];
  T* valp = &val[0];
  for(size_t i = 0; i < part_size; i++) {
    partp[i] = valp[each * i];
  }
  return part;
}

template <class T>
void dfcolumn_sort_partition(std::vector<T>& val,
                             std::vector<size_t>& idx,
                             std::vector<std::vector<T>>& part_val,
                             std::vector<std::vector<size_t>>& part_idx,
                             std::vector<T>& part) {
  int nodesize = get_nodesize();
  part_val.resize(nodesize);
  part_idx.resize(nodesize);
  size_t size = val.size();
  auto last_it = val.begin();
  auto valp = val.data();
  auto idxp = idx.data();
  for(size_t i = 0; i < nodesize - 1; i++) {
    auto it = std::lower_bound(val.begin(), val.end(), part[i]);
    size_t start = last_it - val.begin();
    size_t end = it - val.begin();
    size_t part_size = end - start;
    part_val[i].resize(part_size);
    part_idx[i].resize(part_size);
    T* part_valp = &part_val[i][0];
    size_t* part_idxp = &part_idx[i][0];
    for(size_t i = 0; i < part_size; i++) {
      part_valp[i] = valp[start+i];
      part_idxp[i] = idxp[start+i];
    }
    last_it = it;
  }
  size_t start = last_it - val.begin();
  size_t end = size;
  size_t part_size = end - start;
  part_val[nodesize-1].resize(part_size);
  part_idx[nodesize-1].resize(part_size);
  T* part_valp = &part_val[nodesize-1][0];
  size_t* part_idxp = &part_idx[nodesize-1][0];
  for(size_t i = 0; i < part_size; i++) {
    part_valp[i] = valp[start+i];
    part_idxp[i] = idxp[start+i];
  }
}

// based on distributed sample sort
// http://www3.cs.stonybrook.edu/~rezaul/Spring-2012/CSE613/
// CSE613-lecture-16.pdf
// here, pivot includes fist element to support already sorted entries
template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::sort(node_local<std::vector<size_t>>& res_idx) {
  if(size() == 0) {
    res_idx = make_node_local_allocate<std::vector<size_t>>();
    auto ret = std::make_shared<typed_dfcolumn<T>>();
    ret->val = make_node_local_allocate<std::vector<T>>();
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
    return ret;
  }
  auto to_sort = val; // need to copy because of implace sort
  auto idx = make_node_local_allocate<std::vector<size_t>>();
  auto part_cand =
    to_sort.map(dfcolumn_sort_local<T>, idx).reduce(set_merge<T>);
  size_t nodesize = get_nodesize();
  std::vector<T> part(nodesize-1);
  size_t part_cand_size = part_cand.size();
  T* partp = &part[0];
  T* part_candp = &part_cand[0];
  for(size_t i = 0; i < nodesize - 1; i++) {
    auto idx = std::min((i+1) * nodesize, part_cand_size - 1);
    partp[i] = part_candp[idx];
  }
  auto part_val = make_node_local_allocate<std::vector<std::vector<T>>>();
  auto part_idx = make_node_local_allocate<std::vector<std::vector<size_t>>>();
  to_sort.mapv(dfcolumn_sort_partition<T>, idx, part_val, part_idx,
               broadcast(part));
  auto exchanged_val = alltoall_exchange(part_val);
  auto exchanged_idx = alltoall_exchange(part_idx);
  auto res_val = make_node_local_allocate<std::vector<T>>();
  res_idx = make_node_local_allocate<std::vector<size_t>>();
  exchanged_val.mapv(set_multimerge_pair<T,size_t>, exchanged_idx,
                     res_val, res_idx);
  auto ret = std::make_shared<typed_dfcolumn<T>>();
  if(contain_nulls) {
    // if contain_nulls, sorted column is not reused,
    // since calculating null posistion is not trivial
    ret->val = make_node_local_allocate<std::vector<T>>();
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  } else {
    ret->val = std::move(res_val);
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  }
  return ret;
}

template <class T>
std::vector<T> dfcolumn_sort_local_desc(std::vector<T>& val,
                                        std::vector<size_t>& idx) {
  size_t size = val.size();
  idx.resize(size);
  size_t selfid = static_cast<size_t>(get_selfid());
  size_t nodeinfo = selfid << DFNODESHIFT;
  size_t* idxp = &idx[0];
  for(size_t i = 0; i < size; i++) {
    idxp[i] = i + nodeinfo;
  }
  sort_pair_desc(val, idx);
  int nodesize = get_nodesize();
  size_t each = ceil_div(size, size_t(nodesize));
  size_t part_size = ceil_div(size, each);
  std::vector<T> part(part_size);
  T* partp = &part[0];
  T* valp = &val[0];
  for(size_t i = 0; i < part_size; i++) {
    partp[i] = valp[each * i];
  }
  return part;
}

template <class T>
void dfcolumn_sort_partition_desc(std::vector<T>& val,
                                  std::vector<size_t>& idx,
                                  std::vector<std::vector<T>>& part_val,
                                  std::vector<std::vector<size_t>>& part_idx,
                                  std::vector<T>& part) {
  int nodesize = get_nodesize();
  part_val.resize(nodesize);
  part_idx.resize(nodesize);
  size_t size = val.size();
  auto last_it = val.begin();
  auto valp = val.data();
  auto idxp = idx.data();
  for(size_t i = 0; i < nodesize - 1; i++) {
    auto it = std::lower_bound(val.begin(), val.end(), part[i],
                               std::greater<T>());
    size_t start = last_it - val.begin();
    size_t end = it - val.begin();
    size_t part_size = end - start;
    part_val[i].resize(part_size);
    part_idx[i].resize(part_size);
    T* part_valp = &part_val[i][0];
    size_t* part_idxp = &part_idx[i][0];
    for(size_t i = 0; i < part_size; i++) {
      part_valp[i] = valp[start+i];
      part_idxp[i] = idxp[start+i];
    }
    last_it = it;
  }
  size_t start = last_it - val.begin();
  size_t end = size;
  size_t part_size = end - start;
  part_val[nodesize-1].resize(part_size);
  part_idx[nodesize-1].resize(part_size);
  T* part_valp = &part_val[nodesize-1][0];
  size_t* part_idxp = &part_idx[nodesize-1][0];
  for(size_t i = 0; i < part_size; i++) {
    part_valp[i] = valp[start+i];
    part_idxp[i] = idxp[start+i];
  }
}


template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::sort_desc(node_local<std::vector<size_t>>& res_idx) {
  if(size() == 0) {
    res_idx = make_node_local_allocate<std::vector<size_t>>();
    auto ret = std::make_shared<typed_dfcolumn<T>>();
    ret->val = make_node_local_allocate<std::vector<T>>();
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
    return ret;
  }
  auto to_sort = val; // need to copy because of implace sort
  auto idx = make_node_local_allocate<std::vector<size_t>>();
  auto part_cand =
    to_sort.map(dfcolumn_sort_local_desc<T>, idx).reduce(set_merge_desc<T>);
  size_t nodesize = get_nodesize();
  std::vector<T> part(nodesize-1);
  size_t part_cand_size = part_cand.size();
  T* partp = &part[0];
  T* part_candp = &part_cand[0];
  for(size_t i = 0; i < nodesize - 1; i++) {
    auto idx = std::min((i+1) * nodesize, part_cand_size - 1);
    partp[i] = part_candp[idx];
  }
  auto part_val = make_node_local_allocate<std::vector<std::vector<T>>>();
  auto part_idx = make_node_local_allocate<std::vector<std::vector<size_t>>>();
  to_sort.mapv(dfcolumn_sort_partition_desc<T>, idx, part_val, part_idx,
               broadcast(part));
  auto exchanged_val = alltoall_exchange(part_val);
  auto exchanged_idx = alltoall_exchange(part_idx);
  auto res_val = make_node_local_allocate<std::vector<T>>();
  res_idx = make_node_local_allocate<std::vector<size_t>>();
  exchanged_val.mapv(set_multimerge_pair_desc<T,size_t>, exchanged_idx,
                     res_val, res_idx);
  auto ret = std::make_shared<typed_dfcolumn<T>>();
  if(contain_nulls) {
    // if contain_nulls, sorted column is not reused,
    // since calculating null posistion is not trivial
    ret->val = make_node_local_allocate<std::vector<T>>();
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  } else {
    ret->val = std::move(res_val);
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  }
  return ret;
}

template <class T>
std::vector<T> dfcolumn_sort_with_idx_local(std::vector<T>& val,
                                            std::vector<size_t>& idx) {
  size_t size = val.size();
  sort_pair(val, idx);
  int nodesize = get_nodesize();
  size_t each = ceil_div(size, size_t(nodesize));
  size_t part_size = ceil_div(size, each);
  std::vector<T> part(part_size);
  T* partp = &part[0];
  T* valp = &val[0];
  for(size_t i = 0; i < part_size; i++) {
    partp[i] = valp[each * i];
  }
  return part;
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::sort_with_idx(node_local<std::vector<size_t>>& idx,
                                 node_local<std::vector<size_t>>& res_idx) {
  if(size() == 0) {
    res_idx = make_node_local_allocate<std::vector<size_t>>();
    auto ret = std::make_shared<typed_dfcolumn<T>>();
    ret->val = make_node_local_allocate<std::vector<T>>();
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
    return ret;
  }
  auto to_sort = val; // need to copy because of implace sort
  auto part_cand =
    to_sort.map(dfcolumn_sort_with_idx_local<T>, idx).reduce(set_merge<T>);
  size_t nodesize = get_nodesize();
  std::vector<T> part(nodesize-1);
  size_t part_cand_size = part_cand.size();
  T* partp = &part[0];
  T* part_candp = &part_cand[0];
  for(size_t i = 0; i < nodesize - 1; i++) {
    auto idx = std::min((i+1) * nodesize, part_cand_size - 1);
    partp[i] = part_candp[idx];
  }
  auto part_val = make_node_local_allocate<std::vector<std::vector<T>>>();
  auto part_idx = make_node_local_allocate<std::vector<std::vector<size_t>>>();
  to_sort.mapv(dfcolumn_sort_partition<T>, idx, part_val, part_idx,
               broadcast(part));
  auto exchanged_val = alltoall_exchange(part_val);
  auto exchanged_idx = alltoall_exchange(part_idx);
  auto res_val = make_node_local_allocate<std::vector<T>>();
  res_idx = make_node_local_allocate<std::vector<size_t>>();
  exchanged_val.mapv(set_multimerge_pair<T,size_t>, exchanged_idx,
                     res_val, res_idx);
  auto ret = std::make_shared<typed_dfcolumn<T>>();
  if(contain_nulls) {
    // if contain_nulls, sorted column is not reused,
    // since calculating null posistion is not trivial
    ret->val = make_node_local_allocate<std::vector<T>>();
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  } else {
    ret->val = std::move(res_val);
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  }
  return ret;
}

template <class T>
std::vector<T> dfcolumn_sort_with_idx_local_desc(std::vector<T>& val,
                                                 std::vector<size_t>& idx) {
  size_t size = val.size();
  sort_pair_desc(val, idx);
  int nodesize = get_nodesize();
  size_t each = ceil_div(size, size_t(nodesize));
  size_t part_size = ceil_div(size, each);
  std::vector<T> part(part_size);
  T* partp = &part[0];
  T* valp = &val[0];
  for(size_t i = 0; i < part_size; i++) {
    partp[i] = valp[each * i];
  }
  return part;
}

template <class T>
std::shared_ptr<dfcolumn> typed_dfcolumn<T>::
sort_with_idx_desc(node_local<std::vector<size_t>>& idx,
                   node_local<std::vector<size_t>>& res_idx) {
  if(size() == 0) {
    res_idx = make_node_local_allocate<std::vector<size_t>>();
    auto ret = std::make_shared<typed_dfcolumn<T>>();
    ret->val = make_node_local_allocate<std::vector<T>>();
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
    return ret;
  }
  auto to_sort = val; // need to copy because of implace sort
  auto part_cand = to_sort.map(dfcolumn_sort_with_idx_local_desc<T>, idx).
    reduce(set_merge_desc<T>);
  size_t nodesize = get_nodesize();
  std::vector<T> part(nodesize-1);
  size_t part_cand_size = part_cand.size();
  T* partp = &part[0];
  T* part_candp = &part_cand[0];
  for(size_t i = 0; i < nodesize - 1; i++) {
    auto idx = std::min((i+1) * nodesize, part_cand_size - 1);
    partp[i] = part_candp[idx];
  }
  auto part_val = make_node_local_allocate<std::vector<std::vector<T>>>();
  auto part_idx = make_node_local_allocate<std::vector<std::vector<size_t>>>();
  to_sort.mapv(dfcolumn_sort_partition_desc<T>, idx, part_val, part_idx,
               broadcast(part));
  auto exchanged_val = alltoall_exchange(part_val);
  auto exchanged_idx = alltoall_exchange(part_idx);
  auto res_val = make_node_local_allocate<std::vector<T>>();
  res_idx = make_node_local_allocate<std::vector<size_t>>();
  exchanged_val.mapv(set_multimerge_pair_desc<T,size_t>, exchanged_idx,
                     res_val, res_idx);
  auto ret = std::make_shared<typed_dfcolumn<T>>();
  if(contain_nulls) {
    // if contain_nulls, sorted column is not reused
    // since calculating null posistion is not trivial
    ret->val = make_node_local_allocate<std::vector<T>>();
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  } else {
    ret->val = std::move(res_val);
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  }
  return ret;
}

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::sort(node_local<std::vector<size_t>>& res_idx);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::sort(node_local<std::vector<size_t>>& res_idx);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::sort(node_local<std::vector<size_t>>& res_idx);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::sort(node_local<std::vector<size_t>>& res_idx);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::sort(node_local<std::vector<size_t>>& res_idx);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::sort(node_local<std::vector<size_t>>& res_idx);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::sort_desc(node_local<std::vector<size_t>>& res_idx);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::sort_desc(node_local<std::vector<size_t>>& res_idx);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::sort_desc(node_local<std::vector<size_t>>& res_idx);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::sort_desc(node_local<std::vector<size_t>>& res_idx);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::sort_desc(node_local<std::vector<size_t>>& res_idx);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::sort_desc(node_local<std::vector<size_t>>& res_idx);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::sort_with_idx(node_local<std::vector<size_t>>& idx,
                                   node_local<std::vector<size_t>>& res_idx);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::sort_with_idx(node_local<std::vector<size_t>>& idx,
                                            node_local<std::vector<size_t>>& res_idx);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::sort_with_idx(node_local<std::vector<size_t>>& idx,
                                    node_local<std::vector<size_t>>& res_idx);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::sort_with_idx(node_local<std::vector<size_t>>& idx,
                                             node_local<std::vector<size_t>>& res_idx);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::sort_with_idx(node_local<std::vector<size_t>>& idx,
                                    node_local<std::vector<size_t>>& res_idx);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::sort_with_idx(node_local<std::vector<size_t>>& idx,
                                      node_local<std::vector<size_t>>& res_idx);

template std::shared_ptr<dfcolumn> typed_dfcolumn<int>::
sort_with_idx_desc(node_local<std::vector<size_t>>& idx,
                   node_local<std::vector<size_t>>& res_idx);
template std::shared_ptr<dfcolumn> typed_dfcolumn<unsigned int>::
sort_with_idx_desc(node_local<std::vector<size_t>>& idx,
                   node_local<std::vector<size_t>>& res_idx);
template std::shared_ptr<dfcolumn> typed_dfcolumn<long>::
sort_with_idx_desc(node_local<std::vector<size_t>>& idx,
                   node_local<std::vector<size_t>>& res_idx);
template std::shared_ptr<dfcolumn> typed_dfcolumn<unsigned long>::
sort_with_idx_desc(node_local<std::vector<size_t>>& idx,
                   node_local<std::vector<size_t>>& res_idx);
template std::shared_ptr<dfcolumn> typed_dfcolumn<float>::
sort_with_idx_desc(node_local<std::vector<size_t>>& idx,
                   node_local<std::vector<size_t>>& res_idx);
template std::shared_ptr<dfcolumn> typed_dfcolumn<double>::
sort_with_idx_desc(node_local<std::vector<size_t>>& idx,
                   node_local<std::vector<size_t>>& res_idx);

// ----- dic_string -----
std::vector<size_t>
dic_string_sort_prepare_helper(const std::vector<size_t>& val,
                               const std::vector<size_t>& order,
                               const std::vector<size_t>& nulls) {
  auto orderp = order.data();
  auto valp = val.data();
  auto val_size = val.size();
  std::vector<size_t> new_val(val_size);
  auto new_valp = new_val.data();
  auto null_value = std::numeric_limits<size_t>::max();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < val_size; i++) {
    if(valp[i] != null_value) new_valp[i] = orderp[valp[i]];
  }
  auto nullsp = nulls.data();
  auto nulls_size = nulls.size();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < nulls_size; i++) {
    new_valp[nullsp[i]] = null_value;
  }
  return new_val;
}

typed_dfcolumn<size_t>
typed_dfcolumn<dic_string>::sort_prepare() {
  typed_dfcolumn<size_t> rescol;

  auto& local_dic = *dic;
  auto num_words = local_dic.num_words();
  std::vector<size_t> order(num_words);
  auto orderp = order.data();
  for(size_t i = 0; i < num_words; i++) orderp[i] = i;
  lexical_sort_compressed_words(local_dic.cwords, local_dic.lens,
                                local_dic.lens_num, order);
  rescol.val = val.map(dic_string_sort_prepare_helper, broadcast(order),
                       nulls);
  rescol.nulls = nulls; // copy
  return rescol;
}

shared_ptr<dfcolumn>
typed_dfcolumn<dic_string>::sort(node_local<vector<size_t>>& idx) {
  auto tmpcol = sort_prepare();
  return tmpcol.sort(idx);
}

shared_ptr<dfcolumn>
typed_dfcolumn<dic_string>::sort_desc(node_local<vector<size_t>>& idx) {
  auto tmpcol = sort_prepare();
  return tmpcol.sort_desc(idx);
}

shared_ptr<dfcolumn>
typed_dfcolumn<dic_string>::sort_with_idx(node_local<vector<size_t>>& idx,
                                          node_local<vector<size_t>>& res_idx) {
  auto tmpcol = sort_prepare();
  return tmpcol.sort_with_idx(idx, res_idx);
}

shared_ptr<dfcolumn>
typed_dfcolumn<dic_string>::
sort_with_idx_desc(node_local<vector<size_t>>& idx,
                   node_local<vector<size_t>>& res_idx) {
  auto tmpcol = sort_prepare();
  return tmpcol.sort_with_idx_desc(idx, res_idx);
}

// ----- string -----
vector<size_t> convert_sorted_idx(vector<size_t>& val,
                                  vector<size_t>& idx_dic,
                                  vector<size_t>& nulls) {
  size_t dicsize = idx_dic.size();
  vector<size_t> newidx(dicsize);
  size_t* newidxp = &newidx[0];
  for(size_t i = 0; i < dicsize; i++) newidxp[i] = i;
  auto ht = unique_hashtable<size_t, size_t>(idx_dic, newidx);
  std::vector<size_t> missed; // not used; should be same as nulls
  auto ret = ht.lookup(val, missed);
  auto retp = ret.data();
  auto nullsp = nulls.data();
  auto nulls_size = nulls.size();
  auto null_value = std::numeric_limits<size_t>::max();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < nulls_size; i++) {
    retp[nullsp[i]] = null_value;
  }
  return ret;
}

vector<size_t> extract_idx(vector<pair<string,size_t>>& v) {
  size_t size = v.size();
  vector<size_t> ret(size);
  for(size_t i = 0; i < size; i++) ret[i] = v[i].second;
  return ret;
}

// in typed_dfcolumn_string.cc
vector<pair<string, size_t>> append_idx(vector<string>& vs);

typed_dfcolumn<size_t>
typed_dfcolumn<string>::sort_prepare() {
  typed_dfcolumn<size_t> rescol;
  auto with_idx = dic_idx->map(append_idx);
  auto with_idx_dv = with_idx.moveto_dvector<pair<string,size_t>>();
  auto with_idx_sorted = with_idx_dv.sort().moveto_node_local();
  auto idx_dic = broadcast(with_idx_sorted.map(extract_idx).
                           viewas_dvector<size_t>().gather());
  rescol.val = val.map(convert_sorted_idx, idx_dic, nulls);
  rescol.nulls = nulls; // copy
  return rescol;
}

shared_ptr<dfcolumn>
typed_dfcolumn<string>::sort(node_local<vector<size_t>>& idx) {
  auto tmpcol = sort_prepare();
  return tmpcol.sort(idx);
}

shared_ptr<dfcolumn>
typed_dfcolumn<string>::sort_desc(node_local<vector<size_t>>& idx) {
  auto tmpcol = sort_prepare();
  return tmpcol.sort_desc(idx);
}

shared_ptr<dfcolumn>
typed_dfcolumn<string>::sort_with_idx(node_local<vector<size_t>>& idx,
                                      node_local<vector<size_t>>& res_idx) {
  auto tmpcol = sort_prepare();
  return tmpcol.sort_with_idx(idx, res_idx);
}

shared_ptr<dfcolumn>
typed_dfcolumn<string>::
sort_with_idx_desc(node_local<vector<size_t>>& idx,
                   node_local<vector<size_t>>& res_idx) {
  auto tmpcol = sort_prepare();
  return tmpcol.sort_with_idx_desc(idx, res_idx);
}

// ----- datetime -----
std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::sort(node_local<std::vector<size_t>>& res_idx) {
  auto to_sort = val; // need to copy because of implace sort
  auto idx = make_node_local_allocate<std::vector<size_t>>();
  auto part_cand =
    to_sort.map(dfcolumn_sort_local<datetime_t>, idx).
    reduce(set_merge<datetime_t>);
  size_t nodesize = get_nodesize();
  std::vector<datetime_t> part(nodesize-1);
  size_t part_cand_size = part_cand.size();
  datetime_t* partp = &part[0];
  datetime_t* part_candp = &part_cand[0];
  for(size_t i = 0; i < nodesize - 1; i++) {
    auto idx = std::min((i+1) * nodesize, part_cand_size - 1);
    partp[i] = part_candp[idx];
  }
  auto part_val =
    make_node_local_allocate<std::vector<std::vector<datetime_t>>>();
  auto part_idx = make_node_local_allocate<std::vector<std::vector<size_t>>>();
  to_sort.mapv(dfcolumn_sort_partition<datetime_t>, idx, part_val, part_idx,
               broadcast(part));
  auto exchanged_val = alltoall_exchange(part_val);
  auto exchanged_idx = alltoall_exchange(part_idx);
  auto res_val = make_node_local_allocate<std::vector<datetime_t>>();
  res_idx = make_node_local_allocate<std::vector<size_t>>();
  exchanged_val.mapv(set_multimerge_pair<datetime_t,size_t>, exchanged_idx,
                     res_val, res_idx);
  auto ret = std::make_shared<typed_dfcolumn<datetime>>();
  if(contain_nulls) {
    // if contain_nulls, sorted column is not reused,
    // since calculating null posistion is not trivial
    ret->val = make_node_local_allocate<std::vector<datetime_t>>();
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  } else {
    ret->val = std::move(res_val);
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  }
  return ret;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::sort_desc(node_local<std::vector<size_t>>& res_idx) {
  auto to_sort = val; // need to copy because of implace sort
  auto idx = make_node_local_allocate<std::vector<size_t>>();
  auto part_cand =
    to_sort.map(dfcolumn_sort_local_desc<datetime_t>, idx).
    reduce(set_merge_desc<datetime_t>);
  size_t nodesize = get_nodesize();
  std::vector<datetime_t> part(nodesize-1);
  size_t part_cand_size = part_cand.size();
  datetime_t* partp = &part[0];
  datetime_t* part_candp = &part_cand[0];
  for(size_t i = 0; i < nodesize - 1; i++) {
    auto idx = std::min((i+1) * nodesize, part_cand_size - 1);
    partp[i] = part_candp[idx];
  }
  auto part_val =
    make_node_local_allocate<std::vector<std::vector<datetime_t>>>();
  auto part_idx = make_node_local_allocate<std::vector<std::vector<size_t>>>();
  to_sort.mapv(dfcolumn_sort_partition_desc<datetime_t>, idx, part_val,
               part_idx, broadcast(part));
  auto exchanged_val = alltoall_exchange(part_val);
  auto exchanged_idx = alltoall_exchange(part_idx);
  auto res_val = make_node_local_allocate<std::vector<datetime_t>>();
  res_idx = make_node_local_allocate<std::vector<size_t>>();
  exchanged_val.mapv(set_multimerge_pair_desc<datetime_t,size_t>, exchanged_idx,
                     res_val, res_idx);
  auto ret = std::make_shared<typed_dfcolumn<datetime>>();
  if(contain_nulls) {
    // if contain_nulls, sorted column is not reused,
    // since calculating null posistion is not trivial
    ret->val = make_node_local_allocate<std::vector<datetime_t>>();
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  } else {
    ret->val = std::move(res_val);
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  }
  return ret;
}

std::shared_ptr<dfcolumn> typed_dfcolumn<datetime>::
sort_with_idx(node_local<std::vector<size_t>>& idx,
              node_local<std::vector<size_t>>& res_idx) {
  auto to_sort = val; // need to copy because of implace sort
  auto part_cand =
    to_sort.map(dfcolumn_sort_with_idx_local<datetime_t>, idx).
    reduce(set_merge<datetime_t>);
  size_t nodesize = get_nodesize();
  std::vector<datetime_t> part(nodesize-1);
  size_t part_cand_size = part_cand.size();
  datetime_t* partp = &part[0];
  datetime_t* part_candp = &part_cand[0];
  for(size_t i = 0; i < nodesize - 1; i++) {
    auto idx = std::min((i+1) * nodesize, part_cand_size - 1);
    partp[i] = part_candp[idx];
  }
  auto part_val =
    make_node_local_allocate<std::vector<std::vector<datetime_t>>>();
  auto part_idx = make_node_local_allocate<std::vector<std::vector<size_t>>>();
  to_sort.mapv(dfcolumn_sort_partition<datetime_t>, idx, part_val, part_idx,
               broadcast(part));
  auto exchanged_val = alltoall_exchange(part_val);
  auto exchanged_idx = alltoall_exchange(part_idx);
  auto res_val = make_node_local_allocate<std::vector<datetime_t>>();
  res_idx = make_node_local_allocate<std::vector<size_t>>();
  exchanged_val.mapv(set_multimerge_pair<datetime_t,size_t>, exchanged_idx,
                     res_val, res_idx);
  auto ret = std::make_shared<typed_dfcolumn<datetime>>();
  if(contain_nulls) {
    // if contain_nulls, sorted column is not reused,
    // since calculating null posistion is not trivial
    ret->val = make_node_local_allocate<std::vector<datetime_t>>();
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  } else {
    ret->val = std::move(res_val);
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  }
  return ret;
}

std::shared_ptr<dfcolumn> typed_dfcolumn<datetime>::
sort_with_idx_desc(node_local<std::vector<size_t>>& idx,
                   node_local<std::vector<size_t>>& res_idx) {
  auto to_sort = val; // need to copy because of implace sort
  auto part_cand =
    to_sort.map(dfcolumn_sort_with_idx_local_desc<datetime_t>, idx).
    reduce(set_merge_desc<datetime_t>);
  size_t nodesize = get_nodesize();
  std::vector<datetime_t> part(nodesize-1);
  size_t part_cand_size = part_cand.size();
  datetime_t* partp = &part[0];
  datetime_t* part_candp = &part_cand[0];
  for(size_t i = 0; i < nodesize - 1; i++) {
    auto idx = std::min((i+1) * nodesize, part_cand_size - 1);
    partp[i] = part_candp[idx];
  }
  auto part_val =
    make_node_local_allocate<std::vector<std::vector<datetime_t>>>();
  auto part_idx = make_node_local_allocate<std::vector<std::vector<size_t>>>();
  to_sort.mapv(dfcolumn_sort_partition_desc<datetime_t>, idx, part_val,
               part_idx, broadcast(part));
  auto exchanged_val = alltoall_exchange(part_val);
  auto exchanged_idx = alltoall_exchange(part_idx);
  auto res_val = make_node_local_allocate<std::vector<datetime_t>>();
  res_idx = make_node_local_allocate<std::vector<size_t>>();
  exchanged_val.mapv(set_multimerge_pair_desc<datetime_t,size_t>, exchanged_idx,
                     res_val, res_idx);
  auto ret = std::make_shared<typed_dfcolumn<datetime>>();
  if(contain_nulls) {
    // if contain_nulls, sorted column is not reused,
    // since calculating null posistion is not trivial
    ret->val = make_node_local_allocate<std::vector<datetime_t>>();
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  } else {
    ret->val = std::move(res_val);
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  }
  return ret;
}

}
