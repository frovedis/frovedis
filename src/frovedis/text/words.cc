#include "words.hpp"
#include "find.hpp"
#include "../core/radix_sort.hpp"
#include "../core/set_operations.hpp"
#include "../core/lower_bound.hpp"
#include "../core/upper_bound.hpp"
#include "../core/prefix_sum.hpp"
#include "find_condition.hpp"
#include "char_int_conv.hpp"
#include <stdexcept>

using namespace std;

namespace frovedis {

void remove_doc_heading(const vector<int>& v,
                        vector<size_t>& sep, vector<size_t>& len,
                        const string& del) {
  for(size_t i = 0; i < sep.size(); i++) {
    auto head_end = find(v.data() + sep[i], len[i], del);
    size_t doc_start_offset = 0;
    if(head_end.size() != 0) doc_start_offset = head_end[0] + del.size();
    sep[i] += doc_start_offset;
    len[i] -= doc_start_offset;
  }
}

std::vector<int>
concat_docs(const vector<int>& v, const vector<size_t>& sep,
            const vector<size_t>& len, const int del,
            vector<size_t>& doc_start) {
  auto vp = v.data();
  auto sep_size = sep.size();
  auto sepp = sep.data();
  auto lenp = len.data();
  size_t total = 0;
  for(size_t i = 0; i < sep_size; i++) total += lenp[i];
  total += sep_size; // for delimiter
  vector<int> ret(total);
  auto retp = ret.data();
  size_t crnt_pos = 0;
  doc_start.resize(sep_size);
  auto doc_startp = doc_start.data();
  for(size_t i = 0; i < sep_size; i++) {
    doc_startp[i] = crnt_pos;
    auto crnt_retp = retp + crnt_pos;
    auto crnt_v = vp + sepp[i];
    auto len = lenp[i];
    for(size_t j = 0; j < len; j++) {
      crnt_retp[j] = crnt_v[j];
    }
    crnt_retp[len] = del;
    crnt_pos += (len + 1);
  }
  return ret;
}

void split_to_words(const int* vp, size_t size, size_t offset,
                    vector<size_t>& starts,
                    vector<size_t>& lens,
                    const string& delims) {
  if(size == 0) {
    starts.resize(0);
    lens.resize(0);
    return;
  }
  auto delims_size = delims.size();
  if(delims_size > 1) {
    vector<vector<size_t>> seps(delims_size);
    for(size_t i = 0; i < delims_size; i++) {
      seps[i] = find(vp, size, delims.substr(i,1));
    }
    vector<size_t> merged;
    set_multimerge(seps, merged);
    auto merged_size = merged.size();
    starts.resize(merged_size+1);
    auto startsp = starts.data();
    auto crnt_startsp = startsp + 1; // 1st element is set to 0
    auto mergedp = merged.data();
    for(size_t i = 0; i < merged_size; i++) crnt_startsp[i] = mergedp[i];
    int lastdelim = false;
    int lastchar = vp[size-1];
    for(size_t i = 0; i < delims_size; i++) {
      if(lastchar == static_cast<int>(delims[i])) {
        lastdelim = true;
        break;
      }
    }
    if(lastdelim) starts.resize(starts.size()-1);
    auto num_words = starts.size();
    startsp[0] = offset;
    for(size_t i = 0; i < num_words-1; i++) {
      startsp[i+1] += (1 + offset);
    }
    lens.resize(num_words);
    auto lensp = lens.data();
    for(size_t i = 0; i < num_words-1; i++) {
      lensp[i] = startsp[i+1] - startsp[i] - 1;
    }
    if(lastdelim) {
      lensp[num_words-1] = size + offset - startsp[num_words-1] - 1;
    } else {
      lensp[num_words-1] = size + offset - startsp[num_words-1];
    }    
  } else if(delims_size == 1) {
    auto sep = find(vp, size, delims);
    auto sep_size = sep.size();
    int lastdelim = (vp[size-1] == static_cast<int>(delims[0]));
    auto num_words = lastdelim ? sep_size : sep_size + 1;
    starts.resize(num_words);
    auto startsp = starts.data();
    auto sepp = sep.data();
    startsp[0] = offset;
    for(size_t i = 0; i < num_words-1; i++) {
      startsp[i+1] = sepp[i] + 1 + offset;
    }
    lens.resize(num_words);
    auto lensp = lens.data();
    for(size_t i = 0; i < num_words-1; i++) {
      lensp[i] = startsp[i+1] - startsp[i] - 1;
    }
    if(lastdelim) {
      lensp[num_words-1] = size + offset - startsp[num_words-1] - 1;
    } else {
      lensp[num_words-1] = size + offset - startsp[num_words-1];
    }
  } else {
    starts.resize(0);
    lens.resize(0);
  }
}

void split_to_words(const vector<int>& v,
                    vector<size_t>& starts,
                    vector<size_t>& lens,
                    const string& delims) {
  split_to_words(v.data(), v.size(), 0, starts, lens, delims);
}

void print_words(const int* vp,
                 size_t* starts, size_t* lens, size_t num_words) {
  for(size_t i = 0; i < num_words; i++) {
    auto start = starts[i];
    auto len = lens[i];
    for(size_t j = 0; j < len; j++) {
      cout << static_cast<char>(vp[start+j]);
    }
    cout << " ";
  }
  cout << endl;
}

void print_words(const vector<int>& v,
                 vector<size_t>& starts,
                 vector<size_t>& lens) {
  if(starts.size() != lens.size())
    throw runtime_error("size of starts and lens are diffent");
  print_words(v.data(), starts.data(), lens.data(), starts.size());
}


void trim_head(const int* vp, 
               size_t* starts, size_t* lens, size_t num_words,
               const string& to_trim) {
  size_t size = num_words;
  vector<size_t> crnt(num_words), work(num_words);
  auto crntp = crnt.data();
  auto workp = work.data();
  auto to_trim_size = to_trim.size();
  vector<int> to_trim_int(to_trim_size);
  for(size_t i = 0; i < to_trim_size; i++) {
    to_trim_int[i] = static_cast<int>(to_trim[i]);
  }
  auto to_trim_intp = to_trim_int.data();
  for(size_t i = 0; i < num_words; i++) crnt[i] = i;
  while(size > 0) {
    auto each = size / WORDS_VLEN;
    if(each % 2 == 0 && each > 1) each--;
    // TODO: specialize each == 0 case would improve performance...
    size_t rest = size - each * WORDS_VLEN;
    size_t out_ridx[WORDS_VLEN];
#pragma _NEC vreg(out_ridx)
    for(size_t i = 0; i < WORDS_VLEN; i++) {
      out_ridx[i] = each * i;
    }
    for(size_t j = 0; j < each; j++) {
      int trim_flag_ridx[WORDS_VLEN];
#pragma _NEC vreg(trim_flag_ridx)
      for(size_t i = 0; i < WORDS_VLEN; i++) trim_flag_ridx[i] = 0;
      size_t pos_ridx[WORDS_VLEN];
#pragma _NEC vreg(pos_ridx)
      for(size_t i = 0; i < WORDS_VLEN; i++) pos_ridx[i] = crntp[j + each * i];
      for(size_t k = 0; k < to_trim_size; k++) {
#pragma _NEC ivdep
        for(size_t i = 0; i < WORDS_VLEN; i++) {
          if(lens[pos_ridx[i]] > 0 &&
             vp[starts[pos_ridx[i]]] == to_trim_intp[k]) {
            trim_flag_ridx[i] = 1;
          }
        }
      }
#pragma _NEC ivdep
      for(size_t i = 0; i < WORDS_VLEN; i++) {
        if(trim_flag_ridx[i]) {
          starts[pos_ridx[i]]++;
          lens[pos_ridx[i]]--;
          workp[out_ridx[i]++] = pos_ridx[i];
        }
      }
    }
    size_t rest_idx_start = each * WORDS_VLEN;
    size_t rest_idx = rest_idx_start;
    for(size_t j = 0; j < rest; j++) {
      int trim_flag = 0;
      size_t pos = crntp[j + rest_idx_start];
      for(size_t k = 0; k < to_trim_size; k++) {
        if(lens[pos] > 0 && vp[starts[pos]] == to_trim_intp[k]) {
          trim_flag = 1;
        }
      }
      if(trim_flag) {
        starts[pos]++;
        lens[pos]--;
        workp[rest_idx++] = pos;
      }
    }
    size_t sizes[WORDS_VLEN];
    for(size_t i = 0; i < WORDS_VLEN; i++) {
      sizes[i] = out_ridx[i] - each * i;
    }
    size_t total = 0;
    for(size_t i = 0; i < WORDS_VLEN; i++) {
      total += sizes[i];
    }
    size_t rest_size = rest_idx - rest_idx_start;
    total += rest_size;
    size = total;
    size_t current = 0;
    for(size_t i = 0; i < WORDS_VLEN; i++) {
      for(size_t j = 0; j < sizes[i]; j++) {
        crntp[current + j] = workp[each * i + j];
      }
      current += sizes[i];
    }
    for(size_t j = 0; j < rest_size; j++) {
      crntp[current + j] = workp[rest_idx_start + j];
    }
  }
}
               
void trim_head(const vector<int>& v,
               vector<size_t>& starts,
               vector<size_t>& lens,
               const string& to_trim) {
  if(starts.size() != lens.size())
    throw runtime_error("size of starts and lens are diffent");
  trim_head(v.data(), starts.data(), lens.data(), starts.size(), to_trim);
}


void trim_tail(const int* vp, 
               size_t* starts, size_t* lens, size_t num_words,
               const string& to_trim) {
  size_t size = num_words;
  vector<size_t> crnt(num_words), work(num_words);
  auto crntp = crnt.data();
  auto workp = work.data();
  auto to_trim_size = to_trim.size();
  vector<int> to_trim_int(to_trim_size);
  for(size_t i = 0; i < to_trim_size; i++) {
    to_trim_int[i] = static_cast<int>(to_trim[i]);
  }
  auto to_trim_intp = to_trim_int.data();
  for(size_t i = 0; i < num_words; i++) crnt[i] = i;
  while(size > 0) {
    auto each = size / WORDS_VLEN;
    if(each % 2 == 0 && each > 1) each--;
    // TODO: specialize each == 0 case would improve performance...
    size_t rest = size - each * WORDS_VLEN;
    size_t out_ridx[WORDS_VLEN];
#pragma _NEC vreg(out_ridx)
    for(size_t i = 0; i < WORDS_VLEN; i++) {
      out_ridx[i] = each * i;
    }
    for(size_t j = 0; j < each; j++) {
      int trim_flag_ridx[WORDS_VLEN];
#pragma _NEC vreg(trim_flag_ridx)
      for(size_t i = 0; i < WORDS_VLEN; i++) trim_flag_ridx[i] = 0;
      size_t pos_ridx[WORDS_VLEN];
#pragma _NEC vreg(pos_ridx)
      for(size_t i = 0; i < WORDS_VLEN; i++) pos_ridx[i] = crntp[j + each * i];
      for(size_t k = 0; k < to_trim_size; k++) {
#pragma _NEC ivdep
        for(size_t i = 0; i < WORDS_VLEN; i++) {
          if(lens[pos_ridx[i]] > 0 &&
             vp[starts[pos_ridx[i]] + lens[pos_ridx[i]]-1] == to_trim_intp[k]) {
            trim_flag_ridx[i] = 1;
          }
        }
      }
#pragma _NEC ivdep
      for(size_t i = 0; i < WORDS_VLEN; i++) {
        if(trim_flag_ridx[i]) {
          lens[pos_ridx[i]]--;
          workp[out_ridx[i]++] = pos_ridx[i];
        }
      }
    }
    size_t rest_idx_start = each * WORDS_VLEN;
    size_t rest_idx = rest_idx_start;
    for(size_t j = 0; j < rest; j++) {
      int trim_flag = 0;
      size_t pos = crntp[j + rest_idx_start];
      for(size_t k = 0; k < to_trim_size; k++) {
        if(lens[pos] > 0 && vp[starts[pos]+lens[pos]-1] == to_trim_intp[k]) {
          trim_flag = 1;
        }
      }
      if(trim_flag) {
          lens[pos]--;
          workp[rest_idx++] = pos;
      }
    }
    size_t sizes[WORDS_VLEN];
    for(size_t i = 0; i < WORDS_VLEN; i++) {
      sizes[i] = out_ridx[i] - each * i;
    }
    size_t total = 0;
    for(size_t i = 0; i < WORDS_VLEN; i++) {
      total += sizes[i];
    }
    size_t rest_size = rest_idx - rest_idx_start;
    total += rest_size;
    size = total;
    size_t current = 0;
    for(size_t i = 0; i < WORDS_VLEN; i++) {
      for(size_t j = 0; j < sizes[i]; j++) {
        crntp[current + j] = workp[each * i + j];
      }
      current += sizes[i];
    }
    for(size_t j = 0; j < rest_size; j++) {
      crntp[current + j] = workp[rest_idx_start + j];
    }
  }
}
               
void trim_tail(const vector<int>& v,
               vector<size_t>& starts,
               vector<size_t>& lens,
               const string& to_trim) {
  if(starts.size() != lens.size())
    throw runtime_error("size of starts and lens are diffent");
  trim_tail(v.data(), starts.data(), lens.data(), starts.size(), to_trim);
}

void trim(const int* vp,
          size_t* starts, size_t* lens, size_t num_words,
          const string& to_trim) {
  trim_head(vp, starts, lens, num_words, to_trim);
  trim_tail(vp, starts, lens, num_words, to_trim);
}
               
void trim(const vector<int>& v,
          vector<size_t>& starts,
          vector<size_t>& lens,
          const string& to_trim) {
  if(starts.size() != lens.size())
    throw runtime_error("size of starts and lens are diffent");
  trim(v.data(), starts.data(), lens.data(), starts.size(), to_trim);
}

void trim_noalpha_head(const int* vp, 
                       size_t* starts, size_t* lens, size_t num_words) {
  size_t size = num_words;
  vector<size_t> crnt(num_words), work(num_words);
  auto crntp = crnt.data();
  auto workp = work.data();
  for(size_t i = 0; i < num_words; i++) crnt[i] = i;
  while(size > 0) {
    auto each = size / WORDS_VLEN;
    if(each % 2 == 0 && each > 1) each--;
    // TODO: specialize each == 0 case would improve performance...
    size_t rest = size - each * WORDS_VLEN;
    size_t out_ridx[WORDS_VLEN];
#pragma _NEC vreg(out_ridx)
    for(size_t i = 0; i < WORDS_VLEN; i++) {
      out_ridx[i] = each * i;
    }
    for(size_t j = 0; j < each; j++) {
#pragma _NEC ivdep
      for(size_t i = 0; i < WORDS_VLEN; i++) {
        auto pos = crntp[j + each * i];
        auto loaded =  vp[starts[pos]];
        if(lens[pos] > 0 &&
           (loaded < 'A' || loaded > 'z' || (loaded < 'a' && loaded > 'Z'))) {
          starts[pos]++;
          lens[pos]--;
          workp[out_ridx[i]++] = pos;
        }
      }
    }
    size_t rest_idx_start = each * WORDS_VLEN;
    size_t rest_idx = rest_idx_start;
    for(size_t j = 0; j < rest; j++) {
      size_t pos = crntp[j + rest_idx_start];
      auto loaded = vp[starts[pos]];
      if(lens[pos] > 0 && 
         (loaded < 'A' || loaded > 'z' || (loaded < 'a' && loaded > 'Z'))) {
        starts[pos]++;
        lens[pos]--;
        workp[rest_idx++] = pos;
      }
    }
    size_t sizes[WORDS_VLEN];
    for(size_t i = 0; i < WORDS_VLEN; i++) {
      sizes[i] = out_ridx[i] - each * i;
    }
    size_t total = 0;
    for(size_t i = 0; i < WORDS_VLEN; i++) {
      total += sizes[i];
    }
    size_t rest_size = rest_idx - rest_idx_start;
    total += rest_size;
    size = total;
    size_t current = 0;
    for(size_t i = 0; i < WORDS_VLEN; i++) {
      for(size_t j = 0; j < sizes[i]; j++) {
        crntp[current + j] = workp[each * i + j];
      }
      current += sizes[i];
    }
    for(size_t j = 0; j < rest_size; j++) {
      crntp[current + j] = workp[rest_idx_start + j];
    }
  }
}

void trim_noalpha_head(const vector<int>& v,
                       vector<size_t>& starts,
                       vector<size_t>& lens) {
  if(starts.size() != lens.size())
    throw runtime_error("size of starts and lens are diffent");
  trim_noalpha_head(v.data(), starts.data(), lens.data(), starts.size());
}

void trim_noalpha_tail(const int* vp, 
                       size_t* starts, size_t* lens, size_t num_words) {
  size_t size = num_words;
  vector<size_t> crnt(num_words), work(num_words);
  auto crntp = crnt.data();
  auto workp = work.data();
  for(size_t i = 0; i < num_words; i++) crnt[i] = i;
  while(size > 0) {
    auto each = size / WORDS_VLEN;
    if(each % 2 == 0 && each > 1) each--;
    // TODO: specialize each == 0 case would improve performance...
    size_t rest = size - each * WORDS_VLEN;
    size_t out_ridx[WORDS_VLEN];
#pragma _NEC vreg(out_ridx)
    for(size_t i = 0; i < WORDS_VLEN; i++) {
      out_ridx[i] = each * i;
    }
    for(size_t j = 0; j < each; j++) {
#pragma _NEC ivdep
      for(size_t i = 0; i < WORDS_VLEN; i++) {
        auto pos = crntp[j + each * i];
        auto loaded =  vp[starts[pos] + lens[pos]-1];
        if(lens[pos] > 0 && 
           (loaded < 'A' || loaded > 'z' || (loaded < 'a' && loaded > 'Z'))) {
          lens[pos]--;
          workp[out_ridx[i]++] = pos;
        }
      }
    }
    size_t rest_idx_start = each * WORDS_VLEN;
    size_t rest_idx = rest_idx_start;
    for(size_t j = 0; j < rest; j++) {
      size_t pos = crntp[j + rest_idx_start];
      auto loaded =  vp[starts[pos] + lens[pos]-1];
      if(lens[pos] > 0 && 
         (loaded < 'A' || loaded > 'z' || (loaded < 'a' && loaded > 'Z'))) {
        lens[pos]--;
        workp[rest_idx++] = pos;
      }
    }
    size_t sizes[WORDS_VLEN];
    for(size_t i = 0; i < WORDS_VLEN; i++) {
      sizes[i] = out_ridx[i] - each * i;
    }
    size_t total = 0;
    for(size_t i = 0; i < WORDS_VLEN; i++) {
      total += sizes[i];
    }
    size_t rest_size = rest_idx - rest_idx_start;
    total += rest_size;
    size = total;
    size_t current = 0;
    for(size_t i = 0; i < WORDS_VLEN; i++) {
      for(size_t j = 0; j < sizes[i]; j++) {
        crntp[current + j] = workp[each * i + j];
      }
      current += sizes[i];
    }
    for(size_t j = 0; j < rest_size; j++) {
      crntp[current + j] = workp[rest_idx_start + j];
    }
  }
}
               
void trim_noalpha_tail(const vector<int>& v,
                       vector<size_t>& starts,
                       vector<size_t>& lens) {
  if(starts.size() != lens.size())
    throw runtime_error("size of starts and lens are diffent");
  trim_noalpha_tail(v.data(), starts.data(), lens.data(), starts.size());
}

void trim_noalpha(const int* vp,
                  size_t* starts, size_t* lens, size_t num_words) {
  trim_noalpha_head(vp, starts, lens, num_words);
  trim_noalpha_tail(vp, starts, lens, num_words);
}
               
void trim_noalpha(const vector<int>& v,
                  vector<size_t>& starts,
                  vector<size_t>& lens) {
  if(starts.size() != lens.size())
    throw runtime_error("size of starts and lens are diffent");
  trim_noalpha(v.data(), starts.data(), lens.data(), starts.size());
}

void remove_null(size_t* starts, size_t* lens, size_t& size) {
  if(size == 0) return;
  vector<size_t> starts_buf(size), lens_buf(size);
  auto starts_bufp = starts_buf.data();
  auto lens_bufp = lens_buf.data();
  size_t each = size / WORDS_VLEN; // maybe 0
  if(each % 2 == 0 && each > 1) each--;
  if(each == 0) {
    size_t current = 0;
    for(size_t i = 0; i < size; i++) {
      if(lens[i] != 0) {
        starts_bufp[current] = starts[i];
        lens_bufp[current] = lens[i];
        current++;
      }
    }
    size = current;
    for(size_t i = 0; i < current; i++) {
      starts[i] = starts_buf[i];
      lens[i] = lens_buf[i];
    }
  } else {
    size_t rest = size - each * WORDS_VLEN;
    size_t out_ridx[WORDS_VLEN];
// never remove this vreg! this is needed folowing vovertake
#pragma _NEC vreg(out_ridx)
    for(size_t i = 0; i < WORDS_VLEN; i++) {
      out_ridx[i] = each * i;
    }
#pragma _NEC vob
    for(size_t j = 0; j < each; j++) {
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
      for(size_t i = 0; i < WORDS_VLEN; i++) {
        auto loaded_len = lens[j + each * i];
        if(loaded_len != 0) {
          starts_bufp[out_ridx[i]] = starts[j + each * i];
          lens_bufp[out_ridx[i]] = loaded_len;
          out_ridx[i]++;
        }
      }
    }
    size_t rest_idx_start = each * WORDS_VLEN;
    size_t rest_idx = rest_idx_start;
    for(size_t j = 0; j < rest; j++) {
      auto loaded_len = lens[j + rest_idx_start];
      if(loaded_len != 0) {
        starts_bufp[rest_idx] = starts[j + rest_idx_start];
        lens_bufp[rest_idx] = loaded_len;
        rest_idx++;
      }
    }
    size_t sizes[WORDS_VLEN];
    for(size_t i = 0; i < WORDS_VLEN; i++) {
      sizes[i] = out_ridx[i] - each * i;
    }
    size_t total = 0;
    for(size_t i = 0; i < WORDS_VLEN; i++) {
      total += sizes[i];
    }
    size_t rest_size = rest_idx - rest_idx_start;
    total += rest_size;
    size = total;
    size_t current = 0;
    for(size_t i = 0; i < WORDS_VLEN; i++) {
      for(size_t j = 0; j < sizes[i]; j++) {
        starts[current + j] = starts_bufp[each * i + j];
        lens[current + j] = lens_bufp[each * i + j];
      }
      current += sizes[i];
    }
    for(size_t j = 0; j < rest_size; j++) {
      starts[current + j] = starts_bufp[rest_idx_start + j];
      lens[current + j] = lens_bufp[rest_idx_start + j];
    }
  }
}
               
void remove_null(vector<size_t>& starts,
                 vector<size_t>& lens) {
  if(starts.size() != lens.size())
    throw runtime_error("size of starts and lens are diffent");
  auto num_words = starts.size();
  remove_null(starts.data(), lens.data(), num_words);
  vector<size_t> ret_starts(num_words), ret_lens(num_words);
  auto ret_startsp = ret_starts.data();
  auto ret_lensp = ret_lens.data();
  auto startsp = starts.data();
  auto lensp = lens.data();
  for(size_t i = 0; i < num_words; i++) {
    ret_startsp[i] = startsp[i];
    ret_lensp[i] = lensp[i];
  }
  starts.swap(ret_starts);
  lens.swap(ret_lens);
}

struct words_substr_helper {
  words_substr_helper(){}
  words_substr_helper(size_t c) : c(c) {}
  int operator()(size_t a) const {return a < c;}
  size_t c;
  SERIALIZE(c)
};

void substr(size_t* starts, size_t* lens, size_t num_words,
            size_t pos, size_t num) {
  auto fail = find_condition(lens, num_words, words_substr_helper(pos+num));
  if(fail.size() != 0)
    throw std::runtime_error("substr: pos + num is larger than length at: " +
                             std::to_string(fail[0]));
  for(size_t i = 0; i < num_words; i++) {
    starts[i] += pos;
    lens[i] = num;
  }
}

void substr(size_t* starts, size_t* lens, size_t num_words,
            size_t pos) {
  auto fail = find_condition(lens, num_words, words_substr_helper(pos));
  if(fail.size() != 0)
    throw std::runtime_error("substr: pos is larger than length at: " +
                             std::to_string(fail[0]));
  for(size_t i = 0; i < num_words; i++) {
    starts[i] += pos;
    lens[i] -= pos;
  }
}

void substr(std::vector<size_t>& starts,
            std::vector<size_t>& lens,
            size_t pos, size_t num) {
  substr(starts.data(), lens.data(), starts.size(), pos, num);
}
              
void substr(std::vector<size_t>& starts,
            std::vector<size_t>& lens,
            size_t pos) {
  substr(starts.data(), lens.data(), starts.size(), pos);
}


words split_to_words(const std::vector<int>& v, const std::string& delims) {
  words ret;
  split_to_words(v, ret.starts, ret.lens, delims);
  ret.chars = v;
  return ret;
}

words split_to_words(std::vector<int>&& v, const std::string& delims) {
  words ret;
  split_to_words(v, ret.starts, ret.lens, delims);
  ret.chars.swap(v);
  return ret;
}

std::vector<size_t>
convert_position_to_word_count(const std::vector<size_t>& doc_starts,
                               const std::vector<size_t>& word_starts) {
  return lower_bound(word_starts, doc_starts);
}

words merge_words(const words& a, const words& b) {
  words r;
  auto a_chars_size = a.chars.size();
  auto b_chars_size = b.chars.size();
  auto a_starts_size = a.starts.size();
  auto b_starts_size = b.starts.size();
  r.chars.resize(a_chars_size + b_chars_size);
  r.starts.resize(a_starts_size + b_starts_size);
  r.lens.resize(a_starts_size + b_starts_size);
  auto a_charsp = a.chars.data();
  auto b_charsp = b.chars.data();
  auto r_charsp = r.chars.data();
  for(size_t i = 0; i < a_chars_size; i++) {
    r_charsp[i] = a_charsp[i];
  }
  auto crnt_r_charsp = r_charsp + a_chars_size;
  for(size_t i = 0; i < b_chars_size; i++) {
    crnt_r_charsp[i] = b_charsp[i];
  }
  auto a_startsp = a.starts.data();
  auto b_startsp = b.starts.data();
  auto r_startsp = r.starts.data();
  for(size_t i = 0; i < a_starts_size; i++) {
    r_startsp[i] = a_startsp[i];
  }
  auto crnt_r_startsp = r_startsp + a_starts_size;
  for(size_t i = 0; i < b_starts_size; i++) {
    crnt_r_startsp[i] = b_startsp[i] + a_chars_size;
  }
  auto a_lensp = a.lens.data();
  auto b_lensp = b.lens.data();
  auto r_lensp = r.lens.data();
  for(size_t i = 0; i < a_starts_size; i++) {
    r_lensp[i] = a_lensp[i];
  }
  auto crnt_r_lensp = r_lensp + a_starts_size;
  for(size_t i = 0; i < b_starts_size; i++) {
    crnt_r_lensp[i] = b_lensp[i];
  }
  return r;
}

words merge_multi_words(const vector<words>& vecwords) {
  words r;
  auto vecwords_size = vecwords.size();
  size_t total_chars_size = 0;
  size_t total_starts_size = 0;
  for(size_t i = 0; i < vecwords_size; i++) {
    total_chars_size += vecwords[i].chars.size();
    total_starts_size += vecwords[i].starts.size();
  }
  r.chars.resize(total_chars_size);
  r.starts.resize(total_starts_size);
  r.lens.resize(total_starts_size);
  auto r_charsp = r.chars.data();
  auto crnt_r_charsp = r_charsp;
  for(size_t i = 0; i < vecwords_size; i++) {
    auto crnt_charsp = vecwords[i].chars.data();
    auto crnt_chars_size = vecwords[i].chars.size();
    for(size_t j = 0; j < crnt_chars_size; j++) {
      crnt_r_charsp[j] = crnt_charsp[j];
    }
    crnt_r_charsp += crnt_chars_size;
  }
  auto r_startsp = r.starts.data();
  auto crnt_r_startsp = r_startsp;
  size_t crnt_shift = 0;
  for(size_t i = 0; i < vecwords_size; i++) {
    auto crnt_startsp = vecwords[i].starts.data();
    auto crnt_starts_size = vecwords[i].starts.size();
    for(size_t j = 0; j < crnt_starts_size; j++) {
      crnt_r_startsp[j] = crnt_startsp[j] + crnt_shift;
    }
    crnt_r_startsp += crnt_starts_size;
    crnt_shift += vecwords[i].chars.size();
  }
  auto r_lensp = r.lens.data();
  auto crnt_r_lensp = r_lensp;
  for(size_t i = 0; i < vecwords_size; i++) {
    auto crnt_lensp = vecwords[i].lens.data();
    auto crnt_lens_size = vecwords[i].lens.size();
    for(size_t j = 0; j < crnt_lens_size; j++) {
      crnt_r_lensp[j] = crnt_lensp[j];
    }
    crnt_r_lensp += crnt_lens_size;
  }
  return r;
}

// quite similar to concat_docs, but vectorization is differnt
// since words are short
// delimiter is also added to the end of the vector
vector<int> concat_words(const vector<int>& v,
                         const vector<size_t>& starts,
                         const vector<size_t>& lens,
                         const string& delim,
                         vector<size_t>& new_starts) {
  auto starts_size = starts.size();
  if(starts_size == 0) {
    new_starts.resize(0);
    return vector<int>();
  }
  auto vp = v.data();
  auto startsp = starts.data();
  auto lensp = lens.data();
  auto delim_size = delim.size();
  size_t total = 0;
  for(size_t i = 0; i < starts_size; i++) total += lensp[i];
  total += starts_size * delim_size; // for delimiter
  vector<int> ret(total);
  auto retp = ret.data();
  new_starts.resize(starts_size);
  auto new_startsp = new_starts.data();
  auto lens_size = lens.size();
  vector<size_t> lenstmp(lens_size);
  auto lenstmpp = lenstmp.data();
  for(size_t i = 0; i < lens_size; i++) lenstmpp[i] = lensp[i] + delim_size;
  prefix_sum(lenstmpp, new_startsp+1, lens_size-1);
  auto each = starts_size / WORDS_VLEN; // not loop raking
  auto rest = starts_size - each * WORDS_VLEN;
  for(size_t i = 0; i < each; i++) {
    size_t starts_vreg[WORDS_VLEN];
#pragma _NEC vreg(starts_vreg)
    size_t lens_vreg[WORDS_VLEN];
#pragma _NEC vreg(lens_vreg)
    size_t new_starts_vreg[WORDS_VLEN];
#pragma _NEC vreg(new_starts_vreg)
    for(size_t v = 0; v < WORDS_VLEN; v++) {
      starts_vreg[v] = startsp[i * WORDS_VLEN + v];
      lens_vreg[v] = lensp[i * WORDS_VLEN + v];
      new_starts_vreg[v] = new_startsp[i * WORDS_VLEN + v];
    }
    size_t max = 0;
    for(size_t v = 0; v < WORDS_VLEN; v++) {
      if(max < lens_vreg[v]) max = lens_vreg[v];
    }
#pragma _NEC novector
    for(size_t j = 0; j < max; j++) {
#pragma _NEC ivdep
#pragma _NEC vovertake
      for(size_t v = 0; v < WORDS_VLEN; v++) {
        if(lens_vreg[v] > j) {
          retp[new_starts_vreg[v] + j] = vp[starts_vreg[v] + j];
        }
      }
    }
  }
  size_t i = each;
  size_t starts_vreg2[WORDS_VLEN];
  size_t lens_vreg2[WORDS_VLEN];
  size_t new_starts_vreg2[WORDS_VLEN];
  for(size_t v = 0; v < rest; v++) {
    starts_vreg2[v] = startsp[i * WORDS_VLEN + v];
    lens_vreg2[v] = lensp[i * WORDS_VLEN + v];
    new_starts_vreg2[v] = new_startsp[i * WORDS_VLEN + v];
  }
  size_t max = 0;
  for(size_t v = 0; v < rest; v++) {
    if(max < lens_vreg2[v]) max = lens_vreg2[v];
  }
#pragma _NEC novector
  for(size_t j = 0; j < max; j++) {
#pragma _NEC ivdep
#pragma _NEC vovertake
    for(size_t v = 0; v < rest; v++) {
      if(lens_vreg2[v] > j) {
        retp[new_starts_vreg2[v] + j] = vp[starts_vreg2[v] + j];
      }
    }
  }
  for(size_t d = 0; d < delim_size; d++) {
    int crnt_delim = delim[d];
#pragma _NEC ivdep
#pragma _NEC vovertake
    for(size_t i = 1; i < starts_size; i++) {
      retp[new_startsp[i]-delim_size+d] = crnt_delim;
    }
    retp[ret.size()-delim_size+d] = crnt_delim;
  }
  return ret;
}

std::vector<int> concat_words(const words& w, const string& delim,
                              vector<size_t>& new_starts) {
  return concat_words(w.chars, w.starts, w.lens, delim, new_starts);
}

struct is_not_zero_like {
  int operator()(size_t a) const {return !(a == 0);}
};

struct is_zero_like {
  int operator()(size_t a) const {return a == 0;}
};

struct is_char_like {
  is_char_like(int to_comp) : to_comp(to_comp) {}
  int operator()(int a) const {return a == to_comp;}
  int to_comp;
};

void advance_char_like(const vector<int>& chars,
                       vector<size_t>& crnt_pos,
                       vector<size_t>& crnt_lens,
                       vector<size_t>& crnt_idx,
                       int crnt_char) {
  auto non_zero = find_condition(crnt_lens, is_not_zero_like());
  auto non_zero_size = non_zero.size();
  auto crnt_pos_size = crnt_pos.size();
  if(non_zero_size != crnt_pos_size) {
    vector<size_t> next_pos(non_zero_size);
    vector<size_t> next_lens(non_zero_size);
    vector<size_t> next_idx(non_zero_size);
    auto next_posp = next_pos.data();
    auto next_lensp = next_lens.data();
    auto next_idxp = next_idx.data();
    auto crnt_posp = crnt_pos.data();
    auto crnt_lensp = crnt_lens.data();
    auto crnt_idxp = crnt_idx.data();
    auto non_zerop = non_zero.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < non_zero_size; i++) {
      next_posp[i] = crnt_posp[non_zerop[i]];
      next_lensp[i] = crnt_lensp[non_zerop[i]];
      next_idxp[i] = crnt_idxp[non_zerop[i]];
    }
    crnt_pos.swap(next_pos);
    crnt_lens.swap(next_lens);
    crnt_idx.swap(next_idx);
  }
  auto char_found = find_condition_index(chars, crnt_pos,
                                         is_char_like(crnt_char));
  auto char_found_size = char_found.size();
  vector<size_t> next_pos(char_found_size);
  vector<size_t> next_lens(char_found_size);
  vector<size_t> next_idx(char_found_size);
  auto next_posp = next_pos.data();
  auto next_lensp = next_lens.data();
  auto next_idxp = next_idx.data();
  auto crnt_posp = crnt_pos.data();
  auto crnt_lensp = crnt_lens.data();
  auto crnt_idxp = crnt_idx.data();
  auto char_foundp = char_found.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < char_found_size; i++) {
    next_posp[i] = crnt_posp[char_foundp[i]] + 1;
    next_lensp[i] = crnt_lensp[char_foundp[i]] - 1;
    next_idxp[i] = crnt_idxp[char_foundp[i]];
  }
  crnt_pos.swap(next_pos);
  crnt_lens.swap(next_lens);
  crnt_idx.swap(next_idx);
}

