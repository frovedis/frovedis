#include "dfcolumn.hpp"
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

template <>
std::string dfcolumn::first(bool ignore_nulls) {
  if(dtype() == "string") {
    auto colp2 = dynamic_cast<typed_dfcolumn<std::string>*>(this);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    return colp2->first(ignore_nulls);
  } else if(dtype() == "dic_string") {
    auto colp2 = dynamic_cast<typed_dfcolumn<dic_string>*>(this);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    return colp2->first(ignore_nulls);
  } else if(dtype() == "raw_string") {
    auto colp2 = dynamic_cast<typed_dfcolumn<raw_string>*>(this);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    return colp2->first(ignore_nulls);
  } else throw std::runtime_error("unsupported type: " + dtype());
}

template <>
std::string dfcolumn::last(bool ignore_nulls) {
  if(dtype() == "string") {
    auto colp2 = dynamic_cast<typed_dfcolumn<std::string>*>(this);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    return colp2->last(ignore_nulls);
  } else if(dtype() == "dic_string") {
    auto colp2 = dynamic_cast<typed_dfcolumn<dic_string>*>(this);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    return colp2->last(ignore_nulls);
  } else if(dtype() == "raw_string") {
    auto colp2 = dynamic_cast<typed_dfcolumn<raw_string>*>(this);
    if(!static_cast<bool>(colp2))
      throw std::runtime_error("internal type error");
    return colp2->last(ignore_nulls);
  } else throw std::runtime_error("unsupported type: " + dtype());
}

std::shared_ptr<dfcolumn> dfcolumn::substr(int pos, int num) {
  if(pos > 0) pos--; // convert 1-based to 0-based
  // to avoid error in substr
  auto ws = pos >= 0 ? as_words(6,"%Y-%m-%d",false,std::string(pos+num, 'N'))
    : as_words(6,"%Y-%m-%d",false,std::string(-pos, 'N'));
  auto nulls = get_nulls();
  ws.mapv(+[](words& ws){ws.utf8_to_utf32();});
  ws.mapv(+[](words& ws, int pos, int num){ws.substr(pos, num);},
          broadcast(pos), broadcast(num));
  ws.mapv(+[](words& ws){ws.utf32_to_utf8();});
  if(dtype() == "string") {
    // TODO: in the case of string, not using words would be faster
    auto vs = ws.map(+[](words& ws){return words_to_vector_string(ws);});
    auto ret = std::make_shared<typed_dfcolumn<std::string>>
      (std::move(vs), std::move(nulls));
    return ret;
  } else if (dtype() == "raw_string") {
    auto ret = std::make_shared<typed_dfcolumn<raw_string>>
      (std::move(ws), std::move(nulls));
    return ret;
  } else {
    auto ret = std::make_shared<typed_dfcolumn<dic_string>>
      (std::move(ws), std::move(nulls));
    return ret;
  }
}

std::shared_ptr<dfcolumn>
dfcolumn::substr(const std::shared_ptr<dfcolumn>& pos, int num) {
  if(pos->is_all_null()) {
    if(dtype() == "string") {
      return create_null_column<std::string>(sizes());
    } else if (dtype() == "raw_string") {
      return create_null_column<raw_string>(sizes());
    } else {
      return create_null_column<dic_string>(sizes());
    }
  } else {
    auto nulls = get_nulls();
    auto intcol = pos->type_cast("int");
    auto tintcol = dynamic_pointer_cast<typed_dfcolumn<int>>(intcol);
    if(!static_cast<bool>(tintcol))
      throw std::runtime_error("internal cast error");
    // to guard when words does not contain num size chars
    auto ws = as_words(6,"%Y-%m-%d",false,std::string(num,'N'));
    ws.mapv(+[](words& ws){ws.utf8_to_utf32();});
    if(tintcol->if_contain_nulls() || if_contain_nulls()) {
      auto posval = tintcol->val;
      posval.mapv
        (+[](std::vector<int>& val,
             std::vector<size_t>& nulls1,
             std::vector<size_t>& nulls2) {
          auto valp = val.data();
          auto nulls1p = nulls1.data();
          auto nulls1_size = nulls1.size();
          auto nulls2p = nulls2.data();
          auto nulls2_size = nulls2.size();
#pragma _NEC vob
#pragma _NEC vovertake
#pragma _NEC ivdep
          for(size_t i = 0; i < nulls1_size; i++) valp[nulls1p[i]] = 0;
#pragma _NEC vob
#pragma _NEC vovertake
#pragma _NEC ivdep
          for(size_t i = 0; i < nulls2_size; i++) valp[nulls2p[i]] = 0;
        }, tintcol->nulls, nulls);
      ws.mapv
        (+[](words& ws, const std::vector<size_t>& nulls1,
             const std::vector<size_t>& nulls2, int num) {
          auto lensp = ws.lens.data();
          auto startsp = ws.starts.data();
          auto nulls1p = nulls1.data();
          auto nulls1_size = nulls1.size();
          auto nulls2p = nulls2.data();
          auto nulls2_size = nulls2.size();
#pragma _NEC vob
#pragma _NEC vovertake
#pragma _NEC ivdep
          for(size_t i = 0; i < nulls1_size; i++) {
            startsp[nulls1p[i]] = 0;
            lensp[nulls1p[i]] = num;
          }
          for(size_t i = 0; i < nulls2_size; i++) {
            startsp[nulls2p[i]] = 0;
            lensp[nulls2p[i]] = num;
          }
        }, tintcol->nulls, nulls, broadcast(num));
      ws.mapv(+[](words& ws, std::vector<int>& pos, int num){
          auto posp = pos.data();
          auto pos_size = pos.size();
          for(size_t i = 0; i < pos_size; i++) {if(posp[i] > 0) posp[i]--;}
          ws.substr(pos, num);
        }, posval, broadcast(num));
    } else {
      ws.mapv(+[](words& ws, std::vector<int>& pos, int num){
          auto posp = pos.data();
          auto pos_size = pos.size();
          for(size_t i = 0; i < pos_size; i++) {if(posp[i] > 0) posp[i]--;}
          ws.substr(pos, num);
        }, tintcol->val, broadcast(num));
    }
    if(tintcol->if_contain_nulls()) {
      nulls.mapv(+[](std::vector<size_t>& nulls,
                     std::vector<size_t>& colnulls) {
                   nulls = set_union(nulls, colnulls);
                 }, tintcol->nulls);
    }
    ws.mapv(+[](words& ws){ws.utf32_to_utf8();});
    if(dtype() == "string") {
      auto vs = ws.map(+[](words& ws){return words_to_vector_string(ws);});
      auto ret = std::make_shared<typed_dfcolumn<std::string>>
        (std::move(vs), std::move(nulls));
      return ret;
    } else if (dtype() == "raw_string") {
      auto ret = std::make_shared<typed_dfcolumn<raw_string>>
        (std::move(ws), std::move(nulls));
      return ret;
    } else {
      auto ret = std::make_shared<typed_dfcolumn<dic_string>>
        (std::move(ws), std::move(nulls));
      return ret;
    }
  }
}

