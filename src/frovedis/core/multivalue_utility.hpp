#ifndef MULTIVALUE_UTILITY_HPP
#define MULTIVALUE_UTILITY_HPP

#include <vector>

namespace frovedis {

template <class T>
void multivalue_extract(const T* valp, const size_t* idxp,
                        size_t val_size, size_t idx_size,
                        int num_values, T* retp) {
  for(size_t i = 0; i < num_values; i++) {
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t j = 0; j < idx_size; j++) {
      retp[j * num_values + i] = valp[idxp[j] * num_values + i];
    }
  }
}

template <class T>
std::vector<T> multivalue_extract(const std::vector<T>& val,
                                  const std::vector<size_t>& idx,
                                  int num_values) {
  if(val.size() % num_values != 0)
    throw std::runtime_error("multivalue_extract: wrong num_values");
  std::vector<T> ret(idx.size() * num_values);
  multivalue_extract(val.data(), idx.data(), val.size() / num_values, 
                     idx.size(), num_values, ret.data());
  return ret;
}

}
#endif

