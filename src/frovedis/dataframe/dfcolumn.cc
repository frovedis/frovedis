#include "dfcolumn_impl.hpp"
#include "../core/utility.hpp"
#include "../core/find_condition.hpp"
#include <utility>
#include <regex>
#if !(defined(_SX) || defined(__ve__))
#include <unordered_set>
#endif
#include <stdlib.h>
#include <cstdint>

#define SEPARATE_TO_BUCKET_VLEN 256

#define UNIQ_IDX_MAX_BUCKET_SIZE 65536
#define UNIQ_IDX_TARGET_BUCKET_LEN 16384

#define CREATE_MERGE_MAP_VLEN 256

#ifdef __ve__
#define FILL_SIZED_IDX_VLEN 256
#else
#define FILL_SIZED_IDX_VLEN 4
#endif

namespace frovedis {

using namespace std;

std::vector<std::vector<size_t>> separate_to_bucket(std::vector<int>& key,
                                                    std::vector<size_t>& idx,
                                                    size_t num_bucket);

// separated to take size for using for local split to improve cache usage
void split_idx_with_size(std::vector<size_t>& global_idx,
                         std::vector<std::vector<size_t>>& split_idx,
                         size_t split_size) {
  if(split_size == 0) 
    throw std::runtime_error("split_idx_with_size: split_size is zero");
  else if(split_size == 1) { // skip hash
    size_t* global_idxp = &global_idx[0];
    split_idx.resize(1);
    size_t sepsize = global_idx.size();
    split_idx[0].resize(sepsize);
    size_t* split_idxp = &split_idx[0][0];
    for(size_t j = 0; j < sepsize; j++) {
      split_idxp[j] = global_idxp[j];
    }
    
  } else {
    size_t size = global_idx.size();
    std::vector<int> hash(size);
    int* hashp = &hash[0];
    size_t* global_idxp = &global_idx[0];
    for(size_t i = 0; i < size; i++) {
      hashp[i] = static_cast<int>(myhash(global_idxp[i], split_size));
    }
    std::vector<size_t> local_idx(size);
    size_t* local_idxp = &local_idx[0];
    for(size_t i = 0; i < size; i++) local_idxp[i] = i;
    auto sep = separate_to_bucket(hash, local_idx, split_size);
    split_idx.resize(split_size);
    for(size_t i = 0; i < split_size; i++) {
      size_t sepsize = sep[i].size();
      split_idx[i].resize(sepsize);
      size_t* split_idxp = &split_idx[i][0];
      size_t* sepp = &sep[i][0];
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
      for(size_t j = 0; j < sepsize; j++) {
        split_idxp[j] = global_idxp[sepp[j]];
      }
    }
  }
}

std::vector<size_t> get_unique_idx(std::vector<size_t>& idx) {
/*
  // separate hash version: seems to be slow
  size_t size = idx.size();
  // should not be zero
  if(size == 0) return std::vector<size_t>();
  size_t bucket_size_tmp =
    ceil_div(size, static_cast<size_t>(UNIQ_IDX_TARGET_BUCKET_LEN));
  size_t bucket_size =
  bucket_size_tmp < UNIQ_IDX_MAX_BUCKET_SIZE ?
    bucket_size_tmp : UNIQ_IDX_MAX_BUCKET_SIZE;
  std::vector<std::vector<size_t>> idx_split;
  std::vector<std::vector<size_t>> idx_out_split(bucket_size);
  split_idx_with_size(idx, idx_split, bucket_size);
  for(size_t i = 0; i < bucket_size; i++) {
    auto ht = unique_hashtable<size_t, int>(idx_split[i]);
    idx_out_split[i] = ht.all_keys();
  }
  return flatten(idx_out_split);
*/
/*
  // sort version
  auto idx2 = idx;
  size_t size = idx2.size();
  std::vector<int> dummy(size);
  radix_sort(idx2, dummy);
  return set_unique(idx2);
*/
  auto ht = unique_hashtable<size_t, int>(idx);
  return ht.all_keys();
}

vector<vector<size_t>> 
partition_global_index_bynode_helper(vector<size_t>& global_idx) {
  int nodesize = get_nodesize();
  size_t size = global_idx.size();
  std::vector<int> node(size);
  int* nodep = &node[0];
  size_t* global_idxp = &global_idx[0];
  for(size_t i = 0; i < size; i++) {
    nodep[i] = global_idxp[i] >> DFNODESHIFT;
  }
  return separate_to_bucket(node, global_idx, nodesize);
}

node_local<std::vector<std::vector<size_t>>> 
partition_global_index_bynode(node_local<std::vector<size_t>>& global_idx) {
  return global_idx.map(partition_global_index_bynode_helper);
}

std::vector<std::vector<size_t>>
remove_nodeinfo(std::vector<std::vector<size_t>>& exchanged_idx) {
  size_t nodemask = (size_t(1) << DFNODESHIFT) - 1;
  size_t size = exchanged_idx.size();
  std::vector<std::vector<size_t>> ret(size);
  for(size_t i = 0 ; i < size; i++) {
    size_t size_j = exchanged_idx[i].size();
    ret[i].resize(size_j);
    size_t* exchanged_idxp = &exchanged_idx[i][0];
    size_t* retp = &ret[i][0];
    for(size_t j = 0; j < size_j; j++) {
      retp[j] = exchanged_idxp[j] & nodemask;
    }
  }
  return ret;
}

// also removes node information
node_local<std::vector<std::vector<size_t>>> 
exchange_partitioned_index(node_local<std::vector<std::vector<size_t>>>&
                           partitioned_idx) {
  auto exchanged = alltoall_exchange(partitioned_idx);
  return exchanged.map(remove_nodeinfo);
}

std::vector<size_t>
make_to_store_idx_helper(std::vector<std::vector<size_t>>&
                         partitioned_idx,
                         std::vector<size_t>& global_idx) {
  auto flattened_partitioned_idx = flatten(partitioned_idx);
  auto size = flattened_partitioned_idx.size();
  std::vector<size_t> iota(size);
  auto iotap = iota.data();
  for(size_t i = 0; i < size; i++) iotap[i] = i;
  auto ht = unique_hashtable<size_t, size_t>(flattened_partitioned_idx, iota);
  return ht.lookup(global_idx);
}

node_local<std::vector<size_t>> 
make_to_store_idx(node_local<std::vector<std::vector<size_t>>>&
                  partitioned_idx,
                  node_local<std::vector<size_t>>& global_idx) {
  return partitioned_idx.map(make_to_store_idx_helper, global_idx);
}

// return with node information
std::vector<std::vector<size_t>>
global_extract_null_helper(std::vector<size_t>& nulls,
                           std::vector<std::vector<size_t>>& exchanged_idx) {
  size_t selfid = static_cast<size_t>(get_selfid());  
  size_t nodeinfo = selfid << DFNODESHIFT;
  size_t size = exchanged_idx.size();
  std::vector<std::vector<size_t>> part_nulls(size);
  size_t nullsize = nulls.size();
  if(nullsize == 0) return part_nulls; // quick return
  std::vector<int> dummy(nullsize);
  auto ht = unique_hashtable<size_t, int>(nulls, dummy);
  for(size_t i = 0; i < size; i++) {
    auto hit = ht.check_existence(exchanged_idx[i]);
    auto hit_idx = vector_find_one(hit);
    auto hit_idx_size = hit_idx.size();
    part_nulls[i].resize(hit_idx_size);
    size_t* part_nulls_ip = &part_nulls[i][0];
    size_t* exchanged_idx_ip = &exchanged_idx[i][0];
    auto hit_idxp = hit_idx.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t j = 0; j < hit_idx_size; j++) {
      part_nulls_ip[j] = exchanged_idx_ip[hit_idxp[j]] + nodeinfo;
    }
  }
  return part_nulls;
}

