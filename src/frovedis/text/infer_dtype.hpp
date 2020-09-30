#ifndef INFER_DATA_TYPE_HPP
#define INFER_DATA_TYPE_HPP

#include "parsefloat.hpp"

namespace frovedis {

enum inferred_dtype {
  inferred_dtype_int,
  inferred_dtype_float,
  inferred_dtype_string
};

inferred_dtype infer_dtype(const words& w, size_t words_to_see = 1024);
inferred_dtype infer_dtype(const std::vector<int>& chars,
                           const std::vector<size_t>& starts,
                           const std::vector<size_t>& lens,
                           size_t words_to_see = 1024);
}
#endif
