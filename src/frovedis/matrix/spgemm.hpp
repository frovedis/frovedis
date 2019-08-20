#ifndef SPGEMM_HPP
#define SPGEMM_HPP

#include "ccs_matrix.hpp"
#include "spmspv.hpp"
#include <limits>
#include <functional>

#ifdef __ve__
#define SPGEMM_VLEN 256
#else
#define SPGEMM_VLEN 1
#endif
#define SPGEMM_HASH_TABLE_SIZE_MULT 3
#define SPGEMM_HASH_SCALAR_THR 1024
#define SET_DIAG_ZERO_THR 32

namespace frovedis {

enum spgemm_type {
  esc,
  block_esc,
  hash,
  hash_sort,
  block_esc_binary_input,
  spa,
  spa_sort,
};

template <class T>
size_t max_bits(T a) {
  if(a < 0) throw std::runtime_error("max_bits is only for positive value");
  size_t i = 0;
  size_t max = sizeof(T) * 8;
  for(; i < max; i++) {
    if(a == 0) break;
    else a >>= 1;
  }
  return i;
}

// avoid loop raking because number of _merged vec not so large...
// (it is possible, but there might be load balancing problem...)
template <class I, class O>
void add_column_idx(I* mulout_idx, O* merged_total_nnzp,
                    size_t merged_count, size_t max_idx_bits) {
  for(size_t i = 0; i < merged_count; i++) {
    for(size_t j = 0; j < merged_total_nnzp[i]; j++) {
      mulout_idx[j] += (i << max_idx_bits);
    }
    mulout_idx += merged_total_nnzp[i];
  }
}

template <class I, class O>
void extract_column_idx(size_t* idxtmp, int* extracted, I* idx,
                        O size, size_t bits) {
  size_t mask = (static_cast<size_t>(1) << bits) - 1;
  for(size_t i = 0; i < size; i++) {
    extracted[i] = idxtmp[i] >> bits;
    idx[i] = idxtmp[i] & mask;
  }
}

/* modified version of set_separate at dataframe/set_operations.hpp
   copied to avoid dependency... */
template <class T>
std::vector<size_t> my_set_separate(const std::vector<T>& key) {
  size_t size = key.size();
  if(size == 0) {return std::vector<size_t>(1);} 
  int valid[SPGEMM_VLEN];
  for(int i = 0; i < SPGEMM_VLEN; i++) valid[i] = true;
  size_t each = ceil_div(size, size_t(SPGEMM_VLEN));
  if(each % 2 == 0) each++;
  size_t key_idx[SPGEMM_VLEN];
  size_t key_idx_stop[SPGEMM_VLEN];
  size_t out_idx[SPGEMM_VLEN];
  size_t out_idx_save[SPGEMM_VLEN];
  T current_key[SPGEMM_VLEN];
  std::vector<size_t> out;
  out.resize(size);
  size_t* outp = &out[0];
  const T* keyp = &key[0];
  if(size > 0) {
    key_idx[0] = 1;
    outp[0] = 0;
    out_idx[0] = 1;
    out_idx_save[0] = 0;
    current_key[0] = keyp[0];
  } else {
    valid[0] = false;
    key_idx[0] = size;
    out_idx[0] = size;
    out_idx_save[0] = size;
  }
  for(int i = 1; i < SPGEMM_VLEN; i++) {
    size_t pos = each * i;
    if(pos < size) {
      key_idx[i] = pos;
      out_idx[i] = pos;
      out_idx_save[i] = pos;
      current_key[i] = keyp[pos-1];
    } else {
      valid[i] = false;
      key_idx[i] = size;
      out_idx[i] = size;
      out_idx_save[i] = size;
    }
  }
  for(int i = 0; i < SPGEMM_VLEN - 1; i++) {
    key_idx_stop[i] = key_idx[i + 1];
  }
  key_idx_stop[SPGEMM_VLEN-1] = size;
  // idx 0 is manually advanced; need to be checked
  if(key_idx[0] == key_idx_stop[0]) valid[0] = false;
  size_t max_size = 0;
  for(int i = 0; i < SPGEMM_VLEN; i++) {
    auto current = key_idx_stop[i] - key_idx[i];
    if(max_size < current) max_size = current;
  }
  int valid_vreg[SPGEMM_VLEN];
  size_t key_idx_vreg[SPGEMM_VLEN];
  size_t key_idx_stop_vreg[SPGEMM_VLEN];
  size_t out_idx_vreg[SPGEMM_VLEN];
  T current_key_vreg[SPGEMM_VLEN];
#pragma _NEC vreg(valid_vreg)
#pragma _NEC vreg(key_idx_vreg)
#pragma _NEC vreg(key_idx_stop_vreg)
#pragma _NEC vreg(out_idx_vreg)
#pragma _NEC vreg(current_key_vreg)
  for(int i = 0; i < SPGEMM_VLEN; i++) {
    valid_vreg[i] = valid[i];
    key_idx_vreg[i] = key_idx[i];
    key_idx_stop_vreg[i] = key_idx_stop[i];
    out_idx_vreg[i] = out_idx[i];
    current_key_vreg[i] = current_key[i];
  }
  
  for(size_t j = 0; j < max_size; j++) {
#pragma cdir nodep
#pragma _NEC ivdep
    for(int i = 0; i < SPGEMM_VLEN; i++) {
      if(valid_vreg[i]) {
        auto keyval = keyp[key_idx_vreg[i]];
        if(keyval != current_key_vreg[i]) {
          outp[out_idx_vreg[i]++] = key_idx_vreg[i];
          current_key_vreg[i] = keyval;
        }
        key_idx_vreg[i]++;
        if(key_idx_vreg[i] == key_idx_stop_vreg[i]) {valid_vreg[i] = false;}
      }
    }
  }
  for(int i = 0; i < SPGEMM_VLEN; i++) {
    // valid[i] = valid_vreg[i]; // not used
    // key_idx[i] = key_idx_vreg[i]; // not used
    // key_idx_stop[i] = key_idx_stop_vreg[i]; // not used
    out_idx[i] = out_idx_vreg[i];
    // current_key[i] = current_key_vreg[i]; // not used
  }
  size_t total = 0;
  for(size_t i = 0; i < SPGEMM_VLEN; i++) {
    total += out_idx[i] - out_idx_save[i];
  }
  std::vector<size_t> ret(total+1);
  size_t* retp = &ret[0];
  size_t current = 0;
  for(size_t i = 0; i < SPGEMM_VLEN; i++) {
    for(size_t j = 0; j < out_idx[i] - out_idx_save[i]; j++) {
      retp[current + j] = out[out_idx_save[i] + j];
    }
    current += out_idx[i] - out_idx_save[i];
  }
  retp[current] = size;
  return ret;
}

template <class T, class I, class O>
void spgemm_esc_helper(T* mat_valp, I* mat_idxp, O* mat_offp,
                       T* sv_valp, I* sv_idxp, size_t sv_size,
                       O total_nnz, O* nnzp, O* pfx_sum_nnz,
                       O* merged_total_nnzp, size_t merged_count,
                       size_t max_idx_bits, size_t lnum_row,
                       std::vector<T>& retval, std::vector<I>& retidx,
                       std::vector<O>& retoff) {
  time_spent t(DEBUG);
  if(sv_size == 0) return;
  std::vector<T> mulout_val(total_nnz);
  std::vector<size_t> mulout_idx(total_nnz); // to hold enough bits
  auto mulout_valp = mulout_val.data();
  auto mulout_idxp = mulout_idx.data();
  spmspv_mul(mat_valp, mat_idxp, mat_offp, sv_valp, sv_idxp, sv_size,
             nnzp, pfx_sum_nnz, total_nnz,
             mulout_valp, mulout_idxp);
  t.show("spmspv_mul: ");
  add_column_idx(mulout_idxp, merged_total_nnzp, 
                 merged_count, max_idx_bits);
  t.show("add_column_idx: ");
  auto used_bits = max_bits(merged_count-1);
  auto max_key_size = ceil_div(max_idx_bits + used_bits, size_t(8));
  radix_sort_impl(mulout_idxp, mulout_valp, total_nnz, max_key_size);
  t.show("radix_sort: ");
  std::vector<size_t> retidxtmp;
  groupby_sum(mulout_idxp, mulout_valp, total_nnz, retidxtmp, retval);
  t.show("groupby_sum: ");
  std::vector<int> extracted_idx(retidxtmp.size());
  retidx.resize(retidxtmp.size());
  auto extracted_idxp = extracted_idx.data();
  extract_column_idx(retidxtmp.data(), extracted_idxp, retidx.data(),
                     retidxtmp.size(), max_idx_bits);
  t.show("extract_column_idx: ");
  auto separated = my_set_separate(extracted_idx);
  t.show("set_separate: ");
  // might be smaller than merged_count if sparse vector is zero
  auto separated_size = separated.size();
  retoff.resize(merged_count+1);
  std::vector<O> retofftmp(merged_count);
  auto retofftmpp = retofftmp.data();
  auto separatedp = separated.data();
  auto retoffp = retoff.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < separated_size - 1; i++) {
    retofftmpp[extracted_idxp[separatedp[i]]] =
      separatedp[i+1] - separatedp[i];
  }
  prefix_sum(retofftmpp, retoffp+1, merged_count);
  t.show("prefix_sum: ");
}

template <class T, class I, class O>
void spgemm_esc(T* lval, I* lidx, O* loff,
                size_t lnnz, size_t lnum_row, size_t lnum_col,
                T* rval, I* ridx, O* roff,
                size_t rnnz, size_t rnum_col,
                std::vector<T>& retval, std::vector<I>& retidx,
                std::vector<O>& retoff) {
  if(rnnz == 0) {
    retoff.resize(rnum_col+1);
    return;
  }
  size_t max_idx_bits = max_bits(lnum_row-1);

  time_spent t(DEBUG);
  std::vector<O> merged_interim_nnz(rnnz);
  auto merged_interim_nnzp = merged_interim_nnz.data();
  std::vector<O> each_column_nnz(rnum_col);
  auto each_column_nnzp =  each_column_nnz.data();
  for(size_t rc = 0; rc < rnum_col; rc++) { // rc: right column
    auto crnt_ridx = ridx + roff[rc];
    auto crnt_nnz = roff[rc+1] - roff[rc];
    for(size_t i = 0; i < crnt_nnz; i++) {
      merged_interim_nnzp[i] = loff[crnt_ridx[i]+1] - loff[crnt_ridx[i]];
    }
    for(size_t i = 0; i < crnt_nnz; i++) {
      each_column_nnzp[rc] += merged_interim_nnzp[i];
    }
    merged_interim_nnzp += crnt_nnz;
  }
  t.show("merged_interim_nnz & each_column_nnz: ");

  auto pfx_sum_merged_interim_nnz = prefix_sum(merged_interim_nnz);
  auto total_interim_nnz = pfx_sum_merged_interim_nnz[rnnz - 1];

  t.show(std::string("prefix sum, total_interim_nnz = ")
         + std::to_string(total_interim_nnz) + ", time = ");
  
  spgemm_esc_helper(lval, lidx, loff, rval, ridx, rnnz, total_interim_nnz,
                    merged_interim_nnz.data(),
                    pfx_sum_merged_interim_nnz.data(),
                    each_column_nnzp, rnum_col,
                    max_idx_bits, lnum_row, retval, retidx, retoff);
}

template <class T, class I, class O>
sparse_vector<T,I>
spgemm_block_esc_helper(T* mat_valp, I* mat_idxp, O* mat_offp,
                        T* sv_valp, I* sv_idxp, O sv_size,
                        O total_nnz, O* nnzp, O* pfx_sum_nnz,
                        O* merged_total_nnzp, size_t merged_count,
                        size_t max_idx_bits, O* merged_offp) {
  sparse_vector<T,I> ret;
  if(sv_size == 0) return ret;
  std::vector<T> mulout_val(total_nnz);
  std::vector<size_t> mulout_idx(total_nnz);
  auto mulout_valp = mulout_val.data();
  auto mulout_idxp = mulout_idx.data();
  spmspv_mul(mat_valp, mat_idxp, mat_offp, sv_valp, sv_idxp, sv_size,
             nnzp, pfx_sum_nnz, total_nnz,
             mulout_valp, mulout_idxp);
  add_column_idx(mulout_idxp, merged_total_nnzp,
                 merged_count, max_idx_bits);
  auto used_bits = max_bits(merged_count-1);
  auto max_key_size = ceil_div(max_idx_bits + used_bits, size_t(8));
  radix_sort_impl(mulout_idxp, mulout_valp, total_nnz, max_key_size);
  std::vector<size_t> retidxtmp;
  groupby_sum(mulout_idxp, mulout_valp, total_nnz, retidxtmp, ret.val);
  std::vector<int> extracted_idx(retidxtmp.size());
  ret.idx.resize(retidxtmp.size());
  auto extracted_idxp = extracted_idx.data();
  extract_column_idx(retidxtmp.data(), extracted_idxp, ret.idx.data(),
                     retidxtmp.size(), max_idx_bits);
  auto separated = my_set_separate(extracted_idx);
  // might be smaller than merged_count if sparse vector is zero
  auto separated_size = separated.size();
  auto separatedp = separated.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < separated_size - 1; i++) {
    merged_offp[extracted_idxp[separatedp[i]]] =
      separatedp[i+1] - separatedp[i];
  }
  return ret;
}