unique_hashtable<size_t, int>
create_null_hash_from_partition(std::vector<std::vector<size_t>>& part_idx,
                                int& null_exists) {
  size_t total = 0;
  size_t size = part_idx.size();
  for(size_t i = 0; i < size; i++) total += part_idx[i].size();
  vector<size_t> nulls(total);
  vector<int> dummy(total);
  size_t current = 0;
  size_t* nullsp = &nulls[0];
  for(size_t i = 0; i < size; i++) {
    size_t part_size = part_idx[i].size();
    size_t* part_idxp = &part_idx[i][0];
    for(size_t j = 0; j < part_size; j++) {
      nullsp[current++] = part_idxp[j];
    }
  }
  if(total == 0) null_exists = 0; else null_exists = 1;
  return unique_hashtable<size_t, int>(nulls, dummy);
}

std::vector<size_t>
global_extract_null_helper2(unique_hashtable<size_t, int>& hashtable,
                            std::vector<size_t>& global_idx,
                            int& null_exists) {
  if(null_exists) {
    vector<int> isnull = hashtable.check_existence(global_idx);
    return vector_find_one(isnull);
  } else {
    return vector<size_t>();
  }
}

#if !(defined(_SX) || defined(__ve__))
std::vector<std::vector<size_t>> separate_to_bucket(std::vector<int>& key,
                                                    std::vector<size_t>& idx,
                                                    size_t num_bucket) {
  std::vector<std::vector<size_t>> ret(num_bucket);
  for(size_t i = 0; i < key.size(); i++) {
    ret[key[i]].push_back(idx[i]);
  }
  return ret;
}
#else
// mostly same as radix sort; used for join
// num_bucket should be small enough for memory usage
// contents of key must be 0 to num_bucket - 1
// returns separated index
//
// TODO: improve performance by unrolling, etc.
std::vector<std::vector<size_t>> separate_to_bucket(std::vector<int>& key,
                                                    std::vector<size_t>& idx,
                                                    size_t num_bucket) {
  std::vector<std::vector<size_t>> ret(num_bucket);
  size_t size = key.size();
  std::vector<size_t> rettmp(size);
  size_t* rettmpp = rettmp.data();

  size_t bucket_ldim = SEPARATE_TO_BUCKET_VLEN + 1;
  // bucket_table is columnar (VLEN + 1) by num_bucket matrix
  // "1" is to avoid bank conflict, but reused for "rest" of the data
  std::vector<size_t> bucket_table(num_bucket * bucket_ldim);
  // +1 for bucket_sum
  std::vector<size_t> px_bucket_table(num_bucket * bucket_ldim + 1);
  size_t* bucket_tablep = &bucket_table[0];
  size_t* px_bucket_tablep = &px_bucket_table[0];
  std::vector<size_t> bucket_sum(num_bucket);
  size_t* bucket_sump = &bucket_sum[0];
  int bucket[SEPARATE_TO_BUCKET_VLEN];
#pragma _NEC vreg(bucket)

  std::vector<size_t> pos(size);
  size_t* posp = &pos[0];
  size_t block_size = size / SEPARATE_TO_BUCKET_VLEN;
  if(block_size % 2 == 0 && block_size != 0) block_size -= 1;
  size_t rest = size - SEPARATE_TO_BUCKET_VLEN * block_size;
  for(size_t i = 0; i < bucket_table.size(); i++) bucket_tablep[i] = 0;
  for(size_t i = 0; i < num_bucket; i++) bucket_sump[i] = 0;
  int *keyp = &key[0];
  size_t *idxp = &idx[0];
  for(size_t b = 0; b < block_size; b++) { // b: block
#pragma cdir nodep
#pragma _NEC ivdep
    for(int v = 0; v < SEPARATE_TO_BUCKET_VLEN; v++) { // vector loop, raking
      bucket[v] = keyp[block_size * v + b];
      posp[block_size * v + b] = bucket_tablep[bucket_ldim * bucket[v] + v];
      bucket_tablep[bucket_ldim * bucket[v] + v]++;
    }
  }
  size_t v = SEPARATE_TO_BUCKET_VLEN;
  for(int b = 0; b < rest; b++) { // not vector loop
    int bucket = keyp[block_size * v + b];
    posp[block_size * v + b] = bucket_tablep[bucket_ldim * bucket + v];
    bucket_tablep[bucket_ldim * bucket + v]++;
  }
  // preparing for the copy
  prefix_sum(bucket_tablep, px_bucket_tablep + 1, num_bucket * bucket_ldim);
  // now copy the data to the bucket
#pragma _NEC vob
  for(size_t b = 0; b < block_size; b++) { // b: block
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
    for(int v = 0; v < SEPARATE_TO_BUCKET_VLEN; v++) { // vector loop, raking
      int bucket = keyp[block_size * v + b];
      size_t to = px_bucket_tablep[bucket_ldim * bucket + v] +
        posp[block_size * v + b];
      rettmpp[to] = idxp[block_size * v + b];
    }
  }
  v = SEPARATE_TO_BUCKET_VLEN;
#pragma cdir nodep
#pragma _NEC ivdep
  for(size_t b = 0; b < rest; b++) {
    int bucket = keyp[block_size * v + b];
    size_t to = px_bucket_tablep[bucket_ldim * bucket + v] +
      posp[block_size * v + b];
    rettmpp[to] = idxp[block_size * v + b];
  }

  for(size_t i = 0; i < num_bucket; i++) {
    bucket_sump[i] +=
      px_bucket_tablep[bucket_ldim * (i + 1)] -
      px_bucket_tablep[bucket_ldim * i];
  }
  size_t current = 0;
  for(size_t i = 0; i < num_bucket; i++) {
    ret[i].resize(bucket_sump[i]);
    auto retp = ret[i].data();
    auto rettmp2 = rettmpp + current;
    for(size_t j = 0; j < bucket_sump[i]; j++) {
      retp[j] = rettmp2[j];
    }
    current += bucket_sump[i];
  }
  return ret;
}
#endif

