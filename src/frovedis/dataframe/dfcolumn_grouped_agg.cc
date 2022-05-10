#include "dfcolumn.hpp"

#include <limits>
#include <climits>
#include "../core/vector_operations.hpp"
#include "dfcolumn_helper.hpp"

#define GROUPBY_VLEN 256
// count distinct uses same implementation for both VE and x86
#ifdef __ve__
#define GROUPBY_COUNT_DISTINCT_VLEN 256
#else
#define GROUPBY_COUNT_DISTINCT_VLEN 4
#endif
#define DONOT_ALLOW_MAX_AS_VALUE // for better performance

using namespace std;

namespace frovedis {
template <class T>
void group_by_vector_sum_helper
(const std::vector<T>& val,
 const std::vector<size_t>& split_idx,
 std::vector<T>& ret) {
  auto valp = val.data();
  auto split_idxp = split_idx.data();
  size_t size = val.size();
  if(size == 0) return;
  int valid[GROUPBY_VLEN];
  for(int i = 0; i < GROUPBY_VLEN; i++) valid[i] = true;
  size_t val_idx[GROUPBY_VLEN];
  size_t val_idx_stop[GROUPBY_VLEN];
  size_t out_idx[GROUPBY_VLEN];
  size_t next_group_idx[GROUPBY_VLEN];
  T current_val[GROUPBY_VLEN];
// never remove this vreg! this is needed following vovertake
#pragma _NEC vreg(valid)
#pragma _NEC vreg(val_idx)
#pragma _NEC vreg(val_idx_stop)
#pragma _NEC vreg(out_idx)
#pragma _NEC vreg(next_group_idx)
#pragma _NEC vreg(current_val)
  size_t group_size = split_idx.size() - 1;
  auto retp = ret.data();

  size_t each = ceil_div(size, size_t(GROUPBY_VLEN));
  if(each % 2 == 0) each++;
  auto start_it = split_idx.begin();
  for(size_t i = 0; i < GROUPBY_VLEN; i++) {
    start_it = std::lower_bound(start_it, split_idx.end()-1, each * i);
    if(start_it == split_idx.end()-1) {
      val_idx[i] = size;
      out_idx[i] = group_size;
      next_group_idx[i] = size;
    } else {
      val_idx[i] = *start_it;
      out_idx[i] = start_it - split_idx.begin();
      next_group_idx[i] = split_idxp[out_idx[i]+1];
    }
  }
  for(int i = 0; i < GROUPBY_VLEN - 1; i++) {
    val_idx_stop[i] = val_idx[i + 1];
  }
  val_idx_stop[GROUPBY_VLEN-1] = size;
  for(size_t i = 0; i < GROUPBY_VLEN; i++) {
    if(val_idx[i]  == val_idx_stop[i]) {
      valid[i] = false;
      out_idx[i] = group_size;
    }
    else valid[i] = true;
  }
  size_t max_size = 0;
  for(int i = 0; i < GROUPBY_VLEN; i++) {
    auto current = val_idx_stop[i] - val_idx[i];
    if(max_size < current) max_size = current;
  }
  for(size_t i = 0; i < GROUPBY_VLEN; i++) current_val[i] = 0; // sum

  auto shift_split_idxp = split_idxp + 1;
  for(size_t j = 0; j < max_size; j++) {
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(int i = 0; i < GROUPBY_VLEN; i++) {
      if(valid[i]) {
        if(val_idx[i] == next_group_idx[i]) {
          retp[out_idx[i]++] = current_val[i];
          current_val[i] = 0; // sum
          next_group_idx[i] = shift_split_idxp[out_idx[i]];
        }
        current_val[i] += valp[val_idx[i]]; // sum
        val_idx[i]++;
        if(val_idx[i] == val_idx_stop[i]) {valid[i] = false;}
      }
    }
  }
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(int i = 0; i < GROUPBY_VLEN; i++) {
    if(out_idx[i] != group_size) {
      retp[out_idx[i]] = current_val[i];
    }
  }
}

template <class T>
std::vector<std::vector<T>>
sum_helper(std::vector<T>& org_val,
           std::vector<size_t>& grouped_idx,
           std::vector<size_t>& idx_split,
           std::vector<std::vector<size_t>>& hash_divide,
           std::vector<size_t>& nulls) {
  T* org_valp = org_val.data();
  size_t* nullsp = nulls.data();
  size_t nullssize = nulls.size();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < nullssize; i++) {
    org_valp[nullsp[i]] = 0;
  }
  size_t valsize = grouped_idx.size();
  std::vector<T> val(valsize);
  auto valp = val.data();
  auto grouped_idxp = grouped_idx.data();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < valsize; i++) {
    valp[i] = org_valp[grouped_idxp[i]];
  }
  size_t splitsize = idx_split.size();
  std::vector<T> ret(splitsize-1);
  auto retp = ret.data();
  size_t* idx_splitp = idx_split.data();
#if defined(_SX) || defined(__ve__)
  if(valsize / splitsize > GROUPBY_VLEN * 2) {
    size_t end = 0;
    size_t start = 0;
    for(size_t i = 0; i < splitsize-1; i++) {
      start = end;
      end = idx_splitp[i+1];
      T total = 0;
      for(size_t j = start; j < end; j++) {
        total += valp[j];
      }
      retp[i] = total;
    }
  } else { // for improving vectorization
    group_by_vector_sum_helper(val, idx_split, ret);
  }
#else  
  size_t end = 0;
  size_t start = 0;
  for(size_t i = 0; i < splitsize-1; i++) {
    start = end;
    end = idx_splitp[i+1];
    T total = 0;
    for(size_t j = start; j < end; j++) {
      total += valp[j];
    }
    retp[i] = total;
  }
#endif
  auto nodesize = hash_divide.size();
  std::vector<std::vector<T>> hashed_ret(nodesize);
  for(size_t i = 0; i < nodesize; i++) {
    auto each_size = hash_divide[i].size();
    hashed_ret[i].resize(each_size);
    auto hash_dividep = hash_divide[i].data();
    auto hashed_retp = hashed_ret[i].data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t j = 0; j < each_size; j++) {
      hashed_retp[j] = retp[hash_dividep[j]];
    }
  }
  // restore the null value
  auto max = std::numeric_limits<T>::max();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < nullssize; i++) {
    org_valp[nullsp[i]] = max;
  }
  return hashed_ret;
}

template <class T>
std::vector<std::vector<size_t>>
count_helper(std::vector<T>& val,
             std::vector<size_t>& grouped_idx,
             std::vector<size_t>& idx_split,
             std::vector<std::vector<size_t>>& hash_divide,
             std::vector<size_t>& nulls);

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::sum
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes) {
  auto ret = std::make_shared<typed_dfcolumn<T>>();
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  auto local_agg = val.map(sum_helper<T>, local_grouped_idx, local_idx_split,
                           hash_divide, nulls);
  auto exchanged = alltoall_exchange(local_agg);
  auto newval = exchanged.map
    (+[](std::vector<std::vector<T>>& exchanged,
         std::vector<std::vector<size_t>>& merge_map,
         size_t row_size) {
      std::vector<T> newval(row_size);
      auto newvalp = newval.data();
      for(size_t i = 0; i < exchanged.size(); i++) {
        auto currentp = exchanged[i].data();
        auto current_size = exchanged[i].size();
        auto merge_mapp = merge_map[i].data();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
        for(size_t j = 0; j < current_size; j++) {
          newvalp[merge_mapp[j]] += currentp[j];
        }
      }
      return newval;
    }, merge_map, row_sizes);
  ret->val = std::move(newval);
  if(contain_nulls) {
    // in the case of sum, default value (= 0) cannot be used to check null
    auto local_agg = val.map(count_helper<T>, local_grouped_idx,
                             local_idx_split, hash_divide, nulls);
    auto exchanged = alltoall_exchange(local_agg);
    ret->nulls = exchanged.map
      (+[](std::vector<std::vector<size_t>>& exchanged,
           std::vector<std::vector<size_t>>& merge_map,
           size_t row_size,
           std::vector<T>& val) {
        std::vector<size_t> newval(row_size);
        auto newvalp = newval.data();
        for(size_t i = 0; i < exchanged.size(); i++) {
          auto currentp = exchanged[i].data();
          auto current_size = exchanged[i].size();
          auto merge_mapp = merge_map[i].data();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
          for(size_t j = 0; j < current_size; j++) {
            newvalp[merge_mapp[j]] += currentp[j];
          }
        }
        auto retnulls = find_value(newval, size_t(0));
        auto retnullsp = retnulls.data();
        auto retnullssize = retnulls.size();
        auto valp = val.data();
        auto max = std::numeric_limits<T>::max();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
        for(size_t i = 0; i < retnullssize; i++) {
          valp[retnullsp[i]] = max;
        }
        return retnulls;
      }, merge_map, row_sizes, ret->val);
    ret->contain_nulls_check();
  }
  return ret;
}