std::shared_ptr<dfcolumn>
dfcolumn::substr(int pos, const std::shared_ptr<dfcolumn>& num) {
  if(pos > 0) pos--; // convert 1-based to 0-based
  if(num->is_all_null()) {
    if(dtype() == "string") {
      return create_null_column<std::string>(sizes());
    } else if (dtype() == "raw_string") {
      return create_null_column<raw_string>(sizes());
    } else {
      return create_null_column<dic_string>(sizes());
    }
  } else {
    auto nulls = get_nulls();
    auto intcol = num->type_cast("int");
    auto tintcol = dynamic_pointer_cast<typed_dfcolumn<int>>(intcol);
    if(!static_cast<bool>(tintcol))
      throw std::runtime_error("internal cast error");
    // to guard when words does not contain pos size chars
    auto ws = as_words(6,"%Y-%m-%d",false,std::string(std::abs(pos),'N'));
    ws.mapv(+[](words& ws){ws.utf8_to_utf32();});
    if(tintcol->if_contain_nulls() || if_contain_nulls()) {
      auto numval = tintcol->val;
      numval.mapv
        (+[](std::vector<int>& val,
             std::vector<size_t>& nulls1,
             std::vector<size_t>& nulls2) {
          auto valp = val.data();
          auto nulls1p = nulls1.data();
          auto nulls1_size = nulls1.size();
          auto nulls2p = nulls2.data();
          auto nulls2_size = nulls2.size();
#pragma _NEC vob
#pragma _NEC vovertake
#pragma _NEC ivdep
          for(size_t i = 0; i < nulls1_size; i++) valp[nulls1p[i]] = 0;
#pragma _NEC vob
#pragma _NEC vovertake
#pragma _NEC ivdep
          for(size_t i = 0; i < nulls2_size; i++) valp[nulls2p[i]] = 0;
        }, tintcol->nulls, nulls);
      ws.mapv
        (+[](words& ws, const std::vector<size_t>& nulls1,
             const std::vector<size_t>& nulls2, int abspos) {
          auto lensp = ws.lens.data();
          auto startsp = ws.starts.data();
          auto nulls1p = nulls1.data();
          auto nulls1_size = nulls1.size();
          auto nulls2p = nulls2.data();
          auto nulls2_size = nulls2.size();
#pragma _NEC vob
#pragma _NEC vovertake
#pragma _NEC ivdep
          for(size_t i = 0; i < nulls1_size; i++) {
            startsp[nulls1p[i]] = 0;
            lensp[nulls1p[i]] = abspos;
          }
#pragma _NEC vob
#pragma _NEC vovertake
#pragma _NEC ivdep
          for(size_t i = 0; i < nulls2_size; i++) {
            startsp[nulls2p[i]] = 0;
            lensp[nulls2p[i]] = abspos;
          }
        }, tintcol->nulls, nulls, broadcast(std::abs(pos)));
      ws.mapv(+[](words& ws, int pos, std::vector<int>& num){
          ws.substr(pos, num);
        }, broadcast(pos), numval);
    } else {
      ws.mapv(+[](words& ws, int pos, std::vector<int>& num){
          ws.substr(pos, num);
        }, broadcast(pos), tintcol->val);
    }
    if(tintcol->if_contain_nulls()) {
      nulls.mapv(+[](std::vector<size_t>& nulls,
                     std::vector<size_t>& colnulls) {
                   nulls = set_union(nulls, colnulls);
                 }, tintcol->nulls);
    }
    ws.mapv(+[](words& ws){ws.utf32_to_utf8();});
    if(dtype() == "string") {
      auto vs = ws.map(+[](words& ws){return words_to_vector_string(ws);});
      auto ret = std::make_shared<typed_dfcolumn<std::string>>
        (std::move(vs), std::move(nulls));
      return ret;
    } else if (dtype() == "raw_string") {
      auto ret = std::make_shared<typed_dfcolumn<raw_string>>
        (std::move(ws), std::move(nulls));
      return ret;
    } else {
      auto ret = std::make_shared<typed_dfcolumn<dic_string>>
        (std::move(ws), std::move(nulls));
      return ret;
    }
  }
}

