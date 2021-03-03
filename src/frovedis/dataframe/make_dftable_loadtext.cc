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

int to_be_parsed(const std::vector<int>& colid, 
                 size_t& curpos, size_t i) {
  if (colid.empty()) return true; // in case of empty colid, parse all
  else {
   auto ret = (colid[curpos] == i);
   if(ret) curpos++;
   return ret;
  }
}

// mange duplicate columns like in pandas read_csv(...)
std::vector<std::string>
mangle_duplicate_column_names(const vector<string>& names) {
  auto sz = names.size();
  std::vector<std::string> ret(sz);
  std::map<std::string, int> name_count;
  for (size_t i = 0; i < sz; ++i) {
    auto& e = names[i];
    if(name_count.find(e) != name_count.end()) {
      ret[i] = e + "." + std::to_string(name_count[e]);
      name_count[e] += 1;
    }
    else {
      ret[i] = e;
      name_count[e] = 1;
    }
  }
  return ret;
}

dftable make_dftable_loadtext_helper(const string& filename,
                                     const vector<string>& types,
                                     const vector<string>& names,
                                     int separator,
                                     const string& nullstr,
                                     bool is_crlf,
                                     ssize_t start, ssize_t& end,
                                     const vector<int>& usecols,
                                     bool mangle_dupe_cols) {
  time_spent t(DEBUG);
  auto line_starts_byword = make_node_local_allocate<std::vector<size_t>>();
  auto ws = load_csv_separate(filename, line_starts_byword, start, end,
                              is_crlf, false, separator);
  t.show("make_dftable_loadtext::load_csv: ");
  auto num_cols = types.size();
  if(names.size() != num_cols)
    throw runtime_error("invalid number of colums, types, or names");
  auto& cnames = const_cast<vector<string>&> (names);
  if (mangle_dupe_cols) cnames = mangle_duplicate_column_names(names); 
  dftable ret;
  auto sorted_colid = vector_sort(usecols);
  size_t curpos = 0;
  for(size_t i = 0; i < num_cols; i++) {
    if(to_be_parsed(sorted_colid, curpos, i)) {
      auto col = parse_words(ws, line_starts_byword, types[i], i, nullstr, false);
      ret.append_column(cnames[i], col);
      t.show(std::string("make_dftable_loadtext::parse_words, ")
               + cnames[i] + ": ");
    }
  }
  return ret;
}

dftable make_dftable_loadtext(const string& filename,
                              const vector<string>& types,
                              const vector<string>& names,
                              int separator,
                              const string& nullstr,
                              bool is_crlf,
                              bool to_separate,
                              bool to_keep_order,
                              double separate_mb,
                              const vector<int>& usecols,
                              bool mangle_dupe_cols) {
  if(to_separate) {
    int fd = ::open(filename.c_str(), O_RDONLY);
    if(fd == -1) {
      throw std::runtime_error("open failed: " + std::string(strerror(errno)));
    }
    struct stat sb;
    if(stat(filename.c_str(), &sb) != 0) {
      ::close(fd);
      throw std::runtime_error("stat failed: " + std::string(strerror(errno)));
    }
    auto to_read = sb.st_size;
    ::close(fd);
    ssize_t separate_size = separate_mb * 1024 * 1024;
    ssize_t end;
    dftable df;
    std::vector<dftable> dfs;
    int i = 0;
    time_spent t(DEBUG);
    for(ssize_t start = 0; start < to_read; start = end, i++) {
      end = start + separate_size;
      if(i == 0) {
        df = make_dftable_loadtext_helper(filename, types, names, separator,
                                          nullstr, is_crlf, start, end, 
                                          usecols, mangle_dupe_cols);
      } else {
        auto t = make_dftable_loadtext_helper(filename, types, names, separator,
                                              nullstr, is_crlf, start, end, 
                                              usecols, mangle_dupe_cols);
        dfs.push_back(t);
      }
    }
    t.show("make_dftable_loadtext: load separated df: ");
    auto ret = df.union_tables(dfs, to_keep_order);
    t.show("make_dftable_loadtext: union tables: ");
    return ret;
  } else {
    ssize_t end = -1;
    return make_dftable_loadtext_helper(filename, types, names, separator,
                                        nullstr, is_crlf, 0, end, 
                                        usecols, mangle_dupe_cols);
  }
}

