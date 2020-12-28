#ifndef _GRAPH_COMMON_
#define _GRAPH_COMMON_

#include <frovedis/core/vector_operations.hpp>
#include <frovedis/matrix/crs_matrix.hpp>

#define WEIGHT_VLEN 1024

namespace frovedis {

template <class T>
bool check_if_exist(size_t srcid,
                    std::vector<T>& num_outgoing,
                    std::vector<T>& num_incoming) {
  return (num_incoming[srcid] != 0 || num_outgoing[srcid] != 0);
}

template <class T>
int is_weighted_sum_helper(std::vector<T>& vec) {
  auto size = vec.size();
  for(size_t j = 0; j < size; j += WEIGHT_VLEN) {
    T sum = 0;
    auto vptr = vec.data() + j;
    auto remaining = size - j;
    if (remaining >= WEIGHT_VLEN) {
      for(size_t i = 0; i < WEIGHT_VLEN; ++i) sum += vptr[i];
      if (sum != WEIGHT_VLEN) return true;
    }
    else {
      for(size_t i = 0; i < remaining; ++i) sum += vptr[i];
      if (sum != remaining) return true;
    }
  }
  return false;
}

template <class T, class I, class O>
int has_weight_helper(crs_matrix_local<T,I,O>& mat) {
  return is_weighted_sum_helper(mat.val);
}

template <class T, class I, class O>
bool has_weight(crs_matrix<T,I,O>& mat) {
  auto eq = mat.data.map(has_weight_helper<T,I,O>)
                    .reduce(add<int>);
  return (eq > 0); // if any process has weight information
}

template <class T, class I, class O>
int check_equal_helper(crs_matrix_local<T,I,O>& amat,
                       crs_matrix_local<T,I,O>& bmat) {
  return (amat.local_num_row == bmat.local_num_row &&
          amat.local_num_col == bmat.local_num_col &&
          amat.val == bmat.val && amat.idx == bmat.idx &&
          amat.off == bmat.off);
}

template <class T, class I, class O>
bool has_direction(crs_matrix<T,I,O>& amat,
                   crs_matrix<T,I,O>& bmat) {
  auto eq = amat.data.map(check_equal_helper<T,I,O>, bmat.data)
                     .reduce(add<int>);
  return (eq != get_nodesize()); // if any process confirms "no"
}

template <class T, class I, class O>
std::vector<size_t>
count_edges(crs_matrix_local<T,I,O>& mat) {
  auto nrow = mat.local_num_row;
  std::vector<size_t> num_out(nrow);
  auto offptr = mat.off.data();
  auto retptr = num_out.data();
  for(size_t i = 0; i < nrow; ++i) {
    retptr[i] = offptr[i+1] - offptr[i];
  }
  return num_out;
}

template <class T, class I, class O>
void set_local_ncol(crs_matrix_local<T,I,O>& mat, size_t ncol) {
  mat.local_num_col = ncol;
}

template <class T, class I, class O>
void set_local_nrow(crs_matrix_local<T,I,O>& mat, size_t diff) {
  // assigns empty rows to last process
  if (get_selfid() == get_nodesize() - 1) {
    auto offsz = mat.off.size();
    auto offp = mat.off.data();
    auto nnz = mat.off[offsz - 1];
    std::vector<O> new_off(offsz + diff);
    auto new_offsz = new_off.size();
    auto newoffp = new_off.data();
    for(size_t i = 0; i < offsz; ++i) newoffp[i] = offp[i];
    for(size_t i = offsz; i < new_offsz; ++i) newoffp[i] = nnz;
    mat.off.swap(new_off);
    mat.local_num_row += diff;
  }
}

template <class T, class I, class O>
crs_matrix<T,I,O>
check_input(const crs_matrix<T,I,O>& mat) {
  auto nrow = mat.num_row;
  auto ncol = mat.num_col;
  if (nrow == ncol) return mat;
  else if(nrow > ncol) {
    auto ret = mat;
    ret.num_row = ret.num_col = nrow;
    ret.data.mapv(set_local_ncol<T,I,O>, broadcast(nrow));
    return ret;
  }
  else { // ncol > nrow
    auto ret = mat;
    auto diff = ncol - nrow;
    ret.num_row = ret.num_col = ncol;
    ret.data.mapv(set_local_nrow<T,I,O>, broadcast(diff));
    return ret;
  }
}

}
#endif