std::shared_ptr<dfcolumn>
dfcolumn::substr(const std::shared_ptr<dfcolumn>& pos,
                 const std::shared_ptr<dfcolumn>& num) {
  auto intcol_pos = pos->type_cast("int");
  auto tintcol_pos = dynamic_pointer_cast<typed_dfcolumn<int>>(intcol_pos);
  auto intcol_num = num->type_cast("int");
  auto tintcol_num = dynamic_pointer_cast<typed_dfcolumn<int>>(intcol_num);
  if(!static_cast<bool>(tintcol_pos) || !static_cast<bool>(tintcol_num))
    throw std::runtime_error("internal cast error");
  auto ws = as_words();
  ws.mapv(+[](words& ws){ws.utf8_to_utf32();});
  auto nulls = get_nulls();
  if(tintcol_pos->if_contain_nulls() || tintcol_num->if_contain_nulls() ||
     if_contain_nulls()) {
    // need to copy because they will be changed
    auto posval = tintcol_pos->val;
    auto numval = tintcol_num->val;
    posval.mapv
      (+[](std::vector<int>& val1,
           std::vector<int>& val2,
           std::vector<size_t>& nulls1,
           std::vector<size_t>& nulls2,
           std::vector<size_t>& nulls3) {
        auto val1p = val1.data();
        auto val2p = val2.data();
        auto nulls1p = nulls1.data();
        auto nulls1_size = nulls1.size();
        auto nulls2p = nulls2.data();
        auto nulls2_size = nulls2.size();
        auto nulls3p = nulls3.data();
        auto nulls3_size = nulls3.size();
#pragma _NEC vob
#pragma _NEC vovertake
#pragma _NEC ivdep
        for(size_t i = 0; i < nulls1_size; i++) {
          val1p[nulls1p[i]] = 0;
          val2p[nulls1p[i]] = 0;
        }
#pragma _NEC vob
#pragma _NEC vovertake
#pragma _NEC ivdep
        for(size_t i = 0; i < nulls2_size; i++) {
          val1p[nulls2p[i]] = 0;
          val2p[nulls2p[i]] = 0;
        }
#pragma _NEC vob
#pragma _NEC vovertake
#pragma _NEC ivdep
        for(size_t i = 0; i < nulls3_size; i++) {
          val1p[nulls3p[i]] = 0;
          val2p[nulls3p[i]] = 0;
        }
      }, numval, tintcol_pos->nulls, tintcol_num->nulls, nulls);
    ws.mapv(+[](words& ws, std::vector<int>& pos, std::vector<int>& num){
        auto posp = pos.data();
        auto pos_size = pos.size();
        for(size_t i = 0; i < pos_size; i++) {if(posp[i] > 0) posp[i]--;}
        ws.substr(pos, num);
      }, posval, numval);
    nulls.mapv(+[](std::vector<size_t>& nulls,
                   std::vector<size_t>& colnulls) {
                 nulls = set_union(nulls, colnulls);
               }, tintcol_pos->nulls);
    nulls.mapv(+[](std::vector<size_t>& nulls,
                   std::vector<size_t>& colnulls) {
                 nulls = set_union(nulls, colnulls);
               }, tintcol_num->nulls);
  } else {
    ws.mapv(+[](words& ws, std::vector<int>& pos, std::vector<int>& num){
        auto posp = pos.data();
        auto pos_size = pos.size();
        for(size_t i = 0; i < pos_size; i++) {if(posp[i] > 0) posp[i]--;}
        ws.substr(pos, num);
      }, tintcol_pos->val, tintcol_num->val);
  }
  ws.mapv(+[](words& ws){ws.utf32_to_utf8();});
  if(dtype() == "string") {
    auto vs = ws.map(+[](words& ws){return words_to_vector_string(ws);});
    auto ret = std::make_shared<typed_dfcolumn<std::string>>
      (std::move(vs), std::move(nulls));
    return ret;
  } else if (dtype() == "raw_string") {
    auto ret = std::make_shared<typed_dfcolumn<raw_string>>
      (std::move(ws), std::move(nulls));
    return ret;
  } else {
    auto ret = std::make_shared<typed_dfcolumn<dic_string>>
      (std::move(ws), std::move(nulls));
    return ret;
  }
}

std::shared_ptr<dfcolumn>
dfcolumn::substr(const std::shared_ptr<dfcolumn>& pos) {
  auto intcol = pos->type_cast("int");
  auto tintcol = dynamic_pointer_cast<typed_dfcolumn<int>>(intcol);
  if(!static_cast<bool>(tintcol))
    throw std::runtime_error("internal cast error");
  auto ws = as_words();
  ws.mapv(+[](words& ws){ws.utf8_to_utf32();});
  auto nulls = get_nulls();
  if(tintcol->if_contain_nulls() || if_contain_nulls()) {
    auto posval = tintcol->val; // copy
    posval.mapv
      (+[](std::vector<int>& val,
           std::vector<size_t>& nulls1,
           std::vector<size_t>& nulls2) {
        auto valp = val.data();
        auto nulls1p = nulls1.data();
        auto nulls1_size = nulls1.size();
        auto nulls2p = nulls2.data();
        auto nulls2_size = nulls2.size();
#pragma _NEC vob
#pragma _NEC vovertake
#pragma _NEC ivdep
        for(size_t i = 0; i < nulls1_size; i++) valp[nulls1p[i]] = 0;
        for(size_t i = 0; i < nulls2_size; i++) valp[nulls2p[i]] = 0;
      }, tintcol->nulls, nulls);
    ws.mapv(+[](words& ws, std::vector<int>& pos){
        auto posp = pos.data();
        auto pos_size = pos.size();
        for(size_t i = 0; i < pos_size; i++) {if(posp[i] > 0) posp[i]--;}
        ws.substr(pos);
      }, posval);
  } else {
    ws.mapv(+[](words& ws, std::vector<int>& pos) {
        auto posp = pos.data();
        auto pos_size = pos.size();
        for(size_t i = 0; i < pos_size; i++) {if(posp[i] > 0) posp[i]--;}
        ws.substr(pos);
      }, tintcol->val);
  }
  if(tintcol->if_contain_nulls()) {
    nulls.mapv(+[](std::vector<size_t>& nulls,
                   std::vector<size_t>& colnulls) {
                 nulls = set_union(nulls, colnulls);
               }, tintcol->nulls);
  }
  ws.mapv(+[](words& ws){ws.utf32_to_utf8();});
  if(dtype() == "string") {
    auto vs = ws.map(+[](words& ws){return words_to_vector_string(ws);});
    auto ret = std::make_shared<typed_dfcolumn<std::string>>
      (std::move(vs), std::move(nulls));
    return ret;
  } else if (dtype() == "raw_string") {
    auto ret = std::make_shared<typed_dfcolumn<raw_string>>
      (std::move(ws), std::move(nulls));
    return ret;
  } else {
    auto ret = std::make_shared<typed_dfcolumn<dic_string>>
      (std::move(ws), std::move(nulls));
    return ret;
  }
}

std::shared_ptr<dfcolumn> dfcolumn::substr(int pos) {
  if(pos > 0) pos--; // convert 1-based to 0-based
  auto ws = as_words(6,"%Y-%m-%d",false,std::string(std::abs(pos), 'N'));
  ws.mapv(+[](words& ws){ws.utf8_to_utf32();});
  ws.mapv(+[](words& ws, int pos){ws.substr(pos);}, broadcast(pos));
  ws.mapv(+[](words& ws){ws.utf32_to_utf8();});
  auto nulls = get_nulls();
  if(dtype() == "string") {
    // TODO: in the case of string, not using words would be faster
    auto vs = ws.map(+[](words& ws){return words_to_vector_string(ws);});
    auto ret = std::make_shared<typed_dfcolumn<std::string>>
      (std::move(vs), std::move(nulls));
    return ret;
  } else if (dtype() == "raw_string") {
    auto ret = std::make_shared<typed_dfcolumn<raw_string>>
      (std::move(ws), std::move(nulls));
    return ret;
  } else {
    auto ret = std::make_shared<typed_dfcolumn<dic_string>>
      (std::move(ws), std::move(nulls));
    return ret;
  }
}

