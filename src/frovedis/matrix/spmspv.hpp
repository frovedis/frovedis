#ifndef SPMSPV_HPP
#define SPMSPV_HPP

#include "ccs_matrix.hpp"

#ifdef __ve__
#define SPMSPV_VLEN 256
#define SPMSPV_MUL_THR 8
#else
#define SPMSPV_VLEN 1
#define SPMSPV_MUL_THR 0
#endif

#include "../core/prefix_sum.hpp"
#include "../core/radix_sort.hpp"
#include <limits>

namespace frovedis {

template <class K, class V>
void groupby_sum(const K* keyp, const V* valp, size_t size,
                 std::vector<K>& outkey, std::vector<V>& outval) {
  if(size == 0) {
    outkey.resize(0); outval.resize(0);
    return;
  }
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
    auto current_key_rest = keyp[0];
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
    size_t out_ridx_vreg[SPMSPV_VLEN];
#pragma _NEC vreg(out_ridx_vreg)
    for(size_t i = 0; i < SPMSPV_VLEN; i++)
      out_ridx_vreg[i] = out_ridx[i];

    for(size_t j = 1; j < each; j++) {
#pragma cdir nodep
#pragma _NEC ivdep
      for(size_t i = 0; i < SPMSPV_VLEN; i++) {
        auto loaded_key = keyp[j + each * i];
        auto loaded_val = valp[j + each * i];
        if(loaded_key != current_key[i]) {
          outkeytmpp[out_ridx_vreg[i]] = current_key[i];
          outvaltmpp[out_ridx_vreg[i]] = current_val[i];
          out_ridx_vreg[i]++;
          current_key[i] = loaded_key;
          current_val[i] = loaded_val;
        } else {
          current_val[i] += loaded_val;
        }
      }
    }
    for(size_t i = 0; i < SPMSPV_VLEN; i++)
      out_ridx[i] = out_ridx_vreg[i];

    size_t rest_idx = each * SPMSPV_VLEN;
    if(rest != 0) {
      size_t rest_idx_start = rest_idx;
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

template <class K, class V>
void groupby_sum_binary_input(const K* keyp, size_t size,
                              std::vector<K>& outkey, std::vector<V>& outval) {
  if(size == 0) {
    outkey.resize(0); outval.resize(0);
    return;
  }
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
    auto current_key_rest = keyp[0];
    auto current_val_rest = 0;
    size_t rest_idx = 0;
    // no vector loop
    for(size_t j = 1; j < rest; j++) {
      auto loaded_key_rest = keyp[j]; 
      if(loaded_key_rest != current_key_rest) {
        outkeytmpp[rest_idx] = current_key_rest;
        outvaltmpp[rest_idx] = current_val_rest;
        rest_idx++;
        current_key_rest = loaded_key_rest;
        current_val_rest = 1;
      } else {
        current_val_rest++;
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
      current_val[i] = 1;
    }
    size_t out_ridx_vreg[SPMSPV_VLEN];
#pragma _NEC vreg(out_ridx_vreg)
    for(size_t i = 0; i < SPMSPV_VLEN; i++)
      out_ridx_vreg[i] = out_ridx[i];

    for(size_t j = 1; j < each; j++) {
#pragma cdir nodep
#pragma _NEC ivdep
      for(size_t i = 0; i < SPMSPV_VLEN; i++) {
        auto loaded_key = keyp[j + each * i];
        if(loaded_key != current_key[i]) {
          outkeytmpp[out_ridx_vreg[i]] = current_key[i];
          outvaltmpp[out_ridx_vreg[i]] = current_val[i];
          out_ridx_vreg[i]++;
          current_key[i] = loaded_key;
          current_val[i] = 1;
        } else {
          current_val[i]++;
        }
      }
    }
    for(size_t i = 0; i < SPMSPV_VLEN; i++)
      out_ridx[i] = out_ridx_vreg[i];

    size_t rest_idx = each * SPMSPV_VLEN;
    if(rest != 0) {
      size_t rest_idx_start = rest_idx;
      auto current_key_rest = keyp[rest_idx_start];
      auto current_val_rest = 1;
      // no vector loop
      for(size_t j = 1; j < rest; j++) {
        auto loaded_key_rest = keyp[j + rest_idx_start]; 
        if(loaded_key_rest != current_key_rest) {
          outkeytmpp[rest_idx] = current_key_rest;
          outvaltmpp[rest_idx] = current_val_rest;
          rest_idx++;
          current_key_rest = loaded_key_rest;
          current_val_rest = 1;
        } else {
          current_val_rest++;
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

template <class T, class I, class O, class II>
void spmspv_mul_raking(T* mat_valp, I* mat_idxp, O* mat_offp,
                       T* sv_valp, I* sv_idxp, size_t sv_size,
                       O* nnzp, O* pfx_sum_nnzp, O total_nnz,
                       T* mulout_valp, II* mulout_idxp) {
  auto each = ceil_div(total_nnz, O(SPMSPV_VLEN));
  if(each % 2 == 0) each++;
  size_t svpos_ridx[SPMSPV_VLEN]; // ridx: idx for raking
  int valid[SPMSPV_VLEN];
  for(size_t i = 0; i < SPMSPV_VLEN; i++) valid[i] = true;
  auto begin_it = pfx_sum_nnzp;
  auto end_it = pfx_sum_nnzp + sv_size;
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
  O muloutpos_ridx[SPMSPV_VLEN];
  muloutpos_ridx[0] = 0;
  for(size_t i = 1; i < SPMSPV_VLEN; i++) {
    if(valid[i]) muloutpos_ridx[i] = pfx_sum_nnzp[svpos_ridx[i]-1];
    else muloutpos_ridx[i] = total_nnz;
  }
  O muloutpos_stop_ridx[SPMSPV_VLEN];
  for(int i = 0; i < SPMSPV_VLEN - 1; i++) {
    muloutpos_stop_ridx[i] = muloutpos_ridx[i + 1];
  }
  muloutpos_stop_ridx[SPMSPV_VLEN-1] = total_nnz;
  O matpos_ridx[SPMSPV_VLEN];
  O matpos_stop_ridx[SPMSPV_VLEN];
  T current_sv_val_ridx[SPMSPV_VLEN];
  for(size_t i = 0; i < SPMSPV_VLEN; i++) {
    if(valid[i]) {
      matpos_ridx[i] = mat_offp[sv_idxp[svpos_ridx[i]]];
      matpos_stop_ridx[i] = mat_offp[sv_idxp[svpos_ridx[i]]+1];
      current_sv_val_ridx[i] = sv_valp[svpos_ridx[i]];
    }
  }
  int valid_vreg[SPMSPV_VLEN];
  O muloutpos_ridx_vreg[SPMSPV_VLEN];
  O muloutpos_stop_ridx_vreg[SPMSPV_VLEN];
  O matpos_ridx_vreg[SPMSPV_VLEN];
  O matpos_stop_ridx_vreg[SPMSPV_VLEN];
  size_t svpos_ridx_vreg[SPMSPV_VLEN];
  T current_sv_val_ridx_vreg[SPMSPV_VLEN];
#pragma _NEC vreg(valid_vreg)
#pragma _NEC vreg(muloutpos_ridx_vreg)
#pragma _NEC vreg(muloutpos_stop_ridx_vreg)
#pragma _NEC vreg(matpos_ridx_vreg)
#pragma _NEC vreg(matpos_stop_ridx_vreg)
#pragma _NEC vreg(svpos_ridx_vreg)
#pragma _NEC vreg(current_sv_val_ridx_vreg)
  for(size_t i = 0; i < SPMSPV_VLEN; i++) {
    valid_vreg[i] = valid[i];
    muloutpos_ridx_vreg[i] = muloutpos_ridx[i];
    muloutpos_stop_ridx_vreg[i] = muloutpos_stop_ridx[i];
    matpos_ridx_vreg[i] = matpos_ridx[i];
    matpos_stop_ridx_vreg[i] = matpos_stop_ridx[i];
    svpos_ridx_vreg[i] = svpos_ridx[i];
    current_sv_val_ridx_vreg[i] = current_sv_val_ridx[i];
  }
  int anyvalid = true;
  while(anyvalid) {
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t i = 0; i < SPMSPV_VLEN; i++) {
      if(valid_vreg[i]) {
        if(muloutpos_ridx_vreg[i] == muloutpos_stop_ridx_vreg[i]) {
          valid_vreg[i] = false;
        } else if(matpos_ridx_vreg[i] == matpos_stop_ridx_vreg[i]) {
          svpos_ridx_vreg[i]++;
          matpos_ridx_vreg[i] = mat_offp[sv_idxp[svpos_ridx_vreg[i]]];
          matpos_stop_ridx_vreg[i] = mat_offp[sv_idxp[svpos_ridx_vreg[i]]+1];
          current_sv_val_ridx_vreg[i] = sv_valp[svpos_ridx_vreg[i]];
        } else {
          mulout_valp[muloutpos_ridx_vreg[i]] =
            mat_valp[matpos_ridx_vreg[i]] * current_sv_val_ridx_vreg[i];
          mulout_idxp[muloutpos_ridx_vreg[i]] = mat_idxp[matpos_ridx_vreg[i]];
          muloutpos_ridx_vreg[i]++;
          matpos_ridx_vreg[i]++;
        }
      }
    }
    anyvalid = false;
    for(size_t i = 0; i < SPMSPV_VLEN; i++) {
      if(valid_vreg[i]) anyvalid = true;
    }
  }
}

template <class T, class I, class O, class II>
void spmspv_mul_noraking(T* mat_valp, I* mat_idxp, O* mat_offp,
                         T* sv_valp, I* sv_idxp, size_t sv_size,
                         O* nnzp, O* pfx_sum_nnzp, 
                         T* mulout_valp, II* mulout_idxp) {
  if(sv_size > 0) { // for "outoff = pfx_sum_nnzp[i-1]"
    auto matoff = mat_offp[sv_idxp[0]];
    auto svval = sv_valp[0];
    auto outoff = 0;
    for(size_t j = 0; j < nnzp[0]; j++) {
      mulout_idxp[outoff + j] = mat_idxp[matoff + j];
      mulout_valp[outoff + j] = svval * mat_valp[matoff + j];
    }
  }
  for(size_t i = 1; i < sv_size; i++) {
    auto matoff = mat_offp[sv_idxp[i]];
    auto svval = sv_valp[i];
    auto outoff = pfx_sum_nnzp[i-1];
    for(size_t j = 0; j < nnzp[i]; j++) {
      mulout_idxp[outoff + j] = mat_idxp[matoff + j];
      mulout_valp[outoff + j] = svval * mat_valp[matoff + j];
    }
  }
}

// class II is for spgemm to use size_t arrray
template <class T, class I, class O, class II>
void spmspv_mul(T* mat_valp, I* mat_idxp, O* mat_offp,
                T* sv_valp, I* sv_idxp, size_t sv_size,
                O* nnzp, O* pfx_sum_nnzp, O total_nnz,
                T* mulout_valp, II* mulout_idxp) {
  if(total_nnz / sv_size > SPMSPV_MUL_THR) {
    spmspv_mul_noraking(mat_valp, mat_idxp, mat_offp,
                        sv_valp, sv_idxp, sv_size,
                        nnzp, pfx_sum_nnzp,
                        mulout_valp, mulout_idxp);
  } else {
    spmspv_mul_raking(mat_valp, mat_idxp, mat_offp,
                      sv_valp, sv_idxp, sv_size,
                      nnzp, pfx_sum_nnzp, total_nnz,
                      mulout_valp, mulout_idxp);
  }
}

// for now, only noraking version...
template <class I, class O, class II>
void spmspv_mul_binary_input(I* mat_idxp, O* mat_offp,
                             I* sv_idxp, size_t sv_size,
                             O* nnzp, O* pfx_sum_nnzp, O total_nnz,
                             II* mulout_idxp) {
  if(sv_size > 0) { // for "outoff = pfx_sum_nnzp[i-1]"
    auto matoff = mat_offp[sv_idxp[0]];
    auto outoff = 0;
    for(size_t j = 0; j < nnzp[0]; j++) {
      mulout_idxp[outoff + j] = mat_idxp[matoff + j];
    }
  }
  for(size_t i = 1; i < sv_size; i++) {
    auto matoff = mat_offp[sv_idxp[i]];
    auto outoff = pfx_sum_nnzp[i-1];
    for(size_t j = 0; j < nnzp[i]; j++) {
      mulout_idxp[outoff + j] = mat_idxp[matoff + j];
    }
  }
}

template <class T, class I, class O>
void spmspv_impl(T* mat_valp, I* mat_idxp, O* mat_offp,
                 T* sv_valp, I* sv_idxp, size_t sv_size,
                 T* mulout_valp, I* mulout_idxp,
                 O* nnzp, O* pfx_sum_nnzp, O total_nnz,
                 sparse_vector<T,I>& ret, size_t num_row,
                 bool check_sort_max_key = true,
                 size_t max_key_size = sizeof(I)) {
  spmspv_mul(mat_valp, mat_idxp, mat_offp, sv_valp, sv_idxp, sv_size,
             nnzp, pfx_sum_nnzp, total_nnz,
             mulout_valp, mulout_idxp);
  if(check_sort_max_key) radix_sort(mulout_idxp, mulout_valp, total_nnz, true);
  else radix_sort_impl(mulout_idxp, mulout_valp, total_nnz, max_key_size);
  groupby_sum(mulout_idxp, mulout_valp, total_nnz, ret.idx, ret.val);
  ret.size = num_row;
}

template <class T, class I, class O>
void spmspv(ccs_matrix_local<T,I,O>& mat, sparse_vector<T,I>& sv,
            sparse_vector<T,I>& ret) {
  T* mat_valp = mat.val.data();
  I* mat_idxp = mat.idx.data();
  O* mat_offp = mat.off.data();
  T* sv_valp = sv.val.data();
  I* sv_idxp = sv.idx.data();
  size_t sv_size = sv.val.size();
  if(sv_size == 0) return;
  std::vector<O> nnz(sv_size);
  O* nnzp = nnz.data();
  for(size_t i = 0; i < sv_size; i++) {
    nnzp[i] = mat_offp[sv_idxp[i]+1] - mat_offp[sv_idxp[i]];
  }
  auto pfx_sum_nnz = prefix_sum(nnz);
  size_t total_nnz = pfx_sum_nnz[sv_size - 1];
  std::vector<T> mulout_val(total_nnz);
  std::vector<I> mulout_idx(total_nnz);
  std::vector<T> mulout_val_tmp(total_nnz);
  std::vector<I> mulout_idx_tmp(total_nnz);
  spmspv_impl(mat_valp, mat_idxp, mat_offp, sv_valp, sv_idxp, sv_size,
              mulout_val.data(), mulout_idx.data(),
              mulout_val_tmp.data(), mulout_idx_tmp.data(),
              nnzp, pfx_sum_nnz.data(), total_nnz,
              ret, mat.local_num_row);
}

template <class T, class I, class O>
void spmspv_impl(T* mat_valp, I* mat_idxp, O* mat_offp,
                 T* sv_valp, I* sv_idxp, size_t sv_size,
                 T* mulout_valp, I* mulout_idxp,
                 T* mulout_val_tmpp, I* mulout_idx_tmpp,
                 O* nnzp, O* pfx_sum_nnzp, size_t total_nnz,
                 T* retp, size_t num_row) {
  spmspv_mul(mat_valp, mat_idxp, mat_offp, sv_valp, sv_idxp, sv_size,
             nnzp, pfx_sum_nnzp, total_nnz,
             mulout_valp, mulout_idxp);
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
size_t spmspv_impl_helper(T* mat_valp, I* mat_idxp, O* mat_offp,
                          T* sv_valp, I* sv_idxp, 
                          T* new_sv_valp, I* new_sv_idxp,
                          O* nnzp, O* new_nnzp,
                          size_t sv_size,
                          T* retp) {
  size_t ii = 0;
  for(size_t i = 0; i < sv_size; i++) {
    if(nnzp[i] > SPMSPV_VLEN) {
      auto off = mat_offp[sv_idxp[i]];
      auto valp = mat_valp + off;
      auto idxp = mat_idxp + off;
      auto scal = sv_valp[i];
      auto crnt_nnz = nnzp[i];
      for(size_t j = 0; j < crnt_nnz; j++) {
        retp[idxp[j]] = retp[idxp[j]] + scal * valp[j];
      }
    } else {
      new_sv_idxp[ii] = sv_idxp[i];
      new_sv_valp[ii] = sv_valp[i];
      new_nnzp[ii] = nnzp[i];
      ii++;
    }
  }
  return ii;
}

// retp should point to allocated memory area; result is accumulated
template <class T, class I, class O>
void spmspv(ccs_matrix_local<T,I,O>& mat, sparse_vector<T,I>& sv,
            T* retp) {
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
  std::vector<T> new_sv_val(sv_size);
  std::vector<I> new_sv_idx(sv_size);
  std::vector<O> new_nnz(sv_size);
  auto new_sv_valp = new_sv_val.data();
  auto new_sv_idxp = new_sv_idx.data();
  auto new_nnzp = new_nnz.data();
  auto new_sv_size = 
    spmspv_impl_helper(mat_valp, mat_idxp, mat_offp,
                       sv_valp, sv_idxp, new_sv_valp, new_sv_idxp,
                       nnzp, new_nnzp, sv_size, retp);
  if(new_sv_size == 0) {
    return;
  } else {
    std::vector<O> pfx_sum_new_nnz(new_sv_size);
    prefix_sum(new_nnzp, pfx_sum_new_nnz.data(), new_sv_size);
    auto total_new_nnz = pfx_sum_new_nnz[new_sv_size - 1];
    std::vector<T> mulout_val(total_new_nnz);
    std::vector<I> mulout_idx(total_new_nnz);
    std::vector<T> mulout_val_tmp(total_new_nnz);
    std::vector<I> mulout_idx_tmp(total_new_nnz);
    spmspv_impl(mat_valp, mat_idxp, mat_offp, 
                new_sv_valp, new_sv_idxp, new_sv_size,
                mulout_val.data(), mulout_idx.data(),
                mulout_val_tmp.data(), mulout_idx_tmp.data(),
                new_nnzp, pfx_sum_new_nnz.data(), total_new_nnz,
                retp, mat.local_num_row);
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
