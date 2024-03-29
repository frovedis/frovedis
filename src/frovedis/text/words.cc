#include "words.hpp"
#include "find.hpp"
#include "../core/radix_sort.hpp"
#include "../core/set_operations.hpp"
#include "../core/lower_bound.hpp"
#include "../core/upper_bound.hpp"
#include "../core/prefix_sum.hpp"
#include "../core/vector_operations.hpp"
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
    to_trim_int[i] = static_cast<unsigned char>(to_trim[i]);
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
    to_trim_int[i] = static_cast<unsigned char>(to_trim[i]);
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

// index is 0-based, while dataframe functions are 1-based
void substr(size_t* starts, size_t* lens, size_t num_words,
            int pos, int num) {
  if(num < 0) { // invalid
    for(size_t i = 0; i < num_words; i++) lens[i] = 0;
    return;
  }
  if(pos >= 0) {
    for(size_t i = 0; i < num_words; i++) {
      auto newstarts = starts[i] + pos;
      auto newend = newstarts + num;
      auto end = starts[i] + lens[i];
      if(newstarts > end) newstarts = end;
      if(newend > end) newend = end;
      starts[i] = newstarts;
      lens[i] = newend - newstarts;
    }
  } else {
    for(size_t i = 0; i < num_words; i++) {
      auto end = starts[i] + lens[i];
      auto newstarts = ssize_t(end) + pos;
      auto newend = newstarts + num;
      if(newstarts < ssize_t(starts[i])) newstarts = starts[i];
      if(newend < starts[i]) newend = starts[i];
      else if(newend > end) newend = end;
      starts[i] = newstarts;
      lens[i] = newend - newstarts;
    }
  }
}

void substr(size_t* starts, size_t* lens, size_t num_words,
            const int* pos, int num) {
  if(num < 0) {
    for(size_t i = 0; i < num_words; i++) lens[i] = 0;
    return;
  }
  for(size_t i = 0; i < num_words; i++) {
    auto end = starts[i] + lens[i];
    ssize_t newstarts, newend;
    if(pos[i] >= 0) {
      newstarts = starts[i] + pos[i];
      newend = newstarts + num;
      if(newstarts > end) newstarts = end;
      if(newend > end) newend = end;
    } else {
      newstarts = ssize_t(end) + pos[i];
      newend = newstarts + num;
      if(newstarts < ssize_t(starts[i])) newstarts = starts[i];
      if(newend < starts[i]) newend = starts[i];
      else if(newend > end) newend = end;
    }
    starts[i] = newstarts;
    lens[i] = newend - newstarts;
  }
}

void substr(size_t* starts, size_t* lens, size_t num_words,
            int pos, const int* num) {
  auto failnum = find_condition(num, num_words, is_lt<int>(0));
  auto failnump = failnum.data();
  auto failnum_size = failnum.size();
  if(pos >= 0) {
    for(size_t i = 0; i < num_words; i++) {
      auto newstarts = starts[i] + pos;
      auto newend = newstarts + num[i];
      auto end = starts[i] + lens[i];
      if(newstarts > end) newstarts = end;
      if(newend > end) newend = end;
      starts[i] = newstarts;
      lens[i] = newend - newstarts;
    }
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < failnum_size; i++) {
      lens[failnump[i]] = 0;
    }
  } else {
    for(size_t i = 0; i < num_words; i++) {
      auto end = starts[i] + lens[i];
      auto newstarts = ssize_t(end) + pos;
      auto newend = newstarts + num[i];
      if(newstarts < ssize_t(starts[i])) newstarts = starts[i];
      if(newend < starts[i]) newend = starts[i];
      else if(newend > end) newend = end;
      starts[i] = newstarts;
      lens[i] = newend - newstarts;
    }
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < failnum_size; i++) {
      lens[failnump[i]] = 0;
    }
  }
}