std::shared_ptr<dfcolumn> dfcolumn::initcap() {
  auto ws = as_words();
  ws.mapv(+[](words& ws) { ws.initcap(); });
  auto nulls = get_nulls();
  if(dtype() == "string") {
    auto vs = ws.map(+[](words& ws){return words_to_vector_string(ws);});
    auto ret = std::make_shared<typed_dfcolumn<std::string>>
      (std::move(vs), std::move(nulls));
    return ret;
  } else if (dtype() == "raw_string") {
    auto ret = std::make_shared<typed_dfcolumn<raw_string>>
      (std::move(ws), std::move(nulls));
    return ret;
  } else {
    auto ret = std::make_shared<typed_dfcolumn<dic_string>>
      (std::move(ws), std::move(nulls));
    return ret;
  }
}

// returns hamming distance of two columns
std::shared_ptr<dfcolumn> 
dfcolumn::hamming(const std::shared_ptr<dfcolumn>& right) {
  auto ws1 = as_words();
  auto ws2 = right->as_words();
  auto nulls1 = get_nulls();
  auto nulls2 = right->get_nulls();
  auto nulls = make_node_local_allocate<std::vector<size_t>>();
  auto len = ws1.map(+[](words& ws1, std::vector<size_t>& nulls1,
                         words& ws2, std::vector<size_t>& nulls2,
                         std::vector<size_t>& nulls) {
      bool all_valid;
      auto ret = ws1.hamming_distance(ws2, all_valid);
      nulls = set_union(nulls1, nulls2);
      auto retp = ret.data();
      auto nullsp = nulls.data();
      auto nulls_size = nulls.size();
      auto max = std::numeric_limits<int>::max();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
      for(size_t i = 0; i < nulls_size; i++) {
        retp[nullsp[i]] = max;
      }
      if (!all_valid) nulls = set_union(nulls, vector_find_tmax<int>(ret));
      return ret;}, nulls1, ws2, nulls2, nulls);
  return std::make_shared<typed_dfcolumn<int>>
    (std::move(len), std::move(nulls));
}

// returns ascii of the first character in each word
std::shared_ptr<dfcolumn> dfcolumn::ascii() {
  auto ws = as_words();
  auto nulls = get_nulls();
  auto len = ws.map(+[](words& ws, std::vector<size_t>& nulls){
      auto ret = ws.ascii();
      auto retp = ret.data();
      auto nullsp = nulls.data();
      auto nulls_size = nulls.size();
      auto max = std::numeric_limits<int>::max();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
      for(size_t i = 0; i < nulls_size; i++) {
        retp[nullsp[i]] = max;
      }
      return ret;}, nulls);
  return std::make_shared<typed_dfcolumn<int>>
    (std::move(len), std::move(nulls));
}

// length returns length in bytes
std::shared_ptr<dfcolumn> dfcolumn::length() {
  auto ws = as_words();
  auto nulls = get_nulls();
  auto len = ws.map(+[](words& ws, std::vector<size_t>& nulls){
      auto size = ws.lens.size();
      std::vector<int> ret(size);
      auto retp = ret.data();
      auto lensp = ws.lens.data();
      for(size_t i = 0; i < size; i++) retp[i] = lensp[i];
      auto nullsp = nulls.data();
      auto nulls_size = nulls.size();
      auto max = std::numeric_limits<int>::max();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
      for(size_t i = 0; i < nulls_size; i++) {
        retp[nullsp[i]] = max;
      }
      return ret;}, nulls);
  return std::make_shared<typed_dfcolumn<int>>
    (std::move(len), std::move(nulls));
}

// char_length returns length in number of chars
std::shared_ptr<dfcolumn> dfcolumn::char_length() {
  auto ws = as_words();
  ws.mapv(+[](words& ws){ws.utf8_to_utf32();});
  auto nulls = get_nulls();
  auto len = ws.map(+[](words& ws, std::vector<size_t>& nulls){
      auto size = ws.lens.size();
      std::vector<int> ret(size);
      auto retp = ret.data();
      auto lensp = ws.lens.data();
      for(size_t i = 0; i < size; i++) retp[i] = lensp[i];
      auto nullsp = nulls.data();
      auto nulls_size = nulls.size();
      auto max = std::numeric_limits<int>::max();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
      for(size_t i = 0; i < nulls_size; i++) {
        retp[nullsp[i]] = max;
      }
      return ret;}, nulls);
  return std::make_shared<typed_dfcolumn<int>>
    (std::move(len), std::move(nulls));
}

std::shared_ptr<dfcolumn> dfcolumn::locate(const std::string& str, int start) {
  start--; // change to 0-base
  auto ws = as_words();
  ws.mapv(+[](words& ws){ws.utf8_to_utf32();});
  auto utf32str = utf8_to_utf32(str);
  auto nulls = get_nulls();
  auto len = ws.map
    (+[](words& ws, std::vector<int>& utf32str, int start,
         std::vector<size_t>& nulls){
      std::vector<size_t> idx, pos;
      auto size = ws.lens.size();
      if(start > 0) { // negative is treated as 0
        auto wsstartsp = ws.starts.data();
        auto wslensp = ws.lens.data();
        for(size_t i = 0; i < size; i++) {
          if(start < wslensp[i]) {
            wsstartsp[i] += start;
            wslensp[i] -= start;
          } else {
            wsstartsp[i] += wslensp[i];
            wslensp[i] = 0;
          }
        }
      }
      search(ws, utf32str, idx, pos);
      auto sep = set_separate(idx); // get first occurrence
      auto idxp = idx.data();
      auto posp = pos.data();
      auto sep_size = sep.size();
      auto sepp = sep.data();
      std::vector<size_t> idx2(sep_size-1), pos2(sep_size-1);
      auto idx2p = idx2.data();
      auto pos2p = pos2.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
      for(size_t i = 0; i < sep_size - 1; i++) {
        idx2p[i] = idxp[sepp[i]];
        pos2p[i] = posp[sepp[i]];
      }
      std::vector<int> ret(size); // initialized by 0 (== not found)
      auto retp = ret.data();
      if(start > 0) { 
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
        for(size_t i = 0; i < sep_size - 1; i++) {
          retp[idx2p[i]] = pos2p[i] + 1 + start; // 1-base
        }
      } else {
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
        for(size_t i = 0; i < sep_size - 1; i++) {
          retp[idx2p[i]] = pos2p[i] + 1; // 1-base
        }
      }
      auto nullsp = nulls.data();
      auto nulls_size = nulls.size();
      auto max = std::numeric_limits<int>::max();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
      for(size_t i = 0; i < nulls_size; i++) {
        retp[nullsp[i]] = max;
      }
      return ret;}, broadcast(utf32str), broadcast(start), nulls);
  return std::make_shared<typed_dfcolumn<int>>
    (std::move(len), std::move(nulls));
}