template <class T, class I, class O>
void create_sparse_matrix(std::vector<sparse_vector<T,I>>& sv,
                          std::vector<T>& retval,
                          std::vector<I>& retidx,
                          std::vector<O>& retoff,
                          std::vector<O>& merged_off) {
  
  size_t total_nnz = 0;
  auto svsize = sv.size();
  for(size_t i = 0; i < svsize; i++) total_nnz += sv[i].val.size();
  retval.resize(total_nnz);
  retidx.resize(total_nnz);
  retoff.resize(merged_off.size()+1);
  auto crnt_retvalp = retval.data();
  auto crnt_retidxp = retidx.data();
  for(size_t i = 0; i < svsize; i++) {
    auto crnt_size = sv[i].val.size();
    auto svvalp = sv[i].val.data();
    auto svidxp = sv[i].idx.data();
    for(size_t j = 0; j < crnt_size; j++) {
      crnt_retvalp[j] = svvalp[j];
      crnt_retidxp[j] = svidxp[j];
    }
    crnt_retvalp += crnt_size;
    crnt_retidxp += crnt_size;
  }
  prefix_sum(merged_off.data(), retoff.data()+1, merged_off.size());
}


/*
  expressed as ccs matrix, but can be applied to crs matrix
  (A * B  = C <-> B^T * A^T = C^T and  CRS = CCS^T)
  return value is std::vector, because the size is not known beforehand
*/
template <class T, class I, class O>
void spgemm_block_esc(T* lval, I* lidx, O* loff,
                      size_t lnnz, size_t lnum_row, size_t lnum_col,
                      T* rval, I* ridx, O* roff,
                      size_t rnnz, size_t rnum_col,
                      std::vector<T>& retval, std::vector<I>& retidx,
                      std::vector<O>& retoff,
                      size_t merge_column_size) {
  time_spent t(DEBUG);
  if(rnnz == 0) {
    retoff.resize(rnum_col+1);
    return;
  }
  size_t max_idx_bits = max_bits(lnum_row-1);

  std::vector<O> merged_interim_nnz(rnnz);
  auto merged_interim_nnzp = merged_interim_nnz.data();
  std::vector<O> each_column_nnz(rnum_col);
  auto each_column_nnzp =  each_column_nnz.data();
  for(size_t rc = 0; rc < rnum_col; rc++) { // rc: right column
    auto crnt_ridx = ridx + roff[rc];
    auto crnt_nnz = roff[rc+1] - roff[rc];
    for(size_t i = 0; i < crnt_nnz; i++) {
      merged_interim_nnzp[i] = loff[crnt_ridx[i]+1] - loff[crnt_ridx[i]];
    }
    for(size_t i = 0; i < crnt_nnz; i++) {
      each_column_nnzp[rc] += merged_interim_nnzp[i];
    }
    merged_interim_nnzp += crnt_nnz;
  }
  t.show("count nnz: ");
  auto num_chunks = ceil_div(rnum_col,merge_column_size);
  std::vector<sparse_vector<T,I>> out_sparse_vector;
  out_sparse_vector.reserve(num_chunks);
  
  std::vector<O> retofftmp(rnum_col);
  auto retofftmpp = retofftmp.data();
  size_t crnt_rc = 0;
  merged_interim_nnzp = merged_interim_nnz.data();
  while(true) {
    size_t merge_size = 0;
    if(crnt_rc + merge_column_size < rnum_col)
      merge_size = merge_column_size;
    else
      merge_size = rnum_col - crnt_rc;
    auto crnt_rval = rval + roff[crnt_rc];
    auto crnt_ridx = ridx + roff[crnt_rc];
    auto crnt_size = roff[crnt_rc + merge_size] - roff[crnt_rc];
    if(crnt_size == 0) { // (contiguous) zero sized columns
      out_sparse_vector.push_back(sparse_vector<T,I>());
      crnt_rc += merge_size;
      if(crnt_rc == rnum_col) break;
      merged_interim_nnzp += crnt_size;
      each_column_nnzp += merge_size;
      retofftmpp += merge_size;
    } else {
      std::vector<O> pfx_sum_merged_interim_nnz(crnt_size);
      auto pfx_sum_merged_interim_nnzp = pfx_sum_merged_interim_nnz.data();
      prefix_sum(merged_interim_nnzp, pfx_sum_merged_interim_nnzp, crnt_size);
      auto total_interim_nnz = pfx_sum_merged_interim_nnzp[crnt_size - 1];
      out_sparse_vector.push_back
        (spgemm_block_esc_helper(lval, lidx, loff,
                                 crnt_rval, crnt_ridx, crnt_size,
                                 total_interim_nnz, merged_interim_nnzp,
                                 pfx_sum_merged_interim_nnzp,
                                 each_column_nnzp, merge_size,
                                 max_idx_bits, retofftmpp));
      crnt_rc += merge_size;
      if(crnt_rc == rnum_col) break;
      merged_interim_nnzp += crnt_size;
      each_column_nnzp += merge_size;
      retofftmpp += merge_size;
    }
  }
  t.show("time for creating sparse vectors: ");
  create_sparse_matrix(out_sparse_vector, retval, retidx, retoff, retofftmp);
  t.show("time for creating sparse matrix: ");
}

template <class T, class I, class O>
sparse_vector<T,I>
spgemm_block_esc_binary_input_helper
(T* mat_valp, I* mat_idxp, O* mat_offp,
 T* sv_valp, I* sv_idxp, O sv_size,
 O total_nnz, O* nnzp, O* pfx_sum_nnz,
 O* merged_total_nnzp, size_t merged_count,
 size_t max_idx_bits, O* merged_offp) {
  sparse_vector<T,I> ret;
  if(sv_size == 0) return ret;
  std::vector<size_t> mulout_idx(total_nnz);
  auto mulout_idxp = mulout_idx.data();
  spmspv_mul_binary_input
    (mat_idxp, mat_offp, sv_idxp, sv_size,
     nnzp, pfx_sum_nnz, total_nnz, mulout_idxp);
  add_column_idx(mulout_idxp, merged_total_nnzp,
                 merged_count, max_idx_bits);
  auto used_bits = max_bits(merged_count-1);
  auto max_key_size = ceil_div(max_idx_bits + used_bits, size_t(8));
  radix_sort_impl(mulout_idxp, total_nnz, max_key_size);
  std::vector<size_t> retidxtmp;
  groupby_sum_binary_input(mulout_idxp, total_nnz, retidxtmp, ret.val);
  std::vector<int> extracted_idx(retidxtmp.size());
  ret.idx.resize(retidxtmp.size());
  auto extracted_idxp = extracted_idx.data();
  extract_column_idx(retidxtmp.data(), extracted_idxp, ret.idx.data(),
                     retidxtmp.size(), max_idx_bits);
  auto separated = my_set_separate(extracted_idx);
  // might be smaller than merged_count if sparse vector is zero
  auto separated_size = separated.size();
  auto separatedp = separated.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < separated_size - 1; i++) {
    merged_offp[extracted_idxp[separatedp[i]]] =
      separatedp[i+1] - separatedp[i];
  }
  return ret;
}

template <class T, class I, class O>
void spgemm_block_esc_binary_input
(T* lval, I* lidx, O* loff,
 size_t lnnz, size_t lnum_row, size_t lnum_col,
 T* rval, I* ridx, O* roff,
 size_t rnnz, size_t rnum_col,
 std::vector<T>& retval, std::vector<I>& retidx,
 std::vector<O>& retoff,
 size_t merge_column_size) {
  time_spent t(DEBUG);
  if(rnnz == 0) {
    retoff.resize(rnum_col+1);
    return;
  }
  size_t max_idx_bits = max_bits(lnum_row-1);

  std::vector<O> merged_interim_nnz(rnnz);
  auto merged_interim_nnzp = merged_interim_nnz.data();
  std::vector<O> each_column_nnz(rnum_col);
  auto each_column_nnzp =  each_column_nnz.data();
  for(size_t rc = 0; rc < rnum_col; rc++) { // rc: right column
    auto crnt_ridx = ridx + roff[rc];
    auto crnt_nnz = roff[rc+1] - roff[rc];
    for(size_t i = 0; i < crnt_nnz; i++) {
      merged_interim_nnzp[i] = loff[crnt_ridx[i]+1] - loff[crnt_ridx[i]];
    }
    for(size_t i = 0; i < crnt_nnz; i++) {
      each_column_nnzp[rc] += merged_interim_nnzp[i];
    }
    merged_interim_nnzp += crnt_nnz;
  }
  t.show("count nnz: ");
  auto num_chunks = ceil_div(rnum_col,merge_column_size);
  std::vector<sparse_vector<T,I>> out_sparse_vector;
  out_sparse_vector.reserve(num_chunks);
  
  std::vector<O> retofftmp(rnum_col);
  auto retofftmpp = retofftmp.data();
  size_t crnt_rc = 0;
  merged_interim_nnzp = merged_interim_nnz.data();
  while(true) {
    size_t merge_size = 0;
    if(crnt_rc + merge_column_size < rnum_col)
      merge_size = merge_column_size;
    else
      merge_size = rnum_col - crnt_rc;
    auto crnt_rval = rval + roff[crnt_rc];
    auto crnt_ridx = ridx + roff[crnt_rc];
    auto crnt_size = roff[crnt_rc + merge_size] - roff[crnt_rc];
    if(crnt_size == 0) { // (contiguous) zero sized columns
      out_sparse_vector.push_back(sparse_vector<T,I>());
      crnt_rc += merge_size;
      if(crnt_rc == rnum_col) break;
      merged_interim_nnzp += crnt_size;
      each_column_nnzp += merge_size;
      retofftmpp += merge_size;
    } else {
      std::vector<O> pfx_sum_merged_interim_nnz(crnt_size);
      auto pfx_sum_merged_interim_nnzp = pfx_sum_merged_interim_nnz.data();
      prefix_sum(merged_interim_nnzp, pfx_sum_merged_interim_nnzp, crnt_size);
      auto total_interim_nnz = pfx_sum_merged_interim_nnzp[crnt_size - 1];
      out_sparse_vector.push_back
        (spgemm_block_esc_binary_input_helper(lval, lidx, loff,
                                              crnt_rval, crnt_ridx, crnt_size,
                                              total_interim_nnz,
                                              merged_interim_nnzp,
                                              pfx_sum_merged_interim_nnzp,
                                              each_column_nnzp, merge_size,
                                              max_idx_bits, retofftmpp));
      crnt_rc += merge_size;
      if(crnt_rc == rnum_col) break;
      merged_interim_nnzp += crnt_size;
      each_column_nnzp += merge_size;
      retofftmpp += merge_size;
    }
  }
  t.show("time for creating sparse vectors: ");
  create_sparse_matrix(out_sparse_vector, retval, retidx, retoff, retofftmp);
  t.show("time for creating sparse matrix: ");
}

