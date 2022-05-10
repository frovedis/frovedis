#ifndef DFCOLUMN_HELPER_HPP
#define DFCOLUMN_HELPER_HPP

// helper functions shared by multiple functions
namespace frovedis {

template <class T>
std::vector<size_t> get_local_index_helper(std::vector<T>& val) {
  size_t size = val.size();
  std::vector<size_t> ret(size);
  size_t* retp = &ret[0];
  for(size_t i = 0; i < size; i++) retp[i] = i;
  return ret;
}

template <class T>
std::vector<T> extract_non_null(std::vector<T>& val,
                                std::vector<size_t>& idx,
                                std::vector<size_t>& nulls,
                                std::vector<size_t>& non_null_idx) {
  auto tmp = set_difference(idx, nulls);
  non_null_idx.swap(tmp);
  size_t non_null_size = non_null_idx.size();
  std::vector<T> non_null_val(non_null_size);
  T* non_null_valp = &non_null_val[0];
  T* valp = &val[0];
  size_t* non_null_idxp = &non_null_idx[0];
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < non_null_size; i++) {
    non_null_valp[i] = valp[non_null_idxp[i]];
  }
  return non_null_val;
}

// when contain_nulls == false
template <class T>
std::vector<T> extract_helper2(std::vector<T>& val,
                               std::vector<size_t>& idx) {
  size_t size = idx.size();
  std::vector<T> ret(size);
  T* valp = &val[0];
  T* retp = &ret[0];
  size_t* idxp = &idx[0];
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < size; i++) {
    retp[i] = valp[idxp[i]];
  }
  return ret;
}

template <class T>
std::vector<size_t> calc_hash_base_helper(std::vector<T>& val) {
  size_t size = val.size();
  std::vector<size_t> ret(size);
  T* valp = &val[0];
  size_t* retp = &ret[0];
  for(size_t i = 0; i < size; i++) {
    retp[i] = static_cast<size_t>(valp[i]);
  }
  return ret;
}

template <class T>
struct calc_hash_base_helper2 {
  calc_hash_base_helper2(){}
  calc_hash_base_helper2(int shift) : shift(shift) {}
  void operator()(std::vector<T>& val, std::vector<size_t>& hash_base) {
    size_t size = val.size();
    std::vector<size_t> ret(size);
    T* valp = &val[0];
    size_t* hash_basep = &hash_base[0];
    for(size_t i = 0; i < size; i++) {
      hash_basep[i] =
        ((hash_basep[i] << shift) |
         (hash_basep[i] >> (sizeof(size_t)*CHAR_BIT - shift)))
        + static_cast<size_t>(valp[i]);
    }
  }
  int shift;
  SERIALIZE(shift)
};

template <class T>
std::vector<size_t> find_value(std::vector<T>& val, T tofind) {
  return vector_find_eq(val, tofind);
}

template <class T, class U>
std::vector<U> 
do_static_cast(const std::vector<T>& v, 
               const std::vector<size_t>& nulls) {
  auto ret = vector_astype<U>(v);
  U* retp = ret.data();
  // casting nulls
  U umax = std::numeric_limits<U>::max();
  auto nptr = nulls.data();
  auto nsz = nulls.size();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < nsz; ++i) retp[nptr[i]] = umax;
  return ret;
}

template <class T>
void reset_null(std::vector<T>& val, 
                const std::vector<size_t>& nulls) {
  auto valp = val.data();
  auto nullsp = nulls.data();
  auto size = nulls.size();
  T tmax = std::numeric_limits<T>::max();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < size; i++) valp[nullsp[i]] = tmax;
} 

}
#endif