std::shared_ptr<dfcolumn>
dfcolumn::trim(trim_type kind, const std::string& to_trim) {
  auto ws = as_words();
  if(kind == trim_type::leading) {
    ws.mapv(+[](words& ws, const std::string& to_trim){ws.trim_head(to_trim);},
            broadcast(to_trim));
  } else if(kind == trim_type::trailing) {
    ws.mapv(+[](words& ws, const std::string& to_trim){ws.trim_tail(to_trim);},
            broadcast(to_trim));
  } else { // both
    ws.mapv(+[](words& ws, const std::string& to_trim){ws.trim(to_trim);},
            broadcast(to_trim));
  }
  auto nulls = get_nulls();
  if(dtype() == "string") {
    auto vs = ws.map(+[](words& ws){return words_to_vector_string(ws);});
    auto ret = std::make_shared<typed_dfcolumn<std::string>>
      (std::move(vs), std::move(nulls));
    return ret;
  } else if (dtype() == "raw_string") {
    auto ret = std::make_shared<typed_dfcolumn<raw_string>>
      (std::move(ws), std::move(nulls));
    return ret;
  } else {
    auto ret = std::make_shared<typed_dfcolumn<dic_string>>
      (std::move(ws), std::move(nulls));
    return ret;
  }
}

std::shared_ptr<dfcolumn>
dfcolumn::translate(const std::string& from, const std::string& to) {
  auto ws = as_words();
  ws.mapv(+[](words& ws){ws.utf8_to_utf32();});
  ws.mapv(+[](words& ws, const std::string& from, const std::string& to)
          {ws.translate(from, to);}, broadcast(from), broadcast(to));
  ws.mapv(+[](words& ws){ws.utf32_to_utf8();});
  auto nulls = get_nulls();
  if(dtype() == "string") {
    auto vs = ws.map(+[](words& ws){return words_to_vector_string(ws);});
    auto ret = std::make_shared<typed_dfcolumn<std::string>>
      (std::move(vs), std::move(nulls));
    return ret;
  } else if (dtype() == "raw_string") {
    auto ret = std::make_shared<typed_dfcolumn<raw_string>>
      (std::move(ws), std::move(nulls));
    return ret;
  } else {
    auto ret = std::make_shared<typed_dfcolumn<dic_string>>
      (std::move(ws), std::move(nulls));
    return ret;
  }
}

std::shared_ptr<dfcolumn>
dfcolumn::replace(const std::string& from, const std::string& to) {
  auto ws = as_words();
  ws.mapv(+[](words& ws, const std::string& from, const std::string& to)
          {ws.replace(from, to);}, broadcast(from), broadcast(to));
  auto nulls = get_nulls();
  if(dtype() == "string") {
    auto vs = ws.map(+[](words& ws){return words_to_vector_string(ws);});
    auto ret = std::make_shared<typed_dfcolumn<std::string>>
      (std::move(vs), std::move(nulls));
    return ret;
  } else if (dtype() == "raw_string") {
    auto ret = std::make_shared<typed_dfcolumn<raw_string>>
      (std::move(ws), std::move(nulls));
    return ret;
  } else {
    auto ret = std::make_shared<typed_dfcolumn<dic_string>>
      (std::move(ws), std::move(nulls));
    return ret;
  }
}

std::shared_ptr<dfcolumn>
dfcolumn::reverse() {
  auto ws = as_words();
  ws.mapv(+[](words& ws){ws.utf8_to_utf32();});
  ws.mapv(+[](words& ws){ws.reverse();});
  ws.mapv(+[](words& ws){ws.utf32_to_utf8();});  
  auto nulls = get_nulls();
  if(dtype() == "string") {
    auto vs = ws.map(+[](words& ws){return words_to_vector_string(ws);});
    auto ret = std::make_shared<typed_dfcolumn<std::string>>
      (std::move(vs), std::move(nulls));
    return ret;
  } else if (dtype() == "raw_string") {
    auto ret = std::make_shared<typed_dfcolumn<raw_string>>
      (std::move(ws), std::move(nulls));
    return ret;
  } else {
    auto ret = std::make_shared<typed_dfcolumn<dic_string>>
      (std::move(ws), std::move(nulls));
    return ret;
  }
}

std::shared_ptr<dfcolumn>
dfcolumn::substring_index(const std::string& delim, int count) {
  auto ws = as_words();
  ws.mapv(+[](words& ws, const std::string& delim, int count) {
      auto lens_size = ws.lens.size();
      auto lensp = ws.lens.data();
      auto startsp = ws.starts.data();
      if(count == 0) {
        for(size_t i = 0; i < lens_size; i++) {
          lensp[i] = 0;
        }
        return;
      }
      std::vector<size_t> idx, pos;
      search(ws, delim, idx, pos);
      auto sep = set_separate(idx);
      auto idxp = idx.data();
      auto posp = pos.data();
      auto sep_size = sep.size();
      auto sepp = sep.data();
      std::vector<size_t> num_delim(sep_size-1);
      auto num_delimp = num_delim.data();
      for(size_t i = 0; i < sep_size - 1; i++) {
        num_delimp[i] = sepp[i+1] - sepp[i];
      }
      if(count > 0) {
        auto end_delim = vector_find_ge(num_delim, size_t(count));
        auto end_delimp = end_delim.data();
        auto end_delim_size = end_delim.size();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
        for(size_t i = 0; i < end_delim_size; i++) {
          auto search_pos = sepp[end_delimp[i]];
          auto words_idx = idxp[search_pos];
          lensp[words_idx] = posp[search_pos + count - 1];
        }
      } else {
        auto abscount = -count;
        auto start_delim = vector_find_ge(num_delim, size_t(abscount));
        auto start_delimp = start_delim.data();
        auto start_delim_size = start_delim.size();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
        for(size_t i = 0; i < start_delim_size; i++) {
          auto start_delimp_i = start_delimp[i];
          auto search_pos = sepp[start_delimp_i];
          auto words_idx = idxp[search_pos];
          auto shift =
            posp[search_pos + num_delimp[start_delimp_i] - abscount] + 1;
          startsp[words_idx] += shift;
          lensp[words_idx] -= shift;
        }
      }
    }, broadcast(delim), broadcast(count));
  auto nulls = get_nulls();
  if(dtype() == "string") {
    auto vs = ws.map(+[](words& ws){return words_to_vector_string(ws);});
    auto ret = std::make_shared<typed_dfcolumn<std::string>>
      (std::move(vs), std::move(nulls));
    return ret;
  } else if (dtype() == "raw_string") {
    auto ret = std::make_shared<typed_dfcolumn<raw_string>>
      (std::move(ws), std::move(nulls));
    return ret;
  } else {
    auto ret = std::make_shared<typed_dfcolumn<dic_string>>
      (std::move(ws), std::move(nulls));
    return ret;
  }
}

