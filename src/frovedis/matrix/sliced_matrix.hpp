#ifndef _SLICED_MATRIX_
#define _SLICED_MATRIX_

#include <iostream>
#include <vector>
#include "blockcyclic_matrix.hpp"
#include "colmajor_matrix.hpp"
#include "../core/exceptions.hpp"

namespace frovedis {

template <class T>
struct sliced_colmajor_matrix_local {
  sliced_colmajor_matrix_local() : data(NULL), ldm(0), 
        local_num_row(0), local_num_col(0) {}

  // implicit conversion: colmajor_matrix_local<T> => sliced_colmajor_matrix_local<T>
  sliced_colmajor_matrix_local(const colmajor_matrix_local<T>& inMat) {
    // 'inMat' needs to be const (to support rValue). 
    // But 'data' pointer is a non-const pointer. 
    // Thus the below casting is required.
    auto& mat = const_cast<colmajor_matrix_local<T>&> (inMat);
    data = mat.val.data();
    ldm  = mat.local_num_row;
    local_num_row = mat.local_num_row;
    local_num_col = mat.local_num_col;
  }

  sliced_colmajor_matrix_local(const std::vector<T>& inVec) {
    auto& vec = const_cast<std::vector<T>&> (inVec);
    data = vec.data();
    ldm  = vec.size();
    local_num_row = vec.size();
    local_num_col = 1;
  }

  bool is_valid() const {
    if(ldm < local_num_row)
      return false;
    else
      return true;
  }

  void debug_print() const {
    std::cout << "value = \n";
    for(size_t i=0; i<local_num_row; i++){
       for(size_t j=0; j<local_num_col; j++)
          std::cout << data[j*ldm+i] << " ";
       std::cout << std::endl;
    }
  }

  colmajor_matrix_local<T> get_copy() {
    colmajor_matrix_local<T> ret(local_num_row,local_num_col);
    T* retp = &ret.val[0];
    auto ret_ldm = local_num_row;
    for(size_t i=0; i<local_num_row; i++){
      for(size_t j=0; j<local_num_col; j++)
        retp[j*ret_ldm+i] = data[j*ldm+i];
    }
    return ret;
  }

  T*  data;      // Pointer to the input (colmajor_matrix_local<T>) matrix
  size_t ldm;       // Leading dimension of the input matrix to be sliced
  size_t local_num_row; // No. of rows in the sliced matrix
  size_t local_num_col; // No. of cols in the sliced matrix
  SERIALIZE(data, ldm, local_num_row, local_num_col)
};


// This function can be used for creating sliced_colmajor_matrix_local<T> from both
// colmajor_matrix_local<T> and sliced_colmajor_matrix_local<T>
// because of the "const &" and implicit conversion constructor
template <class T>
sliced_colmajor_matrix_local<T>
make_sliced_colmajor_matrix_local(const sliced_colmajor_matrix_local<T>& inMat,
                                  size_t start_row_index,
                                  size_t start_col_index,
                                  size_t num_row,
                                  size_t num_col) {
  ASSERT_PTR(inMat.data);
  if(!inMat.is_valid()) 
    REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

  if(start_row_index >= inMat.local_num_row ||
     start_col_index >= inMat.local_num_col ||
     num_row > inMat.local_num_row-start_row_index ||
     num_col > inMat.local_num_col-start_col_index) 
    REPORT_ERROR(USER_ERROR,"Slicing not possible!!\n");
     // (start_row_index + num_row) > inMat.local_num_row ||
     // (start_col_index + num_col) > inMat.local_num_col ) 

  sliced_colmajor_matrix_local<T> outMat;
  outMat.ldm = inMat.ldm; 
  outMat.data = inMat.data + start_row_index + (start_col_index * inMat.ldm);
  outMat.local_num_row = num_row;
  outMat.local_num_col = num_col;

  return outMat;
}

/*
  distributed version
*/

template <class T>
struct sliced_blockcyclic_matrix_local {

  sliced_blockcyclic_matrix_local() : data(NULL), descA(NULL), 
         IA(0), JA(0), local_num_row(0), local_num_col(0) {}

  // implicit conversion: 
  // blockcyclic_matrix_local<T> => sliced_blockcyclic_matrix_local<T>
  sliced_blockcyclic_matrix_local(const blockcyclic_matrix_local<T>& inMat) {
    // 'inMat' needs to be const (to support rValue). 
    // But 'data/descA' pointer is a non-const pointer. 
    // Thus the below casting is required.
    auto& mat = const_cast<blockcyclic_matrix_local<T>&> (inMat);

    // node-local informations
    data = &mat.val[0];
    descA = &mat.descA[0];

    // global (distributed) informations
    IA = JA = 1;
    local_num_row = mat.descA[2];
    local_num_col = mat.descA[3];
  }