// return multiple candidates in one word
// "%ab" need to match "acab"; 1st and 3rd pos need to be returned for "a"
void advance_until_char_like(const vector<int>& chars,
                             const vector<size_t>& starts,
                             const vector<size_t>& lens,
                             vector<size_t>& crnt_pos,
                             vector<size_t>& crnt_lens,
                             vector<size_t>& crnt_idx,
                             int crnt_char) {

  std::vector<std::vector<size_t>> hit_pos, hit_idx;
  hit_pos.reserve(1000);
  hit_idx.reserve(1000);
  while(true) {
    auto non_zero = find_condition(crnt_lens, is_not_zero_like());
    auto non_zero_size = non_zero.size();
    if(non_zero_size == 0) break;
    auto crnt_pos_size = crnt_pos.size();
    if(non_zero_size != crnt_pos_size) {
      vector<size_t> next_pos(non_zero_size);
      vector<size_t> next_lens(non_zero_size);
      vector<size_t> next_idx(non_zero_size);
      auto next_posp = next_pos.data();
      auto next_lensp = next_lens.data();
      auto next_idxp = next_idx.data();
      auto crnt_posp = crnt_pos.data();
      auto crnt_lensp = crnt_lens.data();
      auto crnt_idxp = crnt_idx.data();
      auto non_zerop = non_zero.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
      for(size_t i = 0; i < non_zero_size; i++) {
        next_posp[i] = crnt_posp[non_zerop[i]];
        next_lensp[i] = crnt_lensp[non_zerop[i]];
        next_idxp[i] = crnt_idxp[non_zerop[i]];
      }
      crnt_pos.swap(next_pos);
      crnt_lens.swap(next_lens);
      crnt_idx.swap(next_idx);
    }
    auto hit = find_condition_index(chars, crnt_pos, is_char_like(crnt_char));
    auto hit_size = hit.size();
    if(hit_size > 0) {
      std::vector<size_t> hit_pos_tmp(hit_size);
      std::vector<size_t>  hit_idx_tmp(hit_size);
      auto hit_pos_tmpp = hit_pos_tmp.data();
      auto hit_idx_tmpp = hit_idx_tmp.data();
      auto crnt_posp = crnt_pos.data();
      auto crnt_idxp = crnt_idx.data();
      auto hitp = hit.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
      for(size_t i = 0; i < hit_size; i++) {
        hit_pos_tmpp[i] = crnt_posp[hitp[i]];
        hit_idx_tmpp[i] = crnt_idxp[hitp[i]];
      }
      hit_pos.push_back(hit_pos_tmp);
      hit_idx.push_back(hit_idx_tmp);
    }
    crnt_pos_size = crnt_pos.size();
    auto crnt_posp = crnt_pos.data();
    auto crnt_lensp = crnt_lens.data();
    for(size_t i = 0; i < crnt_pos_size; i++) {
      crnt_posp[i]++;
      crnt_lensp[i]--;
    }
  }
  set_multimerge_pair(hit_idx, hit_pos, crnt_idx, crnt_pos);
  auto crnt_idx_size = crnt_idx.size();
  crnt_lens.resize(crnt_idx_size);
  auto crnt_idxp = crnt_idx.data();
  auto crnt_posp = crnt_pos.data();
  auto crnt_lensp = crnt_lens.data();
  auto startsp = starts.data();
  auto lensp = lens.data();
  for(size_t i = 0; i < crnt_idx_size; i++) {
    crnt_posp[i]++;
  }
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < crnt_idx_size; i++) {
    crnt_lensp[i] =
      lensp[crnt_idxp[i]] - (crnt_posp[i] - startsp[crnt_idxp[i]]);
  }
}

