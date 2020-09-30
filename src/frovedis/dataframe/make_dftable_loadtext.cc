#include <fstream>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

#include "make_dftable_loadtext.hpp"
#include "../text/load_csv.hpp"

namespace frovedis {

using namespace std;

// note that this modifies chars! (for performance)
// if words share the same chars, this would not work
vector<size_t> extract_nulls(std::vector<int>& chars,
                             std::vector<size_t>& starts,
                             std::vector<size_t>& lens,
                             const std::string& nullstr,
                             bool to_replace,
                             int replace_char) {
  auto nulls = like(chars, starts, lens, nullstr);
  auto nulls_size = nulls.size();
  if(nulls_size == 0) return vector<size_t>();
  else {
    if(to_replace) {
      if(nullstr.size() == 0)
        throw std::runtime_error("size of string for null is zero");
      auto nullsp = nulls.data();
      auto startsp = starts.data();
      auto lensp = lens.data();
      auto charsp = chars.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
      for(size_t i = 0; i < nulls_size; i++) {
        charsp[startsp[nullsp[i]]] = replace_char;
        lensp[nullsp[i]] = 1;
      }
    }
    return nulls;
  }
}

vector<string> words_to_string(words& ws,
                               const vector<size_t>& line_starts,
                               size_t col,
                               const string& nullstr,
                               vector<size_t>& nulls,
                               bool skip_head) {
  auto startsp = ws.starts.data();
  auto lensp = ws.lens.data();
  auto num_words = ws.starts.size();
  auto num_rows = line_starts.size();
  if(num_rows == 0) return vector<string>();
  auto num_cols = num_words / num_rows;
  if(num_cols < col || num_words % num_rows != 0) 
    throw runtime_error("invalid number of colums, types, or names");
  if(get_selfid() == 0 && skip_head) {
    startsp += num_cols;
    lensp += num_cols;
    num_rows--;
    if(num_rows == 0) return vector<string>();
  }
  std::vector<size_t> new_starts(num_rows), new_lens(num_rows);
  auto new_startsp = new_starts.data();
  auto new_lensp = new_lens.data();
  for(size_t i = 0; i < num_rows; i++) {
    new_startsp[i] = startsp[i * num_cols + col];
    new_lensp[i] = lensp[i * num_cols + col];
  }

  nulls = extract_nulls(ws.chars, new_starts, new_lens, nullstr, false);
  
  std::vector<size_t> concat_starts;
  auto concat = concat_words(ws.chars, new_starts, new_lens, "",
                             concat_starts);
  auto concat_str = int_to_char(concat);
  vector<string> ret(num_rows);
  auto concat_startsp = concat_starts.data();
  for(size_t i = 0; i < num_rows; i++) {
    ret[i] = concat_str.substr(concat_startsp[i], new_lensp[i]);
  }
  return ret;
}

compressed_words extract_compressed_words(words& ws,
                                          const vector<size_t>& line_starts,
                                          size_t col,
                                          const string& nullstr,
                                          vector<size_t>& nulls,
                                          bool skip_head) {
  auto startsp = ws.starts.data();
  auto lensp = ws.lens.data();
  auto num_words = ws.starts.size();
  auto num_rows = line_starts.size();
  if(num_rows == 0) return compressed_words();
  auto num_cols = num_words / num_rows;
  if(num_cols < col || num_words % num_rows != 0) 
    throw runtime_error("invalid number of colums, types, or names");
  if(get_selfid() == 0 && skip_head) {
    startsp += num_cols;
    lensp += num_cols;
    num_rows--;
    if(num_rows == 0) return compressed_words();
  }
  std::vector<size_t> new_starts(num_rows), new_lens(num_rows);
  auto new_startsp = new_starts.data();
  auto new_lensp = new_lens.data();
  for(size_t i = 0; i < num_rows; i++) {
    new_startsp[i] = startsp[i * num_cols + col];
    new_lensp[i] = lensp[i * num_cols + col];
  }
  nulls = extract_nulls(ws.chars, new_starts, new_lens, nullstr, false);
  compressed_words ret;
  compress_words(ws.chars, new_starts, new_lens, ret.cwords,
                 ret.lens, ret.lens_num, ret.order);
  return ret;
}

std::vector<datetime_t> words_to_datetime(words& ws,
                                          const std::vector<size_t>& line_starts,
                                          size_t col,
                                          const std::string& nullstr,
                                          std::vector<size_t>& nulls,
                                          bool skip_head,
                                          const std::string& fmt) {
  auto startsp = ws.starts.data();
  auto lensp = ws.lens.data();
  auto num_words = ws.starts.size();
  auto num_rows = line_starts.size();
  if(num_rows == 0) return std::vector<datetime_t>();
  auto num_cols = num_words / num_rows;
  if(num_cols < col || num_words % num_rows != 0) 
    throw std::runtime_error("invalid number of colums, types, or names");
  if(get_selfid() == 0 && skip_head) {
    startsp += num_cols;
    lensp += num_cols;
    num_rows--;
    if(num_rows == 0) return std::vector<datetime_t>();
  }
  std::vector<size_t> new_starts(num_rows), new_lens(num_rows);
  auto new_startsp = new_starts.data();
  auto new_lensp = new_lens.data();
  for(size_t i = 0; i < num_rows; i++) {
    new_startsp[i] = startsp[i * num_cols + col];
    new_lensp[i] = lensp[i * num_cols + col];
  }
  nulls = extract_nulls(ws.chars, new_starts, new_lens, nullstr);
  auto ret = parsedatetime(ws.chars, new_starts, new_lens, fmt);
  auto nullsp = nulls.data();
  auto nulls_size = nulls.size();
  auto retp = ret.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < nulls_size; i++) {
    retp[nullsp[i]] = std::numeric_limits<datetime_t>::max();
  }
  return ret;
}