void substr(size_t* starts, size_t* lens, size_t num_words,
            const int* pos, const int* num) {
  auto fail = find_condition(num, num_words, is_lt<int>(0));
  for(size_t i = 0; i < num_words; i++) {
    auto end = starts[i] + lens[i];
    ssize_t newstarts, newend;
    if(pos[i] >= 0) {
      newstarts = starts[i] + pos[i];
      newend = newstarts + num[i];
      if(newstarts > end) newstarts = end;
      if(newend > end) newend = end;
    } else {
      newstarts = ssize_t(end) + pos[i];
      newend = newstarts + num[i];
      if(newstarts < ssize_t(starts[i])) newstarts = starts[i];
      if(newend < starts[i]) newend = starts[i];
      else if(newend > end) newend = end;
    }
    starts[i] = newstarts;
    lens[i] = newend - newstarts;
  }
  auto failp = fail.data();
  auto fail_size = fail.size();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < fail_size; i++) {
    lens[failp[i]] = 0;
  }
}

void substr(size_t* starts, size_t* lens, size_t num_words,
            int pos) {
  if(pos >= 0) {
    for(size_t i = 0; i < num_words; i++) {
      auto newstarts = starts[i] + pos;
      auto end = starts[i] + lens[i];
      if(newstarts > end) newstarts = end;
      starts[i] = newstarts;
      lens[i] = end - newstarts;
    }
  } else {
    for(size_t i = 0; i < num_words; i++) {
      auto end = starts[i] + lens[i];
      auto newstarts = ssize_t(end) + pos;
      if(newstarts < ssize_t(starts[i])) newstarts = starts[i];
      starts[i] = newstarts;
      lens[i] = end - newstarts;
    }
  }
}

void substr(size_t* starts, size_t* lens, size_t num_words,
            const int* pos) {
  for(size_t i = 0; i < num_words; i++) {
    ssize_t newstarts, end;
    if(pos[i] >= 0) {
      newstarts = starts[i] + pos[i];
      end = starts[i] + lens[i];
      if(newstarts > end) newstarts = end;
    } else {
      end = starts[i] + lens[i];
      newstarts = ssize_t(end) + pos[i];
      if(newstarts < ssize_t(starts[i])) newstarts = starts[i];
    }
    starts[i] = newstarts;
    lens[i] = end - newstarts;
  }
}

void substr(std::vector<size_t>& starts,
            std::vector<size_t>& lens,
            int pos, int num) {
  substr(starts.data(), lens.data(), starts.size(), pos, num);
}

void substr(std::vector<size_t>& starts,
            std::vector<size_t>& lens,
            const std::vector<int>& pos, int num) {
  if(starts.size() != pos.size())
    throw std::runtime_error("substr: size error");
  substr(starts.data(), lens.data(), starts.size(), pos.data(), num);
}

void substr(std::vector<size_t>& starts,
            std::vector<size_t>& lens,
            int pos, const std::vector<int>& num) {
  if(starts.size() != num.size())
    throw std::runtime_error("substr: size error");
  substr(starts.data(), lens.data(), starts.size(), pos, num.data());
}

void substr(std::vector<size_t>& starts,
            std::vector<size_t>& lens,
            const std::vector<int>& pos, const std::vector<int>& num) {
  if(starts.size() != pos.size() || starts.size() != num.size())
    throw std::runtime_error("substr: size error");
  substr(starts.data(), lens.data(), starts.size(), pos.data(), num.data());
}
              
void substr(std::vector<size_t>& starts,
            std::vector<size_t>& lens,
            int pos) {
  substr(starts.data(), lens.data(), starts.size(), pos);
}

