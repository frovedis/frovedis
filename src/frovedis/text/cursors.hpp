#ifndef CURSORS_HPFF
#define CURSORS_HPFF

#include "../core/find_condition.hpp"
#include "../core/set_operations.hpp"
#include <limits>

#define CURSORS_PAD 32

namespace frovedis {

struct is_boundary {
  int operator()(int a) const {
    return !((a >= '0' && a <= '9') || (a >= 'A' && a <= 'Z') ||
             (a >= 'a' && a <= 'z') || (a == '_'));
  }
};

struct is_word {
  int operator()(int a) const {
    return ((a >= '0' && a <= '9') || (a >= 'A' && a <= 'Z') ||
            (a >= 'a' && a <= 'z') || (a == '_'));
  }
};

struct cursors {
  cursors(const std::vector<int>& chars) : chars(chars) {}
  const std::vector<int>& chars;
  std::vector<size_t> starts;
  std::vector<size_t> ends;

  template <class F>
  void keep_if(const F& f);

  template <class F>
  void advance_if(const F& f);

  void advance_string(const std::string& str);

  template <class F>
  void advance_if_keep_else(const F& f);

  template <class F1, class F2>
  void advance_if_until(const F1& f1, const F2& f2, 
                        int max = std::numeric_limits<int>::max(),
                        bool advance_end = true);

  void merge(const cursors& c);