std::vector<size_t> like(const std::vector<int>& chars,
                         const std::vector<size_t>& starts,
                         const std::vector<size_t>& lens,
                         const std::string& to_search,
                         int wild_card,
                         int escape) {
  auto to_search_size = to_search.size();
  if(to_search_size == 0) return find_condition(lens, is_zero_like());
  if(to_search[to_search_size-1] == escape)
    throw runtime_error("like: format error. last char is escape");
  auto starts_size = starts.size();
  auto startsp = starts.data();
  auto lensp = lens.data();
  std::vector<size_t> crnt_pos(starts_size), crnt_lens(starts_size);
  std::vector<size_t> crnt_idx(starts_size);
  auto crnt_posp = crnt_pos.data();
  auto crnt_lensp = crnt_lens.data();
  auto crnt_idxp = crnt_idx.data();
  for(size_t i = 0; i < starts_size; i++) {
    crnt_posp[i] = startsp[i];
    crnt_lensp[i] = lensp[i];
    crnt_idxp[i] = i;
  }
  bool is_in_wildcard = false;
  bool is_in_escape = false;
  for(size_t i = 0; i < to_search_size; i++) {
    int crnt_char = static_cast<int>(to_search[i]);
    if(is_in_escape && is_in_wildcard) {
      advance_until_char_like(chars, starts, lens, crnt_pos, crnt_lens,
                              crnt_idx, crnt_char);
      is_in_wildcard = false;
      is_in_escape = false;
    } else if(is_in_escape && !is_in_wildcard) {
      advance_char_like(chars, crnt_pos, crnt_lens, crnt_idx, crnt_char);
      is_in_escape = false;
    } else if(!is_in_escape && is_in_wildcard) {
      if(crnt_char == escape) {
        is_in_escape = true;
        continue;
      } else if(crnt_char == wild_card) { // does not make sense, though
        continue;
      } else {
        advance_until_char_like(chars, starts, lens, crnt_pos, crnt_lens,
                                crnt_idx, crnt_char);
        is_in_wildcard = false;
      }
    } else if(!is_in_escape && !is_in_wildcard) {
      if(crnt_char == escape) {
        is_in_escape = true;
        continue;
      } else if(crnt_char == wild_card) {
        is_in_wildcard = true;
        continue;
      } else {
        advance_char_like(chars, crnt_pos, crnt_lens, crnt_idx, crnt_char);
      }
    }
  }
  if(is_in_wildcard) {
    return set_unique(crnt_idx);
  } else {
    auto r = find_condition(crnt_lens, is_zero_like());
    auto r_size = r.size();
    std::vector<size_t> ret_idx(r_size);
    auto ret_idxp = ret_idx.data();
    auto rp = r.data();
    auto crnt_idxp = crnt_idx.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < r_size; i++) {
      ret_idxp[i] = crnt_idxp[rp[i]];
    }
    return set_unique(ret_idx);
  }
}