void substr(std::vector<size_t>& starts,
            std::vector<size_t>& lens,
            const std::vector<int>& pos) {
  if(starts.size() != pos.size())
    throw std::runtime_error("substr: size error");
  substr(starts.data(), lens.data(), starts.size(), pos.data());
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
    int crnt_delim = static_cast<unsigned char>(delim[d]);
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
    int crnt_char = static_cast<unsigned char>(to_search[i]);
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
        charsp[startsp[idx] + j] = static_cast<unsigned char>(crnt_strp[j]);
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
      charsp[startsp[idx] + j] = static_cast<unsigned char>(crnt_strp[j]);
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
      auto crnt_strp = reinterpret_cast<unsigned char*>
        (const_cast<char*>(strp[idx].data()));
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
    auto crnt_strp = reinterpret_cast<unsigned char*>
      (const_cast<char*>(strp[idx].data()));
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
            const std::vector<int>& to_search,
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
  int crnt_char = to_search[0];
  advance_until_char_like(chars, starts, lens, crnt_pos, crnt_lens,
                          crnt_idx, crnt_char);
  for(size_t i = 1; i < to_search_size; i++) {
    crnt_char = to_search[i];
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

void search(const std::vector<int>& chars,
            const std::vector<size_t>& starts,
            const std::vector<size_t>& lens,
            const string& to_search,
            std::vector<size_t>& idx,
            std::vector<size_t>& pos) {
  std::vector<int> int_to_search(to_search.size());
  for(size_t i = 0; i < to_search.size(); i++) {
    int_to_search[i] = static_cast<unsigned char>(to_search[i]);
  }
  search(chars, starts, lens, int_to_search, idx, pos);
}

void search(const words& w, const std::string& to_search,
            std::vector<size_t>& idx, std::vector<size_t>& pos) {
  search(w.chars, w.starts, w.lens, to_search, idx, pos);
}

void search(const words& w, const std::vector<int>& to_search,
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
  if(starts.size() == 0) {
    ret_chars.resize(0);
    ret_starts.resize(0);
    ret_lens.resize(0);
    return;
  }
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

void translate(const std::vector<int>& chars,
               const std::vector<size_t>& starts,
               const std::vector<size_t>& lens,
               std::vector<int>& ret_chars,
               std::vector<size_t>& ret_starts,
               std::vector<size_t>& ret_lens,
               const std::string& from,
               const std::string& to) {
  ret_lens = lens;
  ret_starts = starts;
  ret_chars = chars;
  auto rptr = ret_chars.data();

  size_t i = 0;
  auto from_len = from.length();
  auto to_len = to.length();
  for(; i < from_len && i < to_len; ++i) {
    int from_char = from[i];
    int to_char = to[i];
    auto pos = vector_find_eq(ret_chars, from_char);
    auto pos_ptr = pos.data();
    auto pos_size = pos.size();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t j = 0; j < pos_size; ++j) rptr[pos_ptr[j]] = to_char;
  }

  // remove remaining characters...
  for(size_t k = i ; k < from_len; ++k) {
    std::string from_str(1, from[k]);
    std::string to_str = "";
    words tmp;
    frovedis::replace(ret_chars, ret_starts, ret_lens, 
                      tmp.chars, tmp.starts, tmp.lens,
                      from_str, to_str);
    ret_chars.swap(tmp.chars);
    ret_starts.swap(tmp.starts);
    ret_lens.swap(tmp.lens);
  }
}

words translate(const words& w, const std::string& from, const std::string& to) {
  words ret;
  translate(w.chars, w.starts, w.lens, ret.chars, ret.starts, ret.lens, from, to);
  return ret;
}

void prepend(const std::vector<int>& chars,
             const std::vector<size_t>& starts,
             const std::vector<size_t>& lens,
             std::vector<int>& ret_chars,
             std::vector<size_t>& ret_starts,
             std::vector<size_t>& ret_lens,
             const std::string& to_prepend) {
  auto to_prepend_size = to_prepend.size();
  auto new_chars = concat_words(chars, starts, lens, to_prepend, ret_starts);
  auto new_chars_size = new_chars.size();
  ret_chars.resize(new_chars_size);
  auto ret_charsp = ret_chars.data();
  auto new_charsp = new_chars.data();
  for(size_t i = 0; i < new_chars_size - to_prepend_size; i++) {
    ret_charsp[i + to_prepend_size] = new_charsp[i];
  }
  for(size_t i = 0; i < to_prepend_size; i++) {
    ret_charsp[i] = new_charsp[new_chars_size - to_prepend_size + i];
  }
  auto lens_size = lens.size();
  ret_lens.resize(lens_size);
  auto ret_lensp = ret_lens.data();
  auto lensp = lens.data();
  for(size_t i = 0; i < lens_size; i++) {
    ret_lensp[i] = lensp[i] + to_prepend_size;
  }
}

words prepend(const words& w, const std::string& to_prepend) {
  words ret;
  prepend(w.chars, w.starts, w.lens, ret.chars, ret.starts, ret.lens, to_prepend);
  return ret;
}

void append(const std::vector<int>& chars,
             const std::vector<size_t>& starts,
             const std::vector<size_t>& lens,
             std::vector<int>& ret_chars,
             std::vector<size_t>& ret_starts,
             std::vector<size_t>& ret_lens,
             const std::string& to_append) {
  auto to_append_size = to_append.size();
  ret_chars = concat_words(chars, starts, lens, to_append, ret_starts);
  auto lens_size = lens.size();
  ret_lens.resize(lens_size);
  auto ret_lensp = ret_lens.data();
  auto lensp = lens.data();
  for(size_t i = 0; i < lens_size; i++) {
    ret_lensp[i] = lensp[i] + to_append_size;
  }
}

words append(const words& w, const std::string& to_append) {
  words ret;
  append(w.chars, w.starts, w.lens, ret.chars, ret.starts, ret.lens, to_append);
  return ret;
}

words horizontal_concat_words(std::vector<words>& vec_words) {
  if(vec_words.size() == 0) return words();
  auto num_words = vec_words[0].starts.size();
  auto vec_words_size = vec_words.size();
  for(size_t i = 1; i < vec_words_size; i++) {
    if(vec_words[i].starts.size() != num_words)
      throw std::runtime_error
        ("horizontal_concat_words: different number of words");
  }
  std::vector<size_t> each_chars_size(vec_words_size);
  auto each_chars_sizep = each_chars_size.data();
  for(size_t i = 0; i < vec_words_size; i++) {
    each_chars_sizep[i] = vec_words[i].chars.size();
  }
  std::vector<size_t> pfx_chars_size(vec_words_size+1); // to make it exclusive
  auto pfx_chars_sizep = pfx_chars_size.data();
  prefix_sum(each_chars_sizep, pfx_chars_sizep+1, vec_words_size);
  size_t concat_chars_size = pfx_chars_size[vec_words_size];

  std::vector<int> concat_chars(concat_chars_size);
  auto concat_charsp = concat_chars.data();
  for(size_t i = 0; i < vec_words_size; i++) {
    auto crnt_charsp = vec_words[i].chars.data();
    auto crnt_chars_size = vec_words[i].chars.size();
    auto crnt_concat_charsp = concat_charsp + pfx_chars_sizep[i];
    for(size_t j = 0; j < crnt_chars_size; j++) {
      crnt_concat_charsp[j] = crnt_charsp[j];
    }
  }

  words ret;
  std::vector<size_t> new_starts;
  {
    words tmp;
    tmp.starts.resize(num_words * vec_words_size);
    tmp.lens.resize(num_words * vec_words_size);
    tmp.chars.swap(concat_chars);
    auto tmp_startsp = tmp.starts.data();
    auto tmp_lensp = tmp.lens.data();
    for(size_t i = 0; i < vec_words_size; i++) {
      auto crnt_startsp = vec_words[i].starts.data();
      auto crnt_lensp = vec_words[i].lens.data();
      auto starts_shift = pfx_chars_sizep[i];
      for(size_t j = 0; j < num_words; j++) {
        tmp_startsp[vec_words_size * j + i] = crnt_startsp[j] + starts_shift;
        tmp_lensp[vec_words_size * j + i] = crnt_lensp[j];
      }
    }
    ret.chars = concat_words(tmp, "", new_starts);
  }
  ret.starts.resize(num_words);
  ret.lens.resize(num_words);
  auto ret_startsp = ret.starts.data();
  auto new_startsp = new_starts.data();
  for(size_t i = 0; i < num_words; i++) {
    ret_startsp[i] = new_startsp[i * vec_words_size];
  }
  auto ret_lensp = ret.lens.data();
  for(size_t i = 0; i < vec_words_size; i++) {
    auto crnt_lensp = vec_words[i].lens.data();
    for(size_t j = 0; j < num_words; j++) {
      ret_lensp[j] += crnt_lensp[j];
    }
  }
  return ret;
}

void reverse(const std::vector<int>& chars,
             const std::vector<size_t>& starts,
             const std::vector<size_t>& lens,
             std::vector<int>& ret_chars,
             std::vector<size_t>& ret_starts) {
  auto chars_size = chars.size();
  ret_chars.resize(chars_size);
  auto charsp = chars.data();
  auto ret_charsp = ret_chars.data();
  for(size_t i = 0; i < chars_size; i++) {
    ret_charsp[i] = charsp[chars_size - 1 - i];
  }
  auto starts_size = starts.size();
  ret_starts.resize(starts_size);
  auto startsp = starts.data();
  auto ret_startsp = ret_starts.data();
  auto lensp = lens.data();
  for(size_t i = 0; i < starts_size; i++) {
    ret_startsp[i] = chars_size - startsp[i] - lensp[i];
  }
}

words reverse(const words& w) {
  words ret;
  reverse(w.chars, w.starts, w.lens, ret.chars, ret.starts);
  ret.lens = w.lens;
  return ret;
}

void tolower(const std::vector<int>& chars,
             std::vector<int>& ret_chars) {
  auto chars_size = chars.size();
  ret_chars.resize(chars_size);
  auto charsp = chars.data();
  auto ret_charsp = ret_chars.data();
  for(size_t i = 0; i < chars_size; i++) {
    if(charsp[i] >= 'A' && charsp[i] <= 'Z')
      ret_charsp[i] = charsp[i] + ('a' - 'A');
    else
      ret_charsp[i] = charsp[i];
  }
}

words tolower(const words& ws) {
  words ret;
  frovedis::tolower(ws.chars, ret.chars);
  ret.starts = ws.starts;
  ret.lens = ws.lens;
  return ret;
}

void toupper(const std::vector<int>& chars,
             std::vector<int>& ret_chars) {
  auto chars_size = chars.size();
  ret_chars.resize(chars_size);
  auto charsp = chars.data();
  auto ret_charsp = ret_chars.data();
  for(size_t i = 0; i < chars_size; i++) {
    if(charsp[i] >= 'a' && charsp[i] <= 'z')
      ret_charsp[i] = charsp[i] - ('a' - 'A');
    else
      ret_charsp[i] = charsp[i];
  }
}

words toupper(const words& ws) {
  words ret;
  frovedis::toupper(ws.chars, ret.chars);
  ret.starts = ws.starts;
  ret.lens = ws.lens;
  return ret;
}

// https://atmarkit.itmedia.co.jp/ait/articles/1603/28/news035.html
// https://qiita.com/benikabocha/items/e943deb299d0f816f161
/*
U+000000 - U+00007F 	0xxxxxxx
U+000080 - U+0007FF 	110xxxxx　10xxxxxx
U+000800 - U+00FFFF 	1110xxxx　10xxxxxx　10xxxxxx
U+010000 - U+10FFFF 	11110xxx　10xxxxxx　10xxxxxx　10xxxxxx
*/
struct is_one_byte_utf8{
  int operator()(int c) const {
    return (c < 0x80);
  }
  SERIALIZE_NONE
};
struct is_two_byte_utf8{
  int operator()(int c) const {
    return (c >= 0xC2 && c < 0xE0);
  }
  SERIALIZE_NONE
};
struct is_three_byte_utf8{
  int operator()(int c) const {
    return (c >= 0xE0 && c < 0xF0);
  }
  SERIALIZE_NONE
};
struct is_four_byte_utf8{
  int operator()(int c) const {
    return (c >= 0xF0 && c < 0xF8);
  }
  SERIALIZE_NONE
};

void utf8_to_utf32(const std::vector<int>& chars,
                   const std::vector<size_t>& starts,
                   const std::vector<size_t>& lens,
                   std::vector<int>& ret_chars,
                   std::vector<size_t>& ret_starts,
                   std::vector<size_t>& ret_lens) {
  auto one_byte_start = find_condition(chars, is_one_byte_utf8());
  auto one_byte_start_size = one_byte_start.size();
  auto chars_size = chars.size();
  if(one_byte_start_size == chars_size) {
    ret_chars = chars;
    ret_starts = starts;
    ret_lens = lens;
    return;
  } else {
    auto two_byte_start = find_condition(chars, is_two_byte_utf8());
    auto three_byte_start = find_condition(chars, is_three_byte_utf8());
    auto four_byte_start = find_condition(chars, is_four_byte_utf8());
    auto two_byte_start_size = two_byte_start.size();
    auto three_byte_start_size = three_byte_start.size();
    auto four_byte_start_size = four_byte_start.size();
    auto one_byte_startp = one_byte_start.data();
    auto two_byte_startp = two_byte_start.data();
    auto three_byte_startp = three_byte_start.data();
    auto four_byte_startp = four_byte_start.data();
    std::vector<size_t> is_start(chars_size);
    auto is_startp = is_start.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < one_byte_start_size; i++) {
      is_startp[one_byte_startp[i]] = 1;
    }
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < two_byte_start_size; i++) {
      is_startp[two_byte_startp[i]] = 1;
    }
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < three_byte_start_size; i++) {
      is_startp[three_byte_startp[i]] = 1;
    }
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < four_byte_start_size; i++) {
      is_startp[four_byte_startp[i]] = 1;
    }
    std::vector<size_t> start_index(chars_size+1);
    auto start_indexp = start_index.data();
    prefix_sum(is_startp, start_indexp+1, chars_size);
    auto ret_chars_size = start_indexp[chars_size];
    ret_chars.resize(ret_chars_size);
    auto ret_charsp = ret_chars.data();
    auto charsp = chars.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < one_byte_start_size; i++) {
      ret_charsp[start_indexp[one_byte_startp[i]]] =
        charsp[one_byte_startp[i]];
    }
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < two_byte_start_size; i++) {
      auto pos = two_byte_startp[i];
      ret_charsp[start_indexp[pos]] =
        ((charsp[pos] & 0x1F) << 6) | (charsp[pos+1] & 0x3F);
    }
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < three_byte_start_size; i++) {
      auto pos = three_byte_startp[i];
      ret_charsp[start_indexp[pos]] =
        ((charsp[pos] & 0xF) << 12) | ((charsp[pos+1] & 0x3F) << 6) |
        (charsp[pos+2] & 0x3F);
    }
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < four_byte_start_size; i++) {
      auto pos = four_byte_startp[i];
      ret_charsp[start_indexp[pos]] =
        ((charsp[pos] & 0x7) << 18) | ((charsp[pos+1] & 0x3F) << 12) |
        ((charsp[pos+2] & 0x3F) << 6) | (charsp[pos+3] & 0x3F);
    }
    auto lens_size = lens.size();
    auto lensp = lens.data();
    auto startsp = starts.data();
    ret_starts.resize(lens_size);
    auto ret_startsp = ret_starts.data();
    ret_lens.resize(lens_size);
    auto ret_lensp = ret_lens.data();
    for(size_t i = 0; i < lens_size; i++) {
      auto starts = startsp[i];
      auto ret_starts = start_indexp[starts];
      ret_startsp[i] = ret_starts;
      ret_lensp[i] = start_indexp[starts + lensp[i]] - ret_starts;
    }
  }
}

