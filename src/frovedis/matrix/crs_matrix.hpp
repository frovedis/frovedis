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

#include "rowmajor_matrix.hpp"

#if MPI_VERSION >= 3
#include "../core/shared_vector.hpp"
#endif

#define CRS_VLEN 4096

namespace frovedis {

template <class T, class I = size_t>
struct sparse_vector {
  sparse_vector(){}
  sparse_vector(const sparse_vector<T,I>& s) {
    val = s.val;
    idx = s.idx;
  }
  sparse_vector<T,I>& operator=(const sparse_vector<T,I>& s) {
    val = s.val;
    idx = s.idx;
    return *this;
  }
  sparse_vector(sparse_vector<T,I>&& s) {
    val.swap(s.val);
    idx.swap(s.idx);
  }
  sparse_vector<T,I>& operator=(sparse_vector<T,I>&& s) {
    val.swap(s.val);
    idx.swap(s.idx);
    return *this;
  }
  void debug_print() const {
    std::cout << "val : ";
    for(auto i: val) std::cout << i << " ";
    std::cout << std::endl;
    std::cout << "idx : ";
    for(auto i: idx) std::cout << i << " ";
    std::cout << std::endl;
  }
  std::vector<T> val;
  std::vector<I> idx;

  SERIALIZE(val, idx)
};

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
  }
  sparse_vector<T,I> get_row(size_t r) const;
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
  for(size_t i = 0; i < size; i++) r.val[i] = val[off[k]+i];
  for(size_t i = 0; i < size; i++) r.idx[i] = idx[off[k]+i];
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
  std::vector<size_t> num_item(local_num_col);
  std::vector<size_t> current_item(local_num_col);
  size_t* num_itemp = &num_item[0];
  size_t* current_itemp = &current_item[0];
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
  for(size_t ret_row = 0; ret_row < local_num_col; ret_row++) {
    ret_offp[ret_row + 1] = ret_offp[ret_row] + num_itemp[ret_row];
  }
  for(size_t src_row = 0; src_row < local_num_row; src_row++) {
#pragma cdir nodep
#pragma _NEC ivdep
    for(O src_pos = offp[src_row]; src_pos < offp[src_row + 1];
        src_pos++) {
      size_t src_col = idxp[src_pos];
      T src_val = valp[src_pos];
      size_t dst_pos = ret_offp[src_col] + current_itemp[src_col];
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

inline std::string remove_schema(const std::string& path) {
  auto idx = path.find(':', 0);
  if(idx == std::string::npos) return path;
  else return path.substr(idx + 1);
}

/*
  The directory should contain following files:
  - val: big endian binary data file that contains values of the matrix 
         in row major order
  - idx: big endian binary size_t (64bit) data that contains column index of
         the value
  - off: big endian binary size_t (64bit) data that contains offset of
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
#else  
  std::string numsfile = input + "/nums";
#endif
  std::ifstream numstr;
  numstr.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  numstr.open(numsfile.c_str());
  size_t num_row, num_col;
  numstr >> num_row >> num_col;
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
  std::ofstream numstr;
  numstr.exceptions(std::ofstream::failbit | std::ofstream::badbit);
  numstr.open(numsfile.c_str());
  numstr << local_num_row << "\n" << local_num_col << std::endl;
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
      is >> tmp_i >> tmp_j >> r.v;
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
  for(size_t i = 0; i < mat.val.size(); i++) {
    ret.val[i] = static_cast<TT>(mat.val[i]);
  }
  ret.idx.resize(mat.idx.size());
  for(size_t i = 0; i < mat.idx.size(); i++) {
    ret.idx[i] = static_cast<II>(mat.idx[i]);
  }
  ret.off.resize(mat.off.size());
  for(size_t i = 0; i < mat.off.size(); i++) {
    ret.off[i] = static_cast<OO>(mat.off[i]);
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
  void debug_print() {
    std::cout << "num_row = " << num_row
              << ", num_col = " << num_col << std::endl;
    auto g = data.gather();
    for(size_t i = 0; i < g.size(); i++) {
      std::cout << "node " << i << std::endl;
      g[i].debug_print();
    }
  }
  rowmajor_matrix<T> to_rowmajor();
  // for similar API as dvector
  void save(const std::string& file) {
    std::ofstream str(file.c_str());
    str << *this;
  }
  void savebinary(const std::string& file);
  crs_matrix<T,I,O> transpose();
  sparse_vector<T,I> get_row(size_t r);
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

template <class T, class I = size_t, class O = size_t>
size_t crs_get_local_num_row(crs_matrix_local<T,I,O>& mat) {
  return mat.local_num_row;
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

template <class T, class I, class O>
size_t crs_matrix_get_maxidx(crs_matrix_local<T,I,O>& m) {
  auto it = std::max_element(m.idx.begin(), m.idx.end());
  if(it != m.idx.end()) return *it;
  else return 0;
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
  for(size_t i = 0; i < mat.idx.size(); i++) {
    mat.idx[i]--;
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
  for(size_t i = 0; i < node_size; i++) {
    if(divide_row[i] != mat.local_num_row) {
      send_size[i] =
        mat.off[divide_row[i+1]] - mat.off[divide_row[i]];
    } else {
      send_size[i] = 0;
    }
  }
  std::vector<size_t> recv_size(node_size);
  MPI_Alltoall(&send_size[0], sizeof(size_t), MPI_CHAR,
               &recv_size[0], sizeof(size_t), MPI_CHAR, MPI_COMM_WORLD);
  size_t total_size = 0;
  for(size_t i = 0; i < node_size; i++) {
    total_size += recv_size[i];
  }
  std::vector<size_t> send_displ(node_size);
  std::vector<size_t> recv_displ(node_size);
  send_displ[0] = 0; recv_displ[0] = 0;
  for(size_t i = 1; i < node_size; i++) {
    send_displ[i] = send_displ[i-1] + send_size[i-1];
    recv_displ[i] = recv_displ[i-1] + recv_size[i-1];
  }
  std::vector<T> val_tmp(total_size);
  std::vector<I> idx_tmp(total_size);
  large_alltoallv(sizeof(T),
                  reinterpret_cast<char*>(const_cast<T*>(&mat.val[0])),
                  send_size, send_displ, 
                  reinterpret_cast<char*>(const_cast<T*>(&val_tmp[0])),
                  recv_size, recv_displ, 
                  MPI_COMM_WORLD);
  large_alltoallv(sizeof(I),
                  reinterpret_cast<char*>(const_cast<I*>(&mat.idx[0])),
                  send_size, send_displ,
                  reinterpret_cast<char*>(const_cast<I*>(&idx_tmp[0])),
                  recv_size, recv_displ,
                  MPI_COMM_WORLD);
  size_t my_num_row =
    divide_row[frovedis::get_selfid() + 1] - divide_row[frovedis::get_selfid()];
  std::vector<O> off_tmp((my_num_row + 1) * node_size);
  std::vector<size_t> off_send_size(node_size);
  std::vector<size_t> off_recv_size(node_size);
  std::vector<size_t> off_send_displ(node_size);
  std::vector<size_t> off_recv_displ(node_size);
  for(size_t i = 0; i < node_size; i++) {
    off_send_size[i] = divide_row[i+1] - divide_row[i] + 1;
  }
  for(size_t i = 0; i < node_size; i++) {
    off_recv_size[i] = my_num_row + 1;
  }
  off_send_displ[0] = 0; off_recv_displ[0] = 0; 
  for(size_t i = 1; i < node_size; i++) {
    off_send_displ[i] =
      off_send_displ[i-1] + off_send_size[i-1] - 1;
    off_recv_displ[i] = off_recv_displ[i-1] + off_recv_size[i-1];
  }
  large_alltoallv(sizeof(O),
                  reinterpret_cast<char*>(const_cast<O*>(&mat.off[0])),
                  off_send_size, off_send_displ,
                  reinterpret_cast<char*>(const_cast<O*>(&off_tmp[0])),
                  off_recv_size, off_recv_displ,
                  MPI_COMM_WORLD);
  std::vector<O> off_each((my_num_row + 1) * node_size);
  for(size_t n = 0; n < node_size; n++) {
    for(size_t r = 0; r < my_num_row + 1; r++) {
      off_each[(my_num_row + 1) * n + r] =
        off_tmp[(my_num_row + 1) * n + r] -
        off_tmp[(my_num_row + 1) * n]; 
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
  O* off_eachp = &off_each[0];
  size_t* recv_displp = &recv_displ[0];
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
  for(size_t i = 0; i < node_size + 1; i++) {
    auto it = std::lower_bound(tmp_offs.begin(), tmp_offs.end(),
                               each_size * i);
    if(it != tmp_offs.end()) {
      divide_row[i] = it - tmp_offs.begin();
    } else {
      divide_row[i] = num_col;
    }
  }
  auto bcast_divide_row =
    frovedis::make_node_local_broadcast<std::vector<size_t>>(divide_row);
  auto local_rows = data.map(crs_get_local_num_row<T,I,O>).gather();
  std::vector<size_t> col_shift(node_size + 1);
  col_shift[0] = 0;
  for(size_t i = 1; i < node_size + 1; i++) {
    col_shift[i] = col_shift[i-1] + local_rows[i-1];
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
  std::ifstream numstr;
  numstr.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  numstr.open(numsfile.c_str());
  size_t num_row, num_col;
  numstr >> num_row >> num_col;
  size_t node_size = get_nodesize();
  auto loadoff = make_dvector_loadbinary<O>(offfile).gather();
  size_t total_elm = loadoff[loadoff.size() - 1];
  size_t each_size = frovedis::ceil_div(total_elm, node_size);
  std::vector<size_t> divide_elm(node_size + 1);
  std::vector<size_t> divide_row(node_size + 1);
  for(size_t i = 0; i < node_size + 1; i++) {
    auto it = std::lower_bound(loadoff.begin(), loadoff.end(), each_size * i);
    if(it != loadoff.end()) {
      divide_elm[i] = *it;
      divide_row[i] = it - loadoff.begin();
    } else {
      divide_elm[i] = total_elm;
      divide_row[i] = num_row;
    }
  }
  std::vector<size_t> sizes(node_size);
  for(size_t i = 0; i < node_size; i++) {
    sizes[i] = divide_elm[i + 1] - divide_elm[i];
  }
  crs_matrix<T,I,O> ret;
  ret.data = make_node_local_allocate<crs_matrix_local<T,I,O>>();
  auto loadval = make_dvector_loadbinary<T>(valfile, sizes).moveto_node_local();
  ret.data.mapv(crs_matrix_set_val<T,I,O>, loadval);
  auto loadidx = make_dvector_loadbinary<I>(idxfile, sizes).moveto_node_local();
  ret.data.mapv(crs_matrix_set_idx<T,I,O>, loadidx);

  std::vector<size_t> sizes_off(node_size);
  for(size_t i = 0; i < node_size; i++) {
    sizes_off[i] = divide_row[i+1] - divide_row[i] + 1;
  }
  size_t adjust = 0;
  size_t current_node = 1;
  std::vector<O> newloadoff(loadoff.size() + node_size - 1);
  size_t j = 0;
  for(size_t i = 0; i < loadoff.size(); i++, j++) {
    newloadoff[j] = loadoff[i] - adjust;
    if(current_node < node_size && loadoff[i] == divide_elm[current_node]) {
      j++;
      newloadoff[j] = 0;
      adjust = loadoff[i];
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
  std::ofstream numstr;
  numstr.exceptions(std::ofstream::failbit | std::ofstream::badbit);
  numstr.open(numsfile.c_str());
  numstr << num_row << "\n" << num_col << std::endl;
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
  for(size_t i = 0; i < node_size + 1; i++) {
    auto it = std::lower_bound(data.off.begin(), data.off.end(),
                               each_size * i);
    if(it != data.off.end()) {
      divide_row[i] = it - data.off.begin();
    } else {
      divide_row[i] = data.local_num_row;
    }
  }
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

// TODO: write gather

// TODO: create a version of local_num_col > 256
#if defined(_SX) || defined(__ve__)
template <class T, class I, class O>
rowmajor_matrix_local<T> operator*(const crs_matrix_local<T,I,O>& mat,
                                   const rowmajor_matrix_local<T>& v) {
  rowmajor_matrix_local<T> ret(mat.local_num_row, v.local_num_col);
  T* retvalp = &ret.val[0];
  const T* vvalp = &v.val[0];
  const T* valp = &mat.val[0];
  const I* idxp = &mat.idx[0];
  const O* offp = &mat.off[0];
  size_t num_col = v.local_num_col;
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
  return ret;
}
#else
template <class T, class I, class O>
rowmajor_matrix_local<T> operator*(const crs_matrix_local<T,I,O>& mat,
                                   const rowmajor_matrix_local<T>& v) {
  rowmajor_matrix_local<T> ret(mat.local_num_row, v.local_num_col);
  T* retvalp = &ret.val[0];
  const T* vvalp = &v.val[0];
  const T* valp = &mat.val[0];
  const I* idxp = &mat.idx[0];
  const O* offp = &mat.off[0];
  size_t num_col = v.local_num_col;
  for(size_t r = 0; r < mat.local_num_row; r++) {
    for(O c = offp[r]; c < offp[r+1]; c++) {
      for(size_t mc = 0; mc < num_col; mc++) {
        retvalp[r * num_col + mc] +=
          valp[c] * vvalp[idxp[c] * num_col + mc];
      }
    }
  }
  return ret;
}
#endif

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
    displs[0] = 0;
    for(size_t i = 1; i < size; i++) displs[i] = displs[i-1] + count[i-1];
    size_t total = displs[size-1] + count[size-1];
    std::vector<T> ret(total);
    typed_allgatherv<T>(v.data(), static_cast<int>(v.size()), ret.data(),
                        count.data(), displs.data(), MPI_COMM_WORLD);
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
    for(size_t i = 0; i < size; i++) count[i] = sizes[i]; // cast to int
    bdv = dv.viewas_node_local().map(call_allgatherv<T>(count));
  } else {
    bdv = broadcast(dv.gather());
  }
#else
  auto bdv = broadcast(dv.gather());
#endif
  return mat.data.map(call_crs_mv<T,I,O>, bdv).template moveto_dvector<T>();
}

#if MPI_VERSION >= 3

template <class T, class I, class O>
std::vector<T> call_crs_mv_shared(const crs_matrix_local<T,I,O>& mat,
                                  shared_vector_local<T>& sv) {
  std::vector<T> ret(mat.local_num_row);
  crs_matrix_spmv_impl(mat, ret.data(), sv.data());
  return ret;
}

template <class T, class I, class O>
dvector<T> operator*(crs_matrix<T,I,O>& mat, shared_vector<T>& sdv) {
  if(mat.num_col != sdv.size)
    throw std::runtime_error("operator*: size of dvector does not match");
  return mat.data.map(call_crs_mv_shared<T,I,O>, sdv.data).template moveto_dvector<T>();
}

#endif

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

}
#endif
