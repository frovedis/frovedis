#include "dict.hpp"
#include "../core/utility.hpp"
#include "../core/upper_bound.hpp"
#include "../core/prefix_sum.hpp"
#include "../dataframe/hashtable.hpp"
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
    // if(lensp[i] == 0) lensp[i] should be 1, 
    // because always at least one data should be stored
    compressed_lens_tmpp[i] = (lensp[i] == 0) ? 1 : (lensp[i] - 1) / 8 + 1;
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

words decompress_compressed_words(const std::vector<uint64_t>& cwords,
                                  const std::vector<size_t>& lens,
                                  const std::vector<size_t>& lens_num,
                                  const std::vector<size_t>& order) {
  words ret;
  auto total_chars = cwords.size() * 8;
  ret.chars.resize(total_chars);
  auto charsp = ret.chars.data();
  auto cwordsp = cwords.data();
  auto cwords_size = cwords.size();
  for(size_t i = 0; i < cwords_size; i++) {
    charsp[i*8] = (cwordsp[i] >> (7 * 8)) & 0x00000000000000FF;
    charsp[i*8+1] = (cwordsp[i] >> (6 * 8)) & 0x00000000000000FF;
    charsp[i*8+2] = (cwordsp[i] >> (5 * 8)) & 0x00000000000000FF;
    charsp[i*8+3] = (cwordsp[i] >> (4 * 8)) & 0x00000000000000FF;
    charsp[i*8+4] = (cwordsp[i] >> (3 * 8)) & 0x00000000000000FF;
    charsp[i*8+5] = (cwordsp[i] >> (2 * 8)) & 0x00000000000000FF;
    charsp[i*8+6] = (cwordsp[i] >> (1 * 8)) & 0x00000000000000FF;
    charsp[i*8+7] = cwordsp[i] & 0x00000000000000FF;
  }

  auto num_words_ = order.size();
  vector<size_t> startstmp(num_words_);
  vector<size_t> lenstmp(num_words_);
  auto startstmpp = startstmp.data();
  auto lenstmpp = lenstmp.data();
  size_t crnt_start = 0;
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
  string to_trim; // initializing by "\0" does not work
  to_trim.resize(1);
  to_trim[0] = 0;
  ret.trim_tail(to_trim);
  return ret;
}

words compressed_words::decompress() const {
  return decompress_compressed_words(cwords, lens, lens_num, order);
}

void lexical_sort_compressed_words(const std::vector<uint64_t>& cwords,
                                   const std::vector<size_t>& lens,
                                   const std::vector<size_t>& lens_num,
                                   std::vector<size_t>& order) {
  if(lens.size() == 0) return;
  auto num_words_ = order.size();
  auto cwordsp = cwords.data();
  vector<uint64_t> buf(num_words_);
  vector<uint64_t> buf2(num_words_);
  auto bufp = buf.data();
  auto buf2p = buf2.data();
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
    auto crnt_wordsp = cwordsp + words_off;
    auto work_buf = bufp + num_words_ - work_size;
    auto work_buf2 = buf2p + num_words_ - work_size;
    auto work_rev_orderp = rev_orderp + num_words_ - work_size;
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
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < work_size; i++) {
      work_buf2[i] = work_buf[work_rev_orderp[i]-(num_words_-work_size)];
    }
    radix_sort(work_buf2, work_rev_orderp, work_size);
  }
  auto orderp = order.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < num_words_; i++) {
    orderp[rev_orderp[i]] = i;
  }
}

void compressed_words::lexical_sort() {
  lexical_sort_compressed_words(cwords, lens, lens_num, order);
}