std::vector<size_t>
local_to_global_idx_helper(std::vector<size_t>& local_idx) {
  size_t size = local_idx.size();
  std::vector<size_t> global_idx(size);
  size_t selfid = static_cast<size_t>(get_selfid());
  size_t nodeinfo = selfid << DFNODESHIFT;
  size_t* global_idxp = &global_idx[0];
  size_t* local_idxp = &local_idx[0];
  for(size_t i = 0; i < size; i++) {
    global_idxp[i] = local_idxp[i] + nodeinfo;
  }
  return global_idx;
}

node_local<std::vector<size_t>>
local_to_global_idx(node_local<std::vector<size_t>>& local_idx) {
  return local_idx.map(local_to_global_idx_helper);
}

node_local<std::vector<size_t>>
limit_nulls_head(node_local<std::vector<size_t>>& nulls, 
                 const std::vector<size_t>& sizes,
                 size_t limit) {
  int nodesize = get_nodesize();
  std::vector<size_t> limits(nodesize);
  std::vector<size_t> pxsizes(nodesize);
  auto sizesp = sizes.data();
  auto pxsizesp = pxsizes.data();
  auto limitsp = limits.data();
  for(size_t i = 1; i < nodesize; i++) {
    pxsizesp[i] = pxsizesp[i-1] + sizesp[i-1];
  }
  for(size_t i = 0; i < nodesize; i++) {
    if(pxsizesp[i] > limit) limitsp[i] = 0;
    else limitsp[i] = limit - pxsizesp[i];
  }
  auto slimits = make_node_local_scatter(limits);
  return nulls.map(+[](std::vector<size_t>& nulls, size_t limit) {
      size_t end =
        std::lower_bound(nulls.begin(), nulls.end(), limit) - nulls.begin();
      std::vector<size_t> ret(end);
      auto retp = ret.data();
      auto nullsp = nulls.data();
      for(size_t i = 0; i < end; i++) {
        retp[i] = nullsp[i];
      }
      return ret;
    }, slimits);
}

