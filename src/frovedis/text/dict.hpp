#ifndef DICT_HPP
#define DICT_HPP

#if defined(__ve__) || defined(_SX)
#define DICT_VLEN 256
#else
//#define DICT_VLEN 1
#define DICT_VLEN 4
#endif

#include "../core/mpi_rpc.hpp"
#include "../core/serializehelper.hpp"
#include "../core/radix_sort.hpp"
#include "../core/set_operations.hpp"
#include "../matrix/crs_matrix.hpp"
#include "words.hpp"
#include <limits>

namespace frovedis {

void unique_words(const std::vector<uint64_t>& compressed,
                  const std::vector<size_t>& compressed_lens,
                  const std::vector<size_t>& compressed_lens_num,
                  std::vector<uint64_t>& unique_compressed,
                  std::vector<size_t>& unique_compressed_lens,
                  std::vector<size_t>& unique_compressed_lens_num);

void compress_words(const std::vector<int>& v,
                    const std::vector<size_t>& starts,
                    const std::vector<size_t>& lens,
                    std::vector<uint64_t>& compressed,
                    std::vector<size_t>& compressed_lens,
                    std::vector<size_t>& compressed_lens_num,
                    std::vector<size_t>& order);

void print_compressed_words(const std::vector<uint64_t>& compressed,
                            const std::vector<size_t>& compressed_lens,
                            const std::vector<size_t>& compressed_lens_num,
                            const std::vector<size_t>& order);

void print_compressed_words(const std::vector<uint64_t>& compressed,
                            const std::vector<size_t>& compressed_lens,
                            const std::vector<size_t>& compressed_lens_num,
                            bool print_idx = false);

struct compressed_words {
  std::vector<uint64_t> words; // compressed
  std::vector<size_t> lens;
  std::vector<size_t> lens_num;
  std::vector<size_t> order;
  
  struct words decompress();
  void lexical_sort();
  void print() const {print_compressed_words(words, lens, lens_num, order);}
  size_t num_words() const {return order.size();}
  
  SERIALIZE(words, lens, lens_num, order)
};

compressed_words merge_compressed_words(const compressed_words& a,
                                        const compressed_words& b);

compressed_words make_compressed_words(const words&);

struct dict {
  std::vector<uint64_t> words; // compressed
  std::vector<size_t> lens;
  std::vector<size_t> lens_num;

  void print() const {print_compressed_words(words, lens, lens_num, true);}
  std::vector<size_t> lookup(const compressed_words& cw) const;
  template <class T, class I = size_t, class O = size_t>
  crs_matrix_local<T, I, O>
  lookup_to_crs_matrix(const compressed_words& cw,
                       const std::vector<size_t>& doc_start_word) const;

  size_t num_words() const;
  
  SERIALIZE(words, lens, lens_num)
};

dict make_dict(const words& ws);
dict make_dict(const compressed_words& comp);
// same as above: to use from map
// pointer to overloaded function becomes ambiguous
dict make_dict_from_words(const words& ws);
dict make_dict_from_compressed(const compressed_words& comp);

dict merge_dict(const dict& a, const dict& b);

template <class T, class I, class O>
crs_matrix_local<T,I,O>
dict::lookup_to_crs_matrix(const compressed_words& cw,
                           const std::vector<size_t>& doc_start_word) const {
  auto lookedup = lookup(cw);
  auto lookedupp = lookedup.data();
  auto num_words = lookedup.size();
  auto num_docs = doc_start_word.size();
  if(num_docs == 0) {
    crs_matrix_local<T,I,O> ret;
    ret.local_num_col = this->num_words();
    return ret;
  }
  std::vector<size_t> doc_idx(num_words);
  auto doc_idxp = doc_idx.data();
  auto crnt_doc_idxp = doc_idxp;
  auto doc_start_wordp = doc_start_word.data();
  for(size_t i = 0; i < num_docs-1; i++) {
    auto size = doc_start_wordp[i+1] - doc_start_wordp[i];
    for(size_t j = 0; j < size; j++) {
      crnt_doc_idxp[j] = i;
    }
    crnt_doc_idxp += size;
  }
  auto size = num_words - doc_start_word[num_docs-1];
  for(size_t j = 0; j < size; j++) {
    crnt_doc_idxp[j] = num_docs-1;
  }
  radix_sort(lookedup, doc_idx);
  radix_sort(doc_idx, lookedup);
  auto sep = set_separate(lookedup);
  auto sep2 = set_union(sep, doc_start_word);
  auto sep2p = sep2.data();

  auto nnztmp = sep2.size() - 1;
  std::vector<T> valtmp(nnztmp);
  std::vector<I> idxtmp(nnztmp);
  std::vector<O> offtmp(num_docs+1);
  auto valtmpp = valtmp.data();
  auto idxtmpp = idxtmp.data();
  auto offtmpp = offtmp.data();
  for(size_t i = 0; i < nnztmp; i++) {
    valtmpp[i] = sep2p[i+1] - sep2p[i];
    idxtmpp[i] = lookedupp[sep2p[i]];
  }
  lower_bound(sep2p, nnztmp, doc_start_wordp, num_docs, offtmpp);
  offtmpp[num_docs] = nnztmp;
  
  size_t not_found_count = 0;
  auto NOT_FOUND = std::numeric_limits<size_t>::max();
  for(size_t i = 0; i < num_docs; i++) {
    auto size = offtmpp[i+1] - offtmpp[i];
    if(size > 0 && idxtmpp[offtmpp[i+1]-1] == NOT_FOUND) not_found_count++;
  }

  auto nnz = nnztmp - not_found_count;
  crs_matrix_local<T,I,O> ret;
  ret.val.resize(nnz);
  ret.idx.resize(nnz);
  ret.off.resize(num_docs+1);
  auto valp = ret.val.data();
  auto idxp = ret.idx.data();
  auto offp = ret.off.data();
  auto crnt_src_valp = valtmpp;
  auto crnt_src_idxp = idxtmpp;
  auto crnt_dst_valp = valp;
  auto crnt_dst_idxp = idxp;

  for(size_t i = 0; i < num_docs; i++) {
    auto orgsize = offtmpp[i+1] - offtmpp[i];
    auto size = (orgsize > 0 && crnt_src_idxp[orgsize-1] == NOT_FOUND) ?
      orgsize - 1 : orgsize;
    for(size_t j = 0; j < size; j++) {
      crnt_dst_valp[j] = crnt_src_valp[j];
      crnt_dst_idxp[j] = crnt_src_idxp[j];
    }
    crnt_src_valp += orgsize;
    crnt_src_idxp += orgsize;
    crnt_dst_valp += size;
    crnt_dst_idxp += size;
    offp[i+1] = offp[i] + size;
  }
  ret.local_num_row = num_docs;
  ret.local_num_col = this->num_words();

  return ret;
}

}

#endif

