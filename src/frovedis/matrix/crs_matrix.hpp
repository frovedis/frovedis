#ifndef CRS_MATRIX_HPP
#define CRS_MATRIX_HPP

#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <climits>

#include "../core/dvector.hpp"
#include "../core/dunordered_map.hpp"
#include "../core/mpihelper.hpp"
#include "../core/prefix_sum.hpp"

#include "rowmajor_matrix.hpp"

#if MPI_VERSION >= 3
#include "../core/shared_vector.hpp"
#endif

#define CRS_VLEN 256

#define CRS_SPMM_THR 32
#define CRS_SPMM_VLEN 256
#define SPARSE_VECTOR_VLEN 256
#define TO_SKIP_REBALANCE 256

namespace frovedis {

template <class T, class I = size_t>
struct sparse_vector {
  sparse_vector() : size(0) {}
  sparse_vector(const sparse_vector<T,I>& s) {
    val = s.val;
    idx = s.idx;
    size = s.size;
  }
  sparse_vector<T,I>& operator=(const sparse_vector<T,I>& s) {
    val = s.val;
    idx = s.idx;
    size = s.size;
    return *this;
  }
  sparse_vector(sparse_vector<T,I>&& s) {
    val.swap(s.val);
    idx.swap(s.idx);
    size = s.size;
  }
  sparse_vector<T,I>& operator=(sparse_vector<T,I>&& s) {
    val.swap(s.val);
    idx.swap(s.idx);
    size = s.size;
    return *this;
  }
  sparse_vector(I vsize){
    val.resize(vsize);
    idx.resize(vsize);
  }
  sparse_vector(I vsize, T initv){
    val.assign(vsize, initv);
    idx.assign(vsize, initv);
  }   
  void debug_print(size_t n = 0) const {
    std::cout << "val : "; debug_print_vector(val, n);
    std::cout << "idx : "; debug_print_vector(idx, n);
    std::cout << "size : " << size << std::endl;
  }
  std::vector<T> to_vector();
  
  std::vector<T> val;
  std::vector<I> idx;
  size_t size; // logical length; might not be the same as the last value of idx

  SERIALIZE(val, idx, size)
};

template <class T, class I>
void sparse_to_dense(sparse_vector<T,I>& sv, T* retp) {
  size_t valsize = sv.val.size();
  T* valp = sv.val.data();
  I* idxp = sv.idx.data();
#pragma cdir nodep
#pragma _NEC ivdep
#pragma _NEC vovertake
#pragma _NEC vob
  for(size_t i = 0; i < valsize; i++) {
    retp[idxp[i]] = valp[i];
  }
}

template <class T, class I>
std::vector<T> sparse_vector<T,I>::to_vector() {
  std::vector<T> ret(size);
  sparse_to_dense(*this, ret.data());
  return ret;
}

#if defined(_SX) || defined(__ve__)
// loop raking version
template <class T, class I = size_t>
sparse_vector<T,I> make_sparse_vector(const T* vp, size_t size) {
  if(size == 0) {
    return sparse_vector<T,I>();
  }
  std::vector<T> valtmp(size);
  std::vector<I> idxtmp(size);
  T* valtmpp = valtmp.data();
  I* idxtmpp = idxtmp.data();
  size_t each = size / SPARSE_VECTOR_VLEN; // maybe 0
  if(each % 2 == 0 && each > 1) each--;
  size_t rest = size - each * SPARSE_VECTOR_VLEN;
  size_t out_ridx[SPARSE_VECTOR_VLEN];
// never remove this vreg! this is needed folowing vovertake
// though this prevents ftrace...
// #pragma _NEC vreg(out_ridx)
  for(size_t i = 0; i < SPARSE_VECTOR_VLEN; i++) {
    out_ridx[i] = each * i;
  }
  if(each == 0) {
    size_t current = 0;
    for(size_t i = 0; i < size; i++) {
      if(vp[i] != 0) {
        valtmpp[current] = vp[i];
        idxtmpp[current] = i;
        current++;
      }
    }
    sparse_vector<T,I> ret;
    ret.size = size;
    ret.val.resize(current);
    ret.idx.resize(current);
    T* retvalp = ret.val.data();
    I* retidxp = ret.idx.data();
    for(size_t i = 0; i < current; i++) {
      retvalp[i] = valtmpp[i];
      retidxp[i] = idxtmpp[i];
    }
    return ret;
  } else {
//#pragma _NEC vob
    for(size_t j = 0; j < each; j++) {
#pragma cdir nodep
#pragma _NEC ivdep
//#pragma _NEC vovertake
      for(size_t i = 0; i < SPARSE_VECTOR_VLEN; i++) {
        auto loaded_v = vp[j + each * i];
        if(loaded_v != 0) {
          valtmpp[out_ridx[i]] = loaded_v;
          idxtmpp[out_ridx[i]] = j + each * i;
          out_ridx[i]++;
        }
      }
    }
    size_t rest_idx_start = each * SPARSE_VECTOR_VLEN;
    size_t rest_idx = rest_idx_start;
    if(rest != 0) {
      for(size_t j = 0; j < rest; j++) {
        auto loaded_v = vp[j + rest_idx_start]; 
        if(loaded_v != 0) {
          valtmpp[rest_idx] = loaded_v;
          idxtmpp[rest_idx] = j + rest_idx_start;
          rest_idx++;
        }
      }
    }
    sparse_vector<T,I> ret;
    ret.size = size;
    size_t sizes[SPARSE_VECTOR_VLEN];
    for(size_t i = 0; i < SPARSE_VECTOR_VLEN; i++) {
      sizes[i] = out_ridx[i] - each * i;
    }
    size_t total = 0;
    for(size_t i = 0; i < SPARSE_VECTOR_VLEN; i++) {
      total += sizes[i];
    }
    size_t rest_size = rest_idx - each * SPARSE_VECTOR_VLEN;
    total += rest_size;
    ret.val.resize(total);
    ret.idx.resize(total);
    T* retvalp = ret.val.data();
    I* retidxp = ret.idx.data();
    size_t current = 0;
    for(size_t i = 0; i < SPARSE_VECTOR_VLEN; i++) {
      for(size_t j = 0; j < sizes[i]; j++) {
        retvalp[current + j] = valtmpp[each * i + j];
        retidxp[current + j] = idxtmpp[each * i + j];
      }
      current += sizes[i];
    }
    for(size_t j = 0; j < rest_size; j++) {
      retvalp[current + j] = valtmpp[rest_idx_start + j];
      retidxp[current + j] = idxtmpp[rest_idx_start + j];
    }
    return ret;
  }
}
#else
// original version
template <class T, class I = size_t>
sparse_vector<T,I> make_sparse_vector(const T* vp, size_t size) {
  if(size == 0) {
    return sparse_vector<T,I>();
  }
  std::vector<T> valtmp(size);
  std::vector<I> idxtmp(size);
  T* valtmpp = valtmp.data();
  I* idxtmpp = idxtmp.data();
  size_t current = 0;
  for(size_t i = 0; i < size; i++) {
    if(vp[i] != 0) {
      valtmpp[current] = vp[i];
      idxtmpp[current] = i;
      current++;
    }
  }
  sparse_vector<T,I> ret;
  ret.size = size;
  ret.val.resize(current);
  ret.idx.resize(current);
  T* retvalp = ret.val.data();
  I* retidxp = ret.idx.data();
  for(size_t i = 0; i < current; i++) {
    retvalp[i] = valtmpp[i];
    retidxp[i] = idxtmpp[i];
  }
  return ret;
}
#endif
template <class T, class I = size_t>
sparse_vector<T,I> make_sparse_vector(const std::vector<T>& v) {
  return make_sparse_vector<T,I>(v.data(), v.size());
}

template <class T, class I = size_t, class O = size_t>
struct crs_matrix_local {
  crs_matrix_local() {off.push_back(0);}
  crs_matrix_local(size_t nrows, size_t ncols) :
    local_num_row(nrows), local_num_col(ncols) {off.push_back(0);}
  crs_matrix_local(const crs_matrix_local<T,I,O>& m) {
    val = m.val;
    idx = m.idx;
    off = m.off;
    local_num_row = m.local_num_row;
    local_num_col = m.local_num_col;
  }
  crs_matrix_local<T,I,O>& operator=(const crs_matrix_local<T,I,O>& m) {
    val = m.val;
    idx = m.idx;
    off = m.off;
    local_num_row = m.local_num_row;
    local_num_col = m.local_num_col;
    return *this;
  }
  crs_matrix_local(crs_matrix_local<T,I,O>&& m) {
    val.swap(m.val);
    idx.swap(m.idx);
    off.swap(m.off);
    local_num_row = m.local_num_row;
    local_num_col = m.local_num_col;
  }
  crs_matrix_local<T,I,O>& operator=(crs_matrix_local<T,I,O>&& m) {
    val.swap(m.val);
    idx.swap(m.idx);
    off.swap(m.off);
    local_num_row = m.local_num_row;
    local_num_col = m.local_num_col;
    return *this;
  }
  std::vector<T> val;
  std::vector<I> idx;
  std::vector<O> off; // size is local_num_row + 1 ("0" is added)
  size_t local_num_row;
  size_t local_num_col;
  void copy_from_jarray(int* off_, int* idx_, T* val_, size_t n) {
    val.resize(n); T* valp = &val[0];
    idx.resize(n); I* idxp = &idx[0];
    for(int i=0; i<n; ++i) {
      valp[i] = val_[i];  
      idxp[i] = idx_[i];
    }
    for(int i=0; i<local_num_row; ++i) off.push_back(off_[i+1]);
  } 
  crs_matrix_local<T,I,O> transpose();
  crs_matrix_local<T,I,O> pow_val(T exponent);
  void set_local_num(size_t c) {
    local_num_col = c;
    local_num_row = off.size() - 1;
  }
  rowmajor_matrix_local<T> to_rowmajor();
  void debug_pretty_print() const {
    for(size_t row = 0; row < local_num_row; row++) {
      std::vector<T> tmp(local_num_col);
      for(O pos = off[row]; pos < off[row+1]; pos++) {
        tmp[idx[pos]] = val[pos];
      }
      for(size_t col = 0; col < local_num_col; col++) {
        std::cout << tmp[col] << " ";
      }
      std::cout << std::endl;
    }
  }
  void debug_print(size_t n = 0) const {
    std::cout << "local_num_row = " << local_num_row
              << ", local_num_col = " << local_num_col
              << std::endl;
    std::cout << "val : "; debug_print_vector(val, n);
    std::cout << "idx : "; debug_print_vector(idx, n);
    std::cout << "off : "; debug_print_vector(off, n);
  }
  sparse_vector<T,I> get_row(size_t r) const;
  void save(const std::string& file);
  void savebinary(const std::string&);
  void clear() {
    std::vector<T> tmpval; tmpval.swap(val);
    std::vector<I> tmpidx; tmpidx.swap(idx);
    std::vector<O> tmpoff; tmpoff.swap(off); off.push_back(0);
    local_num_row = 0;
    local_num_col = 0;
  }