std::vector<size_t> like(const words& w, const std::string& to_search,
                         int wild_card, int escape) {
  return like(w.chars, w.starts, w.lens, to_search, wild_card, escape);
}

struct quote_and_escape_char {
  int operator()(int a) const {return ((a == '\"') || (a == '\\'));}
};

// quote by "..." and escape '"' and '\' by '\' for saving csv file
void quote_and_escape(words& ws) {
  if(ws.starts.size() == 0) return;
  vector<size_t> new_starts;
  // later ' ' is replaced by '"'
  auto new_chars_concat = concat_words(ws, "  ", new_starts);
  auto new_chars_size = new_chars_concat.size();
  vector<int> new_chars(new_chars_size);
  auto new_chars_concatp = new_chars_concat.data();
  auto new_charsp = new_chars.data();
  new_charsp[0] = ' ';
  for(size_t i = 0; i < new_chars_size - 1; i++) {
    new_charsp[i+1] = new_chars_concatp[i];
  }
  {vector<int> tmp; tmp.swap(new_chars_concat);} // clear
  auto lensp = ws.lens.data();
  auto new_startsp = new_starts.data();
  auto starts_size = new_starts.size();
  for(size_t i = 0; i < starts_size; i++) {
    lensp[i] += 2;
  }
  auto to_escape = find_condition(new_chars, quote_and_escape_char());
  auto to_escape_size = to_escape.size();
  vector<int> ret_chars;
  if(to_escape_size != 0) {
    ret_chars.resize(new_chars_size + to_escape_size);
    auto ret_charsp = ret_chars.data();
    auto to_escapep = to_escape.data();
    size_t start = 0;
    for(size_t i = 0; i < to_escape_size; i++) {
      size_t end = to_escapep[i];
      for(size_t j = start; j < end; j++) {
        ret_charsp[i + j] = new_charsp[j];
      }
      ret_charsp[i + end] = '\\';
      start = end;
    }
    for(size_t j = start; j < new_chars_size; j++) {
      ret_charsp[to_escape_size + j] = new_charsp[j];
    }
    auto escape_starts = upper_bound(new_starts, to_escape);
    auto escape_starts_size = escape_starts.size();
    auto escape_startsp = escape_starts.data();
    start = 0;
    for(size_t i = 0; i < escape_starts_size; i++) {
      auto estart = escape_startsp[i];
      /* -1 is safe because to_escape_size != 0
         (last word is at starts_size) */
      lensp[estart-1]++; 
      auto end = estart;
      for(size_t j = start; j < end; j++) {
        new_startsp[j] += i;
      }
      start = end;
    }
    for(size_t j = start; j < starts_size; j++) {
      new_startsp[j] += escape_starts_size;
    }
  } else {
    ret_chars.swap(new_chars);
  }
  auto ret_charsp = ret_chars.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < starts_size; i++) {
    ret_charsp[new_startsp[i]] = '"';
    ret_charsp[new_startsp[i] + lensp[i] - 1] = '"';
  }
  ws.chars.swap(ret_chars);
  ws.starts.swap(new_starts);
}