template <class T>
void group_by_vector_max_helper
(const std::vector<T>& val,
 const std::vector<size_t>& split_idx,
 std::vector<T>& ret) {
  auto valp = val.data();
  auto split_idxp = split_idx.data();
  size_t size = val.size();
  if(size == 0) return;
  int valid[GROUPBY_VLEN];
  for(int i = 0; i < GROUPBY_VLEN; i++) valid[i] = true;
  size_t val_idx[GROUPBY_VLEN];
  size_t val_idx_stop[GROUPBY_VLEN];
  size_t out_idx[GROUPBY_VLEN];
  size_t next_group_idx[GROUPBY_VLEN];
  T current_val[GROUPBY_VLEN];
#pragma _NEC vreg(valid)
#pragma _NEC vreg(val_idx)
#pragma _NEC vreg(val_idx_stop)
#pragma _NEC vreg(out_idx)
#pragma _NEC vreg(next_group_idx)
#pragma _NEC vreg(current_val)
  size_t group_size = split_idx.size() - 1;
  auto retp = ret.data();

  size_t each = ceil_div(size, size_t(GROUPBY_VLEN));
  if(each % 2 == 0) each++;
  auto start_it = split_idx.begin();
  for(size_t i = 0; i < GROUPBY_VLEN; i++) {
    start_it = std::lower_bound(start_it, split_idx.end()-1, each * i);
    if(start_it == split_idx.end()-1) {
      val_idx[i] = size;
      out_idx[i] = group_size;
      next_group_idx[i] = size;
    } else {
      val_idx[i] = *start_it;
      out_idx[i] = start_it - split_idx.begin();
      next_group_idx[i] = split_idxp[out_idx[i]+1];
    }
  }
  for(int i = 0; i < GROUPBY_VLEN - 1; i++) {
    val_idx_stop[i] = val_idx[i + 1];
  }
  val_idx_stop[GROUPBY_VLEN-1] = size;
  for(size_t i = 0; i < GROUPBY_VLEN; i++) {
    if(val_idx[i]  == val_idx_stop[i]) {
      valid[i] = false;
      out_idx[i] = group_size;
    }
    else valid[i] = true;
  }
  size_t max_size = 0;
  for(int i = 0; i < GROUPBY_VLEN; i++) {
    auto current = val_idx_stop[i] - val_idx[i];
    if(max_size < current) max_size = current;
  }
  auto min = std::numeric_limits<T>::lowest();
  for(size_t i = 0; i < GROUPBY_VLEN; i++) current_val[i] = min; // max

  auto shift_split_idxp = split_idxp + 1;
  for(size_t j = 0; j < max_size; j++) {
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(int i = 0; i < GROUPBY_VLEN; i++) {
      if(valid[i]) {
        if(val_idx[i] == next_group_idx[i]) {
          retp[out_idx[i]++] = current_val[i];
          current_val[i] = min; // max
          next_group_idx[i] = shift_split_idxp[out_idx[i]];
        }
        if(current_val[i] < valp[val_idx[i]])
          current_val[i] = valp[val_idx[i]]; // max
        val_idx[i]++;
        if(val_idx[i] == val_idx_stop[i]) {valid[i] = false;}
      }
    }
  }
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(int i = 0; i < GROUPBY_VLEN; i++) {
    if(out_idx[i] != group_size) {
      retp[out_idx[i]] = current_val[i];
    }
  }
}

template <class T>
std::vector<std::vector<T>>
max_helper(std::vector<T>& org_val,
           std::vector<size_t>& grouped_idx,
           std::vector<size_t>& idx_split,
           std::vector<std::vector<size_t>>& hash_divide,
           std::vector<size_t>& nulls) {
  T* org_valp = org_val.data();
  size_t* nullsp = nulls.data();
  size_t nullssize = nulls.size();
  auto min = std::numeric_limits<T>::lowest();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < nullssize; i++) {
    org_valp[nullsp[i]] = min;
  }
  size_t valsize = grouped_idx.size();
  std::vector<T> val(valsize);
  auto valp = val.data();
  auto grouped_idxp = grouped_idx.data();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < valsize; i++) {
    valp[i] = org_valp[grouped_idxp[i]];
  }
  size_t splitsize = idx_split.size();
  std::vector<T> ret(splitsize-1);
  auto retp = ret.data();
  size_t* idx_splitp = idx_split.data();
#if defined(_SX) || defined(__ve__)
  if(valsize / splitsize > GROUPBY_VLEN * 2) {
    size_t end = 0;
    size_t start = 0;
    for(size_t i = 0; i < splitsize-1; i++) {
      start = end;
      end = idx_splitp[i+1];
      T max = std::numeric_limits<T>::lowest();
      for(size_t j = start; j < end; j++) {
        if(max < valp[j]) max = valp[j];
      }
      retp[i] = max;
    }
  } else {
    group_by_vector_max_helper(val, idx_split, ret);
  }
#else  
  size_t end = 0;
  size_t start = 0;
  for(size_t i = 0; i < splitsize-1; i++) {
    start = end;
    end = idx_splitp[i+1];
    T max = std::numeric_limits<T>::lowest();
    for(size_t j = start; j < end; j++) {
      if(max < valp[j]) max = valp[j];
    }
    retp[i] = max;
  }
#endif
  auto nodesize = hash_divide.size();
  std::vector<std::vector<T>> hashed_ret(nodesize);
  for(size_t i = 0; i < nodesize; i++) {
    auto each_size = hash_divide[i].size();
    hashed_ret[i].resize(each_size);
    auto hash_dividep = hash_divide[i].data();
    auto hashed_retp = hashed_ret[i].data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t j = 0; j < each_size; j++) {
      hashed_retp[j] = retp[hash_dividep[j]];
    }
  }
  // restore the null value
  auto max = std::numeric_limits<T>::max();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < nullssize; i++) {
    org_valp[nullsp[i]] = max;
  }
  return hashed_ret;
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::max
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes) {
  auto ret = std::make_shared<typed_dfcolumn<T>>();
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  auto local_agg = val.map(max_helper<T>, local_grouped_idx, local_idx_split,
                           hash_divide, nulls);
  auto exchanged = alltoall_exchange(local_agg);
  auto newval = exchanged.map
    (+[](std::vector<std::vector<T>>& exchanged,
         std::vector<std::vector<size_t>>& merge_map,
         size_t row_size) {
      std::vector<T> newval(row_size);
      auto newvalp = newval.data();
      auto min = std::numeric_limits<T>::lowest();
      for(size_t i = 0; i < row_size; i++) {
        newvalp[i] = min;
      }
      for(size_t i = 0; i < exchanged.size(); i++) {
        auto currentp = exchanged[i].data();
        auto current_size = exchanged[i].size();
        auto merge_mapp = merge_map[i].data();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
        for(size_t j = 0; j < current_size; j++) {
          if(newvalp[merge_mapp[j]] < currentp[j])
            newvalp[merge_mapp[j]] = currentp[j];
        }
      }
      return newval;
    }, merge_map, row_sizes);
  ret->val = std::move(newval);
  if(contain_nulls) {
    ret->nulls = ret->val.map(+[](std::vector<T>& val) {
        auto nulls = find_value(val, std::numeric_limits<T>::lowest());
        auto valp = val.data();
        auto nullsp = nulls.data();
        auto size = nulls.size();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
        for(size_t i = 0; i < size; i++)
          valp[nullsp[i]] = std::numeric_limits<T>::max();
        return nulls;
      });
    ret->contain_nulls_check();
  }
  return ret;
}


template <class T>
void group_by_vector_min_helper
(const std::vector<T>& val,
 const std::vector<size_t>& split_idx,
 std::vector<T>& ret) {
  auto valp = val.data();
  auto split_idxp = split_idx.data();
  size_t size = val.size();
  if(size == 0) return;
  int valid[GROUPBY_VLEN];
  for(int i = 0; i < GROUPBY_VLEN; i++) valid[i] = true;
  size_t val_idx[GROUPBY_VLEN];
  size_t val_idx_stop[GROUPBY_VLEN];
  size_t out_idx[GROUPBY_VLEN];
  size_t next_group_idx[GROUPBY_VLEN];
  T current_val[GROUPBY_VLEN];
#pragma _NEC vreg(valid)
#pragma _NEC vreg(val_idx)
#pragma _NEC vreg(val_idx_stop)
#pragma _NEC vreg(out_idx)
#pragma _NEC vreg(next_group_idx)
#pragma _NEC vreg(current_val)
  size_t group_size = split_idx.size() - 1;
  auto retp = ret.data();

  size_t each = ceil_div(size, size_t(GROUPBY_VLEN));
  if(each % 2 == 0) each++;
  auto start_it = split_idx.begin();
  for(size_t i = 0; i < GROUPBY_VLEN; i++) {
    start_it = std::lower_bound(start_it, split_idx.end()-1, each * i);
    if(start_it == split_idx.end()-1) {
      val_idx[i] = size;
      out_idx[i] = group_size;
      next_group_idx[i] = size;
    } else {
      val_idx[i] = *start_it;
      out_idx[i] = start_it - split_idx.begin();
      next_group_idx[i] = split_idxp[out_idx[i]+1];
    }
  }
  for(int i = 0; i < GROUPBY_VLEN - 1; i++) {
    val_idx_stop[i] = val_idx[i + 1];
  }
  val_idx_stop[GROUPBY_VLEN-1] = size;
  for(size_t i = 0; i < GROUPBY_VLEN; i++) {
    if(val_idx[i]  == val_idx_stop[i]) {
      valid[i] = false;
      out_idx[i] = group_size;
    }
    else valid[i] = true;
  }
  size_t max_size = 0;
  for(int i = 0; i < GROUPBY_VLEN; i++) {
    auto current = val_idx_stop[i] - val_idx[i];
    if(max_size < current) max_size = current;
  }
  auto max = std::numeric_limits<T>::max();
  for(size_t i = 0; i < GROUPBY_VLEN; i++) current_val[i] = max; // min

  auto shift_split_idxp = split_idxp + 1;
  for(size_t j = 0; j < max_size; j++) {
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(int i = 0; i < GROUPBY_VLEN; i++) {
      if(valid[i]) {
        if(val_idx[i] == next_group_idx[i]) {
          retp[out_idx[i]++] = current_val[i];
          current_val[i] = max; // min
          next_group_idx[i] = shift_split_idxp[out_idx[i]];
        }
        if(current_val[i] > valp[val_idx[i]])
          current_val[i] = valp[val_idx[i]]; // min
        val_idx[i]++;
        if(val_idx[i] == val_idx_stop[i]) {valid[i] = false;}
      }
    }
  }
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(int i = 0; i < GROUPBY_VLEN; i++) {
    if(out_idx[i] != group_size) {
      retp[out_idx[i]] = current_val[i];
    }
  }
}