std::shared_ptr<dfcolumn>
parse_words(node_local<words>& ws,
            node_local<vector<size_t>>& line_starts,
            const string& type,
            size_t col,
            const string& nullstr, 
            bool skip_head) {
  auto nl_col = broadcast(col);
  auto nl_skip_head = broadcast(skip_head);
  auto nl_nullstr = broadcast(nullstr);
  auto nulls = make_node_local_allocate<vector<size_t>>();
  time_spent t(DEBUG);
  if(type == "string") {
    auto nl = ws.map(words_to_string, line_starts, nl_col, nl_nullstr, nulls,
                     nl_skip_head);
    t.show("parse_words, words_to_string: ");
    return make_shared<typed_dfcolumn<string>>(move(nl), move(nulls));
  } else if(type == "dic_string") {
    auto nl = ws.map(extract_compressed_words, line_starts, nl_col, nl_nullstr,
                     nulls, nl_skip_head);
    t.show("parse_words, extract_compressed_words: ");
    return make_shared<typed_dfcolumn<dic_string>>(move(nl), move(nulls));
  } else if(type == "raw_string") {
    auto nl = ws.map(extract_compressed_words, line_starts, nl_col, nl_nullstr,
                     nulls, nl_skip_head);
    t.show("parse_words, extract_compressed_words: ");
    return make_shared<typed_dfcolumn<raw_string>>(move(nl), move(nulls));
  } else if(type == "float") {
    auto nl = ws.map(words_to_number<float>, line_starts, nl_col, nl_nullstr,
                     nulls, nl_skip_head);
    t.show("parse_words, words_to_number: ");
    return make_shared<typed_dfcolumn<float>>(move(nl), move(nulls));
  } else if(type == "double") {
    auto nl = ws.map(words_to_number<double>, line_starts, nl_col, nl_nullstr,
                     nulls, nl_skip_head);
    t.show("parse_words, words_to_number: ");
    return make_shared<typed_dfcolumn<double>>(move(nl), move(nulls));
  } else if(type == "int") {
    auto nl = ws.map(words_to_number<int>, line_starts, nl_col, nl_nullstr,
                     nulls, nl_skip_head);
    t.show("parse_words, words_to_number: ");
    return make_shared<typed_dfcolumn<int>>(move(nl), move(nulls));
  } else if(type == "long") {
    auto nl = ws.map(words_to_number<long>, line_starts, nl_col, nl_nullstr,
                     nulls, nl_skip_head);
    t.show("parse_words, words_to_number: ");
    return make_shared<typed_dfcolumn<long>>(move(nl), move(nulls));
  } else if(type == "unsigned int") {
    auto nl = ws.map(words_to_number<unsigned int>, line_starts, nl_col,
                     nl_nullstr, nulls, nl_skip_head);
    t.show("parse_words, words_to_number: ");
    return make_shared<typed_dfcolumn<unsigned int>>(move(nl), move(nulls));
  } else if(type == "unsigned long") {
    auto nl = ws.map(words_to_number<unsigned long>, line_starts, nl_col,
                     nl_nullstr, nulls, nl_skip_head);
    t.show("parse_words, words_to_number: ");
    return make_shared<typed_dfcolumn<unsigned long>>(move(nl), move(nulls));
  } else if (type.find("datetime:") == 0) {
    auto fmt = type.substr(9);
    auto nl = ws.map(words_to_datetime, line_starts, nl_col,
                     nl_nullstr, nulls, nl_skip_head, broadcast(fmt));
    t.show("parse_words, words_to_datetime: ");
    return make_shared<typed_dfcolumn<datetime>>(move(nl), move(nulls));
  } else throw runtime_error("unknown type: " + type);
}

