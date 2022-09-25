#include "dfcolumn.hpp"

#include <limits>
#include <climits>

#define GROUPBY_VLEN 256
// count distinct uses same implementation for both VE and x86
#ifdef __ve__
#define GROUPBY_COUNT_DISTINCT_VLEN 256
#else
#define GROUPBY_COUNT_DISTINCT_VLEN 4
#endif

namespace frovedis {

template <class T>
size_t typed_dfcolumn<T>::count() {
  size_t size = val.template viewas_dvector<T>().size();
  if(contain_nulls) {
    size_t nullsize = nulls.template viewas_dvector<size_t>().size();
    return size - nullsize;
  } else return size;
}

template <class T>
T sum_helper2(std::vector<T>& val,
              std::vector<size_t>& nulls) {
  size_t valsize = val.size();
  size_t nullssize = nulls.size();
  T* valp = &val[0];
  size_t* nullsp = &nulls[0];
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < nullssize; i++) {
    valp[nullsp[i]] = 0;
  }
  T total = 0;
  for(size_t i = 0; i < valsize; i++) {
    total += valp[i];
  }
  T max = std::numeric_limits<T>::max();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < nullssize; i++) {
    valp[nullsp[i]] = max;
  }
  return total;
}

template <class T>
T typed_dfcolumn<T>::sum() {
  auto sums = val.map(sum_helper2<T>, nulls).gather();
  T* sumsp = &sums[0];
  size_t size = sums.size();
  T total = 0;
  for(size_t i = 0; i < size; i++) {
    total += sumsp[i];
  }
  return total;
}

template <class T>
double typed_dfcolumn<T>::avg() {
  size_t ct = count();
  double total = static_cast<double>(sum());
  return total/static_cast<double>(ct);
}

template <class T>
double mean_helper2(std::vector<T>& val,
                    std::vector<size_t>& nulls,
                    double mean) {
  size_t valsize = val.size();
  size_t nullssize = nulls.size();
  T* valp = &val[0];
  size_t* nullsp = &nulls[0];
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < nullssize; i++) {
    valp[nullsp[i]] = mean; // mean - mean would become zero in error calculation
  }
  double total = 0;
  for(size_t i = 0; i < valsize; i++) {
    total += (valp[i] - mean) * (valp[i] - mean);
  }
  T max = std::numeric_limits<T>::max();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < nullssize; i++) {
    valp[nullsp[i]] = max;
  }
  return total;
}

template <class T>
double typed_dfcolumn<T>::var(double ddof) {
  size_t size = count();
  double mean = avg();
  auto ssdm = val.map(mean_helper2<T>, nulls, broadcast(mean))
                 .reduce(frovedis::add<double>);
  double n_ddof = size - ddof;
  if (n_ddof > 0) return ssdm / n_ddof;
  return std::numeric_limits<double>::quiet_NaN();
}

template <class T>
double mean_helper3(std::vector<T>& val,
                    std::vector<size_t>& nulls,
                    double mean) {
  size_t valsize = val.size();
  size_t nullssize = nulls.size();
  T* valp = &val[0];
  size_t* nullsp = &nulls[0];
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < nullssize; i++) {
    valp[nullsp[i]] = mean; // mean - mean would become zero in error calculation
  }
  double total = 0;
  for(size_t i = 0; i < valsize; i++) {
    total += fabs(valp[i] - mean); //TODO: replace->ensure subtraction of smaller one from larger one.
  }
  T max = std::numeric_limits<T>::max();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < nullssize; i++) {
    valp[nullsp[i]] = max;
  }
  return total;
}

template <class T>
double typed_dfcolumn<T>::mad() {
  size_t size = count();
  double mean = avg();
  auto res = val.map(mean_helper3<T>, nulls, broadcast(mean))
                 .reduce(frovedis::add<double>);
  return res / static_cast<double>(size);
}

template <class T>
double typed_dfcolumn<T>::std(double ddof) {
  double ret = var(ddof);
  if (isnan(ret)) return ret;
  return std::sqrt(ret);
}

