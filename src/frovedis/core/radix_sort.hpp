#ifndef RADIX_SORT_HPP
#define RADIX_SORT_HPP

#include <vector>
#include <stdint.h>

#include "prefix_sum.hpp"

#if defined(_SX) || defined(__ve__) // might be used in x86
#define RADIX_SORT_VLEN 1024
#define RADIX_SORT_VLEN_EACH 256
// if size is less than this, use insertion sort instead
#define SWITCH_INSERTION_THR 512
#else
#define RADIX_SORT_VLEN 4
#define RADIX_SORT_VLEN_EACH 1
#define SWITCH_INSERTION_THR 64
#endif

#define RADIX_SORT_ALIGN_SIZE 128

#define CONTAIN_NEGATIVE_SIZE 65536

namespace frovedis {

#include "./radix_sort.incl"
#define RADIX_SORT_KV_PAIR
#include "./radix_sort.incl"
#undef RADIX_SORT_KV_PAIR

template <class K, class V>
void insertion_sort(K* data, V* val, size_t size) {
  int i, j;
  for (i = 1; i < size; i++) {
    auto tmp = data[i];
    if (data[i - 1] > tmp) {
      auto vtmp = val[i];
      j = i;
      do {
        data[j] = data[j - 1];
        val[j] = val[j - 1];
        j--;
      } while (j > 0 && data[j - 1] > tmp);
      data[j] = tmp;
      val[j] = vtmp;
    }
  }
}

template <class K>
void insertion_sort(K* data, size_t size) {
  int i, j;
  for (i = 1; i < size; i++) {
    auto tmp = data[i];
    if (data[i - 1] > tmp) {
      j = i;
      do {
        data[j] = data[j - 1];
        j--;
      } while (j > 0 && data[j - 1] > tmp);
      data[j] = tmp;
    }
  }
}

template <class K, class V>
void insertion_sort_desc(K* data, V* val, size_t size) {
  int i, j;
  for (i = 1; i < size; i++) {
    auto tmp = data[i];
    if (data[i - 1] < tmp) {
      auto vtmp = val[i];
      j = i;
      do {
        data[j] = data[j - 1];
        val[j] = val[j - 1];
        j--;
      } while (j > 0 && data[j - 1] < tmp);
      data[j] = tmp;
      val[j] = vtmp;
    }
  }
}

template <class K>
void insertion_sort_desc(K* data, size_t size) {
  int i, j;
  for (i = 1; i < size; i++) {
    auto tmp = data[i];
    if (data[i - 1] < tmp) {
      j = i;
      do {
        data[j] = data[j - 1];
        j--;
      } while (j > 0 && data[j - 1] < tmp);
      data[j] = tmp;
    }
  }
}

template <class K>
bool contain_negative(K* key_array, size_t size) {
  K min = 0;
  size_t i = 0;
  if(size > CONTAIN_NEGATIVE_SIZE) {
    for(; i < size - CONTAIN_NEGATIVE_SIZE; i += CONTAIN_NEGATIVE_SIZE) {
      auto k = key_array + i;
      for(size_t j = 0; j < CONTAIN_NEGATIVE_SIZE; j++) {
        if(k[j] < min) min = k[j];
      }
      if(min < 0) return true; 
    }
  }
  for(; i < size; i++) {
    if(key_array[i] < 0) return true;
  }
  return false;
}

inline
bool contain_negative(unsigned int* key_array, size_t size) {
  return false;
}

inline
bool contain_negative(unsigned long* key_array, size_t size) {
  return false;
}

inline
bool contain_negative(unsigned long long* key_array, size_t size) {
  return false;
}

template <class K>
size_t get_max_key_size(K* key_array, size_t size) {
  auto key_size = sizeof(K);
  K max = 0;
  for(size_t i = 0; i < size; i++) {
    if(key_array[i] > max) max = key_array[i];
  }
  size_t max_key_size = key_size;
  size_t tmp = 0xFF;
  for(size_t i = 1; i < key_size + 1; i++) {
    if(max <= tmp) {
      max_key_size = i;
      break;
    } else {
      tmp = (tmp << 8) | 0xFF;
    }
  }
  return max_key_size;
}

template <class K>
void flip_top(K* key_array, size_t size) {
  if(sizeof(K) == 4) {
    for(size_t i = 0; i < size; i++) {
      key_array[i] ^= 0x80000000;
    }
  } else { // 8
    for(size_t i = 0; i < size; i++) {
      key_array[i] ^= 0x8000000000000000;
    }
  }
}

// see http://stereopsis.com/radix.html
inline void float_flip(float* key_array, size_t size) {
  uint32_t* key = reinterpret_cast<uint32_t*>(key_array);
  for(size_t i = 0; i < size; i++) {
    key[i] ^= -int32_t(key[i] >> 31) | 0x80000000;
  }
}

inline void float_flip_reverse(float* key_array, size_t size) {
  uint32_t* key = reinterpret_cast<uint32_t*>(key_array);
  for(size_t i = 0; i < size; i++) {
    key[i] ^= ((key[i] >> 31) - 1) | 0x80000000;
  }
}

inline void double_flip(double* key_array, size_t size) {
  uint64_t* key = reinterpret_cast<uint64_t*>(key_array);
  for(size_t i = 0; i < size; i++) {
    key[i] ^= -int64_t(key[i] >> 63) | 0x8000000000000000;
  }
}

inline void double_flip_reverse(double* key_array, size_t size) {
  uint64_t* key = reinterpret_cast<uint64_t*>(key_array);
  for(size_t i = 0; i < size; i++) {
    key[i] ^= ((key[i] >> 63) - 1) | 0x8000000000000000;
  }
}

template <class K, class V>
void radix_sort(K* key_array, V* val_array, size_t size,
                bool positive_only = false) {
  if(size < SWITCH_INSERTION_THR) {
    insertion_sort(key_array, val_array, size);
    return;
  }
  if(positive_only) {
    auto max_key_size = get_max_key_size(key_array, size);
    radix_sort_impl(key_array, val_array, size, max_key_size);
  } else {
    auto neg = contain_negative(key_array, size);
    size_t max_key_size;
    if(neg) {
      flip_top(key_array, size);
      max_key_size = sizeof(K);
    } else {
      max_key_size = get_max_key_size(key_array, size);
    }
    radix_sort_impl(key_array, val_array, size, max_key_size);
    if(neg) flip_top(key_array, size);
  }  
}

template <class K>
void radix_sort(K* key_array, size_t size, bool positive_only = false) {
  if(size < SWITCH_INSERTION_THR) {
    insertion_sort(key_array, size);
    return;
  }
  if(positive_only) {
    auto max_key_size = get_max_key_size(key_array, size);
    radix_sort_impl(key_array, size, max_key_size);
  } else {
    auto neg = contain_negative(key_array, size);
    size_t max_key_size;
    if(neg) {
      flip_top(key_array, size);
      max_key_size = sizeof(K);
    } else {
      max_key_size = get_max_key_size(key_array, size);
    }
    radix_sort_impl(key_array, size, max_key_size);
    if(neg) flip_top(key_array, size);
  }  
}

template <class V>
void radix_sort(float* key_array, V* val_array, size_t size,
                bool positive_only = false) {
  if(size < SWITCH_INSERTION_THR) {
    insertion_sort(key_array, val_array, size);
    return;
  }
  auto intkey = reinterpret_cast<uint32_t*>(key_array);
  if(positive_only) {
    auto max_key_size = get_max_key_size(intkey, size);
    radix_sort_impl(intkey, val_array, size, max_key_size);
  } else {
    auto neg = contain_negative(key_array, size);
    size_t max_key_size;
    if(neg) {
      float_flip(key_array, size);
      max_key_size = sizeof(float);
    } else {
      max_key_size = get_max_key_size(intkey, size);
    }
    radix_sort_impl(intkey, val_array, size, max_key_size);
    if(neg) float_flip_reverse(key_array, size);
  }  
}

inline 
void radix_sort(float* key_array, size_t size, bool positive_only = false) {
  if(size < SWITCH_INSERTION_THR) {
    insertion_sort(key_array, size);
    return;
  }
  auto intkey = reinterpret_cast<uint32_t*>(key_array);
  if(positive_only) {
    auto max_key_size = get_max_key_size(intkey, size);
    radix_sort_impl(intkey, size, max_key_size);
  } else {
    auto neg = contain_negative(key_array, size);
    size_t max_key_size;
    if(neg) {
      float_flip(key_array, size);
      max_key_size = sizeof(float);
    } else {
      max_key_size = get_max_key_size(intkey, size);
    }
    radix_sort_impl(intkey, size, max_key_size);
    if(neg) float_flip_reverse(key_array, size);
  }  
}

template <class V>
void radix_sort(double* key_array, V* val_array, size_t size,
                bool positive_only = false) {
  if(size < SWITCH_INSERTION_THR) {
    insertion_sort(key_array, val_array, size);
    return;
  }
  auto intkey = reinterpret_cast<uint64_t*>(key_array);
  if(positive_only) {
    auto max_key_size = get_max_key_size(intkey, size);
    radix_sort_impl(intkey, val_array, size, max_key_size);
  } else {
    auto neg = contain_negative(key_array, size);
    size_t max_key_size;
    if(neg) {
      double_flip(key_array, size);
      max_key_size = sizeof(double);
    } else {
      max_key_size = get_max_key_size(intkey, size);
    }
    radix_sort_impl(intkey, val_array, size, max_key_size);
    if(neg) double_flip_reverse(key_array, size);
  }
}

inline
void radix_sort(double* key_array, size_t size, bool positive_only = false) {
  if(size < SWITCH_INSERTION_THR) {
    insertion_sort(key_array, size);
    return;
  }
  auto intkey = reinterpret_cast<uint64_t*>(key_array);
  if(positive_only) {
    auto max_key_size = get_max_key_size(intkey, size);
    radix_sort_impl(intkey, size, max_key_size);
  } else {
    auto neg = contain_negative(key_array, size);
    size_t max_key_size;
    if(neg) {
      double_flip(key_array, size);
      max_key_size = sizeof(double);
    } else {
      max_key_size = get_max_key_size(intkey, size);
    }
    radix_sort_impl(intkey, size, max_key_size);
    if(neg) double_flip_reverse(key_array, size);
  }
}

template <class K, class V>
void radix_sort_desc(K* key_array, V* val_array, size_t size,
                     bool positive_only = false) {
  if(size < SWITCH_INSERTION_THR) {
    insertion_sort_desc(key_array, val_array, size);
    return;
  }
  if(positive_only) {
    auto max_key_size = get_max_key_size(key_array, size);
    radix_sort_desc_impl(key_array, val_array, size, max_key_size);
  } else {
    auto neg = contain_negative(key_array, size);
    size_t max_key_size;
    if(neg) {
      flip_top(key_array, size);
      max_key_size = sizeof(K);
    } else {
      max_key_size = get_max_key_size(key_array, size);
    }
    radix_sort_desc_impl(key_array, val_array, size, max_key_size);
    if(neg) flip_top(key_array, size);
  }  
}

template <class K>
void radix_sort_desc(K* key_array, size_t size, bool positive_only = false) {
  if(size < SWITCH_INSERTION_THR) {
    insertion_sort_desc(key_array, size);
    return;
  }
  if(positive_only) {
    auto max_key_size = get_max_key_size(key_array, size);
    radix_sort_desc_impl(key_array, size, max_key_size);
  } else {
    auto neg = contain_negative(key_array, size);
    size_t max_key_size;
    if(neg) {
      flip_top(key_array, size);
      max_key_size = sizeof(K);
    } else {
      max_key_size = get_max_key_size(key_array, size);
    }
    radix_sort_desc_impl(key_array, size, max_key_size);
    if(neg) flip_top(key_array, size);
  }  
}

template <class V>
void radix_sort_desc(float* key_array, V* val_array, size_t size,
                     bool positive_only = false) {
  if(size < SWITCH_INSERTION_THR) {
    insertion_sort_desc(key_array, val_array, size);
    return;
  }
  auto intkey = reinterpret_cast<uint32_t*>(key_array);
  if(positive_only) {
    auto max_key_size = get_max_key_size(intkey, size);
    radix_sort_desc_impl(intkey, val_array, size, max_key_size);
  } else {
    auto neg = contain_negative(key_array, size);
    size_t max_key_size;
    if(neg) {
      float_flip(key_array, size);
      max_key_size = sizeof(float);
    } else {
      max_key_size = get_max_key_size(intkey, size);
    }
    radix_sort_desc_impl(intkey, val_array, size, max_key_size);
    if(neg) float_flip_reverse(key_array, size);
  }  
}

inline 
void radix_sort_desc(float* key_array, size_t size,
                     bool positive_only = false) {
  if(size < SWITCH_INSERTION_THR) {
    insertion_sort_desc(key_array, size);
    return;
  }
  auto intkey = reinterpret_cast<uint32_t*>(key_array);
  if(positive_only) {
    auto max_key_size = get_max_key_size(intkey, size);
    radix_sort_desc_impl(intkey, size, max_key_size);
  } else {
    auto neg = contain_negative(key_array, size);
    size_t max_key_size;
    if(neg) {
      float_flip(key_array, size);
      max_key_size = sizeof(float);
    } else {
      max_key_size = get_max_key_size(intkey, size);
    }
    radix_sort_desc_impl(intkey, size, max_key_size);
    if(neg) float_flip_reverse(key_array, size);
  }  
}

template <class V>
void radix_sort_desc(double* key_array, V* val_array, size_t size,
                     bool positive_only = false) {
  if(size < SWITCH_INSERTION_THR) {
    insertion_sort_desc(key_array, val_array, size);
    return;
  }
  auto intkey = reinterpret_cast<uint64_t*>(key_array);
  if(positive_only) {
    auto max_key_size = get_max_key_size(intkey, size);
    radix_sort_desc_impl(intkey, val_array, size, max_key_size);
  } else {
    auto neg = contain_negative(key_array, size);
    size_t max_key_size;
    if(neg) {
      double_flip(key_array, size);
      max_key_size = sizeof(double);
    } else {
      max_key_size = get_max_key_size(intkey, size);
    }
    radix_sort_desc_impl(intkey, val_array, size, max_key_size);
    if(neg) double_flip_reverse(key_array, size);
  }
}

inline
void radix_sort_desc(double* key_array, size_t size,
                     bool positive_only = false) {
  if(size < SWITCH_INSERTION_THR) {
    insertion_sort_desc(key_array, size);
    return;
  }
  auto intkey = reinterpret_cast<uint64_t*>(key_array);
  if(positive_only) {
    auto max_key_size = get_max_key_size(intkey, size);
    radix_sort_desc_impl(intkey, size, max_key_size);
  } else {
    auto neg = contain_negative(key_array, size);
    size_t max_key_size;
    if(neg) {
      double_flip(key_array, size);
      max_key_size = sizeof(double);
    } else {
      max_key_size = get_max_key_size(intkey, size);
    }
    radix_sort_desc_impl(intkey, size, max_key_size);
    if(neg) double_flip_reverse(key_array, size);
  }
}

template <class K, class V>
void radix_sort(std::vector<K>& key_array, std::vector<V>& val_array,
                bool positive_only = false) {
  size_t size = key_array.size();
  if(val_array.size() != size)
    throw std::runtime_error("radix_sort: different size of arrays");
  radix_sort(key_array.data(), val_array.data(), size, positive_only);
}

template <class K>
void radix_sort(std::vector<K>& key_array, bool positive_only = false) {
  size_t size = key_array.size();
  radix_sort(key_array.data(), size, positive_only);
}

template <class K, class V>
void radix_sort_desc(std::vector<K>& key_array, std::vector<V>& val_array,
                     bool positive_only = false) {
  size_t size = key_array.size();
  if(val_array.size() != size)
    throw std::runtime_error("radix_sort: different size of arrays");
  radix_sort_desc(key_array.data(), val_array.data(), size, positive_only);
}

template <class K>
void radix_sort_desc(std::vector<K>& key_array, bool positive_only = false) {
  size_t size = key_array.size();
  radix_sort_desc(key_array.data(), size, positive_only);
}

}
#endif