  SERIALIZE(val, idx, off, local_num_col, local_num_row)
};

template <class T, class I, class O>
sparse_vector<T,I> crs_matrix_local<T,I,O>::get_row(size_t k) const {
  sparse_vector<T,I> r;
  if(k > local_num_row) throw std::runtime_error("get_row: invalid position");
  size_t size = off[k+1] - off[k];
  r.val.resize(size);
  r.idx.resize(size);
  auto offp = off.data();
  auto valp = val.data();
  auto idxp = idx.data();
  auto rvalp = r.val.data();
  auto ridxp = r.idx.data();
  for(size_t i = 0; i < size; i++) rvalp[i] = valp[offp[k]+i];
  for(size_t i = 0; i < size; i++) ridxp[i] = idxp[offp[k]+i];
  r.size = local_num_col;
  return r;
}

/*
  see http://financelab.nctu.edu.tw/DataStructure/lec05.pdf
  shared with ccs_matrix_local, but row/col is for crs_matrix_local
  exchange local_num_row and local_num_col in the case of ccs_matrix_local
 */
template <class T, class I, class O>
void transpose_compressed_matrix(std::vector<T>& val,
                                 std::vector<I>& idx,
                                 std::vector<O>& off,
                                 std::vector<T>& ret_val,
                                 std::vector<I>& ret_idx,
                                 std::vector<O>& ret_off,
                                 size_t local_num_row,
                                 size_t local_num_col) {
  ret_val.resize(val.size());
  ret_idx.resize(idx.size());
  ret_off.resize(local_num_col + 1);
  std::vector<O> num_item(local_num_col);
  std::vector<O> current_item(local_num_col);
  O* num_itemp = &num_item[0];
  O* current_itemp = &current_item[0];
  T* ret_valp = &ret_val[0];
  I* ret_idxp = &ret_idx[0];
  O* ret_offp = &ret_off[0];

  T* valp = &val[0];
  I* idxp = &idx[0];
  O* offp = &off[0];

  for(size_t src_row = 0; src_row < local_num_row; src_row++) {
#pragma cdir nodep
#pragma _NEC ivdep
    for(O src_pos = offp[src_row]; src_pos < offp[src_row + 1];
        src_pos++) {
      num_itemp[idxp[src_pos]]++;
    }
  }
  prefix_sum(num_itemp, ret_offp+1, local_num_col);
  for(size_t src_row = 0; src_row < local_num_row; src_row++) {
#pragma cdir nodep
#pragma _NEC ivdep
    for(O src_pos = offp[src_row]; src_pos < offp[src_row + 1];
        src_pos++) {
      auto src_col = idxp[src_pos];
      T src_val = valp[src_pos];
      auto dst_pos = ret_offp[src_col] + current_itemp[src_col];
      ret_valp[dst_pos] = src_val;
      ret_idxp[dst_pos] = src_row;
      current_itemp[src_col]++;
    }
  }
}

template <class T, class I, class O>
crs_matrix_local<T,I,O> crs_matrix_local<T,I,O>::transpose() {
  crs_matrix_local<T,I,O> ret;
  transpose_compressed_matrix(val, idx, off, ret.val, ret.idx, ret.off,
                              local_num_row, local_num_col);
  ret.local_num_col = local_num_row;
  ret.local_num_row = local_num_col;
  return ret;
}

template <class T, class I, class O>
crs_matrix_local<T,I,O> crs_matrix_local<T,I,O>::pow_val(T exponent) {
  crs_matrix_local<T,I,O> ret(*this);
  auto* valp = ret.val.data();
  auto valsize = ret.val.size();
  for (size_t j = 0; j < valsize; j++) {
    valp[j] = std::pow(valp[j], exponent);
  }
  return ret;
}

template <class T, class I, class O>
rowmajor_matrix_local<T> crs_matrix_local<T,I,O>::to_rowmajor() {
  rowmajor_matrix_local<T> ret(local_num_row, local_num_col);
  T* retvalp = ret.val.data();
  T* valp = val.data();
  I* idxp = idx.data();
  O* offp = off.data();
  for(size_t row = 0; row < local_num_row; row++) {
    for(O pos = offp[row]; pos < offp[row+1]; pos++) {
      retvalp[local_num_col * row + idxp[pos]] = valp[pos];
    }
  }
  return ret;
}

template <class T, class I, class O>
std::ostream& operator<<(std::ostream& str, crs_matrix_local<T,I,O>& mat) {
  for(size_t row = 0; row < mat.local_num_row; row++) {
    for(O col = mat.off[row]; col < mat.off[row + 1]; col++) {
      str << mat.idx[col] << ":" << mat.val[col];
      if(col != mat.off[row + 1] - 1) str << " ";
    }
    str << "\n";
  }
  return str;
}

#ifdef __ve__
struct crs_is_zero {
  int operator()(size_t a) const {return a == 0;}
};

template <class T, class I, class O>
std::vector<char> crs_matrix_local_to_string(const crs_matrix_local<T,I,O>& m) {
  auto size = m.idx.size();
  if(size == 0) return std::vector<char>();
  auto ws = merge_words(number_to_words(m.idx), number_to_words(m.val));
  std::vector<size_t> new_starts(size * 2);
  std::vector<size_t> new_lens(size * 2);
  auto startsp = ws.starts.data();
  auto lensp = ws.lens.data();
  auto new_startsp = new_starts.data();
  auto new_lensp = new_lens.data();
  for(size_t i = 0; i < size; i++) {
    new_startsp[i * 2] = startsp[i];
    new_startsp[i * 2 + 1] = startsp[size + i];
    new_lensp[i * 2] = lensp[i];
    new_lensp[i * 2 + 1] = lensp[size + i];
  }
  std::vector<size_t> concat_starts;
  auto intvec = concat_words(ws.chars, new_starts, new_lens, " ",
                             concat_starts);
  auto intvecp = intvec.data();
  auto intvec_size = intvec.size();
  auto concat_startsp = concat_starts.data();
  auto concat_starts_size = concat_starts.size();
#pragma _NEC ivdep
  for(size_t i = 0; i < concat_starts_size/2; i++) {
    intvecp[concat_startsp[i*2+1]-1] = ':';
  }
  auto offp = m.off.data();
  auto off_size = m.off.size();
  std::vector<size_t> row_nnz(off_size-1);
  auto row_nnzp = row_nnz.data();
  for(size_t i = 0; i < off_size - 1; i++) {
    row_nnzp[i] = offp[i+1] - offp[i];
  }
  auto zeros = find_condition(row_nnz, crs_is_zero());
  auto zeros_size = zeros.size();
  if(zeros_size != 0) {
#pragma _NEC ivdep
    for(size_t i = 1; i < off_size-1; i++) {
      if(offp[i] > 0 && offp[i] < size) // guard first or last is null 
        intvecp[concat_startsp[offp[i]*2]-1] = '\n';
    }
    intvecp[intvec_size-1] = '\n';
    auto zerosp = zeros.data();
    std::vector<int> intvec2(intvec_size + zeros_size);
    auto intvec2p = intvec2.data();
    size_t start = 0;
    size_t end;
    for(size_t i = 0; i < zeros_size; i++) {
      auto pos = offp[zerosp[i]]*2;
      if(pos < concat_starts_size)  // guard last is null
        end = concat_startsp[pos];
      else
        end = intvec_size;
#pragma _NEC ivdep
      for(size_t j = start; j < end; j++) {
        intvec2p[j + i] = intvecp[j];
      }
      intvec2p[end + i] = '\n';
      start = end;
    }
    for(size_t j = start; j < intvec_size; j++) {
      intvec2p[j + zeros_size] = intvecp[j];
    }
    std::vector<char> ret(intvec_size + zeros_size);
    int_to_char(intvec2p, intvec_size + zeros_size, ret.data());
    return ret;
  } else {
#pragma _NEC ivdep
    for(size_t i = 1; i < off_size-1; i++) {
      intvecp[concat_startsp[offp[i]*2]-1] = '\n';
    }
    intvecp[intvec_size-1] = '\n';
    std::vector<char> ret(intvec_size);
    int_to_char(intvec.data(), intvec_size, ret.data());
    return ret;
  }
}

template <class T, class I, class O>
void crs_matrix_local<T,I,O>::save(const std::string& file) {
  auto vc = crs_matrix_local_to_string(*this);
  savebinary_local(vc.data(), vc.size(), file);  
}
#else
template <class T, class I, class O>
void crs_matrix_local<T,I,O>::save(const std::string& file) {
  std::ofstream str(file.c_str());
  str << *this;
}
#endif

inline std::string remove_schema(const std::string& path) {
  auto idx = path.find(':', 0);
  if(idx == std::string::npos) return path;
  else return path.substr(idx + 1);
}

/*
  The directory should contain following files:
  - val: big endian binary data file that contains values of the matrix 
         whose type is T in row major order
  - idx: big endian binary data whose type is I that contains column index of
         the value
  - off: big endian binary data whose type is O that contains offset of
         the row
  - nums: text file that contains num_row in the first line and
          num_col in the second line
 */
template <class T, class I = size_t, class O = size_t>
crs_matrix_local<T,I,O>
make_crs_matrix_local_loadbinary(const std::string& input) {
  std::string valfile = input + "/val";
  std::string idxfile = input + "/idx";
  std::string offfile = input + "/off";
#if defined(_SX) || defined(__ve__)
  std::string numsfile = remove_schema(input + "/nums");
  std::string typefile = remove_schema(input + "/type");
#else  
  std::string numsfile = input + "/nums";
  std::string typefile = input + "/type";
#endif
  std::ifstream numstr;
  numstr.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  numstr.open(numsfile.c_str());
  size_t num_row, num_col;
  numstr >> num_row >> num_col;
  struct stat sb;
  if(stat(typefile.c_str(), &sb) == 0) { // no file/directory
    std::ifstream typestr;
    typestr.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    typestr.open(typefile.c_str());
    std::string valtype, idxtype, offtype;
    typestr >> valtype >> idxtype >> offtype;
    confirm_given_type_against_expected<T>(valtype, __func__, "val");
    confirm_given_type_against_expected<I>(idxtype, __func__, "idx");
    confirm_given_type_against_expected<O>(offtype, __func__, "off");
  }
  else RLOG(INFO) << "no type file is present: skipping the typecheck for binary data!\n";
  crs_matrix_local<T,I,O> ret;
  auto loadval = make_dvector_loadbinary<T>(valfile).gather();
  ret.val.swap(loadval);
  auto loadidx = make_dvector_loadbinary<I>(idxfile).gather();
  ret.idx.swap(loadidx);
  auto loadoff = make_dvector_loadbinary<O>(offfile).gather();
  ret.off.swap(loadoff);
  ret.set_local_num(num_col);
  return ret;
}

template <class T, class I, class O>
void crs_matrix_local<T,I,O>::savebinary(const std::string& dir) {
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
  std::string numsfile = dir + "/nums";
  std::string typefile = dir + "/type";
  std::ofstream numstr;
  numstr.exceptions(std::ofstream::failbit | std::ofstream::badbit);
  numstr.open(numsfile.c_str());
  numstr << local_num_row << "\n" << local_num_col << std::endl;
  auto valtype = get_type_name<T>();
  auto idxtype = get_type_name<I>();
  auto offtype = get_type_name<O>();
  std::ofstream typestr;
  typestr.exceptions(std::ofstream::failbit | std::ofstream::badbit);
  typestr.open(typefile.c_str());
  typestr << valtype << "\n" << idxtype << "\n" << offtype << std::endl;
  make_dvector_scatter(val).savebinary(valfile);
  make_dvector_scatter(idx).savebinary(idxfile);
  make_dvector_scatter(off).savebinary(offfile);
}

template <class T>
T strtox(char* s, char** next);

template <>
float strtox<float>(char* s, char** next);

template <>
double strtox<double>(char* s, char** next);

template <>
int strtox<int>(char* s, char** next);

template <class T, class I, class O>
void make_crs_matrix_local_parseline(const std::string& line,
                                     crs_matrix_local<T,I,O>& ret) {
  char* s = const_cast<char*>(line.c_str());
  while(*s != '\0') {
    char* del;
    long long pos = strtoll(s, &del, 10);
    if(del == s) break;
    ret.idx.push_back(pos);
    s = del + 1;
    char* next;
    T val = strtox<T>(s, &next);
    ret.val.push_back(val);
    s = next;
  }
}

template <class T, class I, class O>
crs_matrix_local<T,I,O>
make_crs_matrix_local_readstream(std::istream& str) {
  crs_matrix_local<T,I,O> ret;
  std::string line;
  while(std::getline(str,line)) {
    make_crs_matrix_local_parseline<T,I,O>(line, ret);
    ret.off.push_back(ret.val.size());
  }
  return ret;
}

#ifdef __ve__
void crs_matrix_split_val_idx(const std::vector<int>& v,
                              std::vector<size_t>& starts, // -> idx
                              std::vector<size_t>& lens, // -> idx
                              std::vector<size_t>& val_starts,
                              std::vector<size_t>& val_lens,
                              std::vector<size_t>& line_starts); // -> off

template <class T, class I = size_t, class O = size_t>
crs_matrix_local<T,I,O>
make_crs_matrix_local_load_helper(words& ws,
                                  std::vector<size_t>& line_starts_byword,
                                  size_t num_col,
                                  bool calc_num_col) {
  std::vector<size_t> val_starts, val_lens;
  crs_matrix_split_val_idx(ws.chars, ws.starts, ws.lens,
                           val_starts, val_lens, line_starts_byword);
  crs_matrix_local<T,I,O> ret;
  ret.idx = parsenumber<I>(ws.chars, ws.starts, ws.lens);
  ret.val = parsenumber<T>(ws.chars, val_starts, val_lens);
  auto offsize = line_starts_byword.size() + 1;
  auto line_startsp = line_starts_byword.data();
  ret.off.resize(offsize);
  auto offp = ret.off.data();
  for(size_t i = 0; i < offsize-1; i++) offp[i] = line_startsp[i];
  offp[offsize-1] = ret.idx.size();
  ret.local_num_row = ret.off.size() - 1;
  if(calc_num_col) {
    if(ret.local_num_row == 0) ret.local_num_col = 0;
    else {
      auto idxp = ret.idx.data();
      auto idx_size = ret.idx.size();
      I max = 0;
      for(size_t i = 0; i < idx_size; i++) {
        if(idxp[i] > max) max = idxp[i];
      }
      ret.local_num_col = max + 1;
    }
  } else ret.local_num_col = num_col;
  return ret;
}

template <class T, class I = size_t, class O = size_t>
crs_matrix_local<T,I,O>
make_crs_matrix_local_load(const std::string& file, size_t num_col) {
  std::vector<size_t> line_starts_byword;
  auto ws = load_simple_csv_local(file, line_starts_byword, false, false, ' ');
  return make_crs_matrix_local_load_helper<T,I,O>(ws, line_starts_byword,
                                                  num_col, false);
}

template <class T, class I = size_t, class O = size_t>
crs_matrix_local<T,I,O>
make_crs_matrix_local_load(const std::string& file) {
  std::vector<size_t> line_starts_byword;
  auto ws = load_simple_csv_local(file, line_starts_byword, false, false, ' ');
  return make_crs_matrix_local_load_helper<T,I,O>(ws, line_starts_byword,
                                                  0, true);
}
#else
template <class T, class I = size_t, class O = size_t>
crs_matrix_local<T,I,O>
make_crs_matrix_local_load(const std::string& file, size_t num_col) {
  std::ifstream str(file.c_str());
  auto ret =  make_crs_matrix_local_readstream<T,I,O>(str);
  ret.local_num_row = ret.off.size() - 1;
  ret.local_num_col = num_col;
  return ret;
}

template <class T, class I = size_t, class O = size_t>
crs_matrix_local<T,I,O>
make_crs_matrix_local_load(const std::string& file) {
  std::ifstream str(file.c_str());
  auto ret =  make_crs_matrix_local_readstream<T,I,O>(str);
  ret.local_num_row = ret.off.size() - 1;
  auto it = std::max_element(ret.idx.begin(), ret.idx.end());
  if(it != ret.idx.end()) ret.local_num_col = *it + 1;
  else ret.local_num_col = 0;
  return ret;
}
#endif
// used for making (distributed) crs_matrix; size is set outside
template <class T, class I, class O>
crs_matrix_local<T,I,O>
make_crs_matrix_local_vectorstring(std::vector<std::string>& vs) {
  crs_matrix_local<T,I,O> ret;
  for(size_t i = 0; i < vs.size(); i++) {
    make_crs_matrix_local_parseline(vs[i], ret);
    ret.off.push_back(ret.val.size());
  }
  return ret;
}

template <class T, class I>
struct coo_triplet {
  I i;
  I j;
  T v;
  bool operator<(const coo_triplet<T,I>& r) const {
    if(i != r.i) return i < r.i;
    else if(j != r.j) return j < r.j;
    else return false;
  }
  SERIALIZE(i,j,v)
};

template <class T, class I>
std::vector<coo_triplet<T,I>> 
parse_coo_triplet(std::vector<std::string>& s, bool zero_origin) {
  std::vector<coo_triplet<T,I>> ret;
  for(size_t i = 0; i < s.size(); i++) {
    if(s[i][0] == '#' || s[i].size() == 0) continue;
    else {
      std::istringstream is(s[i]);
      coo_triplet<T,I> r;
      double tmp_i, tmp_j; // index might be expressed as floating point
      //is >> tmp_i >> tmp_j >> r.v;
      is >> tmp_i >> tmp_j;
      if (!is.eof()) is >> r.v;
      else r.v = 1.0;
      r.i = static_cast<I>(tmp_i);
      r.j = static_cast<I>(tmp_j);
      if(!zero_origin) {
        r.i -= 1;
        r.j -= 1;
      }
      ret.push_back(r);
    }
  }
  return ret;
}

template <class T, class I, class O>
crs_matrix_local<T,I,O>
coo_to_crs(const std::vector<coo_triplet<T,I>>& coo, I current_line) {
  crs_matrix_local<T,I,O> ret;
  for(size_t i = 0; i < coo.size(); i++) {
    if(current_line == coo[i].i) {
      ret.idx.push_back(coo[i].j);
      ret.val.push_back(coo[i].v);
    } else {
      for(; current_line < coo[i].i; current_line++)
        ret.off.push_back(ret.val.size());
      ret.idx.push_back(coo[i].j);
      ret.val.push_back(coo[i].v);
    }
  }
  if(coo.size() != 0) ret.off.push_back(ret.val.size());
  return ret;
}

template <class T, class I = size_t, class O = size_t>
crs_matrix_local<T,I,O>
make_crs_matrix_local_loadcoo(const std::string& file, 
                              bool zero_origin = false) {
  std::ifstream ifs(file.c_str());
  std::vector<std::string> vecstr;
  std::string line;
  while(std::getline(ifs,line)) {vecstr.push_back(line);}
  auto parsed = parse_coo_triplet<T,I>(vecstr, zero_origin);
  std::sort(parsed.begin(), parsed.end());
  I current_line = 0;
  crs_matrix_local<T,I,O> ret = coo_to_crs<T,I,O>(parsed, current_line);
  ret.local_num_row = ret.off.size() - 1;
  auto it = std::max_element(ret.idx.begin(), ret.idx.end());
  ret.local_num_col = *it + 1;
  return ret;
}

template <class T>
std::vector<T> loadlibsvm_extract_label(std::vector<std::string>& vecstr) {
  std::vector<T> ret;
  for(size_t i = 0; i < vecstr.size(); i++) {
    char* next;
    ret.push_back(strtox<T>(const_cast<char*>(vecstr[i].c_str()), &next));
    vecstr[i].erase(0, next - vecstr[i].c_str());
  }
  return ret;
}

template <class T, class I = size_t, class O = size_t>
crs_matrix_local<T,I,O>
make_crs_matrix_local_loadlibsvm(const std::string& file, 
                                 std::vector<T>& label) {
  std::ifstream ifs(file.c_str());
  std::vector<std::string> vecstr;
  std::string line;
  while(std::getline(ifs,line)) {vecstr.push_back(line);}
  label = loadlibsvm_extract_label<T>(vecstr);
  auto ret = make_crs_matrix_local_vectorstring<T,I,O>(vecstr);
  ret.local_num_row = ret.off.size() - 1;
  auto it = std::max_element(ret.idx.begin(), ret.idx.end());
  ret.local_num_col = *it + 1;
  for(size_t i = 0; i < ret.idx.size(); i++) {
    ret.idx[i]--; // one origin to zero origin
  }
  return ret;
}

template <class T, class I, class O>
struct set_local_num_crs_helper {
  set_local_num_crs_helper(){}
  set_local_num_crs_helper(size_t c) :
    local_num_col(c) {}
  void operator()(crs_matrix_local<T,I,O>& mat) {
    mat.set_local_num(local_num_col);
    mat.local_num_row = mat.off.size() - 1;
  }
  size_t local_num_col;
  