template <class T>
void spgemm_make_sparse_vector(const size_t* keyp, const T* valp, size_t size,
                               size_t unused, std::vector<size_t>& retidx,
                               std::vector<T>& retval) {
  if(size == 0) return;
  std::vector<size_t> idxtmp(size);
  std::vector<T> valtmp(size);
  size_t* idxtmpp = idxtmp.data();
  T* valtmpp = valtmp.data();
  size_t each = size / SPGEMM_VLEN; // maybe 0
  if(each % 2 == 0 && each > 1) each--;
  size_t rest = size - each * SPGEMM_VLEN;
  size_t out_ridx[SPGEMM_VLEN];
// never remove this vreg! this is needed folowing vovertake
// though this prevents ftrace...
#pragma _NEC vreg(out_ridx)
  for(size_t i = 0; i < SPGEMM_VLEN; i++) {
    out_ridx[i] = each * i;
  }
  if(each == 0) {
    size_t current = 0;
    for(size_t i = 0; i < size; i++) {
      if(keyp[i] != unused) {
        idxtmpp[current] = keyp[i];
        valtmpp[current] = valp[i];
        current++;
      }
    }
    retidx.resize(current);
    retval.resize(current);
    auto retidxp = retidx.data();
    auto retvalp = retval.data();
    for(size_t i = 0; i < current; i++) {
      retidxp[i] = idxtmpp[i];
      retvalp[i] = valtmpp[i];
    }
  } else {
#pragma _NEC vob
    for(size_t j = 0; j < each; j++) {
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
      for(size_t i = 0; i < SPGEMM_VLEN; i++) {
        auto loaded_key = keyp[j + each * i];
        if(loaded_key != unused) {
          idxtmpp[out_ridx[i]] = loaded_key;
          valtmpp[out_ridx[i]] = valp[j + each * i];
          out_ridx[i]++;
        }
      }
    }
    size_t rest_idx_start = each * SPGEMM_VLEN;
    size_t rest_idx = rest_idx_start;
    if(rest != 0) {
      for(size_t j = 0; j < rest; j++) {
        auto loaded_key = keyp[j + rest_idx_start]; 
        if(loaded_key != unused) {
          idxtmpp[rest_idx] = loaded_key;
          valtmpp[rest_idx] = valp[j + rest_idx_start]; 
          rest_idx++;
        }
      }
    }
    size_t sizes[SPGEMM_VLEN];
    for(size_t i = 0; i < SPGEMM_VLEN; i++) {
      sizes[i] = out_ridx[i] - each * i;
    }
    size_t total = 0;
    for(size_t i = 0; i < SPGEMM_VLEN; i++) {
      total += sizes[i];
    }
    size_t rest_size = rest_idx - each * SPGEMM_VLEN;
    total += rest_size;
    retval.resize(total);
    retidx.resize(total);
    auto retvalp = retval.data();
    auto retidxp = retidx.data();
    size_t current = 0;
    for(size_t i = 0; i < SPGEMM_VLEN; i++) {
      for(size_t j = 0; j < sizes[i]; j++) {
        retidxp[current + j] = idxtmpp[each * i + j];
        retvalp[current + j] = valtmpp[each * i + j];
      }
      current += sizes[i];
    }
    for(size_t j = 0; j < rest_size; j++) {
      retidxp[current + j] = idxtmpp[rest_idx_start + j];
      retvalp[current + j] = valtmpp[rest_idx_start + j];
    }
  }
}

template <class T, class O>
void spgemm_hash_accumulator_unroll(size_t valid_vlen,
                                    O unroll4, O* pos_ridx, size_t shift,
                                    size_t* work_keyp, T* work_valp, 
                                    size_t max_idx_bits, size_t unused,
                                    O* table_sizep, O* table_startp,
                                    size_t* table_keyp, T* table_valp,
                                    size_t* work_collision_keyp,
                                    T* work_collision_valp,
                                    O* collision_ridx) {
  O pos_ridx_vreg[SPGEMM_VLEN];
  O collision_ridx_vreg[SPGEMM_VLEN];
#pragma _NEC vreg(pos_ridx_vreg)
#pragma _NEC vreg(collision_ridx_vreg)
  for(size_t i = 0; i < SPGEMM_VLEN; i++) {
    pos_ridx_vreg[i] = pos_ridx[i];
    collision_ridx_vreg[i] = collision_ridx[i];
  }

  for(O b = 0; b < unroll4; b++) {
#pragma _NEC ivdep
#pragma _NEC shortloop
    for(size_t i = 0; i < valid_vlen; i++) {
      auto loaded_key0 = work_keyp[pos_ridx_vreg[i]];
      auto loaded_val0 = work_valp[pos_ridx_vreg[i]];
      auto loaded_key1 = work_keyp[pos_ridx_vreg[i]+1];
      auto loaded_val1 = work_valp[pos_ridx_vreg[i]+1];
      auto loaded_key2 = work_keyp[pos_ridx_vreg[i]+2];
      auto loaded_val2 = work_valp[pos_ridx_vreg[i]+2];
      auto loaded_key3 = work_keyp[pos_ridx_vreg[i]+3];
      auto loaded_val3 = work_valp[pos_ridx_vreg[i]+3];

      auto col0 = loaded_key0 >> max_idx_bits;
      auto col1 = loaded_key1 >> max_idx_bits;
      auto col2 = loaded_key2 >> max_idx_bits;
      auto col3 = loaded_key3 >> max_idx_bits;

      auto hashval0 =
        (loaded_key0 + shift) % table_sizep[col0] + table_startp[col0];
      auto hashval1 =
        (loaded_key1 + shift) % table_sizep[col1] + table_startp[col1];
      auto hashval2 =
        (loaded_key2 + shift) % table_sizep[col2] + table_startp[col2];
      auto hashval3 =
        (loaded_key3 + shift) % table_sizep[col3] + table_startp[col3];

      auto table_key0 = table_keyp[hashval0];
      if(table_key0 == loaded_key0) { // same key already stored
        table_valp[hashval0] += loaded_val0;
      } else if (table_key0 == unused) { // first time
        table_keyp[hashval0] = loaded_key0;
        table_valp[hashval0] = loaded_val0;
      } else { // collision
        work_collision_keyp[collision_ridx_vreg[i]] = loaded_key0;
        work_collision_valp[collision_ridx_vreg[i]] = loaded_val0;
        collision_ridx_vreg[i]++;
      }

      auto table_key1 = table_keyp[hashval1];
      if(table_key1 == loaded_key1) { // same key already stored
        table_valp[hashval1] += loaded_val1;
      } else if (table_key1 == unused) { // first time
        table_keyp[hashval1] = loaded_key1;
        table_valp[hashval1] = loaded_val1;
      } else { // collision
        work_collision_keyp[collision_ridx_vreg[i]] = loaded_key1;
        work_collision_valp[collision_ridx_vreg[i]] = loaded_val1;
        collision_ridx_vreg[i]++;
      }

      auto table_key2 = table_keyp[hashval2];
      if(table_key2 == loaded_key2) { // same key already stored
        table_valp[hashval2] += loaded_val2;
      } else if (table_key2 == unused) { // first time
        table_keyp[hashval2] = loaded_key2;
        table_valp[hashval2] = loaded_val2;
      } else { // collision
        work_collision_keyp[collision_ridx_vreg[i]] = loaded_key2;
        work_collision_valp[collision_ridx_vreg[i]] = loaded_val2;
        collision_ridx_vreg[i]++;
      }

      auto table_key3 = table_keyp[hashval3];
      if(table_key3 == loaded_key3) { // same key already stored
        table_valp[hashval3] += loaded_val3;
      } else if (table_key3 == unused) { // first time
        table_keyp[hashval3] = loaded_key3;
        table_valp[hashval3] = loaded_val3;
      } else { // collision
        work_collision_keyp[collision_ridx_vreg[i]] = loaded_key3;
        work_collision_valp[collision_ridx_vreg[i]] = loaded_val3;
        collision_ridx_vreg[i]++;
      }

      pos_ridx_vreg[i]+=4;
    }
  }
  for(size_t i = 0; i < SPGEMM_VLEN; i++) {
    pos_ridx[i] = pos_ridx_vreg[i];
    collision_ridx[i] = collision_ridx_vreg[i];
  }
}

template <class T, class O>
void spgemm_hash_accumulator_scalar(O* pos_ridx, O* size_ridx, size_t shift,
                                    size_t* work_keyp, T* work_valp, 
                                    size_t max_idx_bits, size_t unused,
                                    O* table_sizep, O* table_startp,
                                    size_t* table_keyp, T* table_valp) {
  for(size_t i = 0; i < SPGEMM_VLEN; i++) {
    auto pos = pos_ridx[i];
    for(size_t j = 0; j < size_ridx[i]; j++) {
      auto loaded_key = work_keyp[pos + j];
      auto loaded_val = work_valp[pos + j];
      auto col = loaded_key >> max_idx_bits;
      size_t local_shift = shift;
      while(true) {
        auto hashval =
          (loaded_key + local_shift) % table_sizep[col] + table_startp[col];
        auto table_key = table_keyp[hashval];
        if(table_key == loaded_key) { // same key already stored
          table_valp[hashval] += loaded_val;
          break;
        } else if (table_key == unused) { // first time
          table_keyp[hashval] = loaded_key;
          table_valp[hashval] = loaded_val;
          break;
        } else { // collision
          local_shift++;
        }
      }
    }
  }
}

// keyp, valp will be destructed
template <class T, class O>
void spgemm_hash_accumulator(size_t* keyp, T* valp, O total_size,
                             O* nnz_per_columnp, size_t column_size,
                             size_t max_idx_bits,
                             std::vector<size_t>& retidx,
                             std::vector<T>& retval) {
  size_t total_table_size = total_size * SPGEMM_HASH_TABLE_SIZE_MULT;
  size_t unused = std::numeric_limits<size_t>::max();
  std::vector<size_t> table_key(total_table_size, unused);
  std::vector<T> table_val(total_table_size);
  std::vector<size_t> collision_key(total_size);
  std::vector<T> collision_val(total_size);
  std::vector<O> nnz_per_column_pfxsum(column_size);
  std::vector<O> table_size(column_size);
  std::vector<O> table_start(column_size);
  auto nnz_per_column_pfxsump = nnz_per_column_pfxsum.data();
  auto table_startp = table_start.data();
  auto table_sizep = table_size.data();
  prefix_sum(nnz_per_columnp, nnz_per_column_pfxsump+1, column_size-1);
  for(size_t i = 0; i < column_size; i++) {
    table_startp[i] = nnz_per_column_pfxsump[i] * SPGEMM_HASH_TABLE_SIZE_MULT;
    table_sizep[i] = nnz_per_columnp[i] * SPGEMM_HASH_TABLE_SIZE_MULT;
  }

  auto each = ceil_div(total_size, O(SPGEMM_VLEN));
  if(each % 2 == 0) each++;
  each = total_size / O(SPGEMM_VLEN); 
  O pos_ridx[SPGEMM_VLEN]; // ridx: idx for raking
  O pos_stop_ridx[SPGEMM_VLEN];
  auto begin_it = nnz_per_column_pfxsump;
  auto end_it = nnz_per_column_pfxsump + column_size;
  auto current_it = begin_it;
  pos_ridx[0] = 0;
  size_t ii = 1;
  for(size_t i = 1; i < SPGEMM_VLEN; i++) {
    auto it = std::lower_bound(current_it, end_it, each * i);
    if(it == current_it) continue;
    else if(it == end_it) break;
    else {
      pos_ridx[ii++] = *it;
      current_it = it;
    }
  }
  for(size_t i = ii; i < SPGEMM_VLEN; i++) {
    pos_ridx[i] = total_size;
  }
  for(size_t i = 0; i < SPGEMM_VLEN-1; i++) {
    pos_stop_ridx[i] = pos_ridx[i+1];
  }
  pos_stop_ridx[SPGEMM_VLEN-1] = total_size;
  O collision_ridx[SPGEMM_VLEN];
  O original_pos_ridx[SPGEMM_VLEN];
  for(size_t i = 0; i < SPGEMM_VLEN; i++) {
    original_pos_ridx[i] = pos_ridx[i];
    collision_ridx[i] = pos_ridx[i];
  }

  O size_ridx[SPGEMM_VLEN];
  for(size_t i = 0; i < SPGEMM_VLEN; i++)
    size_ridx[i] = pos_stop_ridx[i] - pos_ridx[i];
  O max = 0;
  for(size_t i = 0; i < SPGEMM_VLEN; i++) 
    if(size_ridx[i] > max) max = size_ridx[i];

  size_t valid_vlen = SPGEMM_VLEN;
  for(; valid_vlen > 0; valid_vlen--)
    if(size_ridx[valid_vlen-1] != 0) break;

  O min = std::numeric_limits<O>::max();
  for(size_t i = 0; i < valid_vlen; i++) 
    if(size_ridx[i] < min) min = size_ridx[i];

  auto table_keyp = table_key.data();
  auto table_valp = table_val.data();
  size_t* work_keyp = keyp;
  T* work_valp = valp;
  size_t* work_collision_keyp = collision_key.data();
  T* work_collision_valp = collision_val.data();
  size_t shift = 0;
  while(true) {
    auto unroll4 = min / 4;
    spgemm_hash_accumulator_unroll(valid_vlen, unroll4, pos_ridx, shift,
                                   work_keyp, work_valp,
                                   max_idx_bits, unused,
                                   table_sizep, table_startp,
                                   table_keyp, table_valp,
                                   work_collision_keyp,
                                   work_collision_valp,
                                   collision_ridx);

    O pos_ridx_vreg[SPGEMM_VLEN];
    O pos_stop_ridx_vreg[SPGEMM_VLEN];
    O collision_ridx_vreg[SPGEMM_VLEN];
#pragma _NEC vreg(pos_ridx_vreg)
#pragma _NEC vreg(pos_stop_ridx_vreg)
    for(size_t i = 0; i < SPGEMM_VLEN; i++) {
      pos_ridx_vreg[i] = pos_ridx[i];
      pos_stop_ridx_vreg[i] = pos_stop_ridx[i];
      collision_ridx_vreg[i] = collision_ridx[i];
    }

    auto rest = max - unroll4 * 4;
    for(O b = 0; b < rest; b++) {
#pragma _NEC ivdep
      for(size_t i = 0; i < SPGEMM_VLEN; i++) {
        if(pos_ridx_vreg[i] != pos_stop_ridx_vreg[i]) {
          auto loaded_key = work_keyp[pos_ridx_vreg[i]];
          auto loaded_val = work_valp[pos_ridx_vreg[i]];
          auto col = loaded_key >> max_idx_bits;
          auto hashval =
            (loaded_key + shift) % table_sizep[col] + table_startp[col];
          auto table_key = table_keyp[hashval];
          if(table_key == loaded_key) { // same key already stored
            table_valp[hashval] += loaded_val;
          } else if (table_key == unused) { // first time
            table_keyp[hashval] = loaded_key;
            table_valp[hashval] = loaded_val;
          } else { // collision
            work_collision_keyp[collision_ridx_vreg[i]] = loaded_key;
            work_collision_valp[collision_ridx_vreg[i]] = loaded_val;
            collision_ridx_vreg[i]++;
          }
          pos_ridx_vreg[i]++;
        }
      }
    }
    for(size_t i = 0; i < SPGEMM_VLEN; i++) {
      // pos_ridx[i] = pos_ridx_vreg[i]; // not used
      collision_ridx[i] = collision_ridx_vreg[i];
    }
    int anycollision = false;
    for(size_t i = 0; i < SPGEMM_VLEN; i++) {
      if(collision_ridx[i] != original_pos_ridx[i]) anycollision = true;
    }
    if(anycollision) {
      auto work_keyp_tmp = work_keyp;
      auto work_valp_tmp = work_valp;
      work_keyp = work_collision_keyp;
      work_valp = work_collision_valp;
      work_collision_keyp = work_keyp_tmp;
      work_collision_valp = work_valp_tmp;
      for(size_t i = 0; i < SPGEMM_VLEN; i++) {
        pos_ridx[i] = original_pos_ridx[i];
        pos_stop_ridx[i] = collision_ridx[i];
        collision_ridx[i] = pos_ridx[i];
      }
      for(size_t i = 0; i < SPGEMM_VLEN; i++)
        size_ridx[i] = pos_stop_ridx[i] - pos_ridx[i];
      max = 0;
      for(size_t i = 0; i < SPGEMM_VLEN; i++) 
        if(size_ridx[i] > max) max = size_ridx[i];
      valid_vlen = SPGEMM_VLEN;
      for(; valid_vlen > 0; valid_vlen--)
        if(size_ridx[valid_vlen-1] != 0) break;
      min = std::numeric_limits<O>::max();
      for(size_t i = 0; i < valid_vlen; i++) 
        if(size_ridx[i] < min) min = size_ridx[i];
      shift++;
      size_t remain = 0;
      for(size_t i = 0; i < SPGEMM_VLEN; i++) {
        remain += size_ridx[i];
      }
      if(remain < SPGEMM_HASH_SCALAR_THR) {
        spgemm_hash_accumulator_scalar(pos_ridx, size_ridx, shift,
                                       work_keyp, work_valp, 
                                       max_idx_bits, unused,
                                       table_sizep, table_startp,
                                       table_keyp, table_valp);
        break;
      }
    } else {
      break;
    }
  }
#ifdef __ve__
  spgemm_make_sparse_vector(table_keyp, table_valp, total_table_size, unused,
                            retidx, retval);
#else
  size_t retsize = 0;
  for(size_t i = 0; i < total_table_size; i++) 
    if(table_keyp[i] != unused) retsize++;
  retidx.resize(retsize);
  retval.resize(retsize);
  auto retidxp = retidx.data();
  auto retvalp = retval.data();
  size_t pos = 0;
  for(size_t i = 0; i < total_table_size; i++) {
    if(table_keyp[i] != unused) {
      retidxp[pos] = table_keyp[i];
      retvalp[pos] = table_valp[i];
      pos++;
    }
  }
#endif
}