template <class T>
std::vector<std::vector<T>>
min_helper(std::vector<T>& org_val,
           std::vector<size_t>& grouped_idx,
           std::vector<size_t>& idx_split,
           std::vector<std::vector<size_t>>& hash_divide,
           std::vector<size_t>& nulls) {
  T* org_valp = org_val.data();
  size_t valsize = grouped_idx.size();
  std::vector<T> val(valsize);
  auto valp = val.data();
  auto grouped_idxp = grouped_idx.data();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < valsize; i++) {
    valp[i] = org_valp[grouped_idxp[i]];
  }
  size_t splitsize = idx_split.size();
  std::vector<T> ret(splitsize-1);
  auto retp = ret.data();
  size_t* idx_splitp = idx_split.data();
#if defined(_SX) || defined(__ve__)
  if(valsize / splitsize > GROUPBY_VLEN * 2) {
    size_t end = 0;
    size_t start = 0;
    for(size_t i = 0; i < splitsize-1; i++) {
      start = end;
      end = idx_splitp[i+1];
      T min = std::numeric_limits<T>::max();
      for(size_t j = start; j < end; j++) {
        if(min > valp[j]) min = valp[j];
      }
      retp[i] = min;
    }
  } else {
    group_by_vector_min_helper(val, idx_split, ret);
  }
#else  
  size_t end = 0;
  size_t start = 0;
  for(size_t i = 0; i < splitsize-1; i++) {
    start = end;
    end = idx_splitp[i+1];
    T min = std::numeric_limits<T>::max();
    for(size_t j = start; j < end; j++) {
      if(min > valp[j]) min = valp[j];
    }
    retp[i] = min;
  }
#endif
  auto nodesize = hash_divide.size();
  std::vector<std::vector<T>> hashed_ret(nodesize);
  for(size_t i = 0; i < nodesize; i++) {
    auto each_size = hash_divide[i].size();
    hashed_ret[i].resize(each_size);
    auto hash_dividep = hash_divide[i].data();
    auto hashed_retp = hashed_ret[i].data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t j = 0; j < each_size; j++) {
      hashed_retp[j] = retp[hash_dividep[j]];
    }
  }
  return hashed_ret;
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::min
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes) {
  auto ret = std::make_shared<typed_dfcolumn<T>>();
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  auto local_agg = val.map(min_helper<T>, local_grouped_idx, local_idx_split,
                           hash_divide, nulls);
  auto exchanged = alltoall_exchange(local_agg);
  auto newval = exchanged.map
    (+[](std::vector<std::vector<T>>& exchanged,
         std::vector<std::vector<size_t>>& merge_map,
         size_t row_size) {
      std::vector<T> newval(row_size);
      auto newvalp = newval.data();
      auto max = std::numeric_limits<T>::max();
      for(size_t i = 0; i < row_size; i++) {
        newvalp[i] = max;
      }
      for(size_t i = 0; i < exchanged.size(); i++) {
        auto currentp = exchanged[i].data();
        auto current_size = exchanged[i].size();
        auto merge_mapp = merge_map[i].data();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
        for(size_t j = 0; j < current_size; j++) {
          if(newvalp[merge_mapp[j]] > currentp[j])
            newvalp[merge_mapp[j]] = currentp[j];
        }
      }
      return newval;
    }, merge_map, row_sizes);
  ret->val = std::move(newval);
  if(contain_nulls) {
    ret->nulls = ret->val.map(+[](std::vector<T>& val) {
        return find_value(val, std::numeric_limits<T>::max());
      });
    ret->contain_nulls_check();
  }
  return ret;
}


template <class T>
std::vector<std::vector<size_t>>
size_helper(std::vector<size_t>& idx_split,
            std::vector<std::vector<size_t>>& hash_divide) {
  size_t splitsize = idx_split.size();
  std::vector<size_t> ret(splitsize-1);
  size_t* idx_splitp = &idx_split[0];
  size_t* retp = &ret[0];
  for(size_t i = 0; i < splitsize-1; i++) {
    retp[i] = idx_splitp[i+1] - idx_splitp[i];
  }
  auto nodesize = hash_divide.size();
  std::vector<std::vector<size_t>> hashed_ret(nodesize);
  for(size_t i = 0; i < nodesize; i++) {
    auto each_size = hash_divide[i].size();
    hashed_ret[i].resize(each_size);
    auto hash_dividep = hash_divide[i].data();
    auto hashed_retp = hashed_ret[i].data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t j = 0; j < each_size; j++) {
      hashed_retp[j] = retp[hash_dividep[j]];
    }
  }
  return hashed_ret;
}

template <class T>
std::vector<std::vector<size_t>>
count_helper(std::vector<T>& val,
             std::vector<size_t>& grouped_idx,
             std::vector<size_t>& idx_split,
             std::vector<std::vector<size_t>>& hash_divide,
             std::vector<size_t>& nulls) {
  if(nulls.size() == 0) return size_helper<T>(idx_split, hash_divide);
  else { // slow: takes same time as sum
    std::vector<size_t> tmp(val.size(), 1);
    return sum_helper(tmp, grouped_idx, idx_split, hash_divide, nulls);
  }
}

template <class T>
std::shared_ptr<dfcolumn>
count_impl(node_local<std::vector<T>>& val,
           node_local<std::vector<size_t>>& nulls,
           node_local<std::vector<size_t>>& local_grouped_idx,
           node_local<std::vector<size_t>>& local_idx_split,
           node_local<std::vector<std::vector<size_t>>>& hash_divide,
           node_local<std::vector<std::vector<size_t>>>& merge_map,
           node_local<size_t>& row_sizes) {
  return count_impl(val, nulls, local_grouped_idx, local_idx_split,
                    hash_divide, merge_map,row_sizes, false);
}

template <class T>
std::shared_ptr<dfcolumn>
count_impl(node_local<std::vector<T>>& val,
           node_local<std::vector<size_t>>& nulls,
           node_local<std::vector<size_t>>& local_grouped_idx,
           node_local<std::vector<size_t>>& local_idx_split,
           node_local<std::vector<std::vector<size_t>>>& hash_divide,
           node_local<std::vector<std::vector<size_t>>>& merge_map,
           node_local<size_t>& row_sizes,
           bool count_nulls) {
  auto ret = std::make_shared<typed_dfcolumn<size_t>>();
  node_local<std::vector<std::vector<size_t>>> local_agg;
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  if (count_nulls) local_agg = local_idx_split.map(size_helper<T>, hash_divide);
  else local_agg = val.map(count_helper<T>, local_grouped_idx, local_idx_split,
                           hash_divide, nulls);
  auto exchanged = alltoall_exchange(local_agg);
  auto newval = exchanged.map
    (+[](std::vector<std::vector<size_t>>& exchanged,
         std::vector<std::vector<size_t>>& merge_map,
         size_t row_size) {
      std::vector<size_t> newval(row_size);
      auto newvalp = newval.data();
      for(size_t i = 0; i < exchanged.size(); i++) {
        auto currentp = exchanged[i].data();
        auto current_size = exchanged[i].size();
        auto merge_mapp = merge_map[i].data();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
        for(size_t j = 0; j < current_size; j++) {
          newvalp[merge_mapp[j]] += currentp[j];
        }
      }
      return newval;
    }, merge_map, row_sizes);
  ret->val = std::move(newval);
  return ret;
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::count
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes) {
  return count_impl(val, nulls, local_grouped_idx, local_idx_split,
                    hash_divide, merge_map, row_sizes);
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::size
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes) {
  return count_impl(val, nulls, local_grouped_idx, local_idx_split,
                    hash_divide, merge_map, row_sizes, true);
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::avg
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes) {
  auto ret = std::make_shared<typed_dfcolumn<double>>();
  auto sum_column = std::dynamic_pointer_cast<typed_dfcolumn<T>>
    (sum(local_grouped_idx, local_idx_split, hash_divide, merge_map,
         row_sizes));
  auto count_column = std::dynamic_pointer_cast<typed_dfcolumn<size_t>>
    (count(local_grouped_idx, local_idx_split, hash_divide,
           merge_map, row_sizes));
  ret->val = sum_column->val.map
    (+[](std::vector<T>& val, std::vector<size_t>& count) {
      auto size = val.size();
      auto valp = val.data();
      auto countp = count.data();
      std::vector<double> ret(size);
      auto retp = ret.data();
      for(size_t i = 0; i < size; i++) {
        retp[i] =
          static_cast<double>(valp[i]) / static_cast<double>(countp[i]);
      }
      return ret;
    }, count_column->val);
  if(sum_column->contain_nulls){
    ret->nulls = sum_column->nulls;
    ret->val.mapv
      (+[](std::vector<double>& val, std::vector<size_t>& nulls) {
        auto valp = val.data();
        auto nullsp = nulls.data();
        auto size = nulls.size();
        auto max = std::numeric_limits<T>::max();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
        for(size_t i = 0; i < size; i++) {
          valp[nullsp[i]] = max;
        }
      }, ret->nulls);
    ret->contain_nulls = true;
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
    ret->contain_nulls = false;
  }
  return ret;
}

