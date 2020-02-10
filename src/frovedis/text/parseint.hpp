#ifndef PARSEINT_HPP
#define PARSEINT_HPP

#include "words.hpp"
#include <vector>
#include <stdexcept>
#include <limits>

#if defined(__ve__) || defined(_SX)
#define PARSEINT_VLEN 256
#else
//#define PARSEINT_VLEN 1
#define PARSEINT_VLEN 4
#endif


namespace frovedis {

template <class T>
void parseint(const int* charsp, const size_t* startsp,
              const size_t* lensp, size_t num_words, T* retp) {
                        
  auto block = num_words / PARSEINT_VLEN;
  auto rest = num_words - block * PARSEINT_VLEN;

  size_t starts_buf[PARSEINT_VLEN];
#pragma _NEC vreg(starts_buf)
  size_t lens_buf[PARSEINT_VLEN];
#pragma _NEC vreg(lens_buf)
  size_t stops_buf[PARSEINT_VLEN];
#pragma _NEC vreg(stops_buf)
  T res_buf[PARSEINT_VLEN];
#pragma _NEC vreg(res_buf)
  T sign_buf[PARSEINT_VLEN];
#pragma _NEC vreg(sign_buf)

  for(size_t b = 0; b < block; b++) {
    auto crnt_startsp = startsp + b * PARSEINT_VLEN;
    auto crnt_lensp = lensp + b * PARSEINT_VLEN;
    auto crnt_retp = retp + b * PARSEINT_VLEN;
    for(size_t i = 0; i < PARSEINT_VLEN; i++) {
      starts_buf[i] = crnt_startsp[i];
      lens_buf[i] = crnt_lensp[i];
      stops_buf[i] = starts_buf[i] + lens_buf[i];
      sign_buf[i] = 1;
      res_buf[i] = 0;
    }
    for(size_t i = 0; i < PARSEINT_VLEN; i++) {
      if(lens_buf[i] > 0) {
        auto c = charsp[starts_buf[i]];
        if(c == '+') {
          starts_buf[i]++;
          lens_buf[i]--;
        } else if (c == '-') {
          starts_buf[i]++;
          lens_buf[i]--;
          sign_buf[i] = -1;
        }
      }
    }
    size_t max = 0;
    for(size_t i = 0; i < PARSEINT_VLEN; i++) {
      if(lens_buf[i] > max) max = lens_buf[i];
    }
    for(size_t it = 0; it < max; it++) {
      for(size_t i = 0; i < PARSEINT_VLEN; i++) {
        if(starts_buf[i] != stops_buf[i]) {
          res_buf[i] = res_buf[i] * 10 + charsp[starts_buf[i]++] - '0';
        }
      }
    }
    for(size_t i = 0; i < PARSEINT_VLEN; i++) {
      crnt_retp[i] = res_buf[i] * sign_buf[i];
    }
  }

  size_t starts_buf2[PARSEINT_VLEN];
//#pragma _NEC vreg(starts_buf2)
  size_t lens_buf2[PARSEINT_VLEN];
//#pragma _NEC vreg(lens_buf2)
  size_t stops_buf2[PARSEINT_VLEN];
//#pragma _NEC vreg(stops_buf2)
  T res_buf2[PARSEINT_VLEN];
//#pragma _NEC vreg(res_buf2)
  T sign_buf2[PARSEINT_VLEN];
//#pragma _NEC vreg(sign_buf2)

  auto crnt_startsp = startsp + block * PARSEINT_VLEN;
  auto crnt_lensp = lensp + block * PARSEINT_VLEN;
  auto crnt_retp = retp + block * PARSEINT_VLEN;
  for(size_t i = 0; i < rest; i++) {
    starts_buf2[i] = crnt_startsp[i];
    lens_buf2[i] = crnt_lensp[i];
    stops_buf2[i] = starts_buf2[i] + lens_buf2[i];
    sign_buf2[i] = 1;
    res_buf2[i] = 0;
  }
  for(size_t i = 0; i < rest; i++) {
    if(lens_buf2[i] > 0) {
      auto c = charsp[starts_buf2[i]];
      if(c == '+') {
        starts_buf2[i]++;
        lens_buf2[i]--;
      } else if (c == '-') {
        starts_buf2[i]++;
        lens_buf2[i]--;
        sign_buf2[i] = -1;
      }
    }
  }
  size_t max = 0;
  for(size_t i = 0; i < rest; i++) {
    if(lens_buf2[i] > max) max = lens_buf2[i];
  }
  for(size_t it = 0; it < max; it++) {
    for(size_t i = 0; i < rest; i++) {
      if(starts_buf2[i] != stops_buf2[i]) {
        res_buf2[i] = res_buf2[i] * 10 + charsp[starts_buf2[i]++] - '0';
      }
    }
  }
  for(size_t i = 0; i < rest; i++) {
    crnt_retp[i] = res_buf2[i] * sign_buf2[i];
  }
}

template <class T>
std::vector<T> parseint(const words& w) {
  if(!std::numeric_limits<T>::is_integer)
    throw std::runtime_error("not integer type");
  // w.trim(" "); // at this moment, commented out
  auto charsp = w.chars.data();
  auto startsp = w.starts.data();
  auto lensp = w.lens.data();
  auto num_words = w.starts.size();
  std::vector<T> ret(num_words);
  parseint(charsp, startsp, lensp, num_words, ret.data());
  return ret;
}

template <class T>
std::vector<T> parseint(const std::vector<int>& chars,
                        const std::vector<size_t>& starts,
                        const std::vector<size_t>& lens) {
  if(!std::numeric_limits<T>::is_integer)
    throw std::runtime_error("not integer type");
  // w.trim(" "); // at this moment, commented out
  auto charsp = chars.data();
  auto startsp = starts.data();
  auto lensp = lens.data();
  auto num_words = starts.size();
  std::vector<T> ret(num_words);
  parseint(charsp, startsp, lensp, num_words, ret.data());
  return ret;
}


}
#endif