template <class T>
double typed_dfcolumn<T>::sem(double ddof) {
  double ret = std(ddof);
  if (isnan(ret)) return ret;
  return ret/std::sqrt(count());
}

template <class T>
T max_helper2(std::vector<T>& val,
              std::vector<size_t>& nulls) {
  size_t valsize = val.size();
  size_t nullssize = nulls.size();
  T* valp = &val[0];
  size_t* nullsp = &nulls[0];
  T min = std::numeric_limits<T>::lowest();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < nullssize; i++) {
    valp[nullsp[i]] = min;
  }
  T current_max = std::numeric_limits<T>::lowest();
  for(size_t i = 0; i < valsize; i++) {
    if(current_max < valp[i]) current_max = valp[i];
  }
  T max = std::numeric_limits<T>::max();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < nullssize; i++) {
    valp[nullsp[i]] = max;
  }
  return current_max;
}

template <class T>
T typed_dfcolumn<T>::max() {
  auto maxs = val.map(max_helper2<T>, nulls).gather();
  T* maxsp = &maxs[0];
  size_t size = maxs.size();
  T current_max = std::numeric_limits<T>::lowest();
  for(size_t i = 0; i < size; i++) {
    if(current_max < maxsp[i]) current_max = maxsp[i];
  }
  return current_max;
}

template <class T>
T min_helper2(std::vector<T>& val,
              std::vector<size_t>& nulls/* not used because already max */) {
  size_t valsize = val.size();
  T* valp = &val[0];
  T current_min = std::numeric_limits<T>::max();
  for(size_t i = 0; i < valsize; i++) {
    if(current_min > valp[i]) current_min = valp[i];
  }
  return current_min;
}

template <class T>
T typed_dfcolumn<T>::min() {
  auto mins = val.map(min_helper2<T>, nulls).gather();
  T* minsp = &mins[0];
  size_t size = mins.size();
  T current_min = std::numeric_limits<T>::max();
  for(size_t i = 0; i < size; i++) {
    if(current_min > minsp[i]) current_min = minsp[i];
  }
  return current_min;
}

template <class T>
std::vector<T> first_helper2(std::vector<T>& val,
                             bool ignore_nulls) {
  auto val_size = val.size();
  if(!ignore_nulls) {
    if(val_size != 0) return {val[0]};
    else return std::vector<T>();
  } else {
    if(val_size != 0) {
      T max = std::numeric_limits<T>::max();
      T ret = max;
      auto valp = val.data();
      for(size_t i = 0; i < val_size; i++) {
        if(valp[i] == max) continue;
        else {
          ret = valp[i];
          break;
        }
      }
      return {ret};
    } else {
      return std::vector<T>();
    }
  }
}

template <class T>
T typed_dfcolumn<T>::first(bool ignore_nulls) {
#ifndef DONOT_ALLOW_MAX_AS_VALUE
  throw std::runtime_error
    ("define DONOT_ALLOW_MAX_AS_VALUE to use first");
#endif
  auto firsts = val.map(first_helper2<T>,broadcast(ignore_nulls)).gather();
  auto firsts_size = firsts.size();
  T max = std::numeric_limits<T>::max();
  T ret = max;
  for(size_t i = 0; i < firsts_size; i++) {
    auto crnt = firsts[firsts_size - 1 - i];
    if(!ignore_nulls) {
      if(crnt.size() != 0) ret = crnt[0];
    } else {
      if(crnt.size() != 0 && crnt[0] != max) ret = crnt[0];
    }
  }
  return ret;
}

template <class T>
std::vector<T> last_helper2(std::vector<T>& val,
                            bool ignore_nulls) {
  auto val_size = val.size();
  if(!ignore_nulls) {
    if(val_size != 0) return {val[val_size-1]};
    else return std::vector<T>();
  } else {
    if(val_size != 0) {
      T max = std::numeric_limits<T>::max();
      T ret = max;
      auto valp = val.data();
      for(size_t i = 0; i < val_size; i++) {
        if(valp[val_size - 1 - i] == max) continue;
        else {
          ret = valp[val_size - 1 - i];
          break;
        }
      }
      return {ret};
    } else {
      return std::vector<T>();
    }
  }
}