template <class T>
std::vector<double>
var_helper(std::vector<T>& sum_sq,
           std::vector<T>& sum,
           std::vector<double>& avg, 
           std::vector<size_t>& count,
           std::vector<size_t>& sum_nulls,
           std::vector<size_t>& ret_nulls,
           double ddof) {
  auto size = sum_sq.size();
  std::vector<double> ret(size);
  auto retp = ret.data();
  auto sum_sqp = sum_sq.data();
  auto sump = sum.data();
  auto avgp = avg.data();
  auto countp = count.data();

  auto denom = count - ddof;
  auto denomp = denom.data();
  size_t invalid_count = 0;
  for(size_t i = 0; i < size; i++) {
    retp[i] = (sum_sqp[i] + countp[i]*avgp[i]*avgp[i] - 2*avgp[i]*sump[i]) / denomp[i]; 
    invalid_count += (denomp[i] <= 0.0);
  }
  if (invalid_count) ret_nulls = set_union(sum_nulls, vector_find_le(denom, 0.0));
  else               ret_nulls.swap(sum_nulls); // sum_nulls is no longer required in caller
  return ret;
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::var
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 double ddof) {
  auto ret = std::make_shared<typed_dfcolumn<double>>();
  
  auto sum_column = std::dynamic_pointer_cast<typed_dfcolumn<T>>
    (sum(local_grouped_idx, local_idx_split, hash_divide, merge_map,
         row_sizes));
  
  auto count_column = std::dynamic_pointer_cast<typed_dfcolumn<size_t>>
    (count(local_grouped_idx, local_idx_split, hash_divide,
           merge_map, row_sizes));

  auto avg_column = std::dynamic_pointer_cast<typed_dfcolumn<double>>
    (avg(local_grouped_idx, local_idx_split, hash_divide,
           merge_map, row_sizes));

  auto orig_val = val;
  val.mapv(+[](std::vector<T>& val){
            auto size = val.size();
            auto valp = val.data();
            for(size_t i = 0; i < size; i++) valp[i] = ( valp[i] * valp[i] );
          });
  auto sum_sq_column = std::dynamic_pointer_cast<typed_dfcolumn<T>>
    (sum(local_grouped_idx, local_idx_split, hash_divide, merge_map,
         row_sizes));
  val = std::move(orig_val);

  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  ret->val = sum_sq_column->val.map(var_helper<T>, sum_column->val,
                                    avg_column->val, count_column->val,
                                    sum_column->nulls, ret->nulls,
                                    broadcast(ddof));
  auto ret_nulls_size = ret->nulls.viewas_dvector<size_t>().size();

  if(ret_nulls_size){
    ret->val.mapv
      (+[](std::vector<double>& val, std::vector<size_t>& nulls) {
        auto valp = val.data();
        auto nullsp = nulls.data();
        auto size = nulls.size();
        auto max = std::numeric_limits<double>::max();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
        for(size_t i = 0; i < size; i++) {
          valp[nullsp[i]] = max;
        }
      }, ret->nulls);
    ret->contain_nulls = true;
  } else {
    ret->contain_nulls = false;
  }
  return ret;
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::sem
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 double ddof) {
  auto ret = std::make_shared<typed_dfcolumn<double>>();
  auto var_column = std::dynamic_pointer_cast<typed_dfcolumn<double>>
    (var(local_grouped_idx, local_idx_split, hash_divide, merge_map,
        row_sizes, ddof));
  auto count_column = std::dynamic_pointer_cast<typed_dfcolumn<size_t>>
    (count(local_grouped_idx, local_idx_split, hash_divide,
           merge_map, row_sizes));

  ret->val = var_column->val.map(+[](std::vector<double>& var, std::vector<size_t>& count ){
                                  auto size = var.size();
                                  std::vector<double> ret(size);
                                  auto varp = var.data();
                                  auto countp = count.data();
                                  auto retp = ret.data();
                                  for(size_t i = 0; i < size; i++) retp[i] = sqrt(varp[i] / countp[i]);
                                  auto nans = vector_find_nan(var);
                                  auto nansz = nans.size();
                                  if (nansz > 0){
                                    auto nansp = nans.data();
                                    auto nan_val = std::numeric_limits<double>::quiet_NaN();
                                    #pragma cdir nodep
                                    #pragma _NEC ivdep
                                    #pragma _NEC vovertake
                                    #pragma _NEC vob
                                    for (size_t i = 0; i < nansz; i++) retp[nansp[i]] = nan_val;
                                  }
                                  return ret;
                                }, count_column->val);

  if(var_column->contain_nulls){
    ret->nulls = var_column->nulls;
    ret->val.mapv
      (+[](std::vector<double>& val, std::vector<size_t>& nulls) {
        auto valp = val.data();
        auto nullsp = nulls.data();
        auto size = nulls.size();
        auto max = std::numeric_limits<double>::max();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
        for(size_t i = 0; i < size; i++) {
          valp[nullsp[i]] = max;
        }
      }, ret->nulls);
    ret->contain_nulls = true;
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
    ret->contain_nulls = false;
  }
  return ret;
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::std
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 double ddof) {
  auto ret = std::make_shared<typed_dfcolumn<double>>();
  auto var_column = std::dynamic_pointer_cast<typed_dfcolumn<double>>
    (var(local_grouped_idx, local_idx_split, hash_divide, merge_map,
        row_sizes, ddof));

  ret->val = var_column->val.map(+[](std::vector<double>& var) {
                                  auto size = var.size();
                                  std::vector<double> ret(size);
                                  auto varp = var.data();
                                  auto retp = ret.data();
                                  for(size_t i = 0; i < size; i++) retp[i] = sqrt(varp[i]);
                                  auto nans = vector_find_nan(var);
                                  auto nansz = nans.size();
                                  if (nansz > 0){
                                    auto nansp = nans.data();
                                    auto nan_val = std::numeric_limits<double>::quiet_NaN();
                                    #pragma cdir nodep
                                    #pragma _NEC ivdep
                                    #pragma _NEC vovertake
                                    #pragma _NEC vob
                                    for (size_t i = 0; i < nansz; i++) retp[nansp[i]] = nan_val;
                                  }
                                  return ret;
                                });
  if(var_column->contain_nulls){
    ret->nulls = var_column->nulls;
    ret->val.mapv
      (+[](std::vector<double>& val, std::vector<size_t>& nulls) {
        auto valp = val.data();
        auto nullsp = nulls.data();
        auto size = nulls.size();
        auto max = std::numeric_limits<double>::max();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
        for(size_t i = 0; i < size; i++) {
          valp[nullsp[i]] = max;
        }
      }, ret->nulls);
    ret->contain_nulls = true;
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
    ret->contain_nulls = false;
  }
  return ret;
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::mad
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes) {
  throw std::runtime_error("mad of numeric column is not yet defined"); // TODO 
}

std::vector<size_t>
fill_sized_idx(const size_t* idxp, const size_t* idx_lenp, size_t size);

template <class T>
std::vector<std::vector<T>>
count_distinct_helper(std::vector<T>& org_val,
                      std::vector<size_t>& grouped_idx,
                      std::vector<size_t>& idx_split,
                      std::vector<std::vector<size_t>>& hash_divide,
                      std::vector<std::vector<size_t>>& hashed_distinct_size) {
  T* org_valp = org_val.data();
  size_t valsize = grouped_idx.size();
  std::vector<T> val(valsize);
  auto valp = val.data();
  auto grouped_idxp = grouped_idx.data();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < valsize; i++) {
    valp[i] = org_valp[grouped_idxp[i]];
  }
  size_t splitsize = idx_split.size();
  size_t groupsize = splitsize - 1;
  std::vector<size_t> tmp_group(groupsize);
  std::vector<size_t> each_split(groupsize);
  auto each_splitp = each_split.data();
  auto idx_splitp = idx_split.data();
  for(size_t i = 0; i < groupsize; i++) {
    each_splitp[i] = idx_splitp[i+1] - idx_splitp[i];
  }
  auto tmp_groupp = tmp_group.data();
  for(size_t i = 0; i < groupsize; i++) tmp_groupp[i] = i;
  auto sized_idx = fill_sized_idx(tmp_groupp, each_splitp, groupsize);
  radix_sort(val, sized_idx);
  radix_sort(sized_idx, val);
  auto val_split = set_separate(val);
  auto merged_split = set_union(idx_split, val_split);
  auto merged_split_size = merged_split.size();
  std::vector<T> distinct_val(merged_split_size-1);
  auto distinct_valp = distinct_val.data();
  auto merged_splitp = merged_split.data();
