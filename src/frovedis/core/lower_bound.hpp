#ifndef LOWER_BOUND_HPP
#define LOWER_BOUND_HPP

#if defined(_SX) || defined(__ve__)
#define LOWER_BOUND_VLEN 2048 // not vlen, but better than 256
#else
#define LOWER_BOUND_VLEN 4
#endif

namespace frovedis {

// https://stackoverflow.com/questions/6443569/implementation-of-c-lower-bound
template <class T>
void lower_bound(const T* sorted, size_t sorted_size,
                 const T* to_find, size_t to_find_size,
                 size_t* ret) {
  size_t low[LOWER_BOUND_VLEN];
//#pragma _NEC vreg(low)
  size_t high[LOWER_BOUND_VLEN];
//#pragma _NEC vreg(high)
  T values[LOWER_BOUND_VLEN];
//#pragma _NEC vreg(values)

  size_t block_size = to_find_size / LOWER_BOUND_VLEN;
  size_t rest = to_find_size % LOWER_BOUND_VLEN;

  size_t max_iter = 0;
  auto tmp = sorted_size;
  while(tmp >>= 1) ++max_iter; // log2(sorted_size); to avoid fp error
  max_iter++;

  for(size_t i = 0; i < block_size; i++) {
    size_t offset = i * LOWER_BOUND_VLEN;
    auto crnt_to_find = to_find + offset;
    auto crnt_ret = ret + offset;
    for(size_t j = 0; j < LOWER_BOUND_VLEN; j++) {
      low[j] = 0;
      high[j] = sorted_size;
      values[j] = crnt_to_find[j];
    }
    for(size_t j = 0; j < max_iter; j++) {
#pragma _NEC ivdep
      for(size_t k = 0; k < LOWER_BOUND_VLEN; k++) {
        if(low[k] < high[k]) {
          auto mid = (low[k] + high[k]) / 2;
          if(values[k] <= sorted[mid]) {
            high[k] = mid;
          } else {
            low[k] = mid + 1;
          }
        }
      }
    }
    for(size_t k = 0; k < LOWER_BOUND_VLEN; k++) {
      crnt_ret[k] = low[k];
    }
  }

  // vreg of different length loop is not supported? (2.4.1)
  size_t low2[LOWER_BOUND_VLEN]; 
//#pragma _NEC vreg(low2)
  size_t high2[LOWER_BOUND_VLEN];
//#pragma _NEC vreg(high2)
  T values2[LOWER_BOUND_VLEN];
//#pragma _NEC vreg(values2)

  size_t offset = block_size * LOWER_BOUND_VLEN;
  auto crnt_to_find = to_find + offset;
  auto crnt_ret = ret + offset;
  for(size_t j = 0; j < rest; j++) {
    low2[j] = 0;
    high2[j] = sorted_size;
    values2[j] = crnt_to_find[j];
  }
  for(size_t j = 0; j < max_iter; j++) {
#pragma _NEC ivdep
    for(size_t k = 0; k < rest; k++) {
      if(low2[k] < high2[k]) {
        auto mid = (low2[k] + high2[k]) / 2;
        if(values2[k] <= sorted[mid]) {
          high2[k] = mid;
        } else {
          low2[k] = mid + 1;
        }
      }
    }
  }
  for(size_t k = 0; k < rest; k++) {
    crnt_ret[k] = low2[k];
  }
}

template <class T>
std::vector<size_t> lower_bound(const std::vector<T>& sorted,
                                const std::vector<T>& to_find) {
  std::vector<size_t> ret(to_find.size());
  if(to_find.size() == 0 || sorted.size() == 0) return ret;
  lower_bound(sorted.data(), sorted.size(), to_find.data(), to_find.size(),
              ret.data());
  return ret;
}

template <class T>
void lower_bound_desc(const T* sorted, size_t sorted_size,
                      const T* to_find, size_t to_find_size,
                      size_t* ret) {
  size_t low[LOWER_BOUND_VLEN];
//#pragma _NEC vreg(low)
  size_t high[LOWER_BOUND_VLEN];
//#pragma _NEC vreg(high)
  T values[LOWER_BOUND_VLEN];
//#pragma _NEC vreg(values)

  size_t block_size = to_find_size / LOWER_BOUND_VLEN;
  size_t rest = to_find_size % LOWER_BOUND_VLEN;

  size_t max_iter = 0;
  auto tmp = sorted_size;
  while(tmp >>= 1) ++max_iter; // log2(sorted_size); to avoid fp error
  max_iter++;

  for(size_t i = 0; i < block_size; i++) {
    size_t offset = i * LOWER_BOUND_VLEN;
    auto crnt_to_find = to_find + offset;
    auto crnt_ret = ret + offset;
    for(size_t j = 0; j < LOWER_BOUND_VLEN; j++) {
      low[j] = 0;
      high[j] = sorted_size;
      values[j] = crnt_to_find[j];
    }
    for(size_t j = 0; j < max_iter; j++) {
#pragma _NEC ivdep
      for(size_t k = 0; k < LOWER_BOUND_VLEN; k++) {
        if(low[k] < high[k]) {
          auto mid = (low[k] + high[k]) / 2;
          if(values[k] >= sorted[mid]) {
            high[k] = mid;
          } else {
            low[k] = mid + 1;
          }
        }
      }
    }
    for(size_t k = 0; k < LOWER_BOUND_VLEN; k++) {
      crnt_ret[k] = low[k];
    }
  }

  // vreg of different length loop is not supported? (2.4.1)
  size_t low2[LOWER_BOUND_VLEN]; 
//#pragma _NEC vreg(low2)
  size_t high2[LOWER_BOUND_VLEN];
//#pragma _NEC vreg(high2)
  T values2[LOWER_BOUND_VLEN];
//#pragma _NEC vreg(values2)

  size_t offset = block_size * LOWER_BOUND_VLEN;
  auto crnt_to_find = to_find + offset;
  auto crnt_ret = ret + offset;
  for(size_t j = 0; j < rest; j++) {
    low2[j] = 0;
    high2[j] = sorted_size;
    values2[j] = crnt_to_find[j];
  }
  for(size_t j = 0; j < max_iter; j++) {
#pragma _NEC ivdep
    for(size_t k = 0; k < rest; k++) {
      if(low2[k] < high2[k]) {
        auto mid = (low2[k] + high2[k]) / 2;
        if(values2[k] >= sorted[mid]) {
          high2[k] = mid;
        } else {
          low2[k] = mid + 1;
        }
      }
    }
  }
  for(size_t k = 0; k < rest; k++) {
    crnt_ret[k] = low2[k];
  }
}

template <class T>
std::vector<size_t> lower_bound_desc(const std::vector<T>& sorted,
                                     const std::vector<T>& to_find) {
  std::vector<size_t> ret(to_find.size());
  if(to_find.size() == 0 || sorted.size() == 0) return ret;
  lower_bound_desc(sorted.data(), sorted.size(), to_find.data(), to_find.size(),
                   ret.data());
  return ret;
}

}

#endif