compressed_words merge_compressed_words(const compressed_words& a,
                                        const compressed_words& b) {
  if(a.cwords.size() == 0) return b;
  else if (b.cwords.size() == 0) return a;
  compressed_words ret;
  auto size = a.cwords.size() + b.cwords.size();
  ret.cwords.resize(size);
  ret.lens = set_union(a.lens, b.lens);
  ret.lens_num.resize(ret.lens.size());
  auto num_words = a.order.size() + b.order.size();
  ret.order.resize(num_words);
  auto ret_cwordsp = ret.cwords.data();
  auto a_cwordsp = a.cwords.data();
  auto b_cwordsp = b.cwords.data();
  auto ret_orderp = ret.order.data();
  auto a_orderp = a.order.data();
  auto b_orderp = b.order.data();
  size_t aidx = 0;
  size_t bidx = 0;
  auto crnt_ret_cwordsp = ret_cwordsp;
  auto crnt_a_cwordsp = a_cwordsp;
  auto crnt_b_cwordsp = b_cwordsp;
  auto offset = a.order.size();
  auto crnt_ret_orderp = ret_orderp;
  auto crnt_a_orderp = a_orderp;
  auto crnt_b_orderp = b_orderp;
  auto a_lens_size = a.lens.size();
  auto b_lens_size = b.lens.size();
  for(size_t i = 0; i < ret.lens.size(); i++) {
    size_t crnt_lens_num = 0;
    if(aidx < a_lens_size && a.lens[aidx] == ret.lens[i]) {
      auto a_lens_num = a.lens_num[aidx];
      auto total_size = a_lens_num * a.lens[aidx];
      for(size_t j = 0; j < total_size; j++) {
        crnt_ret_cwordsp[j] = crnt_a_cwordsp[j];
      }
      for(size_t j = 0; j < a_lens_num; j++) {
        crnt_ret_orderp[j] = crnt_a_orderp[j];
      }
      crnt_ret_cwordsp += total_size;
      crnt_a_cwordsp += total_size;
      crnt_ret_orderp += a_lens_num;
      crnt_a_orderp += a_lens_num;
      crnt_lens_num += a_lens_num;
      aidx++;
    }
    if(bidx < b_lens_size && b.lens[bidx] == ret.lens[i]) {
      auto b_lens_num = b.lens_num[bidx];
      auto total_size = b_lens_num * b.lens[bidx];
      for(size_t j = 0; j < total_size; j++) {
        crnt_ret_cwordsp[j] = crnt_b_cwordsp[j];
      }
      for(size_t j = 0; j < b_lens_num; j++) {
        crnt_ret_orderp[j] = crnt_b_orderp[j] + offset;
      }
      crnt_ret_cwordsp += total_size;
      crnt_b_cwordsp += total_size;
      crnt_ret_orderp += b_lens_num;
      crnt_b_orderp += b_lens_num;
      crnt_lens_num += b_lens_num;
      bidx++;
    }
    ret.lens_num[i] = crnt_lens_num;
  }
  return ret;
}

// TODO: copying at a time like merge_multi_words would be faster...
// input is destructive: not const
compressed_words
merge_multi_compressed_words(vector<compressed_words>& vcws) {
  // clear() is for fail safe...
  auto vcws_size = vcws.size();
  if(vcws_size == 0) {
    return compressed_words();
  } else if(vcws_size == 1) {
    return vcws[0];
  } else if(vcws_size == 2) {
    auto r = merge_compressed_words(vcws[0], vcws[1]);
    vcws[0].clear();
    vcws[1].clear();
    return r;
  } else {
    auto left_size = ceil_div(vcws_size, size_t(2));
    auto right_size = vcws_size - left_size;
    vector<compressed_words> left(left_size);
    vector<compressed_words> right(right_size);
    for(size_t i = 0; i < left_size; i++) {
      left[i] = move(vcws[i]);
      vcws[i].clear();
    }
    for(size_t i = 0; i < right_size; i++) {
      right[i] = move(vcws[left_size + i]);
      vcws[left_size + i].clear();
    }
    auto left_merged = merge_multi_compressed_words(left);
    for(size_t i = 0; i < left_size; i++) left[i].clear();
    auto right_merged = merge_multi_compressed_words(right);
    for(size_t i = 0; i < right_size; i++) right[i].clear();
    auto r = merge_compressed_words(left_merged, right_merged);
    left_merged.clear();
    right_merged.clear();
    return r;
  }
}

compressed_words make_compressed_words(const words& w) {
  compressed_words ret;
  compress_words(w.chars, w.starts, w.lens, ret.cwords, ret.lens, ret.lens_num,
                 ret.order);
  return ret;
}

