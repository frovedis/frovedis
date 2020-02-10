#include "dict.hpp"
#include "../core/utility.hpp"
#include <stdexcept>

#include "words.hpp"

using namespace std;

namespace frovedis {

void compress_words(const int* vp,
                    const size_t* org_startsp,
                    const size_t* org_lensp,
                    size_t num_words,
                    std::vector<uint64_t>& compressed,
                    std::vector<size_t>& compressed_lens,
                    std::vector<size_t>& compressed_lens_num,
                    std::vector<size_t>& order) {
  std::vector<size_t> starts(num_words), lens(num_words);
  auto startsp = starts.data();
  auto lensp = lens.data();
  for(size_t i = 0; i < num_words; i++) lensp[i] = org_lensp[i];
  order.resize(num_words);
  auto orderp = order.data();
  for(size_t i = 0; i < num_words; i++) orderp[i] = i;
  radix_sort(lensp, orderp, num_words, true);
  for(size_t i = 0; i < num_words; i++) startsp[i] = org_startsp[orderp[i]];
  auto sep = set_separate(lens);
  auto sep_size = sep.size();
  auto sepp = sep.data();
  std::vector<size_t> compressed_lens_tmp(num_words);
  auto compressed_lens_tmpp = compressed_lens_tmp.data();
  for(size_t i = 0; i < num_words; i++) {
    // not inlined?
    // compressed_lens_tmpp[i] = ceil_div(lensp[i], size_t(8));
    compressed_lens_tmpp[i] = (lensp[i] == 0) ? 0 : (lensp[i] - 1) / 8 + 1;
  }
  auto sep_tmp = set_separate(compressed_lens_tmp);
  auto sep_tmpp = sep_tmp.data();
  auto sep_tmp_size = sep_tmp.size();
  compressed_lens.resize(sep_tmp_size-1);
  compressed_lens_num.resize(sep_tmp_size-1);
  auto compressed_lensp = compressed_lens.data();
  auto compressed_lens_nump = compressed_lens_num.data();
  for(size_t i = 0; i < sep_tmp_size-1; i++) {
    compressed_lensp[i] = compressed_lens_tmpp[sep_tmpp[i]];
    auto start = sep_tmpp[i];
    auto end = sep_tmpp[i+1];
    compressed_lens_nump[i] = end - start;
  }
  size_t total = 0;
  auto compressed_lens_size = compressed_lens.size();
  for(size_t i = 0; i < compressed_lens_size; i++) {
    total += compressed_lensp[i] * compressed_lens_nump[i];
  }
  compressed.resize(total);
  auto compressedp = compressed.data();
  auto current_out = compressedp;
  for(size_t i = 0; i < sep_size - 1; i++) {
    auto start = sepp[i];
    auto end = sepp[i+1];
    auto current_len = lensp[sepp[i]];
    auto current_compressed_len = compressed_lens_tmpp[sepp[i]];
    auto size = end - start;
    for(size_t j = 0; j < current_len; j++) {
      auto packed_word = j / 8;
      auto to_shift = (7 - (j - packed_word * 8)) * 8;
      for(size_t k = 0; k < size; k++) {
        current_out[current_compressed_len * k + packed_word] |=
          uint64_t(vp[startsp[start+k]+j]) << to_shift;
      }
    }
    current_out += size * current_compressed_len;
  }
}

void compress_words(const std::vector<int>& v,
                    const std::vector<size_t>& starts,
                    const std::vector<size_t>& lens,
                    std::vector<uint64_t>& compressed,
                    std::vector<size_t>& compressed_lens,
                    std::vector<size_t>& compressed_lens_num,
                    std::vector<size_t>& order) {
  if(starts.size() != lens.size())
    throw runtime_error("size of starts and lens are diffent");
  compress_words(v.data(), starts.data(), lens.data(), starts.size(),
                 compressed, compressed_lens, compressed_lens_num, order);
}

void print_compressed_words(const std::vector<uint64_t>& compressed,
                            const std::vector<size_t>& compressed_lens,
                            const std::vector<size_t>& compressed_lens_num,
                            const std::vector<size_t>& order) {
  size_t num_words = order.size();
  vector<size_t> rev_order(num_words);
  auto orderp = order.data();
  auto rev_orderp = rev_order.data();
#pragma _NEC ivdep
  for(size_t i = 0; i < num_words; i++) {
    rev_orderp[orderp[i]] = i;
  }
  vector<size_t> starts(num_words);
  vector<size_t> lens(num_words);
  auto startsp = starts.data();
  auto lensp = lens.data();
  auto compressed_lens_size = compressed_lens.size();
  auto compressed_lensp = compressed_lens.data();
  auto compressed_lens_nump = compressed_lens_num.data();
  auto current_startsp = startsp;
  auto current_lensp = lensp;
  size_t current_pos = 0;
  for(size_t i = 0; i < compressed_lens_size; i++) {
    for(size_t j = 0; j < compressed_lens_nump[i]; j++) {
      current_startsp[j] = current_pos + compressed_lensp[i] * j;
      current_lensp[j] = compressed_lensp[i];
    }
    current_pos += compressed_lensp[i] * compressed_lens_nump[i];
    current_startsp += compressed_lens_nump[i];
    current_lensp += compressed_lens_nump[i];
  }

  auto compressedp = compressed.data();
  for(size_t i = 0; i < num_words; i++) {
    auto start = startsp[rev_orderp[i]];
    auto len = lensp[rev_orderp[i]];
    for(size_t j = 0; j < len; j++) {
      auto packed = compressedp[start + j];
      for(size_t k = 0; k < 8; k++) {
        auto c = static_cast<char>((packed >> ((7 - k) * 8)
                                    & 0x00000000000000FF));
        if(c) cout << c;
      }
    }
    cout << " ";
  }
  cout << endl;
}

void print_compressed_words(const std::vector<uint64_t>& compressed,
                            const std::vector<size_t>& compressed_lens,
                            const std::vector<size_t>& compressed_lens_num,
                            bool print_index) {
  auto lens_size = compressed_lens.size();
  size_t current = 0;
  size_t current_idx = 0;
  for(size_t i = 0; i < lens_size; i++) {
    auto lens = compressed_lens[i];
    auto lens_num = compressed_lens_num[i];
    for(size_t j = 0; j < lens_num; j++) {
      if(print_index) {
        cout << current_idx << ":";
      }
      for(size_t k = 0; k < lens; k++) {
        auto packed = compressed[current + lens * j + k];
        for(size_t l = 0; l < 8; l++) {
          auto c = static_cast<char>((packed >> ((7 - l) * 8)
                                      & 0x00000000000000FF));
          if(c) cout << c;
        }
      }
      cout << " ";
      current_idx++;
    }
    current += lens * lens_num;
  }
  cout << endl;
}

struct words compressed_words::decompress() {
  struct words ret;
  auto total_chars = words.size() * 8;
  ret.chars.resize(total_chars);
  auto charsp = ret.chars.data();
  auto wordsp = words.data();
  auto words_size = words.size();
  for(size_t i = 0; i < words_size; i++) {
    charsp[i*8] = (wordsp[i] >> (7 * 8)) & 0x00000000000000FF;
    charsp[i*8+1] = (wordsp[i] >> (6 * 8)) & 0x00000000000000FF;
    charsp[i*8+2] = (wordsp[i] >> (5 * 8)) & 0x00000000000000FF;
    charsp[i*8+3] = (wordsp[i] >> (4 * 8)) & 0x00000000000000FF;
    charsp[i*8+4] = (wordsp[i] >> (3 * 8)) & 0x00000000000000FF;
    charsp[i*8+5] = (wordsp[i] >> (2 * 8)) & 0x00000000000000FF;
    charsp[i*8+6] = (wordsp[i] >> (1 * 8)) & 0x00000000000000FF;
    charsp[i*8+7] = wordsp[i] & 0x00000000000000FF;
  }

