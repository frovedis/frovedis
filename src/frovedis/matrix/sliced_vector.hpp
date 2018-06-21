#ifndef _SLICED_VECTOR_
#define _SLICED_VECTOR_

#include <iostream>
#include <vector>
#include "sliced_matrix.hpp"

namespace frovedis {

template <class T>
struct sliced_colmajor_vector_local {

  sliced_colmajor_vector_local() : data(NULL), size(0), stride(0) {}

  // implicit conversion: 
  // colmajor_matrix_local<T> => sliced_colmajor_vector_local<T>
  // colmajor matrix has to be of Mx1 dimension
  sliced_colmajor_vector_local(const colmajor_matrix_local<T>& inMat) {
    // 'inMat' needs to be const (to support rValue). 
    // But 'data' pointer is a non-const pointer. 
    // Thus the below casting is required.
    auto& mat = const_cast<colmajor_matrix_local<T>&> (inMat);
    if(mat.val.size() == 0)
      REPORT_ERROR(USER_ERROR,"Empty input matrix!!\n");

    if(mat.local_num_col > 1) {
      std::string msg = "Input matrix has more than one columns.\n";
      msg += "It can't be converted to a vector!!\n";
      REPORT_ERROR(USER_ERROR,msg);
    }
    data   = &mat.val[0]; 
    size   = mat.local_num_row;
    stride = 1;
  }
  // implicit conversion: std::vector<T> => sliced_colmajor_vector_local<T>
  sliced_colmajor_vector_local(const std::vector<T>& inVec) {
    // 'inVec' needs to be const (to support rValue). 
    // But 'data' pointer is a non-const pointer. 
    // Thus the below casting is required.
    auto& vec = const_cast<std::vector<T>&> (inVec);
    if(vec.size() == 0) 
      REPORT_ERROR(USER_ERROR,"Empty input vector!!\n");
          
    data   = &vec[0]; 
    size   = vec.size();
    stride = 1;
  }
  bool is_valid() const {
    if(size < 0 || stride < 0)
      return false;
    else
      return true;
  }
  void debug_print() const {
    std::cout << "value = ";
    for(size_t i=0; i<size; i++) std::cout << data[i*stride] << " ";
    std::cout << std::endl;
  }

