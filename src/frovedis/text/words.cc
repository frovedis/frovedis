#include "words.hpp"
#include "find.hpp"
#include "../core/radix_sort.hpp"
#include "../core/set_operations.hpp"
#include "../core/lower_bound.hpp"
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
  auto crnt_pos = 0;
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

}