  auto num_words_ = num_words();
  vector<size_t> startstmp(num_words_);
  vector<size_t> lenstmp(num_words_);
  auto startstmpp = startstmp.data();
  auto lenstmpp = lenstmp.data();
  auto crnt_start = 0;
  auto crnt_startstmpp = startstmpp;
  auto crnt_lenstmpp = lenstmpp;
  for(size_t len_i = 0; len_i < lens.size(); len_i++) {
    auto len = lens[len_i];
    auto len_num = lens_num[len_i];
    for(size_t i = 0; i < len_num; i++) {
      crnt_startstmpp[i] = crnt_start + i * len * 8;
      crnt_lenstmpp[i] = len * 8;
    }
    crnt_startstmpp += len_num;
    crnt_lenstmpp += len_num;
    crnt_start += len * len_num * 8;
  }
  ret.starts.resize(num_words_);
  ret.lens.resize(num_words_);
  auto startsp = ret.starts.data();
  auto lensp = ret.lens.data();
  auto orderp = order.data();
  for(size_t i = 0; i < num_words_; i++) {
    startsp[orderp[i]] = startstmpp[i];
    lensp[orderp[i]] = lenstmpp[i];
  }
  ret.trim_tail("\0");
  return ret;
}

void compressed_words::lexical_sort() {
  if(lens.size() == 0) return;
  auto num_words_ = num_words();
  auto wordsp = words.data();
  vector<uint64_t> buf(num_words_);
  auto bufp = buf.data();
  vector<size_t> rev_order(num_words_);
  auto rev_orderp = rev_order.data();
  for(size_t i = 0; i < num_words_; i++) rev_orderp[i] = i;
  auto lens_size = lens.size();
  auto max_len = lens[lens_size - 1];
  for(size_t crnt_packed = max_len; crnt_packed > 0; crnt_packed--) {
    size_t lenpos = 0;
    for(; lenpos < lens.size(); lenpos++) {
      if(lens[lenpos] >= crnt_packed) break;
    }
    size_t tmp = 0;
    for(size_t i = 0; i < lenpos; i++) tmp += lens_num[i];
    size_t work_size = num_words_ - tmp;
    size_t words_off = 0;
    for(size_t i = 0; i < lenpos; i++) words_off += lens_num[i] * lens[i];
    auto crnt_wordsp = wordsp + words_off;
    auto work_buf = bufp + num_words_ - work_size;
    auto crnt_work_buf = work_buf;
    for(size_t i = lenpos; i < lens_size; i++) {
      auto len = lens[i];
      auto len_num = lens_num[i];
      for(size_t j = 0; j < len_num; j++) {
        crnt_work_buf[j] = crnt_wordsp[len * j + crnt_packed - 1];
      }
      crnt_work_buf += len_num;
      crnt_wordsp += len * len_num;
    }
    radix_sort(work_buf, rev_orderp + num_words_ - work_size, work_size);
  }
  auto orderp = order.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < num_words_; i++) {
    orderp[rev_orderp[i]] = i;
  }
}