template <class T, class I, class O>
sparse_vector<T,I>
spgemm_hash_helper(T* mat_valp, I* mat_idxp, O* mat_offp,
                   T* sv_valp, I* sv_idxp, O sv_size,
                   O total_nnz, O* nnzp, O* pfx_sum_nnz,
                   O* merged_total_nnzp, size_t merged_count,
                   size_t max_idx_bits, O* merged_offp,
                   bool sort) {
  sparse_vector<T,I> ret;
  if(sv_size == 0) return ret;
  std::vector<T> mulout_val(total_nnz);
  std::vector<size_t> mulout_idx(total_nnz);
  auto mulout_valp = mulout_val.data();
  auto mulout_idxp = mulout_idx.data();
  spmspv_mul(mat_valp, mat_idxp, mat_offp, sv_valp, sv_idxp, sv_size,
             nnzp, pfx_sum_nnz, total_nnz,
             mulout_valp, mulout_idxp);
  add_column_idx(mulout_idxp, merged_total_nnzp,
                 merged_count, max_idx_bits);
  std::vector<size_t> retidxtmp;
  spgemm_hash_accumulator(mulout_idxp, mulout_valp, total_nnz,
                          merged_total_nnzp, merged_count,
                          max_idx_bits,
                          retidxtmp, ret.val);
  if(sort) {
    auto used_bits = max_bits(merged_count-1);
    auto max_key_size = ceil_div(max_idx_bits + used_bits, size_t(8));
    radix_sort_impl(retidxtmp.data(), ret.val.data(), retidxtmp.size(),
                    max_key_size);
  }
  std::vector<int> extracted_idx(retidxtmp.size());
  ret.idx.resize(retidxtmp.size());
  auto extracted_idxp = extracted_idx.data();
  extract_column_idx(retidxtmp.data(), extracted_idxp, ret.idx.data(),
                     retidxtmp.size(), max_idx_bits);
  auto separated = my_set_separate(extracted_idx);
  // might be smaller than merged_count if sparse vector is zero
  auto separated_size = separated.size();
  auto separatedp = separated.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < separated_size - 1; i++) {
    merged_offp[extracted_idxp[separatedp[i]]] =
      separatedp[i+1] - separatedp[i];
  }
  return ret;
}

template <class T, class I, class O>
void spgemm_hash(T* lval, I* lidx, O* loff,
                 size_t lnnz, size_t lnum_row, size_t lnum_col,
                 T* rval, I* ridx, O* roff,
                 size_t rnnz, size_t rnum_col,
                 std::vector<T>& retval, std::vector<I>& retidx,
                 std::vector<O>& retoff,
                 size_t merge_column_size,
                 bool sort) {
  time_spent t(DEBUG);
  if(rnnz == 0) {
    retoff.resize(rnum_col+1);
    return;
  }
  size_t max_idx_bits = max_bits(lnum_row-1);

  std::vector<O> merged_interim_nnz(rnnz);
  auto merged_interim_nnzp = merged_interim_nnz.data();
  std::vector<O> each_column_nnz(rnum_col);
  auto each_column_nnzp =  each_column_nnz.data();
  for(size_t rc = 0; rc < rnum_col; rc++) { // rc: right column
    auto crnt_ridx = ridx + roff[rc];
    auto crnt_nnz = roff[rc+1] - roff[rc];
    for(size_t i = 0; i < crnt_nnz; i++) {
      merged_interim_nnzp[i] = loff[crnt_ridx[i]+1] - loff[crnt_ridx[i]];
    }
    for(size_t i = 0; i < crnt_nnz; i++) {
      each_column_nnzp[rc] += merged_interim_nnzp[i];
    }
    merged_interim_nnzp += crnt_nnz;
  }
  t.show("count nnz: ");
  auto num_chunks = ceil_div(rnum_col,merge_column_size);
  std::vector<sparse_vector<T,I>> out_sparse_vector;
  out_sparse_vector.reserve(num_chunks);
  
  std::vector<O> retofftmp(rnum_col);
  auto retofftmpp = retofftmp.data();
  size_t crnt_rc = 0;
  merged_interim_nnzp = merged_interim_nnz.data();
  while(true) {
    size_t merge_size = 0;
    if(crnt_rc + merge_column_size < rnum_col)
      merge_size = merge_column_size;
    else
      merge_size = rnum_col - crnt_rc;
    auto crnt_rval = rval + roff[crnt_rc];
    auto crnt_ridx = ridx + roff[crnt_rc];
    auto crnt_size = roff[crnt_rc + merge_size] - roff[crnt_rc];
    if(crnt_size == 0) { // (contiguous) zero sized columns
      out_sparse_vector.push_back(sparse_vector<T,I>());
      crnt_rc += merge_size;
      if(crnt_rc == rnum_col) break;
      merged_interim_nnzp += crnt_size;
      each_column_nnzp += merge_size;
      retofftmpp += merge_size;
    } else {
      std::vector<O> pfx_sum_merged_interim_nnz(crnt_size);
      auto pfx_sum_merged_interim_nnzp = pfx_sum_merged_interim_nnz.data();
      prefix_sum(merged_interim_nnzp, pfx_sum_merged_interim_nnzp, crnt_size);
      auto total_interim_nnz = pfx_sum_merged_interim_nnzp[crnt_size - 1];
      if(total_interim_nnz == 0) { // zero size cannot be handled
        out_sparse_vector.push_back(sparse_vector<T,I>());
      } else {
        out_sparse_vector.push_back
          (spgemm_hash_helper(lval, lidx, loff,
                              crnt_rval, crnt_ridx, crnt_size,
                              total_interim_nnz, merged_interim_nnzp,
                              pfx_sum_merged_interim_nnzp,
                              each_column_nnzp, merge_size,
                              max_idx_bits, retofftmpp, sort));
      }
      crnt_rc += merge_size;
      if(crnt_rc == rnum_col) break;
      merged_interim_nnzp += crnt_size;
      each_column_nnzp += merge_size;
      retofftmpp += merge_size;
    }
  }
  t.show("time for creating sparse vectors: ");
  create_sparse_matrix(out_sparse_vector, retval, retidx, retoff, retofftmp);
  t.show("time for creating sparse matrix: ");
}

template <class O>
size_t max_interm_nnz_per_merged_columns
(O* interim_nnz_per_columnp, size_t rnum_col, size_t merge_column_size) {
  auto num_chunks = rnum_col / merge_column_size;
  std::vector<O> sizes(num_chunks);
  auto sizesp = sizes.data();
  for(size_t m = 0; m < merge_column_size; m++) {
    for(size_t c = 0; c < num_chunks; c++) {
      sizesp[c] += interim_nnz_per_columnp[merge_column_size * c + m];
    }
  }
  size_t rest_size = 0;
  for(size_t r = num_chunks * merge_column_size; r < rnum_col; r++) {
    rest_size += interim_nnz_per_columnp[r];
  }
  size_t max = 0;
  for(size_t i = 0; i < num_chunks; i++) {
    if(sizes[i] > max) max = sizes[i];
  }
  if(max > rest_size) return max; else return rest_size;
}

template <class T, class I, class O>
size_t spgemm_spa_make_sparse_vector(I* spa_idx_bufp,
                                     O stride,
                                     O* out_ridx,
                                     O* original_out_ridx,
                                     T* retvalp,
                                     I* retidxp,
                                     T* vp,
                                     size_t size){
  if(size == 0) return 0;
  size_t retsize = 0;
  O crnt_ridx_vreg[SPGEMM_VLEN];
#pragma _NEC vreg(crnt_ridx_vreg)
  for(size_t i = 0; i < SPGEMM_VLEN; i++)
    crnt_ridx_vreg[i] = original_out_ridx[i];
  O stop_ridx_vreg[SPGEMM_VLEN];
#pragma _NEC vreg(stop_ridx_vreg)
  for(size_t i = 0; i < SPGEMM_VLEN; i++)
    stop_ridx_vreg[i] = out_ridx[i];

  O min = std::numeric_limits<O>::max();
  O max = 0;
  for(size_t i = 0; i < SPGEMM_VLEN; i++) {
    auto size = out_ridx[i] - original_out_ridx[i];
    if(min > size) min = size;
    if(max < size) max = size;
    retsize += size;
  }
  for(size_t m = 0; m < min; m++) {
#pragma _NEC ivdep
    for(size_t i = 0; i < SPGEMM_VLEN; i++) {
      auto idx = spa_idx_bufp[crnt_ridx_vreg[i]];
      retidxp[i] = idx;
      retvalp[i] = vp[idx];
      crnt_ridx_vreg[i]++;
    }
    retvalp += SPGEMM_VLEN;
    retidxp += SPGEMM_VLEN;
  }

  for(size_t m = 0; m < max - min; m++) {
    size_t ii = 0;
#pragma _NEC ivdep
    for(size_t i = 0; i < SPGEMM_VLEN; i++) {
      if(crnt_ridx_vreg[i] != stop_ridx_vreg[i]) {
        auto idx = spa_idx_bufp[crnt_ridx_vreg[i]];
        retidxp[ii] = idx;
        retvalp[ii] = vp[idx];
        crnt_ridx_vreg[i]++;
        ii++;
      }
    }
    retvalp += ii;
    retidxp += ii;
  }
  return retsize;
}