node_local<std::vector<size_t>>
limit_nulls_tail(node_local<std::vector<size_t>>& nulls, 
                 const std::vector<size_t>& sizes,
                 const std::vector<size_t>& new_sizes,
                 size_t limit) {
  int nodesize = get_nodesize();
  std::vector<size_t> limits(nodesize);
  std::vector<size_t> pxsizes(nodesize);
  auto sizesp = sizes.data();
  auto pxsizesp = pxsizes.data();
  auto limitsp = limits.data();
  for(size_t i = 1; i < nodesize; i++) {
    pxsizesp[i] = pxsizesp[i-1] + sizesp[i-1];
  }
  auto total = pxsizesp[nodesize-1] + sizesp[nodesize-1];
  auto limit_start = total - limit;
  for(size_t i = 0; i < nodesize; i++) {
    if(pxsizesp[i] > limit_start) limitsp[i] = 0;
    else limitsp[i] = limit_start - pxsizesp[i];
  }
  auto slimits = make_node_local_scatter(limits);
  std::vector<size_t> sizediff(nodesize);
  auto sizediffp = sizediff.data();
  auto new_sizesp = new_sizes.data();
  for(size_t i = 0; i < nodesize; i++) {
    sizediffp[i] = sizesp[i] - new_sizesp[i];
  }
  auto nlsizediff = make_node_local_scatter(sizediff);
  return nulls.map(+[](std::vector<size_t>& nulls, size_t limit,
                       size_t sizediff) {
      size_t start =
        std::lower_bound(nulls.begin(), nulls.end(), limit) - nulls.begin();
      size_t retsize = nulls.size() - start;
      std::vector<size_t> ret(retsize);
      auto retp = ret.data();
      auto nullsp = nulls.data();
      for(size_t i = 0; i < retsize; i++) {
        retp[i] = nullsp[i+start] - sizediff;
      }
      return ret;
    }, slimits, nlsizediff);
}

