#include "dfcolumn.hpp"

#include <limits>
#include <climits>
#include "../text/datetime_to_words.hpp"
#include "dfcolumn_helper.hpp"

using namespace std;

namespace frovedis {

// ==================== size() ====================
template <class T>
size_t typed_dfcolumn<T>::size() {
  return val.template viewas_dvector<T>().size();
}

template size_t typed_dfcolumn<int>::size();
template size_t typed_dfcolumn<unsigned int>::size();
template size_t typed_dfcolumn<long>::size();
template size_t typed_dfcolumn<unsigned long>::size();
template size_t typed_dfcolumn<float>::size();
template size_t typed_dfcolumn<double>::size();
size_t typed_dfcolumn<dic_string>::size() {
  return val.viewas_dvector<size_t>().size();
}
size_t typed_dfcolumn<raw_string>::size() {
  auto sizes = this->sizes();
  size_t ret = 0;
  auto sizesp = sizes.data();
  auto sizes_size = sizes.size();
  for(size_t i = 0; i < sizes_size; i++) ret += sizesp[i];
  return ret;
}
size_t typed_dfcolumn<string>::size() {
  return val.viewas_dvector<size_t>().size();
}


// ==================== sizes() ====================
template <class T>
std::vector<size_t> typed_dfcolumn<T>::sizes() {
  return val.template viewas_dvector<T>().sizes();
}

template std::vector<size_t> typed_dfcolumn<int>::sizes();
template std::vector<size_t> typed_dfcolumn<unsigned int>::sizes();
template std::vector<size_t> typed_dfcolumn<long>::sizes();
template std::vector<size_t> typed_dfcolumn<unsigned long>::sizes();
template std::vector<size_t> typed_dfcolumn<float>::sizes();
template std::vector<size_t> typed_dfcolumn<double>::sizes();
std::vector<size_t> typed_dfcolumn<dic_string>::sizes() {
  return val.template viewas_dvector<size_t>().sizes();
}
std::vector<size_t> typed_dfcolumn<raw_string>::sizes() {
  return comp_words.map(+[](const compressed_words& cws)
                        {return cws.num_words();}).gather();
}
std::vector<size_t> typed_dfcolumn<string>::sizes() {
  return val.template viewas_dvector<size_t>().sizes();
}


// ==================== debug_print ====================
template <class T>
void typed_dfcolumn<T>::debug_print() {
  std::cout << "dtype: " << dtype() << std::endl;
  std::cout << "values: ";
  for(auto& i: val.gather()) {
    for(auto j: i) std::cout << j << " ";
    std::cout << ": ";
  }
  std::cout << std::endl;
  std::cout << "nulls: ";
  for(auto& i: nulls.gather()) {
    for(auto j: i) std::cout << j << " ";
    std::cout << ": ";
  }
  std::cout << std::endl;
  std::cout << "contain_nulls: " << contain_nulls << std::endl;
  if(spillable) {
    std::cout << "spill_initialized: " << spill_initialized << std::endl;
    std::cout << "already_spilled_to_disk: " << already_spilled_to_disk
              << std::endl;
    std::cout << "cleared: " << cleared << std::endl;
    std::cout << "spill_state: " << spill_state << std::endl;
    std::cout << "spill_size_cache: " << spill_size_cache << std::endl;
    if(spill_initialized) {
      std::cout << "spill_path: ";
      auto spill_paths = spill_path.gather();
      for(auto& p: spill_paths) std::cout << p << ", ";
      std::cout << std::endl;
    }
  }
}

template void typed_dfcolumn<int>::debug_print();
template void typed_dfcolumn<unsigned int>::debug_print();
template void typed_dfcolumn<long>::debug_print();
template void typed_dfcolumn<unsigned long>::debug_print();
template void typed_dfcolumn<float>::debug_print();
template void typed_dfcolumn<double>::debug_print();

// ----- dic_string -----
void typed_dfcolumn<dic_string>::debug_print() {
  std::cout << "dtype: " << dtype() << std::endl;
  std::cout << "dic: " << std::endl;
  if(dic) dic->print();
  std::cout << "val: " << std::endl;
  for(auto& i: val.gather()) {
    for(auto j: i) {
      std::cout << j << " ";
    }
    std::cout << ": ";
  }
  std::cout << std::endl;
  std::cout << "nulls: ";
  for(auto& i: nulls.gather()) {
    for(auto j: i) std::cout << j << " ";
    std::cout << ": ";
  }
  std::cout << std::endl;
  std::cout << "contain_nulls: " << contain_nulls << std::endl;
  if(spillable) {
    std::cout << "spill_initialized: " << spill_initialized << std::endl;
    std::cout << "already_spilled_to_disk: " << already_spilled_to_disk
              << std::endl;
    std::cout << "cleared: " << cleared << std::endl;
    std::cout << "spill_state: " << spill_state << std::endl;
    std::cout << "spill_size_cache: " << spill_size_cache << std::endl;
    if(spill_initialized) {
      std::cout << "spill_path: ";
      auto spill_paths = spill_path.gather();
      for(auto& p: spill_paths) std::cout << p << ", ";
      std::cout << std::endl;
    }
  }
}

// ----- raw_string -----
void typed_dfcolumn<raw_string>::debug_print() {
  std::cout << "dtype: " << dtype() << std::endl;
  for(auto& i: comp_words.gather()) {
      i.print();
  }
  std::cout << "nulls: ";
  for(auto& i: nulls.gather()) {
    for(auto j: i) std::cout << j << " ";
    std::cout << ": ";
  }
  std::cout << std::endl;
  std::cout << "contain_nulls: " << contain_nulls << std::endl;
  std::cout << "contain_nulls: " << contain_nulls << std::endl;
  if(spillable) {
    std::cout << "spill_initialized: " << spill_initialized << std::endl;
    std::cout << "already_spilled_to_disk: " << already_spilled_to_disk
              << std::endl;
    std::cout << "cleared: " << cleared << std::endl;
    std::cout << "spill_state: " << spill_state << std::endl;
    std::cout << "spill_size_cache: " << spill_size_cache << std::endl;
    if(spill_initialized) {
      std::cout << "spill_path: ";
      auto spill_paths = spill_path.gather();
      for(auto& p: spill_paths) std::cout << p << ", ";
      std::cout << std::endl;
    }
  }
}

// ----- string -----
void typed_dfcolumn<string>::debug_print() {
  std::cout << "dtype: " << dtype() << std::endl;
  size_t nodemask = (size_t(1) << DFNODESHIFT) - 1;
  std::cout << "dic: " << std::endl;
  for(auto& i: dic->viewas_node_local().gather()) {
    for(auto j: i) {
      cout << j.first << "|" 
           << (j.second >> DFNODESHIFT) << "-"
           << (j.second & nodemask) << " ";
    }
    std::cout << ": ";
  }
  cout << endl;
  std::cout << "dic_idx: " << std::endl;
  for(auto& i: dic_idx->gather()) {
    for(auto j: i) {
      cout << j << " ";
    }
    std::cout << ": ";
  }
  cout << endl;
  std::cout << "val: " << std::endl;
  for(auto& i: val.gather()) {
    for(auto j: i) {
      std::cout << (j >> DFNODESHIFT) << "-" << (j & nodemask) << " ";
    }
    std::cout << ": ";
  }
  std::cout << std::endl;
  std::cout << "nulls: ";
  for(auto& i: nulls.gather()) {
    for(auto j: i) std::cout << j << " ";
    std::cout << ": ";
  }
  std::cout << std::endl;
  std::cout << "contain_nulls: " << contain_nulls << std::endl;
  if(spillable) {
    std::cout << "spill_initialized: " << spill_initialized << std::endl;
    std::cout << "already_spilled_to_disk: " << already_spilled_to_disk
              << std::endl;
    std::cout << "cleared: " << cleared << std::endl;
    std::cout << "spill_state: " << spill_state << std::endl;
    std::cout << "spill_size_cache: " << spill_size_cache << std::endl;
    if(spill_initialized) {
      std::cout << "spill_path: ";
      auto spill_paths = spill_path.gather();
      for(auto& p: spill_paths) std::cout << p << ", ";
      std::cout << std::endl;
    }
  }
}

// ----- datetime -----
void typed_dfcolumn<datetime>::debug_print() {
  std::cout << "dtype: " << dtype() << std::endl;
  std::cout << "values: ";
  for(auto& i: val.gather()) {
    for(auto j: i) printf("%lx ", j);
    std::cout << ": ";
  }
  std::cout << std::endl;
  std::cout << "nulls: ";
  for(auto& i: nulls.gather()) {
    for(auto j: i) std::cout << j << " ";
    std::cout << ": ";
  }
  std::cout << std::endl;
  std::cout << "contain_nulls: " << contain_nulls << std::endl;
  if(spillable) {
    std::cout << "spill_initialized: " << spill_initialized << std::endl;
    std::cout << "already_spilled_to_disk: " << already_spilled_to_disk
              << std::endl;
    std::cout << "cleared: " << cleared << std::endl;
    std::cout << "spill_state: " << spill_state << std::endl;
    std::cout << "spill_size_cache: " << spill_size_cache << std::endl;
    if(spill_initialized) {
      std::cout << "spill_path: ";
      auto spill_paths = spill_path.gather();
      for(auto& p: spill_paths) std::cout << p << ", ";
      std::cout << std::endl;
    }
  }
}


// ==================== as_string ====================
template <class T>
std::vector<std::string> as_string_helper(std::vector<T>& val,
                                          const std::vector<size_t>& nulls) {
  // value of NULL position should always be max
  std::vector<std::string> ret(val.size());
  for(size_t i = 0; i < val.size(); i++) {
    if(val[i] ==  std::numeric_limits<T>::max()) ret[i] = "NULL";
    else ret[i] = boost::lexical_cast<std::string>(val[i]);
  }
  return ret;
}

template <class T>
dvector<std::string> typed_dfcolumn<T>::as_string() {
  return val.map(as_string_helper<T>, nulls).
    template moveto_dvector<std::string>();
}

template dvector<std::string> typed_dfcolumn<int>::as_string();
template dvector<std::string> typed_dfcolumn<unsigned int>::as_string();
template dvector<std::string> typed_dfcolumn<long>::as_string();
template dvector<std::string> typed_dfcolumn<unsigned long>::as_string();
template dvector<std::string> typed_dfcolumn<float>::as_string();
template dvector<std::string> typed_dfcolumn<double>::as_string();
dvector<std::string> typed_dfcolumn<string>::as_string() {
  return get_val().template moveto_dvector<std::string>();
}


// ==================== as_words ====================
template <class T>
words dfcolumn_as_words_helper(std::vector<T>& v,
                               const std::vector<size_t>& nulls,
                               size_t precision,
                               const std::string& nullstr) {
  auto nulls_size = nulls.size();
  auto nullsp = nulls.data();
  auto vp = v.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob  
  for(size_t i = 0; i < nulls_size; i++) {
    vp[nullsp[i]] = 0; // max is too long for creating words
  }
  auto ws = number_to_words<T>(v, precision);
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob  
  for(size_t i = 0; i < nulls_size; i++) {
    vp[nullsp[i]] = std::numeric_limits<T>::max();
  }
  auto nullstrvec = char_to_int(nullstr);
  auto nullstr_size = nullstr.size();
  auto chars_size = ws.chars.size();
  auto newchars_size = chars_size + nullstr_size;
  auto charsp = ws.chars.data();
  std::vector<int> newchars(newchars_size);
  auto newcharsp = newchars.data();
  for(size_t i = 0; i < chars_size; i++) newcharsp[i] = charsp[i];
  auto nullstrvecp = nullstrvec.data();
  for(size_t i = 0; i < nullstr_size; i++) {
    newcharsp[chars_size + i] = nullstrvecp[i];
  }
  ws.chars.swap(newchars);
  size_t nullstart = chars_size;
  size_t nulllens = nullstr_size;
  auto startsp = ws.starts.data();
  auto lensp = ws.lens.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob  
  for(size_t i = 0; i < nulls_size; i++) {
    startsp[nullsp[i]] = nullstart;
    lensp[nullsp[i]] = nulllens;
  }
  return ws;
}

template <class T>
node_local<words> typed_dfcolumn<T>::as_words(size_t precision,
                                              const std::string& datetime_fmt, // not used
                                              bool quote_escape, // not used
                                              const std::string& nullstr) {
  if(contain_nulls)
    return val.map(dfcolumn_as_words_helper<T>, nulls, broadcast(precision), 
                   broadcast(nullstr));
  else 
    return val.map(+[](const std::vector<T>& v, size_t precision) {
        return number_to_words<T>(v, precision);
      }, broadcast(precision));
}

template node_local<words> typed_dfcolumn<int>::as_words
(size_t precision,
 const std::string& datetime_fmt,
 bool quote_escape,
 const std::string& nullstr);
template node_local<words> typed_dfcolumn<unsigned int>::as_words
(size_t precision,
 const std::string& datetime_fmt,
 bool quote_escape,
 const std::string& nullstr);
template node_local<words> typed_dfcolumn<long>::as_words
(size_t precision,
 const std::string& datetime_fmt,
 bool quote_escape,
 const std::string& nullstr);
template node_local<words> typed_dfcolumn<unsigned long>::as_words
(size_t precision,
 const std::string& datetime_fmt,
 bool quote_escape,
 const std::string& nullstr);
template node_local<words> typed_dfcolumn<float>::as_words
(size_t precision,
 const std::string& datetime_fmt,
 bool quote_escape,
 const std::string& nullstr);
template node_local<words> typed_dfcolumn<double>::as_words
(size_t precision,
 const std::string& datetime_fmt,
 bool quote_escape,
 const std::string& nullstr);

void dfcolumn_replace_nullstr(words& ws,
                              const std::vector<size_t>& nulls,
                              const std::string& nullstr) {
  auto nulls_size = nulls.size();
  auto nullsp = nulls.data();
  auto nullstrvec = char_to_int(nullstr);
  auto nullstr_size = nullstr.size();
  auto chars_size = ws.chars.size();
  auto newchars_size = chars_size + nullstr_size;
  auto charsp = ws.chars.data();
  std::vector<int> newchars(newchars_size);
  auto newcharsp = newchars.data();
  for(size_t i = 0; i < chars_size; i++) newcharsp[i] = charsp[i];
  auto nullstrvecp = nullstrvec.data();
  for(size_t i = 0; i < nullstr_size; i++) {
    newcharsp[chars_size + i] = nullstrvecp[i];
  }
  ws.chars.swap(newchars);
  size_t nullstart = chars_size;
  size_t nulllens = nullstr_size;
  auto startsp = ws.starts.data();
  auto lensp = ws.lens.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob  
  for(size_t i = 0; i < nulls_size; i++) {
    startsp[nullsp[i]] = nullstart;
    lensp[nullsp[i]] = nulllens;
  }
}

words dic_string_as_words_helper(const dict& d, const std::vector<size_t>& val,
                                 const std::vector<size_t>& nulls) {
  auto nulls_size = nulls.size();
  if(nulls_size == 0) return d.index_to_words(val);
  else {
    auto val_size = val.size();
    std::vector<size_t> newval(val_size);
    auto newvalp = newval.data();
    auto valp = val.data();
    for(size_t i = 0; i < val_size; i++) newvalp[i] = valp[i];
    auto nullsp = nulls.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < nulls_size; i++) newvalp[nullsp[i]] = 0;
    if(d.num_words() == 0) { // does this happen?
      words w;
      w.chars = char_to_int("NULL");
      w.starts = {0};
      w.lens = {w.chars.size()};
      auto tmpd = make_dict(w);
      return tmpd.index_to_words(newval);
    } else {
      return d.index_to_words(newval);  
    }
  }
}