words vector_string_to_words(const vector<string>& str) {
  words ret;
  auto size = str.size();
  if(size == 0) return ret;
  auto strp = str.data();
  ret.lens.resize(size);
  auto lensp = ret.lens.data();
  for(size_t i = 0; i < size; i++) {
    lensp[i] = strp[i].size();
  }
  ret.starts.resize(size);
  auto startsp = ret.starts.data();
  prefix_sum(lensp, startsp+1, size-1);
  auto total_size = startsp[size-1] + lensp[size-1];
  ret.chars.resize(total_size);
  auto charsp = ret.chars.data();
  if(is_bigendian()) throw runtime_error("big endian is not supported");
  std::vector<uint32_t*> intstrp(size);
  auto intstrpp = intstrp.data();
  for(size_t i = 0; i < size; i++) {
    intstrpp[i] =
      reinterpret_cast<uint32_t*>(const_cast<char*>(strp[i].data()));
  }
  std::vector<size_t> pos(size); // int pos; actual_pos / 4
  auto posp = pos.data();
  for(size_t i = 0; i < size; i++) {
    posp[i] = 0;
  }
  size_t num_block = size / WORDS_VECTOR_BLOCK;
  size_t rest = size - num_block * WORDS_VECTOR_BLOCK;
  for(size_t b = 0; b < num_block; b++) {
    int still_working = true;
    while(still_working) {
      still_working = false;
#pragma _NEC ivdep
      for(size_t i = 0; i < WORDS_VECTOR_BLOCK; i++) {
        auto idx = b * WORDS_VECTOR_BLOCK + i;
        if(posp[idx] * 4 + 3 < lensp[idx]) {
          still_working = true;
          auto packed = *(intstrpp[idx] + posp[idx]);
          charsp[startsp[idx] + posp[idx] * 4] = packed & 0xFF;
          charsp[startsp[idx] + posp[idx] * 4 + 1] = (packed >> 8) & 0xFF;
          charsp[startsp[idx] + posp[idx] * 4 + 2] = (packed >> 16) & 0xFF;
          charsp[startsp[idx] + posp[idx] * 4 + 3] = (packed >> 24) & 0xFF;
          posp[idx]++;
        }
      }
    }
    for(size_t i = 0; i < WORDS_VECTOR_BLOCK; i++) {
      auto idx = b * WORDS_VECTOR_BLOCK + i;
      auto pos4 = posp[idx] * 4;
      auto crnt_strp = strp[idx].data();
      auto lensp_idx = lensp[idx];
      for(size_t j = pos4; j < lensp_idx; j++) {
        charsp[startsp[idx] + j] = crnt_strp[j];
      }
    }
  }
  int still_working = true;
  while(still_working) {
    still_working = false;
#pragma _NEC ivdep
    for(size_t i = 0; i < rest; i++) {
      auto idx = num_block * WORDS_VECTOR_BLOCK + i;
      if(posp[idx] * 4 + 3 < lensp[idx]) {
        still_working = true;
        auto packed = *(intstrpp[idx] + posp[idx]);
        charsp[startsp[idx] + posp[idx] * 4] = packed & 0xFF;
        charsp[startsp[idx] + posp[idx] * 4 + 1] = (packed >> 8)& 0xFF;
        charsp[startsp[idx] + posp[idx] * 4 + 2] = (packed >> 16) & 0xFF;
        charsp[startsp[idx] + posp[idx] * 4 + 3] = (packed >> 24) & 0xFF;
        posp[idx]++;
      }
    }
  }
  for(size_t i = 0; i < rest; i++) {
    auto idx = num_block * WORDS_VECTOR_BLOCK + i;
    auto pos4 = posp[idx] * 4;
    auto crnt_strp = strp[idx].data();
    auto lensp_idx = lensp[idx];
    for(size_t j = pos4; j < lensp_idx; j++) {
      charsp[startsp[idx] + j] = crnt_strp[j];
    }
  }
  return ret;
}