template <class T, class I, class O>
sparse_vector<T,I> spgemm_spa_helper(int* spa_flag_bufp,
                                     I* spa_idx_bufp,
                                     O stride,
                                     T* sparse_vector_val_bufp,
                                     I* sparse_vector_idx_bufp,
                                     T* dense_vector_bufp,
                                     size_t lnum_row,
                                     T* lval, I* lidx, O* loff,
                                     T* rval, I* ridx, O* roff,
                                     size_t crnt_rc,
                                     size_t merge_size,
                                     O* merged_offp,
                                     bool sort) {
  sparse_vector<T,I> ret;
  size_t current_sparse_idx = 0;
  auto sv_valp = rval + roff[crnt_rc];
  auto sv_idxp = ridx + roff[crnt_rc];
  O out_ridx[SPGEMM_VLEN];
  O original_out_ridx[SPGEMM_VLEN];
  for(size_t i = 0; i < SPGEMM_VLEN; i++) {
    original_out_ridx[i] = stride * i;
    out_ridx[i] = original_out_ridx[i];
  }
  for(size_t r = 0; r < merge_size; r++) {
    auto sv_size = roff[crnt_rc + r + 1] - roff[crnt_rc + r];
    if(sv_size == 0) {
      continue;
    } else {
      O out_ridx_vreg[SPGEMM_VLEN];
#pragma _NEC vreg(out_ridx_vreg)
      for(size_t i = 0; i < SPGEMM_VLEN; i++) out_ridx_vreg[i] = out_ridx[i];
      for(size_t i = 0; i < sv_size; i++) {
        auto off = loff[sv_idxp[i]];
        auto crnt_nnz = loff[sv_idxp[i]+1] - off;
        auto valp = lval + off;
        auto idxp = lidx + off;
        auto scal = sv_valp[i];
        auto each = crnt_nnz / SPGEMM_VLEN;
        auto rest = crnt_nnz - each * SPGEMM_VLEN;
        for(size_t e = 0; e < each; e++) {
#pragma _NEC ivdep
          for(size_t j = 0; j < SPGEMM_VLEN; j++) {
            auto idx = idxp[j];
            dense_vector_bufp[idx] += scal * valp[j];
            if(spa_flag_bufp[idx] == 0) {
              spa_flag_bufp[idx] = 1;
              spa_idx_bufp[out_ridx_vreg[j]++] = idx;
            }
          }
          valp += SPGEMM_VLEN;
          idxp += SPGEMM_VLEN;
        }
#pragma _NEC ivdep
        for(size_t j = 0; j < rest; j++) {
          auto idx = idxp[j];
          dense_vector_bufp[idx] += scal * valp[j];
          if(spa_flag_bufp[idx] == 0) {
            spa_flag_bufp[idx] = 1;
            spa_idx_bufp[out_ridx_vreg[j]++] = idx;
          }
        }
      }
      for(size_t j = 0; j < SPGEMM_VLEN; j++) out_ridx[j] = out_ridx_vreg[j];
      auto sparse_size = spgemm_spa_make_sparse_vector
        (spa_idx_bufp,
         stride,
         out_ridx,
         original_out_ridx,
         sparse_vector_val_bufp+current_sparse_idx,
         sparse_vector_idx_bufp+current_sparse_idx,
         dense_vector_bufp,
         lnum_row);
      auto sparse_idx = sparse_vector_idx_bufp + current_sparse_idx;
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
      for(size_t i = 0; i < sparse_size; i++) { // clean up for next loop
        dense_vector_bufp[sparse_idx[i]] = 0;
        spa_flag_bufp[sparse_idx[i]] = 0;
      }
      for(size_t i = 0; i < SPGEMM_VLEN; i++) {
        out_ridx[i] = original_out_ridx[i];
      }
      merged_offp[r] = sparse_size;
      current_sparse_idx += sparse_size;
      sv_valp += sv_size;
      sv_idxp += sv_size;
    }
  }
  if(sort) {
    size_t max_idx_bits = max_bits(lnum_row-1);
    auto tmp_idxp = sparse_vector_idx_bufp;
    for(size_t i = 0; i < merge_size; i++) {
      for(size_t j = 0; j < merged_offp[i]; j++) {
        tmp_idxp[j] += (i << max_idx_bits);
      }
      tmp_idxp += merged_offp[i];
    }
    auto used_bits = max_bits(merge_size-1);
    auto max_key_size = ceil_div(max_idx_bits + used_bits, size_t(8));
    radix_sort_impl(sparse_vector_idx_bufp, sparse_vector_val_bufp,
                    current_sparse_idx, max_key_size);
    size_t mask = (static_cast<size_t>(1) << max_idx_bits) - 1;
    for(size_t i = 0; i < current_sparse_idx; i++) {
      sparse_vector_idx_bufp[i] &= mask;
    }
  }
  ret.val.resize(current_sparse_idx);
  ret.idx.resize(current_sparse_idx);
  auto retvalp = ret.val.data();
  auto retidxp = ret.idx.data();
  for(size_t i = 0; i < current_sparse_idx; i++) {
    retvalp[i] = sparse_vector_val_bufp[i];
    retidxp[i] = sparse_vector_idx_bufp[i];
  }
  return ret;
}

template <class T, class I, class O>
void spgemm_spa(T* lval, I* lidx, O* loff,
                size_t lnnz, size_t lnum_row, size_t lnum_col,
                T* rval, I* ridx, O* roff,
                size_t rnnz, size_t rnum_col,
                std::vector<T>& retval, std::vector<I>& retidx,
                std::vector<O>& retoff,
                size_t merge_column_size,
                bool sort) {
  time_spent t(DEBUG);
  if(rnnz == 0) {
    retoff.resize(rnum_col+1);
    return;
  }
  std::vector<O> interim_nnz_per_column(rnum_col);
  std::vector<O> stride_per_column(rnum_col);
  auto interim_nnz_per_columnp =  interim_nnz_per_column.data();
  auto stride_per_columnp = stride_per_column.data();
  for(size_t rc = 0; rc < rnum_col; rc++) { // rc: right column
    auto crnt_ridx = ridx + roff[rc];
    auto crnt_nnz = roff[rc+1] - roff[rc];
    for(size_t i = 0; i < crnt_nnz; i++) {
      auto nnz_per_scalar = loff[crnt_ridx[i]+1] - loff[crnt_ridx[i]];
      interim_nnz_per_columnp[rc] += nnz_per_scalar;
      if(nnz_per_scalar != 0) {
        stride_per_columnp[rc] += (nnz_per_scalar - 1) / O(SPGEMM_VLEN) + 1;
      }
    }
  }
  O max_stride_per_column = 0;
  for(size_t i = 0; i < rnum_col; i++) {
    if(max_stride_per_column < stride_per_columnp[i])
      max_stride_per_column = stride_per_columnp[i];
  }
  auto sparse_vector_bufsize = max_interm_nnz_per_merged_columns
    (interim_nnz_per_columnp, rnum_col, merge_column_size);
  t.show("count nnz: ");
  std::vector<int> spa_flag_buf(lnum_row);
  std::vector<I> spa_idx_buf(max_stride_per_column * SPGEMM_VLEN);
  std::vector<T> sparse_vector_val_buf(sparse_vector_bufsize);
  std::vector<I> sparse_vector_idx_buf(sparse_vector_bufsize);
  std::vector<T> dense_vector_buf(lnum_row);
  
  auto num_chunks = ceil_div(rnum_col,merge_column_size);
  std::vector<sparse_vector<T,I>> out_sparse_vector;
  out_sparse_vector.reserve(num_chunks);
  
  std::vector<O> retofftmp(rnum_col);
  auto retofftmpp = retofftmp.data();
  size_t crnt_rc = 0;
  while(true) {
    size_t merge_size = 0;
    if(crnt_rc + merge_column_size < rnum_col)
      merge_size = merge_column_size;
    else
      merge_size = rnum_col - crnt_rc;
    out_sparse_vector.push_back
      (spgemm_spa_helper(spa_flag_buf.data(),
                         spa_idx_buf.data(),
                         max_stride_per_column,
                         sparse_vector_val_buf.data(),
                         sparse_vector_idx_buf.data(),
                         dense_vector_buf.data(),
                         lnum_row, 
                         lval, lidx, loff,
                         rval, ridx, roff, crnt_rc,
                         merge_size, retofftmpp, sort));
    crnt_rc += merge_size;
    if(crnt_rc == rnum_col) break;
    retofftmpp += merge_size;
  }
  t.show("time for creating sparse vectors: ");
  create_sparse_matrix(out_sparse_vector, retval, retidx, retoff, retofftmp);
  t.show("time for creating sparse matrix: ");
}

template <class T, class I, class O>
ccs_matrix_local<T,I,O>
spgemm(ccs_matrix_local<T,I,O>& left, 
       ccs_matrix_local<T,I,O>& right,
       spgemm_type type = spgemm_type::block_esc,
       size_t merge_column_size = 4096) {
  if(left.local_num_col != right.local_num_row) 
    throw std::runtime_error("spgemm: matrix size mismatch");
  ccs_matrix_local<T,I,O> ret;
  if(type == spgemm_type::esc) {
    spgemm_esc(left.val.data(), left.idx.data(), left.off.data(),
               left.val.size(), left.local_num_row, left.local_num_col,
               right.val.data(), right.idx.data(), right.off.data(),
               right.val.size(), right.local_num_col,
               ret.val, ret.idx, ret.off);
  } else if(type == spgemm_type::block_esc) {
    spgemm_block_esc(left.val.data(), left.idx.data(), left.off.data(),
                     left.val.size(), left.local_num_row, left.local_num_col,
                     right.val.data(), right.idx.data(), right.off.data(),
                     right.val.size(), right.local_num_col,
                     ret.val, ret.idx, ret.off,
                     merge_column_size);
  } else if(type == spgemm_type::block_esc_binary_input) {
    spgemm_block_esc_binary_input
      (left.val.data(), left.idx.data(), left.off.data(),
       left.val.size(), left.local_num_row, left.local_num_col,
       right.val.data(), right.idx.data(), right.off.data(),
       right.val.size(), right.local_num_col,
       ret.val, ret.idx, ret.off,
       merge_column_size);
  } else if(type == spgemm_type::hash) {
    spgemm_hash(left.val.data(), left.idx.data(), left.off.data(),
                left.val.size(), left.local_num_row, left.local_num_col,
                right.val.data(), right.idx.data(), right.off.data(),
                right.val.size(), right.local_num_col,
                ret.val, ret.idx, ret.off,
                merge_column_size, false);
  } else if(type == spgemm_type::hash_sort) {
    spgemm_hash(left.val.data(), left.idx.data(), left.off.data(),
                left.val.size(), left.local_num_row, left.local_num_col,
                right.val.data(), right.idx.data(), right.off.data(),
                right.val.size(), right.local_num_col,
                ret.val, ret.idx, ret.off,
                merge_column_size, true);
  } else if(type == spgemm_type::spa) {
    spgemm_spa(left.val.data(), left.idx.data(), left.off.data(),
               left.val.size(), left.local_num_row, left.local_num_col,
               right.val.data(), right.idx.data(), right.off.data(),
               right.val.size(), right.local_num_col,
               ret.val, ret.idx, ret.off,
               merge_column_size, false);
  } else if(type == spgemm_type::spa_sort) {
    spgemm_spa(left.val.data(), left.idx.data(), left.off.data(),
               left.val.size(), left.local_num_row, left.local_num_col,
               right.val.data(), right.idx.data(), right.off.data(),
               right.val.size(), right.local_num_col,
               ret.val, ret.idx, ret.off,
               merge_column_size, true);
  } else {
    throw std::runtime_error("unknown spgemm_type");
  }
  ret.set_local_num(left.local_num_row);
  return ret;
}