compressed_words merge_compressed_words(const compressed_words& a,
                                        const compressed_words& b) {
  compressed_words ret;
  auto size = a.words.size() + b.words.size();
  ret.words.resize(size);
  ret.lens = set_union(a.lens, b.lens);
  ret.lens_num.resize(ret.lens.size());
  auto num_words = a.order.size() + b.order.size();
  ret.order.resize(num_words);
  auto ret_wordsp = ret.words.data();
  auto a_wordsp = a.words.data();
  auto b_wordsp = b.words.data();
  auto ret_orderp = ret.order.data();
  auto a_orderp = a.order.data();
  auto b_orderp = b.order.data();
  size_t aidx = 0;
  size_t bidx = 0;
  auto crnt_ret_wordsp = ret_wordsp;
  auto crnt_a_wordsp = a_wordsp;
  auto crnt_b_wordsp = b_wordsp;
  auto offset = a.order.size();
  auto crnt_ret_orderp = ret_orderp;
  auto crnt_a_orderp = a_orderp;
  auto crnt_b_orderp = b_orderp;
  for(size_t i = 0; i < ret.lens.size(); i++) {
    auto crnt_lens_num = 0;
    if(a.lens[aidx] == ret.lens[i]) {
      auto a_lens_num = a.lens_num[aidx];
      auto total_size = a_lens_num * a.lens[aidx];
      for(size_t j = 0; j < total_size; j++) {
        crnt_ret_wordsp[j] = crnt_a_wordsp[j];
      }
      for(size_t j = 0; j < a_lens_num; j++) {
        crnt_ret_orderp[j] = crnt_a_orderp[j];
      }
      crnt_ret_wordsp += total_size;
      crnt_a_wordsp += total_size;
      crnt_ret_orderp += a_lens_num;
      crnt_a_orderp += a_lens_num;
      crnt_lens_num += a_lens_num;
      aidx++;
    }
    if(b.lens[bidx] == ret.lens[i]) {
      auto b_lens_num = b.lens_num[bidx];
      auto total_size = b_lens_num * b.lens[bidx];
      for(size_t j = 0; j < total_size; j++) {
        crnt_ret_wordsp[j] = crnt_b_wordsp[j];
      }
      for(size_t j = 0; j < b_lens_num; j++) {
        crnt_ret_orderp[j] = crnt_b_orderp[j] + offset;
      }
      crnt_ret_wordsp += total_size;
      crnt_b_wordsp += total_size;
      crnt_ret_orderp += b_lens_num;
      crnt_b_orderp += b_lens_num;
      crnt_lens_num += b_lens_num;
      bidx++;
    }
    ret.lens_num[i] = crnt_lens_num;
  }
  return ret;
}