template <class T>
T typed_dfcolumn<T>::last(bool ignore_nulls) {
#ifndef DONOT_ALLOW_MAX_AS_VALUE
  throw std::runtime_error
    ("define DONOT_ALLOW_MAX_AS_VALUE to use last");
#endif
  auto lasts = val.map(last_helper2<T>,broadcast(ignore_nulls)).gather();
  auto lasts_size = lasts.size();
  T max = std::numeric_limits<T>::max();
  T ret = max;
  for(size_t i = 0; i < lasts_size; i++) {
    auto crnt = lasts[i];
    if(!ignore_nulls) {
      if(crnt.size() != 0) ret = crnt[0];
    } else {
      if(crnt.size() != 0 && crnt[0] != max) ret = crnt[0];
    }
  }
  return ret;
}

template <class T>
T typed_dfcolumn<T>::at(size_t i) {
  auto sizes = val.template viewas_dvector<T>().sizes();
  auto nproc = sizes.size();
  std::vector<size_t> starts(nproc); starts[0] = 0;
  for(size_t i = 1; i < nproc; ++i) starts[i] = starts[i - 1] + sizes[i - 1];
  auto myst = make_node_local_scatter(starts);
  auto mysz = make_node_local_scatter(sizes);
  return val.map(+[](const std::vector<T>& val, size_t myst, 
                     size_t mysz, size_t index) {
         std::vector<T> ret;
         if (myst <= index && index < myst + mysz) 
           ret.push_back(val[index - myst]);
         return ret;
       }, myst, mysz, broadcast(i)).template moveto_dvector<T>().gather()[0];
}


template size_t typed_dfcolumn<int>::count();
template size_t typed_dfcolumn<unsigned int>::count();
template size_t typed_dfcolumn<long>::count();
template size_t typed_dfcolumn<unsigned long>::count();
template size_t typed_dfcolumn<float>::count();
template size_t typed_dfcolumn<double>::count();

template int typed_dfcolumn<int>::sum();
template unsigned int typed_dfcolumn<unsigned int>::sum();
template long typed_dfcolumn<long>::sum();
template unsigned long typed_dfcolumn<unsigned long>::sum();
template float typed_dfcolumn<float>::sum();
template double typed_dfcolumn<double>::sum();

template double typed_dfcolumn<int>::avg();
template double typed_dfcolumn<unsigned int>::avg();
template double typed_dfcolumn<long>::avg();
template double typed_dfcolumn<unsigned long>::avg();
template double typed_dfcolumn<float>::avg();
template double typed_dfcolumn<double>::avg();

template double typed_dfcolumn<int>::var(double ddof);
template double typed_dfcolumn<unsigned int>::var(double ddof);
template double typed_dfcolumn<long>::var(double ddof);
template double typed_dfcolumn<unsigned long>::var(double ddof);
template double typed_dfcolumn<float>::var(double ddof);
template double typed_dfcolumn<double>::var(double ddof);

template double typed_dfcolumn<int>::mad();
template double typed_dfcolumn<unsigned int>::mad();
template double typed_dfcolumn<long>::mad();
template double typed_dfcolumn<unsigned long>::mad();
template double typed_dfcolumn<float>::mad();
template double typed_dfcolumn<double>::mad();

template double typed_dfcolumn<int>::std(double ddof);
template double typed_dfcolumn<unsigned int>::std(double ddof);
template double typed_dfcolumn<long>::std(double ddof);
template double typed_dfcolumn<unsigned long>::std(double ddof);
template double typed_dfcolumn<float>::std(double ddof);
template double typed_dfcolumn<double>::std(double ddof);