std::shared_ptr<dfcolumn>
dfcolumn::concat(const std::shared_ptr<dfcolumn>& right) {
  auto left_ws = as_words();
  auto right_ws = right->as_words();
  auto ws = left_ws.map
    (+[](words& left_ws, words& right_ws){
      std::vector<words> vws(2);
      vws[0] = std::move(left_ws);
      vws[1] = std::move(right_ws);
      return horizontal_concat_words(vws);
    }, right_ws);
  auto left_nulls = get_nulls();
  auto right_nulls = right->get_nulls();
  auto nulls = left_nulls.map
    (+[](std::vector<size_t>& left_nulls, std::vector<size_t>& right_nulls)
     {return set_union(left_nulls, right_nulls);}, right_nulls);
  if(dtype() == "string") {
    auto vs = ws.map(+[](words& ws){return words_to_vector_string(ws);});
    auto ret = std::make_shared<typed_dfcolumn<std::string>>
      (std::move(vs), std::move(nulls));
    return ret;
  } else if (dtype() == "raw_string") {
    auto ret = std::make_shared<typed_dfcolumn<raw_string>>
      (std::move(ws), std::move(nulls));
    return ret;
  } else {
    auto ret = std::make_shared<typed_dfcolumn<dic_string>>
      (std::move(ws), std::move(nulls));
    return ret;
  }
}

std::shared_ptr<dfcolumn>
dfcolumn::prepend_string(const std::string& str) {
  auto ws = as_words();
  ws.mapv(+[](words& ws, std::string& str){ws.prepend(str);},
          broadcast(str));
  auto nulls = get_nulls();
  if(dtype() == "string") {
    auto vs = ws.map(+[](words& ws){return words_to_vector_string(ws);});
    auto ret = std::make_shared<typed_dfcolumn<std::string>>
      (std::move(vs), std::move(nulls));
    return ret;
  } else if (dtype() == "raw_string") {
    auto ret = std::make_shared<typed_dfcolumn<raw_string>>
      (std::move(ws), std::move(nulls));
    return ret;
  } else {
    auto ret = std::make_shared<typed_dfcolumn<dic_string>>
      (std::move(ws), std::move(nulls));
    return ret;
  }
}

std::shared_ptr<dfcolumn>
dfcolumn::append_string(const std::string& str) {
  auto ws = as_words();
  ws.mapv(+[](words& ws, std::string& str){ws.append(str);},
          broadcast(str));
  auto nulls = get_nulls();
  if(dtype() == "string") {
    auto vs = ws.map(+[](words& ws){return words_to_vector_string(ws);});
    auto ret = std::make_shared<typed_dfcolumn<std::string>>
      (std::move(vs), std::move(nulls));
    return ret;
  } else if (dtype() == "raw_string") {
    auto ret = std::make_shared<typed_dfcolumn<raw_string>>
      (std::move(ws), std::move(nulls));
    return ret;
  } else {
    auto ret = std::make_shared<typed_dfcolumn<dic_string>>
      (std::move(ws), std::move(nulls));
    return ret;
  }
}

std::shared_ptr<dfcolumn>
dfcolumn::lower() {
  auto ws = as_words();
  ws.mapv(+[](words& ws){ws.tolower();});
  auto nulls = get_nulls();
  if(dtype() == "string") {
    auto vs = ws.map(+[](words& ws){return words_to_vector_string(ws);});
    auto ret = std::make_shared<typed_dfcolumn<std::string>>
      (std::move(vs), std::move(nulls));
    return ret;
  } else if (dtype() == "raw_string") {
    auto ret = std::make_shared<typed_dfcolumn<raw_string>>
      (std::move(ws), std::move(nulls));
    return ret;
  } else {
    auto ret = std::make_shared<typed_dfcolumn<dic_string>>
      (std::move(ws), std::move(nulls));
    return ret;
  }
}

std::shared_ptr<dfcolumn>
dfcolumn::upper() {
  auto ws = as_words();
  ws.mapv(+[](words& ws){ws.toupper();});
  auto nulls = get_nulls();
  if(dtype() == "string") {
    auto vs = ws.map(+[](words& ws){return words_to_vector_string(ws);});
    auto ret = std::make_shared<typed_dfcolumn<std::string>>
      (std::move(vs), std::move(nulls));
    return ret;
  } else if (dtype() == "raw_string") {
    auto ret = std::make_shared<typed_dfcolumn<raw_string>>
      (std::move(ws), std::move(nulls));
    return ret;
  } else {
    auto ret = std::make_shared<typed_dfcolumn<dic_string>>
      (std::move(ws), std::move(nulls));
    return ret;
  }
}