compressed_words
compressed_words::extract(const std::vector<size_t>& idx) const {
  auto idxp = idx.data();
  auto idx_size = idx.size();
  if(idx_size == 0) return compressed_words();
  size_t max = 0;
  for(size_t i = 0; i < idx_size; i++) {
    if(max < idxp[i]) max = idxp[i];
  }
  auto order_size = order.size();
  if(max >= order_size)
    throw std::runtime_error
      ("compressed_words::extract: index exceeds num_words");
  std::vector<size_t> rorder(order_size);
  auto rorderp = rorder.data();
  auto orderp = order.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < order_size; i++) {
    rorderp[orderp[i]] = i;
  }
  std::vector<size_t> sidx(idx_size);
  auto sidxp = sidx.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < idx_size; i++) {
    sidxp[i] = rorderp[idxp[i]];
  }
  compressed_words ret;
  ret.order.resize(idx_size);
  auto ret_orderp = ret.order.data();
  for(size_t i = 0; i < idx_size; i++) ret_orderp[i] = i;
  radix_sort(sidx, ret.order);
  auto px_lens_num = prefix_sum(lens_num);
  auto lens_pos = upper_bound(px_lens_num, sidx);
  auto sep = set_separate(lens_pos);
  auto ret_lens_size = sep.size() - 1;
  ret.lens.resize(ret_lens_size);
  ret.lens_num.resize(ret_lens_size);
  auto ret_lensp = ret.lens.data();
  auto ret_lens_nump = ret.lens_num.data();
  auto sepp = sep.data();
  auto lensp = lens.data();
  auto lens_posp = lens_pos.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < ret_lens_size; i++) {
    ret_lensp[i] = lensp[lens_posp[sepp[i]]];
    ret_lens_nump[i] = sepp[i+1] - sepp[i];
  }
  size_t ret_cwords_len = 0;
  for(size_t i = 0; i < ret_lens_size; i++) {
    ret_cwords_len += ret_lensp[i] * ret_lens_nump[i];
  }
  ret.cwords.resize(ret_cwords_len);

  auto lens_nump = lens_num.data();
  auto lens_num_size = lens_num.size();
  std::vector<size_t> mult_lens_num(lens_num_size);
  auto mult_lens_nump = mult_lens_num.data();
  for(size_t i = 0; i < lens_num_size; i++) {
    mult_lens_nump[i] = lens_nump[i] * lensp[i];
  }
  std::vector<size_t> px_mult_lens_num(lens_num_size+1);
  auto px_mult_lens_nump = px_mult_lens_num.data();
  prefix_sum(mult_lens_num.data(), px_mult_lens_nump+1, lens_num_size);

  auto ret_cwordsp = ret.cwords.data();
  auto crnt_ret_cwordsp = ret_cwordsp;
  auto cwordsp = cwords.data();
  auto px_lens_nump = px_lens_num.data();
  size_t crnt_idx = 0;
  sidxp = sidx.data();
  for(size_t i = 0; i < ret_lens_size; i++) {
    auto crnt_cwordsp = cwordsp + px_mult_lens_nump[lens_posp[sepp[i]]];
    auto crnt_len = ret_lensp[i];
    auto crnt_lens_num = ret_lens_nump[i];
    auto to_shift =
      lens_posp[sepp[i]] == 0 ? 0 : px_lens_nump[lens_posp[sepp[i]] - 1];
#pragma _NEC novector
#pragma _NEC vob
    for(size_t l = 0; l < crnt_len; l++) {
#pragma _NEC ivdep
#pragma _NEC vovertake
      for(size_t j = 0; j < crnt_lens_num; j++) {
        crnt_ret_cwordsp[j * crnt_len + l] =
          crnt_cwordsp[(sidxp[crnt_idx + j] - to_shift) * crnt_len + l];
      }
    }
    crnt_ret_cwordsp += crnt_len * crnt_lens_num;
    crnt_idx += crnt_lens_num;
  }
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
               ret.cwords, ret.lens, ret.lens_num);
  return ret;
}

dict make_dict_from_words(const words& w) {
  return make_dict(w);
}

// from compressed words
dict make_dict(const compressed_words& comp) {
  dict ret;
  unique_words(comp.cwords, comp.lens, comp.lens_num,
               ret.cwords, ret.lens, ret.lens_num);
  return ret;
}

dict make_dict_from_compressed(const compressed_words& comp) {
  return make_dict(comp);
}