#if defined(_SX) || defined(__ve__)
void create_merge_map(std::vector<size_t>& nodeid,
                      std::vector<size_t>& split,
                      std::vector<std::vector<size_t>>& merge_map) {
  auto nodeidsize = nodeid.size();
  auto split_size = split.size();
  auto merge_map_size = merge_map.size();
  std::vector<size_t*> merge_mapp(merge_map_size);
  auto merge_mappp = merge_mapp.data();
  for(size_t i = 0; i < merge_map_size; i++) {
    merge_mappp[i] = merge_map[i].data();
  }
  auto splitp = split.data();
  auto nodeidp = nodeid.data();
  int valid[CREATE_MERGE_MAP_VLEN];
  size_t start_idx[CREATE_MERGE_MAP_VLEN];
  size_t current_idx[CREATE_MERGE_MAP_VLEN];
  size_t stop_idx[CREATE_MERGE_MAP_VLEN];
  size_t stop_idx_work[CREATE_MERGE_MAP_VLEN];
// start_idx is not used in for loop
#pragma _NEC vreg(valid)
#pragma _NEC vreg(current_idx)
#pragma _NEC vreg(stop_idx)
  auto each = ceil_div(nodeidsize, size_t(CREATE_MERGE_MAP_VLEN));
  if(each % 2 == 0) each++;
  start_idx[0] = 0;
  size_t current_split[CREATE_MERGE_MAP_VLEN];
  current_split[0] = 0;
  auto split_start = split.begin();
  for(size_t i = 1; i < CREATE_MERGE_MAP_VLEN; i++) {
    split_start = std::lower_bound(split_start, split.end()-1, each * i);
    if(split_start == split.end()-1) {
      for(size_t j = i; j < CREATE_MERGE_MAP_VLEN; j++) {
        start_idx[j] = nodeidsize;
        current_split[j] = split_size-1;
      }
      break;
    } else {
      auto tmp = split_start - split.begin();
      current_split[i] = tmp;
      start_idx[i] = splitp[tmp];
    }
  }
  for(size_t i = 0; i < CREATE_MERGE_MAP_VLEN; i++) {
    current_idx[i] = start_idx[i];
  }
  for(size_t i = 0; i < CREATE_MERGE_MAP_VLEN-1; i++) {
    stop_idx_work[i] = start_idx[i+1];
  }
  stop_idx_work[CREATE_MERGE_MAP_VLEN-1] = nodeidsize;
  for(size_t i = 0; i < CREATE_MERGE_MAP_VLEN; i++) {
    stop_idx[i] = stop_idx_work[i];
  }
  for(size_t i = 0; i < CREATE_MERGE_MAP_VLEN; i++) {
    if(stop_idx[i] == start_idx[i]) valid[i] = false;
    else valid[i] = true;
  }
  size_t max = 0;
  for(size_t i = 0; i < CREATE_MERGE_MAP_VLEN; i++) {
    if(stop_idx[i] - start_idx[i] > max) max = stop_idx[i] - start_idx[i];
  }
  // count to find starting point
  std::vector<size_t> counter(CREATE_MERGE_MAP_VLEN * merge_map_size);
  auto counterp = counter.data();
  for(size_t i = 0; i < max; i++) {
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t j = 0; j < CREATE_MERGE_MAP_VLEN; j++) {
      if(valid[j]) {
        auto to = nodeidp[current_idx[j]++];
        counterp[j * merge_map_size + to]++;
        if(current_idx[j] == stop_idx[j]) valid[j] = false;
      }
    }
  }
  std::vector<size_t> store_idx(CREATE_MERGE_MAP_VLEN * merge_map_size);
  auto store_idxp = store_idx.data();
  for(size_t i = 1; i < CREATE_MERGE_MAP_VLEN; i++) {
    for(size_t j = 0; j < merge_map_size; j++) {
      store_idxp[i * merge_map_size + j] =
        store_idxp[(i - 1) * merge_map_size + j] +
        counterp[(i - 1) * merge_map_size + j];
    }
  }
  for(size_t i = 0; i < CREATE_MERGE_MAP_VLEN; i++) {
    current_idx[i] = start_idx[i];
  }
  for(size_t i = 0; i < CREATE_MERGE_MAP_VLEN; i++) {
    if(stop_idx[i] == start_idx[i]) valid[i] = false;
    else valid[i] = true;
  }
  for(size_t i = 0; i < max; i++) {
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t j = 0; j < CREATE_MERGE_MAP_VLEN; j++) {
      if(valid[j]) {
        auto to = nodeidp[current_idx[j]++];
        merge_mappp[to][store_idxp[j * merge_map_size + to]++] =
          current_split[j];
        if(current_idx[j] == splitp[current_split[j]+1]) {
          current_split[j]++;
        }
        if(current_idx[j] == stop_idx[j]) valid[j] = false;
      }
    }
  }
}
#else
void create_merge_map(std::vector<size_t>& nodeid,
                      std::vector<size_t>& split,
                      std::vector<std::vector<size_t>>& merge_map) {
  auto split_size = split.size();
  auto merge_map_size = merge_map.size();
  std::vector<size_t> current_idx(merge_map_size);
  size_t current = 0;
  for(size_t i = 0; i < split_size-1; i++) {
    for(size_t j = 0; j < split[i+1] - split[i]; j++) {
      auto to = nodeid[current++];
      merge_map[to][current_idx[to]++] = i;
    }
  }
}
#endif

std::vector<size_t>
fill_sized_idx(const size_t* idxp, const size_t* idx_lenp, size_t size) {
  if(size == 0) return std::vector<size_t>();
  size_t total = 0;
  for(size_t i = 0; i < size; i++) {
    total += idx_lenp[i];
  }
  std::vector<size_t> ret(total);
  auto retp = ret.data();
  if(total / size > FILL_SIZED_IDX_VLEN * 2) {
    size_t crnt = 0;
    for(size_t i = 0; i < size; i++) {
      for(size_t j = 0; j < idx_lenp[i]; j++) {
        retp[crnt + j] = idxp[i];
      }
      crnt += idx_lenp[i];
    }
    return ret;
  } else {
    std::vector<size_t> px_idx_len(size+1);
    auto px_idx_lenp = px_idx_len.data();
    prefix_sum(idx_lenp, px_idx_lenp+1, size);
    auto num_block = size / FILL_SIZED_IDX_VLEN;
    auto rest = size - num_block * FILL_SIZED_IDX_VLEN;
    for(size_t b = 0; b < num_block; b++) {
      size_t offset = b * FILL_SIZED_IDX_VLEN;
      auto offset_idx_lenp = idx_lenp + offset;
      auto offset_px_idx_lenp = px_idx_lenp + offset;
      auto offset_idxp = idxp + offset;
      size_t max = 0;
      for(size_t i = 0; i < FILL_SIZED_IDX_VLEN; i++) {
        if(max < offset_idx_lenp[i]) max = offset_idx_lenp[i];
      }
      for(size_t i = 0; i < max; i++) {
#pragma _NEC vob
#pragma _NEC vovertake
#pragma _NEC ivdep
        for(size_t j = 0; j < FILL_SIZED_IDX_VLEN; j++) {
           if(i < offset_idx_lenp[j]) {
            retp[offset_px_idx_lenp[j]+i] = offset_idxp[j];
          }
        }
      }
    }
    size_t offset = num_block * FILL_SIZED_IDX_VLEN;
    auto offset_idx_lenp = idx_lenp + offset;
    auto offset_px_idx_lenp = px_idx_lenp + offset;
    auto offset_idxp = idxp + offset;
    size_t max = 0;
    for(size_t i = 0; i < rest; i++) {
      if(max < offset_idx_lenp[i]) max = offset_idx_lenp[i];
    }
    for(size_t i = 0; i < max; i++) {
#pragma _NEC vob
#pragma _NEC vovertake
#pragma _NEC ivdep
      for(size_t j = 0; j < rest; j++) {
        if(i < offset_idx_lenp[j]) {
          retp[offset_px_idx_lenp[j]+i] = offset_idxp[j];
        }
      }
    }
    return ret;
  }
}