template <class T, class I, class O>
crs_matrix_local<T,I,O>
spgemm(crs_matrix_local<T,I,O>& a,
       crs_matrix_local<T,I,O>& b, 
       spgemm_type type = spgemm_type::block_esc,
       size_t merge_column_size = 4096) {
  if(a.local_num_col != b.local_num_row) 
    throw std::runtime_error("spgemm: matrix size mismatch");
  crs_matrix_local<T,I,O> ret;
  if(type == spgemm_type::esc) {
    spgemm_esc(b.val.data(), b.idx.data(), b.off.data(),
               b.val.size(), b.local_num_col, b.local_num_row,
               a.val.data(), a.idx.data(), a.off.data(),
               a.val.size(), a.local_num_row,
               ret.val, ret.idx, ret.off);
  } else if(type == spgemm_type::block_esc) {
    spgemm_block_esc(b.val.data(), b.idx.data(), b.off.data(),
                     b.val.size(), b.local_num_col, b.local_num_row,
                     a.val.data(), a.idx.data(), a.off.data(),
                     a.val.size(), a.local_num_row,
                     ret.val, ret.idx, ret.off,
                     merge_column_size);
  } else if(type == spgemm_type::block_esc_binary_input) {
    spgemm_block_esc_binary_input
      (b.val.data(), b.idx.data(), b.off.data(),
       b.val.size(), b.local_num_col, b.local_num_row,
       a.val.data(), a.idx.data(), a.off.data(),
       a.val.size(), a.local_num_row,
       ret.val, ret.idx, ret.off,
       merge_column_size);
  } else if(type == spgemm_type::hash) {
    spgemm_hash(b.val.data(), b.idx.data(), b.off.data(),
                b.val.size(), b.local_num_col, b.local_num_row,
                a.val.data(), a.idx.data(), a.off.data(),
                a.val.size(), a.local_num_row,
                ret.val, ret.idx, ret.off,
                merge_column_size, false);
  } else if(type == spgemm_type::hash_sort) {
    spgemm_hash(b.val.data(), b.idx.data(), b.off.data(),
                b.val.size(), b.local_num_col, b.local_num_row,
                a.val.data(), a.idx.data(), a.off.data(),
                a.val.size(), a.local_num_row,
                ret.val, ret.idx, ret.off,
                merge_column_size, true);
  } else if(type == spgemm_type::spa) {
    spgemm_spa(b.val.data(), b.idx.data(), b.off.data(),
               b.val.size(), b.local_num_col, b.local_num_row,
               a.val.data(), a.idx.data(), a.off.data(),
               a.val.size(), a.local_num_row,
               ret.val, ret.idx, ret.off,
               merge_column_size, false);
  } else if(type == spgemm_type::spa_sort) {
    spgemm_spa(b.val.data(), b.idx.data(), b.off.data(),
               b.val.size(), b.local_num_col, b.local_num_row,
               a.val.data(), a.idx.data(), a.off.data(),
               a.val.size(), a.local_num_row,
               ret.val, ret.idx, ret.off,
               merge_column_size, true);
  } else{
    throw std::runtime_error("unknown spgemm_type");
  }
  ret.set_local_num(b.local_num_col);
  return ret;
}

// left/right is ccs base (so left and right are interchanged)
template <class I, class O>
void calc_matrix_separator(I* ridx, O* roff, size_t rnum_col,
                           O* loff, size_t average_nnz_thr,
                           std::vector<O>& type1cols,
                           std::vector<O>& type2cols) {
  
  std::vector<O> avg(rnum_col); // integer is OK
  auto avgp = avg.data();
  for(size_t rc = 0; rc < rnum_col; rc++) { // rc: right column
    auto crnt_ridx = ridx + roff[rc];
    auto crnt_nnz = roff[rc+1] - roff[rc];
    O interim_nnz_per_column = 0;
    for(size_t i = 0; i < crnt_nnz; i++) {
      interim_nnz_per_column += loff[crnt_ridx[i]+1] - loff[crnt_ridx[i]];
    }
    if(crnt_nnz == 0) avgp[rc] = 0;
    else avgp[rc] = interim_nnz_per_column / crnt_nnz;
  }
  std::vector<I> idx(rnum_col);
  auto idxp = idx.data();
  for(size_t i = 0; i < rnum_col; i++) idxp[i] = i;
  radix_sort_desc(avg,idx,true); // positive only
  auto it = std::lower_bound(avg.begin(), avg.end(), average_nnz_thr,
                             std::greater<O>());
  auto type1size = it - avg.begin();
  auto type2size = rnum_col - type1size;
  type1cols.resize(type1size);
  type2cols.resize(type2size);
  auto type1colsp = type1cols.data();
  auto type2colsp = type2cols.data();
  for(size_t i = 0; i < type1size; i++) type1colsp[i] = idxp[i];
  for(size_t i = 0; i < type2size; i++) type2colsp[i] = idxp[type1size + i];
}

// As for left, part of average nnz is larger than the argument threshold is
// separated and type1 is used
template <class T, class I, class O>
crs_matrix_local<T,I,O>
spgemm_hybrid(crs_matrix_local<T,I,O>& left, 
              crs_matrix_local<T,I,O>& right,
              spgemm_type type1,
              spgemm_type type2,
              size_t merge_column_size1,
              size_t merge_column_size2,
              size_t average_nnz_thr) {
  time_spent t(DEBUG);
  if(left.local_num_row == 0) return crs_matrix_local<T,I,O>();
  std::vector<O> type1rows, type2rows;
  calc_matrix_separator(left.idx.data(), left.off.data(),
                        left.local_num_row,
                        right.off.data(), average_nnz_thr,
                        type1rows, type2rows);
  t.show("calc_matrix_separator: ");
  auto type1rows_size = type1rows.size();
  auto type2rows_size = type2rows.size();
  crs_matrix_local<T,I,O> left_type1(type1rows_size, left.local_num_col);
  crs_matrix_local<T,I,O> left_type2(type2rows_size, left.local_num_col);
  size_t type1nnz = 0;
  auto leftoffp = left.off.data();
  auto leftidxp = left.idx.data();
  auto leftvalp = left.val.data();
  auto type1rowsp = type1rows.data();
  auto type2rowsp = type2rows.data();
  for(size_t i = 0; i < type1rows_size; i++) {
    type1nnz += leftoffp[type1rowsp[i]+1] - leftoffp[type1rowsp[i]];
  }
  auto allnnz = left.val.size();
  auto type2nnz = allnnz - type1nnz;
  LOG(DEBUG) << "type1 nnz = " << type1nnz
             << ", type1 nrows = " << type1rows_size
             << ", type2 nnz = " << type2nnz
             << ", type2 nrows = " << type2rows_size << std::endl;
  left_type1.val.resize(type1nnz);
  left_type1.idx.resize(type1nnz);
  left_type1.off.resize(type1rows_size + 1);
  left_type2.val.resize(type2nnz);
  left_type2.idx.resize(type2nnz);
  left_type2.off.resize(type2rows_size + 1);
  auto crnt_type1valp = left_type1.val.data();
  auto crnt_type1idxp = left_type1.idx.data();
  auto crnt_type2valp = left_type2.val.data();
  auto crnt_type2idxp = left_type2.idx.data();
  std::vector<O> type1offtmp(type1rows_size);
  std::vector<O> type2offtmp(type2rows_size);
  auto type1offtmpp = type1offtmp.data();
  auto type2offtmpp = type2offtmp.data();
  for(size_t i = 0; i < type1rows_size; i++) {
    auto off = leftoffp[type1rowsp[i]];
    auto size = leftoffp[type1rowsp[i]+1] - off;
    type1offtmpp[i] = size;
    auto srcvalp = leftvalp + off;
    auto srcidxp = leftidxp + off;
    for(size_t j = 0; j < size; j++) {
      crnt_type1valp[j] = srcvalp[j];
      crnt_type1idxp[j] = srcidxp[j];
    }
    crnt_type1valp += size;
    crnt_type1idxp += size;
  }
  auto type1offp = left_type1.off.data();
  prefix_sum(type1offtmpp, type1offp+1, type1rows_size);
  for(size_t i = 0; i < type2rows_size; i++) {
    auto off = leftoffp[type2rowsp[i]];
    auto size = leftoffp[type2rowsp[i]+1] - off;
    type2offtmpp[i] = size;
    auto srcvalp = leftvalp + off;
    auto srcidxp = leftidxp + off;
    for(size_t j = 0; j < size; j++) {
      crnt_type2valp[j] = srcvalp[j];
      crnt_type2idxp[j] = srcidxp[j];
    }
    crnt_type2valp += size;
    crnt_type2idxp += size;
  }
  auto type2offp = left_type2.off.data();
  prefix_sum(type2offtmpp, type2offp+1, type2rows_size);
  t.show("create separated matrix: ");

  auto type1res = spgemm(left_type1, right, type1, merge_column_size1);
  t.show("type1: ");
  auto type2res = spgemm(left_type2, right, type2, merge_column_size2);
  t.show("type2: ");

  auto retnnz = type1res.val.size() + type2res.val.size();
  crs_matrix_local<T,I,O> ret(left.local_num_row, right.local_num_col);
  ret.val.resize(retnnz);
  ret.idx.resize(retnnz);
  ret.off.resize(left.local_num_row+1);
  auto retvalp = ret.val.data();
  auto retidxp = ret.idx.data();
  auto retoffp = ret.off.data();
  
  auto type1resoffp = type1res.off.data();
  auto type2resoffp = type2res.off.data();
  std::vector<O> offtmp(left.local_num_row);
  auto offtmpp = offtmp.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < type1rows_size; i++) {
    offtmpp[type1rowsp[i]] = type1resoffp[i+1] - type1resoffp[i];
  }
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < type2rows_size; i++) {
    offtmpp[type2rowsp[i]] = type2resoffp[i+1] - type2resoffp[i];
  }
  prefix_sum(offtmpp, retoffp+1, left.local_num_row);

  auto crnt_type1resvalp = type1res.val.data();
  auto crnt_type1residxp = type1res.idx.data();
  for(size_t i = 0; i < type1rows_size; i++) {
    auto off = retoffp[type1rowsp[i]];
    auto size = offtmpp[type1rowsp[i]];
    auto dstvalp = retvalp + off;
    auto dstidxp = retidxp + off;
    for(size_t j = 0; j < size; j++) {
      dstvalp[j] = crnt_type1resvalp[j];
      dstidxp[j] = crnt_type1residxp[j];
    }
    crnt_type1resvalp += size;
    crnt_type1residxp += size;
  }

  auto crnt_type2resvalp = type2res.val.data();
  auto crnt_type2residxp = type2res.idx.data();
  for(size_t i = 0; i < type2rows_size; i++) {
    auto off = retoffp[type2rowsp[i]];
    auto size = offtmpp[type2rowsp[i]];
    auto dstvalp = retvalp + off;
    auto dstidxp = retidxp + off;
    for(size_t j = 0; j < size; j++) {
      dstvalp[j] = crnt_type2resvalp[j];
      dstidxp[j] = crnt_type2residxp[j];
    }
    crnt_type2resvalp += size;
    crnt_type2residxp += size;
  }
  t.show("create ret: ");
  return ret;
}