struct is_one_byte_utf32{
  int operator()(int c) const {
    return (c < 0x80);
  }
  SERIALIZE_NONE
};
struct is_two_byte_utf32{
  int operator()(int c) const {
    return (c >= 0x80 && c < 0x800);
  }
  SERIALIZE_NONE
};
struct is_three_byte_utf32{
  int operator()(int c) const {
    return (c >= 0x800 && c < 0x10000);
  }
  SERIALIZE_NONE
};
struct is_four_byte_utf32{
  int operator()(int c) const {
    return (c >= 0x10000);
  }
  SERIALIZE_NONE
};
void utf32_to_utf8(const std::vector<int>& chars,
                   const std::vector<size_t>& starts,
                   const std::vector<size_t>& lens,
                   std::vector<int>& ret_chars,
                   std::vector<size_t>& ret_starts,
                   std::vector<size_t>& ret_lens) {
  auto one_byte = find_condition(chars, is_one_byte_utf32());
  auto one_byte_size = one_byte.size();
  auto chars_size = chars.size();
  if(one_byte_size == chars_size) {
    ret_chars = chars;
    ret_starts = starts;
    ret_lens = lens;
    return;
  } else {
    auto two_byte = find_condition(chars, is_two_byte_utf32());
    auto three_byte = find_condition(chars, is_three_byte_utf32());
    auto four_byte = find_condition(chars, is_four_byte_utf32());
    auto two_byte_size = two_byte.size();
    auto three_byte_size = three_byte.size();
    auto four_byte_size = four_byte.size();
    auto one_bytep = one_byte.data();
    auto two_bytep = two_byte.data();
    auto three_bytep = three_byte.data();
    auto four_bytep = four_byte.data();
    std::vector<size_t> num_bytes(chars_size);
    auto num_bytesp = num_bytes.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < one_byte_size; i++) {
      num_bytesp[one_bytep[i]] = 1;
    }
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < two_byte_size; i++) {
      num_bytesp[two_bytep[i]] = 2;
    }
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < three_byte_size; i++) {
      num_bytesp[three_bytep[i]] = 3;
    }
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < four_byte_size; i++) {
      num_bytesp[four_bytep[i]] = 4;
    }
    std::vector<size_t> start_index(chars_size+1);
    auto start_indexp = start_index.data();
    prefix_sum(num_bytesp, start_indexp+1, chars_size);
    auto ret_chars_size = start_indexp[chars_size];
    ret_chars.resize(ret_chars_size);
    auto ret_charsp = ret_chars.data();
    auto charsp = chars.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < one_byte_size; i++) {
      ret_charsp[start_indexp[one_bytep[i]]] =
        charsp[one_bytep[i]];
    }
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < two_byte_size; i++) {
      auto pos = two_bytep[i];
      auto start_indexp_pos = start_indexp[pos];
      auto charsp_pos = charsp[pos];
      ret_charsp[start_indexp_pos] = 0xC0 | (charsp_pos >> 6);
      ret_charsp[start_indexp_pos + 1] = 0x80 | (charsp_pos & 0x3F);
    }
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < three_byte_size; i++) {
      auto pos = three_bytep[i];
      auto start_indexp_pos = start_indexp[pos];
      auto charsp_pos = charsp[pos];
      ret_charsp[start_indexp_pos] = 0xE0 | (charsp_pos >> 12);
      ret_charsp[start_indexp_pos + 1] = 0x80 | ((charsp_pos >> 6) & 0x3F);
      ret_charsp[start_indexp_pos + 2] = 0x80 | (charsp_pos & 0x3F);
    }
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < four_byte_size; i++) {
      auto pos = four_bytep[i];
      auto start_indexp_pos = start_indexp[pos];
      auto charsp_pos = charsp[pos];
      ret_charsp[start_indexp_pos] = 0xF0 | (charsp_pos >> 18);
      ret_charsp[start_indexp_pos + 1] = 0x80 | ((charsp_pos >> 12) & 0x3F);
      ret_charsp[start_indexp_pos + 2] = 0x80 | ((charsp_pos >> 6) & 0x3F);
      ret_charsp[start_indexp_pos + 3] = 0x80 | (charsp_pos & 0x3F);
    }
    auto lens_size = lens.size();
    auto lensp = lens.data();
    auto startsp = starts.data();
    ret_starts.resize(lens_size);
    auto ret_startsp = ret_starts.data();
    ret_lens.resize(lens_size);
    auto ret_lensp = ret_lens.data();
    for(size_t i = 0; i < lens_size; i++) {
      auto starts = startsp[i];
      auto ret_starts = start_indexp[starts];
      ret_startsp[i] = ret_starts;
      ret_lensp[i] = start_indexp[starts + lensp[i]] - ret_starts;
    }
  }
}

