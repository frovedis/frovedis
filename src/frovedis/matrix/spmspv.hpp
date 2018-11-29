#ifndef SPMSPV_HPP
#define SPMSPV_HPP

#include "ccs_matrix.hpp"

#define SPMSPV_VLEN 256

#include "../core/prefix_sum.hpp"
#include "../core/radix_sort.hpp"

namespace frovedis {

template <class K, class V>
void groupby_sum(const std::vector<K>& key, const std::vector<V>& val,
                 std::vector<K>& outkey, std::vector<V>& outval) {
  size_t size = key.size();
  if(size == 0) {
    outkey.resize(0); outval.resize(0);
    return;
  }
  const K* keyp = key.data();
  const V* valp = val.data();
  std::vector<K> outkeytmp;
  std::vector<V> outvaltmp;
  outkeytmp.resize(size);
  outvaltmp.resize(size);
  K* outkeytmpp = outkeytmp.data();
  V* outvaltmpp = outvaltmp.data();
  size_t each = size / SPMSPV_VLEN; // maybe 0
  if(each % 2 == 0 && each > 1) each--;
  size_t rest = size - each * SPMSPV_VLEN;
  size_t out_ridx[SPMSPV_VLEN];
  for(size_t i = 0; i < SPMSPV_VLEN; i++) {
    out_ridx[i] = each * i;
  }
  if(each == 0) {
    // rest is not zero, sicne size is not zero
    auto current_key_rest = key[0];
    auto current_val_rest = valp[0];
    size_t rest_idx = 0;
    // no vector loop
    for(size_t j = 1; j < rest; j++) {
      auto loaded_key_rest = keyp[j]; 
      auto loaded_val_rest = valp[j]; 
      if(loaded_key_rest != current_key_rest) {
        outkeytmpp[rest_idx] = current_key_rest;
        outvaltmpp[rest_idx] = current_val_rest;
        rest_idx++;
        current_key_rest = loaded_key_rest;
        current_val_rest = loaded_val_rest;
      } else {
        current_val_rest += loaded_val_rest;
      }
    }
    outkeytmpp[rest_idx] = current_key_rest;
    outvaltmpp[rest_idx] = current_val_rest;
    rest_idx++;
    size_t total = rest_idx;
    outkey.resize(total);
    outval.resize(total);
    K* outkeyp = outkey.data();
    V* outvalp = outval.data();
    for(size_t j = 0; j < rest_idx; j++) {
      outkeyp[j] = outkeytmpp[j];
      outvalp[j] = outvaltmpp[j];
    }
  } else {
    K current_key[SPMSPV_VLEN];
    V current_val[SPMSPV_VLEN];
    // load 1st element
    for(size_t i = 0; i < SPMSPV_VLEN; i++) {
      current_key[i] = keyp[each * i];
      current_val[i] = valp[each * i];
    }
    for(size_t j = 1; j < each; j++) {
#pragma cdir nodep
#pragma _NEC ivdep
      for(size_t i = 0; i < SPMSPV_VLEN; i++) {
        auto loaded_key = keyp[j + each * i];
        auto loaded_val = valp[j + each * i];
        if(loaded_key != current_key[i]) {
          outkeytmpp[out_ridx[i]] = current_key[i];
          outvaltmpp[out_ridx[i]] = current_val[i];
          out_ridx[i]++;
          current_key[i] = loaded_key;
          current_val[i] = loaded_val;
        } else {
          current_val[i] += loaded_val;
        }
      }
    }
    size_t rest_idx = 0;
    if(rest != 0) {
      size_t rest_idx_start = each * SPMSPV_VLEN;
      rest_idx = rest_idx_start;
      auto current_key_rest = keyp[rest_idx_start];
      auto current_val_rest = valp[rest_idx_start];
      // no vector loop
      for(size_t j = 1; j < rest; j++) {
        auto loaded_key_rest = keyp[j + rest_idx_start]; 
        auto loaded_val_rest = valp[j + rest_idx_start]; 
        if(loaded_key_rest != current_key_rest) {
          outkeytmpp[rest_idx] = current_key_rest;
          outvaltmpp[rest_idx] = current_val_rest;
          rest_idx++;
          current_key_rest = loaded_key_rest;
          current_val_rest = loaded_val_rest;
        } else {
          current_val_rest += loaded_val_rest;
        }
      }
      outkeytmpp[rest_idx] = current_key_rest;
      outvaltmpp[rest_idx] = current_val_rest;
      rest_idx++;
    }
    // no vector loop
    for(size_t i = 0; i < SPMSPV_VLEN; i++) {
      if(current_key[i] == keyp[each * (i+1)]) {
        // still working...
        if(i != SPMSPV_VLEN - 1 && current_key[i] == current_key[i+1])
          current_val[i+1] += current_val[i];
        else
          outvaltmpp[each * (i+1)] += current_val[i];
      } else {
        outkeytmpp[out_ridx[i]] = current_key[i];
        outvaltmpp[out_ridx[i]] = current_val[i];
        out_ridx[i]++;        
      }
    }
    size_t sizes[SPMSPV_VLEN];
    for(size_t i = 0; i < SPMSPV_VLEN; i++) {
      sizes[i] = out_ridx[i] - each * i;
    }
    size_t total = 0;
    for(size_t i = 0; i < SPMSPV_VLEN; i++) {
      total += sizes[i];
    }
    size_t rest_size = rest_idx - each * SPMSPV_VLEN;
    total += rest_size;
    outkey.resize(total);
    outval.resize(total);
    K* outkeyp = outkey.data();
    V* outvalp = outval.data();
    size_t current = 0;
    for(size_t i = 0; i < SPMSPV_VLEN; i++) {
      for(size_t j = 0; j < sizes[i]; j++) {
        outkeyp[current + j] = outkeytmpp[each * i + j];
        outvalp[current + j] = outvaltmpp[each * i + j];
      }
      current += sizes[i];
    }
    // rest (and rest_idx) maybe 0
    for(size_t j = 0; j < rest_size; j++) {
      outkeyp[current + j] = outkeytmpp[each * SPMSPV_VLEN + j];
      outvalp[current + j] = outvaltmpp[each * SPMSPV_VLEN + j];
    }
  }
}

template <class T, class I, class O>
void spmspv_mul(ccs_matrix_local<T,I,O>& mat,
                sparse_vector<T,I>& sv,
                std::vector<T>& mulout_val,
                std::vector<I>& mulout_idx) {
  T* mat_valp = mat.val.data();
  I* mat_idxp = mat.idx.data();
  O* mat_offp = mat.off.data();
  T* sv_valp = sv.val.data();
  I* sv_idxp = sv.idx.data();
  size_t sv_size = sv.val.size();
  std::vector<O> nnz(sv_size);
  O* nnzp = nnz.data();
  for(size_t i = 0; i < sv_size; i++) {
    nnzp[i] = mat_offp[sv_idxp[i]+1] - mat_offp[sv_idxp[i]];
  }
  auto pfx_sum_nnz = prefix_sum(nnz);
  auto pfx_sum_nnzp = pfx_sum_nnz.data();
  auto total_nnz = pfx_sum_nnz[sv_size - 1];
  auto each = ceil_div(total_nnz, O(SPMSPV_VLEN));
  if(each % 2 == 0) each++;
  I svpos_ridx[SPMSPV_VLEN]; // ridx: idx for raking
//#pragma _NEC vreg(svpos_ridx)
  int valid[SPMSPV_VLEN];
//#pragma _NEC vreg(valid)
  for(size_t i = 0; i < SPMSPV_VLEN; i++) valid[i] = true;
  auto begin_it = pfx_sum_nnz.begin();
  auto end_it = pfx_sum_nnz.end();
  auto current_it = begin_it;
  svpos_ridx[0] = 0;
  size_t ii = 1;
  for(size_t i = 1; i < SPMSPV_VLEN; i++) {
    auto it = std::lower_bound(current_it, end_it, each * i);
    if(it == current_it) continue;
    else if(it == end_it) break;
    else {
      svpos_ridx[ii++] = it - begin_it;
      current_it = it;
    }
  }
  for(size_t i = ii; i < SPMSPV_VLEN; i++) {
    valid[i] = false;
    svpos_ridx[i] = sv_size;
  }
  mulout_val.resize(total_nnz);
  mulout_idx.resize(total_nnz);
  T* mulout_valp = mulout_val.data();
  I* mulout_idxp = mulout_idx.data();
  I muloutpos_ridx[SPMSPV_VLEN];
//#pragma _NEC vreg(muloutpos_ridx)
  muloutpos_ridx[0] = 0;
  for(size_t i = 1; i < SPMSPV_VLEN; i++) {
    if(valid[i]) muloutpos_ridx[i] = pfx_sum_nnzp[svpos_ridx[i]-1];
    else muloutpos_ridx[i] = total_nnz;
  }
  I muloutpos_stop_ridx[SPMSPV_VLEN];
//#pragma _NEC vreg(muloutpos_stop_ridx)
  for(int i = 0; i < SPMSPV_VLEN - 1; i++) {
    muloutpos_stop_ridx[i] = muloutpos_ridx[i + 1];
  }
  muloutpos_stop_ridx[SPMSPV_VLEN-1] = total_nnz;
  size_t muloutpos_ridx_size[SPMSPV_VLEN];
  for(int i = 0; i < SPMSPV_VLEN; i++) {
    muloutpos_ridx_size[i] = muloutpos_stop_ridx[i] - muloutpos_ridx[i];
  }
  size_t max_size = 0;
  for(size_t i = 0; i < SPMSPV_VLEN; i++) {
    if(max_size < muloutpos_ridx_size[i]) max_size = muloutpos_ridx_size[i];
  }
  O matpos_ridx[SPMSPV_VLEN];
//#pragma _NEC vreg(matpos_ridx)
  O matpos_stop_ridx[SPMSPV_VLEN];
//#pragma _NEC vreg(matpos_stop_ridx)
  T current_sv_val_ridx[SPMSPV_VLEN];
//#pragma _NEC vreg(current_sv_val_ridx)
  for(size_t i = 0; i < SPMSPV_VLEN; i++) {
    if(valid[i]) {
      matpos_ridx[i] = mat_offp[sv_idxp[svpos_ridx[i]]];
      matpos_stop_ridx[i] = mat_offp[sv_idxp[svpos_ridx[i]]+1];
      current_sv_val_ridx[i] = sv_valp[svpos_ridx[i]];
    }
  }
  for(size_t j = 0; j < max_size; j++) {
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t i = 0; i < SPMSPV_VLEN; i++) {
      if(valid[i]) {
        mulout_valp[muloutpos_ridx[i]] =
          mat_valp[matpos_ridx[i]] * current_sv_val_ridx[i];
        mulout_idxp[muloutpos_ridx[i]] = mat_idxp[matpos_ridx[i]];
        muloutpos_ridx[i]++;
        matpos_ridx[i]++;
        if(muloutpos_ridx[i] == muloutpos_stop_ridx[i]) {
          valid[i] = false;
        } else if(matpos_ridx[i] == matpos_stop_ridx[i]) {
          svpos_ridx[i]++;
          matpos_ridx[i] = mat_offp[sv_idxp[svpos_ridx[i]]];
          matpos_stop_ridx[i] = mat_offp[sv_idxp[svpos_ridx[i]]+1];
          current_sv_val_ridx[i] = sv_valp[svpos_ridx[i]];
        }
      }
    }
  }
}