#pragma _NEC vob
#pragma _NEC vovertake
#pragma _NEC ivdep
  for(size_t i = 0; i < merged_split_size - 1; i++) {
    distinct_valp[i] = valp[merged_splitp[i]];
  }
  auto group_split_pos = lower_bound(merged_split, idx_split);
  std::vector<size_t> distinct_size(groupsize);
  auto distinct_sizep = distinct_size.data();
  auto group_split_posp = group_split_pos.data();
  for(size_t i = 0; i < groupsize; i++) {
    distinct_sizep[i] = group_split_posp[i+1] - group_split_posp[i];
  }
  auto nodesize = hash_divide.size();
  hashed_distinct_size.resize(nodesize);
  for(size_t i = 0; i < nodesize; i++) {
    auto each_size = hash_divide[i].size();
    hashed_distinct_size[i].resize(each_size);
    auto hash_dividep = hash_divide[i].data();
    auto hashed_distinct_sizep = hashed_distinct_size[i].data();
#pragma _NEC vob
#pragma _NEC vovertake
#pragma _NEC ivdep
    for(size_t j = 0; j < each_size; j++) {
      hashed_distinct_sizep[j] = distinct_sizep[hash_dividep[j]];
    }
  }
  std::vector<std::vector<size_t>> px_hashed_distinct_size(nodesize);
  for(size_t i = 0; i < nodesize; i++) {
    auto tmp_size = hashed_distinct_size[i].size();
    px_hashed_distinct_size[i].resize(tmp_size + 1);
    prefix_sum(hashed_distinct_size[i].data(),
               px_hashed_distinct_size[i].data() + 1,
               tmp_size);
  }
  std::vector<std::vector<T>> ret(nodesize);
  for(size_t i = 0; i < nodesize; i++) {
    auto size = px_hashed_distinct_size[i].size();
    ret[i].resize(px_hashed_distinct_size[i][size-1]);
  }
  for(size_t i = 0; i < nodesize; i++) {
    auto size = hash_divide[i].size();
    if(size == 0) continue;
    auto hash_dividep = hash_divide[i].data();
    auto ret_size = ret[i].size();
    auto retp = ret[i].data();
    auto px_hashed_distinct_sizep = px_hashed_distinct_size[i].data();
    auto hashed_distinct_sizep = hashed_distinct_size[i].data();
    if(ret_size / size > GROUPBY_COUNT_DISTINCT_VLEN * 2) {
      for(size_t j = 0; j < size; j++) {
        auto crnt_retp = retp + px_hashed_distinct_sizep[j];
        auto crnt_valp = distinct_valp + group_split_posp[hash_dividep[j]];
        auto crnt_distinct_size = hashed_distinct_sizep[j];
        for(size_t k = 0; k < crnt_distinct_size; k++) {
          crnt_retp[k] = crnt_valp[k];
        }
      }
    } else {
      auto num_block = size / GROUPBY_COUNT_DISTINCT_VLEN;
      auto rest = size - num_block * GROUPBY_COUNT_DISTINCT_VLEN;
      for(size_t b = 0; b < num_block; b++) {
        size_t offset = b * GROUPBY_COUNT_DISTINCT_VLEN;
        auto offset_px_hashed_distinct_sizep =
          px_hashed_distinct_sizep + offset;
        auto offset_hash_dividep = hash_dividep + offset;
        auto offset_hashed_distinct_sizep = hashed_distinct_sizep + offset;
        size_t max = 0;
        for(size_t j = 0; j < GROUPBY_COUNT_DISTINCT_VLEN; j++) {
          if(max < offset_hashed_distinct_sizep[j])
            max = offset_hashed_distinct_sizep[j];
        }
        for(size_t j = 0; j < max; j++) {
#pragma _NEC vob
#pragma _NEC vovertake
#pragma _NEC ivdep
          for(size_t k = 0; k < GROUPBY_COUNT_DISTINCT_VLEN; k++) {
            if(j < offset_hashed_distinct_sizep[k]) {
              retp[offset_px_hashed_distinct_sizep[k] + j] =
                distinct_valp[group_split_posp[offset_hash_dividep[k]] + j];
            }
          }
        }
      }
      size_t offset = num_block * GROUPBY_COUNT_DISTINCT_VLEN;
      auto offset_px_hashed_distinct_sizep =
        px_hashed_distinct_sizep + offset;
      auto offset_hash_dividep = hash_dividep + offset;
      auto offset_hashed_distinct_sizep = hashed_distinct_sizep + offset;
      size_t max = 0;
      for(size_t j = 0; j < rest; j++) {
        if(max < offset_hashed_distinct_sizep[j])
          max = offset_hashed_distinct_sizep[j];
      }
      for(size_t j = 0; j < max; j++) {
#pragma _NEC vob
#pragma _NEC vovertake
#pragma _NEC ivdep
        for(size_t k = 0; k < rest; k++) {
          if(j < offset_hashed_distinct_sizep[k]) {
            retp[offset_px_hashed_distinct_sizep[k] + j] =
              distinct_valp[group_split_posp[offset_hash_dividep[k]] + j];
          }
        }
      }
    }
  }
  return ret;
}

template <class T>
std::shared_ptr<dfcolumn>
count_distinct_impl
(node_local<std::vector<T>>& val,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes) {
#ifndef DONOT_ALLOW_MAX_AS_VALUE
  throw std::runtime_error
    ("define DONOT_ALLOW_MAX_AS_VALUE to use count_distinct");
#endif
  auto ret = std::make_shared<typed_dfcolumn<size_t>>();
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  auto local_distinct_size =
    make_node_local_allocate<std::vector<std::vector<size_t>>>();
  auto local_agg = val.map(count_distinct_helper<T>, local_grouped_idx,
                           local_idx_split, hash_divide, local_distinct_size);
  auto exchanged = alltoall_exchange(local_agg);
  auto exchanged_distinct_size = alltoall_exchange(local_distinct_size);
  auto newval = exchanged.map
    (+[](std::vector<std::vector<T>>& exchanged,
         std::vector<std::vector<size_t>>& merge_map,
         std::vector<std::vector<size_t>>& exchanged_distinct_size,
         size_t row_size) {
      std::vector<size_t> newval(row_size);
      auto newvalp = newval.data();
      auto flatten_val = flatten(exchanged);
      auto nodesize = merge_map.size();
      std::vector<std::vector<size_t>> sized_idx(nodesize);
      for(size_t i = 0; i < nodesize; i++) {
        sized_idx[i] = fill_sized_idx(merge_map[i].data(),
                                      exchanged_distinct_size[i].data(),
                                      merge_map[i].size());
      }
      auto flatten_sized_idx = flatten(sized_idx);
      radix_sort(flatten_val, flatten_sized_idx);
      radix_sort(flatten_sized_idx, flatten_val);
      auto val_separate = set_separate(flatten_val);
      auto idx_separate = set_separate(flatten_sized_idx);
      auto merged_separate = set_union(val_separate, idx_separate);
      auto group_pos = lower_bound(merged_separate, idx_separate);
      auto group_posp = group_pos.data();
      auto group_pos_size = group_pos.size();
      auto flatten_valp = flatten_val.data();
      auto merged_separatep = merged_separate.data();
      auto max = std::numeric_limits<T>::max();
#pragma _NEC vob
#pragma _NEC vovertake
#pragma _NEC ivdep
      for(size_t i = 0; i < group_pos_size-1; i++) {
        auto lastdata = flatten_valp[merged_separatep[group_posp[i+1]]-1];
        auto tostore = group_posp[i+1] - group_posp[i];
        if(lastdata == max) tostore--;
        newvalp[i] = tostore;
      }
      return newval;
    }, merge_map, exchanged_distinct_size, row_sizes);
  ret->val = std::move(newval);
  return ret;
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::count_distinct
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes) {
  return count_distinct_impl(val, local_grouped_idx, local_idx_split,
                             hash_divide, merge_map, row_sizes);
}

