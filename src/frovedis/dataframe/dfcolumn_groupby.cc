#include "dfcolumn.hpp"

#include <limits>
#include <climits>
#include "../core/vector_operations.hpp"
#include "dfcolumn_helper.hpp"

using namespace std;

namespace frovedis {

void create_merge_map(std::vector<size_t>& nodeid,
                      std::vector<size_t>& split,
                      std::vector<std::vector<size_t>>& merge_map);

template <class T>
void group_by_impl(node_local<std::vector<T>>& val,
                   node_local<std::vector<size_t>>& nulls,
                   node_local<std::vector<size_t>>& local_idx,
                   node_local<std::vector<size_t>>& split_idx,
                   node_local<std::vector<std::vector<size_t>>>& hash_divide,
                   node_local<std::vector<std::vector<size_t>>>& merge_map,
                   node_local<std::vector<T>>& retval,
                   node_local<std::vector<size_t>>& retnulls) {
  auto hashsplit_group = val.map
    (+[](std::vector<T>& val,
         std::vector<size_t>& idx,
         std::vector<size_t>& split,
         std::vector<std::vector<size_t>>& hash_divide) {
      // null is also used, so extract is not used
      auto valp = val.data();
      auto idxp = idx.data();
      auto size = idx.size();
      std::vector<T> newval(size);
      auto newvalp = newval.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
      for(size_t i = 0; i < size; i++) {newvalp[i] = valp[idxp[i]];}
      radix_sort(newvalp, idxp, size); 
      split = set_separate(newval);
      auto split_size = split.size();
      std::vector<T> group(split_size-1);
      auto groupp = group.data();
      auto splitp = split.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
      for(size_t i = 0; i < split_size-1; i++) {
        groupp[i] = newvalp[splitp[i]];
      }
      std::vector<size_t> iota(split_size-1);
      auto iotap = iota.data();
      for(size_t i = 0; i < split_size-1; i++) iotap[i] = i;
      std::vector<std::vector<T>> hashsplit_group;
      split_by_hash(group, hashsplit_group, iota, hash_divide);
      return hashsplit_group;
    }, local_idx, split_idx, hash_divide);
  auto exchanged_group = alltoall_exchange(hashsplit_group);
  retval = exchanged_group.map
    (+[](std::vector<std::vector<T>>& exchanged_group,
         std::vector<std::vector<size_t>>& merge_map,
         std::vector<size_t>& nulls) {
      size_t nodesize = exchanged_group.size();
      size_t total = 0;
      for(size_t i = 0; i < nodesize; i++) {
        total += exchanged_group[i].size();
      }
      std::vector<size_t> nodeid(total);
      auto nodeidp = nodeid.data();
      size_t current = 0;
      for(size_t i = 0; i < nodesize; i++) {
        auto current_size = exchanged_group[i].size();
        for(size_t j = 0; j < current_size; j++) {
          nodeidp[current + j] = i;
        }
        current += current_size;
      }
      auto flat_exchanged_group = flatten(exchanged_group);
      auto flat_exchanged_groupp = flat_exchanged_group.data();
      radix_sort(flat_exchanged_groupp, nodeidp, total);
      auto sep = set_separate(flat_exchanged_group);
      merge_map.resize(nodesize);
      for(size_t i = 0; i < nodesize; i++) {
        merge_map[i].resize(exchanged_group[i].size());
      }
      create_merge_map(nodeid, sep, merge_map);
      auto sepsize = sep.size();
      std::vector<T> retval(sepsize-1);
      auto retvalp = retval.data();
      auto sepp = sep.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
      for(size_t i = 0; i < sepsize-1; i++) {
        retvalp[i] = flat_exchanged_groupp[sepp[i]];
      }
      if(sepsize > 1 && // there is data and at least one group
         retvalp[sepsize-2] == std::numeric_limits<T>::max()) {
        nulls.resize(1);
        nulls[0] = sepsize-2;
      }
      return retval;
    }, merge_map, retnulls);
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::group_by
(node_local<std::vector<size_t>>& local_idx,
 node_local<std::vector<size_t>>& split_idx,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map) {
  auto ret = std::make_shared<typed_dfcolumn<T>>();
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  group_by_impl(val, nulls, local_idx, split_idx, hash_divide, merge_map,
                ret->val, ret->nulls);
  ret->contain_nulls_check();
  return ret;
}

template <class T>
void multi_group_by_sort_helper(std::vector<T>& val,
                                std::vector<size_t>& local_idx) {
  size_t size = local_idx.size();
  std::vector<T> val2(size);
  T* valp = &val[0];
  T* val2p = &val2[0];
  size_t* local_idxp = &local_idx[0];
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < size; i++) {
    val2p[i] = valp[local_idxp[i]];
  }
  radix_sort(val2p, local_idxp, size);
}

template <class T>
void
typed_dfcolumn<T>::multi_group_by_sort
(node_local<std::vector<size_t>>& local_idx) {
  val.mapv(multi_group_by_sort_helper<T>, local_idx);
}

template <class T>
std::vector<size_t> 
multi_group_by_sort_split_helper(std::vector<T>& val,
                                 std::vector<size_t>& local_idx) {
  size_t size = local_idx.size();
  std::vector<T> val2(size);
  T* valp = &val[0];
  T* val2p = &val2[0];
  size_t* local_idxp = &local_idx[0];
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < size; i++) {
    val2p[i] = valp[local_idxp[i]];
  }
  radix_sort(val2p, local_idxp, size);
  return set_separate(val2);
}