std::vector<std::string> 
words_to_string_vector(words& ws, 
                       std::vector<size_t>& nulls,
                       const std::string& nullstr) {
  auto lensp = ws.lens.data();
  auto startsp = ws.starts.data();
  auto size = ws.lens.size();
  std::vector<std::string> ret(size);
  auto temp = int_to_char(ws.chars);
  for(size_t i = 0; i < size; ++i) ret[i] = temp.substr(startsp[i], lensp[i]);
  for(size_t i = 0; i < nulls.size(); ++i) ret[nulls[i]] = nullstr;
  return ret;
}

template <>
dvector<std::string> dfcolumn::as_dvector() {
  dvector<std::string> ret;
  std::string nullstr = "NULL";
  if (dtype() == "string") {
    auto& typed_col = dynamic_cast<typed_dfcolumn<std::string>&>(*this);
    ret = typed_col.get_val().as_dvector<std::string>();
  }
  else if (dtype() == "dic_string") {
    auto& typed_col = dynamic_cast<typed_dfcolumn<dic_string>&>(*this);
    ret = typed_col.as_words().map(words_to_string_vector, 
                                   get_nulls(), broadcast(nullstr))
                              .as_dvector<std::string>();
  }
  else if (dtype() == "raw_string") {
    auto& typed_col = dynamic_cast<typed_dfcolumn<raw_string>&>(*this);
    ret = typed_col.as_words().map(words_to_string_vector,
                                   get_nulls(), broadcast(nullstr))
                              .as_dvector<std::string>();
  }
  else {
    throw std::runtime_error("as_dvector<std::string>: non-string column is specified with string target!");
  }
  return ret;
}

// for spill-restore

dfcolumn_spill_queue_t dfcolumn_spill_queue;
dfcolumn_spillable_t dfcolumn_spillable;

dfcolumn_spillable_t::dfcolumn_spillable_t() {
  auto envval = getenv("FROVEDIS_DFCOLUMN_SPILLABLE");
  if(envval == NULL) is_spillable = false;
  else if(std::string(envval) == "true") is_spillable = true;
  else is_spillable = false;
}

void dfcolumn::spill() {
  if(spillable) {
    if(!spill_initialized) { // spill_state should be "restored"
      init_spill();
      dfcolumn_spill_queue.put_new(this);
      spill_state = spill_state_type::spilled;
    } else if(spill_state == spill_state_type::restored) {
      dfcolumn_spill_queue.put(this);
      spill_state = spill_state_type::spilled;
    }
  }
}

void dfcolumn::restore() {
  if(spillable) {
    if(spill_state == spill_state_type::spilled) {
      dfcolumn_spill_queue.get(this);
      spill_state = spill_state_type::restored;
    }
  }
}

extern std::string frovedis_tmpdir; // in mpi_rpc.cc

std::string create_spill_path_helper(intptr_t thisptr) {
  char buffer[100];
  std::sprintf(buffer, "%lx", thisptr);
  std::string thisptrstr(buffer);
  return frovedis_tmpdir + "/dfcolumn_" + thisptrstr;
}

node_local<std::string> create_spill_path(dfcolumn* ptr) {
  intptr_t thisptr = reinterpret_cast<intptr_t>(ptr);
  auto bthisptr = broadcast(thisptr);
  return bthisptr.map(+[](intptr_t thisptr){
      auto ret = create_spill_path_helper(thisptr);
      if(!directory_exists(frovedis_tmpdir)) make_directory(frovedis_tmpdir);
      make_directory(ret);
      return ret;
    });
}

// cannot be called from default ctor,
// because dfcolumn might be created partially and val might be set later
void dfcolumn::init_spill() {
  spill_size_cache = calc_spill_size();
  spill_path = create_spill_path(this);
  spill_initialized = true;
}

size_t dfcolumn::spill_size() {
  if(!spill_initialized) init_spill();
  return spill_size_cache;
}

dfcolumn::~dfcolumn() {
  if(spillable) {
    try {
      if(already_spilled_to_disk) {
        spill_path.mapv(+[](const std::string& path){
            // may be moved by move ctor/assignment
            if(directory_exists(path)) remove_directory(path);
          });
      }
      dfcolumn_spill_queue.remove(this);
    } catch (std::exception& e) {
      // exception should not be thrown from dtor
      RLOG(WARNING) << "exception caught at dtor of dfcolumn: " << e.what()
                    << std::endl;
    } 
  }
}

