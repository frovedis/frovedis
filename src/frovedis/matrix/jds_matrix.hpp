#ifndef JDS_MATRIX_HPP
#define JDS_MATRIX_HPP

#include "crs_matrix.hpp"
#if defined(_SX) || defined(__ve__)
#include "../core/radix_sort.hpp"
#endif

#include "rowmajor_matrix.hpp"

namespace frovedis {

#define JDS_VLEN 256

template <class T, class I = size_t, class O = size_t, class P = size_t>
struct jds_matrix_local {
  jds_matrix_local() {off.push_back(0);}
  jds_matrix_local(const jds_matrix_local<T,I,O,P>& m) {
    val = m.val;
    idx = m.idx;
    off = m.off;
    perm = m.perm;
    local_num_row = m.local_num_row;
    local_num_col = m.local_num_col;
  }
  jds_matrix_local<T,I,O,P>& operator=(const jds_matrix_local<T,I,O,P>& m) {
    val = m.val;
    idx = m.idx;
    off = m.off;
    perm = m.perm;
    local_num_row = m.local_num_row;
    local_num_col = m.local_num_col;
    return *this;
  }
  jds_matrix_local(jds_matrix_local<T,I,O,P>&& m) {
    val.swap(m.val);
    idx.swap(m.idx);
    off.swap(m.off);
    perm.swap(m.perm);
    local_num_row = m.local_num_row;
    local_num_col = m.local_num_col;
  }
  jds_matrix_local<T,I,O,P>& operator=(jds_matrix_local<T,I,O,P>&& m) {
    val.swap(m.val);
    idx.swap(m.idx);
    off.swap(m.off);
    perm.swap(m.perm);
    local_num_row = m.local_num_row;
    local_num_col = m.local_num_col;
    return *this;
  }
  jds_matrix_local(const crs_matrix_local<T,I,O>&);
  std::vector<T> val;
  std::vector<I> idx;
  std::vector<O> off; // off[0] is always 0
  std::vector<P> perm;
  size_t local_num_col;
  size_t local_num_row;
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
    std::cout << "off : ";
    for(auto i: off) std::cout << i << " ";
    std::cout << std::endl;
    std::cout << "perm : ";
    for(auto i: perm) std::cout << i << " ";
    std::cout << std::endl;
  }
  void savebinary(const std::string&); // for testing... do not use
  void clear() {
    std::vector<T> tmpval; tmpval.swap(val);
    std::vector<I> tmpidx; tmpidx.swap(idx);
    std::vector<O> tmpoff; tmpoff.swap(off); off.push_back(0);
    std::vector<P> tmpperm; tmpperm.swap(perm);
    local_num_row = 0;
    local_num_col = 0;
  }

  SERIALIZE(val, idx, off, perm, local_num_col, local_num_row)
};

template <class T, class I, class O, class P>
jds_matrix_local<T,I,O,P>::
jds_matrix_local(const crs_matrix_local<T,I,O>& m) {
  local_num_col = m.local_num_col;
  local_num_row = m.local_num_row;
  if(m.val.size() == 0) {
    off.push_back(0);
    return;
  }
  val.resize(m.val.size());
  idx.resize(m.idx.size());
  perm.resize(local_num_row);
#if defined(_SX) || defined(__ve__)
  std::vector<O> perm_tmp_key(local_num_row);
  std::vector<P> perm_tmp_val(local_num_row);
  auto perm_tmp_keyp = perm_tmp_key.data();
  auto perm_tmp_valp = perm_tmp_val.data();
  auto moffpp = m.off.data();
  for(size_t i = 0; i < local_num_row; i++) {
    perm_tmp_keyp[i] = moffpp[i+1] - moffpp[i];
    perm_tmp_valp[i] = i;
  }
  radix_sort(&perm_tmp_key[0], &perm_tmp_val[0], local_num_row);
  auto permpp = perm.data();
  for(size_t i = 0; i < local_num_row; i++) {
    permpp[i] = perm_tmp_valp[local_num_row - i - 1];
  }
  off.reserve(perm_tmp_key[local_num_row - 1] + 1);
#else 
  std::vector<std::pair<O, P>> perm_tmp(local_num_row);
  for(size_t i = 0; i < local_num_row; i++) {
    perm_tmp[i].first = m.off[i+1] - m.off[i];
    perm_tmp[i].second = i;
  }
  std::sort(perm_tmp.begin(), perm_tmp.end(),
            std::greater<std::pair<O, P>>());
  for(size_t i = 0; i < local_num_row; i++) {
    perm[i] = perm_tmp[i].second;
  }
  off.reserve(perm_tmp[0].first+1);
#endif
  off.push_back(0);

  P* permp = &perm[0];
  const T* mvalp = &m.val[0];
  T* valp = &val[0];
  const I* midxp = &m.idx[0];
  I* idxp = &idx[0];
  const O* moffp = &m.off[0];
  size_t jds_col = 0;
  size_t to_store = 0;
  for(size_t row_max = local_num_row; row_max != 0; row_max--) {
#if defined(_SX) || defined(__ve__)
    size_t num_iter = perm_tmp_key[local_num_row - row_max] - jds_col;
#else
    size_t num_iter = perm_tmp[row_max-1].first - jds_col;
#endif
    for(size_t i = 0; i < num_iter; i++, jds_col++) {
      for(size_t r = 0; r < row_max; r++, to_store++) {
        valp[to_store] = mvalp[moffp[permp[r]] + jds_col];
        idxp[to_store] = midxp[moffp[permp[r]] + jds_col];
      }
      off.push_back(to_store);
    }
  }
}