node_local<words>
typed_dfcolumn<dic_string>::as_words(size_t precision, // not used
                                     const std::string& datetime_fmt, // not used
                                     bool quote_escape,
                                     const std::string& nullstr) {
  /* broadcasting dic might be heavy, so if dic is too large,
     gather val and scatter words */
  // approximate one word size == 8
  size_t bcastsize = (dic->cwords.size() + dic->lens.size() +
                      dic->lens_num.size()) * 8 * get_nodesize();
  // gather (1 * 8, size_t = 8B) + scatter (4 * 8, since words is int)
  size_t gathersize = val.viewas_dvector<size_t>().size() * 5 * 8;
  node_local<words> nl_words;
  if(bcastsize < gathersize) {
    auto bdic = broadcast(*dic);
    nl_words = bdic.map(dic_string_as_words_helper, val, nulls);
  } else {
    size_t nodesize = get_nodesize();
    auto vec_val = val.gather();
    auto vec_nulls = nulls.gather();
    std::vector<words> vec_words(nodesize);
    for(size_t i = 0; i < nodesize; i++) {
      vec_words[i] = dic_string_as_words_helper(*dic, vec_val[i], vec_nulls[i]);
    }
    nl_words = make_node_local_scatter(vec_words);
  }
  if(contain_nulls)
    nl_words.mapv(dfcolumn_replace_nullstr, nulls, broadcast(nullstr));
  if(quote_escape) nl_words.mapv(quote_and_escape);
  return nl_words;
}

node_local<words>
typed_dfcolumn<raw_string>::as_words(size_t precision, // not used
                                     const std::string& datetime_fmt, // not used
                                     bool quote_escape,
                                     const std::string& nullstr) {
  auto nl_words = comp_words.map(+[](const compressed_words& cws)
                                 {return cws.decompress();});
  if(contain_nulls)
    nl_words.mapv(dfcolumn_replace_nullstr, nulls, broadcast(nullstr));
  if(quote_escape) nl_words.mapv(quote_and_escape);
  return nl_words;
}

words dfcolumn_string_as_words_helper(const std::vector<string>& str,
                                      const std::vector<size_t>& nulls,
                                      const std::string& nullstr) {
  auto ws = vector_string_to_words(str);
  auto nulls_size = nulls.size();
  auto nullsp = nulls.data();
  auto nullstrvec = char_to_int(nullstr);
  auto nullstr_size = nullstr.size();
  auto chars_size = ws.chars.size();
  auto newchars_size = chars_size + nullstr_size;
  auto charsp = ws.chars.data();
  std::vector<int> newchars(newchars_size);
  auto newcharsp = newchars.data();
  for(size_t i = 0; i < chars_size; i++) newcharsp[i] = charsp[i];
  auto nullstrvecp = nullstrvec.data();
  for(size_t i = 0; i < nullstr_size; i++) {
    newcharsp[chars_size + i] = nullstrvecp[i];
  }
  ws.chars.swap(newchars);
  size_t nullstart = chars_size;
  size_t nulllens = nullstr_size;
  auto startsp = ws.starts.data();
  auto lensp = ws.lens.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob  
  for(size_t i = 0; i < nulls_size; i++) {
    startsp[nullsp[i]] = nullstart;
    lensp[nullsp[i]] = nulllens;
  }
  return ws;
}

node_local<words>
typed_dfcolumn<string>::as_words(size_t precision, // not used
                                 const std::string& datetime_fmt, // not used
                                 bool quote_escape,
                                 const std::string& nullstr) {
  if(contain_nulls) {
    auto nl_words = get_val().map(dfcolumn_string_as_words_helper, nulls,
                                  broadcast(nullstr));
    if(quote_escape) nl_words.mapv(quote_and_escape);
    return nl_words;
  } else {
    auto nl_words = get_val().map(vector_string_to_words);
    if(quote_escape) nl_words.mapv(quote_and_escape);
    return nl_words;
  }
}

// ----- datetime -----
node_local<words>
typed_dfcolumn<datetime>::as_words(size_t precision, // not used
                                   const std::string& datetime_fmt,
                                   bool quote_escape,
                                   const std::string& nullstr) {
  auto nl_words = val.map(+[](const std::vector<datetime_t>& v,
                              const std::string& fmt) {
                            return datetime_to_words(v, fmt);
                          }, broadcast(datetime_fmt));
  if(contain_nulls)
    nl_words.mapv(dfcolumn_replace_nullstr, nulls, broadcast(nullstr));
  if(quote_escape) nl_words.mapv(quote_and_escape);
  return nl_words;
}