template <class T, class I, class O>
void set_diag_zero_impl_raking(T* valp, I* idxp, O* offp,
                               size_t num_row, size_t size,
                               size_t shift = 0, bool sorted = false) {
  auto each = ceil_div(size, size_t(SPGEMM_VLEN));
  if(each % 2 == 0) each++;
  I row_ridx[SPGEMM_VLEN]; // ridx: idx for raking
#pragma _NEC vreg(row_ridx)
  int valid[SPGEMM_VLEN];
//#pragma _NEC vreg(valid) // does not work on 2.3.0
  for(size_t i = 0; i < SPGEMM_VLEN; i++) valid[i] = true;
  auto begin_it = offp;
  auto end_it = offp + num_row;
  auto current_it = begin_it;
  row_ridx[0] = 0;
  size_t ii = 1;
  for(size_t i = 1; i < SPGEMM_VLEN; i++) {
    auto it = std::lower_bound(current_it, end_it, each * i);
    if(it == current_it) continue;
    else if(it == end_it) break;
    else {
      row_ridx[ii++] = it - begin_it;
      current_it = it;
    }
  }
  for(size_t i = ii; i < SPGEMM_VLEN; i++) {
    valid[i] = false;
    row_ridx[i] = num_row;
  }

  I pos_ridx[SPGEMM_VLEN]; 
//#pragma _NEC vreg(pos_ridx) // does not work on 2.3.0
  I pos_stop_ridx[SPGEMM_VLEN]; 
#pragma _NEC vreg(pos_stop_ridx)
  for(size_t i = 0; i < SPGEMM_VLEN; i++) {
    if(valid[i]) pos_ridx[i] = offp[row_ridx[i]];
    else pos_ridx[i] = size;
  }
  for(size_t i = 0; i < SPGEMM_VLEN-1; i++) {
    if(valid[i]) pos_stop_ridx[i] = pos_ridx[i+1];
    else pos_stop_ridx[i] = size;
  }
  pos_stop_ridx[SPGEMM_VLEN-1] = size;

  I pos_nextrow_ridx[SPGEMM_VLEN];
#pragma _NEC vreg(pos_nextrow_ridx)
  for(size_t i = 0; i < SPGEMM_VLEN; i++) {
    if(valid[i]) pos_nextrow_ridx[i] = offp[row_ridx[i]+1];
    else pos_nextrow_ridx[i] = size;
  }

  I diag_ridx[SPGEMM_VLEN];
#pragma _NEC vreg(diag_ridx)
  for(size_t i = 0; i < SPGEMM_VLEN; i++) {
    if(valid[i]) diag_ridx[i] = row_ridx[i] + shift;
    else diag_ridx[i] = num_row;
  }

  int anyvalid = true;
  if(sorted) {
    while(anyvalid) {
#pragma cdir nodep
#pragma _NEC ivdep
      for(size_t i = 0; i < SPGEMM_VLEN; i++) {
        if(valid[i]) {
          if(pos_ridx[i] == pos_stop_ridx[i]) {
            valid[i] = false;
          } else if(pos_ridx[i] == pos_nextrow_ridx[i] ||
                    idxp[pos_ridx[i]] > diag_ridx[i]) {
            pos_ridx[i] = pos_nextrow_ridx[i];
            row_ridx[i]++;
            pos_nextrow_ridx[i] = offp[row_ridx[i]+1];
            diag_ridx[i] = row_ridx[i] + shift;
          } else {
            if(idxp[pos_ridx[i]] == diag_ridx[i]) {
              valp[pos_ridx[i]] = 0;
            } 
            pos_ridx[i]++;
          }
        }
      }
      anyvalid = false;
      for(size_t i = 0; i < SPGEMM_VLEN; i++) {
        if(valid[i]) anyvalid = true;
      }
    }
  } else {
    while(anyvalid) {
#pragma cdir nodep
#pragma _NEC ivdep
      for(size_t i = 0; i < SPGEMM_VLEN; i++) {
        if(valid[i]) {
          if(pos_ridx[i] == pos_stop_ridx[i]) {
            valid[i] = false;
          } else if(pos_ridx[i] == pos_nextrow_ridx[i]) {
            row_ridx[i]++;
            pos_nextrow_ridx[i] = offp[row_ridx[i]+1];
            diag_ridx[i] = row_ridx[i] + shift;
          } else {
            if(idxp[pos_ridx[i]] == diag_ridx[i]) {
              valp[pos_ridx[i]] = 0;
            } 
            pos_ridx[i]++;
          }
        }
      }
      anyvalid = false;
      for(size_t i = 0; i < SPGEMM_VLEN; i++) {
        if(valid[i]) anyvalid = true;
      }
    }
  }
}

template <class T, class I, class O>
void set_diag_zero_impl(T* valp, I* idxp, O* offp,
                        size_t num_row, size_t size,
                        size_t shift = 0, bool sorted = false) {
  if(sorted) {
    for(size_t r = 0; r < num_row; r++) {
      size_t pos = offp[r];
      size_t size = offp[r+1] - offp[r];
      for(size_t cidx = 0; cidx < size; cidx++) {
        if(idxp[pos + cidx] > r + shift) continue;
        else if(idxp[pos + cidx] == r + shift) {
          valp[pos + cidx] = 0;
        }
      }
    }
  } else {
    for(size_t r = 0; r < num_row; r++) {
      size_t pos = offp[r];
      size_t size = offp[r+1] - offp[r];
      for(size_t cidx = 0; cidx < size; cidx++) {
        if(idxp[pos + cidx] == r + shift) {
          valp[pos + cidx] = 0;
        }
      }
    }
  }
}

template <class T, class I, class O>
void set_diag_zero(crs_matrix_local<T,I,O>& mat, size_t shift = 0,
                   bool sorted = false) { // sorted is slower?
  auto valp = mat.val.data();
  auto idxp = mat.idx.data();
  auto offp = mat.off.data();
  auto num_row = mat.local_num_row;
  auto size  = mat.val.size();
  if(size == 0 || num_row == 0) return;
  if(size / num_row > SET_DIAG_ZERO_THR)
    set_diag_zero_impl(valp, idxp, offp, num_row, size, shift, sorted);
  else
    set_diag_zero_impl_raking(valp, idxp, offp, num_row, size, shift, sorted);
}

// used for triangle counting
// diag is not extracted currently
template <class T, class I, class O>
void separate_upper_lower(crs_matrix_local<T,I,O>& mat,
                          crs_matrix_local<T,I,O>& upper,
                          crs_matrix_local<T,I,O>& lower) {
  auto num_row = mat.local_num_row;
  auto num_col = mat.local_num_col;
  upper.local_num_row = num_row;
  upper.local_num_col = num_col;
  lower.local_num_row = num_row;
  lower.local_num_col = num_col;
  auto matnnz = mat.val.size();
  std::vector<T> upperval(matnnz);
  std::vector<I> upperidx(matnnz);
  std::vector<O> upperoff(num_row); // only size; prefix sum later
  auto uppervalp = upperval.data();
  auto upperidxp = upperidx.data();
  auto upperoffp = upperoff.data();
  size_t uppernnz = 0;
  std::vector<T> lowerval(matnnz);
  std::vector<I> loweridx(matnnz);
  std::vector<O> loweroff(num_row);
  auto lowervalp = lowerval.data();
  auto loweridxp = loweridx.data();
  auto loweroffp = loweroff.data();
  size_t lowernnz = 0;
  auto matvalp = mat.val.data();
  auto matidxp = mat.idx.data();
  auto matoffp = mat.off.data();
  for(size_t r = 0; r < num_row; r++) {
    auto off = matoffp[r];
    auto size = matoffp[r+1] - off;
    auto crntmatval = matvalp + off;
    auto crntmatidx = matidxp + off;
    size_t diag_pos = 0;
    for(diag_pos = 0; diag_pos < size; diag_pos++) {
      if(crntmatidx[diag_pos] >= r) break;
    }
    size_t lower_stop = diag_pos; // exclusive
    auto crntlowervalp = lowervalp + lowernnz;
    auto crntloweridxp = loweridxp + lowernnz;
    for(size_t i = 0; i < lower_stop; i++) {
      crntlowervalp[i] = crntmatval[i];
      crntloweridxp[i] = crntmatidx[i];
    }
    loweroffp[r] = lower_stop;
    lowernnz += lower_stop;
    size_t upper_start = crntmatidx[diag_pos] == r ? diag_pos + 1 : diag_pos;
    size_t upper_size = size > upper_start ? size - upper_start : 0;
    auto crntuppervalp = uppervalp + uppernnz;
    auto crntupperidxp = upperidxp + uppernnz;
    crntmatval = matvalp + off + upper_start;
    crntmatidx = matidxp + off + upper_start;
    for(size_t i = 0; i < upper_size; i++) {
      crntuppervalp[i] = crntmatval[i];
      crntupperidxp[i] = crntmatidx[i];
    }
    upperoffp[r] = upper_size;
    uppernnz += upper_size;
  }
  upper.val.resize(uppernnz);
  upper.idx.resize(uppernnz);
  upper.off.resize(num_row+1);
  auto retuppervalp = upper.val.data();
  auto retupperidxp = upper.idx.data();
  auto retupperoffp = upper.off.data();
  for(size_t i = 0; i < uppernnz; i++) {
    retuppervalp[i] = uppervalp[i];
    retupperidxp[i] = upperidxp[i];
  }
  prefix_sum(upperoffp, retupperoffp+1, num_row);
  lower.val.resize(lowernnz);
  lower.idx.resize(lowernnz);
  lower.off.resize(num_row+1);
  auto retlowervalp = lower.val.data();
  auto retloweridxp = lower.idx.data();
  auto retloweroffp = lower.off.data();
  for(size_t i = 0; i < lowernnz; i++) {
    retlowervalp[i] = lowervalp[i];
    retloweridxp[i] = loweridxp[i];
  }
  prefix_sum(loweroffp, retloweroffp+1, num_row);
}

// mostly set_intersection
template <class T>
void elementwise_product_helper(size_t* leftidx, size_t* rightidx,
                                T* leftval, T* rightval,
                                size_t left_size, size_t right_size,
                                size_t* outidx, T* outval,
                                std::vector<size_t>& retidx,
                                std::vector<T>& retval) {
  int valid[SPGEMM_VLEN];
  for(int i = 0; i < SPGEMM_VLEN; i++) valid[i] = true;
  size_t each = ceil_div(left_size, size_t(SPGEMM_VLEN));
  if(each % 2 == 0) each++; // we assume each != 0
  size_t left_ridx[SPGEMM_VLEN];
  size_t right_ridx[SPGEMM_VLEN];
  size_t left_ridx_stop[SPGEMM_VLEN];
  size_t right_ridx_stop[SPGEMM_VLEN];
  size_t out_ridx[SPGEMM_VLEN];
  size_t out_ridx_save[SPGEMM_VLEN];
  for(int i = 0; i < SPGEMM_VLEN; i++) {
    size_t pos = each * i;
    if(pos < left_size) {
      left_ridx[i] = pos;
      out_ridx[i] = pos;
      out_ridx_save[i] = pos;
    } else {
      valid[i] = false;
      left_ridx[i] = left_size;
      out_ridx[i] = left_size;
      out_ridx_save[i] = left_size;
    }
  }
  for(int i = 0; i < SPGEMM_VLEN - 1; i++) {
    left_ridx_stop[i] = left_ridx[i + 1];
  }
  left_ridx_stop[SPGEMM_VLEN-1] = left_size;
  for(int i = 0; i < SPGEMM_VLEN; i++) {
    if(valid[i]) {
      auto it = std::lower_bound(rightidx, rightidx + right_size,
                                 leftidx[left_ridx[i]]);
      if(it != rightidx + right_size) {
        right_ridx[i] = it - rightidx;
      } else {
        valid[i] = false;
        right_ridx[i] = right_size;
      }
    } else {
      right_ridx[i] = right_size;
    }
  }
  for(int i = 0; i < SPGEMM_VLEN - 1; i++) {
    right_ridx_stop[i] = right_ridx[i + 1];
  }
  right_ridx_stop[SPGEMM_VLEN-1] = right_size;
  // search for better stop
  for(int i = 0; i < SPGEMM_VLEN-1; i++) {
    if(valid[i]) {
      auto it = std::upper_bound(rightidx, rightidx + right_size,
                                 leftidx[left_ridx[i+1]-1]);
      if(it != rightidx + right_size) {
        right_ridx_stop[i] = it - rightidx;
      }
    }
  }
  if(valid[SPGEMM_VLEN-1]) {
    auto it = std::upper_bound(rightidx, rightidx + right_size,
                               leftidx[left_size-1]);
    if(it != rightidx + right_size) {
      right_ridx_stop[SPGEMM_VLEN-1] = it - rightidx;
    }
  }
  for(int i = 0; i < SPGEMM_VLEN; i++) {
    if(right_ridx[i] == right_ridx_stop[i]) valid[i] = false;
  }

  while(1) {
    size_t leftelm[SPGEMM_VLEN];
    size_t rightelm[SPGEMM_VLEN];
    // TODO: use vreg
#pragma cdir nodep
#pragma _NEC ivdep
    for(int i = 0; i < SPGEMM_VLEN; i++) {
      if(valid[i]) {
        leftelm[i] = leftidx[left_ridx[i]];
        rightelm[i] = rightidx[right_ridx[i]];
        int eq = leftelm[i] == rightelm[i];
        int lt = leftelm[i] < rightelm[i];
        if(eq) {
          outidx[out_ridx[i]] = leftelm[i];
          outval[out_ridx[i]] =
            leftval[left_ridx[i]] * rightval[right_ridx[i]];
          out_ridx[i]++;
        }
        if(eq || lt) {
          left_ridx[i]++;
        }
        if(eq || !lt) {
          right_ridx[i]++;
        }
        if(left_ridx[i] == left_ridx_stop[i] ||
           right_ridx[i] == right_ridx_stop[i]) {
          valid[i] = false;
        }
      }
    }
    int any_valid = false;
    for(int i = 0; i < SPGEMM_VLEN; i++) {
      if(valid[i]) any_valid = true;
    }
    if(any_valid == false) break;
  }
  size_t total = 0;
  for(size_t i = 0; i < SPGEMM_VLEN; i++) {
    total += out_ridx[i] - out_ridx_save[i];
  }
  retidx.resize(total);
  retval.resize(total);
  auto retidxp = retidx.data();
  auto retvalp = retval.data();
  size_t current = 0;
  for(size_t i = 0; i < SPGEMM_VLEN; i++) {
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t j = 0; j < out_ridx[i] - out_ridx_save[i]; j++) {
      retidxp[current + j] = outidx[out_ridx_save[i] + j];
      retvalp[current + j] = outval[out_ridx_save[i] + j];
    }
    current += out_ridx[i] - out_ridx_save[i];
  }
}

