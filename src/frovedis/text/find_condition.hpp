#ifndef FIND_CONDITION_HPP
#define FIND_CONDITION_HPP

#if defined(__ve__) || defined(_SX)
#define FIND_CONDITION_VLEN 1024
#define FIND_CONDITION_VLEN_EACH 256
#define FIND_CONDITION_THR 16
#define FIND_CONDITION_ALIGN_SIZE 128
#else
#define FIND_CONDITION_VLEN 4
#define FIND_CONDITION_VLEN_EACH 1
#define FIND_CONDITION_THR 1
#define FIND_CONDITION_ALIGN_SIZE 1
#endif

#include <vector>
#include <cstddef>
#include <stdexcept>

namespace frovedis {

template <class T, class F>
std::vector<size_t> find_condition(const T* vp, size_t size,
                                   const F& condition) {
  if(size == 0) {
    return std::vector<size_t>();
  }
  std::vector<size_t> rettmp(size);
  auto rettmpp = rettmp.data();

  size_t each = size / FIND_CONDITION_VLEN; // maybe 0
  auto aligned_each = each * sizeof(T) / FIND_CONDITION_ALIGN_SIZE;
  if(aligned_each % 2 == 0 && aligned_each != 0)
    aligned_each -= 1;
  each = aligned_each * FIND_CONDITION_ALIGN_SIZE / sizeof(T);
  size_t rest = size - each * FIND_CONDITION_VLEN;

  size_t out_ridx[FIND_CONDITION_VLEN];
  size_t out_ridx_0[FIND_CONDITION_VLEN_EACH];
  size_t out_ridx_1[FIND_CONDITION_VLEN_EACH];
  size_t out_ridx_2[FIND_CONDITION_VLEN_EACH];
  size_t out_ridx_3[FIND_CONDITION_VLEN_EACH];
// never remove this vreg! this is needed following vovertake
#pragma _NEC vreg(out_ridx_0)
#pragma _NEC vreg(out_ridx_1)
#pragma _NEC vreg(out_ridx_2)
#pragma _NEC vreg(out_ridx_3)
  for(size_t i = 0; i < FIND_CONDITION_VLEN_EACH; i++) {
    auto v0 = i;
    auto v1 = i + FIND_CONDITION_VLEN_EACH;
    auto v2 = i + FIND_CONDITION_VLEN_EACH * 2;
    auto v3 = i + FIND_CONDITION_VLEN_EACH * 3;
    out_ridx_0[i] = each * v0;
    out_ridx_1[i] = each * v1;
    out_ridx_2[i] = each * v2;
    out_ridx_3[i] = each * v3;
  }

  if(each < FIND_CONDITION_THR) {
    size_t current = 0;
    for(size_t i = 0; i < size; i++) {
      if(condition(vp[i])) {
        rettmpp[current++] = i;
      }
    }
    std::vector<size_t> ret(current);
    auto retp = ret.data();
    for(size_t i = 0; i < current; i++) {
      retp[i] = rettmpp[i];
    }
    return ret;
  } else {
#pragma _NEC vob
    for(size_t j = 0; j < each; j++) {
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
      for(size_t i = 0; i < FIND_CONDITION_VLEN_EACH; i++) {
        auto idx0 = j + each * i;
        auto idx1 = j + each * (i + FIND_CONDITION_VLEN_EACH);
        auto idx2 = j + each * (i + FIND_CONDITION_VLEN_EACH * 2);
        auto idx3 = j + each * (i + FIND_CONDITION_VLEN_EACH * 3);
        auto val0 = vp[idx0];
        auto val1 = vp[idx1];
        auto val2 = vp[idx2];
        auto val3 = vp[idx3];
        if(condition(val0)) {
          rettmpp[out_ridx_0[i]++] = idx0;
        }
        if(condition(val1)) {
          rettmpp[out_ridx_1[i]++] = idx1;
        }
        if(condition(val2)) {
          rettmpp[out_ridx_2[i]++] = idx2;
        }
        if(condition(val3)) {
          rettmpp[out_ridx_3[i]++] = idx3;
        }
      }
    }
    
    size_t rest_idx_start = each * FIND_CONDITION_VLEN;
    size_t rest_idx = rest_idx_start;
    if(rest != 0) {
      for(size_t j = 0; j < rest; j++) {
        if(condition(vp[j + rest_idx_start])) {
          rettmpp[rest_idx++] = j + rest_idx_start;
        }
      }
    }
    for(size_t i = 0; i < FIND_CONDITION_VLEN_EACH; i++) {
      auto v0 = i;
      auto v1 = i + FIND_CONDITION_VLEN_EACH;
      auto v2 = i + FIND_CONDITION_VLEN_EACH * 2;
      auto v3 = i + FIND_CONDITION_VLEN_EACH * 3;
      out_ridx[v0] = out_ridx_0[i];
      out_ridx[v1] = out_ridx_1[i];
      out_ridx[v2] = out_ridx_2[i];
      out_ridx[v3] = out_ridx_3[i];
    }
    size_t sizes[FIND_CONDITION_VLEN];
    for(size_t i = 0; i < FIND_CONDITION_VLEN; i++) {
      sizes[i] = out_ridx[i] - each * i;
    }
    size_t total = 0;
    for(size_t i = 0; i < FIND_CONDITION_VLEN; i++) {
      total += sizes[i];
    }
    size_t rest_size = rest_idx - each * FIND_CONDITION_VLEN;
    total += rest_size;
    std::vector<size_t> ret(total);
    auto retp = ret.data();
    size_t current = 0;
    for(size_t i = 0; i < FIND_CONDITION_VLEN; i++) {
      for(size_t j = 0; j < sizes[i]; j++) {
        retp[current + j] = rettmpp[each * i + j];
      }
      current += sizes[i];
    }
    for(size_t j = 0; j < rest_size; j++) {
      retp[current + j] = rettmpp[rest_idx_start + j];
    }
    return ret;
  }
}

template <class T, class F>
std::vector<size_t> find_condition(const std::vector<T>& v,
                                   const F& condition) {
  return find_condition(v.data(), v.size(), condition);
}

template <class T, class F>
std::vector<size_t> find_condition_index(const T* vp,
                                         const size_t* idxp,
                                         size_t size,
                                         const F& condition) {
  if(size == 0) {
    return std::vector<size_t>();
  }
  std::vector<size_t> rettmp(size);
  auto rettmpp = rettmp.data();

  size_t each = size / FIND_CONDITION_VLEN; // maybe 0
  auto aligned_each = each * sizeof(T) / FIND_CONDITION_ALIGN_SIZE;
  if(aligned_each % 2 == 0 && aligned_each != 0)
    aligned_each -= 1;
  each = aligned_each * FIND_CONDITION_ALIGN_SIZE / sizeof(T);
  size_t rest = size - each * FIND_CONDITION_VLEN;

  size_t out_ridx[FIND_CONDITION_VLEN];
  size_t out_ridx_0[FIND_CONDITION_VLEN_EACH];
  size_t out_ridx_1[FIND_CONDITION_VLEN_EACH];
  size_t out_ridx_2[FIND_CONDITION_VLEN_EACH];
  size_t out_ridx_3[FIND_CONDITION_VLEN_EACH];
// never remove this vreg! this is needed following vovertake
#pragma _NEC vreg(out_ridx_0)
#pragma _NEC vreg(out_ridx_1)
#pragma _NEC vreg(out_ridx_2)
#pragma _NEC vreg(out_ridx_3)
  for(size_t i = 0; i < FIND_CONDITION_VLEN_EACH; i++) {
    auto v0 = i;
    auto v1 = i + FIND_CONDITION_VLEN_EACH;
    auto v2 = i + FIND_CONDITION_VLEN_EACH * 2;
    auto v3 = i + FIND_CONDITION_VLEN_EACH * 3;
    out_ridx_0[i] = each * v0;
    out_ridx_1[i] = each * v1;
    out_ridx_2[i] = each * v2;
    out_ridx_3[i] = each * v3;
  }

  if(each < FIND_CONDITION_THR) {
    size_t current = 0;
    for(size_t i = 0; i < size; i++) {
      if(condition(vp[idxp[i]])) {
        rettmpp[current++] = i;
      }
    }
    std::vector<size_t> ret(current);
    auto retp = ret.data();
    for(size_t i = 0; i < current; i++) {
      retp[i] = rettmpp[i];
    }
    return ret;
  } else {
#pragma _NEC vob
    for(size_t j = 0; j < each; j++) {
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
      for(size_t i = 0; i < FIND_CONDITION_VLEN_EACH; i++) {
        auto idx0 = j + each * i;
        auto idx1 = j + each * (i + FIND_CONDITION_VLEN_EACH);
        auto idx2 = j + each * (i + FIND_CONDITION_VLEN_EACH * 2);
        auto idx3 = j + each * (i + FIND_CONDITION_VLEN_EACH * 3);
        auto val0 = vp[idxp[idx0]];
        auto val1 = vp[idxp[idx1]];
        auto val2 = vp[idxp[idx2]];
        auto val3 = vp[idxp[idx3]];
        if(condition(val0)) {
          rettmpp[out_ridx_0[i]++] = idx0;
        }
        if(condition(val1)) {
          rettmpp[out_ridx_1[i]++] = idx1;
        }
        if(condition(val2)) {
          rettmpp[out_ridx_2[i]++] = idx2;
        }
        if(condition(val3)) {
          rettmpp[out_ridx_3[i]++] = idx3;
        }
      }
    }
    size_t rest_idx_start = each * FIND_CONDITION_VLEN;
    size_t rest_idx = rest_idx_start;
    if(rest != 0) {
      for(size_t j = 0; j < rest; j++) {
        if(condition(vp[idxp[j + rest_idx_start]])) {
          rettmpp[rest_idx++] = j + rest_idx_start;
        }
      }
    }
    for(size_t i = 0; i < FIND_CONDITION_VLEN_EACH; i++) {
      auto v0 = i;
      auto v1 = i + FIND_CONDITION_VLEN_EACH;
      auto v2 = i + FIND_CONDITION_VLEN_EACH * 2;
      auto v3 = i + FIND_CONDITION_VLEN_EACH * 3;
      out_ridx[v0] = out_ridx_0[i];
      out_ridx[v1] = out_ridx_1[i];
      out_ridx[v2] = out_ridx_2[i];
      out_ridx[v3] = out_ridx_3[i];
    }
    size_t sizes[FIND_CONDITION_VLEN];
    for(size_t i = 0; i < FIND_CONDITION_VLEN; i++) {
      sizes[i] = out_ridx[i] - each * i;
    }
    size_t total = 0;
    for(size_t i = 0; i < FIND_CONDITION_VLEN; i++) {
      total += sizes[i];
    }
    size_t rest_size = rest_idx - each * FIND_CONDITION_VLEN;
    total += rest_size;
    std::vector<size_t> ret(total);
    auto retp = ret.data();
    size_t current = 0;
    for(size_t i = 0; i < FIND_CONDITION_VLEN; i++) {
      for(size_t j = 0; j < sizes[i]; j++) {
        retp[current + j] = rettmpp[each * i + j];
      }
      current += sizes[i];
    }
    for(size_t j = 0; j < rest_size; j++) {
      retp[current + j] = rettmpp[rest_idx_start + j];
    }
    return ret;
  }
}

template <class T, class F>
std::vector<size_t> find_condition_index(const std::vector<T>& v,
                                         const std::vector<size_t>& idx,
                                         const F& condition) {
  return find_condition_index(v.data(), idx.data(), idx.size(), condition);
}

template <class T, class F>
std::vector<size_t> find_condition_pointer(T** vpp,
                                           size_t size,
                                           const F& condition) {
  if(size == 0) {
    return std::vector<size_t>();
  }
  std::vector<size_t> rettmp(size);
  auto rettmpp = rettmp.data();

  size_t each = size / FIND_CONDITION_VLEN; // maybe 0
  auto aligned_each = each * sizeof(T) / FIND_CONDITION_ALIGN_SIZE;
  if(aligned_each % 2 == 0 && aligned_each != 0)
    aligned_each -= 1;
  each = aligned_each * FIND_CONDITION_ALIGN_SIZE / sizeof(T);
  size_t rest = size - each * FIND_CONDITION_VLEN;

  size_t out_ridx[FIND_CONDITION_VLEN];
  size_t out_ridx_0[FIND_CONDITION_VLEN_EACH];
  size_t out_ridx_1[FIND_CONDITION_VLEN_EACH];
  size_t out_ridx_2[FIND_CONDITION_VLEN_EACH];
  size_t out_ridx_3[FIND_CONDITION_VLEN_EACH];
// never remove this vreg! this is needed following vovertake
#pragma _NEC vreg(out_ridx_0)
#pragma _NEC vreg(out_ridx_1)
#pragma _NEC vreg(out_ridx_2)
#pragma _NEC vreg(out_ridx_3)
  for(size_t i = 0; i < FIND_CONDITION_VLEN_EACH; i++) {
    auto v0 = i;
    auto v1 = i + FIND_CONDITION_VLEN_EACH;
    auto v2 = i + FIND_CONDITION_VLEN_EACH * 2;
    auto v3 = i + FIND_CONDITION_VLEN_EACH * 3;
    out_ridx_0[i] = each * v0;
    out_ridx_1[i] = each * v1;
    out_ridx_2[i] = each * v2;
    out_ridx_3[i] = each * v3;
  }

  if(each < FIND_CONDITION_THR) {
    size_t current = 0;
    for(size_t i = 0; i < size; i++) {
      if(condition(*vpp[i])) {
        rettmpp[current++] = i;
      }
    }
    std::vector<size_t> ret(current);
    auto retp = ret.data();
    for(size_t i = 0; i < current; i++) {
      retp[i] = rettmpp[i];
    }
    return ret;
  } else {
#pragma _NEC vob
    for(size_t j = 0; j < each; j++) {
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
      for(size_t i = 0; i < FIND_CONDITION_VLEN_EACH; i++) {
        auto idx0 = j + each * i;
        auto idx1 = j + each * (i + FIND_CONDITION_VLEN_EACH);
        auto idx2 = j + each * (i + FIND_CONDITION_VLEN_EACH * 2);
        auto idx3 = j + each * (i + FIND_CONDITION_VLEN_EACH * 3);
        auto val0 = *vpp[idx0];
        auto val1 = *vpp[idx1];
        auto val2 = *vpp[idx2];
        auto val3 = *vpp[idx3];
        if(condition(val0)) {
          rettmpp[out_ridx_0[i]++] = idx0;
        }
        if(condition(val1)) {
          rettmpp[out_ridx_1[i]++] = idx1;
        }
        if(condition(val2)) {
          rettmpp[out_ridx_2[i]++] = idx2;
        }
        if(condition(val3)) {
          rettmpp[out_ridx_3[i]++] = idx3;
        }
      }
    }
    size_t rest_idx_start = each * FIND_CONDITION_VLEN;
    size_t rest_idx = rest_idx_start;
    if(rest != 0) {
      for(size_t j = 0; j < rest; j++) {
        if(condition(*vpp[j + rest_idx_start])) {
          rettmpp[rest_idx++] = j + rest_idx_start;
        }
      }
    }
    for(size_t i = 0; i < FIND_CONDITION_VLEN_EACH; i++) {
      auto v0 = i;
      auto v1 = i + FIND_CONDITION_VLEN_EACH;
      auto v2 = i + FIND_CONDITION_VLEN_EACH * 2;
      auto v3 = i + FIND_CONDITION_VLEN_EACH * 3;
      out_ridx[v0] = out_ridx_0[i];
      out_ridx[v1] = out_ridx_1[i];
      out_ridx[v2] = out_ridx_2[i];
      out_ridx[v3] = out_ridx_3[i];
    }
    size_t sizes[FIND_CONDITION_VLEN];
    for(size_t i = 0; i < FIND_CONDITION_VLEN; i++) {
      sizes[i] = out_ridx[i] - each * i;
    }
    size_t total = 0;
    for(size_t i = 0; i < FIND_CONDITION_VLEN; i++) {
      total += sizes[i];
    }
    size_t rest_size = rest_idx - each * FIND_CONDITION_VLEN;
    total += rest_size;
    std::vector<size_t> ret(total);
    auto retp = ret.data();
    size_t current = 0;
    for(size_t i = 0; i < FIND_CONDITION_VLEN; i++) {
      for(size_t j = 0; j < sizes[i]; j++) {
        retp[current + j] = rettmpp[each * i + j];
      }
      current += sizes[i];
    }
    for(size_t j = 0; j < rest_size; j++) {
      retp[current + j] = rettmpp[rest_idx_start + j];
    }
    return ret;
  }
}

template <class T, class U, class F>
std::vector<size_t> find_condition_pair(const T* lp, const U* rp,
                                        size_t size, const F& condition) {
  if(size == 0) {
    return std::vector<size_t>();
  }
  std::vector<size_t> rettmp(size);
  auto rettmpp = rettmp.data();

  size_t each = size / FIND_CONDITION_VLEN; // maybe 0
  auto aligned_each = each * sizeof(T) / FIND_CONDITION_ALIGN_SIZE;
  if(aligned_each % 2 == 0 && aligned_each != 0)
    aligned_each -= 1;
  each = aligned_each * FIND_CONDITION_ALIGN_SIZE / sizeof(T);
  size_t rest = size - each * FIND_CONDITION_VLEN;

  size_t out_ridx[FIND_CONDITION_VLEN];
  size_t out_ridx_0[FIND_CONDITION_VLEN_EACH];
  size_t out_ridx_1[FIND_CONDITION_VLEN_EACH];
  size_t out_ridx_2[FIND_CONDITION_VLEN_EACH];
  size_t out_ridx_3[FIND_CONDITION_VLEN_EACH];
// never remove this vreg! this is needed following vovertake
#pragma _NEC vreg(out_ridx_0)
#pragma _NEC vreg(out_ridx_1)
#pragma _NEC vreg(out_ridx_2)
#pragma _NEC vreg(out_ridx_3)
  for(size_t i = 0; i < FIND_CONDITION_VLEN_EACH; i++) {
    auto v0 = i;
    auto v1 = i + FIND_CONDITION_VLEN_EACH;
    auto v2 = i + FIND_CONDITION_VLEN_EACH * 2;
    auto v3 = i + FIND_CONDITION_VLEN_EACH * 3;
    out_ridx_0[i] = each * v0;
    out_ridx_1[i] = each * v1;
    out_ridx_2[i] = each * v2;
    out_ridx_3[i] = each * v3;
  }

  if(each < FIND_CONDITION_THR) {
    size_t current = 0;
    for(size_t i = 0; i < size; i++) {
      if(condition(lp[i],rp[i])) {
        rettmpp[current++] = i;
      }
    }
    std::vector<size_t> ret(current);
    auto retp = ret.data();
    for(size_t i = 0; i < current; i++) {
      retp[i] = rettmpp[i];
    }
    return ret;
  } else {
#pragma _NEC vob
    for(size_t j = 0; j < each; j++) {
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
      for(size_t i = 0; i < FIND_CONDITION_VLEN_EACH; i++) {
        auto idx0 = j + each * i;
        auto idx1 = j + each * (i + FIND_CONDITION_VLEN_EACH);
        auto idx2 = j + each * (i + FIND_CONDITION_VLEN_EACH * 2);
        auto idx3 = j + each * (i + FIND_CONDITION_VLEN_EACH * 3);
        auto lval0 = lp[idx0];
        auto lval1 = lp[idx1];
        auto lval2 = lp[idx2];
        auto lval3 = lp[idx3];
        auto rval0 = rp[idx0];
        auto rval1 = rp[idx1];
        auto rval2 = rp[idx2];
        auto rval3 = rp[idx3];
        if(condition(lval0,rval0)) {
          rettmpp[out_ridx_0[i]++] = idx0;
        }
        if(condition(lval1,rval1)) {
          rettmpp[out_ridx_1[i]++] = idx1;
        }
        if(condition(lval2,rval2)) {
          rettmpp[out_ridx_2[i]++] = idx2;
        }
        if(condition(lval3,rval3)) {
          rettmpp[out_ridx_3[i]++] = idx3;
        }
      }
    }
    
    size_t rest_idx_start = each * FIND_CONDITION_VLEN;
    size_t rest_idx = rest_idx_start;
    if(rest != 0) {
      for(size_t j = 0; j < rest; j++) {
        if(condition(lp[j + rest_idx_start], rp[j + rest_idx_start])) {
          rettmpp[rest_idx++] = j + rest_idx_start;
        }
      }
    }
    for(size_t i = 0; i < FIND_CONDITION_VLEN_EACH; i++) {
      auto v0 = i;
      auto v1 = i + FIND_CONDITION_VLEN_EACH;
      auto v2 = i + FIND_CONDITION_VLEN_EACH * 2;
      auto v3 = i + FIND_CONDITION_VLEN_EACH * 3;
      out_ridx[v0] = out_ridx_0[i];
      out_ridx[v1] = out_ridx_1[i];
      out_ridx[v2] = out_ridx_2[i];
      out_ridx[v3] = out_ridx_3[i];
    }
    size_t sizes[FIND_CONDITION_VLEN];
    for(size_t i = 0; i < FIND_CONDITION_VLEN; i++) {
      sizes[i] = out_ridx[i] - each * i;
    }
    size_t total = 0;
    for(size_t i = 0; i < FIND_CONDITION_VLEN; i++) {
      total += sizes[i];
    }
    size_t rest_size = rest_idx - each * FIND_CONDITION_VLEN;
    total += rest_size;
    std::vector<size_t> ret(total);
    auto retp = ret.data();
    size_t current = 0;
    for(size_t i = 0; i < FIND_CONDITION_VLEN; i++) {
      for(size_t j = 0; j < sizes[i]; j++) {
        retp[current + j] = rettmpp[each * i + j];
      }
      current += sizes[i];
    }
    for(size_t j = 0; j < rest_size; j++) {
      retp[current + j] = rettmpp[rest_idx_start + j];
    }
    return ret;
  }
}

template <class T, class U, class F>
std::vector<size_t> find_condition_pair(const std::vector<T>& v1,
                                        const std::vector<U>& v2,
                                        const F& condition) {
  auto size = v1.size();
  if(size != v2.size())
    throw std::runtime_error
      ("find_condition_pair: size of vectors are not same");
  return find_condition_pair(v1.data(), v2.data(), size, condition);
}

}
#endif