vector<string> get_names(const words& ws, const vector<size_t>& line_starts) {
  if(get_selfid() == 0) {
    auto num_rows = line_starts.size();
    if(num_rows == 0) throw runtime_error("zero sized file");
    auto num_words = ws.starts.size();
    auto num_cols = num_words / num_rows;
    vector<string> ret;
    for(size_t i = 0; i < num_cols; i++) {
      string tmp;
      auto start = ws.starts[i];
      auto len = ws.lens[i];
      for(size_t j = start; j < start + len; j++) {
        tmp.push_back(ws.chars[j]);
      }
      ret.push_back(tmp);
    }
    return ret;
  } else {
    return vector<string>();
  }
}

dftable make_dftable_loadtext(const string& filename,
                              const vector<string>& types,
                              const vector<string>& names,
                              int separator,
                              const string& nullstr,
                              bool is_crlf) {
  time_spent t(DEBUG);
  auto line_starts_byword = make_node_local_allocate<std::vector<size_t>>();
  auto ws = load_csv(filename, line_starts_byword, is_crlf, false, separator);
  t.show("make_dftable_loadtext::load_csv: ");
  auto num_cols = types.size();
  if(names.size() != num_cols)
    throw runtime_error("invalid number of colums, types, or names");
  dftable ret;
  for(size_t i = 0; i < num_cols; i++) {
    auto col = parse_words(ws, line_starts_byword, types[i], i, nullstr, false);
    ret.append_column(names[i], col);
    t.show(std::string("make_dftable_loadtext::parse_words, ")
           + names[i] + ": ");
  }
  return ret;
}

dftable make_dftable_loadtext(const string& filename,
                              const vector<string>& types,
                              int separator,
                              const string& nullstr,
                              bool is_crlf) {
  auto line_starts_byword = make_node_local_allocate<std::vector<size_t>>();
  auto ws = load_csv(filename, line_starts_byword, is_crlf, false, separator);
  auto names = ws.map(get_names, line_starts_byword).get(0);
  auto num_cols = types.size();
  if(names.size() != num_cols)
    throw runtime_error("invalid number of colums, types, or names");
  dftable ret;
  for(size_t i = 0; i < num_cols; i++) {
    auto col = parse_words(ws, line_starts_byword, types[i], i, nullstr, true);
    ret.append_column(names[i], col);
  }
  return ret;
}

inferred_dtype infer_dtype_loadtext(words& ws,
                                    const std::vector<size_t>& line_starts,
                                    size_t col,
                                    const std::string& nullstr,
                                    size_t rows_to_see,
                                    bool skip_head) {
  auto startsp = ws.starts.data();
  auto lensp = ws.lens.data();
  auto num_words = ws.starts.size();
  auto num_rows = line_starts.size();
  // int is the smallest set
  if(num_rows == 0) return inferred_dtype::inferred_dtype_int;
  auto num_cols = num_words / num_rows;
  if(num_cols < col || num_words % num_rows != 0) 
    throw std::runtime_error("invalid number of colums, types, or names");
  if(get_selfid() == 0 && skip_head) {
    startsp += num_cols;
    lensp += num_cols;
    num_rows--;
    if(num_rows == 0) return inferred_dtype::inferred_dtype_int;
  }
  rows_to_see = std::min(num_rows, rows_to_see);
  std::vector<size_t> new_starts(rows_to_see), new_lens(rows_to_see);
  auto new_startsp = new_starts.data();
  auto new_lensp = new_lens.data();
  for(size_t i = 0; i < rows_to_see; i++) {
    new_startsp[i] = startsp[i * num_cols + col];
    new_lensp[i] = lensp[i * num_cols + col];
  }
  auto nulls = extract_nulls(ws.chars, new_starts, new_lens, nullstr, false);
  if(nulls.size() != 0) {
    std::vector<size_t> iota(rows_to_see);
    auto iotap = iota.data();
    for(size_t i = 0; i < rows_to_see; i++) {
      iotap[i] = i;
    }
    auto exclude_nulls = set_difference(iota, nulls);
    size_t exclude_nulls_size = exclude_nulls.size();
    std::vector<size_t> new_starts_tmp(exclude_nulls_size),
      new_lens_tmp(exclude_nulls_size);
    auto new_starts_tmpp = new_starts_tmp.data();
    auto new_lens_tmpp = new_lens_tmp.data();
    auto exclude_nullsp = exclude_nulls.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < exclude_nulls_size; i++) {
      new_starts_tmpp[i] = new_startsp[exclude_nullsp[i]];
      new_lens_tmpp[i] = new_lensp[exclude_nullsp[i]];
    }
    new_starts.swap(new_starts_tmp);
    new_lens.swap(new_lens_tmp);
  }
  return infer_dtype(ws.chars, new_starts, new_lens, rows_to_see);
}