  void debug_print();
};

struct cond_table {
  int operator()(int a) const {return table[CURSORS_PAD * a];}
  int table[CURSORS_PAD * 256];
};

template <class F>
cond_table make_cond_table(const F& f) {
  cond_table ret;
  int* tbl = ret.table;
  for(size_t i = 0; i < 256; i++) {
    tbl[CURSORS_PAD * i] = static_cast<int>(f(i));
  }
  return ret;
}

template <class F>
cursors make_cursors_start_at(const std::vector<int>& chars, const F& startf) {
  cursors ret(chars);
  ret.starts = find_condition(chars, startf);
  auto size = ret.starts.size();
  auto startsp = ret.starts.data();
  ret.ends.resize(size);
  auto endsp = ret.ends.data();
  for(size_t i = 0; i < size; i++) {
    endsp[i] = startsp[i] + 1;
  }
  return ret;
}

template <class F>
cursors make_cursors_start_after(const std::vector<int>& chars,
                                 const F& startf, bool include_first = false) {
  cursors ret(chars);
  auto startstmp = find_condition(chars, startf);
  auto size = startstmp.size();
  if(include_first) {
    ret.starts.resize(size+1);
    auto startsp = ret.starts.data();
    ret.ends.resize(size+1);
    auto endsp = ret.ends.data();
    auto startstmpp = startstmp.data();
    for(size_t i = 0; i < size; i++) {
      startsp[i+1] = startstmpp[i] + 1;
      endsp[i+1] = startstmpp[i] + 1;
    }
  } else {
    ret.starts.resize(size);
    auto startsp = ret.starts.data();
    ret.ends.resize(size);
    auto endsp = ret.ends.data();
    auto startstmpp = startstmp.data();
    for(size_t i = 0; i < size; i++) {
      startsp[i] = startstmpp[i] + 1;
      endsp[i] = startstmpp[i] + 1;
    }
  }
  return ret;
}

template <class F>
void cursors::keep_if(const F& f) { // else drop, do not advance
  auto hit = find_condition_index(chars, ends, f);
  auto size = hit.size();
  std::vector<size_t> next_starts(size), next_ends(size);
  auto next_startsp = next_starts.data();
  auto next_endsp = next_ends.data();
  auto startsp = starts.data();
  auto endsp = ends.data();
  auto hitp = hit.data();

  for(size_t i = 0; i < size; i++) {
    next_startsp[i] = startsp[hitp[i]];
    next_endsp[i] = endsp[hitp[i]];
  }
  starts.swap(next_starts);
  ends.swap(next_ends);
}

template <class F>
void cursors::advance_if(const F& f) { // else drop
  auto hit = find_condition_index(chars, ends, f);
  auto size = hit.size();
  std::vector<size_t> next_starts(size), next_ends(size);
  auto next_startsp = next_starts.data();
  auto next_endsp = next_ends.data();
  auto startsp = starts.data();
  auto endsp = ends.data();
  auto hitp = hit.data();

  for(size_t i = 0; i < size; i++) {
    next_startsp[i] = startsp[hitp[i]];
    next_endsp[i] = endsp[hitp[i]] + 1;
  }
  starts.swap(next_starts);
  ends.swap(next_ends);
}

template <class F>
void cursors::advance_if_keep_else(const F& f) {
  auto hit = find_condition_index(chars, ends, f);
  auto size = hit.size();
  auto endsp = ends.data();
  auto hitp = hit.data();

  for(size_t i = 0; i < size; i++) {
    endsp[hitp[i]]++;
  }
}

template <class F1, class F2>
void cursors::advance_if_until(const F1& f1, const F2& f2, int max,
                               bool advance_end) {
  std::vector<size_t> crnt_starts, next_starts;
  std::vector<size_t> crnt_ends, next_ends;
  crnt_starts.swap(starts);
  crnt_ends.swap(ends);

  std::vector<std::vector<size_t>> until_hit_starts, until_hit_ends;
  until_hit_starts.reserve(1000);
  until_hit_ends.reserve(1000);
  for(int i = 0; i < max; i++) {
    auto until_hit = find_condition_index(chars, crnt_ends, f2);
    auto size = until_hit.size();
    std::vector<size_t> rest;
    auto crnt_ends_size = crnt_ends.size();
    std::vector<size_t> iota(crnt_ends_size);
    auto iotap = iota.data();
    for(size_t i = 0; i < crnt_ends_size; i++) iotap[i] = i;
    if(size > 0) {
      std::vector<size_t> until_hit_starts_tmp(size), until_hit_ends_tmp(size);
      auto until_hit_starts_tmpp = until_hit_starts_tmp.data();
      auto until_hit_ends_tmpp = until_hit_ends_tmp.data();
      auto crnt_startsp = crnt_starts.data();
      auto crnt_endsp = crnt_ends.data();
      auto until_hitp = until_hit.data();
      if(advance_end) {
        for(size_t i = 0; i < size; i++) {
          until_hit_starts_tmpp[i] = crnt_startsp[until_hitp[i]];
          until_hit_ends_tmpp[i] = crnt_endsp[until_hitp[i]] + 1;
        }
      } else {
        for(size_t i = 0; i < size; i++) {
          until_hit_starts_tmpp[i] = crnt_startsp[until_hitp[i]];
          until_hit_ends_tmpp[i] = crnt_endsp[until_hitp[i]];
        }
      }
      until_hit_starts.push_back(until_hit_starts_tmp);
      until_hit_ends.push_back(until_hit_ends_tmp);
      rest = set_difference(iota, until_hit);
    } else {
      rest.swap(iota);
    }

    auto rest_size = rest.size();
    if(rest_size == 0) break;
    else {
      next_starts.resize(rest_size);
      next_ends.resize(rest_size);
      auto next_startsp = next_starts.data();
      auto next_endsp = next_ends.data();
      auto crnt_startsp = crnt_starts.data();
      auto crnt_endsp = crnt_ends.data();
      auto restp = rest.data();
      for(size_t i = 0; i < rest_size; i++) {
        next_startsp[i] = crnt_startsp[restp[i]];
        next_endsp[i] = crnt_endsp[restp[i]];
      }
      crnt_starts.swap(next_starts);
      crnt_ends.swap(next_ends);
      auto hit = find_condition_index(chars, crnt_ends, f1);
      auto size = hit.size();
      if(size == 0) break;
      next_starts.resize(size);
      next_ends.resize(size);
      next_startsp = next_starts.data();
      next_endsp = next_ends.data();
      crnt_startsp = crnt_starts.data();
      crnt_endsp = crnt_ends.data();
      auto hitp = hit.data();
      for(size_t i = 0; i < size; i++) {
        next_startsp[i] = crnt_startsp[hitp[i]];
        next_endsp[i] = crnt_endsp[hitp[i]] + 1;
      }
      crnt_starts.swap(next_starts);
      crnt_ends.swap(next_ends);
    }
  }
  set_multimerge_pair(until_hit_starts, until_hit_ends, starts, ends);
}

void mask_string(std::vector<int>& v,
                 const std::vector<size_t>& begin,
                 const std::vector<size_t>& end,
                 int mask_char);

std::vector<int> replace_string(const std::vector<int>& v,
                                const std::vector<size_t>& begin,
                                const std::vector<size_t>& end,
                                const std::string& replace_to);

}
#endif