template <class T, class I, class O, class P>
jds_matrix_local<T,I,O,P> crs2jds(crs_matrix_local<T,I,O>& crs) {
  return jds_matrix_local<T,I,O,P>(crs);
}

// for testing...
template <class T, class I, class O, class P>
void jds_matrix_local<T,I,O,P>::savebinary(const std::string& dir) {
  struct stat sb;
  if(stat(dir.c_str(), &sb) != 0) { // no file/directory
    mode_t mode = S_IRWXU | S_IRWXG | S_IRWXO; // man 2 stat
    if(mkdir(dir.c_str(), mode) != 0) {
      perror("mkdir failed:");
      throw std::runtime_error("mkdir failed");
    }
  } else if(!S_ISDIR(sb.st_mode)) {
    throw std::runtime_error(dir + " is not a directory");
  }
  std::string valfile = dir + "/val";
  std::string idxfile = dir + "/idx";
  std::string offfile = dir + "/off";
  std::string permfile = dir + "/perm";
  std::string numsfile = dir + "/nums";
  std::ofstream numstr;
  numstr.exceptions(std::ofstream::failbit | std::ofstream::badbit);
  numstr.open(numsfile.c_str());
  numstr << local_num_row << "\n" << local_num_col << std::endl;
  make_dvector_scatter(val).savebinary(valfile);
  make_dvector_scatter(idx).savebinary(idxfile);
  make_dvector_scatter(off).savebinary(offfile);
  make_dvector_scatter(perm).savebinary(permfile);
}

// for testing...
template <class T, class I = size_t, class O = size_t, class P = size_t>
jds_matrix_local<T,I,O,P>
make_jds_matrix_local_loadbinary(const std::string& input) {
  std::string valfile = input + "/val";
  std::string idxfile = input + "/idx";
  std::string offfile = input + "/off";
  std::string permfile = input + "/perm";
#if defined(_SX) || defined(__ve__)
  std::string numsfile = remove_schema(input + "/nums");
#else  
  std::string numsfile = input + "/nums";
#endif
  std::ifstream numstr;
  numstr.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  numstr.open(numsfile.c_str());
  size_t num_row, num_col;
  numstr >> num_row >> num_col;
  jds_matrix_local<T,I,O,P> ret;
  auto loadval = make_dvector_loadbinary<T>(valfile).gather();
  ret.val.swap(loadval);
  auto loadidx = make_dvector_loadbinary<I>(idxfile).gather();
  ret.idx.swap(loadidx);
  auto loadoff = make_dvector_loadbinary<O>(offfile).gather();
  ret.off.swap(loadoff);
  auto loadperm = make_dvector_loadbinary<P>(permfile).gather();
  ret.perm.swap(loadperm);
  ret.local_num_row = num_row;
  ret.local_num_col = num_col;
  return ret;
}

template <class T, class I = size_t, class O = size_t, class P = size_t>
struct jds_matrix {
  jds_matrix(){}
  jds_matrix(crs_matrix<T,I,O>& crs)
    : data(crs.data.map(crs2jds<T,I,O,P>)), num_row(crs.num_row),
      num_col(crs.num_col)
    {}
  void debug_print() {
    std::cout << "num_row = " << num_row
              << ", num_col = " << num_col << std::endl;
    auto g = data.gather();
    for(size_t i = 0; i < g.size(); i++) {
      std::cout << "node " << i << std::endl;
      g[i].debug_print();
    }
  }
  void clear();
  frovedis::node_local<jds_matrix_local<T,I,O,P>> data;
  size_t num_row;
  size_t num_col;
};

template <class T, class I, class O, class P>
void jds_clear_helper(jds_matrix_local<T,I,O,P>& mat) {mat.clear();}
template <class T, class I, class O, class P>
void jds_matrix<T,I,O,P>::clear() {
  data.mapv(jds_clear_helper<T,I,O,P>);
  num_row = 0;
  num_col = 0;
}

template <class T, class I, class O, class P>
void jds_spmv_helper1(const T* valp, const I* idxp, const O* offp, const T* vp, T* tmpp,
                      O c, O col_offset) {
  O respos = col_offset;
  O len = offp[c+1] - offp[c] - col_offset;
  O pos0 = offp[c] + col_offset;
#pragma cdir on_adb(vp)
#pragma _NEC ivdep
  for(O i = 0; i < len; i++) {
    tmpp[respos + i] +=
      valp[pos0 + i] * vp[idxp[pos0 + i]];
  }
}

template <class T, class I, class O, class P>
void jds_spmv_helper2(const T* valp, const I* idxp, const O* offp, const T* vp, T* tmpp,
                      O c, O col_offset) {
  O respos = col_offset;
  O len = offp[c+2] - offp[c+1] - col_offset;
  O pos0 = offp[c] + col_offset;
  O pos1 = offp[c+1] + col_offset;
#pragma cdir on_adb(vp)
#pragma _NEC ivdep
  for(O i = 0; i < len; i++) {
    tmpp[respos + i] +=
      valp[pos0 + i] * vp[idxp[pos0 + i]] +
      valp[pos1 + i] * vp[idxp[pos1 + i]];
  }
  jds_spmv_helper1<T,I,O,P>(valp, idxp, offp, vp, tmpp, c, col_offset + len);
}

