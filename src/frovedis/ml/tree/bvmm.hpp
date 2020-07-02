#ifndef _BVMM_HPP_
#define _BVMM_HPP_

#include <algorithm>
#include <climits>
#include <cstdint>
#include <vector>

#include "../../../frovedis.hpp"
#include "../../matrix/colmajor_matrix.hpp"
#include "../../matrix/sliced_matrix.hpp"

#include "pragmas.hpp"
#include "bitcount.hpp"
#include "tree_assert.hpp"

namespace frovedis {
namespace tree {

#if defined(_SX) || defined(__ve__)
constexpr size_t __max_vlen = 256;
#endif

template <typename T, enable_if_uint<T> = nullptr>
constexpr size_t bitwidth_of() { return sizeof(T) * CHAR_BIT; }

template <typename T, typename U = uint64_t>
class bitpack_helper {
  size_t _num_blocks, _eff_width, _last_vlen;

public:
  bitpack_helper(const size_t slice_width) :
#if defined(_SX) || defined(__ve__)
    _num_blocks(std::max(__max_vlen, ceil_div(slice_width, bitwidth()))),
#else
    _num_blocks(ceil_div(slice_width, bitwidth())),
#endif
    _eff_width(ceil_div(slice_width, _num_blocks)),
    _last_vlen(((slice_width + _num_blocks - 1) % _num_blocks) + 1)
  {
    tree_assert(0 < _eff_width && _eff_width <= bitwidth());
    tree_assert(0 < _last_vlen && _last_vlen <= _num_blocks);
  }

  size_t get_num_blocks() const { return _num_blocks; }
  std::vector<U> operator()(const T*, const size_t, const size_t) const;

private:
  static constexpr size_t bitwidth() { return bitwidth_of<U>(); }

  void pack(
    const T* src, U* dst,
    const size_t num_blocks, const size_t leading_dim,
    const size_t j, const size_t t, const size_t k
  ) const {
    dst[j * num_blocks + k] += static_cast<U>(
      src[j * leading_dim + t * num_blocks + k] != T(0)
    ) << t;
  }
};

template <typename T, typename U>
inline std::vector<U> bitpack_helper<T, U>::operator()(
  const T* src, const size_t num_cols, const size_t leading_dim
) const {
  const size_t num_blocks = _num_blocks;
  if (num_blocks == 0) { return std::vector<U>(); }

  const size_t last_bit = _eff_width - 1;
  const size_t last_vlen = _last_vlen;
  std::vector<U> ret(num_blocks * num_cols, 0);
  U* dst = ret.data();

  for (size_t j = 0; j < num_cols; j++) {
_Pragma(__outerloop_unroll__)
    for (size_t t = 0; t < last_bit; t++) {
      for (size_t k = 0; k < num_blocks; k++) {
        pack(src, dst, num_blocks, leading_dim, j, t, k);
      }
    }
    /* last one vector */ {
      for (size_t k = 0; k < last_vlen; k++) {
        pack(src, dst, num_blocks, leading_dim, j, last_bit, k);
      }
    }
  }

  return ret;
}

// Transposed-X * Slicing-Y with Bit-Vector Packing
template <typename T, typename U = uint64_t>
inline void bvmm_txsy(
  const colmajor_matrix_local<T>& x,
  const colmajor_matrix_local<T>& y,
  sliced_colmajor_matrix_local<T>& sz,
  const size_t slice_start, const size_t slice_width
) {
  const size_t num_xcols = x.local_num_col;
  const size_t num_ycols = y.local_num_col;
  const size_t xlead = x.local_num_row;
  const size_t ylead = y.local_num_row;
  tree_assert(slice_width > 0);
  tree_assert(slice_width == x.local_num_row);
  tree_assert(slice_start < y.local_num_row);
  tree_assert(slice_width <= y.local_num_row - slice_start);
  tree_assert(num_xcols > 0);
  tree_assert(num_ycols > 0);
  tree_assert(num_xcols == sz.local_num_row);
  tree_assert(num_ycols == sz.local_num_col);
  tree_assert(sz.ldm == sz.local_num_row);

  const bitpack_helper<T, U> packer(slice_width);
  const size_t num_blocks = packer.get_num_blocks();

  const T* xp = x.val.data();
  const T* yp = y.val.data() + slice_start;
  T* zp = sz.data;

  std::vector<U> x_packed = packer(xp, num_xcols, xlead);
  std::vector<U> y_packed = packer(yp, num_ycols, ylead);
  const U* xpck = x_packed.data();
  const U* ypck = y_packed.data();

  std::vector<U> mm_result(num_xcols * num_ycols, 0);
  const size_t mlead = num_xcols;
  U* mmp = mm_result.data();

  // note: these "noloopchg" pragmas disallow loop strip mining
#pragma cdir noloopchg
  for (size_t k = 0; k < num_ycols; k++) {
#pragma cdir noloopchg
_Pragma(__outerloop_unroll__)
    for (size_t j = 0; j < num_xcols; j++) {
#pragma cdir noloopchg
      for (size_t i = 0; i < num_blocks; i++) {
        mmp[k * mlead + j] += popcount(
          xpck[j * num_blocks + i] & ypck[k * num_blocks + i]
        );
      }
    }
  }

#ifdef __BVMM_ZLEAD_MAYBE_NEQ_NUM_ROWS__
  const size_t zlead = sz.ldm;
  if (zlead == sz.local_num_row)
#endif
  {
    const size_t zn = mm_result.size();
    for (size_t i = 0; i < zn; i++) { zp[i] += static_cast<T>(mmp[i]); }
  }
#ifdef __BVMM_ZLEAD_MAYBE_NEQ_NUM_ROWS__
  else {
    for (size_t k = 0; k < num_ycols; k++) {
      for (size_t j = 0; j < num_xcols; j++) {
        zp[k * zlead + j] += static_cast<T>(mmp[k * mlead + j]);
      }
    }
  }
#endif
}

} // end namespace tree
} // end namespace frovedis

#endif