template <class T>
std::shared_ptr<dfcolumn>
sum_distinct_impl
(node_local<std::vector<T>>& val,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes) {
#ifndef DONOT_ALLOW_MAX_AS_VALUE
  throw std::runtime_error
    ("define DONOT_ALLOW_MAX_AS_VALUE to use sum_distinct");
#endif
  auto ret = std::make_shared<typed_dfcolumn<T>>();
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  auto local_distinct_size =
    make_node_local_allocate<std::vector<std::vector<size_t>>>();
  auto local_agg = val.map(count_distinct_helper<T>, local_grouped_idx,
                           local_idx_split, hash_divide, local_distinct_size);
  auto exchanged = alltoall_exchange(local_agg);
  auto exchanged_distinct_size = alltoall_exchange(local_distinct_size);
  auto newval = exchanged.map
    (+[](std::vector<std::vector<T>>& exchanged,
         std::vector<std::vector<size_t>>& merge_map,
         std::vector<std::vector<size_t>>& exchanged_distinct_size,
         std::vector<size_t>& nulls,
         size_t row_size) {
      std::vector<T> newval(row_size);
      auto newvalp = newval.data();
      auto flatten_val = flatten(exchanged);
      auto nodesize = merge_map.size();
      std::vector<std::vector<size_t>> sized_idx(nodesize);
      for(size_t i = 0; i < nodesize; i++) {
        sized_idx[i] = fill_sized_idx(merge_map[i].data(),
                                      exchanged_distinct_size[i].data(),
                                      merge_map[i].size());
      }
      auto flatten_sized_idx = flatten(sized_idx);
      radix_sort(flatten_val, flatten_sized_idx);
      radix_sort(flatten_sized_idx, flatten_val);
      auto val_separate = set_separate(flatten_val);
      auto idx_separate = set_separate(flatten_sized_idx);
      auto merged_separate = set_union(val_separate, idx_separate);
      auto group_pos = lower_bound(merged_separate, idx_separate);
      auto group_posp = group_pos.data();
      auto group_pos_size = group_pos.size();
      auto flatten_valp = flatten_val.data();
      auto merged_separatep = merged_separate.data();
      auto distinct_val_size = merged_separate.size() - 1;
      std::vector<T> distinct_val(distinct_val_size);
      auto distinct_valp = distinct_val.data();
#pragma _NEC vob
#pragma _NEC vovertake
#pragma _NEC ivdep
      for(size_t i = 0; i < distinct_val_size; i++) {
        distinct_valp[i] = flatten_valp[merged_separatep[i]];
      }
      auto maxpos = vector_find_tmax(distinct_val);
      auto maxpos_size = maxpos.size();
      auto maxposp = maxpos.data();
#pragma _NEC vob
#pragma _NEC vovertake
#pragma _NEC ivdep
      for(size_t i = 0; i < maxpos_size; i++) {
        distinct_valp[maxposp[i]] = 0;
      }
      // always group_pos_size > 0
      if(distinct_val_size / group_pos_size > GROUPBY_VLEN * 2) {
        size_t end = 0;
        size_t start = 0;
        for(size_t i = 0; i < group_pos_size-1; i++) {
          start = end;
          end = group_posp[i+1];
          T total = 0;
          for(size_t j = start; j < end; j++) {
            total += distinct_valp[j];
          }
          newvalp[i] = total;
        }
      } else {
        group_by_vector_sum_helper(distinct_val, group_pos, newval);
      }
      std::vector<size_t> count(group_pos_size - 1);
      auto countp = count.data();
      for(size_t i = 0; i < group_pos_size - 1; i++) {
        countp[i] = group_posp[i+1] - group_posp[i];
      }
      auto one = vector_find_one(count); // maybe only NULL
      auto one_size = one.size();
      std::vector<size_t> one_group_pos(one_size);
      auto onep = one.data();
      auto one_group_posp = one_group_pos.data();
      for(size_t i = 0; i < one_size; i++) {
        one_group_posp[i] = group_posp[onep[i]];
      }
      auto one_and_max = set_intersection(one_group_pos, maxpos);
      nulls = lower_bound(group_pos, one_and_max);
      return newval;
    }, merge_map, exchanged_distinct_size, ret->nulls, row_sizes);
  ret->val = std::move(newval);
  ret->contain_nulls_check();
  return ret;
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::sum_distinct
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes) {
  return sum_distinct_impl(val, local_grouped_idx, local_idx_split,
                           hash_divide, merge_map, row_sizes);
}

template <class T>
std::vector<std::vector<T>>
first_helper(std::vector<T>& org_val,
             std::vector<size_t>& grouped_idx,
             std::vector<size_t>& idx_split,
             std::vector<std::vector<size_t>>& hash_divide,
             bool ignore_nulls) {
  T* org_valp = org_val.data();
  size_t valsize = grouped_idx.size();
  std::vector<T> val(valsize);
  auto valp = val.data();
  auto grouped_idxp = grouped_idx.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < valsize; i++) {
    valp[i] = org_valp[grouped_idxp[i]];
  }
  size_t splitsize = idx_split.size();
  std::vector<T> ret(splitsize-1);
  auto retp = ret.data();
  size_t* idx_splitp = idx_split.data();
  auto max = std::numeric_limits<T>::max();
  if(!ignore_nulls) {
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < splitsize-1; i++) {
      retp[i] = valp[idx_splitp[i]];
    }
  } else {
    // less vectorized, but small computation anyway
    for(size_t i = 0; i < splitsize-1; i++) {
      auto start = idx_splitp[i];
      auto end = idx_splitp[i+1];
      retp[i] = max;
      for(size_t j = start; j < end; j++) {
        auto crnt = valp[j];
        if(crnt == max) continue;
        else {
          retp[i] = crnt;
          break;
        }
      }
    }
  }
  auto nodesize = hash_divide.size();
  std::vector<std::vector<T>> hashed_ret(nodesize);
  for(size_t i = 0; i < nodesize; i++) {
    auto each_size = hash_divide[i].size();
    hashed_ret[i].resize(each_size);
    auto hash_dividep = hash_divide[i].data();
    auto hashed_retp = hashed_ret[i].data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t j = 0; j < each_size; j++) {
      hashed_retp[j] = retp[hash_dividep[j]];
    }
  }
  return hashed_ret;
}

template <class T>
std::shared_ptr<dfcolumn>
first_impl
(node_local<std::vector<T>>& val,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 bool ignore_nulls) {
#ifndef DONOT_ALLOW_MAX_AS_VALUE
  throw std::runtime_error
    ("define DONOT_ALLOW_MAX_AS_VALUE to use first");
#endif
  auto bignore_nulls = broadcast(ignore_nulls);
  auto ret = std::make_shared<typed_dfcolumn<T>>();
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  auto local_agg = val.map(first_helper<T>, local_grouped_idx,
                           local_idx_split, hash_divide,
                           bignore_nulls);
  auto exchanged = alltoall_exchange(local_agg);
  auto newval = exchanged.map
    (+[](std::vector<std::vector<T>>& exchanged,
         std::vector<std::vector<size_t>>& merge_map,
         std::vector<size_t>& nulls,
         bool ignore_nulls,
         size_t row_size) {
      std::vector<T> newval(row_size);
      auto newvalp = newval.data();
      auto max = std::numeric_limits<T>::max();
      if(ignore_nulls) {
        for(size_t i = 0; i < row_size; i++) newvalp[i] = max;
      }
      auto exchanged_size = exchanged.size();
      for(size_t i = 0; i < exchanged_size; i++) {
        auto currentp = exchanged[exchanged_size - 1 - i].data();
        auto current_size = exchanged[exchanged_size - 1 - i].size();
        auto merge_mapp = merge_map[exchanged_size - 1 - i].data();
        if(!ignore_nulls) {
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
          for(size_t j = 0; j < current_size; j++) {
            newvalp[merge_mapp[j]] = currentp[j];
          }
        } else {
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
          for(size_t j = 0; j < current_size; j++) {
            if(currentp[j] != max)
              newvalp[merge_mapp[j]] = currentp[j];
          }
        }
      }
      nulls = vector_find_tmax(newval); 
      return newval;
    }, merge_map, ret->nulls, bignore_nulls, row_sizes);
  ret->val = std::move(newval);
  ret->contain_nulls_check();
  return ret;
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::first
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 bool ignore_nulls) {
  return first_impl(val, local_grouped_idx, local_idx_split,
                    hash_divide, merge_map, row_sizes, ignore_nulls);
}

template <class T>
std::vector<std::vector<T>>
last_helper(std::vector<T>& org_val,
            std::vector<size_t>& grouped_idx,
            std::vector<size_t>& idx_split,
            std::vector<std::vector<size_t>>& hash_divide,
            bool ignore_nulls) {
  T* org_valp = org_val.data();
  size_t valsize = grouped_idx.size();
  std::vector<T> val(valsize);
  auto valp = val.data();
  auto grouped_idxp = grouped_idx.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < valsize; i++) {
    valp[i] = org_valp[grouped_idxp[i]];
  }
  size_t splitsize = idx_split.size();
  std::vector<T> ret(splitsize-1);
  auto retp = ret.data();
  size_t* idx_splitp = idx_split.data();
  auto max = std::numeric_limits<T>::max();
  if(!ignore_nulls) {
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < splitsize-1; i++) {
      retp[i] = valp[idx_splitp[i+1]-1];
    }
  } else {
    // less vectorized, but small computation anyway
    for(size_t i = 0; i < splitsize-1; i++) {
      auto start = idx_splitp[i];
      auto end = idx_splitp[i+1];
      auto len = end - start;
      retp[i] = max;
      for(size_t j = 0; j < len; j++) {
        auto crnt = valp[end - 1 - j];
        if(crnt == max) continue;
        else {
          retp[i] = crnt;
          break;
        }
      }
    }
  }
  auto nodesize = hash_divide.size();
  std::vector<std::vector<T>> hashed_ret(nodesize);
  for(size_t i = 0; i < nodesize; i++) {
    auto each_size = hash_divide[i].size();
    hashed_ret[i].resize(each_size);
    auto hash_dividep = hash_divide[i].data();
    auto hashed_retp = hashed_ret[i].data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t j = 0; j < each_size; j++) {
      hashed_retp[j] = retp[hash_dividep[j]];
    }
  }
  return hashed_ret;
}

template <class T>
std::shared_ptr<dfcolumn>
last_impl
(node_local<std::vector<T>>& val,
 node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 bool ignore_nulls) {
#ifndef DONOT_ALLOW_MAX_AS_VALUE
  throw std::runtime_error
    ("define DONOT_ALLOW_MAX_AS_VALUE to use last");
#endif
  auto bignore_nulls = broadcast(ignore_nulls);
  auto ret = std::make_shared<typed_dfcolumn<T>>();
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  auto local_agg = val.map(last_helper<T>, local_grouped_idx,
                           local_idx_split, hash_divide,
                           bignore_nulls);
  auto exchanged = alltoall_exchange(local_agg);
  auto newval = exchanged.map
    (+[](std::vector<std::vector<T>>& exchanged,
         std::vector<std::vector<size_t>>& merge_map,
         std::vector<size_t>& nulls,
         bool ignore_nulls,
         size_t row_size) {
      std::vector<T> newval(row_size);
      auto newvalp = newval.data();
      auto max = std::numeric_limits<T>::max();
      if(ignore_nulls) {
        for(size_t i = 0; i < row_size; i++) newvalp[i] = max;
      }
      auto exchanged_size = exchanged.size();
      for(size_t i = 0; i < exchanged_size; i++) {
        auto currentp = exchanged[i].data();
        auto current_size = exchanged[i].size();
        auto merge_mapp = merge_map[i].data();
        if(!ignore_nulls) {
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
          for(size_t j = 0; j < current_size; j++) {
            newvalp[merge_mapp[j]] = currentp[j];
          }
        } else {
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
          for(size_t j = 0; j < current_size; j++) {
            if(currentp[j] != max)
              newvalp[merge_mapp[j]] = currentp[j];
          }
        }
      }
      nulls = vector_find_tmax(newval); 
      return newval;
    }, merge_map, ret->nulls, bignore_nulls, row_sizes);
  ret->val = std::move(newval);
  ret->contain_nulls_check();
  return ret;
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::last
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 bool ignore_nulls) {
  return last_impl(val, local_grouped_idx, local_idx_split,
                   hash_divide, merge_map, row_sizes, ignore_nulls);
}