template <class T, class I, class O, class P>
void jds_spmv_helper4(const T* valp, const I* idxp, const O* offp, const T* vp, T* tmpp,
                      O c, O col_offset) {
  O respos = col_offset;
  O len = offp[c+4] - offp[c+3] - col_offset;
  O pos0 = offp[c] + col_offset;
  O pos1 = offp[c+1] + col_offset;
  O pos2 = offp[c+2] + col_offset;
  O pos3 = offp[c+3] + col_offset;
#pragma cdir on_adb(vp)
#pragma _NEC ivdep
  for(O i = 0; i < len; i++) {
    tmpp[respos + i] +=
      valp[pos0 + i] * vp[idxp[pos0 + i]] +
      valp[pos1 + i] * vp[idxp[pos1 + i]] +
      valp[pos2 + i] * vp[idxp[pos2 + i]] +
      valp[pos3 + i] * vp[idxp[pos3 + i]];
  }
  jds_spmv_helper2<T,I,O,P>(valp, idxp, offp, vp, tmpp, c, col_offset + len);
  jds_spmv_helper1<T,I,O,P>(valp, idxp, offp, vp, tmpp, c+2, col_offset + len);
}

template <class T, class I, class O, class P>
void jds_spmv_helper8(const T* valp, const I* idxp, const O* offp, const T* vp, T* tmpp,
                      O c, O col_offset) {
  O respos = col_offset;
  O len = offp[c+8] - offp[c+7] - col_offset;
  O pos0 = offp[c] + col_offset;
  O pos1 = offp[c+1] + col_offset;
  O pos2 = offp[c+2] + col_offset;
  O pos3 = offp[c+3] + col_offset;
  O pos4 = offp[c+4] + col_offset;
  O pos5 = offp[c+5] + col_offset;
  O pos6 = offp[c+6] + col_offset;
  O pos7 = offp[c+7] + col_offset;
#pragma cdir on_adb(vp)
#pragma _NEC ivdep
  for(O i = 0; i < len; i++) {
    tmpp[respos + i] +=
      valp[pos0 + i] * vp[idxp[pos0 + i]] +
      valp[pos1 + i] * vp[idxp[pos1 + i]] +
      valp[pos2 + i] * vp[idxp[pos2 + i]] +
      valp[pos3 + i] * vp[idxp[pos3 + i]] +
      valp[pos4 + i] * vp[idxp[pos4 + i]] +
      valp[pos5 + i] * vp[idxp[pos5 + i]] +
      valp[pos6 + i] * vp[idxp[pos6 + i]] +
      valp[pos7 + i] * vp[idxp[pos7 + i]];
  }
  jds_spmv_helper4<T,I,O,P>(valp, idxp, offp, vp, tmpp, c, col_offset + len);
  jds_spmv_helper2<T,I,O,P>(valp, idxp, offp, vp, tmpp, c+4, col_offset + len);
  jds_spmv_helper1<T,I,O,P>(valp, idxp, offp, vp, tmpp, c+6, col_offset + len);
}

template <class T, class I, class O, class P>
void jds_spmv_helper16(const T* valp, const I* idxp, const O* offp, const T* vp, T* tmpp,
                       O c, O col_offset) {
  O respos = col_offset;
  O len = offp[c+16] - offp[c+15] - col_offset;
  O pos0 = offp[c] + col_offset;
  O pos1 = offp[c+1] + col_offset;
  O pos2 = offp[c+2] + col_offset;
  O pos3 = offp[c+3] + col_offset;
  O pos4 = offp[c+4] + col_offset;
  O pos5 = offp[c+5] + col_offset;
  O pos6 = offp[c+6] + col_offset;
  O pos7 = offp[c+7] + col_offset;
  O pos8 = offp[c+8] + col_offset;
  O pos9 = offp[c+9] + col_offset;
  O pos10 = offp[c+10] + col_offset;
  O pos11 = offp[c+11] + col_offset;
  O pos12 = offp[c+12] + col_offset;
  O pos13 = offp[c+13] + col_offset;
  O pos14 = offp[c+14] + col_offset;
  O pos15 = offp[c+15] + col_offset;
#pragma cdir on_adb(vp)
#pragma _NEC ivdep
  for(O i = 0; i < len; i++) {
    tmpp[respos + i] +=
      valp[pos0 + i] * vp[idxp[pos0 + i]] +
      valp[pos1 + i] * vp[idxp[pos1 + i]] +
      valp[pos2 + i] * vp[idxp[pos2 + i]] +
      valp[pos3 + i] * vp[idxp[pos3 + i]] +
      valp[pos4 + i] * vp[idxp[pos4 + i]] +
      valp[pos5 + i] * vp[idxp[pos5 + i]] +
      valp[pos6 + i] * vp[idxp[pos6 + i]] +
      valp[pos7 + i] * vp[idxp[pos7 + i]] +
      valp[pos8 + i] * vp[idxp[pos8 + i]] +
      valp[pos9 + i] * vp[idxp[pos9 + i]] +
      valp[pos10 + i] * vp[idxp[pos10 + i]] +
      valp[pos11 + i] * vp[idxp[pos11 + i]] +
      valp[pos12 + i] * vp[idxp[pos12 + i]] +
      valp[pos13 + i] * vp[idxp[pos13 + i]] +
      valp[pos14 + i] * vp[idxp[pos14 + i]] +
      valp[pos15 + i] * vp[idxp[pos15 + i]];
  }
  jds_spmv_helper8<T,I,O,P>(valp, idxp, offp, vp, tmpp, c, col_offset + len);
  jds_spmv_helper4<T,I,O,P>(valp, idxp, offp, vp, tmpp, c+8, col_offset + len);
  jds_spmv_helper2<T,I,O,P>(valp, idxp, offp, vp, tmpp, c+12, col_offset + len);
  jds_spmv_helper1<T,I,O,P>(valp, idxp, offp, vp, tmpp, c+14, col_offset + len);
}