words utf8_to_utf32(const words& ws) {
  words ret;
  utf8_to_utf32(ws.chars, ws.starts, ws.lens, ret.chars, ret.starts, ret.lens);
  return ret;
}

words utf32_to_utf8(const words& ws) {
  words ret;
  utf32_to_utf8(ws.chars, ws.starts, ws.lens, ret.chars, ret.starts, ret.lens);
  return ret;
}

vector<int> utf8_to_utf32(const std::string& str) {
  words tmp;
  tmp.chars = char_to_int(str);
  tmp.lens = {str.size()};
  tmp.starts = {0};
  auto tmp2 = utf8_to_utf32(tmp);
  return tmp2.chars;
}

std::vector<int> 
ascii(const std::vector<int>& chars,
      const std::vector<size_t>& starts,
      const std::vector<size_t>& lens) {
  auto size = lens.size();
  std::vector<int> ret(size);
  auto retp = ret.data();
  auto lensp = lens.data();
  auto charsp = chars.data();
  auto startsp = starts.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < size; i++) {
    retp[i] = lensp[i] ? charsp[startsp[i]] : 0;
  }
  return ret;
}

std::vector<int> ascii(const words& ws) {
  return frovedis::ascii(ws.chars, ws.starts, ws.lens);
}

void initcap(const std::vector<int>& chars,
             const std::vector<size_t>& starts,
             const std::vector<size_t>& lens,
             std::vector<int>& ret_chars) {
  int offset = ('a' - 'A');
  tolower(chars, ret_chars);
  auto len = lens.size();
  auto cptr = ret_chars.data();
  auto lptr = lens.data();
  auto sptr = starts.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < len; ++i) {
    if (lptr[i]) {
      auto sidx = sptr[i];
      auto initchar = cptr[sidx];
      if (initchar >= 'a' && initchar <= 'z') cptr[sidx] -= offset; // toUpper
    }
  }

  int space = ' ';
  auto pos = vector_find_eq(chars, space);
  auto pos_ptr = pos.data();
  auto pos_size = pos.size();
  if (pos_size == 0) return;

  auto tsize = pos_size;
  // last character itself can be a space
  auto charsize = chars.size();
  if (pos[pos_size - 1] == (charsize - 1)) tsize--;

