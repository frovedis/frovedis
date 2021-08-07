#include "load_csv.hpp"
#include "find.hpp"
#include "words.hpp"
#include "../core/lower_bound.hpp"
#include "../core/find_condition.hpp"

using namespace std;

namespace frovedis {

void normalize_tail(vector<int>& v) {
  auto v_size = v.size();
  if(v_size > 0 && v[v_size-1] != '\n') {
    vector<int> tmp(v_size + 1);
    auto vp = v.data();
    auto tmpp = tmp.data();
    for(size_t i = 0; i < v_size; i++) {
      tmpp[i] = vp[i];
    }
    tmpp[v_size] = '\n';
    v.swap(tmp);
  }
}

void skip_head_local(vector<size_t>& start) {
  auto start_size = start.size();
  if(start_size == 0) return;
  vector<size_t> tmpstart(start_size);
  auto startp = start.data();
  auto tmpstartp = tmpstart.data();
  for(size_t i = 0; i < start_size-1; i++) {
    tmpstartp[i] = startp[i+1];
  }
  start.swap(tmpstart);
}

void skip_head(vector<size_t>& start) {
  if(get_selfid() == 0) {
    skip_head_local(start);
  }
}

enum csv_state {
  line_start,
  entry_start,
  in_entry,
  in_escape,
  in_quote,
  in_quote_escape,
  out_quote
};

enum csv_action {
  out = 1 << 16,
  start_entry = 1 << 17,
  start_line = 1 << 18
};

void parse_csv_vreg(uint32_t state[][256][32], const int* vp, int* outvp,
                    size_t* startidx, size_t* stopidx, size_t* outidx,
                    size_t max, size_t* outstartp, size_t* outstartidx) {
  // currently unroll 3 is the best
  size_t crnt_ridx0[LOAD_CSV_VLEN_EACH];
  size_t crnt_ridx1[LOAD_CSV_VLEN_EACH];
  size_t crnt_ridx2[LOAD_CSV_VLEN_EACH];
  size_t stop_ridx0[LOAD_CSV_VLEN_EACH];
  size_t stop_ridx1[LOAD_CSV_VLEN_EACH];
  size_t stop_ridx2[LOAD_CSV_VLEN_EACH];
  size_t out_ridx0[LOAD_CSV_VLEN_EACH];
  size_t out_ridx1[LOAD_CSV_VLEN_EACH];
  size_t out_ridx2[LOAD_CSV_VLEN_EACH];
  size_t outstart_ridx0[LOAD_CSV_VLEN_EACH];
  size_t outstart_ridx1[LOAD_CSV_VLEN_EACH];
  size_t outstart_ridx2[LOAD_CSV_VLEN_EACH];
  uint32_t crnt_state_ridx0[LOAD_CSV_VLEN_EACH];
  uint32_t crnt_state_ridx1[LOAD_CSV_VLEN_EACH];
  uint32_t crnt_state_ridx2[LOAD_CSV_VLEN_EACH];
#pragma _NEC vreg(crnt_ridx0)
#pragma _NEC vreg(crnt_ridx1)
#pragma _NEC vreg(crnt_ridx2)
#pragma _NEC vreg(stop_ridx0)
#pragma _NEC vreg(stop_ridx1)
#pragma _NEC vreg(stop_ridx2)
#pragma _NEC vreg(out_ridx0)
#pragma _NEC vreg(out_ridx1)
#pragma _NEC vreg(out_ridx2)
#pragma _NEC vreg(outstart_ridx0)
#pragma _NEC vreg(outstart_ridx1)
#pragma _NEC vreg(outstart_ridx2)
#pragma _NEC vreg(crnt_state_ridx0)
#pragma _NEC vreg(crnt_state_ridx1)
#pragma _NEC vreg(crnt_state_ridx2)

  for(size_t i = 0; i < LOAD_CSV_VLEN_EACH; i++) {
    crnt_ridx0[i] = startidx[i];
    crnt_ridx1[i] = startidx[i + LOAD_CSV_VLEN_EACH];
    crnt_ridx2[i] = startidx[i + LOAD_CSV_VLEN_EACH * 2];
    stop_ridx0[i] = stopidx[i];
    stop_ridx1[i] = stopidx[i + LOAD_CSV_VLEN_EACH];
    stop_ridx2[i] = stopidx[i + LOAD_CSV_VLEN_EACH * 2];
    out_ridx0[i] = startidx[i];
    out_ridx1[i] = startidx[i + LOAD_CSV_VLEN_EACH];
    out_ridx2[i] = startidx[i + LOAD_CSV_VLEN_EACH * 2];
    outstart_ridx0[i] = startidx[i];
    outstart_ridx1[i] = startidx[i + LOAD_CSV_VLEN_EACH];
    outstart_ridx2[i] = startidx[i + LOAD_CSV_VLEN_EACH * 2];
    crnt_state_ridx0[i] = csv_state::line_start;
    crnt_state_ridx1[i] = csv_state::line_start;
    crnt_state_ridx2[i] = csv_state::line_start;
  }

  size_t flag = size_t(1) << (sizeof(size_t) * 8 - 1);

#pragma _NEC vob
  for(size_t i = 0; i < max; i++) {
#pragma _NEC ivdep
#pragma _NEC vovertake
    for(size_t j = 0; j < LOAD_CSV_VLEN_EACH; j++) {
      if(crnt_ridx0[j] != stop_ridx0[j]) {
        auto loaded = vp[crnt_ridx0[j]];
        auto st = state[crnt_state_ridx0[j]][loaded][0];
        if(st & csv_action::start_entry) {
          auto to_store =
            (st & csv_action::start_line) ? out_ridx0[j] + flag : out_ridx0[j];
          outstartp[outstart_ridx0[j]++] = to_store;
        }
        if(st & csv_action::out) {
          outvp[out_ridx0[j]++] = loaded;
        }
        crnt_ridx0[j]++;
        crnt_state_ridx0[j] = st & 0xFFFF;
      }
      if(crnt_ridx1[j] != stop_ridx1[j]) {
        auto loaded = vp[crnt_ridx1[j]];
        auto st = state[crnt_state_ridx1[j]][loaded][0];
        if(st & csv_action::start_entry) {
          auto to_store =
            (st & csv_action::start_line) ? out_ridx1[j] + flag : out_ridx1[j];
          outstartp[outstart_ridx1[j]++] = to_store;
        }
        if(st & csv_action::out) {
          outvp[out_ridx1[j]++] = loaded;
        }
        crnt_ridx1[j]++;
        crnt_state_ridx1[j] = st & 0xFFFF;
      }
      if(crnt_ridx2[j] != stop_ridx2[j]) {
        auto loaded = vp[crnt_ridx2[j]];
        auto st = state[crnt_state_ridx2[j]][loaded][0];
        if(st & csv_action::start_entry) {
          auto to_store =
            (st & csv_action::start_line) ? out_ridx2[j] + flag : out_ridx2[j];
          outstartp[outstart_ridx2[j]++] = to_store;
        }
        if(st & csv_action::out) {
          outvp[out_ridx2[j]++] = loaded;
        }
        crnt_ridx2[j]++;
        crnt_state_ridx2[j] = st & 0xFFFF;
      }
    }
  }
  for(size_t i = 0; i < LOAD_CSV_VLEN_EACH; i++) {
    outidx[i] = out_ridx0[i];
    outidx[i + LOAD_CSV_VLEN_EACH] = out_ridx1[i];
    outidx[i + LOAD_CSV_VLEN_EACH * 2] = out_ridx2[i];
    outstartidx[i] = outstart_ridx0[i];
    outstartidx[i + LOAD_CSV_VLEN_EACH] = outstart_ridx1[i];
    outstartidx[i + LOAD_CSV_VLEN_EACH * 2] = outstart_ridx2[i];
  }
}

struct is_flagged {
  int operator()(size_t a) const {
    size_t flag = size_t(1) << (sizeof(size_t) * 8 - 1);
    return (a & flag) != 0;
  }
  SERIALIZE_NONE
};

words parse_csv_impl(const int*  vp, size_t v_size,
                     const size_t* startp, size_t start_size,
                     vector<size_t>& line_starts_byword, int separator) {
  if(v_size == 0 || start_size == 0) return words();
  // [32] is to place each element on different cache lines
  uint32_t state[7][256][32];
  size_t num_chars = 256;
  for(size_t i = 0; i < num_chars; i++) {
    if(i == '\\') {
      state[csv_state::line_start][i][0] =
        (csv_state::in_escape|csv_action::start_entry|csv_action::start_line);
    } else if(i == '"') {
      state[csv_state::line_start][i][0] =
        (csv_state::in_quote|csv_action::start_entry|csv_action::start_line);
    } else if(i == separator) {
      state[csv_state::line_start][i][0] =
        (csv_state::entry_start|csv_action::start_entry|csv_action::start_line);
    } else if(i == '\n') {
      state[csv_state::line_start][i][0] =
        (csv_state::line_start|csv_action::start_entry|csv_action::start_line);
    } else {
      state[csv_state::line_start][i][0] =
        (csv_state::in_entry|csv_action::out|csv_action::start_entry|
         csv_action::start_line);
    }
  }
  for(size_t i = 0; i < num_chars; i++) {
    if(i == '\\') {
      state[csv_state::entry_start][i][0] =
        (csv_state::in_escape|csv_action::start_entry);
    } else if(i == '"') {
      state[csv_state::entry_start][i][0] =
        (csv_state::in_quote|csv_action::start_entry);
    } else if(i == separator) {
      state[csv_state::entry_start][i][0] =
        (csv_state::entry_start|csv_action::start_entry);
    } else if(i == '\n') {
      state[csv_state::entry_start][i][0] =
        (csv_state::line_start|csv_action::start_entry);
    } else {
      state[csv_state::entry_start][i][0] =
        (csv_state::in_entry|csv_action::out|csv_action::start_entry);
    }
  }
  for(size_t i = 0; i < num_chars; i++) {
    if(i == '\\') state[csv_state::in_entry][i][0] = csv_state::in_escape;
    else if(i == separator) 
      state[csv_state::in_entry][i][0] = csv_state::entry_start;
    else if(i == '\n') state[csv_state::in_entry][i][0] = csv_state::line_start;
    else
      state[csv_state::in_entry][i][0] = (csv_state::in_entry|csv_action::out);
  }
  for(size_t i = 0; i < num_chars; i++) {
    state[csv_state::in_escape][i][0] = (csv_state::in_entry|csv_action::out);
  }
  for(size_t i = 0; i < num_chars; i++) {
    if(i == '\\') state[csv_state::in_quote][i][0] = csv_state::in_quote_escape;
    else if(i == '"') state[csv_state::in_quote][i][0] = csv_state::out_quote;
    else
      state[csv_state::in_quote][i][0] = (csv_state::in_quote|csv_action::out);
  }
  for(size_t i = 0; i < num_chars; i++) {
    state[csv_state::in_quote_escape][i][0] =
      (csv_state::in_quote|csv_action::out);
  }
  for(size_t i = 0; i < num_chars; i++) {
    if(i == separator)
      state[csv_state::out_quote][i][0] = csv_state::entry_start;
    else if(i == '\n')
      state[csv_state::out_quote][i][0] = csv_state::line_start;
    else if(i == '"')
      state[csv_state::out_quote][i][0] = (csv_state::in_quote|csv_action::out);
    else  // invalid
      state[csv_state::out_quote][i][0] =
        (csv_state::out_quote|csv_action::out);
  }

  vector<int> outv(v_size);
  auto outvp = outv.data();
  vector<size_t> outstart(v_size);
  auto outstartp = outstart.data();
  
  auto each = ceil_div(v_size, size_t(LOAD_CSV_VLEN));
  if(each % 2 == 0) each++;
  size_t starttmp[LOAD_CSV_VLEN];
  for(size_t i = 0; i < LOAD_CSV_VLEN; i++) {
    if(each * i < v_size) starttmp[i] = each * i;
    else starttmp[i] = v_size;
  }
  size_t startlineidx[LOAD_CSV_VLEN];
  lower_bound(startp, start_size, starttmp, LOAD_CSV_VLEN, startlineidx);
  size_t startidx[LOAD_CSV_VLEN];
  for(size_t i = 0; i < LOAD_CSV_VLEN; i++) {
    if(startlineidx[i] < start_size) startidx[i] = startp[startlineidx[i]];
    else startidx[i] = v_size;
  }
  size_t stopidx[LOAD_CSV_VLEN];
  for(size_t i = 0; i < LOAD_CSV_VLEN-1; i++) {
    stopidx[i] = startidx[i+1];
  }
  stopidx[LOAD_CSV_VLEN-1] = v_size;
  size_t max = 0;
  for(size_t i = 0; i < LOAD_CSV_VLEN; i++) {
    auto size = stopidx[i] - startidx[i];
    if(size > max) max = size;
  }
  size_t outidx[LOAD_CSV_VLEN];
  size_t outstartidx[LOAD_CSV_VLEN];
  parse_csv_vreg(state, vp, outvp, startidx, stopidx, outidx, max,
                 outstartp, outstartidx);
  size_t totalchars = 0;
  size_t totalwords = 0;
  for(size_t i = 0; i < LOAD_CSV_VLEN; i++) {
    totalchars += outidx[i] - startidx[i];
    totalwords += outstartidx[i] - startidx[i];
  }
  words ret;
  if(totalchars == 0 || totalwords == 0) {
    line_starts_byword.resize(0);
    return ret;
  }
  ret.chars.resize(totalchars);
  ret.starts.resize(totalwords);
  ret.lens.resize(totalwords);
  auto retcharsp = ret.chars.data();
  auto retstartsp = ret.starts.data();
  auto retlensp = ret.lens.data();
  auto crnt_retcharsp = retcharsp;
  for(size_t i = 0; i < LOAD_CSV_VLEN; i++) {
    auto size = outidx[i] - startidx[i];
    auto crnt_src = outvp + startidx[i];
    for(size_t j = 0; j < size; j++) {
      crnt_retcharsp[j] = crnt_src[j];
    }
    crnt_retcharsp += size;
  }
  auto crnt_retstartsp = retstartsp;
  size_t off = 0;
  for(size_t i = 0; i < LOAD_CSV_VLEN; i++) {
    auto size = outstartidx[i] - startidx[i];
    auto crnt_src = outstartp + startidx[i];
    for(size_t j = 0; j < size; j++) {
      crnt_retstartsp[j] = crnt_src[j] - startidx[i] + off;
    }
    crnt_retstartsp += size;
    off += outidx[i] - startidx[i];
  }
  line_starts_byword = find_condition(retstartsp, totalwords, is_flagged());
  size_t flag = size_t(1) << (sizeof(size_t) * 8 - 1);
  auto mask = ~flag;
  for(size_t i = 0; i < totalwords; i++) {
    retstartsp[i] &= mask;
  }
  for(size_t i = 0; i < totalwords-1; i++) {
    retlensp[i] = retstartsp[i+1] - retstartsp[i];
  }
  retlensp[totalwords-1] = totalchars - retstartsp[totalwords-1];
  return ret;
}

words parse_csv(vector<int>& v, vector<size_t>& start,
                vector<size_t>& line_starts_byword, int separator) {
  size_t block_size_mb = 10; // TODO: how to specify?
  auto block_size = block_size_mb * 1024 * 1024 / 4;
  auto v_size = v.size();
  auto num_block = v_size / block_size;
  if(num_block == 0) {
    return parse_csv_impl(v.data(), v.size(), start.data(), start.size(),
                          line_starts_byword, separator);
  }    
  std::vector<size_t> search_block_pos(num_block);
  auto search_block_posp = search_block_pos.data();
  for(size_t i = 0; i < num_block; i++) search_block_posp[i] = block_size * i;
  // block_pos may contain same pos or start.end()
  auto block_pos = lower_bound(start, search_block_pos);
  vector<words> vec_ret(num_block);
  vector<vector<size_t>> vec_line_starts_byword(num_block);
  auto vp = v.data();
  size_t crnt_num_words = 0;
  auto start_size = start.size();
  auto startp = start.data();
  for(size_t i = 0; i < num_block-1; i++) {
    if(block_pos[i] == start_size || block_pos[i+1] == block_pos[i]) continue;
    size_t call_vsize, call_start_size;
    if(block_pos[i+1] == start_size) {
      call_vsize = v_size - start[block_pos[i]];
      call_start_size = start_size - block_pos[i];
    } else {
      call_vsize = start[block_pos[i+1]] - start[block_pos[i]];
      call_start_size = block_pos[i+1] - block_pos[i];
    }
    auto start_pos = block_pos[i];
    auto v_pos = start[block_pos[i]];
    for(size_t j = 0; j < call_start_size; j++) {
      startp[start_pos + j] -= v_pos;
    }
    vec_ret[i] = parse_csv_impl(vp + v_pos,
                                call_vsize,
                                startp + start_pos,
                                call_start_size,
                                vec_line_starts_byword[i],
                                separator);
    auto vec_line_starts_bywordip = vec_line_starts_byword[i].data();
    auto vec_line_starts_bywordi_size = vec_line_starts_byword[i].size();
    for(size_t j = 0; j < vec_line_starts_bywordi_size; j++) {
      vec_line_starts_bywordip[j] += crnt_num_words;
    }
    crnt_num_words += vec_line_starts_bywordi_size;
  }
  if(block_pos[num_block-1] != start_size) {
    size_t call_vsize, call_start_size;
    call_vsize = v_size - start[block_pos[num_block-1]];
    call_start_size = start_size - block_pos[num_block-1];
    auto start_pos = block_pos[num_block-1];
    auto v_pos = start[block_pos[num_block-1]];
    for(size_t j = 0; j < call_start_size; j++) {
      startp[start_pos + j] -= v_pos;
    }
    vec_ret[num_block-1] = parse_csv_impl(vp + v_pos,
                                          call_vsize,
                                          startp + start_pos,
                                          call_start_size,
                                          vec_line_starts_byword[num_block-1],
                                          separator);
    auto vec_line_starts_bywordip = vec_line_starts_byword[num_block-1].data();
    auto vec_line_starts_bywordi_size =
      vec_line_starts_byword[num_block-1].size();
    for(size_t j = 0; j < vec_line_starts_bywordi_size; j++) {
      vec_line_starts_bywordip[j] += crnt_num_words;
    }
    crnt_num_words += vec_line_starts_bywordi_size;
  }
  {vector<int> vtmp; v.swap(vtmp);}
  {vector<size_t> starttmp; start.swap(starttmp);}
  words ret = merge_multi_words(vec_ret);
  line_starts_byword.resize(crnt_num_words);
  auto line_starts_bywordp = line_starts_byword.data();
  auto crnt_line_starts_bywordp = line_starts_bywordp;
  for(size_t i = 0; i < num_block; i++) {
    auto vec_line_starts_bywordip = vec_line_starts_byword[i].data();
    auto vec_line_starts_bywordi_size = vec_line_starts_byword[i].size();
    for(size_t j = 0; j < vec_line_starts_bywordi_size; j++) {
      crnt_line_starts_bywordp[j] = vec_line_starts_bywordip[j];
    }
    crnt_line_starts_bywordp += vec_line_starts_bywordi_size;
  }
  return ret;
}

node_local<words>
load_csv(const std::string& path,
         node_local<std::vector<size_t>>& line_starts_byword,
         bool is_crlf,
         bool to_skip_head,
         int separator) {
  auto start = make_node_local_allocate<vector<size_t>>();
  node_local<vector<int>> lv;
  if(is_crlf) {
    auto len = make_node_local_allocate<vector<size_t>>();
    auto start_tmp = make_node_local_allocate<vector<size_t>>();
    auto lv_tmp = load_text(path, "\r\n", start_tmp, len);
    lv = lv_tmp.map(concat_docs, start_tmp, len, broadcast('\n'), start);
  } else {
    auto len = make_node_local_allocate<vector<size_t>>();
    lv = load_text(path, "\n", start, len);
    lv.mapv(normalize_tail);
  }
  if(to_skip_head) start.mapv(skip_head);
  return lv.map(parse_csv, start, line_starts_byword, broadcast(separator));
}

node_local<words>
load_csv_separate(const std::string& path,
                  node_local<std::vector<size_t>>& line_starts_byword,
                  ssize_t start_pos, ssize_t& end_pos,
                  bool is_crlf,
                  bool to_skip_head,
                  int separator) {
  auto start = make_node_local_allocate<vector<size_t>>();
  node_local<vector<int>> lv;
  if(is_crlf) {
    auto len = make_node_local_allocate<vector<size_t>>();
    auto start_tmp = make_node_local_allocate<vector<size_t>>();
    auto lv_tmp = load_text_separate(path, "\r\n", start_tmp, len,
                                     start_pos, end_pos);
    lv = lv_tmp.map(concat_docs, start_tmp, len, broadcast('\n'), start);
  } else {
    auto len = make_node_local_allocate<vector<size_t>>();
    lv = load_text_separate(path, "\n", start, len, start_pos, end_pos);
    lv.mapv(normalize_tail);
  }
  if(to_skip_head) start.mapv(skip_head);
  return lv.map(parse_csv, start, line_starts_byword, broadcast(separator));
}

words
load_csv_local(const std::string& path,
               std::vector<size_t>& line_starts_byword,
               bool is_crlf,
               bool to_skip_head,
               int separator) {
  vector<size_t> start;
  vector<int> v;
  if(is_crlf) {
    vector<size_t> start_tmp;
    vector<size_t> len;
    auto v_tmp = load_text_local(path, "\r\n", start_tmp, len);
    v = concat_docs(v_tmp, start_tmp, len, '\n', start);
  } else {
    vector<size_t> len;
    v = load_text_local(path, "\n", start, len);
    normalize_tail(v);
  }
  if(to_skip_head) skip_head_local(start);
  return parse_csv(v, start, line_starts_byword, separator);
}

words split_simple_csv(std::vector<int>& v, std::vector<size_t>& start,
                       int separator) {
  string delim(1,separator);
  delim.push_back('\n');
  words ret;
  if(start.size() == 0) return ret;
  auto first_start = start[0];
  split_to_words(v.data() + first_start, v.size() - first_start,
                 first_start, ret.starts, ret.lens, delim);
  ret.chars.swap(v);
  return ret;
}

std::vector<size_t>
convert_line_starts(const std::vector<size_t>& line_starts,
                    const words& w) {
  return convert_position_to_word_count(line_starts, w.starts);
}

node_local<words>
load_simple_csv(const std::string& path,
                node_local<std::vector<size_t>>& line_starts_byword,
                bool is_crlf,
                bool to_skip_head,
                int separator) {
  auto start = make_node_local_allocate<vector<size_t>>();
  node_local<vector<int>> lv;
  if(is_crlf) {
    auto len = make_node_local_allocate<vector<size_t>>();
    auto start_tmp = make_node_local_allocate<vector<size_t>>();
    auto lv_tmp = load_text(path, "\r\n", start_tmp, len);
    lv = lv_tmp.map(concat_docs, start_tmp, len, broadcast('\n'), start);
  } else {
    auto len = make_node_local_allocate<vector<size_t>>();
    lv = load_text(path, "\n", start, len);
    lv.mapv(normalize_tail);
  }
  if(to_skip_head) start.mapv(skip_head);
  auto ret = lv.map(split_simple_csv, start, broadcast(separator));
  line_starts_byword = start.map(convert_line_starts, ret);
  return ret;
}

node_local<words>
load_simple_csv_separate(const std::string& path,
                         node_local<std::vector<size_t>>& line_starts_byword,
                         ssize_t start_pos, ssize_t& end_pos,
                         bool is_crlf,
                         bool to_skip_head,
                         int separator) {
  auto start = make_node_local_allocate<vector<size_t>>();
  node_local<vector<int>> lv;
  if(is_crlf) {
    auto len = make_node_local_allocate<vector<size_t>>();
    auto start_tmp = make_node_local_allocate<vector<size_t>>();
    auto lv_tmp = load_text_separate(path, "\r\n", start_tmp, len,
                                     start_pos, end_pos);
    lv = lv_tmp.map(concat_docs, start_tmp, len, broadcast('\n'), start);
  } else {
    auto len = make_node_local_allocate<vector<size_t>>();
    lv = load_text_separate(path, "\n", start, len, start_pos, end_pos);
    lv.mapv(normalize_tail);
  }
  if(to_skip_head) start.mapv(skip_head);
  auto ret = lv.map(split_simple_csv, start, broadcast(separator));
  line_starts_byword = start.map(convert_line_starts, ret);
  return ret;
}

words
load_simple_csv_local(const std::string& path,
                      std::vector<size_t>& line_starts_byword,
                      bool is_crlf,
                      bool to_skip_head,
                      int separator) {
  vector<size_t> start;
  vector<int> v;
  if(is_crlf) {
    vector<size_t> start_tmp;
    vector<size_t> len;
    auto v_tmp = load_text_local(path, "\r\n", start_tmp, len);
    v = concat_docs(v_tmp, start_tmp, len, '\n', start);
  } else {
    vector<size_t> len;
    v = load_text_local(path, "\n", start, len);
    normalize_tail(v);
  }
  if(to_skip_head) skip_head_local(start);
  auto ret = split_simple_csv(v, start, separator);
  line_starts_byword = convert_line_starts(start, ret);
  return ret;
}

}