template <class T, class I, class O, class P>
void jds_matrix_spmv_impl(const jds_matrix_local<T,I,O,P>& mat,
                          T* retp, const T* vp) {
  std::vector<T> tmp(mat.local_num_row);
  T* tmpp = &tmp[0];
  const T* valp = &mat.val[0];
  const I* idxp = &mat.idx[0];
  const O* offp = &mat.off[0];
  size_t physical_col_size = mat.off.size() - 1;
  size_t group_size = 16;
  size_t num_groups = physical_col_size / group_size;
  for(size_t g = 0; g < num_groups; g++) {
    jds_spmv_helper16<T,I,O,P>(valp, idxp, offp, vp, tmpp, g * group_size, 0);
  }
  size_t c = num_groups * group_size;
  if(c + 7 < physical_col_size) {
    jds_spmv_helper8<T,I,O,P>(valp, idxp, offp, vp, tmpp, c, 0);
    c += 8;
  }
  if(c + 3 < physical_col_size) {
    jds_spmv_helper4<T,I,O,P>(valp, idxp, offp, vp, tmpp, c, 0);
    c += 4;
  }
  if(c + 1 < physical_col_size) {
    jds_spmv_helper2<T,I,O,P>(valp, idxp, offp, vp, tmpp, c, 0);
    c += 2;
  }
  if(c < physical_col_size) {
    jds_spmv_helper1<T,I,O,P>(valp, idxp, offp, vp, tmpp, c, 0);
  }
  const P* permp = &mat.perm[0];
#pragma cdir nodep
#pragma _NEC ivdep
#pragma cdir on_adb(tmpp)
#pragma cdir on_adb(retp)
#pragma cdir on_adb(permp)
  for(size_t i = 0; i < mat.local_num_row; i++) {
    retp[permp[i]] = tmpp[i];
  }
}

template <class T, class I, class O, class P>
std::vector<T> operator*(const jds_matrix_local<T,I,O,P>& mat, const std::vector<T>& v) {
  std::vector<T> ret(mat.local_num_row);
  if(mat.local_num_col != v.size())
    throw std::runtime_error("operator*: size of vector does not match");
  jds_matrix_spmv_impl(mat, ret.data(), v.data());
  return ret;
}

template <class T, class I, class O, class P>
void jds_spmm_helper1(const T* valp, const I* idxp, const O* offp, const T* vvalp, T* tmpvalp,
                      size_t num_col, O c, O col_offset) {
  O respos = col_offset;
  O len = offp[c+1] - offp[c] - col_offset;
  O pos0 = offp[c] + col_offset;
  O block_num = len / JDS_VLEN;
  for(O b = 0; b < block_num; b++) {
    for(size_t mc = 0; mc < num_col; mc++) {
#pragma cdir on_adb(vvalp)
#pragma cdir on_adb(valp)
#pragma cdir on_adb(tmpvalp)
#pragma cdir on_adb(idxp)
#pragma cdir nodep
#pragma _NEC ivdep
      for(O i = 0; i < JDS_VLEN; i++) {
        tmpvalp[(respos + b * JDS_VLEN + i) * num_col + mc] +=
          valp[pos0 + b * JDS_VLEN + i] *
          vvalp[idxp[pos0 + b * JDS_VLEN + i] * num_col + mc];
      }
    }
  }
  for(size_t mc = 0; mc < num_col; mc++) {
#pragma cdir on_adb(vvalp)
#pragma cdir on_adb(valp)
#pragma cdir on_adb(tmpvalp)
#pragma cdir on_adb(idxp)
#pragma cdir nodep
#pragma _NEC ivdep
    for(O i = JDS_VLEN * block_num; i < len; i++) {
      tmpvalp[(respos + i) * num_col + mc] +=
        valp[pos0 + i] * vvalp[idxp[pos0 + i] * num_col + mc];
    }
  }
}