  SERIALIZE(local_num_col)
};

template <class T, class I, class O, class TT, class II, class OO>
crs_matrix_local<TT,II,OO>
change_datatype_helper(crs_matrix_local<T,I,O>& mat) {
  crs_matrix_local<TT,II,OO> ret;
  ret.val.resize(mat.val.size());
  ret.idx.resize(mat.idx.size());
  ret.off.resize(mat.off.size());
  auto matvalp = mat.val.data();
  auto matidxp = mat.idx.data();
  auto matoffp = mat.off.data();
  auto retvalp = ret.val.data();
  auto retidxp = ret.idx.data();
  auto retoffp = ret.off.data();
  auto matvalsize = mat.val.size();
  auto matidxsize = mat.idx.size();
  auto matoffsize = mat.off.size();
  for(size_t i = 0; i < matvalsize; i++) {
    retvalp[i] = static_cast<TT>(matvalp[i]);
  }
  for(size_t i = 0; i < matidxsize; i++) {
    retidxp[i] = static_cast<II>(matidxp[i]);
  }
  for(size_t i = 0; i < matoffsize; i++) {
    retoffp[i] = static_cast<OO>(matoffp[i]);
  }
  ret.local_num_row = mat.local_num_row;
  ret.local_num_col = mat.local_num_col;
  return ret;
}

template <class T, class I = size_t, class O = size_t>
struct crs_matrix {
  crs_matrix(){}
  crs_matrix(frovedis::node_local<crs_matrix_local<T,I,O>>&& d) : data(std::move(d)) {}
  void set_num(size_t r, size_t c) {
    num_row = r; num_col = c;
  }
  template <class TT, class II = size_t, class OO = size_t>
  crs_matrix<TT,II,OO> change_datatype() {
    auto r = data.map(change_datatype_helper<T,I,O,TT,II,OO>);
    crs_matrix<TT,II,OO> ret(std::move(r));
    ret.num_row = num_row;
    ret.num_col = num_col;
    return ret;
  }
  void debug_pretty_print()  {
    auto g = data.gather();
    for(size_t i = 0; i < g.size(); i++) {
      std::cout << "node " << i << std::endl;
      g[i].debug_pretty_print();
    }
  }
  void debug_print(size_t n = 0) {
    std::cout << "num_row = " << num_row
              << ", num_col = " << num_col << std::endl;
    auto g = data.gather();
    for(size_t i = 0; i < g.size(); i++) {
      std::cout << "node " << i << std::endl;
      g[i].debug_print(n);
    }
  }
  rowmajor_matrix<T> to_rowmajor();
  void save(const std::string& file);
  void savebinary(const std::string& file);
  crs_matrix<T,I,O> transpose();
  crs_matrix_local<T,I,O> gather();
  sparse_vector<T,I> get_row(size_t r);
  std::vector<size_t> get_local_num_rows();
  crs_matrix<T,I,O>& align_as(const std::vector<size_t>& size);
  void clear();
  frovedis::node_local<crs_matrix_local<T,I,O>> data;
  size_t num_row;
  size_t num_col;
};

template <class T, class I, class O>
rowmajor_matrix<T> crs_matrix<T,I,O>::to_rowmajor() {
  rowmajor_matrix<T> ret;
  ret.set_num(num_row, num_col);
  ret.data =
    data.map(+[](crs_matrix_local<T,I,O>& m){return m.to_rowmajor();});
  return ret;
}

template <class T, class I, class O>
void crs_clear_helper(crs_matrix_local<T,I,O>& mat) {mat.clear();}

template <class T, class I, class O>
void crs_matrix<T,I,O>::clear() {
  data.mapv(crs_clear_helper<T,I,O>);
  num_row = 0;
  num_col = 0;
}

template <class T, class I, class O>
size_t crs_get_local_num_row(crs_matrix_local<T,I,O>& mat) {
  return mat.local_num_row;
}

template <class T, class I, class O>
size_t crs_get_local_num_col(crs_matrix_local<T,I,O>& mat) {
  return mat.local_num_col;
}

template <class T, class I, class O>
std::vector<size_t> crs_matrix<T,I,O>::get_local_num_rows() {
  return data.map(crs_get_local_num_row<T,I,O>).gather();
}

template <class T, class I, class O>
void crs_get_row_helper(size_t& i, DVID<crs_matrix_local<T,I,O>>& dvid,
                        size_t& pos, intptr_t& retp) {
  if(i == get_selfid()) {
    auto v = dvid.get_selfdata()->get_row(pos);
    send_data_helper(0, v);
  } else if(get_selfid() == 0) {
    sparse_vector<T,I>* ret = reinterpret_cast<sparse_vector<T,I>*>(retp);
    receive_data_helper(i, *ret);
  }
}

template <class T, class I, class O>
sparse_vector<T,I> crs_matrix<T,I,O>::get_row(size_t pos) {
  auto sizes = data.map(crs_get_local_num_row<T,I,O>).gather();
  if(pos < sizes[0]) return data.get_dvid().get_selfdata()->get_row(pos);
  else {
    pos -= sizes[0];
    for(size_t i = 1; i < sizes.size(); i++) {
      if(pos < sizes[i]) {
        sparse_vector<T,I> ret;
        intptr_t retp = reinterpret_cast<intptr_t>(&ret);
        bcast_rpc_oneway(crs_get_row_helper<T,I,O>, i, data.get_dvid(), pos,
                         retp);
        return ret;
      } else pos -= sizes[i];
    }
  }
  throw std::runtime_error("get_row: invalid position");  
}

template <class T, class I, class O>
size_t crs_matrix_get_maxidx(crs_matrix_local<T,I,O>& m) {
  auto idxp = m.idx.data();
  auto idx_size = m.idx.size();
  I max = 0;
  for(size_t i = 0; i < idx_size; i++) {
    if(idxp[i] > max) max = idxp[i];
  }
  return max;
}

#ifdef __ve__
template <class T, class I, class O>
crs_matrix<T,I,O> make_crs_matrix_load_helper(const std::string& file,
                                              size_t num_col,
                                              bool calc_num_col) {
  // to destruct loaded text quickly, it is used as rvalue
  auto line_starts_byword = make_node_local_allocate<std::vector<size_t>>();
  crs_matrix<T,I,O> ret(
    load_simple_csv(file, line_starts_byword, false, false, ' ')
    .map(make_crs_matrix_local_load_helper<T,I,O>,
         line_starts_byword, 
         broadcast(num_col),
         broadcast(calc_num_col)));
  auto num_rows = ret.data.map(crs_get_local_num_row<T,I,O>).gather();
  auto num_rowsp = num_rows.data();
  auto num_rows_size = num_rows.size();
  auto total_num_row = 0;
  for(size_t i = 0; i < num_rows_size; i++) total_num_row += num_rowsp[i];
  ret.num_row = total_num_row;
  if(calc_num_col) {
    auto num_cols = ret.data.map(crs_get_local_num_col<T,I,O>).gather();
    auto num_colsp = num_cols.data();
    auto num_cols_size = num_cols.size();
    auto max_num_col = 0;
    for(size_t i = 0; i < num_cols_size; i++) {
      if(max_num_col < num_colsp[i]) max_num_col = num_colsp[i];
    }
    ret.num_col = max_num_col;
  } else {
    ret.num_col = num_col;
  }
  // if number of row is zero, num_col is not set properly
  ret.data.mapv(set_local_num_crs_helper<T,I,O>(ret.num_col));
  return ret;
}
template <class T, class I = size_t, class O = size_t>
crs_matrix<T,I,O> make_crs_matrix_load(const std::string& file,
                                       size_t num_col) {
  return make_crs_matrix_load_helper<T,I,O>(file, num_col, false);
}
template <class T, class I = size_t, class O = size_t>
crs_matrix<T,I,O> make_crs_matrix_load(const std::string& file) {
  return make_crs_matrix_load_helper<T,I,O>(file, 0, true);
}
#else
template <class T, class I = size_t, class O = size_t>
crs_matrix<T,I,O> make_crs_matrix_load(const std::string& file,
                                       size_t num_col) {
  auto rawdata = frovedis::make_dvector_loadline(file);
  size_t num_row = rawdata.size();
  crs_matrix<T,I,O> ret(rawdata.moveto_node_local().
                        map(make_crs_matrix_local_vectorstring<T,I,O>));
  ret.data.mapv(set_local_num_crs_helper<T,I,O>(num_col));
  ret.num_col = num_col;
  ret.num_row = num_row;
  return ret;
}

template <class T, class I = size_t, class O = size_t>
crs_matrix<T,I,O> make_crs_matrix_load(const std::string& file) {
  auto rawdata = frovedis::make_dvector_loadline(file);
  size_t num_row = rawdata.size();
  crs_matrix<T,I,O> ret(rawdata.moveto_node_local().
                        map(make_crs_matrix_local_vectorstring<T,I,O>));
  auto maxs = ret.data.map(crs_matrix_get_maxidx<T,I,O>).gather();
  size_t num_col = *std::max_element(maxs.begin(), maxs.end()) + 1;
  ret.data.mapv(set_local_num_crs_helper<T,I,O>(num_col));
  ret.num_col = num_col;
  ret.num_row = num_row;
  return ret;
}
#endif

template <class T, class I>
std::vector<std::pair<I, coo_triplet<T,I>>>
  make_coo_pairs(std::vector<coo_triplet<T,I>>& coo) {
  std::vector<std::pair<I, coo_triplet<T,I>>> ret(coo.size());
  for(size_t i = 0; i < coo.size(); i++) {
    ret[i] = std::make_pair(coo[i].i, coo[i]);
  }
  return ret;
}

template <class T, class I>
void myprint(std::vector<std::pair<I, std::vector<coo_triplet<T,I>>>>& data) {
  for(size_t i = 0; i < data.size(); i++) {
    RLOG(FATAL) << "key = " << data[i].first << ", data = ";
    for(size_t j = 0; j < data[i].second.size(); j++) {
      LOG(FATAL) << data[i].second[j].i << " "
                   << data[i].second[j].j << " "
                   << data[i].second[j].v << ", ";
    }
    LOG(FATAL) << std::endl;
  }
}

template <class K, class V>
void sort_values(K key, std::vector<V>& value) {
  std::sort(value.begin(), value.end());
}

template <class T, class I>
I get_coo_sizes(std::pair<I, std::vector<coo_triplet<T,I>>>& data) {
  return data.second.size();
}

template <class T, class I>
I get_first_idx(std::vector<std::pair<I,
                std::vector<coo_triplet<T,I>>>>& data) {
  return data.size() == 0 ? 0 : data[0].first;
}

template <class T, class I, class O>
crs_matrix_local<T,I,O>
coo_to_crs2(std::vector<std::pair<I,std::vector<coo_triplet<T,I>>>>& coo,
            I next_idx) {
  crs_matrix_local<T,I,O> ret;
  if(coo.size() != 0) {
    I current_line;
    if(get_selfid() == 0) current_line = 0;
    else current_line = coo[0].first;
    for(size_t i = 0; i < coo.size(); i++) {
      if(current_line != coo[i].first) {
        for(; current_line < coo[i].first; current_line++)
          ret.off.push_back(ret.val.size());
      }
      for(size_t j = 0; j < coo[i].second.size(); j++) {
        ret.idx.push_back(coo[i].second[j].j);
        ret.val.push_back(coo[i].second[j].v);
      }
    }
    ret.off.push_back(ret.val.size());
    current_line++;
    for(; current_line < next_idx; current_line++)
      ret.off.push_back(ret.val.size());
  }    
  return ret;
}

template <class T, class I = size_t, class O = size_t>
crs_matrix<T,I,O>
make_crs_matrix_loadcoo(frovedis::node_local<std::vector<std::string>>& rawdata, 
                        bool zero_origin = false) {
  auto bzero_origin = make_node_local_broadcast(zero_origin);
  auto dv_parsed = rawdata.map(parse_coo_triplet<T,I>, bzero_origin).
    map(make_coo_pairs<T,I>).
    template moveto_dvector<std::pair<I,coo_triplet<T,I>>>().
    template group_by_key<I, coo_triplet<T,I>>().
    mapv(sort_values<I,coo_triplet<T,I>>).
    moveto_node_local().
    template map<std::vector<std::pair<I,std::vector<coo_triplet<T,I>>>>>
    // maptovector is in dunordered_map.hpp
    (maptovector<I,std::vector<coo_triplet<T,I>>>()). 
    template moveto_dvector<std::pair<I,std::vector<coo_triplet<T,I>>>>().
    sort();
  auto coo_sizes = dv_parsed.map(get_coo_sizes<T,I>).gather();
  if(coo_sizes.size() == 0) {
    crs_matrix<T,I,O> ret;
    ret.data = make_node_local_allocate<crs_matrix_local<T,I,O>>();
    return ret;
  }
  std::vector<size_t> sum_sizes(coo_sizes.size());
  sum_sizes[0] = coo_sizes[0];
  for(size_t i = 1; i < sum_sizes.size(); i++)
    sum_sizes[i] = sum_sizes[i-1] + coo_sizes[i];
  size_t node_size = get_nodesize();
  size_t each = ceil_div(sum_sizes[sum_sizes.size()-1], node_size);
  std::vector<size_t> dvec_end_row(node_size);
  for(size_t i = 0; i < node_size; i++) {
    auto it = std::lower_bound(sum_sizes.begin(), sum_sizes.end(),
                               each * (i + 1));
    if(it != sum_sizes.end()) {
      dvec_end_row[i] = it - sum_sizes.begin();
    } else {
      dvec_end_row[i] = coo_sizes.size() - 1;
    }
  }
  std::vector<size_t> dvec_row_size(node_size);
  dvec_row_size[0] = dvec_end_row[0] + 1;
  for(size_t i = 1; i < node_size; i++)
    dvec_row_size[i] = dvec_end_row[i] - dvec_end_row[i-1];
  dv_parsed.align_as(dvec_row_size);
  auto nl_parsed = dv_parsed.viewas_node_local();
  auto first_idx = nl_parsed.map(get_first_idx<T,I>).gather();
  I current = first_idx[node_size - 1];
  // take care for zero sized data node, which returns 0
  // in this case, next index is on a later node
  for(size_t i = 0; i < first_idx.size(); i++) {
    if(first_idx[node_size - 1 - i] == 0)
      first_idx[node_size - 1 - i] = current;
    else 
      current = first_idx[node_size - 1 - i];
  }
  first_idx.erase(first_idx.begin());
  first_idx.push_back(0);
  auto next_idx = make_node_local_scatter(first_idx);
  crs_matrix<T,I,O> ret(nl_parsed.map(coo_to_crs2<T,I,O>, next_idx));
  auto maxs = ret.data.map(crs_matrix_get_maxidx<T,I,O>).gather();
  auto num_col = *std::max_element(maxs.begin(), maxs.end()) + 1;
  ret.data.mapv(set_local_num_crs_helper<T,I,O>(num_col));
  ret.num_col = num_col;
  auto num_rows = ret.data.map(crs_get_local_num_row<T,I,O>).gather();
  size_t num_row = 0;
  for(size_t i = 0; i < num_rows.size(); i++) num_row += num_rows[i];
  ret.num_row = num_row;
  return ret;
}

template <class T, class I = size_t, class O = size_t>
crs_matrix<T,I,O>
make_crs_matrix_loadcoo(const std::string& file, 
                        bool zero_origin = false) {
  auto rawdata = frovedis::make_dvector_loadline(file).moveto_node_local();
  return make_crs_matrix_loadcoo<T,I,O>(rawdata,zero_origin);
}

template <class T, class I, class O>
void to_zero_origin(crs_matrix_local<T,I,O>& mat) {
  auto matidxp = mat.idx.data();
  auto matidxsize = mat.idx.size();
  for(size_t i = 0; i < matidxsize; i++) {
    matidxp[i]--;
  }
}

template <class T, class I = size_t, class O = size_t>
crs_matrix<T,I,O>
make_crs_matrix_loadlibsvm(const std::string& file, 
                           dvector<T>& label) {
  auto rawdata = frovedis::make_dvector_loadline(file);
  size_t num_row = rawdata.size();
  label = rawdata.viewas_node_local().map(loadlibsvm_extract_label<T>).
    template moveto_dvector<T>();
  crs_matrix<T,I,O> ret(rawdata.moveto_node_local().
                        map(make_crs_matrix_local_vectorstring<T,I,O>));
  ret.data.mapv(to_zero_origin<T,I,O>);
  auto maxs = ret.data.map(crs_matrix_get_maxidx<T,I,O>).gather();
  size_t num_col = *std::max_element(maxs.begin(), maxs.end()) + 1;
  ret.data.mapv(set_local_num_crs_helper<T,I,O>(num_col));
  ret.num_col = num_col;
  ret.num_row = num_row;
  return ret;
}

template <class T, class I, class O>
crs_matrix_local<T,I,O> call_transpose(crs_matrix_local<T,I,O>& mat) {
  return mat.transpose();
}

template <class T, class I, class O>
void get_off(crs_matrix_local<T,I,O>& mat, std::vector<O>& r) {
  r.resize(mat.off.size());
  r = mat.off;
}

template <class T, class I, class O>
crs_matrix_local<T,I,O>
divide_and_exchange(const crs_matrix_local<T,I,O>& mat,
                    const std::vector<size_t>& divide_row,
                    const std::vector<size_t>& col_shift) {
  size_t node_size = frovedis::get_nodesize();
  std::vector<size_t> send_size(node_size);
  auto send_sizep = send_size.data();
  auto matoffp = mat.off.data();
  auto divide_rowp = divide_row.data();
  for(size_t i = 0; i < node_size; i++) {
    if(divide_rowp[i] != mat.local_num_row) {
      send_sizep[i] =
        matoffp[divide_rowp[i+1]] - matoffp[divide_rowp[i]];
    } else {
      send_sizep[i] = 0;
    }
  }
  std::vector<size_t> recv_size(node_size);
  auto recv_sizep = recv_size.data();
  MPI_Alltoall(&send_size[0], sizeof(size_t), MPI_CHAR,
               &recv_size[0], sizeof(size_t), MPI_CHAR, frovedis_comm_rpc);
  size_t total_size = 0;
  for(size_t i = 0; i < node_size; i++) {
    total_size += recv_sizep[i];
  }
  std::vector<size_t> send_displ(node_size);
  std::vector<size_t> recv_displ(node_size);
  auto send_displp = send_displ.data();
  auto recv_displp = recv_displ.data();
  send_displp[0] = 0; recv_displp[0] = 0;
  for(size_t i = 1; i < node_size; i++) {
    send_displp[i] = send_displp[i-1] + send_sizep[i-1];
    recv_displp[i] = recv_displp[i-1] + recv_sizep[i-1];
  }
  std::vector<T> val_tmp(total_size);
  std::vector<I> idx_tmp(total_size);
  large_alltoallv(sizeof(T),
                  reinterpret_cast<char*>(const_cast<T*>(&mat.val[0])),
                  send_size, send_displ, 
                  reinterpret_cast<char*>(const_cast<T*>(&val_tmp[0])),
                  recv_size, recv_displ, 
                  frovedis_comm_rpc);
  large_alltoallv(sizeof(I),
                  reinterpret_cast<char*>(const_cast<I*>(&mat.idx[0])),
                  send_size, send_displ,
                  reinterpret_cast<char*>(const_cast<I*>(&idx_tmp[0])),
                  recv_size, recv_displ,
                  frovedis_comm_rpc);
  size_t my_num_row =
    divide_row[frovedis::get_selfid() + 1] - divide_row[frovedis::get_selfid()];
  std::vector<O> off_tmp((my_num_row + 1) * node_size);
  std::vector<size_t> off_send_size(node_size);
  std::vector<size_t> off_recv_size(node_size);
  std::vector<size_t> off_send_displ(node_size);
  std::vector<size_t> off_recv_displ(node_size);
  auto off_send_sizep = off_send_size.data();
  auto off_recv_sizep = off_recv_size.data();
  auto off_send_displp = off_send_displ.data();
  auto off_recv_displp = off_recv_displ.data();
  for(size_t i = 0; i < node_size; i++) {
    off_send_sizep[i] = divide_rowp[i+1] - divide_rowp[i] + 1;
  }
  for(size_t i = 0; i < node_size; i++) {
    off_recv_sizep[i] = my_num_row + 1;
  }
  off_send_displp[0] = 0; off_recv_displp[0] = 0; 
  for(size_t i = 1; i < node_size; i++) {
    off_send_displp[i] =
      off_send_displp[i-1] + off_send_sizep[i-1] - 1;
    off_recv_displp[i] = off_recv_displp[i-1] + off_recv_sizep[i-1];
  }
  large_alltoallv(sizeof(O),
                  reinterpret_cast<char*>(const_cast<O*>(&mat.off[0])),
                  off_send_size, off_send_displ,
                  reinterpret_cast<char*>(const_cast<O*>(&off_tmp[0])),
                  off_recv_size, off_recv_displ,
                  frovedis_comm_rpc);
  std::vector<O> off_each((my_num_row + 1) * node_size);
  auto off_eachp = off_each.data();
  auto off_tmpp = off_tmp.data();
  for(size_t n = 0; n < node_size; n++) {
    for(size_t r = 0; r < my_num_row + 1; r++) {
      off_eachp[(my_num_row + 1) * n + r] =
        off_tmpp[(my_num_row + 1) * n + r] -
        off_tmpp[(my_num_row + 1) * n]; 
    }
  }
  crs_matrix_local<T,I,O> ret;
  ret.val.resize(total_size);
  ret.idx.resize(total_size);
  ret.off.resize(my_num_row+1);
  T* ret_valp = &ret.val[0];
  I* ret_idxp = &ret.idx[0];
  O* ret_offp = &ret.off[0];
  T* val_tmpp = &val_tmp[0];
  I* idx_tmpp = &idx_tmp[0];
  const size_t* col_shiftp = &col_shift[0];
  size_t off = 0;
  for(size_t r = 0; r < my_num_row; r++) {
    ret_offp[r] = off;
    for(size_t n = 0; n < node_size; n++) {
      for(O crnt = off_eachp[(my_num_row + 1) * n + r];
          crnt < off_eachp[(my_num_row + 1) * n + r + 1]; crnt++) {
        ret_valp[off] = val_tmpp[recv_displp[n] + crnt];
        ret_idxp[off] = idx_tmpp[recv_displp[n] + crnt] + col_shiftp[n];
        off++;
      }
    }
  }
  ret_offp[my_num_row] = off;
  return ret;
}

template <class T, class I, class O>
crs_matrix<T,I,O> crs_matrix<T,I,O>::transpose() {
  auto tmp = data.map(call_transpose<T,I,O>);
  auto dist_offs = make_node_local_allocate<std::vector<O>>();
  tmp.mapv(get_off<T,I,O>, dist_offs);
  std::vector<O> tmp_offs = dist_offs.vector_sum();
  size_t total_dst_row = tmp_offs[tmp_offs.size() - 1];
  size_t node_size = frovedis::get_nodesize();
  size_t each_size = frovedis::ceil_div(total_dst_row, node_size);
  std::vector<size_t> divide_row(node_size+1);
  for(size_t i = 0; i < node_size; i++) {
    auto it = std::lower_bound(tmp_offs.begin(), tmp_offs.end(),
                               each_size * i);
    if(it != tmp_offs.end()) {
      divide_row[i] = it - tmp_offs.begin();
    } else {
      divide_row[i] = num_col;
    }
  }
  divide_row[node_size] = num_col;
  auto bcast_divide_row =
    frovedis::make_node_local_broadcast<std::vector<size_t>>(divide_row);
  auto local_rows = data.map(crs_get_local_num_row<T,I,O>).gather();
  std::vector<size_t> col_shift(node_size + 1);
  auto col_shiftp = col_shift.data();
  auto local_rowsp = local_rows.data();
  col_shiftp[0] = 0;
  for(size_t i = 1; i < node_size + 1; i++) {
    col_shiftp[i] = col_shiftp[i-1] + local_rowsp[i-1];
  }
  auto bcast_col_shift =
    frovedis::make_node_local_broadcast<std::vector<size_t>>(col_shift);
  crs_matrix<T,I,O> ret(tmp.map(divide_and_exchange<T,I,O>,
                                bcast_divide_row, bcast_col_shift));
  ret.set_num(num_col, num_row);
  ret.data.mapv(set_local_num_crs_helper<T,I,O>(num_row));
  return ret;
}

template <class T, class I, class O>
crs_matrix_local<T,I,O> crs_matrix<T,I,O>::gather() {
  return merge_scattered_crs_matrices(data.gather());
}

template <class T, class I, class O>
void crs_matrix_spmv_impl(const crs_matrix_local<T,I,O>& mat, T* retp, const T* vp) {
  const T* valp = &mat.val[0];
  const I* idxp = &mat.idx[0];
  const O* offp = &mat.off[0];
  for(size_t r = 0; r < mat.local_num_row; r++) {
#pragma cdir on_adb(vp)
    for(O c = offp[r]; c < offp[r+1]; c++) {
      retp[r] = retp[r] + valp[c] * vp[idxp[c]];
    }
  }
}

template <class T, class I, class O>
std::vector<T> operator*(const crs_matrix_local<T,I,O>& mat, const std::vector<T>& v) {
  std::vector<T> ret(mat.local_num_row);
  if(mat.local_num_col != v.size())
    throw std::runtime_error("operator*: size of vector does not match");
  crs_matrix_spmv_impl(mat, ret.data(), v.data());
  return ret;
}

template <class T, class I, class O>
std::ostream& operator<<(std::ostream& str, crs_matrix<T,I,O>& mat) {
  auto gmat = mat.data.gather();
  for(auto& l: gmat) str << l;
  return str;
}

template <class T, class I, class O>
void crs_matrix_set_val(crs_matrix_local<T,I,O>& m, std::vector<T>& val) {
  m.val.swap(val);
}

template <class T, class I, class O>
void crs_matrix_set_idx(crs_matrix_local<T,I,O>& m, std::vector<I>& idx) {
  m.idx.swap(idx);
}

template <class T, class I, class O>
void crs_matrix_set_off(crs_matrix_local<T,I,O>& m, std::vector<O>& off) {
  m.off.swap(off);
}

template <class T, class I = size_t, class O = size_t>
crs_matrix<T,I,O>
make_crs_matrix_loadbinary(const std::string& input) {
  std::string valfile = input + "/val";
  std::string idxfile = input + "/idx";
  std::string offfile = input + "/off";
  std::string numsfile = input + "/nums";
  std::string typefile = input + "/type";
  std::ifstream numstr;
  numstr.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  numstr.open(numsfile.c_str());
  size_t num_row, num_col;
  numstr >> num_row >> num_col;
  struct stat sb;
  if(stat(typefile.c_str(), &sb) == 0) { // no file/directory
    std::ifstream typestr;
    typestr.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    typestr.open(typefile.c_str());
    std::string valtype, idxtype, offtype;
    typestr >> valtype >> idxtype >> offtype;
    confirm_given_type_against_expected<T>(valtype, __func__, "val");
    confirm_given_type_against_expected<I>(idxtype, __func__, "idx");
    confirm_given_type_against_expected<O>(offtype, __func__, "off");
  }
  else RLOG(INFO) << "no type file is present: skipping the typecheck for binary data!\n";
  size_t node_size = get_nodesize();
  auto loadoff = make_dvector_loadbinary<O>(offfile).gather();
  size_t total_elm = loadoff[loadoff.size() - 1];
  size_t each_size = frovedis::ceil_div(total_elm, node_size);
  std::vector<size_t> divide_elm(node_size + 1);
  std::vector<size_t> divide_row(node_size + 1);
  for(size_t i = 0; i < node_size; i++) {
    auto it = std::lower_bound(loadoff.begin(), loadoff.end(), each_size * i);
    if(it != loadoff.end()) {
      divide_elm[i] = *it;
      divide_row[i] = it - loadoff.begin();
    } else {
      divide_elm[i] = total_elm;
      divide_row[i] = num_row;
    }
  }
  divide_elm[node_size] = total_elm;
  divide_row[node_size] = num_row;
  std::vector<size_t> sizes(node_size);
  auto sizesp = sizes.data();
  auto divide_elmp = divide_elm.data();
  for(size_t i = 0; i < node_size; i++) {
    sizesp[i] = divide_elmp[i + 1] - divide_elmp[i];
  }
  crs_matrix<T,I,O> ret;
  ret.data = make_node_local_allocate<crs_matrix_local<T,I,O>>();
  auto loadval = make_dvector_loadbinary<T>(valfile, sizes).moveto_node_local();
  ret.data.mapv(crs_matrix_set_val<T,I,O>, loadval);
  auto loadidx = make_dvector_loadbinary<I>(idxfile, sizes).moveto_node_local();
  ret.data.mapv(crs_matrix_set_idx<T,I,O>, loadidx);

  std::vector<size_t> sizes_off(node_size);
  auto sizes_offp = sizes_off.data();
  auto divide_rowp = divide_row.data();
  for(size_t i = 0; i < node_size; i++) {
    sizes_offp[i] = divide_rowp[i+1] - divide_rowp[i] + 1;
  }
  size_t adjust = 0;
  size_t current_node = 1;
  std::vector<O> newloadoff(loadoff.size() + node_size - 1);
  size_t j = 0;
  auto newloadoffp = newloadoff.data();
  auto loadoffp = loadoff.data();
  for(size_t i = 0; i < loadoff.size(); i++, j++) {
    newloadoffp[j] = loadoffp[i] - adjust;
    if(current_node < node_size && loadoffp[i] == divide_elmp[current_node]) {
      j++;
      newloadoffp[j] = 0;
      adjust = loadoffp[i];
      current_node++;
    }
  }
  auto nlocoff = make_dvector_scatter(newloadoff, sizes_off).moveto_node_local();
  ret.data.mapv(crs_matrix_set_off<T,I,O>, nlocoff);
  ret.data.mapv(set_local_num_crs_helper<T,I,O>(num_col));
  ret.set_num(num_row, num_col);
  return ret;
}

template <class T, class I, class O>
std::vector<T> crs_matrix_get_val(crs_matrix_local<T,I,O>& m) {
  return m.val;
}

template <class T, class I, class O>
std::vector<I> crs_matrix_get_idx(crs_matrix_local<T,I,O>& m) {
  return m.idx;
}

template <class T, class I, class O>
std::vector<O> crs_matrix_get_off(crs_matrix_local<T,I,O>& m) {
  return m.off;
}

#ifdef __ve__
template <class T, class I, class O>
void crs_matrix<T,I,O>::save(const std::string& file) {
  auto vc = data.map(crs_matrix_local_to_string<T,I,O>);
  vc.template moveto_dvector<char>().savebinary(file);
}
#else
template <class T, class I, class O>
void crs_matrix<T,I,O>::save(const std::string& file) {
  std::ofstream str(file.c_str());
  str << *this;
}
#endif

template <class T, class I, class O>
void crs_matrix<T,I,O>::savebinary(const std::string& dir) {
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
  std::string numsfile = dir + "/nums";
  std::string typefile = dir + "/type";
  std::ofstream numstr;
  numstr.exceptions(std::ofstream::failbit | std::ofstream::badbit);
  numstr.open(numsfile.c_str());
  numstr << num_row << "\n" << num_col << std::endl;
  auto valtype = get_type_name<T>();
  auto idxtype = get_type_name<I>();
  auto offtype = get_type_name<O>();
  std::ofstream typestr;
  typestr.exceptions(std::ofstream::failbit | std::ofstream::badbit);
  typestr.open(typefile.c_str());
  typestr << valtype << "\n" << idxtype << "\n" << offtype << std::endl;
  data.map(crs_matrix_get_val<T,I,O>).
    template moveto_dvector<T>().savebinary(valfile);
  data.map(crs_matrix_get_idx<T,I,O>).
    template moveto_dvector<I>().savebinary(idxfile);
  auto offs = data.map(crs_matrix_get_off<T,I,O>).gather();
  std::vector<O> off(num_row + 1);
  size_t offidx = 0;
  size_t adjust = 0;
  for(size_t n = 0; n < offs.size(); n++) {
    for(size_t loc = 0; loc < offs[n].size() - 1; loc++) {
      off[offidx++] = offs[n][loc] + adjust;
    }
    if(n != offs.size() - 1) {
      adjust += offs[n][offs[n].size() - 1];
    } else { // last node
      off[offidx] = adjust + offs[n][offs[n].size() - 1];
    }
  }
  make_dvector_scatter(off).savebinary(offfile);
}

template <class T, class I, class O>
std::vector<crs_matrix_local<T,I,O>> 
get_scattered_crs_matrices(crs_matrix_local<T,I,O>& data,
                           size_t node_size) {
  size_t total = data.off[data.off.size() - 1];
  size_t each_size = frovedis::ceil_div(total, node_size);
  std::vector<size_t> divide_row(node_size+1);
  for(size_t i = 0; i < node_size; i++) {
    auto it = std::lower_bound(data.off.begin(), data.off.end(),
                               each_size * i);
    if(it != data.off.end()) {
      divide_row[i] = it - data.off.begin();
    } else {
      divide_row[i] = data.local_num_row;
    }
  }
  divide_row[node_size] = data.local_num_row;
  std::vector<crs_matrix_local<T,I,O>> vret(node_size);
  T* datavalp = &data.val[0];
  I* dataidxp = &data.idx[0];
  O* dataoffp = &data.off[0];
  for(size_t i = 0; i < node_size; i++) {
    vret[i].local_num_col = data.local_num_col;
    size_t start_row = divide_row[i];
    size_t end_row = divide_row[i+1];
    vret[i].local_num_row = end_row - start_row;
    size_t start_off = dataoffp[start_row];
    size_t end_off = dataoffp[end_row];
    size_t off_size = end_off - start_off;
    vret[i].val.resize(off_size);
    vret[i].idx.resize(off_size);
    vret[i].off.resize(end_row - start_row + 1); // off[0] == 0 by ctor
    T* valp = &vret[i].val[0];
    I* idxp = &vret[i].idx[0];
    O* offp = &vret[i].off[0];
    for(size_t j = 0; j < off_size; j++) {
      valp[j] = datavalp[j + start_off];
      idxp[j] = dataidxp[j + start_off];
    }
    for(size_t j = 0; j < end_row - start_row; j++) {
      offp[j+1] = offp[j] + (dataoffp[start_row + j + 1] -
                             dataoffp[start_row + j]);
    }
  }
  return vret;
}

template <class T, class I, class O>
std::vector<crs_matrix_local<T,I,O>> 
get_scattered_crs_matrices(crs_matrix_local<T,I,O>& data) {
  return get_scattered_crs_matrices(data, get_nodesize());
}

template <class T, class I, class O>
crs_matrix<T,I,O> make_crs_matrix_scatter(crs_matrix_local<T,I,O>& data) {
  auto vret = get_scattered_crs_matrices(data);
  crs_matrix<T,I,O> ret(make_node_local_scatter(vret));
  ret.set_num(data.local_num_row, data.local_num_col);
  return ret;
}

template <class T, class I, class O,
          class TT, class II, class OO>
void merge_scattered_crs_matrices_impl(
                         const std::vector<crs_matrix_local<T,I,O>>& vcrs,
                         TT* crnt_retval, 
                         II* crnt_retidx, 
                         OO* crnt_retoff,
                         size_t vsize, size_t osize) {
  size_t vcrssize = vcrs.size();
  size_t num_col = vcrs[0].local_num_col;
  for(size_t i = 1; i < vcrssize; i++) {
    if(vcrs[i].local_num_col != num_col)
      throw std::runtime_error
        ("merge_scattered_crs_matrices_impl: local_num_col is different");
  }
  // added for handling client side issues
  size_t g_vsize = 0, g_osize = 0;
  for(size_t i = 0; i < vcrssize; i++) {
    g_vsize += vcrs[i].val.size();
    g_osize += vcrs[i].off.size();
  }
  g_osize -= vcrssize -1; //adjusting global offset size
  if(!(g_vsize == vsize && g_osize == osize))
    throw std::runtime_error
      ("merge_scattered_crs_matrices_impl: calculated and expected sizes differ!\n");
  O to_add = 0;
  for(size_t i = 0; i < vcrssize; i++) {
    auto valp = vcrs[i].val.data();
    auto idxp = vcrs[i].idx.data();
    auto offp = vcrs[i].off.data();
    auto nnz = vcrs[i].val.size();
    for(size_t j = 0; j < nnz; j++) {
      crnt_retval[j] = static_cast<TT>(valp[j]);
      crnt_retidx[j] = static_cast<II>(idxp[j]);
    }
    crnt_retval += nnz;
    crnt_retidx += nnz;
    auto src_num_row = vcrs[i].local_num_row;
    for(size_t j = 0; j < src_num_row; j++) {
      crnt_retoff[j] = static_cast<OO>(offp[j] + to_add);
    }
    to_add += offp[src_num_row];
    crnt_retoff += src_num_row;
  }
  crnt_retoff[0] = static_cast<OO>(to_add); // addeed final count
}

template <class T, class I, class O>
crs_matrix_local<T,I,O>
merge_scattered_crs_matrices(const std::vector<crs_matrix_local<T,I,O>>& vcrs) {
  crs_matrix_local<T,I,O> ret;
  size_t vcrssize = vcrs.size();
  if(vcrssize == 0) return ret;
  size_t num_row = 0;
  for(size_t i = 0; i < vcrssize; i++) num_row += vcrs[i].local_num_row;
  size_t total_nnz = 0;
  for(size_t i = 0; i < vcrssize; i++) total_nnz += vcrs[i].val.size();
  ret.local_num_col = vcrs[0].local_num_col;
  ret.local_num_row = num_row;
  ret.val.resize(total_nnz);
  ret.idx.resize(total_nnz);
  ret.off.resize(num_row + 1);
  auto crnt_retval = ret.val.data();
  auto crnt_retidx = ret.idx.data();
  auto crnt_retoff = ret.off.data();
  merge_scattered_crs_matrices_impl(vcrs, crnt_retval, crnt_retidx, 
                                    crnt_retoff, total_nnz, num_row + 1);
  return ret;
}

#if defined(_SX) || defined(__ve__)
/*
  This version vectorize column dimension of rowmajor matrix
 */
template <class T, class I, class O>
void crs_matrix_spmm_impl2(const crs_matrix_local<T,I,O>& mat,
                           T* retvalp, const T* vvalp, size_t num_col) {
  const T* valp = &mat.val[0];
  const I* idxp = &mat.idx[0];
  const O* offp = &mat.off[0];
  T current_sum[CRS_SPMM_VLEN];
#pragma _NEC vreg(current_sum)
  for(size_t i = 0; i < CRS_SPMM_VLEN; i++) {
    current_sum[i] = 0;
  }
  size_t each = num_col / CRS_SPMM_VLEN;
  size_t rest = num_col % CRS_SPMM_VLEN;
  for(size_t r = 0; r < mat.local_num_row; r++) {
    for(size_t e = 0; e < each; e++) {
      for(O c = offp[r]; c < offp[r+1]; c++) {
        for(size_t mc = 0; mc < CRS_SPMM_VLEN; mc++) {
          current_sum[mc] +=
            valp[c] * vvalp[idxp[c] * num_col + CRS_SPMM_VLEN * e + mc];
        }
      }
      for(size_t mc = 0; mc < CRS_SPMM_VLEN; mc++) {
        retvalp[r * num_col + CRS_SPMM_VLEN * e + mc] += current_sum[mc];
      }
      for(size_t i = 0; i < CRS_SPMM_VLEN; i++) {
        current_sum[i] = 0;
      }
    }
    for(O c = offp[r]; c < offp[r+1]; c++) {
      for(size_t mc = 0; mc < rest; mc++) {
        current_sum[mc] +=
          valp[c] * vvalp[idxp[c] * num_col + CRS_SPMM_VLEN * each + mc];
      }
    }
    for(size_t mc = 0; mc < rest; mc++) {
      retvalp[r * num_col + CRS_SPMM_VLEN * each + mc] += current_sum[mc];
    }
    for(size_t i = 0; i < rest; i++) {
      current_sum[i] = 0;
    }
  }
}

template <class T, class I, class O>
void crs_matrix_spmm_impl(const crs_matrix_local<T,I,O>& mat,
                          T* retvalp, const T* vvalp, size_t num_col) {
  if(num_col < CRS_SPMM_THR) {
    const T* valp = &mat.val[0];
    const I* idxp = &mat.idx[0];
    const O* offp = &mat.off[0];
    for(size_t r = 0; r < mat.local_num_row; r++) {
      size_t mc = 0;
      for(; mc + 15 < num_col; mc += 16) {
        for(O c = offp[r]; c < offp[r+1]; c++) {
          retvalp[r * num_col + mc] += 
            valp[c] * vvalp[idxp[c] * num_col + mc];
          retvalp[r * num_col + mc + 1] +=
            valp[c] * vvalp[idxp[c] * num_col + mc + 1];
          retvalp[r * num_col + mc + 2] +=
            valp[c] * vvalp[idxp[c] * num_col + mc + 2];
          retvalp[r * num_col + mc + 3] +=
            valp[c] * vvalp[idxp[c] * num_col + mc + 3];
          retvalp[r * num_col + mc + 4] +=
            valp[c] * vvalp[idxp[c] * num_col + mc + 4];
          retvalp[r * num_col + mc + 5] +=
            valp[c] * vvalp[idxp[c] * num_col + mc + 5];
          retvalp[r * num_col + mc + 6] +=
            valp[c] * vvalp[idxp[c] * num_col + mc + 6];
          retvalp[r * num_col + mc + 7] +=
            valp[c] * vvalp[idxp[c] * num_col + mc + 7];
          retvalp[r * num_col + mc + 8] += 
            valp[c] * vvalp[idxp[c] * num_col + mc + 8];
          retvalp[r * num_col + mc + 9] +=
            valp[c] * vvalp[idxp[c] * num_col + mc + 9];
          retvalp[r * num_col + mc + 10] +=
            valp[c] * vvalp[idxp[c] * num_col + mc + 10];
          retvalp[r * num_col + mc + 11] +=
            valp[c] * vvalp[idxp[c] * num_col + mc + 11];
          retvalp[r * num_col + mc + 12] +=
            valp[c] * vvalp[idxp[c] * num_col + mc + 12];
          retvalp[r * num_col + mc + 13] +=
            valp[c] * vvalp[idxp[c] * num_col + mc + 13];
          retvalp[r * num_col + mc + 14] +=
            valp[c] * vvalp[idxp[c] * num_col + mc + 14];
          retvalp[r * num_col + mc + 15] +=
            valp[c] * vvalp[idxp[c] * num_col + mc + 15];
        }
      }
      for(; mc + 7 < num_col; mc += 8) {
        for(O c = offp[r]; c < offp[r+1]; c++) {
          retvalp[r * num_col + mc] += 
            valp[c] * vvalp[idxp[c] * num_col + mc];
          retvalp[r * num_col + mc + 1] +=
            valp[c] * vvalp[idxp[c] * num_col + mc + 1];
          retvalp[r * num_col + mc + 2] +=
            valp[c] * vvalp[idxp[c] * num_col + mc + 2];
          retvalp[r * num_col + mc + 3] +=
            valp[c] * vvalp[idxp[c] * num_col + mc + 3];
          retvalp[r * num_col + mc + 4] +=
            valp[c] * vvalp[idxp[c] * num_col + mc + 4];
          retvalp[r * num_col + mc + 5] +=
            valp[c] * vvalp[idxp[c] * num_col + mc + 5];
          retvalp[r * num_col + mc + 6] +=
            valp[c] * vvalp[idxp[c] * num_col + mc + 6];
          retvalp[r * num_col + mc + 7] +=
            valp[c] * vvalp[idxp[c] * num_col + mc + 7];
        }
      }
      for(; mc + 3 < num_col; mc += 4) {
        for(O c = offp[r]; c < offp[r+1]; c++) {
          retvalp[r * num_col + mc] += 
            valp[c] * vvalp[idxp[c] * num_col + mc];
          retvalp[r * num_col + mc + 1] +=
            valp[c] * vvalp[idxp[c] * num_col + mc + 1];
          retvalp[r * num_col + mc + 2] +=
            valp[c] * vvalp[idxp[c] * num_col + mc + 2];
          retvalp[r * num_col + mc + 3] +=
            valp[c] * vvalp[idxp[c] * num_col + mc + 3];
        }
      }
      for(; mc < num_col; mc++) {
        for(O c = offp[r]; c < offp[r+1]; c++) {
          retvalp[r * num_col + mc] +=
            valp[c] * vvalp[idxp[c] * num_col + mc];
        }
      }
    }
    /*
      for(size_t r = 0; r < mat.local_num_row; r++) {
      O c = offp[r];
      for(; c + CRS_VLEN < offp[r+1]; c += CRS_VLEN) {
      for(size_t mc = 0; mc < num_col; mc++) {
      #pragma cdir on_adb(vvalp)
      for(O i = 0; i < CRS_VLEN; i++) {
      retvalp[r * num_col + mc] +=
      valp[c+i] * vvalp[idxp[c+i] * num_col + mc];
      }
      }
      }
      for(size_t mc = 0; mc < num_col; mc++) {
      #pragma cdir on_adb(vvalp)
      for(O i = 0; c + i < offp[r+1]; i++) {
      retvalp[r * num_col + mc] +=
      valp[c+i] * vvalp[idxp[c+i] * num_col + mc];
      }
      }
      c = offp[r+1];
      }
    */
  } else {
    crs_matrix_spmm_impl2(mat, retvalp, vvalp, num_col);
  }
}
#else
template <class T, class I, class O>
void crs_matrix_spmm_impl(const crs_matrix_local<T,I,O>& mat,
                          T* retvalp, const T* vvalp, size_t num_col) {
  const T* valp = &mat.val[0];
  const I* idxp = &mat.idx[0];
  const O* offp = &mat.off[0];
  for(size_t r = 0; r < mat.local_num_row; r++) {
    for(O c = offp[r]; c < offp[r+1]; c++) {
      for(size_t mc = 0; mc < num_col; mc++) {
        retvalp[r * num_col + mc] +=
          valp[c] * vvalp[idxp[c] * num_col + mc];
      }
    }
  }
}
#endif

template <class T, class I, class O>
rowmajor_matrix_local<T> operator*(const crs_matrix_local<T,I,O>& mat,
                                   const rowmajor_matrix_local<T>& v) {
  rowmajor_matrix_local<T> ret(mat.local_num_row, v.local_num_col);
  T* retvalp = &ret.val[0];
  const T* vvalp = &v.val[0];
  crs_matrix_spmm_impl(mat, retvalp, vvalp, v.local_num_col);
  return ret;
}

template <class T, class I, class O>
std::vector<T> call_crs_mv(const crs_matrix_local<T,I,O>& mat,
                           const std::vector<T>& v) {
  return mat * v;
}

#ifdef SPARSE_MV_USE_ALLREDUCE
template <class T>
struct call_allgatherv {
  call_allgatherv(){}
  call_allgatherv(std::vector<int>& count) : count(count) {}
  