inferred_dtype reduce_inferred_dtype(inferred_dtype a, inferred_dtype b) {
  if(a == inferred_dtype::inferred_dtype_int) {
    if(b == inferred_dtype::inferred_dtype_int)
      return inferred_dtype::inferred_dtype_int;
    else if(b == inferred_dtype::inferred_dtype_float)
      return inferred_dtype::inferred_dtype_float;
    else
      return inferred_dtype::inferred_dtype_string;
  } else if(a == inferred_dtype::inferred_dtype_float) {
    if(b == inferred_dtype::inferred_dtype_int ||
       b == inferred_dtype::inferred_dtype_float)
      return inferred_dtype::inferred_dtype_float;
    else
      return inferred_dtype::inferred_dtype_string;
  } else return inferred_dtype::inferred_dtype_string;
}

dftable make_dftable_loadtext_infertype(const string& filename,
                                        const vector<string>& names,
                                        int separator,
                                        const string& nullstr,
                                        size_t rows_to_see,
                                        bool is_crlf) {
  time_spent t(DEBUG);
  auto line_starts_byword = make_node_local_allocate<std::vector<size_t>>();
  auto ws = load_csv(filename, line_starts_byword, is_crlf, false, separator);
  t.show("make_dftable_loadtext::load_csv: ");
  auto num_cols = names.size();
  dftable ret;
  auto brows_to_see = broadcast(rows_to_see);
  auto bnullstr = broadcast(nullstr);
  auto bfalse = broadcast(false);
  for(size_t i = 0; i < num_cols; i++) {
    auto type = ws.map(infer_dtype_loadtext, line_starts_byword,
                       broadcast(i), bnullstr, brows_to_see, bfalse).
      reduce(reduce_inferred_dtype);
    string stype;
    if(type == inferred_dtype::inferred_dtype_int) stype = "long";
    else if(type == inferred_dtype::inferred_dtype_float) stype = "double";
    else stype = "dic_string";
    auto col = parse_words(ws, line_starts_byword, stype, i, nullstr, false);
    ret.append_column(names[i], col);
    t.show(std::string("make_dftable_loadtext::parse_words, ")
           + names[i] + ": ");
  }
  return ret;
}

dftable make_dftable_loadtext_infertype(const string& filename,
                                        int separator,
                                        const string& nullstr,
                                        size_t rows_to_see,
                                        bool is_crlf) {
  auto line_starts_byword = make_node_local_allocate<std::vector<size_t>>();
  auto ws = load_csv(filename, line_starts_byword, is_crlf, false, separator);
  auto names = ws.map(get_names, line_starts_byword).get(0);
  auto num_cols = names.size();
  dftable ret;
  auto brows_to_see = broadcast(rows_to_see);
  auto bnullstr = broadcast(nullstr);
  auto btrue = broadcast(true);
  for(size_t i = 0; i < num_cols; i++) {
    auto type = ws.map(infer_dtype_loadtext, line_starts_byword,
                       broadcast(i), bnullstr, brows_to_see, btrue).
      reduce(reduce_inferred_dtype);
    string stype;
    if(type == inferred_dtype::inferred_dtype_int) stype = "long";
    else if(type == inferred_dtype::inferred_dtype_float) stype = "double";
    else stype = "dic_string";
    auto col = parse_words(ws, line_starts_byword, stype, i, nullstr, true);
    ret.append_column(names[i], col);
  }
  return ret;
}



}
