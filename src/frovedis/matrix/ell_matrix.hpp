#ifndef ELL_MATRIX_HPP
#define ELL_MATRIX_HPP

#include "crs_matrix.hpp"
#include <algorithm>

#define ELL_VLEN 256

namespace frovedis {

template <class T, class I = size_t>
struct ell_matrix_local {
  template <class O>
  ell_matrix_local(crs_matrix_local<T,I,O>& crs); 

  ell_matrix_local() {}
  ell_matrix_local(ell_matrix_local<T,I>&& ell) {
    val.swap(ell.val);
    idx.swap(ell.idx);
    local_num_row = ell.local_num_row;
    local_num_col = ell.local_num_col;
  }
  ell_matrix_local(const ell_matrix_local<T,I>& m) {
    val = m.val;
    idx = m.idx;
    local_num_row = m.local_num_row;
    local_num_col = m.local_num_col;
  }
  ell_matrix_local<T,I>& operator=(const ell_matrix_local<T,I>& m) {
    val = m.val;
    idx = m.idx;
    local_num_row = m.local_num_row;
    local_num_col = m.local_num_col;
    return *this;
  }
  ell_matrix_local<T,I>& operator=(ell_matrix_local<T,I>&& m) {
    val.swap(m.val);
    idx.swap(m.idx);
    local_num_row = m.local_num_row;
    local_num_col = m.local_num_col;
    return *this;
  }
  void debug_print() const {
    std::cout << "local_num_row = " << local_num_row
              << ", local_num_col = " << local_num_col
              << std::endl;
    std::cout << "val : ";
    for(auto i: val) std::cout << i << " ";
    std::cout << std::endl;
    std::cout << "idx : ";
    for(auto i: idx) std::cout << i << " ";
    std::cout << std::endl;
  }
  template <class O = size_t>
  crs_matrix_local<T,I,O> to_crs();
  std::vector<T> val;
  std::vector<I> idx;
  size_t local_num_row, local_num_col;

  SERIALIZE(val, idx, local_num_row, local_num_col)
};

template <class T, class I>
template <class O>
ell_matrix_local<T,I>::ell_matrix_local(crs_matrix_local<T,I,O>& crs) {
  if(crs.local_num_row == 0) {
    local_num_row = 0;
    local_num_col = 0;
    return;
  }
  local_num_row = crs.local_num_row;
  local_num_col = crs.local_num_col;
  std::vector<O> col_sizes(local_num_row);
  for(size_t i = 0; i < local_num_row; i++) {
    col_sizes[i] = crs.off[i+1] - crs.off[i];
  }
  O physical_num_col
    = *std::max_element(col_sizes.begin(), col_sizes.end());
  val.resize(local_num_row * physical_num_col);
  idx.resize(local_num_row * physical_num_col);
  T* valp = &val[0];
  I* idxp = &idx[0];
  T* crsvalp = &crs.val[0];
  I* crsidxp = &crs.idx[0];
  O* crsoffp = &crs.off[0];
  for(size_t r = 0; r < local_num_row; r++) {
#pragma cdir nodep
#pragma _NEC ivdep
    for(O c = 0; c < col_sizes[r]; c++) {
      valp[local_num_row * c + r] = crsvalp[crsoffp[r] + c];
      idxp[local_num_row * c + r] = crsidxp[crsoffp[r] + c];
    }
  }
}

template <class T, class I>
template <class O>
crs_matrix_local<T,I,O> ell_matrix_local<T,I>::to_crs() {
  crs_matrix_local<T,I,O> ret;
  ret.local_num_row = local_num_row;
  ret.local_num_col = local_num_col;
  size_t physical_num_col = val.size() / local_num_row;
  for(size_t r = 0; r < local_num_row; r++) {
    for(size_t c = 0; c < physical_num_col; c++) {
      if(val[local_num_row * c + r] != 0) {
        ret.val.push_back(val[local_num_row * c + r]);
        ret.idx.push_back(idx[local_num_row * c + r]);
      }
    }
    ret.off.push_back(ret.val.size());
  }
  return ret;
}

template <class T, class I, class O>
ell_matrix_local<T,I> crs2ell(crs_matrix_local<T,I,O>& crs) {
  return ell_matrix_local<T,I>(crs);
}

template <class T, class I, class O = size_t>
crs_matrix_local<T,I,O> ell2crs(ell_matrix_local<T,I>& ell) {
  return ell.template to_crs<O>();
}

template <class T, class I = size_t>
struct ell_matrix {
  ell_matrix(){}
  template <class O>
  ell_matrix(crs_matrix<T,I,O>& crs)
    : data(crs.data.map(crs2ell<T,I,O>)), num_row(crs.num_row),
      num_col(crs.num_col) {}