template <class T, class I, class O, class P>
void jds_spmm_helper2(const T* valp, const I* idxp, const O* offp, const T* vvalp, T* tmpvalp,
                      size_t num_col, O c, O col_offset) {
  O respos = col_offset;
  O len = offp[c+2] - offp[c+1] - col_offset;
  O pos0 = offp[c] + col_offset;
  O pos1 = offp[c+1] + col_offset;
  O block_num = len / JDS_VLEN;
  for(O b = 0; b < block_num; b++) {
    for(size_t mc = 0; mc < num_col; mc++) {
#pragma cdir on_adb(vvalp)
#pragma cdir on_adb(valp)
#pragma cdir on_adb(tmpvalp)
#pragma cdir on_adb(idxp)
#pragma cdir nodep
#pragma _NEC ivdep
      for(O i = 0; i < JDS_VLEN; i++) {
        tmpvalp[(respos + b * JDS_VLEN + i) * num_col + mc] +=
          valp[pos0 + b * JDS_VLEN + i] *
          vvalp[idxp[pos0 + b * JDS_VLEN + i] * num_col + mc] +
          valp[pos1 + b * JDS_VLEN + i] *
          vvalp[idxp[pos1 + b * JDS_VLEN + i] * num_col + mc];
      }
    }
  }
  for(size_t mc = 0; mc < num_col; mc++) {
#pragma cdir on_adb(vvalp)
#pragma cdir on_adb(valp)
#pragma cdir on_adb(tmpvalp)
#pragma cdir on_adb(idxp)
#pragma cdir nodep
#pragma _NEC ivdep
    for(O i = JDS_VLEN * block_num; i < len; i++) {
      tmpvalp[(respos + i) * num_col + mc] +=
        valp[pos0 + i] * vvalp[idxp[pos0 + i] * num_col + mc] +
        valp[pos1 + i] * vvalp[idxp[pos1 + i] * num_col + mc];
    }
  }
  jds_spmm_helper1<T,I,O,P>(valp, idxp, offp, vvalp, tmpvalp, num_col, c,
                            col_offset + len);
}

template <class T, class I, class O, class P>
void jds_spmm_helper4(const T* valp, const I* idxp, const O* offp, const T* vvalp, T* tmpvalp,
                      size_t num_col, O c, O col_offset) {
  O respos = col_offset;
  O len = offp[c+4] - offp[c+3] - col_offset;
  O pos0 = offp[c] + col_offset;
  O pos1 = offp[c+1] + col_offset;
  O pos2 = offp[c+2] + col_offset;
  O pos3 = offp[c+3] + col_offset;
  O block_num = len / JDS_VLEN;
  for(O b = 0; b < block_num; b++) {
    for(size_t mc = 0; mc < num_col; mc++) {
#pragma cdir on_adb(vvalp)
#pragma cdir on_adb(valp)
#pragma cdir on_adb(tmpvalp)
#pragma cdir on_adb(idxp)
#pragma cdir nodep
#pragma _NEC ivdep
      for(O i = 0; i < JDS_VLEN; i++) {
        tmpvalp[(respos + b * JDS_VLEN + i) * num_col + mc] +=
          valp[pos0 + b * JDS_VLEN + i] *
          vvalp[idxp[pos0 + b * JDS_VLEN + i] * num_col + mc] +
          valp[pos1 + b * JDS_VLEN + i] *
          vvalp[idxp[pos1 + b * JDS_VLEN + i] * num_col + mc] +
          valp[pos2 + b * JDS_VLEN + i] *
          vvalp[idxp[pos2 + b * JDS_VLEN + i] * num_col + mc] +
          valp[pos3 + b * JDS_VLEN + i] *
          vvalp[idxp[pos3 + b * JDS_VLEN + i] * num_col + mc];
      }
    }
  }
  for(size_t mc = 0; mc < num_col; mc++) {
#pragma cdir on_adb(vvalp)
#pragma cdir on_adb(valp)
#pragma cdir on_adb(tmpvalp)
#pragma cdir on_adb(idxp)
#pragma cdir nodep
#pragma _NEC ivdep
    for(O i = JDS_VLEN * block_num; i < len; i++) {
      tmpvalp[(respos + i) * num_col + mc] +=
        valp[pos0 + i] * vvalp[idxp[pos0 + i] * num_col + mc] +
        valp[pos1 + i] * vvalp[idxp[pos1 + i] * num_col + mc] +
        valp[pos2 + i] * vvalp[idxp[pos2 + i] * num_col + mc] +
        valp[pos3 + i] * vvalp[idxp[pos3 + i] * num_col + mc];
    }
  }
  jds_spmm_helper2<T,I,O,P>(valp, idxp, offp, vvalp, tmpvalp, num_col, c,
                            col_offset + len);
  jds_spmm_helper1<T,I,O,P>(valp, idxp, offp, vvalp, tmpvalp, num_col, c+2,
                            col_offset + len);
}