compressed_words make_compressed_words(const words& w) {
  compressed_words ret;
  compress_words(w.chars, w.starts, w.lens, ret.words, ret.lens, ret.lens_num,
                 ret.order);
  return ret;
}

void unique_words(const std::vector<uint64_t>& compressed,
                  const std::vector<size_t>& compressed_lens,
                  const std::vector<size_t>& compressed_lens_num,
                  std::vector<uint64_t>& unique_compressed,
                  std::vector<size_t>& unique_compressed_lens,
                  std::vector<size_t>& unique_compressed_lens_num) {
  auto compressed_size = compressed.size();
  auto compressed_lens_size = compressed_lens.size();
  vector<uint64_t> unique_compressed_buf(compressed_size);
  unique_compressed_lens.resize(compressed_lens.size());
  unique_compressed_lens_num.resize(compressed_lens_num.size());

  auto compressedp = compressed.data();
  auto compressed_lensp = compressed_lens.data();
  auto compressed_lens_nump = compressed_lens_num.data();
  auto unique_compressed_lensp = unique_compressed_lens.data();
  auto unique_compressed_lens_nump = unique_compressed_lens_num.data();
  auto unique_bufp = unique_compressed_buf.data();
  auto current_compressedp = compressedp;
  auto current_bufp = unique_bufp;
  for(size_t i = 0; i < compressed_lens_size; i++) {
    auto size = compressed_lens_nump[i];
    vector<uint64_t> packed(size);
    auto packedp = packed.data();
    auto len = compressed_lensp[i];
    vector<size_t> idx(size);
    auto idxp = idx.data();
    for(size_t j = 0; j < size; j++) idxp[j] = j;
    for(size_t j = 0; j < len; j++) {
      for(size_t k = 0; k < size; k++) {
        packedp[k] = current_compressedp[idxp[k] * len + (len - j - 1)];
      }
      radix_sort(packedp, idxp, size, true);
    }
    vector<size_t> sep;
    for(size_t j = 0; j < len; j++) {
      for(size_t k = 0; k < size; k++) {
        packedp[k] = current_compressedp[idxp[k] * len + j];
      }
      auto tmp = set_separate(packed);
      sep = set_union(tmp, sep);
    }
    auto sep_size = sep.size();
    auto sepp = sep.data();
#pragma _NEC novector
    for(size_t j = 0; j < len; j++) {
      for(size_t k = 0; k < sep_size - 1; k++) {
        current_bufp[k * len + j] =
          current_compressedp[idxp[sepp[k]] * len + j];
      }
    }
    unique_compressed_lensp[i] = len;
    unique_compressed_lens_nump[i] = sep_size - 1;
    current_compressedp += len * size;
    current_bufp += len * (sep_size - 1);
  }
  auto res_size = current_bufp - unique_bufp;
  unique_compressed.resize(res_size);
  auto unique_compressedp = unique_compressed.data();
  for(size_t i = 0; i < res_size; i++) {
    unique_compressedp[i] = unique_bufp[i];
  }
}