template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::sum
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::sum
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::sum
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::sum
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::sum
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::sum
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::max
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::max
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::max
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::max
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::max
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::max
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::min
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::min
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::min
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::min
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::min
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::min
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::count
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::count
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::count
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::count
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::count
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::count
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::size
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::size
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::size
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::size
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::size
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::size
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::avg
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::avg
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::avg
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::avg
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::avg
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::avg
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::var
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 double ddof);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::var
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 double ddof);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::var
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 double ddof);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::var
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 double ddof);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::var
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 double ddof);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::var
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 double ddof);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::sem
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 double ddof);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::sem
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 double ddof);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::sem
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 double ddof);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::sem
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 double ddof);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::sem
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 double ddof);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::sem
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 double ddof);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::std
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 double ddof);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::std
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 double ddof);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::std
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 double ddof);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::std
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 double ddof);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::std
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 double ddof);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::std
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 double ddof);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::mad
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::mad
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::mad
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::mad
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::mad
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::mad
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::count_distinct
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::count_distinct
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::count_distinct
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::count_distinct
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::count_distinct
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::count_distinct
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::sum_distinct
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::sum_distinct
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::sum_distinct
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::sum_distinct
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::sum_distinct
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::sum_distinct
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::first
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 bool ignore_nulls);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::first
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 bool ignore_nulls);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::first
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 bool ignore_nulls);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::first
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 bool ignore_nulls);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::first
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 bool ignore_nulls);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::first
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 bool ignore_nulls);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::last
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 bool ignore_nulls);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::last
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 bool ignore_nulls);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::last
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 bool ignore_nulls);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::last
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 bool ignore_nulls);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::last
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 bool ignore_nulls);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::last
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 bool ignore_nulls);

// ----- dic_string -----

std::shared_ptr<dfcolumn>
typed_dfcolumn<dic_string>::count
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes) {
  return count_impl(val, nulls, local_grouped_idx, local_idx_split,
                    hash_divide, merge_map, row_sizes);
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<dic_string>::size
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes) {
  return count_impl(val, nulls, local_grouped_idx, local_idx_split,
                    hash_divide, merge_map, row_sizes, true);
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<dic_string>::count_distinct
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes) {
  return count_distinct_impl(val, local_grouped_idx, local_idx_split,
                             hash_divide, merge_map, row_sizes);
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<dic_string>::first
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 bool ignore_nulls) {
#ifndef DONOT_ALLOW_MAX_AS_VALUE
  throw std::runtime_error
    ("define DONOT_ALLOW_MAX_AS_VALUE to use first");
#endif
  auto bignore_nulls = broadcast(ignore_nulls);
  auto ret = std::make_shared<typed_dfcolumn<dic_string>>();
  ret->dic = dic;
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  auto local_agg = val.map(first_helper<size_t>, local_grouped_idx,
                           local_idx_split, hash_divide,
                           bignore_nulls);
  auto exchanged = alltoall_exchange(local_agg);
  auto newval = exchanged.map
    (+[](std::vector<std::vector<size_t>>& exchanged,
         std::vector<std::vector<size_t>>& merge_map,
         std::vector<size_t>& nulls,
         bool ignore_nulls,
         size_t row_size) {
      std::vector<size_t> newval(row_size);
      auto newvalp = newval.data();
      auto max = std::numeric_limits<size_t>::max();
      if(ignore_nulls) {
        for(size_t i = 0; i < row_size; i++) newvalp[i] = max;
      }
      auto exchanged_size = exchanged.size();
      for(size_t i = 0; i < exchanged_size; i++) {
        auto currentp = exchanged[exchanged_size - 1 - i].data();
        auto current_size = exchanged[exchanged_size - 1 - i].size();
        auto merge_mapp = merge_map[exchanged_size - 1 - i].data();
        if(!ignore_nulls) {
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
          for(size_t j = 0; j < current_size; j++) {
            newvalp[merge_mapp[j]] = currentp[j];
          }
        } else {
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
          for(size_t j = 0; j < current_size; j++) {
            if(currentp[j] != max)
              newvalp[merge_mapp[j]] = currentp[j];
          }
        }
      }
      nulls = vector_find_tmax(newval); 
      return newval;
    }, merge_map, ret->nulls, bignore_nulls, row_sizes);
  ret->val = std::move(newval);
  ret->contain_nulls_check();
  return ret;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<dic_string>::last
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 bool ignore_nulls) {
#ifndef DONOT_ALLOW_MAX_AS_VALUE
  throw std::runtime_error
    ("define DONOT_ALLOW_MAX_AS_VALUE to use last");
#endif
  auto bignore_nulls = broadcast(ignore_nulls);
  auto ret = std::make_shared<typed_dfcolumn<dic_string>>();
  ret->dic = dic;
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  auto local_agg = val.map(last_helper<size_t>, local_grouped_idx,
                           local_idx_split, hash_divide,
                           bignore_nulls);
  auto exchanged = alltoall_exchange(local_agg);
  auto newval = exchanged.map
    (+[](std::vector<std::vector<size_t>>& exchanged,
         std::vector<std::vector<size_t>>& merge_map,
         std::vector<size_t>& nulls,
         bool ignore_nulls,
         size_t row_size) {
      std::vector<size_t> newval(row_size);
      auto newvalp = newval.data();
      auto max = std::numeric_limits<size_t>::max();
      if(ignore_nulls) {
        for(size_t i = 0; i < row_size; i++) newvalp[i] = max;
      }
      auto exchanged_size = exchanged.size();
      for(size_t i = 0; i < exchanged_size; i++) {
        auto currentp = exchanged[i].data();
        auto current_size = exchanged[i].size();
        auto merge_mapp = merge_map[i].data();
        if(!ignore_nulls) {
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
          for(size_t j = 0; j < current_size; j++) {
            newvalp[merge_mapp[j]] = currentp[j];
          }
        } else {
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
          for(size_t j = 0; j < current_size; j++) {
            if(currentp[j] != max)
              newvalp[merge_mapp[j]] = currentp[j];
          }
        }
      }
      nulls = vector_find_tmax(newval); 
      return newval;
    }, merge_map, ret->nulls, bignore_nulls, row_sizes);
  ret->val = std::move(newval);
  ret->contain_nulls_check();
  return ret;
}

// ----- string -----

std::shared_ptr<dfcolumn>
typed_dfcolumn<string>::count
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes) {
  return count_impl(val, nulls, local_grouped_idx, local_idx_split,
                    hash_divide, merge_map, row_sizes);
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<string>::size
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes) {
  return count_impl(val, nulls, local_grouped_idx, local_idx_split,
                    hash_divide, merge_map, row_sizes, true);
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<string>::count_distinct
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes) {
  return count_distinct_impl(val, local_grouped_idx, local_idx_split,
                             hash_divide, merge_map, row_sizes);
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<string>::first
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 bool ignore_nulls) {
#ifndef DONOT_ALLOW_MAX_AS_VALUE
  throw std::runtime_error
    ("define DONOT_ALLOW_MAX_AS_VALUE to use first");
#endif
  auto bignore_nulls = broadcast(ignore_nulls);
  auto ret = std::make_shared<typed_dfcolumn<string>>();
  ret->dic = dic;
  ret->dic_idx = dic_idx;
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  auto local_agg = val.map(first_helper<size_t>, local_grouped_idx,
                           local_idx_split, hash_divide,
                           bignore_nulls);
  auto exchanged = alltoall_exchange(local_agg);
  auto newval = exchanged.map
    (+[](std::vector<std::vector<size_t>>& exchanged,
         std::vector<std::vector<size_t>>& merge_map,
         std::vector<size_t>& nulls,
         bool ignore_nulls,
         size_t row_size) {
      std::vector<size_t> newval(row_size);
      auto newvalp = newval.data();
      auto max = std::numeric_limits<size_t>::max();
      if(ignore_nulls) {
        for(size_t i = 0; i < row_size; i++) newvalp[i] = max;
      }
      auto exchanged_size = exchanged.size();
      for(size_t i = 0; i < exchanged_size; i++) {
        auto currentp = exchanged[exchanged_size - 1 - i].data();
        auto current_size = exchanged[exchanged_size - 1 - i].size();
        auto merge_mapp = merge_map[exchanged_size - 1 - i].data();
        if(!ignore_nulls) {
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
          for(size_t j = 0; j < current_size; j++) {
            newvalp[merge_mapp[j]] = currentp[j];
          }
        } else {
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
          for(size_t j = 0; j < current_size; j++) {
            if(currentp[j] != max)
              newvalp[merge_mapp[j]] = currentp[j];
          }
        }
      }
      nulls = vector_find_tmax(newval); 
      return newval;
    }, merge_map, ret->nulls, bignore_nulls, row_sizes);
  ret->val = std::move(newval);
  ret->contain_nulls_check();
  return ret;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<string>::last
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 bool ignore_nulls) {
#ifndef DONOT_ALLOW_MAX_AS_VALUE
  throw std::runtime_error
    ("define DONOT_ALLOW_MAX_AS_VALUE to use last");
#endif
  auto bignore_nulls = broadcast(ignore_nulls);
  auto ret = std::make_shared<typed_dfcolumn<string>>();
  ret->dic = dic;
  ret->dic_idx = dic_idx;
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  auto local_agg = val.map(last_helper<size_t>, local_grouped_idx,
                           local_idx_split, hash_divide,
                           bignore_nulls);
  auto exchanged = alltoall_exchange(local_agg);
  auto newval = exchanged.map
    (+[](std::vector<std::vector<size_t>>& exchanged,
         std::vector<std::vector<size_t>>& merge_map,
         std::vector<size_t>& nulls,
         bool ignore_nulls,
         size_t row_size) {
      std::vector<size_t> newval(row_size);
      auto newvalp = newval.data();
      auto max = std::numeric_limits<size_t>::max();
      if(ignore_nulls) {
        for(size_t i = 0; i < row_size; i++) newvalp[i] = max;
      }
      auto exchanged_size = exchanged.size();
      for(size_t i = 0; i < exchanged_size; i++) {
        auto currentp = exchanged[i].data();
        auto current_size = exchanged[i].size();
        auto merge_mapp = merge_map[i].data();
        if(!ignore_nulls) {
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
          for(size_t j = 0; j < current_size; j++) {
            newvalp[merge_mapp[j]] = currentp[j];
          }
        } else {
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
          for(size_t j = 0; j < current_size; j++) {
            if(currentp[j] != max)
              newvalp[merge_mapp[j]] = currentp[j];
          }
        }
      }
      nulls = vector_find_tmax(newval); 
      return newval;
    }, merge_map, ret->nulls, bignore_nulls, row_sizes);
  ret->val = std::move(newval);
  ret->contain_nulls_check();
  return ret;
}