vector<string> words_to_vector_string(const words& ws) {
  auto size = ws.starts.size();
  if(size == 0) return vector<string>();
  vector<string> str(size);
  auto strp = str.data();
  auto lensp = ws.lens.data();
  for(size_t i = 0; i < size; i++) {
    strp[i].resize(lensp[i]);
  }
  auto startsp = ws.starts.data();
  auto charsp = ws.chars.data();
  if(is_bigendian()) throw runtime_error("big endian is not supported");
  std::vector<uint32_t*> intstrp(size);
  auto intstrpp = intstrp.data();
  for(size_t i = 0; i < size; i++) {
    intstrpp[i] =
      reinterpret_cast<uint32_t*>(const_cast<char*>(strp[i].data()));
  }
  std::vector<size_t> pos(size); // int pos; actual_pos / 4
  auto posp = pos.data();
  for(size_t i = 0; i < size; i++) {
    posp[i] = 0;
  }
  size_t num_block = size / WORDS_VECTOR_BLOCK;
  size_t rest = size - num_block * WORDS_VECTOR_BLOCK;
  for(size_t b = 0; b < num_block; b++) {
    int still_working = true;
    while(still_working) {
      still_working = false;
#pragma _NEC ivdep
      for(size_t i = 0; i < WORDS_VECTOR_BLOCK; i++) {
        auto idx = b * WORDS_VECTOR_BLOCK + i;
        if(posp[idx] * 4 + 3 < lensp[idx]) {
          still_working = true;
          *(intstrpp[idx] + posp[idx]) = 
            charsp[startsp[idx] + posp[idx] * 4] + 
            (charsp[startsp[idx] + posp[idx] * 4 + 1] << 8) + 
            (charsp[startsp[idx] + posp[idx] * 4 + 2] << 16) + 
            (charsp[startsp[idx] + posp[idx] * 4 + 3] << 24);
          posp[idx]++;
        }
      }
    }
    for(size_t i = 0; i < WORDS_VECTOR_BLOCK; i++) {
      auto idx = b * WORDS_VECTOR_BLOCK + i;
      auto pos4 = posp[idx] * 4;
      auto crnt_strp = const_cast<char*>(strp[idx].data());
      auto lensp_idx = lensp[idx];
      for(size_t j = pos4; j < lensp_idx; j++) {
        crnt_strp[j] = charsp[startsp[idx] + j];
      }
    }
  }
  int still_working = true;
  while(still_working) {
    still_working = false;
#pragma _NEC ivdep
    for(size_t i = 0; i < rest; i++) {
      auto idx = num_block * WORDS_VECTOR_BLOCK + i;
      if(posp[idx] * 4 + 3 < lensp[idx]) {
        still_working = true;
        *(intstrpp[idx] + posp[idx]) = 
          charsp[startsp[idx] + posp[idx] * 4] + 
          (charsp[startsp[idx] + posp[idx] * 4 + 1] << 8) + 
          (charsp[startsp[idx] + posp[idx] * 4 + 2] << 16) + 
          (charsp[startsp[idx] + posp[idx] * 4 + 3] << 24);
        posp[idx]++;
      }
    }
  }
  for(size_t i = 0; i < rest; i++) {
    auto idx = num_block * WORDS_VECTOR_BLOCK + i;
    auto pos4 = posp[idx] * 4;
    auto crnt_strp = const_cast<char*>(strp[idx].data());
    auto lensp_idx = lensp[idx];
    for(size_t j = pos4; j < lensp_idx; j++) {
      crnt_strp[j] = charsp[startsp[idx] + j];
    }
  }
  return str;
}