// from non-compressed words
dict make_dict(const words& w) {
  vector<uint64_t> compressed;
  vector<size_t> compressed_lens;
  vector<size_t> compressed_lens_num;
  vector<size_t> order; // not used
  compress_words(w.chars, w.starts, w.lens, compressed, compressed_lens,
                 compressed_lens_num, order);
  dict ret;
  unique_words(compressed, compressed_lens, compressed_lens_num,
               ret.words, ret.lens, ret.lens_num);
  return ret;
}

dict make_dict_from_words(const words& w) {
  return make_dict(w);
}

// from compressed words
dict make_dict(const compressed_words& comp) {
  dict ret;
  unique_words(comp.words, comp.lens, comp.lens_num,
               ret.words, ret.lens, ret.lens_num);
  return ret;
}

dict make_dict_from_compressed(const compressed_words& comp) {
  return make_dict(comp);
}

dict merge_dict(const dict& a, const dict& b) {
  auto size = a.words.size() + b.words.size();
  vector<uint64_t> tmp_words(size);
  auto tmp_lens = set_union(a.lens, b.lens);
  vector<size_t> tmp_lens_num(tmp_lens.size());
  auto tmp_wordsp = tmp_words.data();
  auto a_wordsp = a.words.data();
  auto b_wordsp = b.words.data();
  size_t aidx = 0;
  size_t bidx = 0;
  auto crnt_tmp_wordsp = tmp_wordsp;
  auto crnt_a_wordsp = a_wordsp;
  auto crnt_b_wordsp = b_wordsp;
  for(size_t i = 0; i < tmp_lens.size(); i++) {
    auto crnt_lens_num = 0;
    if(a.lens[aidx] == tmp_lens[i]) {
      auto total = a.lens_num[aidx] * a.lens[aidx];
      for(size_t j = 0; j < total; j++) {
        crnt_tmp_wordsp[j] = crnt_a_wordsp[j];
      }
      crnt_tmp_wordsp += total;
      crnt_a_wordsp += total;
      crnt_lens_num += a.lens_num[aidx];
      aidx++;
    }
    if(b.lens[bidx] == tmp_lens[i]) {
      auto total = b.lens_num[bidx] * b.lens[bidx];
      for(size_t j = 0; j < total; j++) {
        crnt_tmp_wordsp[j] = crnt_b_wordsp[j];
      }
      crnt_tmp_wordsp += total;
      crnt_b_wordsp += total;
      crnt_lens_num += b.lens_num[bidx];
      bidx++;
    }
    tmp_lens_num[i] = crnt_lens_num;
  }
  dict ret;
  unique_words(tmp_words, tmp_lens, tmp_lens_num,
               ret.words, ret.lens, ret.lens_num);
  return ret;
}