dict merge_dict(const dict& a, const dict& b) {
  auto asize = a.cwords.size();
  auto bsize = b.cwords.size();
  if(asize == 0) return b;
  if(bsize == 0) return a;
  auto size = asize + bsize;
  vector<uint64_t> tmp_cwords(size);
  auto tmp_lens = set_union(a.lens, b.lens);
  vector<size_t> tmp_lens_num(tmp_lens.size());
  auto tmp_cwordsp = tmp_cwords.data();
  auto a_cwordsp = a.cwords.data();
  auto b_cwordsp = b.cwords.data();
  size_t aidx = 0;
  size_t bidx = 0;
  auto crnt_tmp_cwordsp = tmp_cwordsp;
  auto crnt_a_cwordsp = a_cwordsp;
  auto crnt_b_cwordsp = b_cwordsp;
  auto a_lens_size = a.lens.size();
  auto b_lens_size = b.lens.size();
  for(size_t i = 0; i < tmp_lens.size(); i++) {
    size_t crnt_lens_num = 0;
    if(aidx < a_lens_size && a.lens[aidx] == tmp_lens[i]) {
      auto total = a.lens_num[aidx] * a.lens[aidx];
      for(size_t j = 0; j < total; j++) {
        crnt_tmp_cwordsp[j] = crnt_a_cwordsp[j];
      }
      crnt_tmp_cwordsp += total;
      crnt_a_cwordsp += total;
      crnt_lens_num += a.lens_num[aidx];
      aidx++;
    }
    if(bidx < b_lens_size && b.lens[bidx] == tmp_lens[i]) {
      auto total = b.lens_num[bidx] * b.lens[bidx];
      for(size_t j = 0; j < total; j++) {
        crnt_tmp_cwordsp[j] = crnt_b_cwordsp[j];
      }
      crnt_tmp_cwordsp += total;
      crnt_b_cwordsp += total;
      crnt_lens_num += b.lens_num[bidx];
      bidx++;
    }
    tmp_lens_num[i] = crnt_lens_num;
  }
  dict ret;
  unique_words(tmp_cwords, tmp_lens, tmp_lens_num,
               ret.cwords, ret.lens, ret.lens_num);
  return ret;
}

