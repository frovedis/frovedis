#ifndef MAKE_DFTABLE_LOADTEXT_HPP
#define MAKE_DFTABLE_LOADTEXT_HPP

#include "dftable.hpp"
#include "../text/words.hpp"
#include "../text/parsefloat.hpp"
#include "../text/infer_dtype.hpp"

namespace frovedis {

dftable make_dftable_loadtext(const std::string& filename,
                              const std::vector<std::string>& types,
                              const std::vector<std::string>& names,
                              int separator = ',',
                              const std::string& nullstr = "NULL",
                              bool is_crlf = false, 
                              bool to_separate = true,
                              bool to_keep_order = true,
                              double separate_mb = 1024);

dftable make_dftable_loadtext(const std::string& filename,
                              const std::vector<std::string>& types,
                              int separator = ',',
                              const std::string& nullstr = "NULL",
                              bool is_crlf = false,
                              bool to_separate = true,
                              bool to_keep_order = true,
                              double separate_mb = 1024);

dftable make_dftable_loadtext_infertype(const std::string& filename,
                                        const std::vector<std::string>& names,
                                        int separator = ',',
                                        const std::string& nullstr = "NULL",
                                        size_t rows_to_see = 1024,
                                        bool is_crlf = false,
                                        bool to_separate = true,
                                        bool to_keep_order = true,
                                        double separate_mb = 1024);

dftable make_dftable_loadtext_infertype(const std::string& filename,
                                        int separator = ',',
                                        const std::string& nullstr = "NULL",
                                        size_t rows_to_see = 1024,
                                        bool is_crlf = false,
                                        bool to_separate = true,
                                        bool to_keep_order = true,
                                        double separate_mb = 1024);

std::vector<size_t> extract_nulls(std::vector<int>& chars,
                                  std::vector<size_t>& starts,
                                  std::vector<size_t>& lens,
                                  const std::string& nullstr,
                                  bool to_replace = true,
                                  int replace_char = '0');

template <class T>
std::vector<T> words_to_number(words& ws,
                               const std::vector<size_t>& line_starts,
                               size_t col,
                               const std::string& nullstr,
                               std::vector<size_t>& nulls,
                               bool skip_head) {
  auto startsp = ws.starts.data();
  auto lensp = ws.lens.data();
  auto num_words = ws.starts.size();
  auto num_rows = line_starts.size();
  if(num_rows == 0) return std::vector<T>();
  auto num_cols = num_words / num_rows;
  if(num_cols < col || num_words % num_rows != 0) 
    throw std::runtime_error("invalid number of colums, types, or names");
  if(get_selfid() == 0 && skip_head) {
    startsp += num_cols;
    lensp += num_cols;
    num_rows--;
    if(num_rows == 0) return std::vector<T>();
  }
  std::vector<size_t> new_starts(num_rows), new_lens(num_rows);
  auto new_startsp = new_starts.data();
  auto new_lensp = new_lens.data();
  for(size_t i = 0; i < num_rows; i++) {
    new_startsp[i] = startsp[i * num_cols + col];
    new_lensp[i] = lensp[i * num_cols + col];
  }
  nulls = extract_nulls(ws.chars, new_starts, new_lens, nullstr);
  auto ret = parsenumber<T>(ws.chars, new_starts, new_lens);
  auto nullsp = nulls.data();
  auto nulls_size = nulls.size();
  auto retp = ret.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < nulls_size; i++) {
    retp[nullsp[i]] = std::numeric_limits<T>::max();
  }
  return ret;
}

}

#endif