std::vector<size_t> dict::lookup(const compressed_words& cw) const {
  auto& words_lookup = cw.words;
  auto& lens_lookup = cw.lens;
  auto& lens_num_lookup = cw.lens_num;
  if(words_lookup.size() == 0) return vector<size_t>();
  auto NOT_FOUND = numeric_limits<size_t>::max();
  size_t num_words_lookup = cw.num_words();
  std::vector<size_t> ret(num_words_lookup);
  auto retp = ret.data();

  ssize_t left[DICT_VLEN]; // need to be signed?
#pragma _NEC vreg(left) 
  ssize_t right[DICT_VLEN];
#pragma _NEC vreg(right)
  ssize_t mid[DICT_VLEN];
#pragma _NEC vreg(mid)
  int is_valid[DICT_VLEN];
#pragma _NEC vreg(is_valid)
  int is_eq[DICT_VLEN];
#pragma _NEC vreg(is_eq)
  int is_lt[DICT_VLEN];
#pragma _NEC vreg(is_lt)
  size_t retbuf[DICT_VLEN];
#pragma _NEC vreg(retbuf)
  uint64_t packed_lookup_1st[DICT_VLEN];
#pragma _NEC vreg(packed_lookup_1st)

  auto crnt_words_lookupp = words_lookup.data();
  auto crnt_retp = retp;
  auto lens_lookup_size = lens_lookup.size();

  for(size_t len_i = 0; len_i < lens_lookup_size; len_i++) {
    auto len = lens_lookup[len_i];
    auto to_lookup_size = lens_num_lookup[len_i];
    size_t lenpos = 0;
    for(; lenpos < lens.size(); lenpos++) {
      if(lens[lenpos] == len) break;
    }
    if(lenpos == lens.size()) { // no such len in dict
      for(size_t i = 0; i < to_lookup_size; i++) crnt_retp[i] = NOT_FOUND;
      crnt_words_lookupp += len * to_lookup_size;
      crnt_retp += to_lookup_size;
      continue;
    }
    size_t total = 0;
    for(size_t i = 0; i < lenpos; i++) total += lens[i] * lens_num[i];
    auto crnt_wordsp = words.data() + total;
    size_t offset = 0;
    for(size_t i = 0; i < lenpos; i++) offset += lens_num[i];
    size_t block_size = to_lookup_size / DICT_VLEN;
    size_t remain = to_lookup_size % DICT_VLEN;
    // VLEN loop 
    for(size_t i = 0; i < block_size; i++) {
      for(size_t j = 0; j < DICT_VLEN; j++) {
        left[j] = 0;
        right[j] = lens_num[lenpos] - 1;
        is_valid[j] = 1;
      }
      for(size_t j = 0; j < DICT_VLEN; j++) {
        packed_lookup_1st[j] = crnt_words_lookupp[len * j];
      }
      size_t max_iter = 0;
      auto tmp = lens_num[lenpos];
      while(tmp >>= 1) ++max_iter;
      max_iter++;
      for(size_t lp = 0; lp < max_iter; lp++) {
        for(size_t j = 0; j < DICT_VLEN; j++) {
          if(is_valid[j]) mid[j] = (left[j] + right[j])/2;
        }
#pragma _NEC ivdep
        for(size_t j = 0; j < DICT_VLEN; j++) {
          if(is_valid[j]) {
            auto packed_dict = crnt_wordsp[len * mid[j]];
            is_eq[j] = (packed_dict == packed_lookup_1st[j]);
            is_lt[j] = (packed_dict < packed_lookup_1st[j]);
          }
        }
        for(size_t l = 1; l < len; l++) {
#pragma _NEC ivdep
          for(size_t j = 0; j < DICT_VLEN; j++) {
            if(is_valid[j]) {
              auto packed_lookup = crnt_words_lookupp[len * j + l];
              auto packed_dict = crnt_wordsp[len * mid[j] + l];
              int crnt_is_eq = (packed_dict == packed_lookup);
              int crnt_is_lt = (packed_dict < packed_lookup);
              if(is_eq[j]) is_lt[j] = crnt_is_lt;
              if(!crnt_is_eq) is_eq[j] = 0;
            }
          }
        }
#pragma _NEC ivdep
        for(size_t j = 0; j < DICT_VLEN; j++) {
          if(is_valid[j]) {
            if(is_eq[j]) {
              retbuf[j] = mid[j]; // later offset will be added
              is_valid[j] = false;
            } else if(is_lt[j]) {
              left[j] = mid[j] + 1;
            } else {
              right[j] = mid[j] - 1;
            }
            if(right[j] < left[j]) {
              retbuf[j] = NOT_FOUND;
              is_valid[j] = false;
            }
          }
        }
      }
      for(size_t j = 0; j < DICT_VLEN; j++) {
        if(retbuf[j] != NOT_FOUND) crnt_retp[j] = retbuf[j] + offset;
        else crnt_retp[j] = NOT_FOUND;
      }
      crnt_retp += DICT_VLEN;
      crnt_words_lookupp += len * DICT_VLEN;
    }
    // remain; use different variable to avoid error (2.4.1)
    ssize_t left2[DICT_VLEN];
    ssize_t right2[DICT_VLEN];
    ssize_t mid2[DICT_VLEN];
    int is_valid2[DICT_VLEN];
    int is_eq2[DICT_VLEN];
    int is_lt2[DICT_VLEN];
    size_t retbuf2[DICT_VLEN];
    uint64_t packed_lookup_1st2[DICT_VLEN];
    for(size_t j = 0; j < remain; j++) {
      left2[j] = 0;
      right2[j] = lens_num[lenpos] - 1;
      is_valid2[j] = 1;
    }
    for(size_t j = 0; j < remain; j++) {
      packed_lookup_1st2[j] = crnt_words_lookupp[len * j];
    }
    size_t max_iter = 0;
    auto tmp = lens_num[lenpos];
    while(tmp >>= 1) ++max_iter;
    max_iter++;
    for(size_t lp = 0; lp < max_iter; lp++) {
      for(size_t j = 0; j < remain; j++) {
        if(is_valid2[j]) mid2[j] = (left2[j] + right2[j])/2;
      }
#pragma _NEC ivdep
      for(size_t j = 0; j < remain; j++) {
        if(is_valid2[j]) {
          auto packed_dict = crnt_wordsp[len * mid2[j]];
          is_eq2[j] = (packed_dict == packed_lookup_1st2[j]);
          is_lt2[j] = (packed_dict < packed_lookup_1st2[j]);
        }
      }
      for(size_t l = 1; l < len; l++) {
#pragma _NEC ivdep
        for(size_t j = 0; j < remain; j++) {
          if(is_valid2[j]) {
            auto packed_lookup = crnt_words_lookupp[len * j + l];
            auto packed_dict = crnt_wordsp[len * mid2[j] + l];
            int crnt_is_eq2 = (packed_dict == packed_lookup);
            int crnt_is_lt2 = (packed_dict < packed_lookup);
            if(is_eq2[j]) is_lt2[j] = crnt_is_lt2;
            if(!crnt_is_eq2) is_eq2[j] = 0;
          }
        }
      }
#pragma _NEC ivdep
      for(size_t j = 0; j < remain; j++) {
        if(is_valid2[j]) {
          if(is_eq2[j]) {
            retbuf2[j] = mid2[j]; // later offset will be added
            is_valid2[j] = false;
          } else if(is_lt2[j]) {
            left2[j] = mid2[j] + 1;
          } else {
            right2[j] = mid2[j] - 1;
          }
          if(right2[j] < left2[j]) {
            retbuf2[j] = NOT_FOUND;
            is_valid2[j] = false;
          }
        }
      }
    }
    for(size_t j = 0; j < remain; j++) {
      if(retbuf2[j] != NOT_FOUND) crnt_retp[j] = retbuf2[j] + offset;
      else crnt_retp[j] = NOT_FOUND;
    }
    crnt_retp += remain;
    crnt_words_lookupp += len * remain;
  }
  auto orderp = cw.order.data();
  std::vector<size_t> order_ret(num_words_lookup);
  auto order_retp = order_ret.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < num_words_lookup; i++) {
    order_retp[orderp[i]] = retp[i];
  }
  return order_ret;
}

size_t dict::num_words() const {
  size_t num_words = 0;
  for(size_t i = 0; i < lens_num.size(); i++) num_words += lens_num[i];
  return num_words;
}

}