std::vector<size_t> dict::lookup(const compressed_words& cw) const {
  auto& cwords_lookup = cw.cwords;
  auto& lens_lookup = cw.lens;
  auto& lens_num_lookup = cw.lens_num;
  if(cwords_lookup.size() == 0) return vector<size_t>();
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

  // to surpress "may be used uninitialized" warning
  for(int i = 0; i < DICT_VLEN; i++) {
    left[i] = 0;
    right[i] = 0;
    mid[i] = 0;
    is_valid[i] = 0;
    is_eq[i] = 0;
    is_lt[i] = 0;
    retbuf[i] = 0;
    packed_lookup_1st[i] = 0;
  }

  auto crnt_cwords_lookupp = cwords_lookup.data();
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
      crnt_cwords_lookupp += len * to_lookup_size;
      crnt_retp += to_lookup_size;
      continue;
    }
    size_t total = 0;
    for(size_t i = 0; i < lenpos; i++) total += lens[i] * lens_num[i];
    auto crnt_cwordsp = cwords.data() + total;
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
        packed_lookup_1st[j] = crnt_cwords_lookupp[len * j];
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
            auto packed_dict = crnt_cwordsp[len * mid[j]];
            is_eq[j] = (packed_dict == packed_lookup_1st[j]);
            is_lt[j] = (packed_dict < packed_lookup_1st[j]);
          }
        }
        for(size_t l = 1; l < len; l++) {
#pragma _NEC ivdep
          for(size_t j = 0; j < DICT_VLEN; j++) {
            if(is_valid[j]) {
              auto packed_lookup = crnt_cwords_lookupp[len * j + l];
              auto packed_dict = crnt_cwordsp[len * mid[j] + l];
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
      crnt_cwords_lookupp += len * DICT_VLEN;
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

    // to surpress "may be used uninitialized" warning
    for(int i = 0; i < DICT_VLEN; i++) {
      left2[i] = 0;
      right2[i] = 0;
      mid2[i] = 0;
      is_valid2[i] = 0;
      is_eq2[i] = 0;
      is_lt2[i] = 0;
      retbuf2[i] = 0;
      packed_lookup_1st2[i] = 0;
    }

    for(size_t j = 0; j < remain; j++) {
      left2[j] = 0;
      right2[j] = lens_num[lenpos] - 1;
      is_valid2[j] = 1;
    }
    for(size_t j = 0; j < remain; j++) {
      packed_lookup_1st2[j] = crnt_cwords_lookupp[len * j];
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
          auto packed_dict = crnt_cwordsp[len * mid2[j]];
          is_eq2[j] = (packed_dict == packed_lookup_1st2[j]);
          is_lt2[j] = (packed_dict < packed_lookup_1st2[j]);
        }
      }
      for(size_t l = 1; l < len; l++) {
#pragma _NEC ivdep
        for(size_t j = 0; j < remain; j++) {
          if(is_valid2[j]) {
            auto packed_lookup = crnt_cwords_lookupp[len * j + l];
            auto packed_dict = crnt_cwordsp[len * mid2[j] + l];
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
    crnt_cwords_lookupp += len * remain;
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

words dict::index_to_words(const std::vector<size_t>& idx) const {
  auto num_words_ = num_words();
  auto idx_size = idx.size();
  if(num_words_ < idx_size) { // small dictionary
    vector<size_t> order(num_words_);
    auto orderp = order.data();
    for(size_t i = 0; i < num_words_; i++) orderp[i] = i;
    auto ws = decompress_compressed_words(cwords, lens, lens_num, order);
    auto idx_size = idx.size();
    auto idxp = idx.data();
    vector<size_t> new_starts(idx_size), new_lens(idx_size);
    auto new_startsp = new_starts.data();
    auto new_lensp = new_lens.data();
    auto startsp = ws.starts.data();
    auto lensp = ws.lens.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < idx_size; i++) {
      new_startsp[i] = startsp[idxp[i]];
      new_lensp[i] = lensp[idxp[i]];
    }
    ws.starts.swap(new_starts);
    ws.lens.swap(new_lens);
    return ws;
  } else { // large dictionary, pick up only used part
    auto ht = unique_hashtable<size_t,int>(idx);
    auto uniq = ht.all_keys();
    ht.clear();
    radix_sort(uniq);
    auto uniq_size = uniq.size();
    std::vector<size_t> order(uniq_size);
    auto orderp = order.data();
    for(size_t i = 0; i < uniq_size; i++) orderp[i] = i;
    auto ht2 = unique_hashtable<size_t, size_t>(uniq, order);
    auto conv = ht2.lookup(idx);
    ht2.clear();
    compressed_words cw;
    cw.order.resize(num_words_);
    auto cworderp = cw.order.data();
    for(size_t i = 0; i < num_words_; i++) cworderp[i] = i;
    // because this is const method...
    auto& cwords_ = const_cast<std::vector<uint64_t>&>(cwords);
    auto& lens_ = const_cast<std::vector<size_t>&>(lens);
    auto& lens_num_ = const_cast<std::vector<size_t>&>(lens_num);
    cw.cwords.swap(cwords_);
    cw.lens.swap(lens_);
    cw.lens_num.swap(lens_num_);
    auto newcw = cw.extract(uniq);
    cw.cwords.swap(cwords_);
    cw.lens.swap(lens_);
    cw.lens_num.swap(lens_num_);
    auto ws = newcw.decompress();
    auto conv_size = conv.size();
    auto convp = conv.data();
    vector<size_t> new_starts(conv_size), new_lens(conv_size);
    auto new_startsp = new_starts.data();
    auto new_lensp = new_lens.data();
    auto startsp = ws.starts.data();
    auto lensp = ws.lens.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < conv_size; i++) {
      new_startsp[i] = startsp[convp[i]];
      new_lensp[i] = lensp[convp[i]];
    }
    ws.starts.swap(new_starts);
    ws.lens.swap(new_lens);
    return ws;
  }
}

words dict::decompress() const {
  auto num_words_ = num_words();
  vector<size_t> order(num_words_);
  auto orderp = order.data();
  for(size_t i = 0; i < num_words_; i++) orderp[i] = i;
  return decompress_compressed_words(cwords, lens, lens_num, order);
}  

}
