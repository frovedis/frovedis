#include "dftable_to_words.hpp"

namespace frovedis {

using namespace std;

struct put_words_ith {
  put_words_ith(){}
  put_words_ith(size_t i) : i(i) {}
  void operator()(vector<words>& vw, words& w) {
    // or move assignment?
    vw[i].chars.swap(w.chars);
    vw[i].starts.swap(w.starts);
    vw[i].lens.swap(w.lens);
  }
  size_t i;
  SERIALIZE(i)
};

words dftable_to_words_helper(vector<words>& vw) {
  auto num_col = vw.size();
  auto ret = merge_multi_words(vw);
  auto starts_size = ret.starts.size();
  auto startsp = ret.starts.data();
  auto lensp = ret.lens.data();
  vector<size_t> new_starts(starts_size), new_lens(starts_size);
  auto new_startsp = new_starts.data();
  auto new_lensp = new_lens.data();
  auto num_row = starts_size / num_col;
  if(starts_size % num_col != 0) 
    throw runtime_error("dftable_to_words_helper: incorrect size of row or col");
#pragma _NEC novector
  for(size_t i = 0; i < num_col; i++) {
    for(size_t j = 0; j < num_row; j++) {
      new_startsp[j * num_col + i] = startsp[i * num_row + j];
      new_lensp[j * num_col + i] = lensp[i * num_row + j];
    }
  }
  ret.starts.swap(new_starts);
  ret.lens.swap(new_lens);
  return ret;
}

node_local<words>
dftable_to_words(dftable_base& table, bool quote_escape,
                 const std::string& nullstr) {
  auto cols = table.columns();
  auto num_col = table.num_col();
  vector<words> work(num_col);
  auto nl_work = broadcast(work);
  for(size_t i = 0; i < cols.size(); i++) {
    auto ws = table.column(cols[i])->as_words(quote_escape, nullstr);
    nl_work.mapv(put_words_ith(i), ws);
  }
  return nl_work.map(dftable_to_words_helper);
}

}