template <class T, class I, class O, class P>
void jds_spmm_helper8(const T* valp, const I* idxp, const O* offp, const T* vvalp, T* tmpvalp,
                      size_t num_col, O c, O col_offset) {
  O respos = col_offset;
  O len = offp[c+8] - offp[c+7] - col_offset;
  O pos0 = offp[c] + col_offset;
  O pos1 = offp[c+1] + col_offset;
  O pos2 = offp[c+2] + col_offset;
  O pos3 = offp[c+3] + col_offset;
  O pos4 = offp[c+4] + col_offset;
  O pos5 = offp[c+5] + col_offset;
  O pos6 = offp[c+6] + col_offset;
  O pos7 = offp[c+7] + col_offset;
  O block_num = len / JDS_VLEN;
  for(O b = 0; b < block_num; b++) {
    for(size_t mc = 0; mc < num_col; mc++) {
#pragma cdir on_adb(vvalp)
#pragma cdir on_adb(valp)
#pragma cdir on_adb(tmpvalp)
#pragma cdir on_adb(idxp)
#pragma cdir nodep
#pragma _NEC ivdep
      for(O i = 0; i < JDS_VLEN; i++) {
        tmpvalp[(respos + b * JDS_VLEN + i) * num_col + mc] +=
          valp[pos0 + b * JDS_VLEN + i] *
          vvalp[idxp[pos0 + b * JDS_VLEN + i] * num_col + mc] +
          valp[pos1 + b * JDS_VLEN + i] *
          vvalp[idxp[pos1 + b * JDS_VLEN + i] * num_col + mc] +
          valp[pos2 + b * JDS_VLEN + i] *
          vvalp[idxp[pos2 + b * JDS_VLEN + i] * num_col + mc] +
          valp[pos3 + b * JDS_VLEN + i] *
          vvalp[idxp[pos3 + b * JDS_VLEN + i] * num_col + mc] +
          valp[pos4 + b * JDS_VLEN + i] *
          vvalp[idxp[pos4 + b * JDS_VLEN + i] * num_col + mc] +
          valp[pos5 + b * JDS_VLEN + i] *
          vvalp[idxp[pos5 + b * JDS_VLEN + i] * num_col + mc] +
          valp[pos6 + b * JDS_VLEN + i] *
          vvalp[idxp[pos6 + b * JDS_VLEN + i] * num_col + mc] +
          valp[pos7 + b * JDS_VLEN + i] *
          vvalp[idxp[pos7 + b * JDS_VLEN + i] * num_col + mc];
      }
    }
  }
  for(size_t mc = 0; mc < num_col; mc++) {
#pragma cdir on_adb(vvalp)
#pragma cdir on_adb(valp)
#pragma cdir on_adb(tmpvalp)
#pragma cdir on_adb(idxp)
#pragma cdir nodep
#pragma _NEC ivdep
    for(O i = JDS_VLEN * block_num; i < len; i++) {
      tmpvalp[(respos + i) * num_col + mc] +=
        valp[pos0 + i] * vvalp[idxp[pos0 + i] * num_col + mc] +
        valp[pos1 + i] * vvalp[idxp[pos1 + i] * num_col + mc] +
        valp[pos2 + i] * vvalp[idxp[pos2 + i] * num_col + mc] +
        valp[pos3 + i] * vvalp[idxp[pos3 + i] * num_col + mc] +
        valp[pos4 + i] * vvalp[idxp[pos4 + i] * num_col + mc] +
        valp[pos5 + i] * vvalp[idxp[pos5 + i] * num_col + mc] +
        valp[pos6 + i] * vvalp[idxp[pos6 + i] * num_col + mc] +
        valp[pos7 + i] * vvalp[idxp[pos7 + i] * num_col + mc];
    }
  }
  jds_spmm_helper4<T,I,O,P>(valp, idxp, offp, vvalp, tmpvalp, num_col, c,
                            col_offset + len);
  jds_spmm_helper2<T,I,O,P>(valp, idxp, offp, vvalp, tmpvalp, num_col, c+4,
                            col_offset + len);
  jds_spmm_helper1<T,I,O,P>(valp, idxp, offp, vvalp, tmpvalp, num_col, c+6,
                            col_offset + len);
}

