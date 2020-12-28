#include "crs_matrix.hpp"
#include "../core/lower_bound.hpp"

namespace frovedis {

template <>
float strtox<float>(char* s, char** next) {
  return strtof(s, next);
}

template <>
double strtox<double>(char* s, char** next) {
  return strtod(s, next);
}

template <>
int strtox<int>(char* s, char** next) {
  return strtol(s, next, 10);
}

#ifdef __ve__

void crs_matrix_split_val_idx(const std::vector<int>& v,
                              std::vector<size_t>& starts, // -> idx
                              std::vector<size_t>& lens, // -> idx
                              std::vector<size_t>& val_starts,
                              std::vector<size_t>& val_lens,
                              std::vector<size_t>& line_starts) { // -> off
  auto zero_lens = vector_find_zero(lens); 
  auto zero_lens_size = zero_lens.size();
  if(zero_lens_size != 0) {
    // zero len should be null line; check for fail safe
    auto zero_lensp = zero_lens.data();
    // 1st char might be LF; treat separately
    size_t zero_lens_pos_size =
      zero_lens[0] == 0 ? zero_lens_size - 1 : zero_lens_size;
    std::vector<size_t> zero_lens_pos(zero_lens_pos_size);
    auto zero_lens_posp = zero_lens_pos.data();
    auto startsp = starts.data();
    auto zero_lens_shiftp = zero_lens[0] == 0 ? zero_lensp + 1 : zero_lensp;
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < zero_lens_pos_size; i++) { 
      zero_lens_posp[i] = startsp[zero_lens_shiftp[i]]-1;
    }
    auto not_lf = find_condition_index(v, zero_lens_pos, is_not_lf<int>());
    // though internal or tailing space is detected,
    // heading space of a line is not detected (except 1st line)
    if(not_lf.size() != 0 || (zero_lens[0] == 0 && v[0] == ' '))
      throw std::runtime_error
        ("crs_matrix: format error. please check if there is no tailing space");

    auto not_zero_lens = vector_find_nonzero(lens);
    auto not_zero_lensp = not_zero_lens.data();
    auto not_zero_lens_size = not_zero_lens.size();
    std::vector<size_t> new_starts(not_zero_lens_size);
    std::vector<size_t> new_lens(not_zero_lens_size);
    auto new_startsp = new_starts.data();
    auto new_lensp = new_lens.data();
    auto lensp = lens.data();
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
    for(size_t i = 0; i < not_zero_lens_size; i++) {
      new_startsp[i] = startsp[not_zero_lensp[i]];
      new_lensp[i] = lensp[not_zero_lensp[i]];
    }
    starts.swap(new_starts);
    lens.swap(new_lens);
    auto zero_lens_line_pos = lower_bound(line_starts, zero_lens);
    auto zero_lens_line_pos_size = zero_lens_line_pos.size();
    auto zero_lens_line_posp = zero_lens_line_pos.data();
    auto line_starts_size = line_starts.size();
    auto line_startsp = line_starts.data();
    for(size_t i = 0; i < zero_lens_line_pos_size-1; i++) {
      auto start = zero_lens_line_posp[i] + 1;
      auto end = zero_lens_line_posp[i+1] + 1;
      for(size_t j = start; j < end; j++) {
        line_startsp[j] -= (i+1);
      }
    }
    auto start = zero_lens_line_posp[zero_lens_line_pos_size-1] + 1;
    auto end = line_starts_size;
    for(size_t j = start; j < end; j++) {
      line_startsp[j] -= zero_lens_line_pos_size;
    }
  }

  auto colon = find_condition(v, is_colon<int>());
  auto colonp = colon.data();
  auto size = starts.size();
  val_starts.resize(size);
  val_lens.resize(size);
  auto val_startsp = val_starts.data();
  auto val_lensp = val_lens.data();
  auto startsp = starts.data();
  auto lensp = lens.data();
  for(size_t i = 0; i < size; i++) {
    auto len = lensp[i];
    auto idx_len = colonp[i] - startsp[i];
    lensp[i] = idx_len;
    val_startsp[i] = colonp[i] + 1;
    val_lensp[i] = len - idx_len - 1;
  }
}


#endif

}
