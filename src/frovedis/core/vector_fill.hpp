#ifndef __VECTOR__FILL__
#define __VECTOR__FILL__

#define FILL_VLEN 256

namespace frovedis {

template <class T>
size_t adjust_start_end_ffill(size_t* start_idx, size_t* end_idx,
                              const T* vptr, size_t size, T target) {

  // step1: advance, if the first element of next the partition is the target
  int anyvalid = true;
  while(anyvalid) {
    anyvalid = false;
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t i = 0; i < FILL_VLEN; ++i) {
      auto idx = end_idx[i]; // ith end == start of (i+1)th partition
      if (idx < size && vptr[idx] == target) {
        end_idx[i]++;
        anyvalid = true;
      }
    }
  }

  // step2: adjust start as per the modified end
  for(size_t i = 1; i < FILL_VLEN; ++i) start_idx[i] = end_idx[i - 1];

  // step3: find max of all partition lengths
  auto max = end_idx[0] - start_idx[0];
  for(size_t i = 1; i < FILL_VLEN; ++i) {
    auto cur = end_idx[i] - start_idx[i];
    if (cur > max) max = cur;
  }
  return max;
}

template <class T>
void simple_ffill_inplace(T* vptr_in, size_t size, T target) {
  if (size == 0) return;

  // to avoid initial targets which cannot be filled...
  size_t k = 0;
  if (vptr_in[0] == target) {
    for(; k < size; ++k) if (vptr_in[k] != target) break;
  }
  auto vptr = vptr_in + k;
  size -= k;
  auto tmax = std::numeric_limits<T>::max();
  auto non_target = tmax;

/*
  size_t i = 0;
  while(i < size) {
    auto val = vptr[i];
    if (val == target) {
      for(; i < size; ++i) { // vectorized-loop, possibly with very poor vector-length  
        if (vptr[i] != target) break;
        vptr[i] = non_target;
      }
    } else{
      non_target = val;
      i++;
    }
  }
*/

  for(size_t i = 0; i < size; ++i) {
    auto val = vptr[i];
    if (val == target) {
      vptr[i] = non_target;
    } else{
      non_target = val;
    }
  }
}

template <class T>
std::vector<T>
simple_ffill(const T* vptr, size_t size, T target) {
  std::vector<T> ret(size);
  auto rptr = ret.data();
  for(size_t i = 0; i < size; ++i) rptr[i] = vptr[i];
  simple_ffill_inplace(rptr, size, target);
  return ret;
}

template <class T>
std::vector<T>
simple_ffill(const std::vector<T>& vec, T target) {
  return simple_ffill(vec.data(), vec.size(), target);
}

template <class T>
void simple_ffill_inplace(std::vector<T>& vec, T target) {
  simple_ffill_inplace(vec.data(), vec.size(), target);
}

// loop-raking version...
template <class T>
void ffill_inplace(T* vptr_in, size_t size, T target) {
  if (size == 0) return;

  // to avoid initial targets which cannot be filled...
  size_t k = 0;
  if (vptr_in[0] == target) {
    for(; k < size; ++k) if (vptr_in[k] != target) break;
  }
  auto vptr = vptr_in + k;
  size -= k;
  if (size < FILL_VLEN) {
    simple_ffill_inplace(vptr, size, target);
    return;
  }

  size_t each = ceil_div(size, size_t(FILL_VLEN));
  if (each % 2 == 0) each++;

  size_t start_idx[FILL_VLEN];
  size_t end_idx[FILL_VLEN];
  size_t end_idx_vreg[FILL_VLEN];
  size_t cur_idx[FILL_VLEN];
  T cur_non_target[FILL_VLEN];
#pragma _NEC vreg(end_idx_vreg)
#pragma _NEC vreg(cur_idx)
#pragma _NEC vreg(cur_non_target)

  for(size_t i = 0; i < FILL_VLEN; ++i) {
    size_t st = i * each;
    size_t end = (i + 1) * each;
    start_idx[i] = st < size ? st : size;
    end_idx[i] = end < size ? end : size;
  }

  auto max = adjust_start_end_ffill(start_idx, end_idx, vptr, size, target);
  auto tmax = std::numeric_limits<T>::max();
  for(size_t i = 0; i < FILL_VLEN; ++i) {
    end_idx_vreg[i] = end_idx[i];
    cur_idx[i] = start_idx[i];
    cur_non_target[i] = tmax;
  }

#pragma _NEC vob
  for (size_t i = 0; i < max; ++i) {
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
    for(size_t j = 0; j < FILL_VLEN; ++j) {
      if (cur_idx[j] < end_idx_vreg[j]) {
        auto idx = cur_idx[j];
        auto val = vptr[idx];
        if (val == target) {
          vptr[idx] = cur_non_target[j];
        } else {
          cur_non_target[j] = val;
        }
        cur_idx[j]++;
      }
    }
  }
}

template <class T>
std::vector<T>
ffill(const T* vptr, size_t size, T target) {
  std::vector<T> ret(size);
  auto rptr = ret.data();
  for(size_t i = 0; i < size; ++i) rptr[i] = vptr[i];
  ffill_inplace(rptr, size, target);
  return ret;
}

template <class T>
std::vector<T>
ffill(const std::vector<T>& vec, T target) {
  return ffill(vec.data(), vec.size(), target);
}

template <class T>
void ffill_inplace(std::vector<T>& vec, T target) {
  ffill_inplace(vec.data(), vec.size(), target);
}

} // end of namespace
#endif