  std::vector<T> operator()(std::vector<T>& v) {
    size_t size = count.size();
    std::vector<int> displs(size);
    auto displsp = displs.data();
    auto countp = count.data();
    displsp[0] = 0;
    for(size_t i = 1; i < size; i++) displsp[i] = displsp[i-1] + countp[i-1];
    size_t total = displsp[size-1] + countp[size-1];
    std::vector<T> ret(total);
    typed_allgatherv<T>(v.data(), static_cast<int>(v.size()), ret.data(),
                        count.data(), displs.data(), frovedis_comm_rpc);
    return ret;
  }
  
  std::vector<int> count;
  SERIALIZE(count)
};
#endif

template <class T, class I, class O>
dvector<T> operator*(crs_matrix<T,I,O>& mat, dvector<T>& dv) {
// not that fast?
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
  return mat.data.map(call_crs_mv<T,I,O>, bdv).template moveto_dvector<T>();
}

template <class T>
template <class I, class O>
crs_matrix_local<T,I,O> rowmajor_matrix_local<T>::to_crs() {
  crs_matrix_local<T,I,O> ret;
  ret.local_num_row = local_num_row;
  ret.local_num_col = local_num_col;
  size_t nnz = 0;
  T* valp = val.data();
  size_t valsize = val.size();
  for(size_t i = 0; i < valsize; i++) {
    if(valp[i] != 0) nnz++;
  }
  ret.val.resize(nnz);
  ret.idx.resize(nnz);
  ret.off.resize(local_num_row + 1);
  size_t current = 0;
  T* retvalp = ret.val.data();
  I* retidxp = ret.idx.data();
  O* retoffp = ret.off.data();
  for(size_t i = 0; i < local_num_row; i++) {
    for(size_t j = 0; j < local_num_col; j++) {
      T v = valp[i * local_num_col + j];
      if(v != 0) {
        retvalp[current] = v;
        retidxp[current] = j;
        current++;
      }
    }
    retoffp[i+1] = current;
  }
  return ret;
}

template <class T>
template <class I, class O>
crs_matrix<T,I,O> rowmajor_matrix<T>::to_crs() {
  crs_matrix<T,I,O> ret;
  ret.set_num(num_row, num_col);
  ret.data = data.map(+[](rowmajor_matrix_local<T>& m){return m.to_crs();});
  return ret;
}

template <class T, class I, class O>
crs_matrix<T,I,O>&
crs_matrix<T,I,O>::align_as(const std::vector<size_t>& sizes) {
  auto off_tmp = make_node_local_allocate<std::vector<O>>();
  data.mapv(+[](crs_matrix_local<T,I,O>& m, std::vector<O>& o) {
      o.resize(m.off.size() - 1);
      auto op = o.data();
      auto offp = m.off.data();
      for(size_t i = 0; i < m.off.size()-1; i++) {
        op[i] = offp[i+1] - offp[i];
      }
    }, off_tmp);
  auto off_tmp2 = off_tmp.template moveto_dvector<O>().align_as(sizes).
    moveto_node_local();
  data.mapv(+[](crs_matrix_local<T,I,O>& m, std::vector<O>& o) {
      m.off.resize(o.size() + 1);
      auto op = o.data();
      auto offp = m.off.data();
      offp[0] = 0;
      prefix_sum(op, offp+1, o.size());
    }, off_tmp2);
  auto val_sizes = data.map(+[](crs_matrix_local<T,I,O>& m) {
      return static_cast<size_t>(m.off[m.off.size() - 1]);}).gather();
  auto val_tmp = make_node_local_allocate<std::vector<T>>();
  data.mapv(+[](crs_matrix_local<T,I,O>& m, std::vector<T>& v) 
            {m.val.swap(v);}, val_tmp);
  auto val_tmp2 = val_tmp.template moveto_dvector<T>().align_as(val_sizes).
    moveto_node_local();
  data.mapv(+[](crs_matrix_local<T,I,O>& m, std::vector<T>& v)
            {m.val.swap(v);}, val_tmp2);

  auto idx_tmp = make_node_local_allocate<std::vector<I>>();
  data.mapv(+[](crs_matrix_local<T,I,O>& m, std::vector<I>& i) 
            {m.idx.swap(i);}, idx_tmp);
  auto idx_tmp2 = idx_tmp.template moveto_dvector<I>().align_as(val_sizes).
    moveto_node_local();
  data.mapv(+[](crs_matrix_local<T,I,O>& m, std::vector<I>& i)
            {m.idx.swap(i);}, idx_tmp2);

  data.mapv(+[](crs_matrix_local<T,I,O>& m)
            {m.local_num_row = m.off.size() - 1;});
  return *this;
}

#if MPI_VERSION >= 3

template <class T, class I, class O>
shared_vector<T> make_shared_vector_as_spmv_output(crs_matrix<T,I,O>& crs) {
  auto sizes = crs.data.map(+[](crs_matrix_local<T,I,O>& m) {
      // type of size_t depends on architecture, so cast to long long
      long long size = static_cast<long long>(m.local_num_row);
      long long total_in_shm = 0;
      MPI_Allreduce(&size, &total_in_shm, 1, MPI_LONG_LONG, MPI_SUM,
                    frovedis_shm_comm);
      return static_cast<size_t>(total_in_shm);
    });
  return make_shared_vector<T>(sizes);
}

template <class T, class I, class O>
shared_vector<T> make_shared_vector_as_spmm_output(crs_matrix<T,I,O>& crs,
                                                   size_t rm_num_col) {
  auto bc_rm_num_col = broadcast(rm_num_col);
  auto sizes = crs.data.map
    (+[](crs_matrix_local<T,I,O>& m, size_t rm_num_col) {
      long long size = static_cast<long long>(m.local_num_row);
      long long total_in_shm = 0;
      MPI_Allreduce(&size, &total_in_shm, 1, MPI_LONG_LONG, MPI_SUM,
                    frovedis_shm_comm);
      return static_cast<size_t>(total_in_shm * rm_num_col);
    }, bc_rm_num_col);
  return make_shared_vector<T>(sizes);
}

/*
  Assign working area in the shared vector
  Please make sure that the shared vector is not destructed
  while using ptr_t!
 */
template <class T, class I, class O>
ptr_t<T> assign_working_area_for_spmv(shared_vector<T>& sv,
                                      crs_matrix<T,I,O>& crs) {
  ptr_t<T> ret;
  ret.data = crs.data.map
    (+[](crs_matrix_local<T,I,O>& m, shared_vector_local<T>& s) {
      long long size = static_cast<long long>(m.local_num_row);
      std::vector<long long> sizes(frovedis_shm_comm_size);
      MPI_Allgather(&size, 1, MPI_LONG_LONG, sizes.data(), 1, MPI_LONG_LONG,
                    frovedis_shm_comm);
      long long my_offset = 0;
      auto sizesp = sizes.data();
      for(size_t i = 0; i < frovedis_shm_self_rank; i++) my_offset += sizesp[i];
      ptr_t_local<T> local_ret;
      local_ret.intptr = reinterpret_cast<intptr_t>(s.data() + my_offset);
      local_ret.size = size;
      return local_ret;
    }, sv.data);
  return ret;
}

template <class T, class I, class O>
ptr_t<T> assign_working_area_for_spmm(shared_vector<T>& sv,
                                      crs_matrix<T,I,O>& crs,
                                      size_t rm_num_col) {
  ptr_t<T> ret;
  auto bc_rm_num_col = broadcast(rm_num_col);
  ret.data = crs.data.map
    (+[](crs_matrix_local<T,I,O>& m, shared_vector_local<T>& s, size_t rc) {
      long long size = static_cast<long long>(m.local_num_row);
      std::vector<long long> sizes(frovedis_shm_comm_size);
      MPI_Allgather(&size, 1, MPI_LONG_LONG, sizes.data(), 1, MPI_LONG_LONG,
                    frovedis_shm_comm);
      long long my_offset = 0;
      auto sizesp = sizes.data();
      for(size_t i = 0; i < frovedis_shm_self_rank; i++) my_offset += sizesp[i];
      ptr_t_local<T> local_ret;
      // cast to long long is needed to make VE compiler happy
      local_ret.intptr = reinterpret_cast<intptr_t>
      (s.data() + my_offset * static_cast<long long>(rc));
      local_ret.size = size * rc;
      return local_ret;
    }, sv.data, bc_rm_num_col);
  return ret;
}

template <class T, class I, class O>
void spmv(crs_matrix<T,I,O>& crs, shared_vector<T>& input, ptr_t<T>& output) {
  output.data.mapv(+[](ptr_t_local<T>& ptr) {
      T* p = ptr.data();
      for(size_t i = 0; i < ptr.size; i++) p[i] = 0;
    });
  crs.data.mapv
    (+[](crs_matrix_local<T,I,O>& crs, shared_vector_local<T>& input,
         ptr_t_local<T>& ptr) {
      crs_matrix_spmv_impl(crs, ptr.data(), input.data());
    }, input.data, output.data);
}

template <class T, class I, class O>
void spmm(crs_matrix<T,I,O>& crs, shared_vector<T>& input, size_t num_col,
          ptr_t<T>& output) {
  // zero clear output
  output.data.mapv(+[](ptr_t_local<T>& ptr) {
      T* p = ptr.data();
      for(size_t i = 0; i < ptr.size; i++) p[i] = 0;
    });
  auto bcast_num_col = broadcast(num_col);
  crs.data.mapv
    (+[](crs_matrix_local<T,I,O>& crs, shared_vector_local<T>& input,
         size_t num_col, ptr_t_local<T>& ptr) {
      crs_matrix_spmm_impl(crs, ptr.data(), input.data(), num_col);
    }, input.data, bcast_num_col, output.data);
}

/*
  Just for measuring; use same routine for both x86 and VE
 */
template <class T, class I, class O>
std::vector<double>
crs_matrix_spmm_measure(const crs_matrix_local<T,I,O>& mat,
                        T* retvalp, const T* vvalp, size_t num_col) {
  std::vector<double> measure(mat.local_num_row);
  if(mat.local_num_row == 0) return measure;
  double* measurep = measure.data();
  const T* valp = &mat.val[0];
  const I* idxp = &mat.idx[0];
  const O* offp = &mat.off[0];
  T current_sum[CRS_SPMM_VLEN];
#pragma _NEC vreg(current_sum)
  for(size_t i = 0; i < CRS_SPMM_VLEN; i++) {
    current_sum[i] = 0;
  }
  size_t each = num_col / CRS_SPMM_VLEN;
  size_t rest = num_col % CRS_SPMM_VLEN;
  double end = get_dtime();
  for(size_t r = 0; r < mat.local_num_row; r++) {
    double start = end;
    for(size_t e = 0; e < each; e++) {
      for(O c = offp[r]; c < offp[r+1]; c++) {
        for(size_t mc = 0; mc < CRS_SPMM_VLEN; mc++) {
          current_sum[mc] +=
            valp[c] * vvalp[idxp[c] * num_col + CRS_SPMM_VLEN * e + mc];
        }
      }
      for(size_t mc = 0; mc < CRS_SPMM_VLEN; mc++) {
        retvalp[r * num_col + CRS_SPMM_VLEN * e + mc] += current_sum[mc];
      }
      for(size_t i = 0; i < CRS_SPMM_VLEN; i++) {
        current_sum[i] = 0;
      }
    }
    for(O c = offp[r]; c < offp[r+1]; c++) {
      for(size_t mc = 0; mc < rest; mc++) {
        current_sum[mc] +=
          valp[c] * vvalp[idxp[c] * num_col + CRS_SPMM_VLEN * each + mc];
      }
    }
    for(size_t mc = 0; mc < rest; mc++) {
      retvalp[r * num_col + CRS_SPMM_VLEN * each + mc] += current_sum[mc];
    }
    for(size_t i = 0; i < rest; i++) {
      current_sum[i] = 0;
    }
    if(r % TO_SKIP_REBALANCE == 0) end = get_dtime();
    measurep[r] = end - start;
  }
  return measure;
}

template <class T, class I, class O>
void rebalance_for_spmm(crs_matrix<T,I,O>& crs, shared_vector<T>& input,
                        size_t rm_num_col, ptr_t<T>& output) {
  // zero clear input/output
  input.data.mapv(+[](shared_vector_local<T>& in) {
      T* p = in.data();
      for(size_t i = 0; i < in.size; i++) p[i] = 0;
    });
  output.data.mapv(+[](ptr_t_local<T>& ptr) {
      T* p = ptr.data();
      for(size_t i = 0; i < ptr.size; i++) p[i] = 0;
    });
  auto bcast_rm_num_col = broadcast(rm_num_col);
  auto measure = crs.data.map
    (+[](crs_matrix_local<T,I,O>& crs, shared_vector_local<T>& input,
         size_t rm_num_col, ptr_t_local<T>& ptr) {
      return crs_matrix_spmm_measure(crs, ptr.data(), input.data(), rm_num_col);
    }, input.data, bcast_rm_num_col, output.data)
    .template moveto_dvector<double>().gather();
  auto pxmeasure = prefix_sum(measure);
  size_t nodesize = get_nodesize();
  auto each  = pxmeasure[pxmeasure.size() - 1] / static_cast<double>(nodesize);
  std::vector<size_t> sizes(nodesize);
  auto sizesp = sizes.data();
  auto start = pxmeasure.begin();
  std::vector<double>::iterator end;
  for(size_t i = 0; i < nodesize; i++) {
    if(i == nodesize - 1)
      end = pxmeasure.end();
    else
      end = std::lower_bound(start, pxmeasure.end(), each * (i+1));
    sizesp[i] = end - start;
    if(end == pxmeasure.end()) {
      for(size_t j = i+1; j < nodesize; j++) sizesp[j] = 0;
      break;
    } else
      start = end;
  }
  output.data.mapv(+[](ptr_t_local<T>& ptr) {
      T* p = ptr.data();
      for(size_t i = 0; i < ptr.size; i++) p[i] = 0;
    });
  crs.align_as(sizes);
}

#endif

template <class T, class I, class O>
std::vector<T>
sum_of_rows_helper(crs_matrix_local<T, I, O>& mat,
                   std::vector<I>& count) {
  auto nrow = mat.local_num_row;
  auto ncol = mat.local_num_col;
  std::vector<T> ret(ncol);
  count.resize(ncol);
  auto rptr = ret.data();
  auto cptr = count.data();
  auto valptr = mat.val.data();
  auto idxptr = mat.idx.data();
  auto offptr = mat.off.data();
  for(size_t i = 0; i < nrow; ++i) {
#pragma _NEC ivdep
    for(size_t j = offptr[i]; j < offptr[i + 1]; ++j) {
      rptr[idxptr[j]] += valptr[j]; // summing
      cptr[idxptr[j]] += 1; // counting nnz column-wise
    }
  }
  return ret;
}

template <class T, class I, class O>
std::vector<T>
sum_of_rows(crs_matrix<T, I, O>& mat,
            std::vector<I>& count) {
  auto lcount = make_node_local_allocate<std::vector<I>>();
  auto sum = mat.data.map(sum_of_rows_helper<T,I,O>, lcount).vector_sum();
  count = lcount.vector_sum();
  return sum;
}

template <class T, class I, class O>
std::vector<T>
squared_sum_of_rows_helper(crs_matrix_local<T, I, O>& mat,
                           std::vector<I>& count) {
  auto nrow = mat.local_num_row;
  auto ncol = mat.local_num_col;
  std::vector<T> ret(ncol);
  count.resize(ncol);
  auto rptr = ret.data();
  auto cptr = count.data();
  auto valptr = mat.val.data();
  auto idxptr = mat.idx.data();
  auto offptr = mat.off.data();
  for(size_t i = 0; i < nrow; ++i) {
#pragma _NEC ivdep
    for(size_t j = offptr[i]; j < offptr[i + 1]; ++j) {
      rptr[idxptr[j]] += (valptr[j] * valptr[j]); // squared summing
      cptr[idxptr[j]] += 1; // counting
    }
  }
  return ret;
}

template <class T, class I, class O>
std::vector<T>
squared_sum_of_rows(crs_matrix<T, I, O>& mat,
                    std::vector<I>& count) {
  auto lcount = make_node_local_allocate<std::vector<I>>();
  auto sqsum = mat.data.map(squared_sum_of_rows_helper<T,I,O>, lcount).vector_sum();
  count = lcount.vector_sum();
  return sqsum;
}

template <class T, class I, class O>
void crs_sub_vector_row(crs_matrix_local<T, I, O>& mat,
                        std::vector<T>& vec) {
  auto vptr = vec.data();
  auto valptr = mat.val.data();
  auto idxptr = mat.idx.data();
  auto nnz = mat.val.size();
  for(size_t i = 0; i < nnz; ++i) valptr[i] -= vptr[idxptr[i]];
}

template <class T, class I, class O>
void crs_add_vector_row(crs_matrix_local<T, I, O>& mat,
                        std::vector<T>& vec) {
  auto vptr = vec.data();
  auto valptr = mat.val.data();
  auto idxptr = mat.idx.data();
  auto nnz = mat.val.size();
  for(size_t i = 0; i < nnz; ++i) valptr[i] += vptr[idxptr[i]];
}

template <class T, class I, class O>
void crs_mul_vector_row(crs_matrix_local<T, I, O>& mat,
                        std::vector<T>& vec) {
  auto vptr = vec.data();
  auto valptr = mat.val.data();
  auto idxptr = mat.idx.data();
  auto nnz = mat.val.size();
  for(size_t i = 0; i < nnz; ++i) valptr[i] *= vptr[idxptr[i]];
}

template <class T, class I, class O>
void scale_matrix(crs_matrix<T, I, O>& mat,
                  std::vector<T>& vec) {
  mat.data.mapv(crs_mul_vector_row<T,I,O>, broadcast(vec));
}

template <class T, class I, class O>
std::vector<T>
compute_mean(crs_matrix<T, I, O>& mat,
             std::vector<I>& count,
             bool with_nnz = false) {
  auto ncol = mat.num_col;
  auto nrow = mat.num_row;
  if(nrow == 0)
    throw std::runtime_error("matrix with ZERO rows for mean computation!");
  auto tmp = sum_of_rows(mat, count);
  auto tmpptr = tmp.data();
  // tmp: sum of rows will inplace be updated with mean
  if(with_nnz) {
    auto cptr = count.data();
    for(size_t i = 0; i < ncol; ++i) {
      if (cptr[i] != 0) tmpptr[i] /= cptr[i];
    }
  }
  else{
    T to_mult = static_cast<T>(1) / static_cast<T>(nrow);
    for(size_t i = 0; i < ncol; ++i) tmpptr[i] *= to_mult;
  }
  return tmp;
}

template <class T, class I, class O>
std::vector<T>
compute_mean(crs_matrix<T, I, O>& mat,
             bool with_nnz = false) {
  std::vector<I> count;
  return compute_mean(mat, count, with_nnz);
}

template <class T, class I, class O>
void centerize(crs_matrix<T,I,O>& mat, std::vector<T>& mean) {
  throw std::runtime_error("cannot centerize sparse matrices!\n");
}

template <class T, class I, class O>
void centerize(crs_matrix<T,I,O>& mat) {
  throw std::runtime_error("cannot centerize sparse matrices!\n");
}

template <class T, class I, class O>
void decenterize(crs_matrix<T,I,O>& mat, std::vector<T>& mean) {
  throw std::runtime_error("cannot decenterize sparse matrices!\n");
}

template <class T, class I, class O>
std::vector<T>
compute_stddev(crs_matrix<T, I, O>& mat, 
               std::vector<T>& mean, 
               bool sample_stddev = true,
               bool with_nnz = false) {
  auto ncol = mat.num_col;
  auto nrow = mat.num_row;
  if(nrow < 2)
    throw std::runtime_error("cannot compute stddev if number of row is 0 or 1");
  mat.data.mapv(crs_sub_vector_row<T,I,O>, broadcast(mean));
  std::vector<I> count; // count of non-zeros in each column
  auto tmp = squared_sum_of_rows(mat, count);
  auto tmpptr = tmp.data();
  auto cptr = count.data();
  // tmp: sqsum of rows will inplace be updated with reciprocal of stddev
  if(!with_nnz) {
    auto meanp = mean.data();
    for(size_t i = 0; i < ncol; ++i) {
      auto nzeros = nrow - cptr[i];
      tmpptr[i] += (meanp[i] * meanp[i] * nzeros);
    }
    T to_div;
    if(sample_stddev) to_div = static_cast<T>(nrow - 1);
    else to_div = static_cast<T>(nrow);
    for(size_t i = 0; i < ncol; ++i) {
      if(tmpptr[i] == 0) tmpptr[i] = 1.0;
      else tmpptr[i] = sqrt(tmpptr[i] / to_div);
    }
  }
  else {
    if(sample_stddev) {
      for(size_t i = 0; i < ncol; ++i) {
        if(tmpptr[i] == 0) tmpptr[i] = 1.0;
        else tmpptr[i] = sqrt(tmpptr[i] / (cptr[i] - 1));
      }
    }
    else {
      for(size_t i = 0; i < ncol; ++i) {
        if(tmpptr[i] == 0) tmpptr[i] = 1.0;
        else tmpptr[i] = sqrt(tmpptr[i] / cptr[i]);
      }
    }
  }
  return tmp;
}

template <class T, class I, class O>
std::vector<T>
compute_stddev(crs_matrix<T, I, O>& mat, 
               bool sample_stddev = true,
               bool with_nnz = false) {
  auto mean = compute_mean(mat, with_nnz);
  return compute_stddev(mat, mean, sample_stddev, with_nnz);
}

template <class T, class I, class O>
void standardize(crs_matrix<T, I, O>& mat,
                 std::vector<T>& mean, 
                 bool sample_stddev = true,
                 bool with_nnz = false) {
  auto ncol = mat.num_col;
  auto nrow = mat.num_row;
  if(nrow < 2)
    throw std::runtime_error("cannot compute stddev if number of row is 0 or 1");
  mat.data.mapv(crs_sub_vector_row<T,I,O>, broadcast(mean));
  std::vector<I> count; // count of non-zeros in each column
  auto tmp = squared_sum_of_rows(mat, count);
  auto tmpptr = tmp.data();
  auto cptr = count.data();
  // tmp: sqsum of rows will inplace be updated with reciprocal of stddev
  if(!with_nnz) {
    auto meanp = mean.data();
    for(size_t i = 0; i < ncol; ++i) {
      auto nzeros = nrow - cptr[i];
      tmpptr[i] += (meanp[i] * meanp[i] * nzeros);
    }
    T to_div;
    if(sample_stddev) to_div = static_cast<T>(nrow - 1);
    else to_div = static_cast<T>(nrow);
    for(size_t i = 0; i < ncol; ++i) {
      if(tmpptr[i] == 0) tmpptr[i] = 1.0;
      else tmpptr[i] = sqrt(to_div / tmpptr[i]);
    }
  }
  else {
    if(sample_stddev) {
      for(size_t i = 0; i < ncol; ++i) {
        if(tmpptr[i] == 0) tmpptr[i] = 1.0;
        else tmpptr[i] = sqrt((cptr[i] - 1) / tmpptr[i]);
      }
    }
    else {
      for(size_t i = 0; i < ncol; ++i) {
        if(tmpptr[i] == 0) tmpptr[i] = 1.0;
        else tmpptr[i] = sqrt(cptr[i] / tmpptr[i]);
      }
    }
  }
  // tmp: reciprocal of stddev
  scale_matrix(mat, tmp);
}

template <class T, class I, class O>
void standardize(crs_matrix<T, I, O>& mat,
                 bool sample_stddev = true,
                 bool with_nnz = false) {
  auto mean = compute_mean(mat, with_nnz);
  standardize(mat, mean, sample_stddev, with_nnz);
}

}
#endif