dftable make_dftable_loadtext_helper2(const string& filename,
                                      const vector<string>& types,
                                      vector<string>& names,
                                      int separator,
                                      const string& nullstr,
                                      bool is_crlf,
                                      ssize_t& end,
                                      const vector<int>& usecols,
                                      bool mangle_dupe_cols) {
  time_spent t(DEBUG);
  auto line_starts_byword = make_node_local_allocate<std::vector<size_t>>();
  auto ws = load_csv_separate(filename, line_starts_byword, 0, end,
                              is_crlf, false, separator);
  t.show("make_dftable_loadtext::load_csv: ");
  names = ws.map(get_names, line_starts_byword).get(0);
  auto num_cols = types.size();
  if(names.size() != num_cols)
    throw runtime_error("invalid number of colums, types, or names");
  auto& cnames = const_cast<vector<string>&> (names);
  if (mangle_dupe_cols) cnames = mangle_duplicate_column_names(names); 
  dftable ret;
  auto sorted_colid = vector_sort(usecols);
  size_t curpos = 0;
  for(size_t i = 0; i < num_cols; i++) {
    if(to_be_parsed(sorted_colid, curpos, i)) {
      auto col = parse_words(ws, line_starts_byword, types[i], i, nullstr, true);
      ret.append_column(cnames[i], col);
      t.show(std::string("make_dftable_loadtext::parse_words, ")
             + cnames[i] + ": ");
    }
  }
  return ret;
}