template double typed_dfcolumn<int>::sem(double ddof);
template double typed_dfcolumn<unsigned int>::sem(double ddof);
template double typed_dfcolumn<long>::sem(double ddof);
template double typed_dfcolumn<unsigned long>::sem(double ddof);
template double typed_dfcolumn<float>::sem(double ddof);
template double typed_dfcolumn<double>::sem(double ddof);

template int typed_dfcolumn<int>::max();
template unsigned int typed_dfcolumn<unsigned int>::max();
template long typed_dfcolumn<long>::max();
template unsigned long typed_dfcolumn<unsigned long>::max();
template float typed_dfcolumn<float>::max();
template double typed_dfcolumn<double>::max();

template int typed_dfcolumn<int>::min();
template unsigned int typed_dfcolumn<unsigned int>::min();
template long typed_dfcolumn<long>::min();
template unsigned long typed_dfcolumn<unsigned long>::min();
template float typed_dfcolumn<float>::min();
template double typed_dfcolumn<double>::min();

template int typed_dfcolumn<int>::first(bool ignore_nulls);
template unsigned int typed_dfcolumn<unsigned int>::first(bool ignore_nulls);
template long typed_dfcolumn<long>::first(bool ignore_nulls);
template unsigned long typed_dfcolumn<unsigned long>::first(bool ignore_nulls);
template float typed_dfcolumn<float>::first(bool ignore_nulls);
template double typed_dfcolumn<double>::first(bool ignore_nulls);

template int typed_dfcolumn<int>::last(bool ignore_nulls);
template unsigned int typed_dfcolumn<unsigned int>::last(bool ignore_nulls);
template long typed_dfcolumn<long>::last(bool ignore_nulls);
template unsigned long typed_dfcolumn<unsigned long>::last(bool ignore_nulls);
template float typed_dfcolumn<float>::last(bool ignore_nulls);
template double typed_dfcolumn<double>::last(bool ignore_nulls);

template int typed_dfcolumn<int>::at(size_t i);
template unsigned int typed_dfcolumn<unsigned int>::at(size_t i);
template long typed_dfcolumn<long>::at(size_t i);
template unsigned long typed_dfcolumn<unsigned long>::at(size_t i);
template float typed_dfcolumn<float>::at(size_t i);
template double typed_dfcolumn<double>::at(size_t i);

using namespace std;
// ----- dic_string -----

std::string
typed_dfcolumn<dic_string>::first(bool ignore_nulls) {
#ifndef DONOT_ALLOW_MAX_AS_VALUE
  throw std::runtime_error
    ("define DONOT_ALLOW_MAX_AS_VALUE to use first");
#endif
  auto firsts = val.map(first_helper2<size_t>,broadcast(ignore_nulls)).gather();
  auto firsts_size = firsts.size();
  size_t max = std::numeric_limits<size_t>::max();
  size_t ret = max;
  for(size_t i = 0; i < firsts_size; i++) {
    auto crnt = firsts[firsts_size - 1 - i];
    if(!ignore_nulls) {
      if(crnt.size() != 0) ret = crnt[0];
    } else {
      if(crnt.size() != 0 && crnt[0] != max) ret = crnt[0];
    }
  }
  if(ret == std::numeric_limits<size_t>::max()) return "NULL";
  else {
    auto ws = dic->index_to_words({ret});
    auto st = words_to_vector_string(ws);
    return st[0];
  }
}

std::string
typed_dfcolumn<dic_string>::last(bool ignore_nulls) {
#ifndef DONOT_ALLOW_MAX_AS_VALUE
  throw std::runtime_error
    ("define DONOT_ALLOW_MAX_AS_VALUE to use last");
#endif
  auto lasts = val.map(last_helper2<size_t>,broadcast(ignore_nulls)).gather();
  auto lasts_size = lasts.size();
  size_t max = std::numeric_limits<size_t>::max();
  size_t ret = max;
  for(size_t i = 0; i < lasts_size; i++) {
    auto crnt = lasts[i];
    if(!ignore_nulls) {
      if(crnt.size() != 0) ret = crnt[0];
    } else {
      if(crnt.size() != 0 && crnt[0] != max) ret = crnt[0];
    }
  }
  if(ret == std::numeric_limits<size_t>::max()) return "NULL";
  else {
    auto ws = dic->index_to_words({ret});
    auto st = words_to_vector_string(ws);
    return st[0];
  }
}