/* // unroll 8 is optimal
template <class T, class I, class O, class P>
void jds_spmm_helper16(const T* valp, const I* idxp, const O* offp, const T* vvalp, T* tmpvalp,
                      size_t num_col, O c, O col_offset) {
  O respos = col_offset;
  O len = offp[c+16] - offp[c+15] - col_offset;
  O pos0 = offp[c] + col_offset;
  O pos1 = offp[c+1] + col_offset;
  O pos2 = offp[c+2] + col_offset;
  O pos3 = offp[c+3] + col_offset;
  O pos4 = offp[c+4] + col_offset;
  O pos5 = offp[c+5] + col_offset;
  O pos6 = offp[c+6] + col_offset;
  O pos7 = offp[c+7] + col_offset;
  O pos8 = offp[c+8] + col_offset;
  O pos9 = offp[c+9] + col_offset;
  O pos10 = offp[c+10] + col_offset;
  O pos11 = offp[c+11] + col_offset;
  O pos12 = offp[c+12] + col_offset;
  O pos13 = offp[c+13] + col_offset;
  O pos14 = offp[c+14] + col_offset;
  O pos15 = offp[c+15] + col_offset;
  O block_num = len / JDS_VLEN;
  for(O b = 0; b < block_num; b++) {
    for(size_t mc = 0; mc < num_col; mc++) {
#pragma cdir on_adb(vvalp)
#pragma cdir on_adb(valp)
#pragma cdir on_adb(tmpvalp)
#pragma cdir on_adb(idxp)
#pragma cdir nodep
#pragma _NEC ivdep
      for(O i = 0; i < JDS_VLEN; i++) {
        tmpvalp[(respos + b * JDS_VLEN + i) * num_col + mc] +=
          valp[pos0 + b * JDS_VLEN + i] *
          vvalp[idxp[pos0 + b * JDS_VLEN + i] * num_col + mc] +
          valp[pos1 + b * JDS_VLEN + i] *
          vvalp[idxp[pos1 + b * JDS_VLEN + i] * num_col + mc] +
          valp[pos2 + b * JDS_VLEN + i] *
          vvalp[idxp[pos2 + b * JDS_VLEN + i] * num_col + mc] +
          valp[pos3 + b * JDS_VLEN + i] *
          vvalp[idxp[pos3 + b * JDS_VLEN + i] * num_col + mc] +
          valp[pos4 + b * JDS_VLEN + i] *
          vvalp[idxp[pos4 + b * JDS_VLEN + i] * num_col + mc] +
          valp[pos5 + b * JDS_VLEN + i] *
          vvalp[idxp[pos5 + b * JDS_VLEN + i] * num_col + mc] +
          valp[pos6 + b * JDS_VLEN + i] *
          vvalp[idxp[pos6 + b * JDS_VLEN + i] * num_col + mc] +
          valp[pos7 + b * JDS_VLEN + i] *
          vvalp[idxp[pos7 + b * JDS_VLEN + i] * num_col + mc] +
          valp[pos8 + b * JDS_VLEN + i] *
          vvalp[idxp[pos8 + b * JDS_VLEN + i] * num_col + mc] +
          valp[pos9 + b * JDS_VLEN + i] *
          vvalp[idxp[pos9 + b * JDS_VLEN + i] * num_col + mc] +
          valp[pos10 + b * JDS_VLEN + i] *
          vvalp[idxp[pos10 + b * JDS_VLEN + i] * num_col + mc] +
          valp[pos11 + b * JDS_VLEN + i] *
          vvalp[idxp[pos11 + b * JDS_VLEN + i] * num_col + mc] +
          valp[pos12 + b * JDS_VLEN + i] *
          vvalp[idxp[pos12 + b * JDS_VLEN + i] * num_col + mc] +
          valp[pos13 + b * JDS_VLEN + i] *
          vvalp[idxp[pos13 + b * JDS_VLEN + i] * num_col + mc] +
          valp[pos14 + b * JDS_VLEN + i] *
          vvalp[idxp[pos14 + b * JDS_VLEN + i] * num_col + mc] +
          valp[pos15 + b * JDS_VLEN + i] *
          vvalp[idxp[pos15 + b * JDS_VLEN + i] * num_col + mc];
      }
    }
  }
  for(size_t mc = 0; mc < num_col; mc++) {
#pragma cdir on_adb(vvalp)
#pragma cdir on_adb(valp)
#pragma cdir on_adb(tmpvalp)
#pragma cdir on_adb(idxp)
#pragma cdir nodep
#pragma _NEC ivdep
    for(O i = JDS_VLEN * block_num; i < len; i++) {
      tmpvalp[(respos + i) * num_col + mc] +=
        valp[pos0 + i] * vvalp[idxp[pos0 + i] * num_col + mc] +
        valp[pos1 + i] * vvalp[idxp[pos1 + i] * num_col + mc] +
        valp[pos2 + i] * vvalp[idxp[pos2 + i] * num_col + mc] +
        valp[pos3 + i] * vvalp[idxp[pos3 + i] * num_col + mc] +
        valp[pos4 + i] * vvalp[idxp[pos4 + i] * num_col + mc] +
        valp[pos5 + i] * vvalp[idxp[pos5 + i] * num_col + mc] +
        valp[pos6 + i] * vvalp[idxp[pos6 + i] * num_col + mc] +
        valp[pos7 + i] * vvalp[idxp[pos7 + i] * num_col + mc] +
        valp[pos8 + i] * vvalp[idxp[pos8 + i] * num_col + mc] +
        valp[pos9 + i] * vvalp[idxp[pos9 + i] * num_col + mc] +
        valp[pos10 + i] * vvalp[idxp[pos10 + i] * num_col + mc] +
        valp[pos11 + i] * vvalp[idxp[pos11 + i] * num_col + mc] +
        valp[pos12 + i] * vvalp[idxp[pos12 + i] * num_col + mc] +
        valp[pos13 + i] * vvalp[idxp[pos13 + i] * num_col + mc] +
        valp[pos14 + i] * vvalp[idxp[pos14 + i] * num_col + mc] +
        valp[pos15 + i] * vvalp[idxp[pos15 + i] * num_col + mc];
    }
  }
  jds_spmm_helper8<T,I,O,P>(valp, idxp, offp, vvalp, tmpvalp, num_col, c,
                            col_offset + len);
  jds_spmm_helper4<T,I,O,P>(valp, idxp, offp, vvalp, tmpvalp, num_col, c+8,
                            col_offset + len);
  jds_spmm_helper2<T,I,O,P>(valp, idxp, offp, vvalp, tmpvalp, num_col, c+12,
                            col_offset + len);
  jds_spmm_helper1<T,I,O,P>(valp, idxp, offp, vvalp, tmpvalp, num_col, c+14,
                            col_offset + len);
}
*/