template <class T>
node_local<std::vector<size_t>>
typed_dfcolumn<T>::multi_group_by_sort_split
(node_local<std::vector<size_t>>& local_idx) {
  return val.map(multi_group_by_sort_split_helper<T>, local_idx);
}

template <class T>
std::vector<size_t>
multi_group_by_split_helper(std::vector<T>& val,
                            std::vector<size_t>& local_idx) {
  size_t size = local_idx.size();
  std::vector<T> val2(size);
  T* valp = &val[0];
  T* val2p = &val2[0];
  size_t* local_idxp = &local_idx[0];
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < size; i++) {
    val2p[i] = valp[local_idxp[i]];
  }
  return set_separate(val2);
}

template <class T>
node_local<std::vector<size_t>>
typed_dfcolumn<T>::multi_group_by_split
(node_local<std::vector<size_t>>& local_idx) {
  return val.map(multi_group_by_split_helper<T>, local_idx);
}

template <class T>
std::vector<T>
multi_group_by_extract_helper(std::vector<T>& val,
                              std::vector<size_t>& local_idx,
                              std::vector<size_t>& split_idx) {
  auto valp = val.data();
  auto local_idxp = local_idx.data();
  auto split_idxp = split_idx.data();
  auto size = split_idx.size();
  std::vector<T> ret(size-1);
  auto retp = ret.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < size-1; i++) {
    retp[i] = valp[local_idxp[split_idxp[i]]];
  }
  return ret;
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::multi_group_by_extract
(node_local<std::vector<size_t>>& local_idx,
 node_local<std::vector<size_t>>& split_idx,
 bool check_nulls) {
  auto ret = std::make_shared<typed_dfcolumn<T>>();
  ret->val = val.map(multi_group_by_extract_helper<T>, local_idx, split_idx);
  ret->contain_nulls = contain_nulls;
  if(contain_nulls && check_nulls) {
    ret->nulls = ret->val.map(+[](std::vector<T>& val) {
        return find_value(val, std::numeric_limits<T>::max());
      });
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  }
  return ret;
}

template <class T>
node_local<std::vector<size_t>> 
typed_dfcolumn<T>::calc_hash_base() {
  return val.map(calc_hash_base_helper<T>);
}

template <class T>
void 
typed_dfcolumn<T>::calc_hash_base(node_local<std::vector<size_t>>& hash_base,
                                  int shift) {
  val.mapv(calc_hash_base_helper2<T>(shift), hash_base);
}

template <class T>
node_local<std::vector<T>>
multi_group_by_exchange_helper
(node_local<std::vector<T>>& val,
 node_local<std::vector<std::vector<size_t>>>& hash_divide) {
  auto tmp = val.map
    (+[](std::vector<T>& val,
         std::vector<std::vector<size_t>>& hash_divide) {
      auto valp = val.data();
      std::vector<std::vector<T>> ret(hash_divide.size());
      for(size_t i = 0; i < hash_divide.size(); i++) {
        auto size = hash_divide[i].size();
        ret[i].resize(size);
        auto retp = ret[i].data();
        auto hash_dividep = hash_divide[i].data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
        for(size_t j = 0; j < size; j++) {
          retp[j] = valp[hash_dividep[j]];
        }
      }
      return ret;
    }, hash_divide);
  auto exchanged = alltoall_exchange(tmp);
  return exchanged.map(flatten<T>);
}

template <class T>
std::shared_ptr<dfcolumn> 
typed_dfcolumn<T>::multi_group_by_exchange
(node_local<std::vector<std::vector<size_t>>>& hash_divide) {
  auto ret = std::make_shared<typed_dfcolumn<T>>();
  ret->val = multi_group_by_exchange_helper(val, hash_divide);
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  ret->contain_nulls = contain_nulls;
  return ret;
}