std::shared_ptr<dfcolumn>
dfcolumn::lpad(int len, const std::string& pad) {
  auto utf32pad = utf8_to_utf32(pad);
  std::vector<int> repeatpad(len);
  auto utf32pad_size = utf32pad.size();
  auto rep = len / utf32pad_size;
  size_t crnt = 0;
  for(size_t i = 0; i < rep; i++) {
    for(size_t j = 0; j < utf32pad_size; j++) {
      repeatpad[crnt++] = utf32pad[j];
    }
  }
  auto rest = len - utf32pad_size * rep;
  if(utf32pad_size == 0) {
    for(size_t i = 0; i < rest; i++) {
      repeatpad[crnt++] = ' ';
    }
  } else {
    for(size_t i = 0; i < rest; i++) {
      repeatpad[crnt++] = utf32pad[i];
    }
  }
  auto ws = as_words();
  auto padws = ws.map(+[](words& ws, int len, std::vector<int>& pad) {
      ws.utf8_to_utf32();
      auto lens_size = ws.lens.size();
      auto lensp = ws.lens.data();
      std::vector<words> catv(2);
      catv[1] = std::move(ws);
      catv[0].lens.resize(lens_size);
      catv[0].starts.resize(lens_size);
      catv[0].chars = pad;
      auto padlensp = catv[0].lens.data();
      for(size_t i = 0; i < lens_size; i++) {
        padlensp[i] = lensp[i] < len ? len - lensp[i] : 0;
      }
      auto padstartsp = catv[0].starts.data();
      for(size_t i = 0; i < lens_size; i++) {
        padstartsp[i] = 0;
      }
      auto retws = horizontal_concat_words(catv);
      retws.utf32_to_utf8();
      return retws;
    }, broadcast(len), broadcast(repeatpad));
  auto nulls = get_nulls();
  if(dtype() == "string") {
    auto vs = padws.map(+[](words& ws){return words_to_vector_string(ws);});
    auto ret = std::make_shared<typed_dfcolumn<std::string>>
      (std::move(vs), std::move(nulls));
    return ret;
  } else if (dtype() == "raw_string") {
    auto ret = std::make_shared<typed_dfcolumn<raw_string>>
      (std::move(padws), std::move(nulls));
    return ret;
  } else {
    auto ret = std::make_shared<typed_dfcolumn<dic_string>>
      (std::move(padws), std::move(nulls));
    return ret;
  }
}

std::shared_ptr<dfcolumn>
dfcolumn::rpad(int len, const std::string& pad) {
  auto utf32pad = utf8_to_utf32(pad);
  std::vector<int> repeatpad(len);
  auto utf32pad_size = utf32pad.size();
  auto rep = len / utf32pad_size;
  size_t crnt = 0;
  for(size_t i = 0; i < rep; i++) {
    for(size_t j = 0; j < utf32pad_size; j++) {
      repeatpad[crnt++] = utf32pad[j];
    }
  }
  auto rest = len - utf32pad_size * rep;
  if(utf32pad_size == 0) {
    for(size_t i = 0; i < rest; i++) {
      repeatpad[crnt++] = ' ';
    }
  } else {
    for(size_t i = 0; i < rest; i++) {
      repeatpad[crnt++] = utf32pad[i];
    }
  }
  auto ws = as_words();
  auto padws = ws.map(+[](words& ws, int len, std::vector<int>& pad) {
      ws.utf8_to_utf32();
      auto lens_size = ws.lens.size();
      auto lensp = ws.lens.data();
      std::vector<words> catv(2);
      catv[0] = std::move(ws);
      catv[1].lens.resize(lens_size);
      catv[1].starts.resize(lens_size);
      catv[1].chars = pad;
      auto padlensp = catv[1].lens.data();
      for(size_t i = 0; i < lens_size; i++) {
        padlensp[i] = lensp[i] < len ? len - lensp[i] : 0;
      }
      auto padstartsp = catv[1].starts.data();
      for(size_t i = 0; i < lens_size; i++) {
        padstartsp[i] = 0;
      }
      auto retws = horizontal_concat_words(catv);
      retws.utf32_to_utf8();
      return retws;
    }, broadcast(len), broadcast(repeatpad));
  auto nulls = get_nulls();
  if(dtype() == "string") {
    auto vs = padws.map(+[](words& ws){return words_to_vector_string(ws);});
    auto ret = std::make_shared<typed_dfcolumn<std::string>>
      (std::move(vs), std::move(nulls));
    return ret;
  } else if (dtype() == "raw_string") {
    auto ret = std::make_shared<typed_dfcolumn<raw_string>>
      (std::move(padws), std::move(nulls));
    return ret;
  } else {
    auto ret = std::make_shared<typed_dfcolumn<dic_string>>
      (std::move(padws), std::move(nulls));
    return ret;
  }
}