void search(const std::vector<int>& chars,
            const std::vector<size_t>& starts,
            const std::vector<size_t>& lens,
            const std::string& to_search,
            std::vector<size_t>& idx,
            std::vector<size_t>& pos) {
  auto to_search_size = to_search.size();
  if(to_search_size == 0) {
    idx.resize(0);
    pos.resize(0);
    return;
  }
  auto starts_size = starts.size();
  auto startsp = starts.data();
  auto lensp = lens.data();
  std::vector<size_t> crnt_pos(starts_size), crnt_lens(starts_size);
  std::vector<size_t> crnt_idx(starts_size);
  auto crnt_posp = crnt_pos.data();
  auto crnt_lensp = crnt_lens.data();
  auto crnt_idxp = crnt_idx.data();
  for(size_t i = 0; i < starts_size; i++) {
    crnt_posp[i] = startsp[i];
    crnt_lensp[i] = lensp[i];
    crnt_idxp[i] = i;
  }
  // size of to_search is guarantted not to be zero
  int crnt_char = static_cast<int>(to_search[0]);
  advance_until_char_like(chars, starts, lens, crnt_pos, crnt_lens,
                          crnt_idx, crnt_char);
  for(size_t i = 1; i < to_search_size; i++) {
    crnt_char = static_cast<int>(to_search[i]);
    advance_char_like(chars, crnt_pos, crnt_lens, crnt_idx, crnt_char);
  }
  crnt_posp = crnt_pos.data();
  auto crnt_posp_size = crnt_pos.size();
  crnt_idxp = crnt_idx.data();
  for(size_t i = 0; i < crnt_posp_size; i++) {
    crnt_posp[i] -= (to_search_size + startsp[crnt_idxp[i]]);
  }
  idx.swap(crnt_idx);
  pos.swap(crnt_pos);
}

void search(const words& w, const std::string& to_search,
            std::vector<size_t>& idx, std::vector<size_t>& pos) {
  search(w.chars, w.starts, w.lens, to_search, idx, pos);
}