  T* data;            // pointer to the node-local distributed matrix
  int* descA;         // pointer to the node-local distributed array descriptor
  size_t IA;             // Start row-index of the global distributed submatrix
  size_t JA;             // Start col-index of the global distributed submatrix
  size_t local_num_row; // No. of rows in the global distributed submatrix
  size_t local_num_col; // No. of cols in the global distributed submatrix
  SERIALIZE(data, descA, IA, JA, local_num_row, local_num_col)
};


// returns an empty sliced matrix
template <class T>
sliced_blockcyclic_matrix_local<T>
get_empty_sliced_blockcyclic_matrix_local(bool dummy) {
  return sliced_blockcyclic_matrix_local<T>();
}

template <class T>
sliced_blockcyclic_matrix_local<T>
to_sliced_blockcyclic_matrix_local(blockcyclic_matrix_local<T>& inMat) {
  return sliced_blockcyclic_matrix_local<T>(inMat);
}

template <class T>
size_t get_local_num_row(sliced_blockcyclic_matrix_local<T>& inMat) {
  return inMat.local_num_row;
}

template <class T>
size_t get_local_num_col(sliced_blockcyclic_matrix_local<T>& inMat) {
  return inMat.local_num_col;
}


template <class T>
struct sliced_blockcyclic_matrix {

  sliced_blockcyclic_matrix() {
    auto dummy = make_node_local_allocate<bool>();
    data = std::move(dummy.map(get_empty_sliced_blockcyclic_matrix_local<T>));
    set_num(0,0);
  }

  sliced_blockcyclic_matrix(frovedis::node_local<sliced_blockcyclic_matrix_local<T>>&& inMat) : 
     data(std::move(inMat)) {}

  // implicit conversion:
  // blockcyclic_matrix<T> => sliced_blockcyclic_matrix<T>
  sliced_blockcyclic_matrix(const blockcyclic_matrix<T>& inMat) {
    auto& mat2 = const_cast<blockcyclic_matrix<T>&> (inMat);
    data = std::move(mat2.data.map(to_sliced_blockcyclic_matrix_local<T>));
    set_num(mat2.num_row, mat2.num_col);
  }

  void set_num(size_t nrow, size_t ncol) {
    num_row = nrow;
    num_col = ncol;
  }

  frovedis::node_local<sliced_blockcyclic_matrix_local<T>> data;
  size_t num_row, num_col; // global dimension of the distributed matrix
};


// slice-engine-functor for slicing a matrix from the input blockcyclic matrix
template <class T>
struct slice_matrix_from_blockcyclic_matrix {
  slice_matrix_from_blockcyclic_matrix() : 
        ridx(0), cidx(0), num_row(0), num_col(0) {}

  slice_matrix_from_blockcyclic_matrix(size_t st_rIndex, size_t st_cIndex, 
                                       size_t nrow, size_t ncol) :
     ridx(st_rIndex), cidx(st_cIndex), num_row(nrow), num_col(ncol) {}

  sliced_blockcyclic_matrix_local<T>
  operator() (sliced_blockcyclic_matrix_local<T>& inMat) {

    sliced_blockcyclic_matrix_local<T> outMat;

    // node-local informations
    outMat.data  = inMat.data;
    outMat.descA = inMat.descA;

    // global (distributed) informations
    outMat.IA = inMat.IA + ridx;
    outMat.JA = inMat.JA + cidx;
    outMat.local_num_row = num_row;
    outMat.local_num_col = num_col;

    return outMat;
  }

  size_t ridx, cidx, num_row, num_col;
  SERIALIZE(ridx, cidx, num_row, num_col)
};


// This function can be used for creating sliced_blockcyclic_matrix<T> from both
// blockcyclic_matrix<T> and sliced_blockcyclic_matrix<T>
// because of the "const &" and implicit conversion constructor
template <class T>
sliced_blockcyclic_matrix<T>
make_sliced_blockcyclic_matrix(const sliced_blockcyclic_matrix<T>& inMat,
                               size_t start_row_index,
		               size_t start_col_index,
		               size_t num_row,
		               size_t num_col) {

  auto& mat2 = const_cast<sliced_blockcyclic_matrix<T>&> (inMat);
  size_t local_num_row = mat2.data.map(get_local_num_row<T>).get(0);
  size_t local_num_col = mat2.data.map(get_local_num_col<T>).get(0);
  if(start_row_index >= local_num_row ||
     start_col_index >= local_num_col ||
     num_row > local_num_row-start_row_index  ||
     num_col > local_num_col-start_col_index) 
     REPORT_ERROR(USER_ERROR,"Slicing not possible!!\n");

  sliced_blockcyclic_matrix<T> outMat(mat2.data.
                               template map<sliced_blockcyclic_matrix_local<T>>(
                       slice_matrix_from_blockcyclic_matrix<T>(start_row_index,
			                                       start_col_index,
							       num_row,
							       num_col)));
  outMat.set_num(mat2.num_row, mat2.num_col);
  return outMat;
}

}

#endif