size_t typed_dfcolumn<dic_string>::count() {
  size_t size = val.viewas_dvector<size_t>().size();
  if(contain_nulls) {
    size_t nullsize = nulls.template viewas_dvector<size_t>().size();
    return size - nullsize;
  } else return size;
}

// ----- raw_string -----

size_t typed_dfcolumn<raw_string>::count() {
  size_t size = this->size();
  if(contain_nulls) {
    size_t nullsize = nulls.template viewas_dvector<size_t>().size();
    return size - nullsize;
  } else return size;
}

// bool: true if NULL
std::vector<std::pair<bool, std::string>>
raw_string_first_helper(compressed_words& cws,
                        std::vector<size_t>& nulls,
                        bool ignore_nulls) {
  auto num_words = cws.order.size();
  if(ignore_nulls == true) {
    std::vector<size_t> iota(num_words);
    auto iotap = iota.data();
    for(size_t i = 0; i < num_words; i++) iotap[i] = i;
    auto non_null = set_difference(iota, nulls); // TODO: inefficient
    if(non_null.size() == 0) {
      if(num_words != 0) return {std::make_pair(true,std::string("NULL"))};
      else return std::vector<std::pair<bool, std::string>>();
    } else {
      auto vs = words_to_vector_string(cws.extract({non_null[0]}).decompress());
      return {std::make_pair(false, vs[0])};
    }
  } else {
    if(num_words == 0) {
      return std::vector<std::pair<bool, std::string>>();
    } else {
      if(nulls.size() != 0 && nulls[0] == 0) {
        return {std::make_pair(true,std::string("NULL"))};
      } else {
        auto vs = words_to_vector_string(cws.extract({0}).decompress());
        return {std::make_pair(false, vs[0])};
      }
    }
  }
}

std::string typed_dfcolumn<raw_string>::first(bool ignore_nulls) {
  auto firsts = comp_words.map(raw_string_first_helper,
                               nulls,
                               broadcast(ignore_nulls)).gather();
  auto firsts_size = firsts.size();
  std::string ret = "NULL";
  for(size_t i = 0; i < firsts_size; i++) {
    auto crnt = firsts[firsts_size - 1 - i];
    if(!ignore_nulls) {
      if(crnt.size() != 0) ret = crnt[0].second;
    } else {
      if(crnt.size() != 0 && !crnt[0].first) ret = crnt[0].second;
    }
  }
  return ret;
}

std::vector<std::pair<bool, std::string>>
raw_string_last_helper(compressed_words& cws,
                       std::vector<size_t>& nulls,
                       bool ignore_nulls) {
  auto num_words = cws.order.size();
  if(ignore_nulls == true) {
    std::vector<size_t> iota(num_words);
    auto iotap = iota.data();
    for(size_t i = 0; i < num_words; i++) iotap[i] = i;
    auto non_null = set_difference(iota, nulls); // TODO: inefficient
    if(non_null.size() == 0) {
      if(num_words != 0) return {std::make_pair(true,std::string("NULL"))};
      else return std::vector<std::pair<bool, std::string>>();
    } else {
      auto vs = words_to_vector_string
        (cws.extract({non_null[non_null.size()-1]}).decompress());
      return {std::make_pair(false, vs[0])};
    }
  } else {
    if(num_words == 0) {
      return std::vector<std::pair<bool, std::string>>();
    } else {
      if(nulls.size() != 0 && nulls[0] == num_words-1) {
        return {std::make_pair(true,std::string("NULL"))};
      } else {
        auto vs = words_to_vector_string
          (cws.extract({num_words-1}).decompress());
        return {std::make_pair(false, vs[0])};
      }
    }
  }
}