dftable make_dftable_loadtext(const string& filename,
                              const vector<string>& types,
                              int separator,
                              const string& nullstr,
                              bool is_crlf,
                              bool to_separate,
                              bool to_keep_order,
                              double separate_mb,
                              const vector<int>& usecols,
                              bool mangle_dupe_cols) {
  if(to_separate) {
    int fd = ::open(filename.c_str(), O_RDONLY);
    if(fd == -1) {
      throw std::runtime_error("open failed: " + std::string(strerror(errno)));
    }
    struct stat sb;
    if(stat(filename.c_str(), &sb) != 0) {
      ::close(fd);
      throw std::runtime_error("stat failed: " + std::string(strerror(errno)));
    }
    auto to_read = sb.st_size;
    ::close(fd);
    ssize_t separate_size = separate_mb * 1024 * 1024;
    ssize_t end;
    dftable df;
    std::vector<dftable> dfs;
    int i = 0;
    time_spent t(DEBUG);
    std::vector<std::string> names;
    for(ssize_t start = 0; start < to_read; start = end, i++) {
      end = start + separate_size;
      if(i == 0) {
        df = make_dftable_loadtext_helper2(filename, types, names, separator,
                                           nullstr, is_crlf, end, 
                                           usecols, mangle_dupe_cols);
      } else {
        auto t = make_dftable_loadtext_helper(filename, types, names, separator,
                                              nullstr, is_crlf, start, end, 
                                              usecols, mangle_dupe_cols);
        dfs.push_back(t);
      }
    }
    t.show("make_dftable_loadtext: load separated df: ");
    auto ret = df.union_tables(dfs, to_keep_order);
    t.show("make_dftable_loadtext: union tables: ");
    return ret;
  } else {
    ssize_t end = -1;
    std::vector<std::string> names;
    return make_dftable_loadtext_helper2(filename, types, names, separator,
                                         nullstr, is_crlf, end, 
                                         usecols, mangle_dupe_cols);
  }
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

dftable make_dftable_loadtext_helper3(const string& filename,
                                      vector<string>& types,
                                      const vector<string>& names,
                                      int separator,
                                      const string& nullstr,
                                      size_t rows_to_see,
                                      bool is_crlf,
                                      ssize_t& end,
                                      const vector<int>& usecols,
                                      bool mangle_dupe_cols) {
  time_spent t(DEBUG);
  auto line_starts_byword = make_node_local_allocate<std::vector<size_t>>();
  auto ws = load_csv_separate(filename, line_starts_byword, 0, end,
                              is_crlf, false, separator);
  t.show("make_dftable_loadtext::load_csv: ");
  auto num_cols = names.size();
  auto& cnames = const_cast<vector<string>&> (names);
  if (mangle_dupe_cols) cnames = mangle_duplicate_column_names(names); 
  dftable ret;
  auto brows_to_see = broadcast(rows_to_see);
  auto bnullstr = broadcast(nullstr);
  auto bfalse = broadcast(false);
  auto sorted_colid = vector_sort(usecols);
  size_t curpos = 0;
  for(size_t i = 0; i < num_cols; i++) {
    if(to_be_parsed(sorted_colid, curpos, i)) {
      auto type = ws.map(infer_dtype_loadtext, line_starts_byword,
                         broadcast(i), bnullstr, brows_to_see, bfalse).
        reduce(reduce_inferred_dtype);
      string stype;
      if(type == inferred_dtype::inferred_dtype_int) stype = "long";
      else if(type == inferred_dtype::inferred_dtype_float) stype = "double";
      else stype = "dic_string";
      auto col = parse_words(ws, line_starts_byword, stype, i, nullstr, false);
      ret.append_column(cnames[i], col);
      types.push_back(stype);
      t.show(std::string("make_dftable_loadtext::parse_words, ")
             + cnames[i] + ": ");
    }
  }
  return ret;
}

// partial
dftable make_dftable_loadtext_helper3_partial(const string& filename,
                                      vector<string>& types,
                                      const vector<string>& names,
                                      std::map<std::string, std::string>& type_map,
                                      int separator,
                                      const string& nullstr,
                                      size_t rows_to_see,
                                      bool is_crlf,
                                      ssize_t& end,
                                      const vector<int>& usecols,
                                      bool mangle_dupe_cols) {
  time_spent t(DEBUG);
  auto line_starts_byword = make_node_local_allocate<std::vector<size_t>>();
  auto ws = load_csv_separate(filename, line_starts_byword, 0, end,
                              is_crlf, false, separator);
  t.show("make_dftable_loadtext::load_csv: ");
  auto num_cols = names.size();
  auto& cnames = const_cast<vector<string>&> (names);
  if (mangle_dupe_cols) cnames = mangle_duplicate_column_names(names); 
  dftable ret;
  auto brows_to_see = broadcast(rows_to_see);
  auto bnullstr = broadcast(nullstr);
  auto bfalse = broadcast(false);
  auto sorted_colid = vector_sort(usecols);
  size_t curpos = 0;
  for(size_t i = 0; i < num_cols; i++) {
    if(to_be_parsed(sorted_colid, curpos, i)) {
      string stype;
      if (type_map.find(names[i]) != type_map.end()) stype = type_map[names[i]];
      else {
        auto type = ws.map(infer_dtype_loadtext, line_starts_byword,
                         broadcast(i), bnullstr, brows_to_see, bfalse).
        reduce(reduce_inferred_dtype);
        if(type == inferred_dtype::inferred_dtype_int) stype = "long";
        else if(type == inferred_dtype::inferred_dtype_float) stype = "double";
        else stype = "dic_string";
      }
      auto col = parse_words(ws, line_starts_byword, stype, i, nullstr, false);
      ret.append_column(cnames[i], col);
      types.push_back(stype);
      t.show(std::string("make_dftable_loadtext::parse_words, ")
             + cnames[i] + ": ");
    }
  }
  return ret;
}

dftable make_dftable_loadtext_infertype(const string& filename,
                                        const vector<string>& names,
                                        int separator,
                                        const string& nullstr,
                                        size_t rows_to_see,
                                        bool is_crlf,
                                        bool to_separate,
                                        bool to_keep_order,
                                        double separate_mb,
                                        const vector<int>& usecols,
                                        bool mangle_dupe_cols) {
  if(to_separate) {
    int fd = ::open(filename.c_str(), O_RDONLY);
    if(fd == -1) {
      throw std::runtime_error("open failed: " + std::string(strerror(errno)));
    }
    struct stat sb;
    if(stat(filename.c_str(), &sb) != 0) {
      ::close(fd);
      throw std::runtime_error("stat failed: " + std::string(strerror(errno)));
    }
    auto to_read = sb.st_size;
    ::close(fd);
    ssize_t separate_size = separate_mb * 1024 * 1024;
    ssize_t end;
    dftable df;
    std::vector<dftable> dfs;
    int i = 0;
    std::vector<string> types;
    time_spent t(DEBUG);
    for(ssize_t start = 0; start < to_read; start = end, i++) {
      end = start + separate_size;
      if(i == 0) {
        df = make_dftable_loadtext_helper3(filename, types, names, separator,
                                           nullstr, rows_to_see, is_crlf, end, 
                                           usecols, mangle_dupe_cols);
      } else {
        auto t = make_dftable_loadtext_helper(filename, types, names, separator,
                                              nullstr, is_crlf, start, end, 
                                              usecols, mangle_dupe_cols);
        dfs.push_back(t);
      }
    }
    t.show("make_dftable_loadtext: load separated df: ");
    auto ret = df.union_tables(dfs, to_keep_order);
    t.show("make_dftable_loadtext: union tables: ");
    return ret;
  } else {
    ssize_t end = -1;
    std::vector<string> types;
    return make_dftable_loadtext_helper3(filename, types, names, separator,
                                         nullstr, is_crlf, 0, end, 
                                         usecols, mangle_dupe_cols);
  }
}

dftable make_dftable_loadtext_infertype(const string& filename,
                                        const vector<string>& names,
                                        std::map<std::string, std::string>& type_map,
                                        int separator,
                                        const string& nullstr,
                                        size_t rows_to_see,
                                        bool is_crlf,
                                        bool to_separate,
                                        bool to_keep_order,
                                        double separate_mb,
                                        const vector<int>& usecols,
                                        bool mangle_dupe_cols) {
  if(to_separate) {
    int fd = ::open(filename.c_str(), O_RDONLY);
    if(fd == -1) {
      throw std::runtime_error("open failed: " + std::string(strerror(errno)));
    }
    struct stat sb;
    if(stat(filename.c_str(), &sb) != 0) {
      ::close(fd);
      throw std::runtime_error("stat failed: " + std::string(strerror(errno)));
    }
    auto to_read = sb.st_size;
    ::close(fd);
    ssize_t separate_size = separate_mb * 1024 * 1024;
    ssize_t end;
    dftable df;
    std::vector<dftable> dfs;
    int i = 0;
    std::vector<string> types;
    time_spent t(DEBUG);
    for(ssize_t start = 0; start < to_read; start = end, i++) {
      end = start + separate_size;
      if(i == 0) {
        df = make_dftable_loadtext_helper3_partial(filename, types, names, type_map, 
                                                  separator, nullstr, rows_to_see,
                                                  is_crlf, end, 
                                                  usecols, mangle_dupe_cols);
      } else {
        auto t = make_dftable_loadtext_helper(filename, types, names, separator,
                                              nullstr, is_crlf, start, end, 
                                              usecols, mangle_dupe_cols);
        dfs.push_back(t);
      }
    }
    t.show("make_dftable_loadtext: load separated df: ");
    auto ret = df.union_tables(dfs, to_keep_order);
    t.show("make_dftable_loadtext: union tables: ");
    return ret;
  } else {
    ssize_t end = -1;
    std::vector<string> types;
    return make_dftable_loadtext_helper3_partial(filename, types, names, type_map,
                                                separator, nullstr, is_crlf, 0, end, 
                                                usecols, mangle_dupe_cols);
  }
}


dftable make_dftable_loadtext_helper4(const string& filename,
                                      vector<string>& types,
                                      vector<string>& names,
                                      int separator,
                                      const string& nullstr,
                                      size_t rows_to_see,
                                      bool is_crlf,
                                      ssize_t& end,
                                      const vector<int>& usecols,
                                      bool mangle_dupe_cols) {
  time_spent t(DEBUG);
  auto line_starts_byword = make_node_local_allocate<std::vector<size_t>>();
  auto ws = load_csv_separate(filename, line_starts_byword, 0, end,
                              is_crlf, false, separator);
  t.show("make_dftable_loadtext::load_csv: ");
  names = ws.map(get_names, line_starts_byword).get(0);
  auto num_cols = names.size();
  auto& cnames = const_cast<vector<string>&> (names);
  if (mangle_dupe_cols) cnames = mangle_duplicate_column_names(names); 
  dftable ret;
  auto brows_to_see = broadcast(rows_to_see);
  auto bnullstr = broadcast(nullstr);
  auto btrue = broadcast(true);
  auto sorted_colid = vector_sort(usecols);
  size_t curpos = 0;
  for(size_t i = 0; i < num_cols; i++) {
    if(to_be_parsed(sorted_colid, curpos, i)) {
      auto type = ws.map(infer_dtype_loadtext, line_starts_byword,
                         broadcast(i), bnullstr, brows_to_see, btrue).
        reduce(reduce_inferred_dtype);
      string stype;
      if(type == inferred_dtype::inferred_dtype_int) stype = "long";
      else if(type == inferred_dtype::inferred_dtype_float) stype = "double";
      else stype = "dic_string";
      types.push_back(stype);
      auto col = parse_words(ws, line_starts_byword, stype, i, nullstr, true);
      ret.append_column(cnames[i], col);
      t.show(std::string("make_dftable_loadtext::parse_words, ")
             + cnames[i] + ": ");
    }
  }
  return ret;
}

dftable make_dftable_loadtext_helper4_partial(const string& filename,
                                              vector<string>& types,
                                              vector<string>& names,
                                              std::map<std::string, std::string>& type_map,
                                              int separator,
                                              const string& nullstr,
                                              size_t rows_to_see,
                                              bool is_crlf,
                                              ssize_t& end,
                                              const vector<int>& usecols,
                                              bool mangle_dupe_cols) {
  time_spent t(DEBUG);
  auto line_starts_byword = make_node_local_allocate<std::vector<size_t>>();
  auto ws = load_csv_separate(filename, line_starts_byword, 0, end,
                              is_crlf, false, separator);
  t.show("make_dftable_loadtext::load_csv: ");
  names = ws.map(get_names, line_starts_byword).get(0);
  auto num_cols = names.size();
  auto& cnames = const_cast<vector<string>&> (names);
  if (mangle_dupe_cols) cnames = mangle_duplicate_column_names(names); 
  dftable ret;
  auto brows_to_see = broadcast(rows_to_see);
  auto bnullstr = broadcast(nullstr);
  auto btrue = broadcast(true);
  auto sorted_colid = vector_sort(usecols);
  size_t curpos = 0;
  for(size_t i = 0; i < num_cols; i++) {
    if(to_be_parsed(sorted_colid, curpos, i)) {
      string stype;
      if (type_map.find(names[i]) != type_map.end()) stype = type_map[names[i]];
      else {
        auto type = ws.map(infer_dtype_loadtext, line_starts_byword,
                         broadcast(i), bnullstr, brows_to_see, btrue).
        reduce(reduce_inferred_dtype);
        if(type == inferred_dtype::inferred_dtype_int) stype = "long";
        else if(type == inferred_dtype::inferred_dtype_float) stype = "double";
        else stype = "dic_string";
      }
      types.push_back(stype);
      auto col = parse_words(ws, line_starts_byword, stype, i, nullstr, true);
      ret.append_column(cnames[i], col);
      t.show(std::string("make_dftable_loadtext::parse_words, ")
             + cnames[i] + ": ");
    }
  }
  return ret;
}

dftable make_dftable_loadtext_infertype(const string& filename,
                                        int separator,
                                        const string& nullstr,
                                        size_t rows_to_see,
                                        bool is_crlf,
                                        bool to_separate,
                                        bool to_keep_order,
                                        double separate_mb,
                                        const vector<int>& usecols,
                                        bool mangle_dupe_cols) {
  if(to_separate) {
    int fd = ::open(filename.c_str(), O_RDONLY);
    if(fd == -1) {
      throw std::runtime_error("open failed: " + std::string(strerror(errno)));
    }
    struct stat sb;
    if(stat(filename.c_str(), &sb) != 0) {
      ::close(fd);
      throw std::runtime_error("stat failed: " + std::string(strerror(errno)));
    }
    auto to_read = sb.st_size;
    ::close(fd);
    ssize_t separate_size = separate_mb * 1024 * 1024;
    ssize_t end;
    dftable df;
    std::vector<dftable> dfs;
    int i = 0;
    std::vector<string> types;
    vector<string> names;
    time_spent t(DEBUG);
    for(ssize_t start = 0; start < to_read; start = end, i++) {
      end = start + separate_size;
      if(i == 0) {
        df = make_dftable_loadtext_helper4(filename, types, names, separator,
                                           nullstr, rows_to_see, is_crlf, end, 
                                           usecols, mangle_dupe_cols);
      } else {
        auto t = make_dftable_loadtext_helper(filename, types, names, separator,
                                              nullstr, is_crlf, start, end, 
                                              usecols, mangle_dupe_cols);
        dfs.push_back(t);
      }
    }
    t.show("make_dftable_loadtext: load separated df: ");
    auto ret = df.union_tables(dfs, to_keep_order);
    t.show("make_dftable_loadtext: union tables: ");
    return ret;
  } else {
    ssize_t end = -1;
    std::vector<string> types;
    std::vector<string> names;
    return make_dftable_loadtext_helper4(filename, types, names, separator,
                                         nullstr, is_crlf, 0, end, 
                                         usecols, mangle_dupe_cols);
  }
}

dftable make_dftable_loadtext_infertype(const string& filename,
                                        std::map<std::string, std::string>& type_map,
                                        int separator,
                                        const string& nullstr,
                                        size_t rows_to_see,
                                        bool is_crlf,
                                        bool to_separate,
                                        bool to_keep_order,
                                        double separate_mb,
                                        const vector<int>& usecols,
                                        bool mangle_dupe_cols) {
  if(to_separate) {
    int fd = ::open(filename.c_str(), O_RDONLY);
    if(fd == -1) {
      throw std::runtime_error("open failed: " + std::string(strerror(errno)));
    }
    struct stat sb;
    if(stat(filename.c_str(), &sb) != 0) {
      ::close(fd);
      throw std::runtime_error("stat failed: " + std::string(strerror(errno)));
    }
    auto to_read = sb.st_size;
    ::close(fd);
    ssize_t separate_size = separate_mb * 1024 * 1024;
    ssize_t end;
    dftable df;
    std::vector<dftable> dfs;
    int i = 0;
    std::vector<string> types;
    vector<string> names;
    time_spent t(DEBUG);
    for(ssize_t start = 0; start < to_read; start = end, i++) {
      end = start + separate_size;
      if(i == 0) {
        df = make_dftable_loadtext_helper4_partial(filename, types, names, type_map,
                                                   separator, nullstr, rows_to_see,
                                                   is_crlf, end, 
                                                   usecols, mangle_dupe_cols);
      } else {
        auto t = make_dftable_loadtext_helper(filename, types, names, separator,
                                              nullstr, is_crlf, start, end, 
                                              usecols, mangle_dupe_cols);
        dfs.push_back(t);
      }
    }
    t.show("make_dftable_loadtext: load separated df: ");
    auto ret = df.union_tables(dfs, to_keep_order);
    t.show("make_dftable_loadtext: union tables: ");
    return ret;
  } else {
    ssize_t end = -1;
    std::vector<string> types;
    std::vector<string> names;
    return make_dftable_loadtext_helper4_partial(filename, types, names, type_map,
                                                 separator, nullstr, is_crlf, 0, end, 
                                                 usecols, mangle_dupe_cols);
  }
}

}