dfcolumn::dfcolumn(const dfcolumn& c) : 
  spillable(dfcolumn_spillable.get()), spill_initialized(false),
  already_spilled_to_disk(false), cleared(false),
  spill_state(spill_state_type::restored),
  spill_size_cache(-1) {
  if(c.cleared) {
    // special handling is needed for actually spilled dfcolumn
    spill_size_cache = c.spill_size_cache;
    spill_path = create_spill_path(this);
    spill_initialized = true;
    spill_path.mapv(+[](const std::string& to, const std::string& from) {
        copy_directory(from, to);
      }, c.spill_path);
    cleared = true;
    already_spilled_to_disk = true;
    spill_state = spill_state_type::spilled;
  }
}

dfcolumn::dfcolumn(dfcolumn&& c) :
  spillable(dfcolumn_spillable.get()), spill_initialized(false),
  already_spilled_to_disk(false), cleared(false),
  spill_state(spill_state_type::restored),
  spill_size_cache(-1) {
  if(c.cleared) {
    // special handling is needed for actually spilled dfcolumn
    spill_size_cache = c.spill_size_cache;
    spill_path = create_spill_path(this);
    spill_initialized = true;
    spill_path.mapv(+[](const std::string& to, const std::string& from) {
        int r = rename(from.c_str(), to.c_str());
        if(r) throw std::runtime_error("move ctor of dfcolumn: " +
                                       std::string(strerror(errno)));
      }, c.spill_path);
    cleared = true;
    already_spilled_to_disk = true;
    spill_state = spill_state_type::spilled;
  }
}

dfcolumn& dfcolumn::operator=(const dfcolumn& c) {
  spillable = dfcolumn_spillable.get();
  spill_initialized = false;
  already_spilled_to_disk = false;
  cleared = false;
  spill_state = spill_state_type::restored;
  spill_size_cache = -1;
  if(c.cleared) {
    // special handling is needed for actually spilled dfcolumn
    spill_size_cache = c.spill_size_cache;
    spill_path = create_spill_path(this);
    spill_initialized = true;
    spill_path.mapv(+[](const std::string& to, const std::string& from) {
        copy_directory(from, to);
      }, c.spill_path);
    cleared = true;
    already_spilled_to_disk = true;
    spill_state = spill_state_type::spilled;
  }
  return *this;
}

dfcolumn& dfcolumn::operator=(dfcolumn&& c) {
  spillable = dfcolumn_spillable.get();
  spill_initialized = false;
  already_spilled_to_disk = false;
  cleared = false;
  spill_state = spill_state_type::restored;
  spill_size_cache = -1;
  if(c.cleared) {
    // special handling is needed for actually spilled dfcolumn
    spill_size_cache = c.spill_size_cache;
    spill_path = create_spill_path(this);
    spill_initialized = true;
    spill_path.mapv(+[](const std::string& to, const std::string& from) {
        int r = rename(from.c_str(), to.c_str());
        if(r) throw std::runtime_error("move assignment of dfcolumn: " +
                                       std::string(strerror(errno)));
      }, c.spill_path);
    cleared = true;
    already_spilled_to_disk = true;
    spill_state = spill_state_type::spilled;
  }
  return *this;
}

void dfcolumn_spill_queue_t::init_queue_capacity() {
  // can't be done at ctor, because get_nodesize() is not set yet
  auto queue_capacity_char = getenv("FROVEDIS_DFCOLUMN_SPILLQ_SIZE");
  if(queue_capacity_char != NULL) {
    // in MB
    queue_capacity =
      size_t(atoi(queue_capacity_char)) * size_t(1024) * size_t(1024);
  } else {
    // 1GB in total
    queue_capacity = size_t(1024) * size_t(1024) * size_t(1024);
  }
  capacity_initialized = true;
}

void dfcolumn_spill_queue_t::put_new(dfcolumn* c) {
  if(!capacity_initialized) init_queue_capacity();
  number_of_put++;
  if(item_map.count(c) > 0) {
    throw std::runtime_error
      ("internal error: dfcolumn_spill_queue::put_new called existing column");
  }
  item_list.push_front(c);
  item_map.insert(std::make_pair(c, item_list.begin()));
  auto csize = c->spill_size();
  current_usage += csize;
  spill_until_capacity();
}

void dfcolumn_spill_queue_t::put(dfcolumn* c) {
  if(!capacity_initialized) init_queue_capacity(); // shouldn't occur, though
  number_of_put++;
  if(item_map.count(c) > 0) {
    throw std::runtime_error
      ("internal error: dfcolumn_spill_queue::put called existing column");
  }
  item_list.push_front(c);
  item_map.insert(std::make_pair(c, item_list.begin()));
  auto csize = c->spill_size();
  current_usage += csize;
  queue_capacity += csize;
  spill_until_capacity();
}

void dfcolumn_spill_queue_t::get(dfcolumn* c) {
  if(!capacity_initialized) 
    throw std::runtime_error
      ("internal error: dfcolumn_spill_queue::get called before put");
  number_of_get++;
  auto csize = c->spill_size();
  queue_capacity -= csize; // queue_capacity might become negative
  auto it = item_map.find(c);
  if(it == item_map.end()) { // spilled
    spill_until_capacity();
    double t1 = get_dtime();
    c->restore_from_disk();
    number_of_restore_from_disk++;
    double t2 = get_dtime();
    restore_time += (t2 - t1);
  } else {
    item_list.erase(it->second);
    item_map.erase(it);
    current_usage -= csize; // current_usage should be positive or 0
    spill_until_capacity();
  }
}

