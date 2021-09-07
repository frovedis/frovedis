#ifndef MULTIVALUE_UPPER_BOUND_HPP
#define MULTIVALUE_UPPER_BOUND_HPP

#if defined(__ve__)
#define MULTIVALUE_UPPER_BOUND_VLEN 16384
#else
#define MULTIVALUE_UPPER_BOUND_VLEN 4
#endif

namespace frovedis {

template <class T>
void multivalue_upper_bound(const T* sorted, size_t sorted_size,
                            const T* to_find, size_t to_find_size,
                            int num_values, size_t* ret) {
  size_t low[MULTIVALUE_UPPER_BOUND_VLEN];
//#pragma _NEC vreg(low)
  size_t high[MULTIVALUE_UPPER_BOUND_VLEN];
//#pragma _NEC vreg(high)
  size_t mid[MULTIVALUE_UPPER_BOUND_VLEN];
//#pragma _NEC vreg(mid)
  int is_eq[MULTIVALUE_UPPER_BOUND_VLEN];
//#pragma _NEC vreg(is_eq)
  int is_gt[MULTIVALUE_UPPER_BOUND_VLEN];
//#pragma _NEC vreg(is_gt)

  // to surpress "may be used uninitialized" warning  
  for(int i = 0; i < MULTIVALUE_UPPER_BOUND_VLEN; i++) {
    low[i] = 0;
    high[i] = 0;
    mid[i] = 0;
    is_eq[i] = 0;
    is_gt[i] = 0;
  }

  size_t block_size = to_find_size / MULTIVALUE_UPPER_BOUND_VLEN;
  size_t rest = to_find_size % MULTIVALUE_UPPER_BOUND_VLEN;

  size_t max_iter = 0;
  auto tmp = sorted_size;
  while(tmp >>= 1) ++max_iter; // log2(sorted_size); to avoid fp error
  max_iter++;

  for(size_t i = 0; i < block_size; i++) {
    size_t offset = i * MULTIVALUE_UPPER_BOUND_VLEN;
    auto crnt_to_find = to_find + offset * num_values;
    auto crnt_ret = ret + offset;
    for(size_t j = 0; j < MULTIVALUE_UPPER_BOUND_VLEN; j++) {
      low[j] = 0;
      high[j] = sorted_size;
    }
    for(size_t j = 0; j < max_iter; j++) {
#pragma _NEC ivdep
      for(size_t k = 0; k < MULTIVALUE_UPPER_BOUND_VLEN; k++) {
        is_eq[k] = 1;
        is_gt[k] = 0;
        if(low[k] < high[k]) {
          mid[k] = (low[k] + high[k]) / 2;
        }
      }
      for(size_t len = 0; len < num_values; len++) {
#pragma _NEC ivdep
        for(size_t k = 0; k < MULTIVALUE_UPPER_BOUND_VLEN; k++) {
          if(low[k] < high[k]) {
            auto value = crnt_to_find[k * num_values + len];
            int crnt_is_eq = (value == sorted[mid[k] * num_values + len]);
            int crnt_is_gt = (value > sorted[mid[k] * num_values + len]);
            if(is_eq[k]) is_gt[k] = crnt_is_gt;
            if(!crnt_is_eq) is_eq[k] = 0;
          }
        }
      }
#pragma _NEC ivdep
      for(size_t k = 0; k < MULTIVALUE_UPPER_BOUND_VLEN; k++) {
        if(low[k] < high[k]) {
          if(is_gt[k] || is_eq[k]) {
            low[k] = mid[k] + 1;
          } else {
            high[k] = mid[k];
          }
        }
      }
    }
    for(size_t k = 0; k < MULTIVALUE_UPPER_BOUND_VLEN; k++) {
      crnt_ret[k] = low[k];
    }
  }

  size_t low2[MULTIVALUE_UPPER_BOUND_VLEN];
  size_t high2[MULTIVALUE_UPPER_BOUND_VLEN];
  size_t mid2[MULTIVALUE_UPPER_BOUND_VLEN];
  int is_eq2[MULTIVALUE_UPPER_BOUND_VLEN];
  int is_gt2[MULTIVALUE_UPPER_BOUND_VLEN];

  // to surpress "may be used uninitialized" warning  
  for(int i = 0; i < MULTIVALUE_UPPER_BOUND_VLEN; i++) {
    low2[i] = 0;
    high2[i] = 0;
    mid2[i] = 0;
    is_eq2[i] = 0;
    is_gt2[i] = 0;
  }

  size_t offset = block_size * MULTIVALUE_UPPER_BOUND_VLEN;
  auto crnt_to_find = to_find + offset * num_values;
  auto crnt_ret = ret + offset;
  for(size_t j = 0; j < rest; j++) {
    low2[j] = 0;
    high2[j] = sorted_size;
  }
  for(size_t j = 0; j < max_iter; j++) {
#pragma _NEC ivdep
    for(size_t k = 0; k < rest; k++) {
      is_eq2[k] = 1;
      is_gt2[k] = 0;
      if(low2[k] < high2[k]) {
        mid2[k] = (low2[k] + high2[k]) / 2;
      }
    }
    for(size_t len = 0; len < num_values; len++) {
#pragma _NEC ivdep
      for(size_t k = 0; k < rest; k++) {
        if(low2[k] < high2[k]) {
          auto value = crnt_to_find[k * num_values + len];
          int crnt_is_eq = (value == sorted[mid2[k] * num_values + len]);
          int crnt_is_gt = (value > sorted[mid2[k] * num_values + len]);
          if(is_eq2[k]) is_gt2[k] = crnt_is_gt;
          if(!crnt_is_eq) is_eq2[k] = 0;
        }
      }
    }
#pragma _NEC ivdep
    for(size_t k = 0; k < rest; k++) {
      if(low2[k] < high2[k]) {
        if(is_gt2[k] || is_eq2[k]) {
          low2[k] = mid2[k] + 1;
        } else {
          high2[k] = mid2[k];
        }
      }
    }
  }
  for(size_t k = 0; k < rest; k++) {
    crnt_ret[k] = low2[k];
  }
}

// values are stored in rowmajor order
template <class T>
std::vector<size_t>
multivalue_upper_bound(const std::vector<T>& sorted,
                       const std::vector<T>& to_find,
                       int num_values) {
  if(sorted.size() % num_values != 0 || to_find.size() % num_values != 0)
    throw std::runtime_error("multivalue_upper_bound: wrong num_values");
  auto to_find_size = to_find.size() / num_values;
  std::vector<size_t> ret(to_find_size);
  if(to_find.size() == 0 || sorted.size() == 0) return ret;
  multivalue_upper_bound(sorted.data(), sorted.size()/num_values,
                         to_find.data(), to_find_size, num_values, ret.data());
  return ret;
}

template <class T>
void multivalue_upper_bound_desc(const T* sorted, size_t sorted_size,
                                 const T* to_find, size_t to_find_size,
                                 int num_values, size_t* ret) {
  size_t low[MULTIVALUE_UPPER_BOUND_VLEN];
//#pragma _NEC vreg(low)
  size_t high[MULTIVALUE_UPPER_BOUND_VLEN];
//#pragma _NEC vreg(high)
  size_t mid[MULTIVALUE_UPPER_BOUND_VLEN];
//#pragma _NEC vreg(mid)
  int is_eq[MULTIVALUE_UPPER_BOUND_VLEN];
//#pragma _NEC vreg(is_eq)
  int is_lt[MULTIVALUE_UPPER_BOUND_VLEN];
//#pragma _NEC vreg(is_lt)

  // to surpress "may be used uninitialized" warning  
  for(int i = 0; i < MULTIVALUE_UPPER_BOUND_VLEN; i++) {
    low[i] = 0;
    high[i] = 0;
    mid[i] = 0;
    is_eq[i] = 0;
    is_lt[i] = 0;
  }

  size_t block_size = to_find_size / MULTIVALUE_UPPER_BOUND_VLEN;
  size_t rest = to_find_size % MULTIVALUE_UPPER_BOUND_VLEN;

  size_t max_iter = 0;
  auto tmp = sorted_size;
  while(tmp >>= 1) ++max_iter; // log2(sorted_size); to avoid fp error
  max_iter++;

  for(size_t i = 0; i < block_size; i++) {
    size_t offset = i * MULTIVALUE_UPPER_BOUND_VLEN;
    auto crnt_to_find = to_find + offset * num_values;
    auto crnt_ret = ret + offset;
    for(size_t j = 0; j < MULTIVALUE_UPPER_BOUND_VLEN; j++) {
      low[j] = 0;
      high[j] = sorted_size;
    }
    for(size_t j = 0; j < max_iter; j++) {
#pragma _NEC ivdep
      for(size_t k = 0; k < MULTIVALUE_UPPER_BOUND_VLEN; k++) {
        is_eq[k] = 1;
        is_lt[k] = 0;
        if(low[k] < high[k]) {
          mid[k] = (low[k] + high[k]) / 2;
        }
      }
      for(size_t len = 0; len < num_values; len++) {
#pragma _NEC ivdep
        for(size_t k = 0; k < MULTIVALUE_UPPER_BOUND_VLEN; k++) {
          if(low[k] < high[k]) {
            auto value = crnt_to_find[k * num_values + len];
            int crnt_is_eq = (value == sorted[mid[k] * num_values + len]);
            int crnt_is_lt = (value < sorted[mid[k] * num_values + len]);
            if(is_eq[k]) is_lt[k] = crnt_is_lt;
            if(!crnt_is_eq) is_eq[k] = 0;
          }
        }
      }
#pragma _NEC ivdep
      for(size_t k = 0; k < MULTIVALUE_UPPER_BOUND_VLEN; k++) {
        if(low[k] < high[k]) {
          if(is_lt[k] || is_eq[k]) {
            low[k] = mid[k] + 1;
          } else {
            high[k] = mid[k];
          }
        }
      }
    }
    for(size_t k = 0; k < MULTIVALUE_UPPER_BOUND_VLEN; k++) {
      crnt_ret[k] = low[k];
    }
  }

  size_t low2[MULTIVALUE_UPPER_BOUND_VLEN];
  size_t high2[MULTIVALUE_UPPER_BOUND_VLEN];
  size_t mid2[MULTIVALUE_UPPER_BOUND_VLEN];
  int is_eq2[MULTIVALUE_UPPER_BOUND_VLEN];
  int is_lt2[MULTIVALUE_UPPER_BOUND_VLEN];

  // to surpress "may be used uninitialized" warning  
  for(int i = 0; i < MULTIVALUE_UPPER_BOUND_VLEN; i++) {
    low2[i] = 0;
    high2[i] = 0;
    mid2[i] = 0;
    is_eq2[i] = 0;
    is_lt2[i] = 0;
  }

  size_t offset = block_size * MULTIVALUE_UPPER_BOUND_VLEN;
  auto crnt_to_find = to_find + offset * num_values;
  auto crnt_ret = ret + offset;
  for(size_t j = 0; j < rest; j++) {
    low2[j] = 0;
    high2[j] = sorted_size;
  }
  for(size_t j = 0; j < max_iter; j++) {
#pragma _NEC ivdep
    for(size_t k = 0; k < rest; k++) {
      is_eq2[k] = 1;
      is_lt2[k] = 0;
      if(low2[k] < high2[k]) {
        mid2[k] = (low2[k] + high2[k]) / 2;
      }
    }
    for(size_t len = 0; len < num_values; len++) {
#pragma _NEC ivdep
      for(size_t k = 0; k < rest; k++) {
        if(low2[k] < high2[k]) {
          auto value = crnt_to_find[k * num_values + len];
          int crnt_is_eq = (value == sorted[mid2[k] * num_values + len]);
          int crnt_is_lt = (value < sorted[mid2[k] * num_values + len]);
          if(is_eq2[k]) is_lt2[k] = crnt_is_lt;
          if(!crnt_is_eq) is_eq2[k] = 0;
        }
      }
    }
#pragma _NEC ivdep
    for(size_t k = 0; k < rest; k++) {
      if(low2[k] < high2[k]) {
        if(is_lt2[k] || is_eq2[k]) {
          low2[k] = mid2[k] + 1;
        } else {
          high2[k] = mid2[k];
        }
      }
    }
  }
  for(size_t k = 0; k < rest; k++) {
    crnt_ret[k] = low2[k];
  }
}

// values are stored in rowmajor order
template <class T>
std::vector<size_t>
multivalue_upper_bound_desc(const std::vector<T>& sorted,
                            const std::vector<T>& to_find,
                            int num_values) {
  if(sorted.size() % num_values != 0 || to_find.size() % num_values != 0)
    throw std::runtime_error("multivalue_upper_bound_desc: wrong num_values");
  auto to_find_size = to_find.size() / num_values;
  std::vector<size_t> ret(to_find_size);
  if(to_find.size() == 0 || sorted.size() == 0) return ret;
  multivalue_upper_bound_desc(sorted.data(), sorted.size()/num_values,
                              to_find.data(), to_find_size, num_values,
                              ret.data());
  return ret;
}

}

#endif