template <>
dvector<std::string> dfcolumn::as_dvector() {
  dvector<std::string> ret;
  if (dtype() == "string") {
    auto& typed_col = dynamic_cast<typed_dfcolumn<std::string>&>(*this);
    ret = typed_col.get_val().as_dvector<std::string>();
  }
  else if (dtype() == "dic_string") {
    auto& typed_col = dynamic_cast<typed_dfcolumn<dic_string>&>(*this);
    ret = typed_col.as_words().map(words_to_vector_string)
      .as_dvector<std::string>();
  }
  else if (dtype() == "raw_string") {
    auto& typed_col = dynamic_cast<typed_dfcolumn<raw_string>&>(*this);
    ret = typed_col.as_words().map(words_to_vector_string)
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

std::vector<int>
get_bool_mask_helper(std::vector<size_t>& local_idx, size_t size) {
  std::vector<int> res(size, 0);
  auto resp = res.data();
  auto idxp = local_idx.data();
  auto local_idx_size = local_idx.size();
  #pragma _NEC ivdep
  #pragma _NEC vovertake
  #pragma _NEC vob
  for (size_t i = 0; i < local_idx_size; i++) resp[idxp[i]] = 1;
  return res;
}

// copied from dfcolumn_impl.hpp, to avoid including it
template <class T>
void reset_null(std::vector<T>& val, 
                const std::vector<size_t>& nulls) {
  auto valp = val.data();
  auto nullsp = nulls.data();
  auto size = nulls.size();
  T tmax = std::numeric_limits<T>::max();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < size; i++) valp[nullsp[i]] = tmax;
} 

std::shared_ptr<dfcolumn>
create_boolean_column(node_local<std::vector<size_t>>& filter_idx,
                      node_local<std::vector<size_t>>& ret_nulls,
                      const std::vector<size_t>& sizes) {
  auto ret_val = filter_idx.map(get_bool_mask_helper,
                                make_node_local_scatter(sizes));
  auto has_null = ret_nulls.viewas_dvector<size_t>().size() > 0;
  if (has_null) {
    ret_val.mapv(reset_null<int>, ret_nulls);
    return std::make_shared<typed_dfcolumn<int>>(std::move(ret_val),
                                                 std::move(ret_nulls));
  } else {
    auto dvval = ret_val.template moveto_dvector<int>();
    return std::make_shared<typed_dfcolumn<int>>(std::move(dvval));
  }
}

std::shared_ptr<dfcolumn>
dfcolumn::eq(const std::shared_ptr<dfcolumn>& right) {
  auto nrows = sizes();
  checkAssumption(nrows == right->sizes());
  auto& right_ = const_cast<std::shared_ptr<dfcolumn>&>(right);
  auto filter_idx = filter_eq(right_);
  auto ret_nulls = get_nulls().map(set_union<size_t>, right->get_nulls());
  return create_boolean_column(filter_idx, ret_nulls, nrows);
}

std::shared_ptr<dfcolumn>
dfcolumn::eq_im(const std::shared_ptr<frovedis::dfscalar>& right) {
  auto& right_ = const_cast<std::shared_ptr<dfscalar>&>(right);
  auto filter_idx = filter_eq_immed(right_);
  auto left_nulls  = get_nulls();
  return create_boolean_column(filter_idx, left_nulls, sizes());
}

std::shared_ptr<dfcolumn>
dfcolumn::neq(const std::shared_ptr<dfcolumn>& right) {
  auto nrows = sizes();
  checkAssumption(nrows == right->sizes());
  auto& right_ = const_cast<std::shared_ptr<dfcolumn>&>(right);
  auto filter_idx = filter_neq(right_);
  auto ret_nulls = get_nulls().map(set_union<size_t>, right->get_nulls());
  return create_boolean_column(filter_idx, ret_nulls, nrows);
}

std::shared_ptr<dfcolumn>
dfcolumn::neq_im(const std::shared_ptr<frovedis::dfscalar>& right) {
  auto& right_ = const_cast<std::shared_ptr<dfscalar>&>(right);
  auto filter_idx = filter_neq_immed(right_);
  auto left_nulls  = get_nulls();
  return create_boolean_column(filter_idx, left_nulls, sizes());
}

std::shared_ptr<dfcolumn>
dfcolumn::gt(const std::shared_ptr<dfcolumn>& right) {
  auto nrows = sizes();
  checkAssumption(nrows == right->sizes());
  auto& right_ = const_cast<std::shared_ptr<dfcolumn>&>(right);
  auto filter_idx = filter_gt(right_);
  auto ret_nulls = get_nulls().map(set_union<size_t>, right->get_nulls());
  return create_boolean_column(filter_idx, ret_nulls, nrows);
}

std::shared_ptr<dfcolumn>
dfcolumn::gt_im(const std::shared_ptr<frovedis::dfscalar>& right) {
  auto& right_ = const_cast<std::shared_ptr<dfscalar>&>(right);
  auto filter_idx = filter_gt_immed(right_);
  auto left_nulls  = get_nulls();
  return create_boolean_column(filter_idx, left_nulls, sizes());
}

std::shared_ptr<dfcolumn>
dfcolumn::ge(const std::shared_ptr<dfcolumn>& right) {
  auto nrows = sizes();
  checkAssumption(nrows == right->sizes());
  auto& right_ = const_cast<std::shared_ptr<dfcolumn>&>(right);
  auto filter_idx = filter_ge(right_);
  auto ret_nulls = get_nulls().map(set_union<size_t>, right->get_nulls());
  return create_boolean_column(filter_idx, ret_nulls, nrows);
}

std::shared_ptr<dfcolumn>
dfcolumn::ge_im(const std::shared_ptr<frovedis::dfscalar>& right) {
  auto& right_ = const_cast<std::shared_ptr<dfscalar>&>(right);
  auto filter_idx = filter_ge_immed(right_);
  auto left_nulls  = get_nulls();
  return create_boolean_column(filter_idx, left_nulls, sizes());
}

std::shared_ptr<dfcolumn>
dfcolumn::lt(const std::shared_ptr<dfcolumn>& right) {
  auto nrows = sizes();
  checkAssumption(nrows == right->sizes());
  auto& right_ = const_cast<std::shared_ptr<dfcolumn>&>(right);
  auto filter_idx = filter_lt(right_);
  auto ret_nulls = get_nulls().map(set_union<size_t>, right->get_nulls());
  return create_boolean_column(filter_idx, ret_nulls, nrows);
}

std::shared_ptr<dfcolumn>
dfcolumn::lt_im(const std::shared_ptr<frovedis::dfscalar>& right) {
  auto& right_ = const_cast<std::shared_ptr<dfscalar>&>(right);
  auto filter_idx = filter_lt_immed(right_);
  auto left_nulls  = get_nulls();
  return create_boolean_column(filter_idx, left_nulls, sizes());
}

std::shared_ptr<dfcolumn>
dfcolumn::le(const std::shared_ptr<dfcolumn>& right) {
  auto nrows = sizes();
  checkAssumption(nrows == right->sizes());
  auto& right_ = const_cast<std::shared_ptr<dfcolumn>&>(right);
  auto filter_idx = filter_le(right_);
  auto ret_nulls = get_nulls().map(set_union<size_t>, right->get_nulls());
  return create_boolean_column(filter_idx, ret_nulls, nrows);
}

std::shared_ptr<dfcolumn>
dfcolumn::le_im(const std::shared_ptr<frovedis::dfscalar>& right) {
  auto& right_ = const_cast<std::shared_ptr<dfscalar>&>(right);
  auto filter_idx = filter_le_immed(right_);
  auto left_nulls  = get_nulls();
  return create_boolean_column(filter_idx, left_nulls, sizes());
}

std::shared_ptr<dfcolumn>
dfcolumn::is_null() {
  auto ret_val = get_nulls().map(get_bool_mask_helper,
                                 make_node_local_scatter(sizes()));
  auto dvval = ret_val.template moveto_dvector<int>();
  return std::make_shared<typed_dfcolumn<int>>(std::move(dvval));
}

std::shared_ptr<dfcolumn>
dfcolumn::is_not_null() {
  auto ret_val = get_nulls().map(+[](const std::vector<size_t>& nulls, 
                                     size_t size) {
      std::vector<int> ret(size, 1);
      auto retp = ret.data();
      auto nullsp = nulls.data();
      auto nulls_size = nulls.size();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
      for(size_t i = 0; i < nulls_size; i++) retp[nullsp[i]] = 0;
      return ret;
    }, make_node_local_scatter(sizes()));
  auto dvval = ret_val.template moveto_dvector<int>();
  return std::make_shared<typed_dfcolumn<int>>(std::move(dvval));
}

}