std::string typed_dfcolumn<raw_string>::last(bool ignore_nulls) {
  auto lasts = comp_words.map(raw_string_last_helper,
                              nulls,
                              broadcast(ignore_nulls)).gather();
  auto lasts_size = lasts.size();
  std::string ret = "NULL";
  for(size_t i = 0; i < lasts_size; i++) {
    auto crnt = lasts[i];
    if(!ignore_nulls) {
      if(crnt.size() != 0) ret = crnt[0].second;
    } else {
      if(crnt.size() != 0 && !crnt[0].first) ret = crnt[0].second;
    }
  }
  return ret;
}

// ----- string -----
std::string
typed_dfcolumn<string>::first(bool ignore_nulls) {
#ifndef DONOT_ALLOW_MAX_AS_VALUE
  throw std::runtime_error
    ("define DONOT_ALLOW_MAX_AS_VALUE to use first");
#endif
  auto firsts = val.map(first_helper2<size_t>,broadcast(ignore_nulls)).gather();
  auto firsts_size = firsts.size();
  size_t max = std::numeric_limits<size_t>::max();
  size_t ret = max;
  for(size_t i = 0; i < firsts_size; i++) {
    auto crnt = firsts[firsts_size - 1 - i];
    if(!ignore_nulls) {
      if(crnt.size() != 0) ret = crnt[0];
    } else {
      if(crnt.size() != 0 && crnt[0] != max) ret = crnt[0];
    }
  }
  auto gdic_idx = dic_idx->gather();    
  if(ret == numeric_limits<size_t>::max()) return "NULL";
  else {
    size_t nodemask = (size_t(1) << DFNODESHIFT) - 1;
    auto node = ret >> DFNODESHIFT;
    auto idx = ret & nodemask;
    return gdic_idx[node][idx];
  }
}

std::string
typed_dfcolumn<string>::last(bool ignore_nulls) {
#ifndef DONOT_ALLOW_MAX_AS_VALUE
  throw std::runtime_error
    ("define DONOT_ALLOW_MAX_AS_VALUE to use last");
#endif
  auto lasts = val.map(last_helper2<size_t>,broadcast(ignore_nulls)).gather();
  auto lasts_size = lasts.size();
  size_t max = std::numeric_limits<size_t>::max();
  size_t ret = max;
  for(size_t i = 0; i < lasts_size; i++) {
    auto crnt = lasts[i];
    if(!ignore_nulls) {
      if(crnt.size() != 0) ret = crnt[0];
    } else {
      if(crnt.size() != 0 && crnt[0] != max) ret = crnt[0];
    }
  }
  auto gdic_idx = dic_idx->gather();    
  if(ret == numeric_limits<size_t>::max()) return "NULL";
  else {
    size_t nodemask = (size_t(1) << DFNODESHIFT) - 1;
    auto node = ret >> DFNODESHIFT;
    auto idx = ret & nodemask;
    return gdic_idx[node][idx];
  }
}

size_t typed_dfcolumn<string>::count() {
  size_t size = val.viewas_dvector<size_t>().size();
  if(contain_nulls) {
    size_t nullsize = nulls.template viewas_dvector<size_t>().size();
    return size - nullsize;
  } else return size;
}

// ----- datetime -----
datetime_t typed_dfcolumn<datetime>::first(bool ignore_nulls) {
  return typed_dfcolumn<datetime_t>::first(ignore_nulls);
}

datetime_t typed_dfcolumn<datetime>::last(bool ignore_nulls) {
  return typed_dfcolumn<datetime_t>::last(ignore_nulls);
}

datetime_t typed_dfcolumn<datetime>::max() {
  return typed_dfcolumn<datetime_t>::max();
}

datetime_t typed_dfcolumn<datetime>::min() {
  return typed_dfcolumn<datetime_t>::min();
}

}