// used for triangle counting
// it is assumed that sparsity pattern is different
template <class T, class I, class O>
crs_matrix_local<T,I,O> elementwise_product(crs_matrix_local<T,I,O>& left,
                                            crs_matrix_local<T,I,O>& right) {
  auto num_row = left.local_num_row;
  auto num_col = left.local_num_col;
  if(num_row != right.local_num_row || num_col != right.local_num_col)
    throw std::runtime_error("elementwise_product: size mismatch");
  crs_matrix_local<T,I,O> ret(num_row, num_col);
  auto leftnnz = left.val.size();
  auto leftvalp = left.val.data();
  auto leftidxp = left.idx.data();
  auto leftoffp = left.off.data();
  auto rightnnz = right.val.size();
  auto rightvalp = right.val.data();
  auto rightidxp = right.idx.data();
  auto rightoffp = right.off.data();
  if(leftnnz == 0 || rightnnz == 0) return ret;
  
  std::vector<size_t> leftidxtmp(leftnnz);
  auto leftidxtmpp = leftidxtmp.data();
  std::vector<size_t> rightidxtmp(rightnnz);
  auto rightidxtmpp = rightidxtmp.data();

  auto max_idx_bits = max_bits(num_col-1);

  for(size_t r = 0; r < num_row; r++) {
    auto off = leftoffp[r];
    auto size = leftoffp[r+1] - off;
    auto crnt_leftidxtmp = leftidxtmpp + off;
    auto crnt_leftidx = leftidxp + off;
    for(size_t i = 0; i < size; i++) {
      crnt_leftidxtmp[i] = crnt_leftidx[i] + (r << max_idx_bits);
    }
  }

  for(size_t r = 0; r < num_row; r++) {
    auto off = rightoffp[r];
    auto size = rightoffp[r+1] - off;
    auto crnt_rightidxtmp = rightidxtmpp + off;
    auto crnt_rightidx = rightidxp + off;
    for(size_t i = 0; i < size; i++) {
      crnt_rightidxtmp[i] = crnt_rightidx[i] + (r << max_idx_bits);
    }
  }

  auto tmpretnnz = leftnnz > rightnnz ? leftnnz : rightnnz;
  std::vector<size_t> tmpretidx(tmpretnnz);
  auto tmpretidxp = tmpretidx.data();
  std::vector<T> tmpretval(tmpretnnz);
  auto tmpretvalp = tmpretval.data();
  std::vector<size_t> tmpretidx2;
  elementwise_product_helper(leftidxtmpp, rightidxtmpp,
                             leftvalp, rightvalp,
                             leftnnz, rightnnz,
                             tmpretidxp, tmpretvalp,
                             tmpretidx2, ret.val);
  auto retsize = tmpretidx2.size();
  ret.idx.resize(retsize);
  std::vector<int> extracted_idx(retsize);
  auto extracted_idxp = extracted_idx.data();
  extract_column_idx(tmpretidx2.data(), extracted_idxp, ret.idx.data(),
                    retsize, max_idx_bits);
  auto separated = my_set_separate(extracted_idx);
  // might be smaller than num_row if sparse vector is zero
  auto separated_size = separated.size();
  auto separatedp = separated.data();
  std::vector<O> merged_off(num_row);
  auto merged_offp = merged_off.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < separated_size - 1; i++) {
    merged_offp[extracted_idxp[separatedp[i]]] =
      separatedp[i+1] - separatedp[i];
  }
  ret.off.resize(num_row+1);
  auto retoffp = ret.off.data();
  prefix_sum(merged_offp, retoffp+1, num_row);

  return ret;
}

// separate left according to interim nnz
template <class T, class I, class O>
std::vector<crs_matrix_local<T,I,O>> 
separate_crs_matrix_for_spgemm(crs_matrix_local<T,I,O>& left,
                               crs_matrix_local<T,I,O>& right,
                               size_t separate_size) {
  if(left.local_num_col != right.local_num_row) 
    throw std::runtime_error
      ("separate_crs_matrix_for_spgemm: matrix size mismatch");

  auto lnum_row = left.local_num_row;
  auto lidx = left.idx.data();
  auto loff = left.off.data();
  auto roff = right.off.data();
  std::vector<O> interim_nnz_per_column(lnum_row);
  auto interim_nnz_per_columnp = interim_nnz_per_column.data();
  for(size_t lr = 0; lr < lnum_row; lr++) { // lr: left row
    auto crnt_lidx = lidx + loff[lr];
    auto crnt_nnz = loff[lr+1] - loff[lr];
    for(size_t i = 0; i < crnt_nnz; i++) {
      interim_nnz_per_columnp[lr] += roff[crnt_lidx[i]+1] - roff[crnt_lidx[i]];
    }
  }
  std::vector<O> pfx_interim_nnz_per_column(lnum_row+1);
  prefix_sum(interim_nnz_per_columnp, pfx_interim_nnz_per_column.data()+1,
             lnum_row);

  size_t total = pfx_interim_nnz_per_column[lnum_row];
  size_t each_size = frovedis::ceil_div(total, separate_size);
  std::vector<size_t> divide_row(separate_size+1);
  for(size_t i = 0; i < separate_size + 1; i++) {
    auto it = std::lower_bound(pfx_interim_nnz_per_column.begin(),
                               pfx_interim_nnz_per_column.end(),
                               each_size * i);
    if(it != pfx_interim_nnz_per_column.end()) {
      divide_row[i] = it - pfx_interim_nnz_per_column.begin();
    } else {
      divide_row[i] = lnum_row;
    }
  }

  std::vector<crs_matrix_local<T,I,O>> vret(separate_size);
  T* leftvalp = left.val.data();
  I* leftidxp = left.idx.data();
  O* leftoffp = left.off.data();
  for(size_t i = 0; i < separate_size; i++) {
    vret[i].local_num_col = left.local_num_col;
    size_t start_row = divide_row[i];
    size_t end_row = divide_row[i+1];
    vret[i].local_num_row = end_row - start_row;
    size_t start_off = leftoffp[start_row];
    size_t end_off = leftoffp[end_row];
    size_t off_size = end_off - start_off;
    vret[i].val.resize(off_size);
    vret[i].idx.resize(off_size);
    vret[i].off.resize(end_row - start_row + 1); // off[0] == 0 by ctor
    T* valp = &vret[i].val[0];
    I* idxp = &vret[i].idx[0];
    O* offp = &vret[i].off[0];
    for(size_t j = 0; j < off_size; j++) {
      valp[j] = leftvalp[j + start_off];
      idxp[j] = leftidxp[j + start_off];
    }
    for(size_t j = 0; j < end_row - start_row; j++) {
      offp[j+1] = offp[j] + (leftoffp[start_row + j + 1] -
                             leftoffp[start_row + j]);
    }
  }
  return vret;
}

// assume that left and right is broadcasted,
// returns only my part of left
template <class T, class I, class O>
crs_matrix_local<T,I,O>
separate_crs_matrix_for_spgemm_mpi(crs_matrix_local<T,I,O>& left,
                                   crs_matrix_local<T,I,O>& right) {
  if(left.local_num_col != right.local_num_row) 
    throw std::runtime_error
      ("separate_crs_matrix_for_spgemm: matrix size mismatch");

  int size, rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // first, separate according to nnz for interm nnz calculation
  size_t total_nnz = left.off[left.off.size() - 1];
  size_t each_size_nnz = frovedis::ceil_div(total_nnz, size_t(size));
  std::vector<size_t> divide_row_nnz(size+1);
  for(size_t i = 0; i < size + 1; i++) {
    auto it = std::lower_bound(left.off.begin(), left.off.end(),
                               each_size_nnz * i);
    if(it != left.off.end()) {
      divide_row_nnz[i] = it - left.off.begin();
    } else {
      divide_row_nnz[i] = left.local_num_row;
    }
  }
  auto my_row_start = divide_row_nnz[rank];
  auto my_row_end = divide_row_nnz[rank+1];
  auto my_num_row = my_row_end - my_row_start;
  auto lidx = left.idx.data();
  auto loff = left.off.data();
  auto roff = right.off.data();
  std::vector<O> interim_nnz_per_column(my_num_row);
  auto interim_nnz_per_columnp = interim_nnz_per_column.data();
  for(size_t j = 0; j < my_num_row; j++) { 
    auto lr = my_row_start + j; // lr: left row
    auto crnt_lidx = lidx + loff[lr];
    auto crnt_nnz = loff[lr+1] - loff[lr];
    for(size_t i = 0; i < crnt_nnz; i++) {
      interim_nnz_per_columnp[j] += roff[crnt_lidx[i]+1] - roff[crnt_lidx[i]];
    }
  }
  std::vector<O> pfx_interim_nnz_per_column_local(my_num_row);
  prefix_sum(interim_nnz_per_columnp, pfx_interim_nnz_per_column_local.data(),
             my_num_row);

  auto lnum_row = left.local_num_row;
  std::vector<O> pfx_interim_nnz_per_column(lnum_row+1);

  std::vector<int> recvcounts(size);
  std::vector<int> displs(size);
  auto recvcountsp = recvcounts.data();
  auto displsp = displs.data();
  auto divide_row_nnzp = divide_row_nnz.data();
  for(size_t i = 0; i < size; i++)
    recvcountsp[i] = (divide_row_nnzp[i+1] - divide_row_nnzp[i]) * sizeof(O);
  for(size_t i = 0; i < size-1; i++)
    displsp[i+1] = displsp[i] + recvcountsp[i];
  MPI_Allgatherv(pfx_interim_nnz_per_column_local.data(),
                 my_num_row * sizeof(O), MPI_CHAR,
                 pfx_interim_nnz_per_column.data()+1,
                 recvcountsp, displsp, MPI_CHAR, MPI_COMM_WORLD);
  
  auto pfx_interim_nnz_per_columnp = pfx_interim_nnz_per_column.data();
  for(size_t i = 1; i < size; i++) {
    auto to_add = pfx_interim_nnz_per_columnp[divide_row_nnzp[i]];
    for(size_t j = divide_row_nnzp[i]; j < divide_row_nnzp[i+1]; j++) {
      pfx_interim_nnz_per_columnp[j+1] += to_add;
    }
  }

  size_t total = pfx_interim_nnz_per_column[lnum_row];
  size_t each_size = frovedis::ceil_div(total, size_t(size));
  std::vector<size_t> divide_row(size+1);
  for(size_t i = 0; i < size + 1; i++) {
    auto it = std::lower_bound(pfx_interim_nnz_per_column.begin(),
                               pfx_interim_nnz_per_column.end(),
                               each_size * i);
    if(it != pfx_interim_nnz_per_column.end()) {
      divide_row[i] = it - pfx_interim_nnz_per_column.begin();
    } else {
      divide_row[i] = lnum_row;
    }
  }
  crs_matrix_local<T,I,O> ret;
  T* leftvalp = left.val.data();
  I* leftidxp = left.idx.data();
  O* leftoffp = left.off.data();

  ret.local_num_col = left.local_num_col;
  size_t start_row = divide_row[rank];
  size_t end_row = divide_row[rank+1];
  ret.local_num_row = end_row - start_row;
  size_t start_off = leftoffp[start_row];
  size_t end_off = leftoffp[end_row];
  size_t off_size = end_off - start_off;
  ret.val.resize(off_size);
  ret.idx.resize(off_size);
  ret.off.resize(end_row - start_row + 1); // off[0] == 0 by ctor
  T* valp = ret.val.data();
  I* idxp = ret.idx.data();
  O* offp = ret.off.data();
  for(size_t i = 0; i < off_size; i++) {
    valp[i] = leftvalp[i + start_off];
    idxp[i] = leftidxp[i + start_off];
  }
  for(size_t i = 0; i < end_row - start_row; i++) {
    offp[i+1] = offp[i] + (leftoffp[start_row + i + 1] -
                           leftoffp[start_row + i]);
  }

  return ret;
}

}
#endif