#pragma _NEC ivdep
  for(size_t i = 0; i < tsize; ++i) {
    auto nextchar = cptr[pos_ptr[i] + 1];
    if (nextchar >= 'a' && nextchar <= 'z') cptr[pos_ptr[i] + 1] -= offset; // toUpper
  }
}

words initcap(const words& ws) {
  words ret;
  frovedis::initcap(ws.chars, ws.starts, ws.lens, ret.chars);
  ret.starts = ws.starts;
  ret.lens = ws.lens;
  return ret;
}

std::vector<int> 
hamming_distance(const std::vector<int>& chars1,
                 const std::vector<size_t>& starts1,
                 const std::vector<size_t>& lens1,
                 const std::vector<int>& chars2,
                 const std::vector<size_t>& starts2,
                 const std::vector<size_t>& lens2,
                 bool& all_valid) {
  auto vlen = lens1.size();
  checkAssumption(vlen == lens2.size());

  auto cptr1 = chars1.data();
  auto cptr2 = chars2.data();
  auto sptr1 = starts1.data();
  auto sptr2 = starts2.data();
  auto lptr1 = lens1.data();
  auto lptr2 = lens2.data();

  int maxlen = 0, sum = 0;
  std::vector<int> difflen;
  std::vector<int> valid(vlen);
  auto validp = valid.data();

  for(size_t i = 0; i < vlen; ++i) {
    validp[i] = (lptr1[i] == lptr2[i]);
    sum += validp[i];
  }
  all_valid = (sum == vlen);

  if (!all_valid) { // some targets with mismatched lengths
    auto imax = std::numeric_limits<int>::max();
    difflen = vector_full<int>(vlen, imax);
    auto diffp = difflen.data();

    auto pos = vector_find_one(valid);
    auto psize = pos.size();
    auto posp = pos.data();
#pragma _NEC ivdep
    for(size_t i = 0; i < psize; ++i) {
      auto idx = posp[i];
      diffp[idx] = 0;
      auto l = lptr1[idx];
      if (l > maxlen) maxlen = l;
    }

    for(size_t i = 0; i < maxlen; ++i) {
#pragma _NEC ivdep
      for(size_t j = 0; j < psize; ++j) {
        auto idx = posp[j];
        if (i < lptr1[idx]) {
          auto c1 = cptr1[sptr1[idx] + i];
          auto c2 = cptr2[sptr2[idx] + i];
          diffp[idx] += (c1 != c2);
        }
      }
    }
  } else {
    difflen = vector_zeros<int>(vlen);
    auto diffp = difflen.data();

    for(size_t i = 0; i < vlen; ++i) {
      auto l = lptr1[i];
      if (l > maxlen) maxlen = l;
    }

    for(size_t i = 0; i < maxlen; ++i) {
#pragma _NEC ivdep
      for(size_t j = 0; j < vlen; ++j) {
        if (i < lptr1[j]) {
          auto c1 = cptr1[sptr1[j] + i];
          auto c2 = cptr2[sptr2[j] + i];
          diffp[j] += (c1 != c2);
        }
      }
    }
  }
  return difflen;
}

std::vector<int> 
hamming_distance(const words& w1, const words& w2, bool& all_valid) {
  return hamming_distance(w1.chars, w1.starts, w1.lens, 
                          w2.chars, w2.starts, w2.lens, all_valid);
}

// declared in core/vector_operations.hpp
template <>
void vector_replace_inplace(std::vector<std::string>& vec,
                            std::string& from, std::string& to) {
  auto w = vector_string_to_words(vec);
  w.replace(from, to);
  vec = words_to_vector_string(w);
}

template <>
std::vector<std::string>
vector_replace(const std::vector<std::string>& vec,
               std::string& from, std::string& to) {
  auto w = vector_string_to_words(vec);
  w.replace(from, to);
  return words_to_vector_string(w);
}

}
