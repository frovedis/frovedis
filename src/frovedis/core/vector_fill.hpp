#ifndef __VECTOR__FILL__
#define __VECTOR__FILL__

#define FILL_VLEN 256

namespace frovedis {

template <class T>
size_t adjust_start_end_ffill(size_t* start_idx, size_t* end_idx,
                              const T* vptr, size_t size, T target) {

  // step1: advance if start element of next partition is the target
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

  // step3: find max of each partition
  auto max = end_idx[0] - start_idx[0];
  for(size_t i = 1; i < FILL_VLEN; ++i) {
    auto cur = end_idx[i] - start_idx[i];
    if (cur > max) max = cur;
  }
  return max;
}

template <class T>
std::vector<T>
simple_ffill_impl(const std::vector<T>& vec, T target) {
  auto size = vec.size();
  std::vector<T> ret = vec;
  if (size == 0) return ret;

  auto vptr = vec.data();
  auto rptr = ret.data();
  auto tmax = std::numeric_limits<T>::max();
  auto non_target = tmax;
  for(size_t i = 0; i < size; ++i) {
    auto val = vptr[i];
    if (val == target) {
      if (non_target != tmax) rptr[i] = non_target;
    } else{
      non_target = val;
    }
  }
  return ret;
}

template <class T>
std::vector<T>
ffill(const std::vector<T>& vec, T target) {
  auto size = vec.size();
  if (size < FILL_VLEN) return simple_ffill_impl(vec, target);

  size_t each = ceil_div(size, size_t(FILL_VLEN));
  if (each % 2 == 0) each++;

  size_t start_idx[FILL_VLEN];
  size_t end_idx[FILL_VLEN];
  size_t cur_idx[FILL_VLEN];
  T cur_non_target[FILL_VLEN];
#pragma _NEC vreg(start_idx)
#pragma _NEC vreg(end_idx)
#pragma _NEC vreg(cur_idx)
#pragma _NEC vreg(cur_non_target)

  for(size_t i = 0; i < FILL_VLEN; ++i) {
    size_t st = i * each;
    size_t end = (i + 1) * each;
    start_idx[i] = st < size ? st : size;
    end_idx[i] = end < size ? end : size;
  }

  auto vptr = vec.data();
  auto max = adjust_start_end_ffill(start_idx, end_idx, vptr, size, target);
  auto tmax = std::numeric_limits<T>::max();
  for(size_t i = 0; i < FILL_VLEN; ++i) {
    cur_idx[i] = start_idx[i];
    cur_non_target[i] = tmax;
  }

  std::vector<T> ret = vec;
  auto rptr = ret.data();
  for (size_t i = 0; i < max; ++i) {
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t j = 0; j < FILL_VLEN; ++j) {
      if (cur_idx[j] < end_idx[j]) {
        auto idx = cur_idx[j];
        auto val = vptr[idx];
        if (val == target) {
          // very initial values might be same as targets...
          if (cur_non_target[j] != tmax) rptr[idx] = cur_non_target[j];
        } else {
          cur_non_target[j] = val;
        }
        cur_idx[j]++;
      }
    }
  }

  return ret;
}


} // end of namespace
#endif