// ----- datetime -----

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::max
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes) {
  auto ret = std::make_shared<typed_dfcolumn<datetime>>();
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  auto local_agg = val.map(max_helper<datetime_t>, local_grouped_idx,
                           local_idx_split, hash_divide, nulls);
  auto exchanged = alltoall_exchange(local_agg);
  auto newval = exchanged.map
    (+[](std::vector<std::vector<datetime_t>>& exchanged,
         std::vector<std::vector<size_t>>& merge_map,
         size_t row_size) {
      std::vector<datetime_t> newval(row_size);
      auto newvalp = newval.data();
      auto min = std::numeric_limits<datetime_t>::lowest();
      for(size_t i = 0; i < row_size; i++) {
        newvalp[i] = min;
      }
      for(size_t i = 0; i < exchanged.size(); i++) {
        auto currentp = exchanged[i].data();
        auto current_size = exchanged[i].size();
        auto merge_mapp = merge_map[i].data();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
        for(size_t j = 0; j < current_size; j++) {
          if(newvalp[merge_mapp[j]] < currentp[j])
            newvalp[merge_mapp[j]] = currentp[j];
        }
      }
      return newval;
    }, merge_map, row_sizes);
  ret->val = std::move(newval);
  if(contain_nulls) {
    ret->nulls = ret->val.map(+[](std::vector<datetime_t>& val) {
        auto nulls = find_value(val, std::numeric_limits<datetime_t>::lowest());
        auto valp = val.data();
        auto nullsp = nulls.data();
        auto size = nulls.size();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
        for(size_t i = 0; i < size; i++)
          valp[nullsp[i]] = std::numeric_limits<datetime_t>::max();
        return nulls;
      });
    ret->contain_nulls_check();
  }
  return ret;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::min
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes) {
  auto ret = std::make_shared<typed_dfcolumn<datetime>>();
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  auto local_agg = val.map(min_helper<datetime_t>, local_grouped_idx,
                           local_idx_split, hash_divide, nulls);
  auto exchanged = alltoall_exchange(local_agg);
  auto newval = exchanged.map
    (+[](std::vector<std::vector<datetime_t>>& exchanged,
         std::vector<std::vector<size_t>>& merge_map,
         size_t row_size) {
      std::vector<datetime_t> newval(row_size);
      auto newvalp = newval.data();
      auto max = std::numeric_limits<datetime_t>::max();
      for(size_t i = 0; i < row_size; i++) {
        newvalp[i] = max;
      }
      for(size_t i = 0; i < exchanged.size(); i++) {
        auto currentp = exchanged[i].data();
        auto current_size = exchanged[i].size();
        auto merge_mapp = merge_map[i].data();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
        for(size_t j = 0; j < current_size; j++) {
          if(newvalp[merge_mapp[j]] > currentp[j])
            newvalp[merge_mapp[j]] = currentp[j];
        }
      }
      return newval;
    }, merge_map, row_sizes);
  ret->val = std::move(newval);
  if(contain_nulls) {
    ret->nulls = ret->val.map(+[](std::vector<datetime_t>& val) {
        return find_value(val, std::numeric_limits<datetime_t>::max());
      });
    ret->contain_nulls_check();
  }
  return ret;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::first
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 bool ignore_nulls) {
#ifndef DONOT_ALLOW_MAX_AS_VALUE
  throw std::runtime_error
    ("define DONOT_ALLOW_MAX_AS_VALUE to use first");
#endif
  auto bignore_nulls = broadcast(ignore_nulls);
  auto ret = std::make_shared<typed_dfcolumn<datetime>>();
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  auto local_agg = val.map(first_helper<datetime_t>, local_grouped_idx,
                           local_idx_split, hash_divide,
                           bignore_nulls);
  auto exchanged = alltoall_exchange(local_agg);
  auto newval = exchanged.map
    (+[](std::vector<std::vector<datetime_t>>& exchanged,
         std::vector<std::vector<size_t>>& merge_map,
         std::vector<size_t>& nulls,
         bool ignore_nulls,
         size_t row_size) {
      std::vector<datetime_t> newval(row_size);
      auto newvalp = newval.data();
      auto max = std::numeric_limits<datetime_t>::max();
      if(ignore_nulls) {
        for(size_t i = 0; i < row_size; i++) newvalp[i] = max;
      }
      auto exchanged_size = exchanged.size();
      for(size_t i = 0; i < exchanged_size; i++) {
        auto currentp = exchanged[exchanged_size - 1 - i].data();
        auto current_size = exchanged[exchanged_size - 1 - i].size();
        auto merge_mapp = merge_map[exchanged_size - 1 - i].data();
        if(!ignore_nulls) {
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
          for(size_t j = 0; j < current_size; j++) {
            newvalp[merge_mapp[j]] = currentp[j];
          }
        } else {
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
          for(size_t j = 0; j < current_size; j++) {
            if(currentp[j] != max)
              newvalp[merge_mapp[j]] = currentp[j];
          }
        }
      }
      nulls = vector_find_tmax(newval); 
      return newval;
    }, merge_map, ret->nulls, bignore_nulls, row_sizes);
  ret->val = std::move(newval);
  ret->contain_nulls_check();
  return ret;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::last
(node_local<std::vector<size_t>>& local_grouped_idx,
 node_local<std::vector<size_t>>& local_idx_split,
 node_local<std::vector<std::vector<size_t>>>& hash_divide,
 node_local<std::vector<std::vector<size_t>>>& merge_map,
 node_local<size_t>& row_sizes,
 bool ignore_nulls) {
#ifndef DONOT_ALLOW_MAX_AS_VALUE
  throw std::runtime_error
    ("define DONOT_ALLOW_MAX_AS_VALUE to use last");
#endif
  auto bignore_nulls = broadcast(ignore_nulls);
  auto ret = std::make_shared<typed_dfcolumn<datetime>>();
  ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  auto local_agg = val.map(last_helper<datetime_t>, local_grouped_idx,
                           local_idx_split, hash_divide,
                           bignore_nulls);
  auto exchanged = alltoall_exchange(local_agg);
  auto newval = exchanged.map
    (+[](std::vector<std::vector<datetime_t>>& exchanged,
         std::vector<std::vector<size_t>>& merge_map,
         std::vector<size_t>& nulls,
         bool ignore_nulls,
         size_t row_size) {
      std::vector<datetime_t> newval(row_size);
      auto newvalp = newval.data();
      auto max = std::numeric_limits<datetime_t>::max();
      if(ignore_nulls) {
        for(size_t i = 0; i < row_size; i++) newvalp[i] = max;
      }
      auto exchanged_size = exchanged.size();
      for(size_t i = 0; i < exchanged_size; i++) {
        auto currentp = exchanged[i].data();
        auto current_size = exchanged[i].size();
        auto merge_mapp = merge_map[i].data();
        if(!ignore_nulls) {
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
          for(size_t j = 0; j < current_size; j++) {
            newvalp[merge_mapp[j]] = currentp[j];
          }
        } else {
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
          for(size_t j = 0; j < current_size; j++) {
            if(currentp[j] != max)
              newvalp[merge_mapp[j]] = currentp[j];
          }
        }
      }
      nulls = vector_find_tmax(newval); 
      return newval;
    }, merge_map, ret->nulls, bignore_nulls, row_sizes);
  ret->val = std::move(newval);
  ret->contain_nulls_check();
  return ret;
}


}