// ==================== extract ====================
template <class T>
std::vector<T> extract_helper(std::vector<T>& val,
                              std::vector<size_t>& idx,
                              std::vector<size_t>& nulls,
                              std::vector<size_t>& retnulls) {
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
  size_t nullssize = nulls.size();
  if(nullssize != 0) {
#ifdef DONOT_ALLOW_MAX_AS_VALUE
    // assume that val always contains max() iff NULL; this is much faster
    retnulls = vector_find_eq(ret, std::numeric_limits<T>::max());
#else
    std::vector<int> dummy(nullssize);
    auto nullhash = unique_hashtable<size_t, int>(nulls, dummy);
    auto isnull = nullhash.check_existence(idx);
    retnulls = vector_find_one(isnull);
#endif
  }
  return ret;
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::extract(node_local<std::vector<size_t>>& idx) {
  auto ret = std::make_shared<typed_dfcolumn<T>>();
  auto retnulls = make_node_local_allocate<std::vector<size_t>>();
  if(contain_nulls) {
    ret->val = val.map(extract_helper<T>, idx, nulls, retnulls);
    ret->nulls = std::move(retnulls);
    ret->contain_nulls_check();
  } else {
    ret->val = val.map(extract_helper2<T>, idx);
    ret->nulls = std::move(retnulls);
  }
  return ret;
}

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::extract(node_local<std::vector<size_t>>& idx);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::extract(node_local<std::vector<size_t>>& idx);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::extract(node_local<std::vector<size_t>>& idx);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::extract(node_local<std::vector<size_t>>& idx);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::extract(node_local<std::vector<size_t>>& idx);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::extract(node_local<std::vector<size_t>>& idx);

// ----- dic_string -----
std::shared_ptr<dfcolumn>
typed_dfcolumn<dic_string>::extract(node_local<std::vector<size_t>>& idx) {
  auto ret = std::make_shared<typed_dfcolumn<dic_string>>();
  auto retnulls = make_node_local_allocate<std::vector<size_t>>();
  if(contain_nulls) {
    ret->val = val.map(extract_helper<size_t>, idx, nulls, retnulls);
    ret->nulls = std::move(retnulls);
    ret->contain_nulls_check();
  } else {
    ret->val = val.map(extract_helper2<size_t>, idx);
    ret->nulls = std::move(retnulls);
  }
  ret->dic = dic;
  return ret;
}

// ----- raw_string -----
compressed_words
raw_string_extract_helper(const compressed_words& cws,
                          const std::vector<size_t>& idx,
                          const std::vector<size_t>& nulls,
                          std::vector<size_t>& retnulls) {
  compressed_words ret = cws.extract(idx);
  size_t nullssize = nulls.size();
  if(nullssize != 0) {
    std::vector<int> dummy(nullssize);
    auto nullhash = unique_hashtable<size_t, int>(nulls, dummy);
    auto isnull = nullhash.check_existence(idx);
    retnulls = vector_find_one(isnull);
  }
  return ret;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<raw_string>::extract(node_local<std::vector<size_t>>& idx) {
  auto ret = std::make_shared<typed_dfcolumn<raw_string>>();
  auto retnulls = make_node_local_allocate<std::vector<size_t>>();
  if(contain_nulls) {
    ret->comp_words = comp_words.map(raw_string_extract_helper, idx,
                                     nulls, retnulls);
    ret->nulls = std::move(retnulls);
    ret->contain_nulls_check();
  } else {
    ret->comp_words = comp_words.map(+[](const compressed_words& cws,
                                         const std::vector<size_t>& idx)
                                     {return cws.extract(idx);}, idx);
    ret->nulls = std::move(retnulls);
  }
  return ret;
}

// ----- string -----
std::shared_ptr<dfcolumn>
typed_dfcolumn<string>::extract(node_local<std::vector<size_t>>& idx) {
  auto ret = std::make_shared<typed_dfcolumn<std::string>>();
  auto retnulls = make_node_local_allocate<std::vector<size_t>>();
  if(contain_nulls) {
    ret->val = val.map(extract_helper<size_t>, idx, nulls, retnulls);
    ret->nulls = std::move(retnulls);
    ret->contain_nulls_check();
  } else {
    ret->val = val.map(extract_helper2<size_t>, idx);
    ret->nulls = std::move(retnulls);
  }
  ret->dic = dic;
  ret->dic_idx = dic_idx;
  return ret;
}

// ----- datetime -----
std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::extract(node_local<std::vector<size_t>>& idx) {
  auto ret = std::make_shared<typed_dfcolumn<datetime>>();
  auto retnulls = make_node_local_allocate<std::vector<size_t>>();
  if(contain_nulls) {
    ret->val = val.map(extract_helper<datetime_t>, idx, nulls, retnulls);
    ret->nulls = std::move(retnulls);
    ret->contain_nulls_check();
  } else {
    ret->val = val.map(extract_helper2<datetime_t>, idx);
    ret->nulls = std::move(retnulls);
  }
  return ret;
}

// ==================== global_extract ====================
std::vector<std::vector<size_t>>
global_extract_null_helper(std::vector<size_t>& nulls,
                           std::vector<std::vector<size_t>>& exchanged_idx);

std::vector<size_t>
global_extract_null_helper2(unique_hashtable<size_t, int>& hashtable,
                            std::vector<size_t>& global_idx,
                            int& null_exists);

unique_hashtable<size_t, int>
create_null_hash_from_partition(std::vector<std::vector<size_t>>& part_idx,
                                int& null_exists);

template <class T>
std::vector<std::vector<T>>
global_extract_helper(std::vector<T>& val,
                      std::vector<std::vector<size_t>>& exchanged_idx) {
  size_t size = exchanged_idx.size();
  std::vector<std::vector<T>> ret(size);
  T* valp = &val[0];
  for(size_t i = 0; i < size; i++) {
    size_t exchanged_size = exchanged_idx[i].size();
    ret[i].resize(exchanged_size);
    size_t* exchanged_idxp = &exchanged_idx[i][0];
    T* retp = &ret[i][0];
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t j = 0; j < exchanged_size; j++) {
      retp[j] = valp[exchanged_idxp[j]];
    }
  }
  return ret;
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::global_extract
(node_local<std::vector<size_t>>& global_idx,
 node_local<std::vector<size_t>>& to_store_idx,
 node_local<std::vector<std::vector<size_t>>>& exchanged_idx) {
  time_spent t(DEBUG);
  auto ret = std::make_shared<typed_dfcolumn<T>>();
  auto exdata = val.map(global_extract_helper<T>, exchanged_idx);
  t.show("global_exract_helper: ");
  auto exchanged_back = alltoall_exchange(exdata).map(flatten<T>);
  t.show("alltoall_exchange + flatten: ");
  ret->val = exchanged_back.map
    (+[](std::vector<T>& val, std::vector<size_t>& idx) {
      auto valp = val.data();
      auto idxp = idx.data();
      auto size = idx.size();
      std::vector<T> ret(size);
      auto retp = ret.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
      for(size_t i = 0 ; i < size; i++) {
        retp[i] = valp[idxp[i]];
      }
      return ret;
    }, to_store_idx);
  t.show("store: ");
  if(contain_nulls) {
#ifdef DONOT_ALLOW_MAX_AS_VALUE
    // assume that val always contains max() if NULL; this is much faster
    ret->nulls = ret->val.map(+[](std::vector<T>& val) {
        return vector_find_eq(val, std::numeric_limits<T>::max());
      });
    ret->contain_nulls_check();
#else
    auto exnulls = nulls.map(global_extract_null_helper, exchanged_idx);
    t.show("global_extract_null_helper: ");
    auto exchanged_back_nulls = alltoall_exchange(exnulls);
    t.show("alltoall_exchange (nulls): ");
    auto null_exists = make_node_local_allocate<int>();
    auto nullhashes = exchanged_back_nulls.map(create_null_hash_from_partition,
                                               null_exists);
    t.show("create_null_hash_from_partition: ");
    ret->nulls = nullhashes.map(global_extract_null_helper2, global_idx,
                                null_exists);
    t.show("global_extract_null_helper2: ");
#endif
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  }
  return ret;
}

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::global_extract
(node_local<std::vector<size_t>>& global_idx,
 node_local<std::vector<size_t>>& to_store_idx,
 node_local<std::vector<std::vector<size_t>>>& exchanged_idx);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::global_extract
(node_local<std::vector<size_t>>& global_idx,
 node_local<std::vector<size_t>>& to_store_idx,
 node_local<std::vector<std::vector<size_t>>>& exchanged_idx);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::global_extract
(node_local<std::vector<size_t>>& global_idx,
 node_local<std::vector<size_t>>& to_store_idx,
 node_local<std::vector<std::vector<size_t>>>& exchanged_idx);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::global_extract
(node_local<std::vector<size_t>>& global_idx,
 node_local<std::vector<size_t>>& to_store_idx,
 node_local<std::vector<std::vector<size_t>>>& exchanged_idx);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::global_extract
(node_local<std::vector<size_t>>& global_idx,
 node_local<std::vector<size_t>>& to_store_idx,
 node_local<std::vector<std::vector<size_t>>>& exchanged_idx);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::global_extract
(node_local<std::vector<size_t>>& global_idx,
 node_local<std::vector<size_t>>& to_store_idx,
 node_local<std::vector<std::vector<size_t>>>& exchanged_idx);

// ----- dic_string -----
std::shared_ptr<dfcolumn>
typed_dfcolumn<dic_string>::global_extract
(node_local<std::vector<size_t>>& global_idx,
 node_local<std::vector<size_t>>& to_store_idx,
 node_local<std::vector<std::vector<size_t>>>& exchanged_idx) {
  auto ret = std::make_shared<typed_dfcolumn<dic_string>>();
  auto exdata = val.map(global_extract_helper<size_t>, exchanged_idx);
  auto exchanged_back = alltoall_exchange(exdata).map(flatten<size_t>);
  ret->val = exchanged_back.map
    (+[](std::vector<size_t>& val, std::vector<size_t>& idx) {
      auto valp = val.data();
      auto idxp = idx.data();
      auto size = idx.size();
      std::vector<size_t> ret(size);
      auto retp = ret.data();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
      for(size_t i = 0 ; i < size; i++) {
        retp[i] = valp[idxp[i]];
      }
      return ret;
    }, to_store_idx);
  if(contain_nulls) {
    /*
    auto exnulls = nulls.map(global_extract_null_helper, exchanged_idx);
    auto exchanged_back_nulls = alltoall_exchange(exnulls);
    auto null_exists = make_node_local_allocate<int>();
    auto nullhashes = exchanged_back_nulls.map(create_null_hash_from_partition,
                                               null_exists);
    ret->nulls = nullhashes.map(global_extract_null_helper2, global_idx,
                                null_exists);
    */
    // assume that val always contains max() iff NULL; this is much faster
    // val does not become max() in the case of dic_string
    ret->nulls = ret->val.map(+[](std::vector<size_t>& val) {
        return vector_find_eq(val, std::numeric_limits<size_t>::max());
      });
    ret->contain_nulls_check();
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();    
  }
  ret->dic = dic;
  return ret;
}

// ----- raw_string -----
std::vector<compressed_words>
raw_string_global_extract_helper(const compressed_words& cws,
                                 const std::vector<std::vector<size_t>>&
                                 exchanged_idx) {
  size_t size = exchanged_idx.size();
  std::vector<compressed_words> ret(size);
  for(size_t i = 0; i < size; i++) {
    ret[i] = cws.extract(exchanged_idx[i]);
  }
  return ret;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<raw_string>::global_extract
(node_local<std::vector<size_t>>& global_idx,
 node_local<std::vector<size_t>>& to_store_idx,
 node_local<std::vector<std::vector<size_t>>>& exchanged_idx) {
  auto ret = std::make_shared<typed_dfcolumn<raw_string>>();
  auto exdata = comp_words.map(raw_string_global_extract_helper, exchanged_idx);
  auto exchanged_back =
    alltoall_exchange(exdata).map(merge_multi_compressed_words);
  ret->comp_words =
    exchanged_back.map(+[](const compressed_words& cw,
                           const std::vector<size_t>& idx)
                       {return cw.extract(idx);}, to_store_idx);
  if(contain_nulls) {
    auto exnulls = nulls.map(global_extract_null_helper, exchanged_idx);
    auto exchanged_back_nulls = alltoall_exchange(exnulls);
    auto null_exists = make_node_local_allocate<int>();
    auto nullhashes = exchanged_back_nulls.map(create_null_hash_from_partition,
                                               null_exists);
    ret->nulls = nullhashes.map(global_extract_null_helper2, global_idx,
                                null_exists);
    ret->contain_nulls_check();
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();    
  }
  return ret;
}

// ----- string -----
std::shared_ptr<dfcolumn>
typed_dfcolumn<string>::global_extract
(node_local<std::vector<size_t>>& global_idx,
 node_local<std::vector<size_t>>& to_store_idx,
 node_local<std::vector<std::vector<size_t>>>& exchanged_idx) {
  auto ret = std::make_shared<typed_dfcolumn<std::string>>();
  auto exdata = val.map(global_extract_helper<size_t>, exchanged_idx);
  auto exchanged_back = alltoall_exchange(exdata).map(flatten<size_t>);
  ret->val = exchanged_back.map
    (+[](std::vector<size_t>& val, std::vector<size_t>& idx) {
      auto valp = val.data();
      auto idxp = idx.data();
      auto size = idx.size();
      std::vector<size_t> ret(size);
      auto retp = ret.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
      for(size_t i = 0 ; i < size; i++) {
        retp[i] = valp[idxp[i]];
      }
      return ret;
    }, to_store_idx);
  if(contain_nulls) {
    /*
    auto exnulls = nulls.map(global_extract_null_helper, exchanged_idx);
    auto exchanged_back_nulls = alltoall_exchange(exnulls);
    auto null_exists = make_node_local_allocate<int>();
    auto nullhashes = exchanged_back_nulls.map(create_null_hash_from_partition,
                                               null_exists);
    ret->nulls = nullhashes.map(global_extract_null_helper2, global_idx,
                                null_exists);
    */
    // assume that val always contains max() iff NULL; this is much faster
    // val does not become max() in the case of string
    ret->nulls = ret->val.map(+[](std::vector<size_t>& val) {
        return vector_find_eq(val, std::numeric_limits<size_t>::max());
      });
    ret->contain_nulls_check();
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();    
  }
  ret->dic = dic;
  ret->dic_idx = dic_idx;
  return ret;
}

// ----- datetime -----
std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::global_extract
(node_local<std::vector<size_t>>& global_idx,
 node_local<std::vector<size_t>>& to_store_idx,
 node_local<std::vector<std::vector<size_t>>>& exchanged_idx) {
  time_spent t(DEBUG);
  auto ret = std::make_shared<typed_dfcolumn<datetime>>();
  auto exdata = val.map(global_extract_helper<datetime_t>, exchanged_idx);
  t.show("global_exract_helper: ");
  auto exchanged_back = alltoall_exchange(exdata).map(flatten<datetime_t>);
  t.show("alltoall_exchange + flatten: ");
  ret->val = exchanged_back.map
    (+[](std::vector<datetime_t>& val, std::vector<size_t>& idx) {
      auto valp = val.data();
      auto idxp = idx.data();
      auto size = idx.size();
      std::vector<datetime_t> ret(size);
      auto retp = ret.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
      for(size_t i = 0 ; i < size; i++) {
        retp[i] = valp[idxp[i]];
      }
      return ret;
    }, to_store_idx);
  t.show("store: ");
  if(contain_nulls) {
    /*
      auto exnulls = nulls.map(global_extract_null_helper, exchanged_idx);
      t.show("global_extract_null_helper: ");
      auto exchanged_back_nulls = alltoall_exchange(exnulls);
      t.show("alltoall_exchange (nulls): ");
      auto null_exists = make_node_local_allocate<int>();
      auto nullhashes = exchanged_back_nulls.map(create_null_hash_from_partition,
      null_exists);
      t.show("create_null_hash_from_partition: ");
      ret->nulls = nullhashes.map(global_extract_null_helper2, global_idx,
      null_exists);
      t.show("global_extract_null_helper2: ");
    */
    // assume that val always contains max() iff NULL; this is much faster
    // datetime_t does not become max()
    ret->nulls = ret->val.map(+[](std::vector<datetime_t>& val) {
        return vector_find_eq(val, std::numeric_limits<datetime_t>::max());
      });
    ret->contain_nulls_check();
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();
  }
  return ret;
}

// ==================== get_local_index ====================
template <class T>
node_local<std::vector<size_t>>
typed_dfcolumn<T>::get_local_index() {
  return val.map(get_local_index_helper<T>);
}

template node_local<std::vector<size_t>>
typed_dfcolumn<int>::get_local_index();
template node_local<std::vector<size_t>>
typed_dfcolumn<unsigned int>::get_local_index();
template node_local<std::vector<size_t>>
typed_dfcolumn<long>::get_local_index();
template node_local<std::vector<size_t>>
typed_dfcolumn<unsigned long>::get_local_index();
template node_local<std::vector<size_t>>
typed_dfcolumn<float>::get_local_index();
template node_local<std::vector<size_t>>
typed_dfcolumn<double>::get_local_index();

// ----- dic_string -----
node_local<std::vector<size_t>> typed_dfcolumn<dic_string>::get_local_index() {
  return val.map(get_local_index_helper<size_t>);
}

// ----- raw_string -----
node_local<std::vector<size_t>> typed_dfcolumn<raw_string>::get_local_index() {
  return comp_words.map(+[](const compressed_words& cws) {
      auto num_words = cws.num_words();
      std::vector<size_t> r(num_words);
      auto rp = r.data();
      for(size_t i = 0; i < num_words; i++) rp[i] = i;
      return r;
    });
}

// ----- string -----
node_local<std::vector<size_t>> typed_dfcolumn<string>::get_local_index() {
  return val.map(get_local_index_helper<size_t>);
}

// ==================== append_nulls ====================
template <class T>
void append_nulls_helper(std::vector<T>& val, std::vector<size_t>& to_append,
                         std::vector<size_t>& nulls) {
  size_t val_size = val.size();
  size_t to_append_size = to_append.size();
  std::vector<T> newval(val_size+to_append_size);
  auto valp = val.data();
  auto newvalp = newval.data();
  for(size_t i = 0; i < val_size; i++) {
    newvalp[i] = valp[i];
  }
  for(size_t i = 0; i < to_append_size; i++) {
    newvalp[val_size + i] = std::numeric_limits<T>::max();
  }
  val.swap(newval);
  auto nulls_size = nulls.size();
  auto new_nulls_size = nulls_size + to_append_size;
  std::vector<size_t> new_nulls(new_nulls_size);
  auto new_nullsp = new_nulls.data();
  auto nullsp = nulls.data();
  for(size_t i = 0; i < nulls_size; i++) new_nullsp[i] = nullsp[i];
  for(size_t i = 0; i < to_append_size; i++) {
    new_nullsp[nulls_size + i] = val_size + i;
  }
  nulls.swap(new_nulls);
}

template <class T>
void
typed_dfcolumn<T>::append_nulls(node_local<std::vector<size_t>>& to_append) {
  val.mapv(append_nulls_helper<T>, to_append, nulls);
  contain_nulls = true;
}

template void typed_dfcolumn<int>::append_nulls
(node_local<std::vector<size_t>>& to_append);
template void typed_dfcolumn<unsigned int>::append_nulls
(node_local<std::vector<size_t>>& to_append);
template void typed_dfcolumn<long>::append_nulls
(node_local<std::vector<size_t>>& to_append);
template void typed_dfcolumn<unsigned long>::append_nulls
(node_local<std::vector<size_t>>& to_append);
template void typed_dfcolumn<float>::append_nulls
(node_local<std::vector<size_t>>& to_append);
template void typed_dfcolumn<double>::append_nulls
(node_local<std::vector<size_t>>& to_append);

// ----- dic_string -----
void typed_dfcolumn<dic_string>::append_nulls
(node_local<std::vector<size_t>>& to_append) {
  val.mapv(append_nulls_helper<size_t>, to_append, nulls);
  contain_nulls = true;
}

// ----- raw_string -----
// use "NULL" as string for NULL, which will not be used directly anyway
void raw_string_append_nulls_helper(compressed_words& cw,
                                    std::vector<size_t>& to_append,
                                    std::vector<size_t>& nulls) {
  auto num_words = cw.num_words();
  size_t to_append_size = to_append.size();
  words null_words;
  string nullstr = "NULL";
  auto nullstr_size = nullstr.size();
  null_words.chars = char_to_int(nullstr);
  null_words.starts.resize(to_append_size);
  null_words.lens.resize(to_append_size);
  auto startsp = null_words.starts.data();
  auto lensp = null_words.lens.data();
  for(size_t i = 0; i < to_append_size; i++) {
    startsp[i] = 0;
    lensp[i] = nullstr_size;
  }
  auto comp_null_words = make_compressed_words(null_words);
  auto newcw = merge_compressed_words(cw, comp_null_words);
  cw.cwords.swap(newcw.cwords);
  cw.lens.swap(newcw.lens);
  cw.lens_num.swap(newcw.lens_num);
  cw.order.swap(newcw.order);
  auto nulls_size = nulls.size();
  auto new_nulls_size = nulls_size + to_append_size;
  std::vector<size_t> new_nulls(new_nulls_size);
  auto new_nullsp = new_nulls.data();
  auto nullsp = nulls.data();
  for(size_t i = 0; i < nulls_size; i++) new_nullsp[i] = nullsp[i];
  for(size_t i = 0; i < to_append_size; i++) {
    new_nullsp[nulls_size + i] = num_words + i;
  }
  nulls.swap(new_nulls);
}

void typed_dfcolumn<raw_string>::append_nulls
(node_local<std::vector<size_t>>& to_append) {
  comp_words.mapv(raw_string_append_nulls_helper, to_append, nulls);
  contain_nulls = true;
}

// ----- string -----
void typed_dfcolumn<string>::append_nulls
(node_local<std::vector<size_t>>& to_append) {
  val.mapv(append_nulls_helper<size_t>, to_append, nulls);
  contain_nulls = true;
}


// ==================== as_dvector_float/double ====================
template <class T>
dvector<float> typed_dfcolumn<T>::as_dvector_float() {
  auto dv = as_dvector<T>();
  return val.map(do_static_cast<T,float>, nulls)
            .template moveto_dvector<float>();
}

template <class T>
dvector<double> typed_dfcolumn<T>::as_dvector_double() {
  auto dv = as_dvector<T>();
  return val.map(do_static_cast<T,double>, nulls)
            .template moveto_dvector<double>();
}


template dvector<float> typed_dfcolumn<int>::as_dvector_float();
template dvector<float> typed_dfcolumn<unsigned int>::as_dvector_float();
template dvector<float> typed_dfcolumn<long>::as_dvector_float();
template dvector<float> typed_dfcolumn<unsigned long>::as_dvector_float();
template dvector<float> typed_dfcolumn<float>::as_dvector_float();
template dvector<float> typed_dfcolumn<double>::as_dvector_float();

template dvector<double> typed_dfcolumn<int>::as_dvector_double();
template dvector<double> typed_dfcolumn<unsigned int>::as_dvector_double();
template dvector<double> typed_dfcolumn<long>::as_dvector_double();
template dvector<double> typed_dfcolumn<unsigned long>::as_dvector_double();
template dvector<double> typed_dfcolumn<float>::as_dvector_double();
template dvector<double> typed_dfcolumn<double>::as_dvector_double();


// ==================== union_columns ====================
template <class T>
void union_columns_helper(std::vector<T>& newval,
                          std::vector<size_t>& newnulls,
                          std::vector<std::vector<T>*>& val_colsp,
                          std::vector<std::vector<size_t>*>& nulls_colsp) {
  auto cols_size = val_colsp.size();
  std::vector<size_t> val_sizes(cols_size);
  auto val_colspp = val_colsp.data();
  auto val_sizesp = val_sizes.data();
  for(size_t i = 0; i < cols_size; i++) {
    val_sizesp[i] = val_colspp[i]->size();
  }
  size_t total_val_size = 0;
  for(size_t i = 0; i < cols_size; i++) {
    total_val_size += val_sizesp[i];
  }
  newval.resize(total_val_size);
  auto crnt_newvalp = newval.data();
  for(size_t i = 0; i < cols_size; i++) {
    auto val_size = val_sizesp[i];
    auto valp = val_colspp[i]->data();
    for(size_t j = 0; j < val_size; j++) {
      crnt_newvalp[j] = valp[j];
    }
    crnt_newvalp += val_size;
  }
  std::vector<size_t> nulls_sizes(cols_size);
  auto nulls_colspp = nulls_colsp.data();
  auto nulls_sizesp = nulls_sizes.data();
  for(size_t i = 0; i < cols_size; i++) {
    nulls_sizesp[i] = nulls_colspp[i]->size();
  }
  size_t total_nulls_size = 0;
  for(size_t i = 0; i < cols_size; i++) {
    total_nulls_size += nulls_sizesp[i];
  }
  newnulls.resize(total_nulls_size);
  auto crnt_newnullsp = newnulls.data();
  auto crnt_shift = 0;
  for(size_t i = 0; i < cols_size; i++) {
    auto nulls_size = nulls_sizesp[i];
    auto nullsp = nulls_colspp[i]->data();
    for(size_t j = 0; j < nulls_size; j++) {
      crnt_newnullsp[j] = nullsp[j] + crnt_shift;
    }
    crnt_shift += val_sizesp[i];
    crnt_newnullsp += nulls_size;
  }
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::union_columns
(const std::vector<std::shared_ptr<dfcolumn>>& cols) {
  auto cols_size = cols.size();
  if(cols_size == 0) {
    return std::make_shared<typed_dfcolumn<T>>(val, nulls);
  }
  std::vector<std::shared_ptr<typed_dfcolumn<T>>> rights(cols_size);
  for(size_t i = 0; i < cols_size; i++) {
    rights[i] = std::dynamic_pointer_cast<typed_dfcolumn<T>>(cols[i]);
    if(!static_cast<bool>(rights[i]))
      throw std::runtime_error("union_columns: different type");
  }
  auto val_colsp = make_node_local_allocate<std::vector<std::vector<T>*>>();
  auto nulls_colsp =
    make_node_local_allocate<std::vector<std::vector<size_t>*>>();
  val.mapv(+[](std::vector<T>& val, std::vector<std::vector<T>*>& val_colsp)
           {val_colsp.push_back(&val);}, val_colsp);
  nulls.mapv(+[](std::vector<size_t>& nulls,
                 std::vector<std::vector<size_t>*>& nulls_colsp)
             {nulls_colsp.push_back(&nulls);}, nulls_colsp);
  for(size_t i = 0; i < cols_size; i++) {
    rights[i]->val.mapv(+[](std::vector<T>& val,
                            std::vector<std::vector<T>*>& val_colsp)
                        {val_colsp.push_back(&val);}, val_colsp);
    rights[i]->nulls.mapv(+[](std::vector<size_t>& nulls,
                              std::vector<std::vector<size_t>*>& nulls_colsp)
                          {nulls_colsp.push_back(&nulls);}, nulls_colsp);
  }
  auto newval = make_node_local_allocate<std::vector<T>>();
  auto newnulls = make_node_local_allocate<std::vector<size_t>>();
  newval.mapv(union_columns_helper<T>, newnulls, val_colsp, nulls_colsp);
  return std::make_shared<typed_dfcolumn<T>>(std::move(newval),
                                             std::move(newnulls));
}

// ----- dic_string -----
dict union_columns_dic_string_create_dic(std::vector<dict*>& dic_colsp) {
  auto dic_colsp_size = dic_colsp.size();
  if(dic_colsp_size == 0) return dict();
  else if(dic_colsp_size == 1) return *dic_colsp[0];
  else if(dic_colsp_size == 2) {
    return merge_dict(*dic_colsp[0], *dic_colsp[1]);
  } else {
    auto left_size = ceil_div(dic_colsp_size, size_t(2));
    auto right_size = dic_colsp_size - left_size;
    vector<dict*> left(left_size);
    vector<dict*> right(right_size);
    for(size_t i = 0; i < left_size; i++) {
      left[i] = dic_colsp[i];
    }
    for(size_t i = 0; i < right_size; i++) {
      right[i] = dic_colsp[left_size + i];
    }
    auto left_merged = union_columns_dic_string_create_dic(left);
    auto right_merged = union_columns_dic_string_create_dic(right);
    return merge_dict(left_merged, right_merged);
  }
}

// same as string, but copied to avoid confusion
void union_columns_dic_string_resize_newval
(std::vector<size_t>& newval, std::vector<std::vector<size_t>*>& val_colsp) {
  auto cols_size = val_colsp.size();
  std::vector<size_t> val_sizes(cols_size);
  auto val_colspp = val_colsp.data();
  auto val_sizesp = val_sizes.data();
  for(size_t i = 0; i < cols_size; i++) {
    val_sizesp[i] = val_colspp[i]->size();
  }
  size_t total_val_size = 0;
  for(size_t i = 0; i < cols_size; i++) {
    total_val_size += val_sizesp[i];
  }
  newval.resize(total_val_size);
}

// in dfcolumn_filter_eq
std::vector<size_t>
dic_string_equal_prepare_helper(const std::vector<size_t>& rval,
                                const std::vector<size_t>& trans);

node_local<vector<size_t>>
union_columns_dic_string_prepare(dict& newdic,
                                 dict& dic,
                                 node_local<std::vector<size_t>>& val) {
  compressed_words to_lookup;
  auto num_words = dic.num_words();
  to_lookup.cwords.swap(dic.cwords);
  to_lookup.lens.swap(dic.lens);
  to_lookup.lens_num.swap(dic.lens_num);
  to_lookup.order.resize(num_words);
  auto orderp = to_lookup.order.data();
  for(size_t i = 0; i < num_words; i++) orderp[i] = i;
  auto trans_table = broadcast(newdic.lookup(to_lookup));
  to_lookup.cwords.swap(dic.cwords);
  to_lookup.lens.swap(dic.lens);
  to_lookup.lens_num.swap(dic.lens_num);
  return val.map(dic_string_equal_prepare_helper, trans_table);
}

// same as string, but copied to avoid confusion
void union_columns_dic_string_update_nulls
(std::vector<size_t>& newnulls,
 std::vector<std::vector<size_t>*>& val_colsp,
 std::vector<std::vector<size_t>*>& nulls_colsp) {

  auto cols_size = val_colsp.size();
  std::vector<size_t> val_sizes(cols_size);
  auto val_colspp = val_colsp.data();
  auto val_sizesp = val_sizes.data();
  for(size_t i = 0; i < cols_size; i++) {
    val_sizesp[i] = val_colspp[i]->size();
  }
  std::vector<size_t> nulls_sizes(cols_size);
  auto nulls_colspp = nulls_colsp.data();
  auto nulls_sizesp = nulls_sizes.data();
  for(size_t i = 0; i < cols_size; i++) {
    nulls_sizesp[i] = nulls_colspp[i]->size();
  }
  size_t total_nulls_size = 0;
  for(size_t i = 0; i < cols_size; i++) {
    total_nulls_size += nulls_sizesp[i];
  }
  newnulls.resize(total_nulls_size);
  auto crnt_newnullsp = newnulls.data();
  auto crnt_shift = 0;
  for(size_t i = 0; i < cols_size; i++) {
    auto nulls_size = nulls_sizesp[i];
    auto nullsp = nulls_colspp[i]->data();
    for(size_t j = 0; j < nulls_size; j++) {
      crnt_newnullsp[j] = nullsp[j] + crnt_shift;
    }
    crnt_shift += val_sizesp[i];
    crnt_newnullsp += nulls_size;
  }
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<dic_string>::union_columns
(const std::vector<std::shared_ptr<dfcolumn>>& cols) {
  auto cols_size = cols.size();
  if(cols_size == 0) {
    return std::make_shared<typed_dfcolumn<dic_string>>(dic, val, nulls);
  }
  std::vector<std::shared_ptr<typed_dfcolumn<dic_string>>> rights(cols_size);
  for(size_t i = 0; i < cols_size; i++) {
    rights[i] = std::dynamic_pointer_cast<typed_dfcolumn<dic_string>>(cols[i]);
    if(!static_cast<bool>(rights[i]))
      throw std::runtime_error("union_columns: different type");
  }
  auto val_colsp =
    make_node_local_allocate<std::vector<std::vector<size_t>*>>();
  auto nulls_colsp =
    make_node_local_allocate<std::vector<std::vector<size_t>*>>();
  std::vector<dict*> dic_colsp;
  val.mapv(+[](std::vector<size_t>& val,
               std::vector<std::vector<size_t>*>& val_colsp)
           {val_colsp.push_back(&val);}, val_colsp);
  nulls.mapv(+[](std::vector<size_t>& nulls,
                 std::vector<std::vector<size_t>*>& nulls_colsp)
             {nulls_colsp.push_back(&nulls);}, nulls_colsp);
  dic_colsp.push_back(&(*dic));
  for(size_t i = 0; i < cols_size; i++) {
    rights[i]->val.mapv(+[](std::vector<size_t>& val,
                            std::vector<std::vector<size_t>*>& val_colsp)
                        {val_colsp.push_back(&val);}, val_colsp);
    rights[i]->nulls.mapv(+[](std::vector<size_t>& nulls,
                              std::vector<std::vector<size_t>*>& nulls_colsp)
                          {nulls_colsp.push_back(&nulls);}, nulls_colsp);
    dic_colsp.push_back(&(*(rights[i]->dic)));
  }
  auto newval = make_node_local_allocate<std::vector<size_t>>();
  auto newnulls = make_node_local_allocate<std::vector<size_t>>();
  auto newdic =
    std::make_shared<dict>(union_columns_dic_string_create_dic(dic_colsp));
              
  newval.mapv(union_columns_dic_string_resize_newval, val_colsp);
  auto crnt_pos = broadcast(size_t(0));
  {
    auto crnt_newval = union_columns_dic_string_prepare(*newdic, *dic, val);
    newval.mapv
      (+[](std::vector<size_t>& newval, std::vector<size_t>& val, size_t& pos) {
        auto crnt_newvalp = newval.data() + pos;
        auto valp = val.data();
        auto val_size = val.size();
        for(size_t i = 0; i < val_size; i++) {crnt_newvalp[i] = valp[i];}
        pos += val_size;
      }, crnt_newval, crnt_pos);
  }
  for(size_t i = 0; i < cols_size; i++) {
    auto crnt_newval = union_columns_dic_string_prepare
      (*newdic, *(rights[i]->dic), rights[i]->val);
    newval.mapv
      (+[](std::vector<size_t>& newval, std::vector<size_t>& val, size_t& pos) {
        auto crnt_newvalp = newval.data() + pos;
        auto valp = val.data();
        auto val_size = val.size();
        for(size_t i = 0; i < val_size; i++) {crnt_newvalp[i] = valp[i];}
        pos += val_size;
      }, crnt_newval, crnt_pos);
  }
  newnulls.mapv(union_columns_dic_string_update_nulls, val_colsp, nulls_colsp);
  return std::make_shared<typed_dfcolumn<dic_string>>
    (std::move(newdic), std::move(newval), std::move(newnulls));
}

// ----- raw_string -----
compressed_words
merge_multi_compressed_words_ptr(vector<compressed_words*>& vcws) {
  auto vcws_size = vcws.size();
  if(vcws_size == 0) {
    return compressed_words();
  } else if(vcws_size == 1) {
    return *vcws[0];
  } else if(vcws_size == 2) {
    auto r = merge_compressed_words(*vcws[0], *vcws[1]);
    return r;
  } else {
    auto left_size = ceil_div(vcws_size, size_t(2));
    auto right_size = vcws_size - left_size;
    vector<compressed_words*> left(left_size);
    vector<compressed_words*> right(right_size);
    for(size_t i = 0; i < left_size; i++) {
      left[i] = vcws[i];
    }
    for(size_t i = 0; i < right_size; i++) {
      right[i] = vcws[left_size + i];
    }
    auto left_merged = merge_multi_compressed_words_ptr(left);
    auto right_merged = merge_multi_compressed_words_ptr(right);
    auto r = merge_compressed_words(left_merged, right_merged);
    return r;
  }
}

void union_columns_raw_string_helper
(compressed_words& newcomp_words,
 std::vector<size_t>& newnulls,
 std::vector<compressed_words*>& comp_words_colsp,
 std::vector<std::vector<size_t>*>& nulls_colsp) {

  newcomp_words = merge_multi_compressed_words_ptr(comp_words_colsp);
  auto cols_size = comp_words_colsp.size();
  std::vector<size_t> val_sizes(cols_size);
  auto comp_words_colspp = comp_words_colsp.data();
  auto val_sizesp = val_sizes.data();
  for(size_t i = 0; i < cols_size; i++) {
    val_sizesp[i] = comp_words_colspp[i]->num_words();
  }
  std::vector<size_t> nulls_sizes(cols_size);
  auto nulls_colspp = nulls_colsp.data();
  auto nulls_sizesp = nulls_sizes.data();
  for(size_t i = 0; i < cols_size; i++) {
    nulls_sizesp[i] = nulls_colspp[i]->size();
  }
  size_t total_nulls_size = 0;
  for(size_t i = 0; i < cols_size; i++) {
    total_nulls_size += nulls_sizesp[i];
  }
  newnulls.resize(total_nulls_size);
  auto crnt_newnullsp = newnulls.data();
  auto crnt_shift = 0;
  for(size_t i = 0; i < cols_size; i++) {
    auto nulls_size = nulls_sizesp[i];
    auto nullsp = nulls_colspp[i]->data();
    for(size_t j = 0; j < nulls_size; j++) {
      crnt_newnullsp[j] = nullsp[j] + crnt_shift;
    }
    crnt_shift += val_sizesp[i];
    crnt_newnullsp += nulls_size;
  }
}


std::shared_ptr<dfcolumn>
typed_dfcolumn<raw_string>::union_columns
(const std::vector<std::shared_ptr<dfcolumn>>& cols) {
  auto cols_size = cols.size();
  if(cols_size == 0) {
    return std::make_shared<typed_dfcolumn<raw_string>>(comp_words, nulls);
  }
  std::vector<std::shared_ptr<typed_dfcolumn<raw_string>>> rights(cols_size);
  for(size_t i = 0; i < cols_size; i++) {
    rights[i] = std::dynamic_pointer_cast<typed_dfcolumn<raw_string>>(cols[i]);
    if(!static_cast<bool>(rights[i]))
      throw std::runtime_error("union_columns: different type");
  }
  auto comp_words_colsp =
    make_node_local_allocate<std::vector<compressed_words*>>();
  auto nulls_colsp =
    make_node_local_allocate<std::vector<std::vector<size_t>*>>();
  comp_words.mapv(+[](compressed_words& comp_words,
                      std::vector<compressed_words*>& comp_words_colsp)
                  {comp_words_colsp.push_back(&comp_words);}, comp_words_colsp);
  nulls.mapv(+[](std::vector<size_t>& nulls,
                 std::vector<std::vector<size_t>*>& nulls_colsp)
             {nulls_colsp.push_back(&nulls);}, nulls_colsp);
  for(size_t i = 0; i < cols_size; i++) {
    rights[i]->comp_words.
      mapv(+[](compressed_words& comp_words,
               std::vector<compressed_words*>& comp_words_colsp)
           {comp_words_colsp.push_back(&comp_words);}, comp_words_colsp);
    rights[i]->nulls.mapv(+[](std::vector<size_t>& nulls,
                              std::vector<std::vector<size_t>*>& nulls_colsp)
                          {nulls_colsp.push_back(&nulls);}, nulls_colsp);
  }
  auto newcomp_words = make_node_local_allocate<compressed_words>();
  auto newnulls = make_node_local_allocate<std::vector<size_t>>();
  newcomp_words.mapv(union_columns_raw_string_helper, newnulls,
                     comp_words_colsp, nulls_colsp);
  return std::make_shared<typed_dfcolumn<raw_string>>
    (std::move(newcomp_words), std::move(newnulls));
}

bool typed_dfcolumn<raw_string>::is_all_null() {
  return comp_words.
    map(+[](compressed_words& comp_words, std::vector<size_t>& nulls)
        {return comp_words.num_words() == nulls.size();}, nulls).
    reduce(+[](bool left, bool right){return left && right;});
}

// ----- string -----
// in dfcolumn_filter_eq.cc
void create_string_trans_table(my_map<string,size_t>& leftdic,
                               my_map<string,size_t>& rightdic,
                               vector<size_t>& from,
                               vector<size_t>& to);

vector<size_t> equal_prepare_helper(vector<size_t>& val,
                                    vector<size_t>& from,
                                    vector<size_t>& to);

void union_columns_string_create_dic
(my_map<std::string,size_t>& newdic,
 std::vector<std::string>& newdic_idx,
 std::vector<my_map<std::string,size_t>*>& dic_colsp,
 std::vector<std::vector<std::string>*>& dic_idx_colsp) {
  auto cols_size = dic_colsp.size();
  size_t selfid = static_cast<size_t>(get_selfid());
  size_t nodeinfo = selfid << DFNODESHIFT;
  for(size_t i = 0; i < cols_size; i++) {
    auto& crnt_dic_idx = *dic_idx_colsp[i];
    auto crnt_dic_idx_size = crnt_dic_idx.size();
    for(size_t j = 0; j < crnt_dic_idx_size; j++) {
      newdic[crnt_dic_idx[j]] = 0;
    }
  }
  size_t i = 0;
  for(auto it = newdic.begin(); it != newdic.end(); ++it, ++i) {
    newdic_idx.push_back(it->first);
    it->second = i + nodeinfo;
  }
}

void union_columns_string_resize_newval
(std::vector<size_t>& newval, std::vector<std::vector<size_t>*>& val_colsp) {
  auto cols_size = val_colsp.size();
  std::vector<size_t> val_sizes(cols_size);
  auto val_colspp = val_colsp.data();
  auto val_sizesp = val_sizes.data();
  for(size_t i = 0; i < cols_size; i++) {
    val_sizesp[i] = val_colspp[i]->size();
  }
  size_t total_val_size = 0;
  for(size_t i = 0; i < cols_size; i++) {
    total_val_size += val_sizesp[i];
  }
  newval.resize(total_val_size);
}

node_local<vector<size_t>>
union_columns_string_prepare(node_local<my_map<std::string, size_t>>& newdic,
                             node_local<my_map<std::string, size_t>>& dic,
                             node_local<std::vector<size_t>>& val) {
  auto nlfrom = make_node_local_allocate<std::vector<size_t>>();
  auto nlto = make_node_local_allocate<std::vector<size_t>>();
  newdic.mapv(create_string_trans_table, dic, nlfrom, nlto);
  auto bcastfrom = broadcast(nlfrom.moveto_dvector<size_t>().gather());
  auto bcastto = broadcast(nlto.moveto_dvector<size_t>().gather());
  return val.map(equal_prepare_helper, bcastfrom, bcastto);
}

void union_columns_string_update_nulls
(std::vector<size_t>& newnulls,
 std::vector<std::vector<size_t>*>& val_colsp,
 std::vector<std::vector<size_t>*>& nulls_colsp) {

  auto cols_size = val_colsp.size();
  std::vector<size_t> val_sizes(cols_size);
  auto val_colspp = val_colsp.data();
  auto val_sizesp = val_sizes.data();
  for(size_t i = 0; i < cols_size; i++) {
    val_sizesp[i] = val_colspp[i]->size();
  }
  std::vector<size_t> nulls_sizes(cols_size);
  auto nulls_colspp = nulls_colsp.data();
  auto nulls_sizesp = nulls_sizes.data();
  for(size_t i = 0; i < cols_size; i++) {
    nulls_sizesp[i] = nulls_colspp[i]->size();
  }
  size_t total_nulls_size = 0;
  for(size_t i = 0; i < cols_size; i++) {
    total_nulls_size += nulls_sizesp[i];
  }
  newnulls.resize(total_nulls_size);
  auto crnt_newnullsp = newnulls.data();
  auto crnt_shift = 0;
  for(size_t i = 0; i < cols_size; i++) {
    auto nulls_size = nulls_sizesp[i];
    auto nullsp = nulls_colspp[i]->data();
    for(size_t j = 0; j < nulls_size; j++) {
      crnt_newnullsp[j] = nullsp[j] + crnt_shift;
    }
    crnt_shift += val_sizesp[i];
    crnt_newnullsp += nulls_size;
  }
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<std::string>::union_columns
(const std::vector<std::shared_ptr<dfcolumn>>& cols) {
  auto cols_size = cols.size();
  if(cols_size == 0) {
    return std::make_shared<typed_dfcolumn<std::string>>
      (val, nulls, dic, dic_idx);
  }
  std::vector<std::shared_ptr<typed_dfcolumn<std::string>>> rights(cols_size);
  for(size_t i = 0; i < cols_size; i++) {
    rights[i] = std::dynamic_pointer_cast<typed_dfcolumn<std::string>>(cols[i]);
    if(!static_cast<bool>(rights[i]))
      throw std::runtime_error("union_columns: different type");
  }
  auto val_colsp =
    make_node_local_allocate<std::vector<std::vector<size_t>*>>();
  auto nulls_colsp =
    make_node_local_allocate<std::vector<std::vector<size_t>*>>();
  auto dic_colsp =
    make_node_local_allocate<std::vector<my_map<std::string,size_t>*>>();
  auto dic_idx_colsp =
    make_node_local_allocate<std::vector<std::vector<std::string>*>>();
  val.mapv(+[](std::vector<size_t>& val,
               std::vector<std::vector<size_t>*>& val_colsp)
           {val_colsp.push_back(&val);}, val_colsp);
  nulls.mapv(+[](std::vector<size_t>& nulls,
                 std::vector<std::vector<size_t>*>& nulls_colsp)
             {nulls_colsp.push_back(&nulls);}, nulls_colsp);
  dic->viewas_node_local().
    mapv(+[](my_map<std::string,size_t>& dic,
             std::vector<my_map<std::string,size_t>*>& dic_colsp)
         {dic_colsp.push_back(&dic);}, dic_colsp);
  dic_idx->mapv(+[](std::vector<std::string>& dic_idx,
                    std::vector<std::vector<std::string>*>& dic_idx_colsp)
                {dic_idx_colsp.push_back(&dic_idx);}, dic_idx_colsp);
  for(size_t i = 0; i < cols_size; i++) {
    rights[i]->val.mapv(+[](std::vector<size_t>& val,
                            std::vector<std::vector<size_t>*>& val_colsp)
                        {val_colsp.push_back(&val);}, val_colsp);
    rights[i]->nulls.mapv(+[](std::vector<size_t>& nulls,
                              std::vector<std::vector<size_t>*>& nulls_colsp)
                          {nulls_colsp.push_back(&nulls);}, nulls_colsp);
    rights[i]->dic->viewas_node_local().
      mapv(+[](my_map<std::string,size_t>& dic,
               std::vector<my_map<std::string,size_t>*>& dic_colsp)
           {dic_colsp.push_back(&dic);}, dic_colsp);
    rights[i]->dic_idx->
      mapv(+[](std::vector<std::string>& dic_idx,
               std::vector<std::vector<std::string>*>& dic_idx_colsp)
           {dic_idx_colsp.push_back(&dic_idx);}, dic_idx_colsp);
  }
  auto newval = make_node_local_allocate<std::vector<size_t>>();
  auto newnulls = make_node_local_allocate<std::vector<size_t>>();
  auto newdic = make_shared<dunordered_map<std::string,size_t>>
    (make_dunordered_map_allocate<std::string,size_t>());
  auto newdic_idx = make_shared<node_local<vector<std::string>>>
    (make_node_local_allocate<vector<std::string>>());
  auto newdicnl = newdic->viewas_node_local();
  newdicnl.mapv(union_columns_string_create_dic, *newdic_idx,
                dic_colsp, dic_idx_colsp);
  newval.mapv(union_columns_string_resize_newval, val_colsp);
  auto crnt_pos = broadcast(size_t(0));
  {
    auto newdicnl = newdic->viewas_node_local(); // for fail safe
    auto dicnl = dic->viewas_node_local();
    auto crnt_newval = union_columns_string_prepare(newdicnl, dicnl, val);
    newval.mapv
      (+[](std::vector<size_t>& newval, std::vector<size_t>& val, size_t& pos) {
        auto crnt_newvalp = newval.data() + pos;
        auto valp = val.data();
        auto val_size = val.size();
        for(size_t i = 0; i < val_size; i++) {crnt_newvalp[i] = valp[i];}
        pos += val_size;
      }, crnt_newval, crnt_pos);
  }
  for(size_t i = 0; i < cols_size; i++) {
    auto newdicnl = newdic->viewas_node_local(); // for fail safe
    auto dicnl = rights[i]->dic->viewas_node_local();
    auto crnt_newval =
      union_columns_string_prepare(newdicnl, dicnl, rights[i]->val);
    newval.mapv
      (+[](std::vector<size_t>& newval, std::vector<size_t>& val, size_t& pos) {
        auto crnt_newvalp = newval.data() + pos;
        auto valp = val.data();
        auto val_size = val.size();
        for(size_t i = 0; i < val_size; i++) {crnt_newvalp[i] = valp[i];}
        pos += val_size;
      }, crnt_newval, crnt_pos);
  }
  newnulls.mapv(union_columns_string_update_nulls, val_colsp, nulls_colsp);
  return std::make_shared<typed_dfcolumn<std::string>>
    (std::move(newval), std::move(newnulls),
     std::move(newdic), std::move(newdic_idx));
}

// ----- datetime -----
std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::union_columns
(const std::vector<std::shared_ptr<dfcolumn>>& cols) {
  auto cols_size = cols.size();
  if(cols_size == 0) {
    return std::make_shared<typed_dfcolumn<datetime>>(val, nulls);
  }
  std::vector<std::shared_ptr<typed_dfcolumn<datetime>>> rights(cols_size);
  for(size_t i = 0; i < cols_size; i++) {
    rights[i] = std::dynamic_pointer_cast<typed_dfcolumn<datetime>>(cols[i]);
    if(!static_cast<bool>(rights[i]))
      throw std::runtime_error("union_columns: different type");
  }
  auto val_colsp =
    make_node_local_allocate<std::vector<std::vector<datetime_t>*>>();
  auto nulls_colsp =
    make_node_local_allocate<std::vector<std::vector<size_t>*>>();
  val.mapv(+[](std::vector<datetime_t>& val,
               std::vector<std::vector<datetime_t>*>& val_colsp)
           {val_colsp.push_back(&val);}, val_colsp);
  nulls.mapv(+[](std::vector<size_t>& nulls,
                 std::vector<std::vector<size_t>*>& nulls_colsp)
             {nulls_colsp.push_back(&nulls);}, nulls_colsp);
  for(size_t i = 0; i < cols_size; i++) {
    rights[i]->val.mapv(+[](std::vector<datetime_t>& val,
                            std::vector<std::vector<datetime_t>*>& val_colsp)
                        {val_colsp.push_back(&val);}, val_colsp);
    rights[i]->nulls.mapv(+[](std::vector<size_t>& nulls,
                              std::vector<std::vector<size_t>*>& nulls_colsp)
                          {nulls_colsp.push_back(&nulls);}, nulls_colsp);
  }
  auto newval = make_node_local_allocate<std::vector<datetime_t>>();
  auto newnulls = make_node_local_allocate<std::vector<size_t>>();
  newval.mapv(union_columns_helper<datetime_t>,
              newnulls, val_colsp, nulls_colsp);
  return std::make_shared<typed_dfcolumn<datetime>>(std::move(newval),
                                                    std::move(newnulls));
}


// ==================== head/tail ====================
node_local<std::vector<size_t>>
limit_nulls_head(node_local<std::vector<size_t>>& nulls, 
                 const std::vector<size_t>& sizes,
                 size_t limit);

node_local<std::vector<size_t>>
limit_nulls_tail(node_local<std::vector<size_t>>& nulls, 
                 const std::vector<size_t>& sizes,
                 const std::vector<size_t>& new_sizes,
                 size_t limit);

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::head(size_t limit) {
  auto ret = std::make_shared<typed_dfcolumn<T>>();
  ret->val = val.template viewas_dvector<T>().head(limit).moveto_node_local();
  if(contain_nulls) {
    ret->nulls = limit_nulls_head(nulls, sizes(), limit);
    ret->contain_nulls_check();
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();    
  }
  return ret;
}

template <class T>
std::shared_ptr<dfcolumn>
typed_dfcolumn<T>::tail(size_t limit) {
  auto ret = std::make_shared<typed_dfcolumn<T>>();
  ret->val = val.template viewas_dvector<T>().tail(limit).moveto_node_local();
  auto new_sizes = ret->val.template viewas_dvector<T>().sizes();
  if(contain_nulls) {
    ret->nulls = limit_nulls_tail(nulls, sizes(), new_sizes, limit);
    ret->contain_nulls_check();
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();    
  }
  return ret;
}

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::head(size_t limit);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::head(size_t limit);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::head(size_t limit);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::head(size_t limit);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::head(size_t limit);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::head(size_t limit);

template std::shared_ptr<dfcolumn>
typed_dfcolumn<int>::tail(size_t limit);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned int>::tail(size_t limit);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<long>::tail(size_t limit);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<unsigned long>::tail(size_t limit);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<float>::tail(size_t limit);
template std::shared_ptr<dfcolumn>
typed_dfcolumn<double>::tail(size_t limit);

// ----- dic_string -----
std::shared_ptr<dfcolumn>
typed_dfcolumn<dic_string>::head(size_t limit) {
  auto ret = std::make_shared<typed_dfcolumn<dic_string>>();
  ret->val = val.viewas_dvector<size_t>().head(limit).moveto_node_local();
  if(contain_nulls) {
    ret->nulls = limit_nulls_head(nulls, sizes(), limit);
    ret->contain_nulls = true;
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();    
  }
  ret->dic = dic;
  return ret;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<dic_string>::tail(size_t limit) {
  auto ret = std::make_shared<typed_dfcolumn<dic_string>>();
  ret->val = val.viewas_dvector<size_t>().tail(limit).moveto_node_local();
  auto new_sizes = ret->val.template viewas_dvector<size_t>().sizes();
  if(contain_nulls) {
    ret->nulls = limit_nulls_tail(nulls, sizes(), new_sizes, limit);
    ret->contain_nulls = true;
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();    
  }
  ret->dic = dic;
  return ret;
}

// ----- raw_string -----
compressed_words raw_string_head_helper(const compressed_words& cws,
                                        size_t to_trunc) {
  auto num_words = cws.num_words();
  if(num_words > to_trunc) {
    std::vector<size_t> idx(to_trunc);
    auto idxp = idx.data();
    for(size_t i = 0; i < to_trunc; i++) idxp[i] = i;
    return cws.extract(idx);
  } else return cws;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<raw_string>::head(size_t limit) {
  auto ret = std::make_shared<typed_dfcolumn<raw_string>>();
  auto all_sizes = sizes();
  size_t node_size = get_nodesize();
  std::vector<size_t> trunc_sizes(node_size);
  auto to_trunc_size = limit;
  for(size_t i = 0; i < node_size; i++) {
    trunc_sizes[i] = std::min(to_trunc_size, all_sizes[i]);
    if(to_trunc_size > all_sizes[i]) {
      to_trunc_size -= all_sizes[i];
    } else to_trunc_size = 0;
  }
  auto dtrunc_sizes = make_node_local_scatter(trunc_sizes);
  ret->comp_words = comp_words.map(raw_string_head_helper, dtrunc_sizes);

  if(contain_nulls) {
    ret->nulls = limit_nulls_head(nulls, sizes(), limit);
    ret->contain_nulls_check();    
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();    
  }
  return ret;
}


compressed_words raw_string_tail_helper(const compressed_words& cws,
                                        size_t to_trunc) {
  auto num_words = cws.num_words();
  if(num_words > to_trunc) {
    auto shift = num_words - to_trunc;
    std::vector<size_t> idx(to_trunc);
    auto idxp = idx.data();
    for(size_t i = 0; i < to_trunc; i++) idxp[i] = shift + i;
    return cws.extract(idx);
  } else return cws;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<raw_string>::tail(size_t limit) {
  auto ret = std::make_shared<typed_dfcolumn<raw_string>>();
  auto all_sizes = sizes();
  size_t node_size = get_nodesize();
  std::vector<size_t> trunc_sizes(node_size);
  auto to_trunc_size = limit;
  for(size_t i = 0; i < node_size; i++) {
    auto current_node = node_size - i - 1;
    trunc_sizes[current_node] =
      std::min(to_trunc_size, all_sizes[current_node]);
    if(to_trunc_size > all_sizes[current_node]) {
      to_trunc_size -= all_sizes[current_node];
    } else to_trunc_size = 0;
  }
  auto dtrunc_sizes = make_node_local_scatter(trunc_sizes);
  ret->comp_words = comp_words.map(raw_string_tail_helper, dtrunc_sizes);

  if(contain_nulls) {
    ret->nulls = limit_nulls_tail(nulls, all_sizes, trunc_sizes, limit);
    ret->contain_nulls_check();
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();    
  }
  return ret;
}

// ----- string -----
std::shared_ptr<dfcolumn>
typed_dfcolumn<string>::head(size_t limit) {
  auto ret = std::make_shared<typed_dfcolumn<std::string>>();
  ret->val = val.viewas_dvector<size_t>().head(limit).moveto_node_local();
  if(contain_nulls) {
    ret->nulls = limit_nulls_head(nulls, sizes(), limit);
    ret->contain_nulls_check();
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();    
  }
  ret->dic = dic;
  ret->dic_idx = dic_idx;
  return ret;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<string>::tail(size_t limit) {
  auto ret = std::make_shared<typed_dfcolumn<std::string>>();
  ret->val = val.viewas_dvector<size_t>().tail(limit).moveto_node_local();
  auto new_sizes = ret->val.template viewas_dvector<size_t>().sizes();
  if(contain_nulls) {
    ret->nulls = limit_nulls_tail(nulls, sizes(), new_sizes, limit);
    ret->contain_nulls_check();
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();    
  }
  ret->dic = dic;
  ret->dic_idx = dic_idx;
  return ret;
}

// ----- datetime -----
std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::head(size_t limit) {
  auto ret = std::make_shared<typed_dfcolumn<datetime>>();
  ret->val =
    val.template viewas_dvector<datetime_t>().head(limit).moveto_node_local();
  if(contain_nulls) {
    ret->nulls = limit_nulls_head(nulls, sizes(), limit);
    ret->contain_nulls_check();
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();    
  }
  return ret;
}

std::shared_ptr<dfcolumn>
typed_dfcolumn<datetime>::tail(size_t limit) {
  auto ret = std::make_shared<typed_dfcolumn<datetime>>();
  ret->val =
    val.template viewas_dvector<datetime_t>().tail(limit).moveto_node_local();
  auto new_sizes = ret->val.template viewas_dvector<datetime_t>().sizes();
  if(contain_nulls) {
    ret->nulls = limit_nulls_tail(nulls, sizes(), new_sizes, limit);
    ret->contain_nulls_check();
  } else {
    ret->nulls = make_node_local_allocate<std::vector<size_t>>();    
  }
  return ret;
}

// ==================== dtype ====================
template <class T>
std::string typed_dfcolumn<T>::dtype() const {
  return get_dftype_name<T>();
}

template std::string typed_dfcolumn<int>::dtype() const;
template std::string typed_dfcolumn<unsigned int>::dtype() const;
template std::string typed_dfcolumn<long>::dtype() const;
template std::string typed_dfcolumn<unsigned long>::dtype() const;
template std::string typed_dfcolumn<float>::dtype() const;
template std::string typed_dfcolumn<double>::dtype() const;

// ==================== save ====================
template <class T>
void typed_dfcolumn<T>::save(const std::string& file) {
  val.template viewas_dvector<T>().savebinary(file);
  auto dv_nulls = nulls.template viewas_dvector<size_t>();
  auto sizes = val.template viewas_dvector<T>().sizes();
  auto sizesp = sizes.data();
  std::vector<size_t> pxsizes(sizes.size());
  auto pxsizesp = pxsizes.data();
  auto pxsizessize = pxsizes.size();
  for(size_t i = 1; i < pxsizessize; i++) {
    pxsizesp[i] += pxsizesp[i-1] + sizesp[i-1];
  }
  auto nl_sizes = make_node_local_scatter(pxsizes);
  dv_nulls.map<size_t>(shift_local_index(), nl_sizes).
    savebinary(file+"_nulls");
}

template void typed_dfcolumn<int>::save(const std::string& file);
template void typed_dfcolumn<unsigned int>::save(const std::string& file);
template void typed_dfcolumn<long>::save(const std::string& file);
template void typed_dfcolumn<unsigned long>::save(const std::string& file);
template void typed_dfcolumn<float>::save(const std::string& file);
template void typed_dfcolumn<double>::save(const std::string& file);

// ----- dic_string -----
// TODO: shrink unused dic
void typed_dfcolumn<dic_string>::save(const std::string& file) {
  vector<size_t> new_starts; // not used
  auto to_save =
    int_to_vchar(concat_words(dic->decompress(), "\n", new_starts));
  savebinary_local(to_save, file + "_dic");
  val.viewas_dvector<size_t>().savebinary(file + "_idx");
  auto dv_nulls = nulls.template viewas_dvector<size_t>();
  auto sizes = val.template viewas_dvector<size_t>().sizes();
  auto sizesp = sizes.data();
  std::vector<size_t> pxsizes(sizes.size());
  auto pxsizesp = pxsizes.data();
  auto pxsizessize = pxsizes.size();
  for(size_t i = 1; i < pxsizessize; i++) {
    pxsizesp[i] += pxsizesp[i-1] + sizesp[i-1];
  }
  auto nl_sizes = make_node_local_scatter(pxsizes);
  dv_nulls.map<size_t>(shift_local_index(), nl_sizes).
    savebinary(file+"_nulls");
}

// ----- raw_string -----
// representation of NULL is not normalized, which is OK because of _nulls file
void typed_dfcolumn<raw_string>::save(const std::string& file) {
  comp_words.map(+[](const compressed_words& cws) {
      vector<size_t> new_starts; // not used
      return int_to_vchar(concat_words(cws.decompress(), "\n", new_starts));
    }).moveto_dvector<char>().savebinary(file);
  auto dv_nulls = nulls.template viewas_dvector<size_t>();
  auto sizes = this->sizes();
  auto sizesp = sizes.data();
  std::vector<size_t> pxsizes(sizes.size());
  auto pxsizesp = pxsizes.data();
  auto pxsizessize = pxsizes.size();
  for(size_t i = 1; i < pxsizessize; i++) {
    pxsizesp[i] += pxsizesp[i-1] + sizesp[i-1];
  }
  auto nl_sizes = make_node_local_scatter(pxsizes);
  dv_nulls.map<size_t>(shift_local_index(), nl_sizes).
    savebinary(file+"_nulls");
}

// ----- string -----
vector<size_t> to_contiguous_idx(vector<size_t>& idx, vector<size_t>& sizes) {
  size_t nodemask = (size_t(1) << DFNODESHIFT) - 1;
  int nodesize = get_nodesize();
  vector<size_t> toadd(nodesize);
  size_t* toaddp = toadd.data();
  size_t* sizesp = sizes.data();
  for(size_t i = 1; i < nodesize; i++) {
    toaddp[i] = toaddp[i-1] + sizesp[i-1];
  }
  size_t size = idx.size();
  vector<size_t> ret(size);
  size_t* idxp = idx.data();
  size_t* retp = ret.data();
  size_t max = std::numeric_limits<size_t>::max();
#pragma cdir nodep
#pragma _NEC ivdep
  for(size_t i = 0; i < size; i++) {
    if(idxp[i] != max) {
      int node = idxp[i] >> DFNODESHIFT;
      retp[i] = (idxp[i] & nodemask) + toaddp[node];
    }
    else retp[i] = idxp[i];
  }
  return ret;
}

vector<vector<size_t>> split_by_node(vector<size_t>& idx) {
  size_t nodemask = (size_t(1) << DFNODESHIFT) - 1;
  int nodesize = get_nodesize();
  size_t max = std::numeric_limits<size_t>::max();
  vector<vector<size_t>> split(nodesize);
  vector<size_t> each_size(nodesize);
  size_t* each_sizep = each_size.data();
  size_t size = idx.size();
  size_t* idxp = idx.data();
#pragma cdir nodep
#pragma _NEC ivdep
  for(size_t i = 0; i < size; i++) {
    if(idxp[i] != max) {
      int node = idxp[i] >> DFNODESHIFT;
      each_sizep[node]++;
    }
  }
  for(size_t i = 0; i < nodesize; i++) {
    split[i].resize(each_size[i]);
  }
  vector<size_t*> each_split(nodesize);
  size_t** each_splitp = each_split.data();
  vector<size_t> each_split_current(nodesize);
  size_t* each_split_currentp = each_split_current.data();
  for(size_t i = 0; i < nodesize; i++) each_splitp[i] = split[i].data();
#pragma cdir nodep
#pragma _NEC ivdep
  for(size_t i = 0; i < size; i++) {
    if(idxp[i] != max) {
      int node = idxp[i] >> DFNODESHIFT;
      each_splitp[node][each_split_currentp[node]] = idxp[i] & nodemask;
      each_split_currentp[node]++;
    }
  }
  return split;
}

void sort_idx(vector<size_t>& idx) {
#if defined(_SX) || defined(__ve__)
  std::vector<size_t> dummy(idx.size());
  radix_sort(idx, dummy);
#else
  std::sort(idx.begin(), idx.end());
#endif
}

vector<size_t> create_exist_map(vector<size_t>& exist_idx,
                                vector<size_t>& table_val) {
  size_t selfid = static_cast<size_t>(get_selfid());
  size_t nodeinfo = selfid << DFNODESHIFT;
  size_t* exist_idxp = exist_idx.data();
  size_t size = exist_idx.size();
  size_t tbsize;
  if(selfid == 0) tbsize = size + 1;
  else tbsize = size;
  vector<size_t> table_key(tbsize);
  size_t* table_keyp = table_key.data();
  table_val.resize(tbsize);
  size_t* table_valp = table_val.data();
  for(size_t i = 0; i < size; i++) {
    table_keyp[i] = exist_idxp[i] + nodeinfo;
    table_valp[i] = i + nodeinfo;
  }
  if(selfid == 0) {
    size_t max = std::numeric_limits<size_t>::max();
    table_keyp[size] = max;
    table_valp[size] = max;
  }
  return table_key;
}

vector<size_t> shrink_string_idx(vector<size_t>& val,
                                 vector<size_t>& table_key,
                                 vector<size_t>& table_val) {
  unique_hashtable<size_t, size_t> ht(table_key, table_val);
  return ht.lookup(val);
}

vector<string> shrink_string_dic(vector<string>& dic_idx,
                                 vector<size_t>& exist_idx) {
  size_t size = exist_idx.size();
  vector<string> ret(size);
  for(size_t i = 0; i < size; i++) {
    ret[i] = dic_idx[exist_idx[i]];
  }
  return ret;
}

void typed_dfcolumn<string>::save(const std::string& file) {
  auto split = val.map(split_by_node);
  auto exchanged = alltoall_exchange(split);
  auto exist_idx = exchanged.map(flatten<size_t>).map(get_unique_idx);
  if(exist_idx.viewas_dvector<size_t>().size() !=
     dic_idx->viewas_dvector<string>().size()) {
    exist_idx.mapv(sort_idx);
    auto table_val = make_node_local_allocate<vector<size_t>>();
    auto table_key = exist_idx.map(create_exist_map, table_val);
    auto bcast_table_key =
      broadcast(table_key.viewas_dvector<size_t>().gather());
    auto bcast_table_val =
      broadcast(table_val.viewas_dvector<size_t>().gather());
    auto new_val = val.map(shrink_string_idx, bcast_table_key,
                           bcast_table_val);
    auto new_dic_idx = dic_idx->map(shrink_string_dic,
                                    exist_idx);
    auto dicsizes = new_dic_idx.viewas_dvector<string>().sizes();
    new_val.map(to_contiguous_idx, broadcast(dicsizes)).
      moveto_dvector<size_t>().savebinary(file + "_idx");
    new_dic_idx.viewas_dvector<string>().saveline(file + "_dic");
  } else {
    auto dicsizes = dic_idx->viewas_dvector<string>().sizes();
    val.map(to_contiguous_idx, broadcast(dicsizes)).
      moveto_dvector<size_t>().savebinary(file + "_idx");
    dic_idx->viewas_dvector<string>().saveline(file + "_dic");
  }
  auto dv_nulls = nulls.template viewas_dvector<size_t>();
  auto sizes = val.template viewas_dvector<size_t>().sizes();
  std::vector<size_t> pxsizes(sizes.size());
  for(size_t i = 1; i < pxsizes.size(); i++) {
    pxsizes[i] += pxsizes[i-1] + sizes[i-1];
  }
  auto nl_sizes = make_node_local_scatter(pxsizes);
  dv_nulls.map<size_t>(shift_local_index(), nl_sizes).
    savebinary(file+"_nulls");
}

// ==================== contain_nulls_check ====================
template <class T>
void typed_dfcolumn<T>::contain_nulls_check() {
  if(nulls.template viewas_dvector<size_t>().size() == 0)
    contain_nulls = false;
  else contain_nulls = true;
}

template void typed_dfcolumn<int>::contain_nulls_check();
template void typed_dfcolumn<unsigned int>::contain_nulls_check();
template void typed_dfcolumn<long>::contain_nulls_check();
template void typed_dfcolumn<unsigned long>::contain_nulls_check();
template void typed_dfcolumn<float>::contain_nulls_check();
template void typed_dfcolumn<double>::contain_nulls_check();

// ----- dic_string -----
void typed_dfcolumn<dic_string>::contain_nulls_check() {
  if(nulls.template viewas_dvector<size_t>().size() == 0)
    contain_nulls = false;
  else contain_nulls = true;
}

// ----- raw_string -----
void typed_dfcolumn<raw_string>::contain_nulls_check() {
  if(nulls.template viewas_dvector<size_t>().size() == 0)
    contain_nulls = false;
  else contain_nulls = true;
}

// ----- string -----
void typed_dfcolumn<string>::contain_nulls_check() {
  if(nulls.template viewas_dvector<size_t>().size() == 0)
    contain_nulls = false;
  else contain_nulls = true;
}


// ==================== is_unique ====================
template <class T>
bool vector_is_unique(const std::vector<T>& vec) {
  int unq = 0;
  auto dummy = vector_zeros<int>(vec.size()); 
  unique_hashtable<T,int> obj(vec, dummy, unq);
  return unq == 1;
}

template <class T>
bool typed_dfcolumn<T>::is_unique() {
  auto& typed_col = dynamic_cast<typed_dfcolumn<T>&>(*this);
  // TODO: optimize without gathering the column
  auto key = typed_col.get_val().template viewas_dvector<T>().gather();
  auto nulls_count = typed_col.get_nulls().template viewas_dvector<size_t>().size();
  return (nulls_count <= 1) && vector_is_unique(key);
}

template bool typed_dfcolumn<int>::is_unique();
template bool typed_dfcolumn<unsigned int>::is_unique();
template bool typed_dfcolumn<long>::is_unique();
template bool typed_dfcolumn<unsigned long>::is_unique();
template bool typed_dfcolumn<float>::is_unique();
template bool typed_dfcolumn<double>::is_unique();

// ----- dic_string -----
bool typed_dfcolumn<dic_string>::is_unique() {
  auto& typed_col = dynamic_cast<typed_dfcolumn<dic_string>&>(*this);
  auto key = typed_col.val.viewas_dvector<size_t>().gather();
  auto nulls_count = typed_col.get_nulls().viewas_dvector<size_t>().size();
  return (nulls_count <= 1) && vector_is_unique(key);
}

// ----- string -----
bool typed_dfcolumn<string>::is_unique() {
  auto& typed_col = dynamic_cast<typed_dfcolumn<string>&>(*this);
  auto key = typed_col.val.viewas_dvector<size_t>().gather();
  auto nulls_count = typed_col.get_nulls().viewas_dvector<size_t>().size();
  return (nulls_count <= 1) && vector_is_unique(key);
}


// ==================== is_all_null ====================
template <class T>
bool typed_dfcolumn<T>::is_all_null() {
  return val.map(+[](std::vector<T>& val, std::vector<size_t>& nulls)
                 {return val.size() == nulls.size();}, nulls).
    reduce(+[](bool left, bool right){return left && right;});
}

template bool typed_dfcolumn<int>::is_all_null();
template bool typed_dfcolumn<unsigned int>::is_all_null();
template bool typed_dfcolumn<long>::is_all_null();
template bool typed_dfcolumn<unsigned long>::is_all_null();
template bool typed_dfcolumn<float>::is_all_null();
template bool typed_dfcolumn<double>::is_all_null();

// ----- dic_string -----
bool typed_dfcolumn<dic_string>::is_all_null() {
  return val.map(+[](std::vector<size_t>& val, std::vector<size_t>& nulls)
                 {return val.size() == nulls.size();}, nulls).
    reduce(+[](bool left, bool right){return left && right;});
}

// ----- string -----
bool typed_dfcolumn<string>::is_all_null() {
  return val.map(+[](std::vector<size_t>& val, std::vector<size_t>& nulls)
                 {return val.size() == nulls.size();}, nulls).
    reduce(+[](bool left, bool right){return left && right;});
}


// ==================== spill/restore ====================
template <class T>
void typed_dfcolumn<T>::spill_to_disk() {
  if(already_spilled_to_disk) {
    val.mapv(+[](std::vector<T>& v){
        std::vector<T> tmp;
        tmp.swap(v);
      });
    nulls.mapv(+[](std::vector<size_t>& n){
        std::vector<size_t> tmp;
        tmp.swap(n);
      });
  } else {
    val.mapv(+[](std::vector<T>& v, std::string& spill_path){
        savebinary_local(v, spill_path+"/val");
        std::vector<T> tmp;
        tmp.swap(v);
      }, spill_path);
    nulls.mapv(+[](std::vector<size_t>& n, std::string& spill_path){
        savebinary_local(n, spill_path+"/nulls");
        std::vector<size_t> tmp;
        tmp.swap(n);
      }, spill_path);
    already_spilled_to_disk = true;
  }
  cleared = true;
}

template <class T>
void typed_dfcolumn<T>::restore_from_disk() {
  val.mapv(+[](std::vector<T>& v, std::string& spill_path){
      v = loadbinary_local<T>(spill_path+"/val");
    }, spill_path);
  nulls.mapv(+[](std::vector<size_t>& n, std::string& spill_path){
      n = loadbinary_local<size_t>(spill_path+"/nulls");
    }, spill_path);
  cleared = false;
}

template <class T>
size_t typed_dfcolumn<T>::calc_spill_size() {
  auto valsize = size();
  auto nullsize = nulls.map(+[](std::vector<size_t>& n){return n.size();}).
    reduce(+[](const size_t l, const size_t r){return l + r;});
  return valsize * sizeof(T) + nullsize * sizeof(size_t);
}

template void typed_dfcolumn<int>::spill_to_disk();
template void typed_dfcolumn<unsigned int>::spill_to_disk();
template void typed_dfcolumn<long>::spill_to_disk();
template void typed_dfcolumn<unsigned long>::spill_to_disk();
template void typed_dfcolumn<float>::spill_to_disk();
template void typed_dfcolumn<double>::spill_to_disk();

template void typed_dfcolumn<int>::restore_from_disk();
template void typed_dfcolumn<unsigned int>::restore_from_disk();
template void typed_dfcolumn<long>::restore_from_disk();
template void typed_dfcolumn<unsigned long>::restore_from_disk();
template void typed_dfcolumn<float>::restore_from_disk();
template void typed_dfcolumn<double>::restore_from_disk();

template size_t typed_dfcolumn<int>::calc_spill_size();
template size_t typed_dfcolumn<unsigned int>::calc_spill_size();
template size_t typed_dfcolumn<long>::calc_spill_size();
template size_t typed_dfcolumn<unsigned long>::calc_spill_size();
template size_t typed_dfcolumn<float>::calc_spill_size();
template size_t typed_dfcolumn<double>::calc_spill_size();

// ----- dic_string -----
void typed_dfcolumn<dic_string>::spill_to_disk() {
  if(already_spilled_to_disk) {
    val.mapv(+[](std::vector<size_t>& v){
        std::vector<size_t> tmp;
        tmp.swap(v);
      });
    nulls.mapv(+[](std::vector<size_t>& n){
        std::vector<size_t> tmp;
        tmp.swap(n);
      });
    dic.reset();
  } else {
    val.mapv(+[](std::vector<size_t>& v, std::string& spill_path){
        savebinary_local(v, spill_path+"/val");
        std::vector<size_t> tmp;
        tmp.swap(v);
      }, spill_path);
    nulls.mapv(+[](std::vector<size_t>& n, std::string& spill_path){
        savebinary_local(n, spill_path+"/nulls");
        std::vector<size_t> tmp;
        tmp.swap(n);
      }, spill_path);
    savebinary_local(dic->cwords, spill_path.get(0)+"/dic_cwords");
    savebinary_local(dic->lens, spill_path.get(0)+"/dic_lens");
    savebinary_local(dic->lens_num, spill_path.get(0)+"/dic_lens_num");
    dic.reset();
    already_spilled_to_disk = true;
  }
  cleared = true;
}

void typed_dfcolumn<dic_string>::restore_from_disk() {
  val.mapv(+[](std::vector<size_t>& v, std::string& spill_path){
      v = loadbinary_local<size_t>(spill_path+"/val");
    }, spill_path);
  nulls.mapv(+[](std::vector<size_t>& n, std::string& spill_path){
      n = loadbinary_local<size_t>(spill_path+"/nulls");
    }, spill_path);
  dic = make_shared<dict>();
  dic->cwords = loadbinary_local<uint64_t>(spill_path.get(0)+"/dic_cwords");
  dic->lens = loadbinary_local<size_t>(spill_path.get(0)+"/dic_lens");
  dic->lens_num = loadbinary_local<size_t>(spill_path.get(0)+"/dic_lens_num");
  cleared = false;
}

size_t typed_dfcolumn<dic_string>::calc_spill_size() {
  auto valsize = size();
  auto nullsize = nulls.map(+[](std::vector<size_t>& n){return n.size();}).
    reduce(+[](const size_t l, const size_t r){return l + r;});
  auto dicsize = dic->cwords.size() * sizeof(uint64_t) +
    dic->lens.size() * sizeof(size_t) + dic->lens_num.size() * sizeof(size_t);
  return valsize * sizeof(size_t) + nullsize * sizeof(size_t) + dicsize;
}

// ----- raw_string -----
void save_compressed_words(const compressed_words& cw,
                           const std::string& path) {
  savebinary_local(cw.cwords, path + "/cwords");
  savebinary_local(cw.lens, path + "/lens");
  savebinary_local(cw.lens_num, path + "/lens_num");
  savebinary_local(cw.order, path + "/order");
}

compressed_words load_compressed_words(const std::string& path) {
  compressed_words cw;
  cw.cwords = loadbinary_local<uint64_t>(path + "/cwords");
  cw.lens = loadbinary_local<size_t>(path + "/lens");
  cw.lens_num = loadbinary_local<size_t>(path + "/lens_num");
  cw.order = loadbinary_local<size_t>(path + "/order");
  return cw;
}

void typed_dfcolumn<raw_string>::spill_to_disk() {
  if(already_spilled_to_disk) {
    comp_words.mapv(+[](compressed_words& cw){
        cw.clear();
      });
    nulls.mapv(+[](std::vector<size_t>& n){
        std::vector<size_t> tmp;
        tmp.swap(n);
      });
  } else {
    comp_words.mapv(+[](compressed_words& cw, std::string& spill_path){
        make_directory(spill_path + "/comp_words");
        save_compressed_words(cw, spill_path + "/comp_words");
        cw.clear();
      }, spill_path);
    nulls.mapv(+[](std::vector<size_t>& n, std::string& spill_path){
        savebinary_local(n, spill_path+"/nulls");
        std::vector<size_t> tmp;
        tmp.swap(n);
      }, spill_path);
    already_spilled_to_disk = true;
  }
  cleared = true;
}

void typed_dfcolumn<raw_string>::restore_from_disk() {
  comp_words.mapv(+[](compressed_words& cw, std::string& spill_path){
      cw = load_compressed_words(spill_path + "/comp_words");
    }, spill_path);
  nulls.mapv(+[](std::vector<size_t>& n, std::string& spill_path){
      n = loadbinary_local<size_t>(spill_path+"/nulls");
    }, spill_path);
  cleared = false;
}

size_t compressed_words_size(compressed_words& cw) {
  return
    cw.cwords.size() * sizeof(uint64_t) +
    cw.lens.size() * sizeof(size_t) +
    cw.lens_num.size() * sizeof(size_t) +
    cw.order.size() * sizeof(size_t);
}

size_t typed_dfcolumn<raw_string>::calc_spill_size() {
  auto cwsize = comp_words.map(+[](compressed_words& cw)
                               {return compressed_words_size(cw);}).
    reduce(+[](const size_t l, const size_t r){return l + r;});
  auto nullsize = nulls.map(+[](std::vector<size_t>& n){return n.size();}).
    reduce(+[](const size_t l, const size_t r){return l + r;});
  return cwsize + nullsize * sizeof(size_t);
}

// ----- string -----
// TODO: spill dic and dic_idx; currently they are shared_ptr,
// so it is difficult to deallocate them
void typed_dfcolumn<string>::spill_to_disk() {
  if(already_spilled_to_disk) {
    val.mapv(+[](std::vector<size_t>& v){
        std::vector<size_t> tmp;
        tmp.swap(v);
      });
    nulls.mapv(+[](std::vector<size_t>& n){
        std::vector<size_t> tmp;
        tmp.swap(n);
      });
  } else {
    val.mapv(+[](std::vector<size_t>& v, std::string& spill_path){
        savebinary_local(v, spill_path+"/val");
        std::vector<size_t> tmp;
        tmp.swap(v);
      }, spill_path);
    nulls.mapv(+[](std::vector<size_t>& n, std::string& spill_path){
        savebinary_local(n, spill_path+"/nulls");
        std::vector<size_t> tmp;
        tmp.swap(n);
      }, spill_path);
    already_spilled_to_disk = true;
  }
  cleared = true;
}

void typed_dfcolumn<string>::restore_from_disk() {
  val.mapv(+[](std::vector<size_t>& v, std::string& spill_path){
      v = loadbinary_local<size_t>(spill_path+"/val");
    }, spill_path);
  nulls.mapv(+[](std::vector<size_t>& n, std::string& spill_path){
      n = loadbinary_local<size_t>(spill_path+"/nulls");
    }, spill_path);
  cleared = false;
}

size_t typed_dfcolumn<string>::calc_spill_size() {
  auto valsize = size();
  auto nullsize = nulls.map(+[](std::vector<size_t>& n){return n.size();}).
    reduce(+[](const size_t l, const size_t r){return l + r;});
  return valsize * sizeof(size_t) + nullsize * sizeof(size_t);
}

}