void replace(const std::vector<int>& chars,
             const std::vector<size_t>& starts,
             const std::vector<size_t>& lens,
             std::vector<int>& ret_chars,
             std::vector<size_t>& ret_starts,
             std::vector<size_t>& ret_lens,
             const std::string& from,
             const std::string& to) {
  std::vector<size_t> idx, pos;
  search(chars, starts, lens, from, idx, pos);

  auto sep_idx = set_separate(idx);
  auto sep_idx_size = sep_idx.size();
  std::vector<size_t> num_occurrence(sep_idx_size-1);
  auto num_occurrence_size = num_occurrence.size();
  auto sep_idxp = sep_idx.data();
  auto num_occurrencep = num_occurrence.data();
  for(size_t i = 0; i < num_occurrence_size; i++) {
    num_occurrencep[i] = sep_idxp[i+1] - sep_idxp[i];
  }
  auto starts_size = starts.size();
  std::vector<size_t> all_num_occurrence(starts_size);
  auto all_num_occurrencep = all_num_occurrence.data();
  auto idxp = idx.data();
#pragma _NEC ivdep
  for(size_t i = 0; i < num_occurrence_size; i++) {
    all_num_occurrencep[idxp[sep_idxp[i]]] = num_occurrencep[i];
  }
  auto pfx_all_num_occurrence = prefix_sum(all_num_occurrence);
  auto total_occurrence = pfx_all_num_occurrence[starts_size-1];
  std::vector<size_t> starts_work(starts_size + total_occurrence * 2);
  auto starts_work_size = starts_work.size();
  std::vector<size_t> lens_work(starts_work_size);
  std::vector<size_t> original_starts_idx(starts_size);
  auto original_starts_idxp = original_starts_idx.data();
  auto pfx_all_num_occurrencep = pfx_all_num_occurrence.data();
  for(size_t i = 1; i < starts_size; i++) {
    original_starts_idxp[i] = i + pfx_all_num_occurrencep[i-1] * 2;
  }
  auto starts_workp = starts_work.data();
  auto lens_workp = lens_work.data();
  auto startsp = starts.data();
  auto lensp = lens.data();
#pragma _NEC ivdep
  for(size_t i = 0; i < starts_size; i++) {
    starts_workp[original_starts_idxp[i]] = startsp[i];
    lens_workp[original_starts_idxp[i]] = lensp[i];
  }

  auto chars_size = chars.size();
  auto to_int = char_to_int(to);
  auto to_int_size = to_int.size();
  std::vector<int> chars_work(chars_size + to_int_size);
  auto chars_workp = chars_work.data();
  auto charsp = chars.data();
  for(size_t i = 0; i < chars_size; i++) {
    chars_workp[i] = charsp[i];
  }
  auto to_intp = to_int.data();
  for(size_t i = 0; i < to_int_size; i++) {
    chars_workp[chars_size + i] = to_intp[i];
  }
  auto to_start = chars_size;
  auto to_size = to_int_size;
  auto from_size = from.size();

  std::vector<size_t> search_idx(sep_idx_size - 1);
  auto search_idx_size = search_idx.size();
  auto search_idxp = search_idx.data();
  for(size_t i = 0; i < search_idx_size; i++) {
    search_idxp[i] = sep_idxp[i];
  }
  std::vector<size_t> original_num_occurrence(num_occurrence_size);
  auto original_num_occurrencep = original_num_occurrence.data();
  for(size_t i = 0; i < num_occurrence_size; i++) {
    original_num_occurrencep[i] = num_occurrencep[i];
  }
  // separate 1st iteration: num_occurence is always greater than 0
  auto posp = pos.data();
#pragma _NEC ivdep  
  for(size_t i = 0; i < search_idx_size; i++) {
    auto search_pos = search_idxp[i];
    auto crnt_word = idxp[search_pos];
    auto crnt_pos = posp[search_pos];
    auto work_idx = original_starts_idxp[crnt_word];
    lens_workp[work_idx] = crnt_pos;
    starts_workp[work_idx+1] = to_start;
    lens_workp[work_idx+1] = to_size;
    starts_workp[work_idx+2] = startsp[crnt_word] + crnt_pos + from_size;
  }
  for(size_t i = 0; i < num_occurrence_size; i++) {num_occurrencep[i]--;}
  auto non_zero = find_condition(num_occurrence, is_not_zero_like());
  auto zero = find_condition(num_occurrence, is_zero_like());
  auto zero_size = zero.size();
  auto zerop = zero.data();
  for(size_t i = 0; i < zero_size; i++) {
    auto search_pos = search_idxp[zerop[i]];
    auto crnt_word = idxp[search_pos];
    auto crnt_pos = posp[search_pos];
    auto work_idx = original_starts_idxp[crnt_word];
    lens_workp[work_idx+2] = lensp[crnt_word] - crnt_pos - from_size;
  }
  {
    auto non_zero_size = non_zero.size();
    std::vector<size_t> new_num_occurrence(non_zero_size);
    std::vector<size_t> new_search_idx(non_zero_size);
    auto new_num_occurrencep = new_num_occurrence.data();
    auto new_search_idxp = new_search_idx.data();
    auto non_zerop = non_zero.data();
#pragma _NEC ivdep
    for(size_t i = 0; i < non_zero_size; i++) {
      new_num_occurrencep[i] = num_occurrencep[non_zerop[i]];
      new_search_idxp[i] = search_idxp[non_zerop[i]] + 1;
    }
    num_occurrence.swap(new_num_occurrence);
    search_idx.swap(new_search_idx);
  }
  size_t iter = 1;
  while(true) {
    search_idx_size = search_idx.size();
    search_idxp = search_idx.data();
    num_occurrence_size = num_occurrence.size();
    num_occurrencep = num_occurrence.data();
    if(search_idx_size == 0) break;
#pragma _NEC ivdep  
    for(size_t i = 0; i < search_idx_size; i++) {
      auto search_pos = search_idxp[i];
      auto crnt_word = idxp[search_pos];
      auto crnt_pos = posp[search_pos];
      auto work_idx = original_starts_idxp[crnt_word] + iter * 2;
      lens_workp[work_idx] = crnt_pos - posp[search_pos-1] - from_size;
      starts_workp[work_idx+1] = to_start;
      lens_workp[work_idx+1] = to_size;
      starts_workp[work_idx+2] = startsp[crnt_word] + crnt_pos + from_size;
    }
    for(size_t i = 0; i < num_occurrence_size; i++) {num_occurrencep[i]--;}
    auto non_zero = find_condition(num_occurrence, is_not_zero_like());
    auto zero = find_condition(num_occurrence, is_zero_like());
    auto zero_size = zero.size();
    auto zerop = zero.data();
#pragma _NEC ivdep  
    for(size_t i = 0; i < zero_size; i++) {
      auto search_pos = search_idxp[zerop[i]];
      auto crnt_word = idxp[search_pos];
      auto crnt_pos = posp[search_pos];
      auto work_idx = original_starts_idxp[crnt_word] + iter * 2;
      lens_workp[work_idx+2] = lensp[crnt_word] - crnt_pos - from_size;
    }
    {
      auto non_zero_size = non_zero.size();
      std::vector<size_t> new_num_occurrence(non_zero_size);
      std::vector<size_t> new_search_idx(non_zero_size);
      auto new_num_occurrencep = new_num_occurrence.data();
      auto new_search_idxp = new_search_idx.data();
      auto non_zerop = non_zero.data();
#pragma _NEC ivdep
      for(size_t i = 0; i < non_zero_size; i++) {
        new_num_occurrencep[i] = num_occurrencep[non_zerop[i]];
        new_search_idxp[i] = search_idxp[non_zerop[i]] + 1;
      }
      num_occurrence.swap(new_num_occurrence);
      search_idx.swap(new_search_idx);
    }
    iter++;
  }
  std::vector<size_t> new_starts;
  ret_chars = concat_words(chars_work, starts_work, lens_work, "", new_starts);
  ret_starts.resize(starts_size);
  auto ret_startsp = ret_starts.data();
  auto new_startsp = new_starts.data();
#pragma _NEC ivdep  
  for(size_t i = 0; i < starts_size; i++) {
    ret_startsp[i] = new_startsp[original_starts_idxp[i]];
  }
  ret_lens.resize(starts_size);
  auto ret_lensp = ret_lens.data();
  for(size_t i = 0; i < starts_size; i++) {
    ret_lensp[i] = lensp[i];
  }
  // might be negative
  ssize_t size_diff = (ssize_t)to_size - (ssize_t)from_size; 
#pragma _NEC ivdep
  for(size_t i = 0; i < sep_idx_size-1; i++) {
    ret_lensp[idxp[sep_idxp[i]]] += size_diff * original_num_occurrencep[i];
  }
}

words replace(const words& w, const std::string& from, const std::string& to) {
  words ret;
  replace(w.chars, w.starts, w.lens, ret.chars, ret.starts, ret.lens, from, to);
  return ret;
}

}
