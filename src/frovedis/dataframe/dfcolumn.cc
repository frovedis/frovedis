#include "dfcolumn_impl.hpp"
#include <utility>
#include <regex>
#if !(defined(_SX) || defined(__ve__))
#include <unordered_set>
#endif

#define SEPARATE_TO_BUCKET_VLEN 256

#define UNIQ_IDX_MAX_BUCKET_SIZE 65536
#define UNIQ_IDX_TARGET_BUCKET_LEN 16384

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
    size_t exchanged_idx_i_size = exchanged_idx[i].size();
    int* hitp = &hit[0];
    size_t nullsize = 0;
    for(size_t j = 0; j < exchanged_idx_i_size; j++) {
      if(hitp[j]) nullsize++;
    }
    part_nulls[i].resize(nullsize);
    size_t* part_nulls_ip = &part_nulls[i][0];
    size_t* exchanged_idx_ip = &exchanged_idx[i][0];
    size_t current = 0;
    for(size_t j = 0; j < exchanged_idx_i_size; j++) {
      if(hitp[j]) {
        part_nulls_ip[current++] = exchanged_idx_ip[j];
      }
    }
    for(size_t j = 0; j < nullsize; j++) {
      part_nulls_ip[j] += nodeinfo;
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
    int* isnullp = &isnull[0];
    size_t size = isnull.size();
    vector<size_t> rettmp(size);
    size_t* rettmpp = &rettmp[0];
    size_t current = 0;
    for(size_t i = 0; i < size; i++) {
      if(isnullp[i] == 1) {
        rettmpp[current++] = i;
      }
    }
    vector<size_t> ret(current);
    size_t* retp = &ret[0];
    for(size_t i = 0; i < current; i++) {
      retp[i] = rettmpp[i];
    }
    return ret;
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
      int bucket = keyp[block_size * v + b];
      posp[block_size * v + b] = bucket_tablep[bucket_ldim * bucket + v];
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(int v = 0; v < SEPARATE_TO_BUCKET_VLEN; v++) { // vector loop, raking
      int bucket = keyp[block_size * v + b];
      bucket_tablep[bucket_ldim * bucket + v]++;
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

std::vector<size_t> count_helper(std::vector<size_t>& idx_split,
                                 std::vector<size_t>& nulls) {
  size_t splitsize = idx_split.size();
  size_t nullssize = nulls.size();
  std::vector<size_t> ret(splitsize-1);
  size_t* idx_splitp = &idx_split[0];
  size_t* retp = &ret[0];
  for(size_t i = 0; i < splitsize-1; i++) {
    retp[i] = idx_splitp[i+1] - idx_splitp[i];
  }
  // slow if # of nulls is large...
  for(size_t i = 0; i < nullssize; i++) {
    auto it = std::upper_bound(idx_split.begin(), idx_split.end(), nulls[i]);
    ret[it - idx_split.begin() - 1]--; 
  }
  return ret;
}

template<>
std::string get_type_name<int>(){return std::string("int");}

template<>
std::string get_type_name<unsigned int>(){return std::string("unsigned int");}

template<>
std::string get_type_name<long>(){return std::string("long");}

template<>
std::string get_type_name<unsigned long>(){
  return std::string("unsigned long");
}

template<>
std::string get_type_name<float>(){return std::string("float");}

template<>
std::string get_type_name<double>(){return std::string("double");}

}
