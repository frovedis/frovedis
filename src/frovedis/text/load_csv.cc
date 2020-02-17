#include "load_csv.hpp"
#include "find.hpp"
#include "words.hpp"
#include "../core/lower_bound.hpp"

using namespace std;

namespace frovedis {

void normalize_tail(vector<int>& v) {
  auto v_size = v.size();
  if(v[v_size-1] != '\n') {
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

void skip_head(vector<int>& v, vector<size_t>& start) {
  if(get_selfid() == 0) {
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

void parse_csv_vreg(uint32_t state[][256], const int* vp, int* outvp,
                    size_t* startidx, size_t* stopidx, size_t* outidx,
                    size_t max, size_t* outstartp, size_t* outstartidx,
                    size_t* outlinep, size_t* outlineidx) {
  size_t crnt_ridx[LOAD_CSV_VLEN];
#pragma _NEC vreg(crnt_ridx)
  size_t stop_ridx[LOAD_CSV_VLEN];
#pragma _NEC vreg(stop_ridx)
  size_t out_ridx[LOAD_CSV_VLEN];
#pragma _NEC vreg(out_ridx) // doesn't compile with ncc 2.4.1
  size_t outstart_ridx[LOAD_CSV_VLEN];
#pragma _NEC vreg(outstart_ridx)
  uint32_t crnt_state_ridx[LOAD_CSV_VLEN];
#pragma _NEC vreg(crnt_state_ridx)
  size_t outline_ridx[LOAD_CSV_VLEN];
#pragma _NEC vreg(outline_ridx)

  for(size_t i = 0; i < LOAD_CSV_VLEN; i++) {
    crnt_ridx[i] = startidx[i];
    stop_ridx[i] = stopidx[i];
    out_ridx[i] = startidx[i];
    outstart_ridx[i] = startidx[i];
    crnt_state_ridx[i] = csv_state::line_start;
    outline_ridx[i] = outlineidx[i];
  }

#pragma _NEC vob
  for(size_t i = 0; i < max; i++) {
#pragma _NEC ivdep
#pragma _NEC vovertake
    for(size_t j = 0; j < LOAD_CSV_VLEN; j++) {
      if(crnt_ridx[j] != stop_ridx[j]) {
        auto loaded = vp[crnt_ridx[j]];
        auto st = state[crnt_state_ridx[j]][loaded];
        if(st & csv_action::start_line) {
          outlinep[outline_ridx[j]++] = outstart_ridx[j]; // calculated later
        }
        if(st & csv_action::start_entry) {
          outstartp[outstart_ridx[j]++] = out_ridx[j];
        }
        if(st & csv_action::out) {
          outvp[out_ridx[j]++] = loaded;
        }
        crnt_ridx[j]++;
        crnt_state_ridx[j] = st & 0xFFFF;
      }
    }
  }

  for(size_t i = 0; i < LOAD_CSV_VLEN; i++) {
    outidx[i] = out_ridx[i];
    outstartidx[i] = outstart_ridx[i];
  }
}

words parse_csv(const vector<int>& v, const vector<size_t>& start,
                vector<size_t>& line_starts_byword, int separator) {
  uint32_t state[7][256];
  size_t num_chars = 256;
  for(size_t i = 0; i < num_chars; i++) {
    if(i == '\\') {
      state[csv_state::line_start][i] =
        (csv_state::in_escape|csv_action::start_entry|csv_action::start_line);
    } else if(i == '"') {
      state[csv_state::line_start][i] =
        (csv_state::in_quote|csv_action::start_entry|csv_action::start_line);
    } else if(i == separator) {
      state[csv_state::line_start][i] =
        (csv_state::entry_start|csv_action::start_entry|csv_action::start_line);
    } else if(i == '\n') {
      state[csv_state::line_start][i] =
        (csv_state::line_start|csv_action::start_entry|csv_action::start_line);
    } else {
      state[csv_state::line_start][i] =
        (csv_state::in_entry|csv_action::out|csv_action::start_entry|
         csv_action::start_line);
    }
  }
  for(size_t i = 0; i < num_chars; i++) {
    if(i == '\\') {
      state[csv_state::entry_start][i] =
        (csv_state::in_escape|csv_action::start_entry);
    } else if(i == '"') {
      state[csv_state::entry_start][i] =
        (csv_state::in_quote|csv_action::start_entry);
    } else if(i == separator) {
      state[csv_state::entry_start][i] =
        (csv_state::entry_start|csv_action::start_entry);
    } else if(i == '\n') {
      state[csv_state::entry_start][i] =
        (csv_state::line_start|csv_action::start_entry);
    } else {
      state[csv_state::entry_start][i] =
        (csv_state::in_entry|csv_action::out|csv_action::start_entry);
    }
  }
  for(size_t i = 0; i < num_chars; i++) {
    if(i == '\\') state[csv_state::in_entry][i] = csv_state::in_escape;
    else if(i == separator) 
      state[csv_state::in_entry][i] = csv_state::entry_start;
    else if(i == '\n') state[csv_state::in_entry][i] = csv_state::line_start;
    else state[csv_state::in_entry][i] = (csv_state::in_entry|csv_action::out);
  }
  for(size_t i = 0; i < num_chars; i++) {
    state[csv_state::in_escape][i] = (csv_state::in_entry|csv_action::out);
  }
  for(size_t i = 0; i < num_chars; i++) {
    if(i == '\\') state[csv_state::in_quote][i] = csv_state::in_quote_escape;
    else if(i == '"') state[csv_state::in_quote][i] = csv_state::out_quote;
    else state[csv_state::in_quote][i] = (csv_state::in_quote|csv_action::out);
  }
  for(size_t i = 0; i < num_chars; i++) {
    state[csv_state::in_quote_escape][i] =
      (csv_state::in_quote|csv_action::out);
  }
  for(size_t i = 0; i < num_chars; i++) {
    if(i == separator) state[csv_state::out_quote][i] = csv_state::entry_start;
    else if(i == '\n') state[csv_state::out_quote][i] = csv_state::line_start;
    else if(i == '"')
      state[csv_state::out_quote][i] = (csv_state::in_quote|csv_action::out);
    else  // invalid
      state[csv_state::out_quote][i] = (csv_state::out_quote|csv_action::out);
  }

  auto v_size = v.size();
  vector<int> outv(v_size);
  auto vp = v.data();
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
  auto startp = start.data();
  auto start_size = start.size();
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
  vector<size_t> startline(start_size);
  auto startlinep = startline.data();
  parse_csv_vreg(state, vp, outvp, startidx, stopidx, outidx, max,
                 outstartp, outstartidx, startlinep, startlineidx);
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
  for(size_t i = 0; i < totalwords-1; i++) {
    retlensp[i] = retstartsp[i+1] - retstartsp[i];
  }
  retlensp[totalwords-1] = totalchars - retstartsp[totalwords-1];
  auto crnt_startlinep = startlinep;
  off = 0;
  for(size_t i = 0; i < LOAD_CSV_VLEN-1; i++) {
    auto size = startlineidx[i+1] - startlineidx[i];
    for(size_t j = 0; j < size; j++) {
      crnt_startlinep[j] = crnt_startlinep[j] - startidx[i] + off;
    }
    off += outstartidx[i] - startidx[i];
    crnt_startlinep += size;
  }
  for(size_t j = 0; j < start_size - startlineidx[LOAD_CSV_VLEN-1]; j++) {
    crnt_startlinep[j] = crnt_startlinep[j] - startidx[LOAD_CSV_VLEN-1] + off;
  }
  line_starts_byword.resize(start_size);
  auto line_starts_bywordp = line_starts_byword.data();
  for(size_t i = 0; i < start_size; i++) {
    line_starts_bywordp[i] = startlinep[i];
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
  if(to_skip_head) lv.mapv(skip_head, start);
  return lv.map(parse_csv, start, line_starts_byword, broadcast(separator));
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
  if(to_skip_head) lv.mapv(skip_head, start);
  auto ret = lv.map(split_simple_csv, start, broadcast(separator));
  line_starts_byword = start.map(convert_line_starts, ret);
  return ret;
}

}