  T* data;    // pointer to the target row/col vector of the input matrix
  size_t size;   // number of elements in the sliced vector
  size_t stride; // stride between two consecutive elements in the sliced vector
  SERIALIZE(data, size, stride)
};

// Target row vector can be created for both
// sliced_colmajor_matrix_local<T> and colmajor_matrix_local<T> objects
// because of the "const &" and implicit conversion constructor
template <class T>
sliced_colmajor_vector_local<T>
make_row_vector(const sliced_colmajor_matrix_local<T>& inMat,
                size_t row_index) {
  
  ASSERT_PTR(inMat.data);
  if(!inMat.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

  if(row_index < 0 || row_index >= inMat.sliced_num_row) 
    REPORT_ERROR(USER_ERROR,"Invalid row index!!\n");

  sliced_colmajor_vector_local<T> outVec;
  outVec.data   = inMat.data + row_index;
  outVec.size   = inMat.sliced_num_col;
  outVec.stride = inMat.ldm;

  return outVec;
}

// Target column vector can be created for both
// sliced_colmajor_matrix_local<T> and colmajor_matrix_local<T> objects
// because of the "const &" and implicit conversion constructor
template <class T>
sliced_colmajor_vector_local<T>
make_col_vector(const sliced_colmajor_matrix_local<T>& inMat,
                size_t col_index) {

  ASSERT_PTR(inMat.data);
  if(!inMat.is_valid())
    REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

  if(col_index < 0 || col_index >= inMat.sliced_num_col) 
    REPORT_ERROR(USER_ERROR,"Invalid col index!!\n");

  sliced_colmajor_vector_local<T> outVec;
  outVec.data   = inMat.data + col_index * inMat.ldm;
  outVec.size   = inMat.sliced_num_row;
  outVec.stride = 1;

  return outVec;
}

/*
  distributed version
*/

template <class T>
struct sliced_blockcyclic_vector_local {

  sliced_blockcyclic_vector_local() : data(NULL), descA(NULL),
      IA(0), JA(0), size(0), stride(0) {}

  // implicit conversion:
  // blockcyclic_matrix_local<T> => sliced_blockcyclic_vector_local<T>
  // input matrix has to be of Mx1 Dimension 
  sliced_blockcyclic_vector_local(const blockcyclic_matrix_local<T>& inMat) {
    // 'inMat' needs to be const (to support rValue). 
    // But 'data/descA' pointer is a non-const pointer. 
    // Thus the below casting is required.
    auto& mat = const_cast<blockcyclic_matrix_local<T>&> (inMat);

    if(!mat.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input blockcyclic matrix!!\n");
    if(mat.descA[3] > 1) {
      std::string msg = "Input matrix has more than one columns.\n";
      msg += "It can't be converted to a vector!!\n";
      REPORT_ERROR(USER_ERROR,msg);
    }

    // node-local informations
    data  = &mat.val[0];
    descA = &mat.descA[0];
    // global informations
    IA = JA = stride = 1;
    size = mat.descA[2];
  }
  bool is_valid() const {
    if(IA < 0 || JA < 0 || size < 0 || stride < 0)
      return false;
    else
      return true;
  }

  // pointer to the node-local matrix of the global distributed matrix
  // from which (distributed) vector to be sliced
  T* data;     
  int* descA; // pointer to the node-local distributed matrix array descriptor
  size_t IA;   // start row-index of the global distributed matrix
  size_t JA;   // start col-index of the global distributed matrix
  size_t size; // total no. of elements in the sliced (distributed) vector
  size_t stride; // stride between two consecutive elements of the sliced (distributed) vector
  SERIALIZE(data, descA, IA, JA, size, stride)
};

// returns an empty matrix
template <class T>
sliced_blockcyclic_vector_local<T>
get_empty_sliced_blockcyclic_vector_local(bool dummy) {
  return sliced_blockcyclic_vector_local<T>();
}

template <class T>
sliced_blockcyclic_vector_local<T>
to_sliced_blockcyclic_vector_local(blockcyclic_matrix_local<T>& inMat) {
  return sliced_blockcyclic_vector_local<T> (inMat);
}

template <class T>
struct sliced_blockcyclic_vector {

  sliced_blockcyclic_vector() {
    auto dummy = make_node_local_allocate<bool>();
    data = std::move(dummy.map(get_empty_sliced_blockcyclic_vector_local<T>));
    set_num(0);
  }

  sliced_blockcyclic_vector(frovedis::node_local<sliced_blockcyclic_vector_local<T>>&& v) : 
     data(std::move(v)) {}

  // implicit conversion:
  // blockcyclic_matrix<T> => sliced_blockcyclic_vector<T>
  sliced_blockcyclic_vector(const blockcyclic_matrix<T>& inMat) {
    auto& mat2 = const_cast<blockcyclic_matrix<T>&> (inMat);
    if(mat2.num_col > 1) {
       std::string msg = "Input matrix has more than one columns.\n";
       msg += "It can't be converted to a vector!!\n"; 
       REPORT_ERROR(USER_ERROR,msg);
    }
    data = std::move(mat2.data.map(to_sliced_blockcyclic_vector_local<T>));
    set_num(mat2.num_row);
  }
  void set_num(size_t len) {
    size = len;
  }

  frovedis::node_local<sliced_blockcyclic_vector_local<T>> data;
  size_t size; // global size of the distributed vector
};

// slice-engine-functor for slicing a vector from the input blockcyclic matrix
template <class T>
struct slice_vector_from_blockcyclic_matrix {

  slice_vector_from_blockcyclic_matrix() : index(-1), what('\0') {}

  slice_vector_from_blockcyclic_matrix(size_t idx, char choice) : 
                                   index(idx), what(choice) {}

  sliced_blockcyclic_vector_local<T>
  operator() (sliced_blockcyclic_matrix_local<T>& inMat) {
    if(!inMat.is_valid())
      REPORT_ERROR(USER_ERROR,"Invalid input matrix!!\n");

    sliced_blockcyclic_vector_local<T> outVec;

    // local informations
    outVec.data  = inMat.data;
    outVec.descA = inMat.descA;

    // global informations
    if(what == 'r') { // for row-vector
      outVec.IA = inMat.IA + index;
      outVec.JA = inMat.JA;
      outVec.size   = inMat.sliced_num_col;
      // leadieng dimension (no. of rows) of the global (distributed) matrix
      outVec.stride = inMat.descA[2]; 
    }
    else { // for col-vector
      outVec.IA = inMat.IA;
      outVec.JA = inMat.JA + index;
      outVec.size   = inMat.sliced_num_row;
      outVec.stride = 1;
    }

    return outVec;
  }

  size_t index;
  char what;
  SERIALIZE(index, what)
};

// Target row vector can be created for both
// sliced_blockcyclic_matrix<T> and blockcyclic_matrix<T> objects
// because of the "const &" and implicit conversion constructor
template <class T>
sliced_blockcyclic_vector<T>
make_row_vector(const sliced_blockcyclic_matrix<T>& inMat, size_t rIndex) {
   auto& mat2 = const_cast<sliced_blockcyclic_matrix<T>&> (inMat);
   size_t sliced_num_row = mat2.data.map(get_sliced_num_row<T>).get(0);
   if(rIndex < 0 || rIndex >= sliced_num_row)
      REPORT_ERROR(USER_ERROR,"Invalid row index!!\n");
   sliced_blockcyclic_vector<T> outVec(mat2.data.
                                template map<sliced_blockcyclic_vector_local<T>>
                                (slice_vector_from_blockcyclic_matrix<T>(rIndex, 'r')));
   outVec.set_num(mat2.num_col); // global (distributed) row-size
   return outVec;
}

// Target column vector can be created for both
// sliced_blockcyclic_matrix<T> and blockcyclic_matrix<T> objects
// because of the "const &" and implicit conversion constructor
template <class T>
sliced_blockcyclic_vector<T>
make_col_vector(const sliced_blockcyclic_matrix<T>& inMat, size_t cIndex) {
   auto& mat2 = const_cast<sliced_blockcyclic_matrix<T>&> (inMat);
   size_t sliced_num_col = mat2.data.map(get_sliced_num_col<T>).get(0);
   if(cIndex < 0 || cIndex >= sliced_num_col)
      REPORT_ERROR(USER_ERROR,"Invalid col index!!\n");
   sliced_blockcyclic_vector<T> outVec(mat2.data.
                                template map<sliced_blockcyclic_vector_local<T>>
                                (slice_vector_from_blockcyclic_matrix<T>(cIndex, 'c')));
   outVec.set_num(mat2.num_row); // global (distributed) col-size
   return outVec;
}

}

#endif