template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::group_by
(node_local<std::vector<size_t>>& local_idx,
 node_local<std::vector<size_t>>& split_idx,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::group_by
(node_local<std::vector<size_t>>& local_idx,
 node_local<std::vector<size_t>>& split_idx,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::group_by
(node_local<std::vector<size_t>>& local_idx,
 node_local<std::vector<size_t>>& split_idx,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::group_by
(node_local<std::vector<size_t>>& local_idx,
 node_local<std::vector<size_t>>& split_idx,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::group_by
(node_local<std::vector<size_t>>& local_idx,
 node_local<std::vector<size_t>>& split_idx,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::group_by
(node_local<std::vector<size_t>>& local_idx,
 node_local<std::vector<size_t>>& split_idx,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map);

template void
typed_dfcolumn<int>::multi_group_by_sort
(node_local<std::vector<size_t>>& local_idx);
template void
typed_dfcolumn<unsigned int>::multi_group_by_sort
(node_local<std::vector<size_t>>& local_idx);
template void
typed_dfcolumn<long>::multi_group_by_sort
(node_local<std::vector<size_t>>& local_idx);
template void
typed_dfcolumn<unsigned long>::multi_group_by_sort
(node_local<std::vector<size_t>>& local_idx);
template void
typed_dfcolumn<float>::multi_group_by_sort
(node_local<std::vector<size_t>>& local_idx);
template void
typed_dfcolumn<double>::multi_group_by_sort
(node_local<std::vector<size_t>>& local_idx);

template node_local<std::vector<size_t>>
typed_dfcolumn<int>::multi_group_by_sort_split
(node_local<std::vector<size_t>>& local_idx);
template node_local<std::vector<size_t>>
typed_dfcolumn<unsigned int>::multi_group_by_sort_split
(node_local<std::vector<size_t>>& local_idx);
template node_local<std::vector<size_t>>
typed_dfcolumn<long>::multi_group_by_sort_split
(node_local<std::vector<size_t>>& local_idx);
template node_local<std::vector<size_t>>
typed_dfcolumn<unsigned long>::multi_group_by_sort_split
(node_local<std::vector<size_t>>& local_idx);
template node_local<std::vector<size_t>>
typed_dfcolumn<float>::multi_group_by_sort_split
(node_local<std::vector<size_t>>& local_idx);
template node_local<std::vector<size_t>>
typed_dfcolumn<double>::multi_group_by_sort_split
(node_local<std::vector<size_t>>& local_idx);

template node_local<std::vector<size_t>>
typed_dfcolumn<int>::multi_group_by_split
(node_local<std::vector<size_t>>& local_idx);
template node_local<std::vector<size_t>>
typed_dfcolumn<unsigned int>::multi_group_by_split
(node_local<std::vector<size_t>>& local_idx);
template node_local<std::vector<size_t>>
typed_dfcolumn<long>::multi_group_by_split
(node_local<std::vector<size_t>>& local_idx);
template node_local<std::vector<size_t>>
typed_dfcolumn<unsigned long>::multi_group_by_split
(node_local<std::vector<size_t>>& local_idx);
template node_local<std::vector<size_t>>
typed_dfcolumn<float>::multi_group_by_split
(node_local<std::vector<size_t>>& local_idx);
template node_local<std::vector<size_t>>
typed_dfcolumn<double>::multi_group_by_split
(node_local<std::vector<size_t>>& local_idx);

template
std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::multi_group_by_extract
(node_local<std::vector<size_t>>& local_idx,
 node_local<std::vector<size_t>>& split_idx,
 bool check_nulls);
template
std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::multi_group_by_extract
(node_local<std::vector<size_t>>& local_idx,
 node_local<std::vector<size_t>>& split_idx,
 bool check_nulls);
template
std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::multi_group_by_extract
(node_local<std::vector<size_t>>& local_idx,
 node_local<std::vector<size_t>>& split_idx,
 bool check_nulls);
template
std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::multi_group_by_extract
(node_local<std::vector<size_t>>& local_idx,
 node_local<std::vector<size_t>>& split_idx,
 bool check_nulls);
template
std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::multi_group_by_extract
(node_local<std::vector<size_t>>& local_idx,
 node_local<std::vector<size_t>>& split_idx,
 bool check_nulls);
template
std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::multi_group_by_extract
(node_local<std::vector<size_t>>& local_idx,
 node_local<std::vector<size_t>>& split_idx,
 bool check_nulls);

template node_local<std::vector<size_t>> 
typed_dfcolumn<int>::calc_hash_base();
template node_local<std::vector<size_t>> 
typed_dfcolumn<unsigned int>::calc_hash_base();
template node_local<std::vector<size_t>> 
typed_dfcolumn<long>::calc_hash_base();
template node_local<std::vector<size_t>> 
typed_dfcolumn<unsigned long>::calc_hash_base();
template node_local<std::vector<size_t>> 
typed_dfcolumn<float>::calc_hash_base();
template node_local<std::vector<size_t>> 
typed_dfcolumn<double>::calc_hash_base();

template void 
typed_dfcolumn<int>::calc_hash_base
(node_local<std::vector<size_t>>& hash_base, int shift);
template void 
typed_dfcolumn<unsigned int>::calc_hash_base
(node_local<std::vector<size_t>>& hash_base, int shift);
template void 
typed_dfcolumn<long>::calc_hash_base
(node_local<std::vector<size_t>>& hash_base, int shift);
template void 
typed_dfcolumn<unsigned long>::calc_hash_base
(node_local<std::vector<size_t>>& hash_base, int shift);
template void 
typed_dfcolumn<float>::calc_hash_base
(node_local<std::vector<size_t>>& hash_base, int shift);
template void 
typed_dfcolumn<double>::calc_hash_base
(node_local<std::vector<size_t>>& hash_base, int shift);

template std::shared_ptr<dfcolumn> 
typed_dfcolumn<int>::multi_group_by_exchange
(node_local<std::vector<std::vector<size_t>>>& hash_divide);
template std::shared_ptr<dfcolumn> 
typed_dfcolumn<unsigned int>::multi_group_by_exchange
(node_local<std::vector<std::vector<size_t>>>& hash_divide);
template std::shared_ptr<dfcolumn> 
typed_dfcolumn<long>::multi_group_by_exchange
(node_local<std::vector<std::vector<size_t>>>& hash_divide);
template std::shared_ptr<dfcolumn> 
typed_dfcolumn<unsigned long>::multi_group_by_exchange
(node_local<std::vector<std::vector<size_t>>>& hash_divide);
template std::shared_ptr<dfcolumn> 
typed_dfcolumn<float>::multi_group_by_exchange
(node_local<std::vector<std::vector<size_t>>>& hash_divide);
template std::shared_ptr<dfcolumn> 
typed_dfcolumn<double>::multi_group_by_exchange
(node_local<std::vector<std::vector<size_t>>>& hash_divide);


// ----- dic_string -----

std::shared_ptr<dfcolumn>
typed_dfcolumn<dic_string>::group_by
(node_local<std::vector<size_t>>& local_idx,
 node_local<std::vector<size_t>>& split_idx,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map) {
  auto ret = std::make_shared<typed_dfcolumn<dic_string>>();
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  group_by_impl(val, nulls, local_idx, split_idx, hash_divide, merge_map,
                ret->val, ret->nulls);
  ret->dic = dic;
  ret->contain_nulls_check();
  return ret;
}

void typed_dfcolumn<dic_string>::multi_group_by_sort
(node_local<std::vector<size_t>>& local_idx) {
  val.mapv(multi_group_by_sort_helper<size_t>, local_idx);
}

node_local<std::vector<size_t>>
typed_dfcolumn<dic_string>::multi_group_by_sort_split
(node_local<std::vector<size_t>>& local_idx) {
  return val.map(multi_group_by_sort_split_helper<size_t>, local_idx);
}

node_local<std::vector<size_t>>
typed_dfcolumn<dic_string>::multi_group_by_split
(node_local<std::vector<size_t>>& local_idx) {
  return val.map(multi_group_by_split_helper<size_t>, local_idx);
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<dic_string>::multi_group_by_extract
(node_local<std::vector<size_t>>& local_idx,
 node_local<std::vector<size_t>>& split_idx,
 bool check_nulls) {
  auto ret = std::make_shared<typed_dfcolumn<dic_string>>();
  ret->val = val.map(multi_group_by_extract_helper<size_t>, local_idx, split_idx);
  ret->contain_nulls = contain_nulls;
  if(contain_nulls && check_nulls) {
    ret->nulls = ret->val.map(+[](std::vector<size_t>& val) {
        return find_value(val, std::numeric_limits<size_t>::max());        
      });
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  }
  ret->dic = dic;
  return ret;
}

node_local<std::vector<size_t>>
typed_dfcolumn<dic_string>::calc_hash_base() {
  return val.map(calc_hash_base_helper<size_t>);
}

void typed_dfcolumn<dic_string>::calc_hash_base
(node_local<std::vector<size_t>>& hash_base, int shift) {
  val.mapv(calc_hash_base_helper2<size_t>(shift), hash_base);
}

std::shared_ptr<dfcolumn> 
typed_dfcolumn<dic_string>::multi_group_by_exchange
(node_local<std::vector<std::vector<size_t>>>& hash_divide) {
  auto ret = std::make_shared<typed_dfcolumn<dic_string>>();
  ret->val = multi_group_by_exchange_helper(val, hash_divide);
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  ret->contain_nulls = contain_nulls;
  ret->dic = dic;
  return ret;
}

// ----- string -----
std::shared_ptr<dfcolumn>
typed_dfcolumn<string>::group_by
(node_local<std::vector<size_t>>& local_idx,
 node_local<std::vector<size_t>>& split_idx,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map) {
  auto ret = std::make_shared<typed_dfcolumn<string>>();
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  group_by_impl(val, nulls, local_idx, split_idx, hash_divide, merge_map,
                ret->val, ret->nulls);
  ret->dic = dic;
  ret->dic_idx = dic_idx;
  ret->contain_nulls_check();
  return ret;
}

void typed_dfcolumn<string>::multi_group_by_sort
(node_local<std::vector<size_t>>& local_idx) {
  val.mapv(multi_group_by_sort_helper<size_t>, local_idx);
}

node_local<std::vector<size_t>>
typed_dfcolumn<string>::multi_group_by_sort_split
(node_local<std::vector<size_t>>& local_idx) {
  return val.map(multi_group_by_sort_split_helper<size_t>, local_idx);
}

node_local<std::vector<size_t>>
typed_dfcolumn<string>::multi_group_by_split
(node_local<std::vector<size_t>>& local_idx) {
  return val.map(multi_group_by_split_helper<size_t>, local_idx);
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<string>::multi_group_by_extract
(node_local<std::vector<size_t>>& local_idx,
 node_local<std::vector<size_t>>& split_idx,
 bool check_nulls) {
  auto ret = std::make_shared<typed_dfcolumn<string>>();
  ret->val = val.map(multi_group_by_extract_helper<size_t>, local_idx, split_idx);
  ret->contain_nulls = contain_nulls;
  if(contain_nulls && check_nulls) {
    ret->nulls = ret->val.map(+[](std::vector<size_t>& val) {
        return find_value(val, std::numeric_limits<size_t>::max());        
      });
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  }
  ret->dic = dic;
  ret->dic_idx = dic_idx;
  return ret;
}

node_local<std::vector<size_t>>
typed_dfcolumn<string>::calc_hash_base() {
  return val.map(calc_hash_base_helper<size_t>);
}

void typed_dfcolumn<string>::calc_hash_base
(node_local<std::vector<size_t>>& hash_base, int shift) {
  val.mapv(calc_hash_base_helper2<size_t>(shift), hash_base);
}

std::shared_ptr<dfcolumn> 
typed_dfcolumn<string>::multi_group_by_exchange
(node_local<std::vector<std::vector<size_t>>>& hash_divide) {
  auto ret = std::make_shared<typed_dfcolumn<string>>();
  ret->val = multi_group_by_exchange_helper(val, hash_divide);
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  ret->contain_nulls = contain_nulls;
  ret->dic = dic;
  ret->dic_idx = dic_idx;
  return ret;
}


// ----- datetime -----

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::group_by
(node_local<std::vector<size_t>>& local_idx,
 node_local<std::vector<size_t>>& split_idx,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map) {
  auto ret = std::make_shared<typed_dfcolumn<datetime>>();
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  group_by_impl(val, nulls, local_idx, split_idx, hash_divide, merge_map,
                ret->val, ret->nulls);
  ret->contain_nulls_check();
  return ret;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::multi_group_by_extract
(node_local<std::vector<size_t>>& local_idx,
 node_local<std::vector<size_t>>& split_idx,
 bool check_nulls) {
  auto ret = std::make_shared<typed_dfcolumn<datetime>>();
  ret->val =
    val.map(multi_group_by_extract_helper<datetime_t>, local_idx, split_idx);
  ret->contain_nulls = contain_nulls;
  if(contain_nulls && check_nulls) {
    ret->nulls = ret->val.map(+[](std::vector<datetime_t>& val) {
        return find_value(val, std::numeric_limits<datetime_t>::max());
      });
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  }
  return ret;
}

std::shared_ptr<dfcolumn> 
typed_dfcolumn<datetime>::multi_group_by_exchange
(node_local<std::vector<std::vector<size_t>>>& hash_divide) {
  auto ret = std::make_shared<typed_dfcolumn<datetime>>();
  ret->val = multi_group_by_exchange_helper(val, hash_divide);
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  ret->contain_nulls = contain_nulls;
  return ret;
}


}