template <class T, class I, class O>
void spmspv(ccs_matrix_local<T,I,O>& mat, sparse_vector<T,I>& sv,
            sparse_vector<T,I>& ret) {
  std::vector<T> mulout_val;
  std::vector<I> mulout_idx;
  spmspv_mul(mat, sv, mulout_val, mulout_idx);
  radix_sort(mulout_idx.data(), mulout_val.data(), mulout_idx.size());
  groupby_sum(mulout_idx, mulout_val, ret.idx, ret.val);
  ret.size = mat.local_num_row;
}

// retp should point to allocated memory area; result is accumulated
template <class T, class I, class O>
void spmspv(ccs_matrix_local<T,I,O>& mat, sparse_vector<T,I>& sv,
            T* retp) {
  std::vector<T> mulout_val;
  std::vector<I> mulout_idx;
  spmspv_mul(mat, sv, mulout_val, mulout_idx);
  T* mulout_valp = mulout_val.data();
  I* mulout_idxp = mulout_idx.data();

  size_t total_nnz = mulout_val.size();
  std::vector<T> mulout_val_tmp(total_nnz);
  std::vector<I> mulout_idx_tmp(total_nnz);
  T* mulout_val_tmpp = mulout_val.data();
  I* mulout_idx_tmpp = mulout_idx.data();
  size_t num_row = mat.local_num_row;
  std::vector<I> idx_checker(num_row);
  std::vector<size_t> missed(total_nnz);
  size_t* missedp = &missed[0];
  size_t missed_idx = 0;
  I* idx_checkerp = idx_checker.data();
  size_t block_size = total_nnz / SPMSPV_VLEN;
  size_t remain_size = total_nnz % SPMSPV_VLEN;
  while(1) {
    for(size_t b = 0; b < block_size; b++) {
      size_t offset = b * SPMSPV_VLEN;
#pragma cdir nodep
#pragma _NEC ivdep
      for(size_t ii = 0; ii < SPMSPV_VLEN; ii++) {
        size_t i = offset + ii;
        idx_checkerp[mulout_idxp[i]] = i;
      }
#pragma cdir nodep
#pragma _NEC ivdep
      for(size_t ii = 0; ii < SPMSPV_VLEN; ii++) {
        size_t i = offset + ii;
        if(idx_checkerp[mulout_idxp[i]] == i) {
          retp[mulout_idxp[i]] += mulout_valp[i];
        } else {
          missedp[missed_idx++] = i;      
        }
      }
    }
    size_t offset = block_size * SPMSPV_VLEN;
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t ii = 0; ii < remain_size; ii++) {
      size_t i =  offset + ii;
      idx_checkerp[mulout_idxp[i]] = i;
    }
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t ii = 0; ii < remain_size; ii++) {
      size_t i = offset + ii;
      if(idx_checkerp[mulout_idxp[i]] == i) {
        retp[mulout_idxp[i]] += mulout_valp[i];
      } else {
        missedp[missed_idx++] = i;      
      }
    }
    if(missed_idx == 0) break;
    else {
      for(size_t i = 0; i < missed_idx; i++) {
        mulout_val_tmpp[i] = mulout_valp[missedp[i]];
        mulout_idx_tmpp[i] = mulout_idxp[missedp[i]];
      }
      for(size_t i = 0; i < missed_idx; i++) {
        mulout_valp[i] = mulout_val_tmpp[i];
        mulout_idxp[i] = mulout_idx_tmpp[i];
      }
      block_size = missed_idx / SPMSPV_VLEN;
      remain_size = missed_idx % SPMSPV_VLEN;
      missed_idx = 0;
    }
  }
}

template <class T, class I, class O>
void spmspv(ccs_matrix_local<T,I,O>& mat, sparse_vector<T,I>& sv,
            std::vector<T>& ret) {
  ret.resize(mat.local_num_row);
  spmspv(mat, sv, ret.data());
}

}
#endif