  template <class O = size_t>
  crs_matrix<T,I,O> to_crs() {
    crs_matrix<T,I,O> ret(data.map(ell2crs<T,I,O>));
    ret.set_num(num_row, num_col);
    return ret;
  }
  void debug_print() {
    std::cout << "num_row = " << num_row
              << ", num_col = " << num_col << std::endl;
    auto g = data.gather();
    for(size_t i = 0; i < g.size(); i++) {
      std::cout << "node " << i << std::endl;
      g[i].debug_print();
    }
  }
  frovedis::node_local<ell_matrix_local<T,I>> data;
  size_t num_row;
  size_t num_col;
};

/*
template <class T, class I>
std::vector<T> operator*(const ell_matrix_local<T,I>& mat,
                         const std::vector<T>& v) {
  std::vector<T> ret(mat.local_num_row);
  T* retp = &ret[0];
  const T* vp = &v[0];
  const T* valp = &mat.val[0];
  const I* idxp = &mat.idx[0];
  size_t num_blk = mat.local_num_row / ELL_VLEN;
  size_t blk_len = mat.val.size() / mat.local_num_row;
  size_t stride = mat.local_num_row;
  for(size_t i = 0; i < num_blk; i++) {
    for(size_t j = 0; j < blk_len; j++) {
#pragma cdir on_adb(retp)
#pragma cdir on_adb(vp)
      for(size_t k = 0; k < ELL_VLEN; k++) {
        size_t pos = stride * j + ELL_VLEN * i + k;
        size_t respos = ELL_VLEN * i + k;
        retp[respos] = retp[respos] + valp[pos] * vp[idxp[pos]];
      }
    }
  }
  for(size_t j = 0; j < blk_len; j++) {
    size_t rest = mat.local_num_row - num_blk * ELL_VLEN;
#pragma cdir on_adb(retp)
#pragma cdir on_adb(vp)
      for(size_t k = 0; k < rest; k++) {
        size_t pos = stride * j + ELL_VLEN * num_blk + k;
        size_t respos = ELL_VLEN * num_blk + k;
        retp[respos] = retp[respos] + valp[pos] * vp[idxp[pos]];
      }
  }
  return ret;
}
*/

template <class T, class I>
std::vector<T> operator*(const ell_matrix_local<T,I>& mat,
                         const std::vector<T>& v) {
  std::vector<T> ret(mat.local_num_row);
  T* retp = &ret[0];
  const T* vp = &v[0];
  const T* valp = &mat.val[0];
  const I* idxp = &mat.idx[0];
  size_t num_blk = mat.local_num_row / ELL_VLEN;
  size_t blk_len = mat.val.size() / mat.local_num_row;
  size_t stride = mat.local_num_row;
  for(size_t i = 0; i < num_blk; i++) {
    size_t j = 0;
    for(; j+15 < blk_len; j+=16) {
#pragma cdir on_adb(retp)
#pragma cdir on_adb(vp)
      for(size_t k = 0; k < ELL_VLEN; k++) {
        size_t pos0 = stride * j + ELL_VLEN * i + k;
        size_t pos1 = stride * (j+1) + ELL_VLEN * i + k;
        size_t pos2 = stride * (j+2) + ELL_VLEN * i + k;
        size_t pos3 = stride * (j+3) + ELL_VLEN * i + k;
        size_t pos4 = stride * (j+4) + ELL_VLEN * i + k;
        size_t pos5 = stride * (j+5) + ELL_VLEN * i + k;
        size_t pos6 = stride * (j+6) + ELL_VLEN * i + k;
        size_t pos7 = stride * (j+7) + ELL_VLEN * i + k;
        size_t pos8 = stride * (j+8) + ELL_VLEN * i + k;
        size_t pos9 = stride * (j+9) + ELL_VLEN * i + k;
        size_t pos10 = stride * (j+10) + ELL_VLEN * i + k;
        size_t pos11 = stride * (j+11) + ELL_VLEN * i + k;
        size_t pos12 = stride * (j+12) + ELL_VLEN * i + k;
        size_t pos13 = stride * (j+13) + ELL_VLEN * i + k;
        size_t pos14 = stride * (j+14) + ELL_VLEN * i + k;
        size_t pos15 = stride * (j+15) + ELL_VLEN * i + k;
        size_t respos = ELL_VLEN * i + k;
        retp[respos] +=
          valp[pos0] * vp[idxp[pos0]] +
          valp[pos1] * vp[idxp[pos1]] + 
          valp[pos2] * vp[idxp[pos2]] + 
          valp[pos3] * vp[idxp[pos3]] + 
          valp[pos4] * vp[idxp[pos4]] + 
          valp[pos5] * vp[idxp[pos5]] + 
          valp[pos6] * vp[idxp[pos6]] + 
          valp[pos7] * vp[idxp[pos7]] + 
          valp[pos8] * vp[idxp[pos8]] + 
          valp[pos9] * vp[idxp[pos9]] + 
          valp[pos10] * vp[idxp[pos10]] + 
          valp[pos11] * vp[idxp[pos11]] + 
          valp[pos12] * vp[idxp[pos12]] + 
          valp[pos13] * vp[idxp[pos13]] + 
          valp[pos14] * vp[idxp[pos14]] + 
          valp[pos15] * vp[idxp[pos15]];
      }
    }
    for(; j+7 < blk_len; j+=8) {
#pragma cdir on_adb(retp)
#pragma cdir on_adb(vp)
      for(size_t k = 0; k < ELL_VLEN; k++) {
        size_t pos0 = stride * j + ELL_VLEN * i + k;
        size_t pos1 = stride * (j+1) + ELL_VLEN * i + k;
        size_t pos2 = stride * (j+2) + ELL_VLEN * i + k;
        size_t pos3 = stride * (j+3) + ELL_VLEN * i + k;
        size_t pos4 = stride * (j+4) + ELL_VLEN * i + k;
        size_t pos5 = stride * (j+5) + ELL_VLEN * i + k;
        size_t pos6 = stride * (j+6) + ELL_VLEN * i + k;
        size_t pos7 = stride * (j+7) + ELL_VLEN * i + k;
        size_t respos = ELL_VLEN * i + k;
        retp[respos] +=
          valp[pos0] * vp[idxp[pos0]] +
          valp[pos1] * vp[idxp[pos1]] + 
          valp[pos2] * vp[idxp[pos2]] + 
          valp[pos3] * vp[idxp[pos3]] + 
          valp[pos4] * vp[idxp[pos4]] + 
          valp[pos5] * vp[idxp[pos5]] + 
          valp[pos6] * vp[idxp[pos6]] + 
          valp[pos7] * vp[idxp[pos7]];
      }
    }
    for(; j+3 < blk_len; j+=4) {
#pragma cdir on_adb(retp)
#pragma cdir on_adb(vp)
      for(size_t k = 0; k < ELL_VLEN; k++) {
        size_t pos0 = stride * j + ELL_VLEN * i + k;
        size_t pos1 = stride * (j+1) + ELL_VLEN * i + k;
        size_t pos2 = stride * (j+2) + ELL_VLEN * i + k;
        size_t pos3 = stride * (j+3) + ELL_VLEN * i + k;
        size_t respos = ELL_VLEN * i + k;
        retp[respos] +=
          valp[pos0] * vp[idxp[pos0]] +
          valp[pos1] * vp[idxp[pos1]] + 
          valp[pos2] * vp[idxp[pos2]] + 
          valp[pos3] * vp[idxp[pos3]];
      }
    }
    for(; j+1 < blk_len; j+=2) {
#pragma cdir on_adb(retp)
#pragma cdir on_adb(vp)
      for(size_t k = 0; k < ELL_VLEN; k++) {
        size_t pos0 = stride * j + ELL_VLEN * i + k;
        size_t pos1 = stride * (j+1) + ELL_VLEN * i + k;
        size_t respos = ELL_VLEN * i + k;
        retp[respos] +=
          valp[pos0] * vp[idxp[pos0]] +
          valp[pos1] * vp[idxp[pos1]];
      }
    }
    for(; j < blk_len; j++) {
#pragma cdir on_adb(retp)
#pragma cdir on_adb(vp)
      for(size_t k = 0; k < ELL_VLEN; k++) {
        size_t pos = stride * j + ELL_VLEN * i + k;
        size_t respos = ELL_VLEN * i + k;
        retp[respos] += valp[pos] * vp[idxp[pos]];
      }
    }
  }
  for(size_t j = 0; j < blk_len; j++) {
    size_t rest = mat.local_num_row - num_blk * ELL_VLEN;
#pragma cdir on_adb(retp)
#pragma cdir on_adb(vp)
      for(size_t k = 0; k < rest; k++) {
        size_t pos = stride * j + ELL_VLEN * num_blk + k;
        size_t respos = ELL_VLEN * num_blk + k;
        retp[respos] += valp[pos] * vp[idxp[pos]];
      }
  }
  return ret;
}

template <class T, class I>
std::vector<T> trans_mv(const ell_matrix_local<T,I>& mat,
                        const std::vector<T>& v) {
  std::vector<T> ret(mat.local_num_col);
  T* retp = &ret[0];
  const T* vp = &v[0];
  const T* valp = &mat.val[0];
  const I* idxp = &mat.idx[0];
  size_t stride = mat.local_num_row;
  size_t physical_num_col = mat.val.size() / mat.local_num_row;
  for(size_t r = 0; r < mat.local_num_row; r++) {
#pragma cdir on_adb(valp)
#pragma cdir on_adb(idxp)
#pragma cdir on_adb(v)
#pragma cdir on_adb(retp)
#pragma cdir nodep
#pragma _NEC ivdep
    for(size_t pc = 0; pc < physical_num_col; pc++) {
      size_t ellpos = pc * stride + r;
      retp[idxp[ellpos]] += valp[ellpos] * vp[r];
    }
  }
  return ret;
}

template <class T, class I>
std::vector<T> call_ell_mv(const ell_matrix_local<T,I>& mat,
                           const std::vector<T>& v) {
  return mat * v;
}

template <class T, class I>
dvector<T> operator*(ell_matrix<T,I>& mat, dvector<T>& dv) {
#ifdef SPARSE_MV_USE_ALLREDUCE
  auto dvsize = dv.size();
  node_local<std::vector<T>> bdv;
  // TODO: support larger size of allgather at dvector
  if(dvsize < INT_MAX) { 
    auto sizes = dv.sizes();
    size_t size = sizes.size();
    std::vector<int> count(size);
    for(size_t i = 0; i < size; i++) count[i] = sizes[i]; // cast to int
    bdv = dv.viewas_node_local().map(call_allgatherv<T>(count));
  } else {
    bdv = broadcast(dv.gather());
  }
#else
  auto bdv = broadcast(dv.gather());
#endif
  return mat.data.map(call_ell_mv<T,I>, bdv).template moveto_dvector<T>();
}

}
#endif
