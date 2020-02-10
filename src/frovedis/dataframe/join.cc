#include <cstddef>
#include <stdexcept>
#include "join.hpp"
#include "../core/set_operations.hpp"

#define SHRINK_MISSED_VLEN 256

namespace frovedis {

std::vector<size_t> shrink_missed(std::vector<size_t>& to_shrink, 
                                  std::vector<size_t>& missed) {
  size_t missedsize = missed.size();
  if(missedsize == 0) return to_shrink;
  size_t to_shrinksize = to_shrink.size();
  if(to_shrinksize > missedsize * SHRINK_MISSED_VLEN) {
    size_t retsize = to_shrinksize - missedsize;
    std::vector<size_t> ret(retsize);
    size_t* to_shrinkp = &to_shrink[0];
    size_t* missedp = &missed[0];
    size_t* retp = &ret[0];
    size_t ret_idx = 0;
    size_t src_idx = 0;
    size_t start = 0;
    size_t end;
    for(size_t i = 0; i < missedsize; i++) {
      end = missedp[i];
      size_t len = end - start;
      for(size_t j = 0; j < len; j++) {
        retp[ret_idx + j]  = to_shrinkp[src_idx + j];
      }
      ret_idx += end - start;
      src_idx += end - start + 1;
      start = end + 1;
    }
    end = to_shrinksize;
    size_t len = end - start;
    for(size_t j = 0; j < len; j++) {
      retp[ret_idx + j]  = to_shrinkp[src_idx + j];
    }
    return ret;
  } else {
    std::vector<size_t> tmp(to_shrinksize);
    size_t* tmpp = &tmp[0];
    for(size_t i = 0; i < to_shrinksize; i++) {
      tmpp[i] = i;
    }
    auto to_shrink_idx = set_difference(tmp, missed);
    size_t* to_shrink_idxp = &to_shrink_idx[0];
    size_t to_shrink_idx_size = to_shrink_idx.size();
    std::vector<size_t> ret(to_shrink_idx_size);
    size_t* retp = &ret[0];
    size_t* to_shrinkp = &to_shrink[0];
    for(size_t i = 0; i < to_shrink_idx_size; i++) {
      retp[i] = to_shrinkp[to_shrink_idxp[i]];
    }
    return ret;
  }
}

}