template <class T, class I, class O, class P>
rowmajor_matrix_local<T> operator*(const jds_matrix_local<T,I,O,P>& mat,
                                   const rowmajor_matrix_local<T>& v) {
  rowmajor_matrix_local<T> tmp(mat.local_num_row, v.local_num_col);
  rowmajor_matrix_local<T> ret(mat.local_num_row, v.local_num_col);
  T* tmpvalp = &tmp.val[0];
  const T* vvalp = &v.val[0];
  const T* valp = &mat.val[0];
  const I* idxp = &mat.idx[0];
  const O* offp = &mat.off[0];
  size_t physical_col_size = mat.off.size() - 1;
  size_t group_size = 8; // 8 is fastest
  size_t num_groups = physical_col_size / group_size;
  for(size_t g = 0; g < num_groups; g++) {
    jds_spmm_helper8<T,I,O,P>(valp, idxp, offp, vvalp, tmpvalp,
                              v.local_num_col, g * group_size, 0);
  }
  size_t c = num_groups * group_size;
/* // for unroll 16
  if(c + 7 < physical_col_size) {
    jds_spmm_helper8<T,I,O,P>(valp, idxp, offp, vvalp, tmpvalp,
                              v.local_num_col, c, 0);
    c += 8;
  }
*/
  if(c + 3 < physical_col_size) {
    jds_spmm_helper4<T,I,O,P>(valp, idxp, offp, vvalp, tmpvalp,
                              v.local_num_col, c, 0);
    c += 4;
  }
  if(c + 1 < physical_col_size) {
    jds_spmm_helper2<T,I,O,P>(valp, idxp, offp, vvalp, tmpvalp,
                              v.local_num_col, c, 0);
    c += 2;
  }
  if(c < physical_col_size) {
    jds_spmm_helper1<T,I,O,P>(valp, idxp, offp, vvalp, tmpvalp,
                              v.local_num_col, c, 0);
  }
  T* retvalp = &ret.val[0];
  const P* permp = &mat.perm[0];
  O block_num = mat.local_num_row / JDS_VLEN;
  for(O b = 0; b < block_num; b++) {
    for(size_t mc = 0; mc < v.local_num_col; mc++) {
#pragma cdir nodep
#pragma _NEC ivdep
#pragma cdir on_adb(tmpvalp)
#pragma cdir on_adb(retvalp)
#pragma cdir on_adb(permp)
      for(O i = 0; i < JDS_VLEN; i++) {
        retvalp[permp[b * JDS_VLEN + i] * v.local_num_col + mc] =
          tmpvalp[(b * JDS_VLEN + i) * v.local_num_col + mc];
      }
    }
  }
  for(size_t mc = 0; mc < v.local_num_col; mc++) {
#pragma cdir nodep
#pragma _NEC ivdep
#pragma cdir on_adb(tmpvalp)
#pragma cdir on_adb(retvalp)
#pragma cdir on_adb(permp)
    for(O i = block_num * JDS_VLEN; i < mat.local_num_row; i++) {
      retvalp[permp[i] * v.local_num_col + mc] =
        tmpvalp[i * v.local_num_col + mc];
    }
  }
  return ret;
}

/*
template <class T, class I, class O, class P>
std::vector<T> operator*(const jds_matrix_local<T,I,O,P>& mat, const std::vector<T>& v) {
  std::vector<T> tmp(mat.local_num_row);
  std::vector<T> ret(mat.local_num_row);
  T* tmpp = &tmp[0];
  const T* vp = &v[0];
  const T* valp = &mat.val[0];
  const I* idxp = &mat.idx[0];q
  const O* offp = &mat.off[0];
  for(size_t c = 0; c < mat.off.size() - 1; c++) {
    O r = 0;
#pragma cdir on_adb(tmpp)
#pragma cdir on_adb(vp)
    for(O pos = offp[c]; pos < offp[c+1]; pos++, r++) {
      tmpp[r] = tmpp[r] + valp[pos] * vp[idxp[pos]];
    }
  }
  T* retp = &ret[0];
  const P* permp = &mat.perm[0];
#pragma cdir nodep
#pragma _NEC ivdep
#pragma cdir on_adb(tmpp)
#pragma cdir on_adb(retp)
#pragma cdir on_adb(permp)
  for(size_t i = 0; i < mat.local_num_row; i++) {
    retp[permp[i]] = tmpp[i];
  }
  return ret;
}
*/

template <class T, class I, class O>
std::vector<T> call_jds_mv(const jds_matrix_local<T,I,O>& mat,
                           const std::vector<T>& v) {
  return mat * v;
}

template <class T, class I, class O>
dvector<T> operator*(jds_matrix<T,I,O>& mat, dvector<T>& dv) {
#ifdef SPARSE_MV_USE_ALLREDUCE
  auto dvsize = dv.size();
  node_local<std::vector<T>> bdv;
  // TODO: support larger size of allgather at dvector
  if(dvsize < INT_MAX) { 
    auto sizes = dv.sizes();
    size_t size = sizes.size();
    std::vector<int> count(size);
    auto sizesp = sizes.data();
    auto countp = count.data();
    for(size_t i = 0; i < size; i++) countp[i] = sizesp[i]; // cast to int
    bdv = dv.viewas_node_local().map(call_allgatherv<T>(count));
  } else {
    bdv = broadcast(dv.gather());
  }
#else
  auto bdv = broadcast(dv.gather());
#endif
  return mat.data.map(call_jds_mv<T,I,O>, bdv).template moveto_dvector<T>();
}
#if MPI_VERSION >= 3

template <class T, class I, class O>
void spmv(jds_matrix<T,I,O>& jds, shared_vector<T>& input, ptr_t<T>& output) {
  // omit zero clear because output is overwritten
  jds.data.mapv
    (+[](jds_matrix_local<T,I,O>& jds, shared_vector_local<T>& input,
         ptr_t_local<T>& ptr) {
      jds_matrix_spmv_impl(jds, ptr.data(), input.data());
    }, input.data, output.data);
}

#endif // MPI_VERSION >= 3
}

#endif