void dfcolumn_spill_queue_t::remove(dfcolumn* c) {
  auto it = item_map.find(c);
  if(it != item_map.end()) { // in the queue
    item_list.erase(it->second);
    item_map.erase(it);
    current_usage -= c->spill_size();
  }
}

void dfcolumn_spill_queue_t::spill_until_capacity() {
  while(current_usage > queue_capacity && item_list.size() > 0) {
    spill_one();
  }
}

void dfcolumn_spill_queue_t::spill_all() {
  while(item_list.size() > 0) {
    spill_one();
  }
}

void dfcolumn_spill_queue_t::spill_one() {
  if(item_list.size() > 0) {
    auto c = *item_list.rbegin();
    double t1 = get_dtime();
    c->spill_to_disk();
    number_of_spill_to_disk++;
    double t2 = get_dtime();
    spill_time += (t2 - t1);
    item_list.pop_back();
    auto it = item_map.find(c);
    if(it == item_map.end())
      throw std::runtime_error
        ("internal error: dfcolumn_spill_queue::spill_one");
    else item_map.erase(it);
    auto csize = c->spill_size();
    current_usage -= csize;
  } else {
    RLOG(WARNING) << "dfcolumn_spill_queue::spill_one: no column to spill"
                  << std::endl;
  }
}

// converts bool-like case-insensitive words (Y, N, 1, 0, T, F, ON, OFF, YES, NO, 
// TRUE, FALSE) to boolean
std::vector<int>
words_to_bool(words& w) {
  auto nwords = w.lens.size();
  std::vector<int> word_to_bool_map(nwords);
  auto map_ptr = word_to_bool_map.data();
  auto charsp = w.chars.data();
  auto lensp = w.lens.data();
  auto startsp = w.starts.data();
  for(size_t i = 0; i < nwords; ++i) {
    auto len = lensp[i];
    auto st = startsp[i];
    int tf = -1;
    if (len == 1) {
      if ((charsp[st + 0] == 89 || charsp[st + 0] == 121) ||         // Y or y
          (charsp[st + 0] == 84 || charsp[st + 0] == 116) ||         // T or t
          (charsp[st + 0] == 49)) {                                  // 1
        tf = 1; 
      } else if ((charsp[st + 0] == 78 || charsp[st + 0] == 110) ||  // N or n
                 (charsp[st + 0] == 70 || charsp[st + 0] == 102) ||  // F or f
                 (charsp[st + 0] == 48)) {                           // 0
        tf = 0; 
      }
    } else if (len == 2) {
      if ((charsp[st + 0] == 79 || charsp[st + 0] == 111) &&
          (charsp[st + 1] == 78 || charsp[st + 1] == 110)) {         // ON
        tf = 1; 
      } else if ((charsp[st + 0] == 78 || charsp[st + 0] == 110) &&
                 (charsp[st + 1] == 79 || charsp[st + 1] == 111)) {  // NO
        tf = 0; 
      }
    } else if (len == 3) {
      if ((charsp[st + 0] == 89 || charsp[st + 0] == 121) &&
          (charsp[st + 1] == 69 || charsp[st + 1] == 101) &&
          (charsp[st + 2] == 83 || charsp[st + 2] == 115)) {         // YES
        tf = 1; 
      } else if ((charsp[st + 0] == 79 || charsp[st + 0] == 111) &&
                 (charsp[st + 1] == 70 || charsp[st + 1] == 102) && 
                 (charsp[st + 2] == 70 || charsp[st + 2] == 102)) {  // OFF
        tf = 0;
      }
    } else if (len == 4) {                                           // TRUE
      int T = charsp[st + 0] == 84 || charsp[st + 0] == 116;
      int R = charsp[st + 1] == 82 || charsp[st + 1] == 114;
      int U = charsp[st + 2] == 85 || charsp[st + 2] == 117;
      int E = charsp[st + 3] == 69 || charsp[st + 3] == 101;
      if (T & R & U & E) tf = 1;
    } else if (len == 5) {                                           // FALSE
      int F = charsp[st + 0] == 70 || charsp[st + 0] == 102;
      int A = charsp[st + 1] == 65 || charsp[st + 1] == 97;
      int L = charsp[st + 2] == 76 || charsp[st + 2] == 108;
      int S = charsp[st + 3] == 83 || charsp[st + 3] == 115;
      int E = charsp[st + 4] == 69 || charsp[st + 4] == 101;
      if (F & A & L & S & E) tf = 0;
    }
    map_ptr[i] = tf;
  }

  // would be short loop -> no need for vector_find_eq()
  for(size_t i = 0; i < nwords; ++i) {
    if (map_ptr[i] == -1) {
      auto len = lensp[i];
      auto st = startsp[i];
      std::string str;
      str.resize(len);
      int_to_char(charsp + st, len, const_cast<char*>(str.data()));
      REPORT_ERROR(USER_ERROR, "parsed string: '" + str + "' cannot be read as true/false");
    }
  }
  return word_to_bool_map;
}

}

